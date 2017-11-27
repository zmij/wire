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
            local coerced = {"WIRE: ", }
            for i, v in ipairs({...}) do
                table.insert(coerced, tostring(v))
            end
            info(table.concat(coerced," "))
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

local segment_type_id = {
    type_number             = 0x00,
    string_type_id          = 0x01,
    hash_type_id            = 0x02,
}

local msgtype_valstr 	  	= makeValString(flagstype)
local reqmode_valstr        = makeValString(reqmode)
local repstatus_valstr  	= makeValString(repstatus)
local segment_type_id_valstr= makeValString(segment_type_id)

------------------------------------------------------------------------------
--	Helper functions
------------------------------------------------------------------------------
function range64( tvbuf, offset, size )
    return tvbuf:range(UInt64(offset):tonumber(), UInt64(size):tonumber())
end

local function tvb64( tvbuf, offset, size )
    dprint2("Create tvb", tostring(offset), ",", tostring(size))
    return tvbuf(UInt64(offset):tonumber(), UInt64(size):tonumber()):tvb()
end

local function add_tree_field(tree, proto, tvbuf, val)
	-- https://wiki.wireshark.org/LuaAPI/TreeItem
	-- treeitem:add(proto_field [,tvbrange] [,value [,text1 [,text2] ...] ])
	local item = tree:add(proto, range64(tvbuf, val.offset, val.size), tostring(val.value))
	return item
end

local function make_abbrev( name )
    local abbrev    = name:gsub("::", "."):gsub("^%.", ""):gsub("[<>,]", "_"):gsub(" ", "")

    if abbrev:find("wire", 0) == nil then
        abbrev = "wire." .. abbrev
    end
    return abbrev
end

local function qname_abbrev(owner_name, name)
    local fname     = owner_name .. "::" .. name
    return fname, make_abbrev(fname)
end

local function field_to_string( val )
    return tostring(val.value)
end

local function const_string (str)
    return function ()
        return str
    end
end
local function acceccor( field )
    return function ( val )
        return tostring(val[field])
    end
end

function make_formatter(fmt)
    if fmt == nil then
        return nil
    end
    if type(fmt) ~= "string" then
        return fmt
    end

    local placeholder_pattern = "((.-){([^}]+)})"
    local all, before, placeholder = fmt:match(placeholder_pattern)

    local formatters = {}

    local l = 1

    while all ~= nil do
        if (before ~= nil and before:len() > 0) then
            table.insert(formatters, const_string(before))
        end
        table.insert(formatters, acceccor(placeholder))
        l = l + all:len()

        all, before, placeholder = fmt:match(placeholder_pattern, l)
    end

    return function ( val )
        local res = ""
        for i, formatter in ipairs(formatters) do
            res = res .. formatter(val)
        end
        return res
    end
end

------------------------------------------------------------------------------
-- this holds the plain "data" Dissector for opaque message contents
local data = Dissector.get("data")


