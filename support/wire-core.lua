------------------------------------------------------------------------------
-- author: Sergei Fedorov <sergei.a.fedorov@gmail.com>
-- Copyright (c) 2017, Sergei Fedorov
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- do not modify this table
local debug_level = {
    DISABLED = 0,
    LEVEL_1  = 1,
    LEVEL_2  = 2
}
------------------------------------------------------------------------------
-- set this DEBUG to debug_level.LEVEL_1 to enable printing debug_level info
-- set it to debug_level.LEVEL_2 to enable really verbose printing
-- set it to debug_level.DISABLED to disable debug printing
-- note: this will be overridden by user's preference settings
local DEBUG = debug_level.LEVEL_2

-- a table of our default settings - these can be changed by changing
-- the preferences through the GUI or command-line; the Lua-side of that
-- preference handling is at the end of this script file
local default_settings =
{
    debug_level  = DEBUG,
    enabled      = true, -- whether this dissector is enabled or not
}


dprint = function() end
dprint2 = function() end

local function resetDebugLevel()
    if default_settings.debug_level > debug_level.DISABLED then
        dprint = function(...)
            info(table.concat({"WIRE: ", ...}," "))
        end

        if default_settings.debug_level > debug_level.LEVEL_1 then
            dprint2 = dprint
        end
    else
        dprint = function() end
        dprint2 = dprint
    end
end
-- call it now
resetDebugLevel()

----------------------------------------
-- a function to convert tables of enumerated types to value-string tables
-- i.e., from { "name" = number } to { number = "name" }
local function makeValString(enumTable)
    local t = {}
    for name,num in pairs(enumTable) do
        t[num] = name
    end
    return t
end

local flagstype = {
    request   = 0x00,
    reply     = 0x02,
    validate  = 0x03,
    close     = 0x04,
    protocol  = 0x08,
    encoding  = 0x10,
}

local reqmode = {
    normal    = 0x00,
    one_way   = 0x01
}

local repstatus = {
    success                 = 0x00,
    success_no_body         = 0x01,
    user_exception          = 0x02,
    no_object               = 0x03,
    no_facet                = 0x04,
    no_operation            = 0x05,
    unknown_wire_exception  = 0x06,
    unknown_user_exception  = 0x07,
    unknown_exception       = 0x08,
}

local msgtype_valstr 	  	= makeValString(flagstype)
local reqmode_valstr        = makeValString(reqmode)
local repstatus_valstr  	= makeValString(repstatus)

------------------------------------------------------------------------------
--	Helper functions
------------------------------------------------------------------------------
local function add_tree_field(tree, proto, tvbuf, val)
	-- https://wiki.wireshark.org/LuaAPI/TreeItem
	-- treeitem:add(proto_field [,tvbrange] [,value [,text1 [,text2] ...] ])
	local item = tree:add(proto, tvbuf:range(val.offset, val.size), val.value)
	return item
end

local function qname_abbrev(owner_name, name)
    local fname     = owner_name .. "::" .. name
    local abbrev    = fname:gsub("::", ".")
    if abbrev:find("wire", 0) == nil then
        abbrev = "wire." .. abbrev
    end
    return fname, abbrev
end

-- this holds the plain "data" Dissector for opaque message contents
local data = Dissector.get("data")


local field_protos = {
	none 		= function ( abbrev, name, description )
		return ProtoField.new(name, abbrev, ftypes.NONE, nil, nil, nil, description)
	end,
	string 		= function ( abbrev, name, description )
		return ProtoField.new(name, abbrev, ftypes.STRING, nil, nil, nil, description)
	end,
	bool 		= function ( abbrev, name, mask )
		return ProtoField.new(name, abbrev, ftypes.BOOLEAN, nil, base.HEX, mask)
	end,
	enum 		= function ( abbrev, name, strings, mask, size, base_ )
		if size == nil then
			size = 8
		end
		if base_ == nil then
			base_ = base.HEX
		end
		ft = nil
		if size == 8 then
			ft = ftypes.UINT8
		elseif size == 16 then
			ft = ftypes.UINT16
		elseif size == 32 then
			ft = ftypes.UINT32
		elseif size == 64 then
			ft = ftypes.UINT64
		end

		return ProtoField.new(name, abbrev, ft, strings, base_, mask)
	end
}