local field_protos = {
	none 		= function ( abbrev, name, description )
		return ProtoField.new(name, abbrev, ftypes.NONE, nil, nil, nil, description)
	end,
	string 		= function ( abbrev, name, description )
        dprint2("Make field abbrev", abbrev)
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

------------------------------------------------------------------------------
Version = {
    offset      = nil,
    size        = nil,

    major       = nil,
    minor       = nil,

    value       = nil,
}
function Version:new(ver)
    ver = ver or {
        offset      = nil,
        size        = nil,

        major       = nil,
        minor       = nil,

        value       = nil,
    }
    setmetatable(ver, self)
    self.__index = self
    return ver
end

Encapsulation = {
    tvbuf           = nil,
    offset          = nil,
    pktinfo         = nil,
    version         = nil,
    size            = nil,
    tree            = nil,
    ind_tree        = nil,
    indirection     = nil,
    types           = {},
}
function Encapsulation:new(encaps)
    encaps = encaps or {
        tvbuf           = nil,
        offset          = nil,
        pktinfo         = nil,
        tree            = nil,
        ind_tree        = nil,
    }
    encaps.version      = Version:new()
    encaps.size         = nil
    encaps.indirection  = {}

    setmetatable(encaps, self)
    self.__index = self
    return encaps
end

Type = {}
Type.prototype = {
    name        = "<UNNAMED TYPE>",
    dissect     = function ( self, encaps, offset, proto, tree )
        dprint("WARN Default empty dissect called")
        return 0, nil
    end,
    new_value   = function ( self, v )
        if v == nil then
            v = {}
        end
        v.__type = self
        setmetatable(v, Type.field_mt)
        return v
    end,
    add_to_tree = function ( self, tree, proto, tvbuf, n, v )
        if n > 0 then
            local val  = self:new_value(v)
            local item = proto ~= nil and add_tree_field(tree, proto, tvbuf, val) or nil
            return n, val, item
        end
        return n, nil, nil
    end,
    format      = nil,
}
Type.mt = {}
Type.mt.__index = Type.prototype
Type.mt.__tostring = function ( v )
    return "wire type " .. v.name
end

Type.field_proto = {
    offset  = 0,
    size    = 0,
    value   = nil
}
Type.field_mt = {}
Type.field_mt.__index = Type.field_proto
Type.field_mt.__tostring = function ( v )
    if v.__type ~= nil and v.__type.format ~= nil then
        return v.__type.format(v)
    end
    return tostring(v.__type.name)
end

function Type:new( t )
    setmetatable(t, self.mt)
    return t
end

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
    reply_exc     = field_protos.string ("wire.reply.exception",        "Exception",            "Wire reply exception"      ),

    encaps        = field_protos.string ("wire.encaps",                 "Encapsulation",        "Wire encapsulation"        ),
    encaps_version= field_protos.string ("wire.encaps.version",         "Encoding version",     "Encapsulation verion"      ),
    encaps_ind_tbl= field_protos.string ("wire.encaps.indirection_tbl",	"Indirection table", 	"Indirection table"   		),
    ind_tbl_count = field_protos.string ("wire.encaps.ind_tbl.count",   "count"                                             ),

    encaps_object = field_protos.string ("wire.encaps.object",          "object",               "Polymorphic object"        ),
    encaps_objects= field_protos.string ("wire.encaps.objects",         "Objects",              "Polymorphic objects"       ),

    segment       = field_protos.string ("wire.segment",                "Segment"                                           ),
    segment_type  = field_protos.enum   ("wire.segment.type",           "Type",                 segment_type_id_valstr, 0x03),
    segment_last  = field_protos.bool   ("wire.segment.last",           "Last segment",         0x04                        ),
    segment_t_id  = field_protos.string ("wire.segment.type_id",        "Type ID"                                           ),

    func_params   = field_protos.string ("wire.request.params", 		"Params", 				"Wire reqest params"		),

    seq_element   = field_protos.string ("wire.sequence.element",       " "                                                 ),
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
	local byte = range64(tvbuf, offset, 1):uint()
	return 1, byte ~= 0
end,

--------------------------------------------------------------------------------
--  Read a variable-length uint from buffer
--  Returns consumed length and the int value
--  If the length is 0 means read failure
read_uint = function(tvbuf, offset, max_bytes)
    local msglen = tvbuf:len() - offset

    local val = UInt64(0)
    local n = 0
    local more = true
    while more and (n * 7 <= max_bytes * 8) and msglen > 0 do
        local byte = range64(tvbuf, offset + n, 1):uint()
        val = val:bor( UInt64(bit.band(byte, 0x7f)):lshift(7 * n) )
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

dissect_uint_field = function ( t, tvbuf, offset, proto, tree, max_bytes )
    local n, v = wire.encoding.read_uint_field( tvbuf, offset, max_bytes)
    return t:add_to_tree( tree, proto, tvbuf, n, v )
end,

--------------------------------------------------------------------------------
read_int = function ( tvbuf, offset, max_bytes )
    local n, uint = wire.encoding.read_uint(tvbuf, offset, max_bytes)
    if n <= 0 then
        return 0
    end
    
    local shift_bits = 63
    local v = Int64(uint:arshift(1)):bxor(Int64(uint):lshift(shift_bits):arshift(shift_bits))
    --dprint2("ZigZag uint", tostring(uint), "to int", tostring(v))
    return n, v
end,

read_int_field = function(tvbuf, offset, max_bytes)
    local n, val = wire.encoding.read_int(tvbuf, offset, max_bytes)
    if n <= 0 then
        return 0
    end
    return n, { value = val, offset = offset, size = n }
end,

dissect_int_field = function ( t, tvbuf, offset, proto, tree, max_bytes )
    local n, v = wire.encoding.read_int_field( tvbuf, offset, max_bytes)
    return t:add_to_tree( tree, proto, tvbuf, n, v )
end,

--------------------------------------------------------------------------------
read_float      = function ( tvbuf, offset, size )
    local val = range64(tvbuf,offset, size):le_float()
    return size, val
end,

read_float_field = function ( tvbuf, offset, size )
    local n, val = wire.encoding.read_float(tvbuf, offset, size)
    if n <= 0 then
        return 0
    end
    return n, { value = tostring(val), offset = offset, size = n }
end,

dissect_float_field = function ( t, tvbuf, offset, proto, tree, size )
    local n, v = wire.encoding.read_float_field( tvbuf, offset, size)
    return t:add_to_tree( tree, proto, tvbuf, n, v )
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
                value     = range64(tvbuf, offset + n, sz):string()
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
        local byte = range64(tvbuf, offset + i, 1):uint()
        uuid_str = uuid_str .. string.format("%02x", byte)
    end
    return 16, uuid_str
end,

read_uuid_field = function(tvbuf, offset)
	local n, value = wire.encoding.read_uuid(tvbuf, offset)

	return n, {
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
    local type = range64(tvbuf, (offset + consumed):tonumber(), 1):uint()
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
    op.type = range64(tvbuf, offset, 1):uint()
    local consumed = 1
    if op.type == 0 then -- Op hash
        local hash = range64(tvbuf, offset + consumed, 4):le_uint()
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
    local n, val = wire.encoding.read_uint(tvbuf, offset,4)
    if n == 0 then
        dprint("Failed to read protocol major")
        return 0
    end
    version.major = val
    consumed = consumed + n
    offset = offset + consumed
    n, val = wire.encoding.read_uint(tvbuf, offset,4)
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

read_object    = function ( encaps, offset, proto, tree, fn )
    -- read object id and add a function to encaps indirection table
    local n, id = wire.encoding.read_int(encaps.tvbuf, offset, 8)

    local inf = {
            offset = offset,
            size   = n,
    }
    if id == Int64(0) then
        inf.value = "null object"
    elseif id < Int64(0) then
        inf.value = "object #" .. tostring(-id) .. " (see objects tree)"
        -- TODO Add dissector for reading indirection table
        encaps:add_object_reference(-id, fn)
    end
    local item = add_tree_field(tree, proto, encaps.tvbuf, inf)
    return n, inf, item
end,

dissect_exception = function ( encaps, offset, tree )
    dprint2("Dissect exception")

    local ex_tree = tree:add(wire.hdr_fields.reply_exc, range64(encaps.tvbuf, offset, 0))

    local n = encaps:dissect_object(offset, ex_tree)

    ex_tree:set_len(UInt64(n):tonumber())
    return n
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
    local msglen = UInt64(tvbuf:len() - offset)

    -- check if capture was only capturing partial packet size
    if msglen ~= UInt64(tvbuf:reported_length_remaining(UInt64(offset):tonumber())) then
        -- captured packets are being sliced/cut-off, so don't try to desegment/reassemble
        dprint("Captured packet was shorter than original, can't reassemble")
        return 0
    end

    if msglen < UInt64(wire.encoding.WIRE_MIN_HEADER_SIZE) then
        -- we need more bytes, so tell the main dissector function that we
        -- didn't dissect anything, and we need an unknown number of more
        -- bytes (which is what "DESEGMENT_ONE_MORE_SEGMENT" is used for)
        dprint("Need more bytes to figure out WIRE header")
        -- return as a negative number
        return -DESEGMENT_ONE_MORE_SEGMENT
    end

    -- if we got here, then we know we have enough bytes in the Tvb buffer
    -- to check the magic number and flags.
    local magic_tvbr = range64(tvbuf, offset, 4)
    local magic_val  = magic_tvbr:string()

    if magic_val ~= "wire" then
        dprint("Invalid magic number")
        return 0
    end
    local start = offset
    offset = offset + 4

    local flags_tvbr = range64(tvbuf, offset, 1)
    offset = offset + 1
    local flags_val  = flags_tvbr:uint()

    local msg_header = {}
    msg_header.type = bit32.band(flags_val, 0x7)
    dprint2("Message flags value " .. flags_val .. " type " .. msg_header.type)
    if (bit32.band(flags_val, 0x8) > 0) then
        dprint2("Read protocol version")
        local consumed, tmp = wire.encoding.read_version(tvbuf, offset)
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
        local consumed, tmp = wire.encoding.read_version(tvbuf, offset)
        if consumed <= 0 then
            dprint("Failed to read encoding version")
            return -DESEGMENT_ONE_MORE_SEGMENT
        end
        msg_header.encoding_version = tmp
        offset = offset + consumed
        dprint2("Encoding version " .. msg_header.encoding_version.value)
    end
    local consumed, msg_size = wire.encoding.read_uint(tvbuf, offset,8)
    if (consumed <= 0) then
        return -DESEGMENT_ONE_MORE_SEGMENT
    end
    msg_header.size_offset = offset
    msg_header.size_length = consumed
    offset = offset + consumed
    dprint2("Message size " .. msg_size)

    msglen = UInt64(tvbuf:len() - offset)
    if msglen < msg_size then
        dprint2("Need more bytes to desegment wire message")
        return -Int64(msg_size - msglen):tonumber()
    end

    msg_header.size = msg_size
    msg_header.hdr_size = offset - start
    return offset - start, msg_header
end,

--------------------------------------------------------------------------------
dissect_encapsulation = function ( tvbuf, pktinfo, offset, tree, parse_fn )
    local encaps = Encapsulation:new{ tvbuf = tvbuf, offset = offset, pktinfo = pktinfo }
    return encaps:dissect(tree, parse_fn)
end,

--------------------------------------------------------------------------------
dissect_request = function(tvbuf, pktinfo, root, offset)
    local inf = ""
    local req = {}
    dprint2("Dissect request header")
    local consumed, req_no = wire.encoding.read_uint(tvbuf, offset,8)
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

    local mode = range64(tvbuf, offset + consumed, 1):uint()
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

    local req_tree = root:add(wire.hdr_fields.request, range64(tvbuf, offset, consumed), inf)
    req_tree:add(wire.hdr_fields.request_no, range64(tvbuf, offset, num_sz), req_no:tonumber())
    req_tree:add(wire.hdr_fields.request_tgt, range64(tvbuf, identity.offset, identity.size),
            identity.value)
    req_tree:add(wire.hdr_fields.request_fct, range64(tvbuf, facet.offset, facet.size),
            facet.value)
    req_tree:add(wire.hdr_fields.request_op, range64(tvbuf, op.offset, op.size),
            op.value)
    req_tree:add(wire.hdr_fields.request_mode, range64(tvbuf, op.offset + op.size, 1))
    req_tree:add(wire.hdr_fields.request_multi, range64(tvbuf, op.offset + op.size, 1))
    req_tree:add(wire.hdr_fields.request_noctx, range64(tvbuf, op.offset + op.size, 1))
    req_tree:add(wire.hdr_fields.request_nobody, range64(tvbuf, op.offset + op.size, 1))

    if (multitarget == true) then
        dprint2("Targets offset " .. targets.offset .. " size " .. targets.size .. " count " .. #targets)
        local mt_tree = req_tree:add(hdr_fields.request_mt, range64(tvbuf, targets.offset, targets.size), "Count " .. #targets)
        for i, tgt in ipairs(targets) do
            local inf = tgt.identity.value
            if tgt.facet.value ~= nil then
                inf = inf .. " [" .. tgt.facet.value .. "]"
            end
            dprint2("Target " .. inf)
            mt_tree:add(wire.hdr_fields.request_tgt, range64(tvbuf, tgt.offset, tgt.size),
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
    local consumed, req_no = wire.encoding.read_uint(tvbuf, offset,8)
    if consumed == 0 then
        dprint("Failed to read request number")
        return 0
    end
    local num_sz = consumed
    local status = range64(tvbuf, offset + consumed, 1):uint()

    local inf = " #" .. req_no .. " " .. wire.encoding.repstatus_valstr[status]
    pktinfo.cols.info:append(" Reply" .. inf)
    root:append_text(" Reply" .. inf)

    consumed = consumed + 1 -- message status
    local rep_tree = root:add(wire.hdr_fields.reply, range64(tvbuf, offset, consumed), inf)
    rep_tree:add(wire.hdr_fields.reply_no, range64(tvbuf, offset, num_sz), req_no:tonumber())
    rep_tree:add(wire.hdr_fields.reply_status, range64(tvbuf, offset + num_sz, 1))

    if status ~= repstatus.success_no_body then
        local fn = nil
        if status == repstatus.user_exception or status >= repstatus.unknown_wire_exception then
            dprint2("Reply contains an exception in the encapsulation")
            fn = wire.encoding.dissect_exception
        end

        local n = wire.protocol.dissect_encapsulation(tvbuf, pktinfo, offset + consumed, rep_tree, fn)
        if n <= 0 then
        	return 0
        end
        consumed = consumed + n
    end

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

--  Parser dictionary
parsers 		= {
	string 		= Type:new {
		dissect = function ( self, encaps, offset, proto, tree )
			-- TODO Add the tree field
			local n, v = wire.encoding.read_string_field(encaps.tvbuf, offset)
            return self:add_to_tree( tree, proto, encaps.tvbuf, n, v )
		end,
	},
	bool 		= Type:new {
		dissect = function ( self, encaps, offset, proto, tree )
			local n, v = wire.encoding.read_bool( encaps.tvbuf, offset, proto, tree )
            if n > 0 then
                local val = self:new_value{ offset = offset, size = n, value = n }
                local item = proto and tree:add(proto, range64(encaps.tvbuf, offset, n)) or nil
                if item ~= nil then
                    item:append_text(tostring(val))
                end
                return n, val, item
            end
            return n, nil, nil
		end,
        format  = function ( val )
            return val.value and "true" or "false"
        end
	},
    int16       = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_int_field( self, encaps.tvbuf, offset, proto, tree, 2)
        end,
    },
    int32       = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_int_field( self, encaps.tvbuf, offset, proto, tree, 4)
        end,
    },
    int64       = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_int_field( self, encaps.tvbuf, offset, proto, tree, 8)
        end,
    },
    uint16      = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_uint_field( self, encaps.tvbuf, offset, proto, tree, 2)
        end,
    },
    uint32      = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_uint_field( self, encaps.tvbuf, offset, proto, tree, 4)
        end,
    },
    uint64      = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_uint_field( self, encaps.tvbuf, offset, proto, tree, 8)
        end,
    },
    float       = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_float_field( self, encaps.tvbuf, offset, proto, tree, 4 )
        end
    },
    double      = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_float_field( self, encaps.tvbuf, offset, proto, tree, 8 )
        end
    },
    uuid        = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            local n, v = wire.encoding.read_uuid_field(encaps.tvbuf, offset)
            return self:add_to_tree( tree, proto, encaps.tvbuf, n, v )
        end,
    },
    time_point  = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_int_field( self, encaps.tvbuf, offset, proto, tree, 8)
        end,
    },
    duration    = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            return wire.encoding.dissect_int_field( self, encaps.tvbuf, offset, proto, tree, 8)
        end,
    },
    proxy       = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            dprint2("Dissect proxy")
            local n, is_there = wire.encoding.read_bool(encaps.tvbuf, offset)
            if n > 0 then
                if is_there then
                    local l = wire.types.type("::wire::core::reference_data")
                    local m, data, item = l():dissect(encaps, offset + n, proto, tree)
                    return m + n, data, item
                else
                    local inf = {
                        offset = offset,
                        size   = n,
                        value  = "Null proxy"
                    }
                    local item = proto ~= nil and add_tree_field(tree, proto, encaps.tvbuf, inf) or nil
                    return n, nil, item
                end
            end
            return n
        end
    },
    wildcard    = Type:new {
        dissect = function ( self, encaps, offset, proto, tree )
            local  inf = self.new_value {
                offset = offset,
                size   = 1,
                value  = "~any~",
            }
            local item = proto ~= nil and add_tree_field(tree, proto, encaps.tvbuf, inf) or nil
            return 1, inf, item
        end
    },
},

--  Dictionary of functions by hashes
functions   = {},

aliases     = {},

--  Return a pair of lambdas - parser and field creator
type 			= function ( name )
	return function() 
        local t = wire.types.parsers[name]
        if t == nil then
            dprint2("Type", name, "not found")
            -- Check aliases
            if wire.types.aliases[name] ~= nil then
                local aliased = wire.types.aliases[name]
                t = Type:new{
                    name        = name,
                    dissect     = aliased().dissect,
                    ind_dissect = aliased().ind_dissect,
                    format      = aliased().format,
                }
                wire.types.parsers[name] = t
            end
        end
        return t
    end
end,

--  Sequence type
sequence 		= function ( element_type )
    local seq_name = "wire.__sequence_of__." .. element_type().name
    if wire.types.parsers[seq_name] == nil then
        local abbrev = make_abbrev(seq_name)
        local elem_proto = field_protos.string(abbrev, " ")
        wire.hdr_fields[abbrev] = elem_proto
        local name = "sequence< " .. element_type().name .. " >"
        wire.types.parsers[seq_name] = Type:new {
            name = name,
            dissect = function ( self, encaps, offset, proto, tree )
                local consumed = 0
                local n, sz = wire.encoding.read_uint(encaps.tvbuf, offset, 8)
                if n <= 0 then
                    dprint("Failed to read", name, "size")
                    return 0
                end
                consumed = consumed + n
                local seq_tree = tree:add(proto, range64(encaps.tvbuf, offset, 0))
                seq_tree:append_text(name .. ": " .. tostring(sz) .. " element(s)")
                dprint2("Dissect", name, "of size", tostring(sz))
                for i = 0, (sz:tonumber() - 1) do
                    local n, val, item = element_type():dissect(encaps, offset + consumed, elem_proto, seq_tree)
                    if n <= 0 then
                        dprint("Failed to read sequence element")
                        return 0
                    end
                    consumed = consumed + n
                    item:prepend_text("#" .. i)
                end
                seq_tree:append_text(" " .. tostring(consumed) .. " byte(s)")
                seq_tree:set_len(UInt64(consumed):tonumber())
                return consumed, nil, seq_tree
            end,
        }
    end
    return wire.types.type(seq_name)