wire = {
------------------------------------------------------------------------------
--	Table of wire function call parsers by their hash
------------------------------------------------------------------------------
function_dictionary = {
},

------------------------------------------------------------------------------
--   Constants

----------------------------------------
-- a table of all of our Protocol's fields
hdr_fields =
{
    header        = field_protos.string ("wire.header", 				"Header",				"Wire message header"		),
    magic_type    = field_protos.string ("wire.magic", 					"Magic"												),
    msg_type      = field_protos.enum 	("wire.type", 					"Type", 				msgtype_valstr, 0x07		),
    proto_flag    = field_protos.bool   ("wire.header.proto_flag", 		"Protocol Flag", 		0x08),
    enc_flag      = field_protos.bool   ("wire.header.encoding_flag", 	"Encoding Flag", 		0x10),

    proto_version = field_protos.string ("wire.header.proto_version", 	"Protocol Version", 	"Wire protocol version"		),
    enc_version   = field_protos.string ("wire.header.enc_version", 	"Encoding Version", 	"Wire encoding version"		),
    msg_size      = field_protos.string ("wire.length", 				"Length", 				"Wire message length"		),

    request       = field_protos.string ("wire.request", 				"Request", 				"Wire request"				),
    request_no    = field_protos.string ("wire.request.number", 		"Number", 				"Request number"			),
    request_tgt   = field_protos.string ("wire.request.target", 		"Request Target", 		"Wire request target"		),
    request_fct   = field_protos.string ("wire.request.facet", 			"Request Facet", 		"Wire request facet"		),
    request_op    = field_protos.string ("wire.request.op", 			"Request Operation",	"Wire request operation"	),
    request_mode  = field_protos.enum 	("wire.request.mode", 			"Request Mode", 		reqmode_valstr, 0x1 		),
    request_multi = field_protos.bool   ("wire.request.multi_target", 	"Multi-Target",			0x2 						),
    request_noctx = field_protos.bool   ("wire.request.no_context", 	"No Context", 			0x10 						),
    request_nobody= field_protos.bool   ("wire.request.no_body", 		"No Body", 				0x20 						),
    request_mt    = field_protos.string ("wire.request.targets", "Targets", "Wire request targets"),

    reply         = field_protos.string ("wire.reply",					"Reply",				"Wire reply"				),
    reply_no      = field_protos.string ("wire.reply.number", 			"Number", 				"Reply number"				),
    reply_status  = field_protos.enum 	("wire.reply.status", 			"Reply Status", 		repstatus_valstr			),

    encaps        = field_protos.string ("wire.encaps",                 "Encapsulation",        "Wire encapsulation"        ),
    encaps_version= field_protos.string ("wire.encaps.version",         "Encoding version",     "Encapsulation verion"      ),
    encaps_ind_tbl= field_protos.string ("wire.encaps.indirection_tbl",	"Indirection table", 	"Indirection table"   		),

    func_params   = field_protos.string ("wire.request.params", 		"Params", 				"Wire reqest params"		),

    seq_element   = field_protos.string ("wire.sequence.element",       "element"                                           )
},

------------------------------------------------------------------------------
--	Wire encoding
------------------------------------------------------------------------------
encoding = {
------------------------------------------------------------------------------
--   Constants
WIRE_MIN_HEADER_SIZE = 6,
WIRE_MAX_HEADER_SIZE = 18,

flagstype		  = flagstype,
msgtype_valstr 	  = makeValString(flagstype),
reqmode   		  = reqmode,
reqmode_valstr    = makeValString(reqmode),
repstatus 		  = repstatus,
repstatus_valstr  = makeValString(repstatus),
--------------------------------------------------------------------------------
read_bool = function (tvbuf, offset, max_bytes)
	local byte = tvbuf:range(offset, 1):uint()
	return 1, byte ~= 0
end,

--------------------------------------------------------------------------------
--  Read a variable-length uint from buffer
--  Returns consumed length and the int value
--  If the length is 0 means read failure
read_uint = function(tvbuf, offset, max_bytes)
    local msglen = tvbuf:len() - offset

    local val = 0
    local n = 0
    local more = true
    while more and (n * 7 <= max_bytes * 8) and msglen > 0 do
        local byte = tvbuf:range(offset + n, 1):uint()
        val = bit.bor( val, bit.lshift( bit.band(byte, 0x7f), 7 * n ) )
        more = bit.band(byte, 0x80) > 0
        n = n + 1
        msglen = msglen - 1
    end
    if more then
        return 0
    end
    return n, val
end,

read_uint_field = function(tvbuf, offset, max_bytes)
	local n, val = wire.encoding.read_uint(tvbuf, offset, max_bytes)
	if n <= 0 then
		return 0
	end
	return n, { value = val, offset = offset, size = n }
end,

dissect_uint_field = function ( tvbuf, offset, proto, tree, pktinfo, max_bytes )
    local n, v = wire.encoding.read_uint_field( tvbuf, offset, max_bytes)
    if n > 0 then
        add_tree_field(tree, proto, tvbuf, v)
    end
    return n, v        
end,

--------------------------------------------------------------------------------
--  Read string from buffer
--  Return number of bytes consumed and string together with sizes and offsets
read_string = function(tvbuf, offset)
    --dprint2("Read string")
    local n, sz = wire.encoding.read_uint(tvbuf, offset, 8)
    if n == 0 then
        dprint2("Failed to read string size")
        return 0
    end
    --dprint2("String size " .. sz)
    local str = {}
    if sz > 0 then
        if sz > tvbuf:len() then
            str = {
                offset    = offset,
                size      = n + sz,
                size_len  = n,
                value     = "Invalid string size " .. sz
            }
            return 0, str
        else
            str = {
                offset    = offset,
                size      = n + sz,
                size_len  = n,
                value     = tvbuf:range(offset + n, sz):string()
            }
        --dprint2("Value is " .. str.value)
        end
    else
        str = {
            offset    = offset,
            size      = n,
            size_len  = n,
            value     = ""
        }
        --dprint2("Value is nil")
    end
    return n + sz, str
end,

read_string_field = function(tvbuf, offset)
	local n, val = wire.encoding.read_string(tvbuf, offset)
	return n, val
end,

--------------------------------------------------------------------------------
--  Read uuid from buffer
--
read_uuid = function(tvbuf, offset)
    --dprint2("Read uuid")
    local uuid_str = ""
    for i=0,15 do
        if i == 4 or i == 6 or i == 8 or i == 10 then
            uuid_str = uuid_str .. "-"
        end
        local byte = tvbuf:range(offset + i, 1):uint()
        uuid_str = uuid_str .. string.format("%02x", byte)
    end
    return 16, uuid_str
end,

read_uuid_field = function(tvbuf, offset)
	local n, value = wire.encoding.read_uuid(tvbuf, offset)

	return {
		offset 	= offset,
		size 	= n,
		value   = value
	}
end,

--------------------------------------------------------------------------------
--  Read identity from buffer
--  Return number of bytes consumed and identity object (if any)
read_identity = function(tvbuf, offset)
    dprint2("Read identity")
    local consumed = 0
    local n, cat = wire.encoding.read_string(tvbuf, offset)
    if n == 0 then
        dprint("Failed to read identity:category from buffer")
        return 0
    end
    local identity = {
        offset  = offset,
        cat_size    = n,
        category    = cat
    }
    if identity.category.value ~= nil then
        identity.value = identity.category.value .. "/"
    else
        identity.value = nil
    end
    consumed = consumed + n
    local type = tvbuf:range(offset + consumed, 1):uint()
    consumed = consumed + 1
    if type == 0 then -- id is string
        dprint2("Id is string")
        local n, id = wire.encoding.read_string(tvbuf, offset + consumed)
        if n == 0 then
            dprint("Failed to read identity:id string from buffer")
            return 0
        end
        consumed = consumed + n
        identity.id = id
        identity.id_size = n

        if identity.value ~= nil then
            identity.value = identity.value .. id.value
        else
            identity.value = id.value
        end
    elseif type == 1 then  -- id is uuid
        dprint2("Id is uuid")
        local n, id = wire.encoding.read_uuid(tvbuf, offset + consumed)
        if n == 0 then
            dprint("Failed to read identity:id uuid from buffer")
            return 0
        end
        consumed = consumed + n
        identity.id = id
        identity.id_size = n

        if identity.value ~= nil then
            identity.value = identity.value .. id
        else
            identity.value = id
        end
    else -- id is wildcard
        dprint2("Id is wildcard")
        consumed = consumed + 1
        identity.value = identity.value .. "*"
    end
    if identity.value == nil then
        dprint2("Identity value is nil")
        identity.value = "<none>"
    end
    identity.size = consumed
    return consumed, identity
end,

--------------------------------------------------------------------------------
--  Read a sequence of elements
read_sequence = function(tvbuf, offset, elem_func)
    local seq = {
        offset = offset,
    }
    local n, sz = wire.encoding.read_uint(tvbuf, offset, 8)
    if n == 0 then
        dprint("Failed to read sequence size")
        return 0
    end
    if sz > 0 then
        dprint2("Read sequence size " .. sz)
        for i = 1, sz do
            local c, elem = elem_func(tvbuf, offset + n)
            if c == 0 then
                dprint("Failed to read sequence element")
                return 0
            end
            seq[i] = elem
            n = n + c
        end
    end
    seq.size = n
    return n, seq
end,

--------------------------------------------------------------------------------
--  Read identity/facet pair
read_invocation_target = function(tvbuf, offset)
    dprint2("Read target identity")
    local n, id = wire.encoding.read_identity(tvbuf, offset)
    if n == 0 then
        dprint("Failed to read identity")
        return 0
    end
    dprint2("Read target facet")
    local s, fct = wire.encoding.read_string(tvbuf, offset)
    if s == 0 then
        dprint("Failed to read facet")
        return 0
    end
    return n + s, { identity = id, facet = fct, offset = id.offset, size = id.size + fct.size }
end,

--------------------------------------------------------------------------------
--  Read operation id
read_operation = function(tvbuf, offset)
    local op = {
        offset = offset
    }
    op.type = tvbuf:range(offset, 1):uint()
    local consumed = 1
    if op.type == 0 then -- Op hash
        local hash = tvbuf:range(offset + consumed, 4):le_uint()
        local h_string = string.format("0x%x", hash)
        if wire.types.functions[h_string] ~= nil then
        	op.func = wire.types.functions[h_string]
        	op.value = h_string .. "(" .. op.func.name .. ")"
		else
        	op.value = h_string
        end
        consumed = consumed + 4
    else  -- Op name
        local n, name = wire.encoding.read_string(tvbuf, offset + consumed)
        if n == 0 then
            dprint("Failed to read literal op name")
            return 0
        end
        op.value = name.value
        consumed = consumed + n
    end
    op.size = consumed
    return consumed, op
end,

--------------------------------------------------------------------------------
--  Read version information from buffer
--  uint32 major uint32 minor
--  Returns number of consumed bytes and version object (if any)
read_version = function(tvbuf, offset)
    local consumed = 0
    local version  = {
        offset = offset
    }
    local n, val = wire.encoding.read_uint(tvbuf,offset,4)
    if n == 0 then
        dprint("Failed to read protocol major")
        return 0
    end
    version.major = val
    consumed = consumed + n
    offset = offset + consumed
    n, val = wire.encoding.read_uint(tvbuf,offset,4)
    if n == 0 then
        dprint("Failed to read protocol minor")
        return 0
    end
    version.minor = val
    consumed = consumed + n
    version.size = consumed
    version.value = version.major .. "." .. version.minor
    return consumed, version
end,

},
------------------------------------------------------------------------------
--	Wire encoding end
------------------------------------------------------------------------------
------------------------------------------------------------------------------
--	Wire protocol
------------------------------------------------------------------------------
protocol = {
------------------------------------------------------------------------------
--	Read message header from buffer, check for sanity
--  Return number of bytes consumed and a structure descibing header
--    type
--    proto_version
--    encoding_version
--    TODO make all size/offset fields consistent
read_header = function( tvbuf, offset )
    local msglen = tvbuf:len() - offset

    -- check if capture was only capturing partial packet size
    if msglen ~= tvbuf:reported_length_remaining(offset) then
        -- captured packets are being sliced/cut-off, so don't try to desegment/reassemble
        dprint("Captured packet was shorter than original, can't reassemble")
        return 0
    end

    if msglen < wire.encoding.WIRE_MIN_HEADER_SIZE then
        -- we need more bytes, so tell the main dissector function that we
        -- didn't dissect anything, and we need an unknown number of more
        -- bytes (which is what "DESEGMENT_ONE_MORE_SEGMENT" is used for)
        dprint("Need more bytes to figure out WIRE header")
        -- return as a negative number
        return -DESEGMENT_ONE_MORE_SEGMENT
    end

    -- if we got here, then we know we have enough bytes in the Tvb buffer
    -- to check the magic number and flags.
    local magic_tvbr = tvbuf:range(offset, 4)
    local magic_val  = magic_tvbr:string()

    if magic_val ~= "wire" then
        dprint("Invalid magic number")
        return 0
    end
    local start = offset
    offset = offset + 4

    local flags_tvbr = tvbuf:range(offset, 1)
    offset = offset + 1
    local flags_val  = flags_tvbr:uint()

    local msg_header = {}
    msg_header.type = bit32.band(flags_val, 0x7)
    dprint2("Message flags value " .. flags_val .. " type " .. msg_header.type)
    if (bit32.band(flags_val, 0x8) > 0) then
        dprint2("Read protocol version")
        local consumed, tmp = wire.encoding.read_version(tvbuf,offset)
        if consumed <= 0 then
            dprint("Failed to read protocol version")
            return -DESEGMENT_ONE_MORE_SEGMENT
        end
        msg_header.proto_version = tmp
        offset = offset + consumed
        dprint2("Protocol version " .. msg_header.proto_version.value ..
            " offset " .. tmp.offset .. " size " .. tmp.size)
    end
    if (bit32.band(flags_val, 0x10) > 0) then
        dprint2("Read encoding version")
        local consumed, tmp = wire.encoding.read_version(tvbuf,offset)
        if consumed <= 0 then
            dprint("Failed to read encoding version")
            return -DESEGMENT_ONE_MORE_SEGMENT
        end
        msg_header.encoding_version = tmp
        offset = offset + consumed
        dprint2("Encoding version " .. msg_header.encoding_version.value)
    end
    local consumed, msg_size = wire.encoding.read_uint(tvbuf,offset,8)
    if (consumed <= 0) then
        return -DESEGMENT_ONE_MORE_SEGMENT
    end
    msg_header.size_offset = offset
    msg_header.size_length = consumed
    offset = offset + consumed
    dprint2("Message size " .. msg_size)

    msglen = tvbuf:len() - offset
    if msglen < msg_size then
        dprint2("Need more bytes to desegment wire message")
        return -(msg_size - msglen)
    end

    msg_header.size = msg_size
    msg_header.hdr_size = offset - start
    return offset - start, msg_header
end,

--------------------------------------------------------------------------------
dissect_encapsulation = function ( tvbuf, pktinfo, offset, tree, parse_fn )
    dprint2("Dissect encapsulation")

    local ver_size, ver = wire.encoding.read_version(tvbuf, offset)
    if ver_size <= 0 then
    	dprint2("Failed to read encapsulation version")
    	return 0
    end
    local n, sz = wire.encoding.read_uint_field(tvbuf, offset + ver_size, 8)
    if n <= 0 then
    	dprint2("Failed to read encapsulation size")
    	return 0
    end
    local consumed = ver_size + n
    dprint2("Encapsulation version", ver.value, "size", sz.value)

    local inf = "version " .. ver.value .. " size " .. sz.value
    local enc_tree = tree:add(wire.hdr_fields.encaps, tvbuf:range(offset, consumed + sz.value), inf)
    add_tree_field(enc_tree, wire.hdr_fields.encaps_version, tvbuf, ver)
    add_tree_field(enc_tree, wire.hdr_fields.msg_size, tvbuf, sz)

    if sz.value > 0 then
    	if parse_fn ~= nil then
    		local n = parse_fn(tvbuf, offset + consumed, tree, pktinfo)
    		if n < sz.value then
                local ind_table_sz = sz.value - n
    			dprint2("Encapsulation indirection table size", ind_table_sz, "bytes")
                local ind_table = enc_tree:add(wire.hdr_fields.encaps_ind_tbl, tvbuf:range(offset + consumed + n, ind_table_sz))

                -- TODO Read indirection table
                local tvb = tvbuf(offset + consumed + n, ind_table_sz):tvb()
                data:call(tvb, pktinfo, ind_table)
    		end
    	else
	    	local tvb = tvbuf(offset + consumed, sz.value):tvb()
	    	data:call(tvb, pktinfo, enc_tree)
    	end
	    consumed = consumed + sz.value
    end


	return consumed
end,

--------------------------------------------------------------------------------
dissect_request = function(tvbuf, pktinfo, root, offset)
    local inf = ""
    local req = {}
    dprint2("Dissect request header")
    local consumed, req_no = wire.encoding.read_uint(tvbuf,offset,8)
    if consumed == 0 then
        dprint("Failed to read request number")
        return 0
    end
    local num_sz = consumed

    inf = inf .. " #" .. req_no

    local n, identity = wire.encoding.read_identity(tvbuf, offset + consumed)
    if n == 0 then
        dprint("Failed to read request target")
        return 0
    end
    consumed = consumed + n

    -- read facet
    local n, facet = wire.encoding.read_string(tvbuf, offset + consumed)
    if n == 0 then
        dprint("Failed to read request target facet")
        return 0
    end
    if facet.value == nil then
        facet.value = "none"
    end
    consumed = consumed + n

    -- read op
    local n, op = wire.encoding.read_operation(tvbuf, offset + consumed)
    if n == 0 then
        dprint("Failed to read op specs")
        return 0
    end
    consumed = consumed + n

    local mode = tvbuf:range(offset + consumed, 1):uint()
    consumed = consumed + 1

    -- TODO Read context

    local multitarget = false
    local targets = {}
    -- Read multitarget
    if bit.band(mode, 0x02) > 0 then
        dprint2("Read multitarget")
        local n = 0;
        n, targets = wire.encoding.read_sequence(tvbuf, offset + consumed, wire.encoding.read_invocation_target)
        consumed = consumed + n
        multitarget = true
    end

    if multitarget == false then
        dprint2("Request target: " .. identity.value)
        inf = inf .. " " .. identity.value
    else
        inf = inf .. " <multiple targets>"
    end

    inf = inf .. "->" .. op.value
    pktinfo.cols.info:append(" Request" .. inf)
    root:append_text(" Request" .. inf)

    local req_tree = root:add(wire.hdr_fields.request, tvbuf:range(offset, consumed), inf)
    req_tree:add(wire.hdr_fields.request_no, tvbuf:range(offset, num_sz), req_no)
    req_tree:add(wire.hdr_fields.request_tgt, tvbuf:range(identity.offset, identity.size),
            identity.value)
    req_tree:add(wire.hdr_fields.request_fct, tvbuf:range(facet.offset, facet.size),
            facet.value)
    req_tree:add(wire.hdr_fields.request_op, tvbuf:range(op.offset, op.size),
            op.value)
    req_tree:add(wire.hdr_fields.request_mode, tvbuf:range(op.offset + op.size, 1))
    req_tree:add(wire.hdr_fields.request_multi, tvbuf:range(op.offset + op.size, 1))
    req_tree:add(wire.hdr_fields.request_noctx, tvbuf:range(op.offset + op.size, 1))
    req_tree:add(wire.hdr_fields.request_nobody, tvbuf:range(op.offset + op.size, 1))

    if (multitarget == true) then
        dprint2("Targets offset " .. targets.offset .. " size " .. targets.size .. " count " .. #targets)
        local mt_tree = req_tree:add(hdr_fields.request_mt, tvbuf:range(targets.offset, targets.size), "Count " .. #targets)
        for i, tgt in ipairs(targets) do
            local inf = tgt.identity.value
            if tgt.facet.value ~= nil then
                inf = inf .. " [" .. tgt.facet.value .. "]"
            end
            dprint2("Target " .. inf)
            mt_tree:add(wire.hdr_fields.request_tgt, tvbuf:range(tgt.offset, tgt.size),
                    inf)
        end
    end

    local fn = nil

    if op.func ~= nil and op.func.dissect ~= nil then
    	fn = op.func.dissect
    end

    n = wire.protocol.dissect_encapsulation(tvbuf, pktinfo, offset + consumed, req_tree, fn)
    if n <= 0 then
    	return 0
    end
    consumed = consumed + n

    return consumed
end,

--------------------------------------------------------------------------------
--  Dissect reply
dissect_reply = function(tvbuf, pktinfo, root, offset)
    local rep = {}
    dprint2("Dissect reply header")
    local consumed, req_no = wire.encoding.read_uint(tvbuf,offset,8)
    if consumed == 0 then
        dprint("Failed to read request number")
        return 0
    end
    local num_sz = consumed
    local status = tvbuf:range(offset + consumed, 1):uint()

    local inf = " #" .. req_no .. " " .. wire.encoding.repstatus_valstr[status]
    pktinfo.cols.info:append(" Reply" .. inf)
    root:append_text(" Reply" .. inf)

    consumed = consumed + 1 -- message status
    local rep_tree = root:add(wire.hdr_fields.reply, tvbuf:range(offset, consumed), inf)
    rep_tree:add(wire.hdr_fields.reply_no, tvbuf:range(offset, num_sz), req_no)
    rep_tree:add(wire.hdr_fields.reply_status, tvbuf:range(offset + num_sz, 1))

    local n = wire.protocol.dissect_encapsulation(tvbuf, pktinfo, offset + consumed, rep_tree, nil)
    if n <= 0 then
    	return 0
    end
    consumed = consumed + n

    return consumed
end,


},
------------------------------------------------------------------------------
--	Wire protocol end
------------------------------------------------------------------------------
------------------------------------------------------------------------------
--	Wire types
------------------------------------------------------------------------------
types = {

type_dictionary = {},

--  Parser dictionary
parsers 		= {
	string 		= {
		dissect = function ( tvbuf, offset, proto, tree, pktinfo )
			-- TODO Add the tree field
			local n, v = wire.encoding.read_string_field(tvbuf, offset)
            if n > 0 then
                add_tree_field(tree, proto, tvbuf, v)
            end
			return n, v
		end,
		proto   = function ( abbrev, name )
			return field_protos.string(abbrev, name)
		end
	},
	bool 		= {
		dissect = function ( tvbuf, offset, proto, tree, pktinfo )
			local n, v = wire.encoding.read_bool( tvbuf, offset, proto, tree, pktinfo )
            if n > 0 then
                tree:add(proto, tvbuf(offset, n))
            end
            return n, v
		end,
		proto   = function ( abbrev, name, mask )
			return field_protos.bool(abbrev, name, mask)
		end
	},
    uint16      = {
        dissect = function ( tvbuf, offset, proto, tree, pktinfo )
            return wire.encoding.dissect_uint_field( tvbuf, offset, proto, tree, pktinfo, 2)
        end,
        proto   = function ( abbrev, name )
            -- As far as ints are compressed, there is no way to use integer protos for fields
            return field_protos.string(abbrev, name)
        end
    },
    uint32      = {
        dissect = function ( tvbuf, offset, proto, tree, pktinfo )
            return wire.encoding.dissect_uint_field( tvbuf, offset, proto, tree, pktinfo, 4)
        end,
        proto   = function ( abbrev, name )
            -- As far as ints are compressed, there is no way to use integer protos for fields
            return field_protos.string(abbrev, name)
        end
    },
    uint64      = {
        dissect = function ( tvbuf, offset, proto, tree, pktinfo )
            return wire.encoding.dissect_uint_field( tvbuf, offset, proto, tree, pktinfo, 8)
        end,
        proto   = function ( abbrev, name )
            -- As far as ints are compressed, there is no way to use integer protos for fields
            return field_protos.string(abbrev, name)
        end
    },
    uuid        = {
        dissect = function ( tvbuf, offset, proto, tree, pktinfo )
            local n, v = wire.encoding.read_uuid_field(tvbuf, offset)
            if n > 0 then
                add_tree_field(tree, proto, tvbuf, v)
            end
            return n, v
        end,
        proto   = function ( abbrev, name )
            return field_protos.string(abbrev, name)
        end
    }
},

--  Dictionary of functions by hashes
functions = {},

--  Return a pair of lambdas - parser and field creator
type 			= function ( name )
	return wire.types.parsers[name]
end,

-- Enumeration
enum            = function ( name, definition )
    dprint2("Add enum", name, "definition")
    local valstr = makeValString(definition)
    local e = {
        name = name,
        enumerators  = valstr,
        dissect = function ( tvbuf, offset, proto, tree, pktinfo )
            local n, v = wire.encoding.read_uint(tvbuf, offset, 8)
            if n > 0 then
                local val = {
                    offset = offset,
                    size   = n,
                    value  = valstr[v]
                }
                if val.value == nil then
                    val.value = "<UNKNOWN VALUE " .. n .. ">"
                end
                add_tree_field(tree, proto, tvbuf, val)
            end
            return n, v
        end,
        proto   = function ( abbrev, name )
            return field_protos.string(abbrev, name)
        end,
    }
    wire.types.parsers[name] = e
    return e
end,

--  Sequence type
sequence 		= function ( element_type )
    local seq_name = "__sequence_of__." .. element_type.name
    if wire.types.parsers[seq_name] == nil then
        local abbrev = seq_name:gsub("::", ".")
        local elem_proto = element_type.proto(abbrev, "element")
        wire.hdr_fields[abbrev] = elem_proto

        wire.types.parsers[seq_name] = {
            name = seq_name,
            dissect = function ( tvbuf, offset, proto, tree, pktinfo )
                local consumed = 0
                local n, sz = wire.encoding.read_uint(tvbuf, offset, 8)
                if n <= 0 then
                    dprint("Failed to read sequence size")
                    return 0
                end
                consumed = consumed + n
                local seq_tree = tree:add(proto, tvbuf:range(offset, 0))
                seq_tree:append_text(n .. " element(s)")
                for i=1, sz do
                    local n, val = element_type.dissect(tvbuf, offset + consumed, elem_proto, seq_tree, pktinfo)
                    if n <= 0 then
                        dprint("Failed to read sequence element")
                        return 0
                    end
                    consumed = consumed + n
                end
                return consumed
            end,
            proto   = function ( abbrev, name )
                return field_protos.string(abbrev, name)
            end,
        }
    end
    return wire.types.parsers[seq_name]
end,

--  Dictionary
dictionary 		= function ( name, key_type, element_type )
	return {
        dissect = nil,
		proto   = nil,
	}
end,

-- Optional
optional 		= function ( element_type )
    return {
        dissect = function ( tvbuf, offset, proto, tree, pktinfo )
            local consumed = 0
            local n, b = wire.encoding.read_bool(tvbuf, offset)
            if n <= 0 then
                return 0
            end
            dprint2("Dissect optional value")
            consumed = consumed + n
            local v = nil
            if b then
                dprint2("Optional value is present")
                n, v = element_type.dissect(tvbuf, offset + consumed, proto, tree, pktinfo)
                if n <= 0 then
                    return 0
                end
                consumed = consumed + n
            else
                dprint2("Optional value is not present")
                local val = {
                    offset = offset,
                    size   = consumed,
                    value  = "optional value not present"
                }
                add_tree_field(tree, proto, tvbuf, val)
            end
            return consumed, v
        end,
        proto   = element_type.proto,
    }
end,

-- Variant
-- Accepts a list of dissectors for a type number and an optional type number to string map
variant 		= function ( variants, var_map )
    local name = "__variant_of__."
    for i, t in ipairs(variants) do
        name = name .. t.name
    end

    if wire.types.parsers[name] == nil then

        dprint2("Create variant type with", #variants, "types")
        local type_proto = nil
        if var_map ~= nil and var_map.enumerators ~= nil then
            local abbrev = name:gsub("::", ".")
            type_proto = var_map.proto(abbrev, "Variant type")

            for i,v in pairs(var_map.enumerators) do
                dprint2(i,v)
            end
        end
        wire.types.parsers[name] = {
            name = name,
            dissect = function ( tvbuf, offset, proto, tree, pktinfo )
                local consumed = 0
                local n, v = wire.encoding.read_uint(tvbuf, offset, 8)
                if n <= 0 then
                    dprint("Failed to read varant type index")
                    return 0
                end
                if v >= #variants then
                    dprint("Type index is invalid for variant value")
                    return 0
                end

                local val_tree = tree
                local val = nil
                if type_proto ~= nil then
                    local t = {
                        offset = offset,
                        size   = n,
                        value  = var_map.enumerators[v]
                    }
                    val_tree = add_tree_field(tree, type_proto, tvbuf, t)
                end
                consumed = consumed + n
                n, val = variants[v + 1].dissect(tvbuf, offset + consumed, proto, val_tree, pktinfo)
                if n <= 0 then
                    return 0
                end

                return consumed + n, val
            end,
            proto   = function ( abbrev, name )
                return field_protos.string(abbrev, name)
            end,
        }
    end
    return wire.types.parsers[name]
end,

alias           = function ( name, t )
    wire.types.parsers[name] = {
        name    = name,
        dissect = t.dissect,
        proto   = t.proto
    }
    return wire.types.parsers[name]
end,

-- Create a field/function param parser and proto
field           = function ( owner_name, definition )
    local fname, abbrev     = qname_abbrev(owner_name, definition.name)

    dprint2("Make field", definition.name, "at", owner_name, "abbrev", abbrev)
    local proto = definition.type.proto(abbrev, definition.name)
    wire.hdr_fields[ fname ] = proto
    return {
        name    = definition.name,
        dissect = definition.type.dissect, 
        proto   = proto,
    }
end,

func            = function ( owner_name, name, definition )
    local fname, abbrev     = qname_abbrev(owner_name, name)

    dprint2("Make function", name, "at", owner_name, "abbrev", abbrev, "hash", definition.hash)
    local proto = field_protos.string(abbrev, fname .. " params")
    wire.hdr_fields[ fname ] = proto

    -- Create param protos
    local params = {}
    if definition.params ~= nil then
        for i, field in ipairs(definition.params) do
            table.insert(params, wire.types.field(fname, field))
        end
    end

    local dissector = function ( tvbuf, offset, tree, pktinfo )
        local consumed = 0
        dprint2("Parse function", fname, "params")
        local param_tree = tree:add(proto, tvbuf:range(offset, 0))
        for i, p in ipairs(params) do
            dprint2("Parse", p.name, "param")
            local n, v = p.dissect(tvbuf, offset + consumed, p.proto, param_tree, pktinfo)
            if n <= 0 then
                return n
            end
            consumed = consumed + n
        end
        return consumed
    end

    local fn = {
        name    = fname,
        -- TODO invocation/return
        dissect = dissector,
        proto   = proto
    }

    wire.types.functions[definition.hash] = fn
    return fn
end,

--  Add/get wire structure
structure 		= function ( name, definition )
    if definition ~= nil then
        dprint2("Add structure", name, "definition")
		-- Create field protos
		local fields = {}
		if definition.fields ~= nil then
			for i, field in ipairs(definition.fields) do
                table.insert(fields, wire.types.field(name, field))
			end
		end
		-- Create dissector for fields
		local dissector = function ( tvbuf, offset, proto, tree, pktinfo )
            local consumed = 0
            local data     = {}
            -- Add field with type name
			local struct_tree = tree:add(proto, tvbuf:range(offset, 0), name)
            -- Dissect structure fields
            for i, f in ipairs(fields) do
                local n, v = f.dissect(tvbuf, offset + consumed, f.proto, struct_tree, pktinfo)
                if n <= 0 then
                    return n, nil
                end
                data[f.name] = v
                consumed = consumed + n
            end
			return consumed, data
		end

        wire.types.parsers[name] = {
            name    = name,
            dissect = dissector,
            proto   = wire.types.parsers.string.proto
        }
	end

   -- Find type by name
    return wire.types.parsers[name]
end,

interface 		= function ( name, definition )
    dprint2("Add interface", name, "definition")
	-- create parsers for functions
    for n, func in pairs(definition.functions) do
        wire.types.func(name, n, func)
    end
	return {}
end,

class 			= function ( name, definition )
	wire.types.interface( name, definition )
    -- TODO add by hash
	return wire.types.structure( name, definition )
end,

}, -- types

}

local function name_predefined_types()
    for n,t in pairs(wire.types.parsers) do
        dprint2("Predefined type", n)
        t.name = n
    end
end

name_predefined_types()