end,

array           = function ( element_type, size )
    local seq_name = "wire.__array_" .. size .. "_of__." .. element_type().name

    if wire.types.parsers[seq_name] == nil then
        local abbrev = make_abbrev(seq_name)
        local elem_proto = field_protos.string(abbrev, "element")
        wire.hdr_fields[abbrev] = elem_proto
        name = "array< " .. element_type().name .. ", " .. size .. " >"
        wire.types.parsers[seq_name] = Type:new {
            name = name,
            dissect = function ( self, encaps, offset, proto, tree )
                local consumed = 0
                local seq_tree = tree:add(proto, range64(encaps.tvbuf, offset, 0))
                seq_tree:append_text(name .. ": " .. tostring(sz) .. " element(s)")
                dprint2("Dissect", name, "of size", tostring(size))
                for i=1, size do
                    local n, val = element_type():dissect(encaps, offset + consumed, elem_proto, seq_tree)
                    if n <= 0 then
                        dprint("Failed to read sequence element")
                        return 0
                    end
                    consumed = consumed + n
                end
                seq_tree:append_text(" " .. tostring(consumed) .. " byte(s)")
                seq_tree:set_len(UInt64(consumed):tonumber())
                return consumed, nil, seq_tree
            end,
        }
    end
    return wire.types.type(seq_name)
end,

--  Dictionary
dictionary 		= function ( key_type, element_type )
    local dict_name = make_abbrev("wire.__dict_of__." .. key_type().name .. "._to_." .. element_type().name)
    if wire.types.parsers[dict_name] == nil then
        local key_abbrev = dict_name .. ".key"
        local val_abbrev = dict_name .. ".value"
        local key_proto = field_protos.string(key_abbrev, "key")
        local val_proto = field_protos.string(val_abbrev, "value")
        wire.hdr_fields[key_abbrev] = key_proto
        wire.hdr_fields[val_abbrev] = val_proto
        local name = "dictionary< " .. key_type().name .. ", " .. element_type().name .. " >"

        wire.types.parsers[dict_name] = Type:new {
            name = name,
            dissect = function ( self, encaps, offset, proto, tree )
                local consumed = 0
                local n, sz = wire.encoding.read_uint(encaps.tvbuf, offset, 8)
                if n <= 0 then
                    dprint("Failed to read", name, "size")
                    return 0
                end
                consumed = consumed + n
                local seq_tree = tree:add(proto, range64(encaps.tvbuf, offset, 0))
                seq_tree:append_text(name .. ": " .. tostring(sz) .. " element(s)")
                local kf = key_type()
                local vf = element_type()
                dprint2("Dissect", name, "of size", tostring(sz))
                for i=1, sz:tonumber() do
                    local n, key = kf:dissect(encaps, offset + consumed, key_proto, seq_tree)
                    if n <= 0 then
                        dprint("Failed to read", name, "key")
                        return 0
                    end
                    consumed = consumed + n
                    local m, val = vf:dissect(encaps, offset + consumed, val_proto, seq_tree)
                    if m <= 0 then
                        dprint("Failed to read", name, "value")
                        return 0
                    end
                    consumed = consumed + m
                end
                seq_tree:append_text(" " .. tostring(consumed) .. " byte(s)")
                seq_tree:set_len(UInt64(consumed):tonumber())
                return consumed, nil, seq_tree
            end,
        }
    end
    return wire.types.type(dict_name)
end,

-- Optional
optional 		= function ( element_type )
    local abbrev = make_abbrev("wire.__optional__." .. element_type().name)
    if wire.types.parsers[abbrev] == nil then
        local name = "optional< " .. element_type().name .. " >"
        local t = Type:new {
            name    = name,
            dissect = function ( self, encaps, offset, proto, tree )
                local consumed = 0
                local n, b = wire.encoding.read_bool(encaps.tvbuf, offset)
                if n <= 0 then
                    return 0
                end
                dprint2("Dissect optional value")
                consumed = consumed + n
                if b then
                    dprint2("Optional value is present")
                    local n, val, item = element_type():dissect(encaps, offset + consumed, proto, tree)
                    if n <= 0 then
                        return 0
                    end
                    consumed = consumed + n
                    return consumed, val, item
                else
                    dprint2("Optional value is not present")
                    local val = {
                        offset = offset,
                        size   = consumed,
                        value  = "optional value not present"
                    }
                    local item = add_tree_field(tree, proto, encaps.tvbuf, val)
                    return consumed, val, item
                end
                return consumed, nil, nil
            end,
        }
        wire.types.parsers[abbrev] = t
    end
    return wire.types.type(abbrev)
end,

-- Variant
-- Accepts a list of dissectors for a type number and an optional type number to string map
variant 		= function ( variants, var_map )
    local name = "wire.__variant_of__."
    for i, t in ipairs(variants) do
        name = name .. t().name
    end

    if wire.types.parsers[name] == nil then

        dprint2("Create variant type with", #variants, "types")
        local name_variant = false
        if var_map ~= nil and var_map.enumerators ~= nil then
            name_variant = true
        end
        wire.types.parsers[name] = Type:new {
            name = name,
            dissect = function ( self, encaps, offset, proto, tree )
                local consumed = 0
                local n, v = wire.encoding.read_uint(encaps.tvbuf, offset, 8)
                if n <= 0 then
                    dprint("Failed to read varant type index")
                    return 0
                end
                local tno = v:tonumber()
                if tno >= #variants then
                    dprint("Type index", tno, "is invalid for variant value, max is ", #variants)
                    return 0
                end

                local val_tree = tree
                consumed = consumed + n
                n, val, item = variants[tno + 1]():dissect(encaps, offset + consumed, proto, val_tree)
                if n <= 0 then
                    return 0
                end
                if item ~= nil and name_variant then
                    item:prepend_text(var_map.enumerators[tno])
                end

                return consumed + n, val, val_tree
            end,
        }
    end
    return wire.types.type(name)
end,

alias           = function ( name, t )
    dprint2("Add alias", name)
    -- Defer dereferencing the type
    wire.types.aliases[name] = t
    return wire.types.type(name)
end,

-- Enumeration
enum            = function ( name, definition )
    dprint2("Add enum", name, "definition")
    local valstr = makeValString(definition)
    local e = Type:new {
        name = name,
        enumerators  = valstr,
        dissect = function ( self, encaps, offset, proto, tree )
            dprint2("Dissect enum", name)
            local n, v = wire.encoding.read_uint(encaps.tvbuf, offset, 8)
            if n > 0 then
                local val = self:new_value {
                    offset = offset,
                    size   = n,
                    value  = self.enumerators[v:tonumber()] or "<UNKNOWN " .. name .. " VALUE " .. tostring(v) .. ">",
                }
                local item = proto ~= nil and add_tree_field(tree, proto, encaps.tvbuf, val) or nil
                return n, val, item
            end
            return n, nil, nil
        end,
        format = function ( val )
            if val.value == nil then
                dprint("Enum", name, "value is nil at format")
            else
                dprint("Enum", name, "value", val.value, "at format")
            end
            return tostring(val.value)
        end
    }
    wire.types.parsers[name] = e
    return e
end,

-- Create a field/function param parser and proto
field           = function ( owner_name, definition )
    local fname, abbrev     = qname_abbrev(owner_name, definition.name)

    --dprint2("Make field", definition.name, "at", owner_name, "abbrev", abbrev)
    local proto = field_protos.string(abbrev, definition.name)
    wire.hdr_fields[ fname ] = proto
    return {
        name    = definition.name,
        type    = definition.type, 
        proto   = proto,
    }
end,

fields          = function ( owner_name, definition )
    local fields = {}
    if definition ~= nil then
        for i, field in ipairs(definition) do
            table.insert(fields, wire.types.field( owner_name, field ))
        end
    end
    local parse_fn = function ( encaps, offset, tree, data )
        local consumed = 0
        for i, f in ipairs(fields) do
            dprint2("Dissect field", f.name, ":", f.type().name)
            local n, v = f.type():dissect(encaps, offset + consumed, f.proto, tree)
            if n <= 0 then
                dprint2("Failed to dissect field", f.name)
                return n, nil
            end
            if v == nil then
                dprint2("Data for field", f.name, "is nil")
            else
                dprint2(f.name, "=", v)
            end
            if data ~= nil then
                data[f.name] = v
            end
            consumed = consumed + n
        end
        return consumed, data
    end
    return parse_fn
end,

func            = function ( owner_name, name, definition )
    local fname, abbrev     = qname_abbrev(owner_name, name)

    dprint2("Make function", name, "at", owner_name, "abbrev", abbrev, "hash", definition.hash)
    local proto = field_protos.string(abbrev, fname .. " params")
    wire.hdr_fields[ fname ] = proto

    -- Create param protos
    local params = wire.types.fields(fname, definition.params)

    -- Function dissector is called directly from encaps dissector
    local dissector = function ( encaps, offset, tree )
        dprint2("Parse function", fname, "params")
        local param_tree = tree:add(proto, range64(encaps.tvbuf, offset, 0))
        local consumed = params(encaps, offset, param_tree)
        param_tree:append_text(" " .. consumed .. " byte(s)")
        param_tree:set_len(Int64(consumed):tonumber())
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
        local fields = wire.types.fields(name, definition.fields)
		-- Create dissector for fields
		local dissector = function ( self, encaps, offset, proto, tree )
            dprint2("Dissect structure", name)
            local data     = self:new_value{}
            -- Add field with type name
			local struct_tree = tree:add(proto, range64(encaps.tvbuf, offset, 0))
            -- Dissect structure fields
            local consumed = fields(encaps, offset, struct_tree, data)
            struct_tree:append_text(tostring(data))
            struct_tree:set_len(Int64(consumed):tonumber())
            dprint2("Dissected structure", data)
			return consumed, data, struct_tree
		end

        wire.types.parsers[name] = Type:new {
            name    = name,
            dissect = dissector,
            format  = make_formatter(definition.format)
        }
	end

   -- Find type by name
    return wire.types.type(name)
end,

interface 		= function ( name, definition )
    dprint2("Add interface", name, "definition")
	-- create parsers for functions
    if definition.functions ~= nil then
        for n, func in pairs(definition.functions) do
            wire.types.func(name, n, func)
        end
    end
	return {}
end,

class 			= function ( name, definition )
    if definition ~= nil then
        wire.types.interface( name, definition )
        dprint2("Add class", name, "definition")
        
        local fields = wire.types.fields(name, definition.fields)
        local data_dissector = function ( encaps, offset, tree, first_segment )
            local data     = {}
            if first_segment then
                -- Add type name to field
                tree:append_text(name)
            end
            -- Dissect structure fields
            local consumed = fields(encaps, offset, tree, data)
            -- TODO return parent
            return consumed, data
        end
        local dissector = function ( self, encaps, offset, proto, tree )
            return wire.encoding.read_object( encaps, offset, proto, tree, data_dissector )
        end
        local p = Type:new {
            name        = name,
            dissect     = dissector,
            ind_dissect = data_dissector,
        }
        wire.types.parsers[name] = p
        wire.types.parsers[definition.hash] = p
    end

    return wire.types.type(name)
end,

exception       = function ( name, definition )
    if definition ~= nil then
        dprint2("Add exception", name, "definition")
        local parent = definition.parent
        -- Create field protos
        local fields = wire.types.fields(name, definition.fields)
        local data_dissector = function ( encaps, offset, tree, first_segment )
            local data     = {}
            if first_segment then
                -- Add type name to field
                tree:append_text(name)
            end
            -- Dissect structure fields
            local consumed = fields(encaps, offset, tree, data)

            return consumed, data, parent ~= nil
        end
        local dissector = function ( self, encaps, offset, proto, tree )
            local data     = {}
            -- Add type name to field
            tree:append_text(name)
            -- Dissect structure fields
            local consumed = fields(encaps, offset, tree, data)

            return consumed, data, parent ~= nil
        end

        local e = Type:new {
            name        = name,
            dissect     = dissector,
            ind_dissect = data_dissector,
        }
        wire.types.parsers[name] = e
        wire.types.parsers[definition.hash] = e
    end
    return wire.types.type(name)
end,

}, -- types

}

function Version:read( tvbuf, offset )
    self.offset = offset
    local consumed = 0
    local n, v = wire.encoding.read_uint(tvbuf, offset, 4)
    if n == 0 then
        dprint2("Failed to read version major")
        return 0
    end
    self.major = v
    consumed = consumed + n

    n, v = wire.encoding.read_uint(tvbuf, offset + n, 4)
    if n == 0 then
        dprint2("Failed to read version minor")
        return 0
    end
    self.minor = v
    consumed = consumed + n

    self.size = consumed
    self.value = self.major .. "." .. self.minor
    dprint2("Version from wire " .. self.value)

    return consumed, self
end

function Encapsulation:dissect( tree, fn )
    dprint2("Dissect encapsulation")
    local n = self.version:read(self.tvbuf, self.offset)

    if n <= 0 then
        dprint("Failed to read encapsulation version")
        return 0
    end

    n, self.size = wire.encoding.read_uint_field(self.tvbuf, self.offset + self.version.size, 8)
    if n <= 0 then
        dprint2("Failed to read encapsulation size")
        return 0
    end
    local consumed = self.version.size + n
    dprint2("Encapsulation version", self.version.value, "size", self.size.value)

    local inf = "version " .. self.version.value .. " size " .. self.size.value
    -- TODO Mark as expert
    self.tree = tree:add(wire.hdr_fields.encaps, range64(self.tvbuf, self.offset, consumed + self.size.value), inf)
    add_tree_field(self.tree, wire.hdr_fields.encaps_version, self.tvbuf, self.version)
    add_tree_field(self.tree, wire.hdr_fields.msg_size, self.tvbuf, self.size)

    if self.size.value > 0 then
        if fn ~= nil then
            local n = fn(self, self.offset + consumed, tree)
            if n < self.size.value then
                self:read_indirection_table(self.offset + consumed + n, n, tree)
            end
        else
            local tvb = tvb64(self.tvbuf, self.offset + consumed, self.size.value)
            data:call(tvb, self.pktinfo, self.tree)
        end
    end

    consumed = consumed + self.size.value

    return consumed
end

function Encapsulation:add_object_reference(obj_id, f)
    local str_id = tostring(obj_id)
    if self.indirection[str_id] == nil then
        dprint2("Add object indirection", tostring(obj_id))
        self.indirection[str_id] = {

        }
    end

end

function Encapsulation:read_indirection_table(offset, size, tree)
    local consumed, count = wire.encoding.read_uint(self.tvbuf, offset, 8)

    local ind_table_sz = UInt64(self.size.value - size - consumed)
    dprint2("Encapsulation indirection table size", ind_table_sz:tonumber(), "bytes", count:tonumber(), "items")
    local inf = "size " .. tostring(ind_table_sz)
    self.ind_tree = self.tree:add(wire.hdr_fields.encaps_ind_tbl, range64(self.tvbuf, offset, ind_table_sz), inf)

    inf = {
        offset = offset,
        size   = consumed,
        value  = tostring(count)
    }
    add_tree_field(self.ind_tree, wire.hdr_fields.ind_tbl_count, self.tvbuf, inf)
    if count > UInt64(0) then
        local objects = tree:add(wire.hdr_fields.encaps_objects, range64(self.tvbuf, offset + consumed, 0))
        while (count > UInt64(0)) do

            while count > UInt64(0) do
                local n, id = wire.encoding.read_int(self.tvbuf, offset + consumed, 8)
                local str_id = tostring(id)
                dprint2("Object id", str_id)
                consumed = consumed + n

                if self.indirection[str_id] == nil then
                    dprint("Unexpected object id", str_id)
                    break
                end
                local obj = objects:add(wire.hdr_fields.encaps_object, range64(self.tvbuf, offset + consumed, 0))
                obj:set_text("#" .. str_id .. ": ")
                local obj_sz = self:dissect_object(offset + consumed, obj)
                if obj_sz <= 0 then
                    return 0
                end
                obj:set_len(UInt64(obj_sz):tonumber())

                consumed = consumed + obj_sz
                count = count - 1
                dprint2("Objects remain", tostring(count))
            end

            local n, cnt = wire.encoding.read_uint(self.tvbuf, offset + consumed, 8)
            local inf = {
                offset = offset + consumed,
                size   = n,
                value  = tostring(cnt)
            }
            dprint2("Next table size", tostring(cnt))
            add_tree_field(self.ind_tree, wire.hdr_fields.ind_tbl_count, self.tvbuf, inf)

            consumed = consumed + n
            count = cnt
        end
    end

    if Int64(ind_table_sz) - consumed > Int64(0) then
        local tvb = tvb64(self.tvbuf, offset + consumed, ind_table_sz - consumed)
        data:call(tvb, self.pktinfo, self.ind_tree)
    end

    return consumed
end

function Encapsulation:dissect_object( offset, tree )
    dprint2("Dissect object data")
    -- TODO Add the tree item here, so that polymorphic parsers only add fields
    local consumed, last = self:dissect_segment(offset, tree, true, true, nil)
    while not last do
        local n = 0
        n, last = self:dissect_segment(offset + consumed, tree, true, false, nil)
        if n <= 0 then
            return 0
        end
        consumed = consumed + n
    end
    return consumed
end

function Encapsulation:read_segment_header( offset )
    local consumed = 0
    local n, f = wire.encoding.read_uint(self.tvbuf, offset, 8)
    if n <= 0 then
        dprint2("Failed to read segment header flags")
        return 0
    end
    consumed = consumed + n

    local hdr = {
        flags   = f,
        type_id = nil,
        size    = nil,
        type    = nil,
        last    = f:band(0x04) > 0
    }
    dprint2("Segment header flags", hdr.flags:tonumber())

    if hdr.flags:band(0x01) > 0 then
        dprint2("Type id is string", hdr.flags:band(0x01):tonumber())
        local n, type_id = wire.encoding.read_string_field(self.tvbuf, consumed)
        if n <= 0 then
            dprint2("Failed to read segment type id string")
            return 0
        end
        hdr.type_id = type_id
        consumed = consumed + n

        self.types[ #self.types + 1 ] = hdr.type_id.value
    elseif hdr.flags:band(0x02) > 0 then
        dprint2("Type id is hash")
        local n, hash = wire.encoding.read_uint(self.tvbuf, offset + consumed, 8)
        if n <= 0 then
            dprint2("Failed to read segment type id hash")
            return 0
        end
        hdr.type_id = {
            size   = n,
            offset = offset + consumed,
            value  = "0x" .. hash:tohex()
        }
        consumed = consumed + n

        self.types[ #self.types + 1 ] = hdr.type_id.value
    else
        dprint2("Type id is number")
        local n, num = wire.encoding.read_uint(self.tvbuf, offset + consumed, 8)
        if n <= 0 then
            dprint2("Failed to read segment type id number")
            return 0
        end

        if num <= 0 or self.types[ num:tonumber() ] == nil then
            dprint2("Type number", num:tonumber(), "is invalid")
        end

        hdr.type_id = {
            size   = n,
            offset = offset + consumed,
            value  = self.types[ num:tonumber() ]
        }
        consumed = consumed + n
    end

    local sz
    n, sz = wire.encoding.read_uint_field(self.tvbuf, offset + consumed, 8)
    if n <= 0 then
        dprint2("Failed to read segment size")
        return 0
    end
    hdr.size = sz
    consumed = consumed + n

    hdr.type = wire.types.parsers[hdr.type_id.value]

    return consumed, hdr
end

function Encapsulation:dissect_segment( offset, tree, read_head, first_segment, fn )
    local consumed = 0
    local last = false
    if read_head then
        local n, hdr = self:read_segment_header(offset)
        if n <= 0 then
            dprint("Failed to read segment header")
            return 0
        end

        local inf = ""
        if hdr.type == nil then
            inf = "Unknown type"
        else
            inf = hdr.type.name
        end

        last = hdr.last

        -- TODO Mark as expert
        local parent = self.ind_tree
        if parent == nil then
            parent = self.tree
        end
        local seg_tree = parent:add(wire.hdr_fields.segment, range64(self.tvbuf, offset, n + hdr.size.value), inf)
        seg_tree:add(wire.hdr_fields.segment_type, range64(self.tvbuf, offset, 1))
        seg_tree:add(wire.hdr_fields.segment_last, range64(self.tvbuf, offset, 1))

        add_tree_field(seg_tree, wire.hdr_fields.segment_t_id, self.tvbuf, hdr.type_id)
        add_tree_field(seg_tree, wire.hdr_fields.msg_size, self.tvbuf, hdr.size)

        consumed = consumed + n

        if fn == nil and hdr.type ~= nil then
            fn = hdr.type.ind_dissect
        end
    end
    if fn ~= nil then
        local n, data, parent = fn(self, offset + consumed, tree, first_segment)
        if n <= 0 then
            return 0
        end
        consumed = consumed + n
    end
    return consumed, last
end



local function name_predefined_types()
    for n,t in pairs(wire.types.parsers) do
        dprint2("Predefined type", n)
        --if t.name == nil then
            t.name = n
        --end
        if t.format == nil then
            t.format = field_to_string
        end
    end
end

name_predefined_types()
