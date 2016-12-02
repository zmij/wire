------------------------------------------------------------------------------
-- author: Sergei Fedorov <sergei.a.fedorov@gmail.com>
-- Copyright (c) 2016, Sergei Fedorov
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
local DEBUG = debug_level.LEVEL_1

-- a table of our default settings - these can be changed by changing
-- the preferences through the GUI or command-line; the Lua-side of that
-- preference handling is at the end of this script file
local default_settings =
{
    debug_level  = DEBUG,
    enabled      = true, -- whether this dissector is enabled or not
    port         = 11333, -- default TCP port number for FPM
    --max_msg_len  = 4096, -- max length of FPM message
}


local dprint = function() end
local dprint2 = function() end
local function resetDebugLevel()
    if default_settings.debug_level > debug_level.DISABLED then
        dprint = function(...)
            info(table.concat({"Lua: ", ...}," "))
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

--------------------------------------------------------------------------------
-- creates a Proto object, but doesn't register it yet
local wire_proto = Proto("wire", "Wire Protocol");

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
local msgtype_valstr = makeValString(flagstype)

local reqmode = {
    normal    = 0x01,
    one_way   = 0x02
}
local reqmode_valstr = makeValString(reqmode)

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
local repstatus_valstr = makeValString(repstatus)

----------------------------------------
-- a table of all of our Protocol's fields
local hdr_fields =
{
    header        = ProtoField.string("wire.header", "Header", "Wire message header"),
    magic_type    = ProtoField.string("wire.magic", "Magic"),
    msg_type      = ProtoField.uint8 ("wire.type", "Type", base.HEX, msgtype_valstr, 0x07),
    proto_flag    = ProtoField.bool("wire.header.proto_flag", "Protocol Flag", base.HEX, nil, 0x08),
    enc_flag      = ProtoField.bool("wire.header.encoding_flag", "Encoding Flag", base.HEX, nil, 0x10),

    proto_version = ProtoField.uint16 ("wire.header.proto_version", "Protocol Version", base.HEX),
    enc_version   = ProtoField.uint16 ("wire.header.enc_version", "Encoding Version", base.HEX),
    msg_size      = ProtoField.uint64("wire.length", "Length", base.HEX),

    request       = ProtoField.string("wire.request", "Request", "Wire request"),
    request_no    = ProtoField.string("wire.request.number", "Number", "Request number"),
    request_tgt   = ProtoField.string("wire.request.target", "Request Target", "Wire request target"),
    request_fct   = ProtoField.string("wire.request.facet", "Request Facet", "Wire request facet"),
    request_op    = ProtoField.string("wire.request.op", "Request Operation", "Wire request operation"),
    request_mode  = ProtoField.uint8("wire.request.mode", "Request Mode", base.HEX, reqmode_valstr, 0x3),
    request_multi = ProtoField.bool("wire.request.multi_target", "Request Multi-Target", base.HEX, nil, 0x4),
    request_noctx = ProtoField.bool("wire.request.no_context", "Request Without Context", base.HEX, nil, 0x10),
    request_nobody= ProtoField.bool("wire.request.no_body", "Request Without Body", base.HEX, nil, 0x20),

    reply         = ProtoField.string("wire.reply", "Reply", "Wire reply"),
    reply_no      = ProtoField.string("wire.reply.number", "Number", "Reply number"),
    reply_status  = ProtoField.uint8("wire.reply.status", "Reply Status", base.HEX, repstatus_valstr),
}

wire_proto.fields = hdr_fields

dprint2("wire_proto ProtoFields registered")

-- due to a bug in older (prior to 1.12) wireshark versions, we need to keep newly created
-- Tvb's for longer than the duration of the dissect function (see bug 10888)
-- this bug only affects dissectors that create new Tvb's, which is not that common
-- but this FPM dissector happens to do it in order to create the fake SLL header
-- to pass on to the Netlink dissector
local tvbs = {}

---------------------------------------
-- This function will be invoked by Wireshark during initialization, such as
-- at program start and loading a new file
function wire_proto.init()
    -- reset the save Tvbs
    tvbs = {}
end

local WIRE_MIN_HEADER_SIZE = 6
local WIRE_MAX_HEADER_SIZE = 18

-- some forward "declarations" of helper functions we use in the dissector
local checkWireHeader, dissectWire, dissectRequest, dissectReply
local read_uint, read_string, read_uuid, read_identity, read_operation

-- this holds the plain "data" Dissector for opaque message contents
local data = Dissector.get("data")

--------------------------------------------------------------------------------
-- The following creates the callback function for the dissector.
-- The 'tvbuf' is a Tvb object, 'pktinfo' is a Pinfo object, and 'root' is a TreeItem object.
-- Whenever Wireshark dissects a packet that our Proto is hooked into, it will call
-- this function and pass it these arguments for the packet it's dissecting.
function wire_proto.dissector(tvbuf, pktinfo, root)
    -- get the length of the packet buffer (Tvb).
    local pktlen = tvbuf:len()
    dprint2("wire_proto.dissector called. pktlen = " .. pktlen)

    local bytes_consumed = 0

    -- we do this in a while loop, because there could be multiple WIRE messages
    -- inside a single TCP segment, and thus in the same tvbuf - but our
    -- wire_proto.dissector() will only be called once per TCP segment, so we
    -- need to do this loop to dissect each WIRE message in it
    while bytes_consumed < pktlen do

        -- We're going to call our "dissect()" function, which is defined
        -- later in this script file. The dissect() function returns the
        -- length of the WIRE message it dissected as a positive number, or if
        -- it's a negative number then it's the number of additional bytes it
        -- needs if the Tvb doesn't have them all. If it returns a 0, it's a
        -- dissection error.
        local result = dissectWire(tvbuf, pktinfo, root, bytes_consumed)

        if result > 0 then
            -- we successfully processed an FPM message, of 'result' length
            bytes_consumed = bytes_consumed + result
            -- go again on another while loop
        elseif result == 0 then
            -- If the result is 0, then it means we hit an error of some kind,
            -- so return 0. Returning 0 tells Wireshark this packet is not for
            -- us, and it will try heuristic dissectors or the plain "data"
            -- one, which is what should happen in this case.
            return 0
        else
            -- we need more bytes, so set the desegment_offset to what we
            -- already consumed, and the desegment_len to how many more
            -- are needed
            pktinfo.desegment_offset = bytes_consumed

            -- invert the negative result so it's a positive number
            result = -result

            pktinfo.desegment_len = result

            -- even though we need more bytes, this packet is for us, so we
            -- tell wireshark all of its bytes are for us by returning the
            -- number of Tvb bytes we "successfully processed", namely the
            -- length of the Tvb
            return pktlen
        end
    end

    -- In a TCP dissector, you can either return nothing, or return the number of
    -- bytes of the tvbuf that belong to this protocol, which is what we do here.
    -- Do NOT return the number 0, or else Wireshark will interpret that to mean
    -- this packet did not belong to your protocol, and will try to dissect it
    -- with other protocol dissectors (such as heuristic ones)
    return bytes_consumed
end

--------------------------------------------------------------------------------
dissectWire = function(tvbuf, pktinfo, root, offset)
    dprint2("Wire dissect function called. offset = " .. offset)

    local consumed, msg_header = checkWireHeader(tvbuf, offset)

    if consumed <= 0 then
      return consumed
    end
    dprint2("Header checked OK")

    -- if we are here, then we have the whole message in the Tvb buffer
    -- and we can at least show some info

    -- set the protocol column to show our protocol name
    pktinfo.cols.protocol:set("WIRE")

    -- set the INFO column too, but only if we haven't already set it before
    -- for this frame/packet, because this function can be called multiple
    -- times per packet/Tvb
    if string.find(tostring(pktinfo.cols.info), "^WIRE") == nil then
        dprint2("Previous info value " .. tostring(pktinfo.cols.info))
        -- TODO Extract ports and add to info
        pktinfo.cols.info:set("WIRE: " .. pktinfo.src_port .. "→" .. pktinfo.dst_port)
    end

    local tree = root:add(wire_proto, tvbuf:range(offset, consumed + msg_header.size))
    local hdr_tree = tree:add(hdr_fields.header, tvbuf:range(offset, consumed),
        "Size: " .. consumed)
    -- magic number
    hdr_tree:add(hdr_fields.magic_type, tvbuf:range(offset, 4))
    -- flags
    hdr_tree:add(hdr_fields.msg_type, tvbuf:range(offset + 4, 1))
    hdr_tree:add(hdr_fields.proto_flag, tvbuf:range(offset + 4, 1))
    hdr_tree:add(hdr_fields.enc_flag, tvbuf:range(offset + 4, 1))

    local data_size = msg_header.size
    dprint2("Tree added, adding info")
    if msg_header.type == 0 then
        local rn = dissectRequest(tvbuf, pktinfo, tree, offset + consumed)
        if rn == 0 then
            dprint("Failed to dissect request")
            return 0
        end
        consumed = consumed + rn
        data_size = data_size - rn
    elseif msg_header.type == 2 then
        local rn = dissectReply(tvbuf, pktinfo, tree, offset + consumed)
        if rn == 0 then
            dprint("Failed to dissect reply")
            return 0
        end
        consumed = consumed + rn
        data_size = data_size - rn
    elseif msg_header.type == 3 then
        pktinfo.cols.info:append(" Validate")
    elseif msg_header.type == 4 then
        pktinfo.cols.info:append(" Close")
    else
        pktinfo.cols.info:append(" Unknown message type " .. msg_header.type)
    end
    if msg_header.proto_version then
        pktinfo.cols.info:append(" protocol " .. msg_header.proto_version.major ..
            "." .. msg_header.proto_version.minor)
        hdr_tree:add(hdr_fields.proto_version,
            tvbuf:range(msg_header.proto_version.offset,
                msg_header.proto_version.size))
    end
    if msg_header.encoding_version then
        pktinfo.cols.info:append(" encoding " .. msg_header.encoding_version.major ..
            "." .. msg_header.encoding_version.minor)
        hdr_tree:add(hdr_fields.enc_version,
            tvbuf:range(msg_header.encoding_version.offset,
                msg_header.encoding_version.size))
    end
    pktinfo.cols.info:append(" size " .. msg_header.size)
    hdr_tree:add(hdr_fields.msg_size,
          tvbuf:range(msg_header.size_offset, msg_header.size_length))

    if data_size > 0 then
        local tvb = tvbuf(offset + consumed, data_size):tvb()
        data:call(tvb, pktinfo, tree)
    end

    return consumed + data_size
end

--------------------------------------------------------------------------------
dissectRequest = function(tvbuf, pktinfo, root, offset)
    local inf = ""
    local req = {}
    dprint2("Dissect request header")
    local consumed, req_no = read_uint(tvbuf,offset,8)
    if consumed == 0 then
        dprint("Failed to read request number")
        return 0
    end
    local num_sz = consumed

    inf = inf .. " #" .. req_no

    local n, identity = read_identity(tvbuf, offset + consumed)
    if n == 0 then
        dprint("Failed to read request target")
        return 0
    end
    dprint2("Request target: " .. identity.value)
    consumed = consumed + n

    inf = inf .. " " .. identity.value

    -- read facet
    local n, facet = read_string(tvbuf, offset + consumed)
    if n == 0 then
        dprint("Failed to read request target facet")
        return 0
    end
    if facet.value == nil then
        facet.value = "none"
    end
    consumed = consumed + n

    -- read op
    local n, op = read_operation(tvbuf, offset + consumed)
    if n == 0 then
        dprint("Failed to read op specs")
        return 0
    end
    consumed = consumed + n

    inf = inf .. "::" .. op.value
    pktinfo.cols.info:append(" Request" .. inf)
    root:append_text(" Request" .. inf)

    local mode = tvbuf:range(offset + consumed, 1)
    consumed = consumed + 1

    -- TODO Read context

    local req_tree = root:add(hdr_fields.request, tvbuf:range(offset, consumed), inf)
    req_tree:add(hdr_fields.request_no, tvbuf:range(offset, num_sz), req_no)
    req_tree:add(hdr_fields.request_tgt, tvbuf:range(identity.offset, identity.size),
            identity.value)
    req_tree:add(hdr_fields.request_fct, tvbuf:range(facet.offset, facet.size),
            facet.value)
    req_tree:add(hdr_fields.request_op, tvbuf:range(op.offset, op.size),
            op.value)
    req_tree:add(hdr_fields.request_mode, tvbuf:range(op.offset + op.size, 1))
    req_tree:add(hdr_fields.request_multi, tvbuf:range(op.offset + op.size, 1))
    req_tree:add(hdr_fields.request_noctx, tvbuf:range(op.offset + op.size, 1))
    req_tree:add(hdr_fields.request_nobody, tvbuf:range(op.offset + op.size, 1))

    return consumed
end

--------------------------------------------------------------------------------
--  Dissect reply
dissectReply = function(tvbuf, pktinfo, root, offset)
    local rep = {}
    dprint2("Dissect reply header")
    local consumed, req_no = read_uint(tvbuf,offset,8)
    if consumed == 0 then
        dprint("Failed to read request number")
        return 0
    end
    local num_sz = consumed
    local status = tvbuf:range(offset + consumed, 1):uint()

    local inf = " #" .. req_no .. " " .. repstatus_valstr[status]
    pktinfo.cols.info:append(" Reply" .. inf)
    root:append_text(" Reply" .. inf)

    consumed = consumed + 1 -- message status
    local rep_tree = root:add(hdr_fields.reply, tvbuf:range(offset, consumed), inf)
    rep_tree:add(hdr_fields.reply_no, tvbuf:range(offset, num_sz), req_no)
    rep_tree:add(hdr_fields.reply_status, tvbuf:range(offset + num_sz, 1))

    return consumed
end
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
end
--------------------------------------------------------------------------------
--  Read string from buffer
--  Return number of bytes consumed and string together with sizes and offsets
read_string = function(tvbuf, offset)
    dprint2("Read string")
    local n, sz = read_uint(tvbuf, offset, 8)
    if n == 0 then
        dprint2("Failed to read string size")
        return 0
    end
    dprint2("String size " .. sz)
    local str = {}
    if sz > 0 then
        str = {
            offset    = offset,
            size      = n + sz,
            size_len  = n,
            value     = tvbuf:range(offset + n, sz):string()
        }
        dprint2("Value is " .. str.value)
    else
        str = {
            offset    = offset,
            size      = n,
            size_len  = n,
            value     = nil
        }
        dprint2("Value is nil")
    end
    return n + sz, str
end

--------------------------------------------------------------------------------
--  Read uuid from buffer
--
read_uuid = function(tvbuf, offset)
    dprint2("Read uuid")
    local uuid_str = ""
    for i=0,15 do
        if i == 4 or i == 6 or i == 8 or i == 10 then
            uuid_str = uuid_str .. "-"
        end
        local byte = tvbuf:range(offset + i, 1):uint()
        uuid_str = uuid_str .. string.format("%02x", byte)
    end
    return 16, uuid_str
end

--------------------------------------------------------------------------------
--  Read identity from buffer
--  Return number of bytes consumed and identity object (if any)
read_identity = function(tvbuf, offset)
    dprint2("Read identity")
    local consumed = 0
    local n, cat = read_string(tvbuf, offset)
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
        identity.value = ""
    end
    consumed = consumed + n
    local type = tvbuf:range(offset + consumed, 1):uint()
    consumed = consumed + 1
    if type == 0 then -- id is string
        local n, id = read_string(tvbuf, offset + consumed)
        if n == 0 then
            dprint("Failed to read identity:id string from buffer")
            return 0
        end
        consumed = consumed + n
        identity.id = id
        identity.id_size = n

        identity.value = identity.value .. id.value
    elseif type == 1 then  -- id is uuid
        local n, id = read_uuid(tvbuf, offset + consumed)
        if n == 0 then
            dprint("Failed to read identity:id uuid from buffer")
            return 0
        end
        consumed = consumed + n
        identity.id = id
        identity.id_size = n

        identity.value = identity.value .. id
    else -- id is wildcard
        consumed = consumed + 1
        identity.value = identity.value .. "*"
    end
    identity.size = consumed
    return consumed, identity
end

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
        op.value = string.format("0x%x", hash)
        consumed = consumed + 4
    else  -- Op name
        local n, name = read_string(tvbuf, offset + consumed)
        if n == 0 then
            dprint("Failed to read literal op name")
            return 0
        end
        op.value = name.value
        consumed = consumed + n
    end
    op.size = consumed
    return consumed, op
end

--------------------------------------------------------------------------------
--  Read version information from buffer
--  uint32 major uint32 minor
--  Returns number of consumed bytes and version object (if any)
read_version = function(tvbuf, offset)
    local consumed = 0
    local version  = {
        offset = offset
    }
    local n, val = read_uint(tvbuf,offset,4)
    if n == 0 then
        dprint("Failed to read protocol major")
        return 0
    end
    version.major = val
    consumed = consumed + n
    offset = offset + consumed
    n, val = read_uint(tvbuf,offset,4)
    if n == 0 then
        dprint("Failed to read protocol minor")
        return 0
    end
    version.minor = val
    consumed = consumed + n
    version.size = consumed
    return consumed, version
end

--------------------------------------------------------------------------------
checkWireHeader = function(tvbuf, offset)
    local msglen = tvbuf:len() - offset

    -- check if capture was only capturing partial packet size
    if msglen ~= tvbuf:reported_length_remaining(offset) then
        -- captured packets are being sliced/cut-off, so don't try to desegment/reassemble
        dprint("Captured packet was shorter than original, can't reassemble")
        return 0
    end

    if msglen < WIRE_MIN_HEADER_SIZE then
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
    dprint2("Magic number OK offset " .. offset)

    local flags_tvbr = tvbuf:range(offset, 1)
    offset = offset + 1
    local flags_val  = flags_tvbr:uint()

    local msg_header = {}
    msg_header.type = bit32.band(flags_val, 0x7)
    dprint2("Message flags value " .. flags_val .. " type " .. msg_header.type)
    if (bit32.band(flags_val, 0x8) > 0) then
        dprint2("Read protocol version")
        local consumed, tmp = read_version(tvbuf,offset)
        if consumed <= 0 then
            dprint("Failed to read protocol version")
            return -DESEGMENT_ONE_MORE_SEGMENT
        end
        msg_header.proto_version = tmp
        offset = offset + consumed
        dprint2("Protocol version " .. msg_header.proto_version.major ..
            "." .. msg_header.proto_version.minor ..
            " offset " .. tmp.offset .. " size " .. tmp.size)
    end
    if (bit32.band(flags_val, 0x10) > 0) then
        dprint2("Read encoding version")
        local consumed, tmp = read_version(tvbuf,offset)
        if consumed <= 0 then
            dprint("Failed to read encoding version")
            return -DESEGMENT_ONE_MORE_SEGMENT
        end
        msg_header.encoding_version = tmp
        offset = offset + consumed
        dprint2("Encoding version " .. msg_header.encoding_version.major ..
            "." .. msg_header.encoding_version.minor)
    end
    local consumed, msg_size = read_uint(tvbuf,offset,8)
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
end

local function wire_proto_heuristic (tvbuf, pktinfo, root)
    local bytesConsumed = wire_proto.dissector(tvbuf,pktinfo,root)
    return bytesConsumed ~= 0
end
--------------------------------------------------------------------------------
-- We want to have our protocol dissection invoked for a specific TCP port,
-- so get the TCP dissector table and add our protocol to it.
local function enableDissector()
    -- using DissectorTable:set() removes existing dissector(s), whereas the
    -- DissectorTable:add() one adds ours before any existing ones, but
    -- leaves the other ones alone, which is better
    --Proto.register_heuristic("tcp", wire_proto_heuristic)
    wire_proto:register_heuristic("tcp", wire_proto_heuristic)
    --DissectorTable.get("tcp.port"):add(default_settings.port, wire_proto)
    dprint2("Dissector enabled")
end
-- call it now, because we're enabled by default
enableDissector()

local function disableDissector()
    DissectorTable.get("tcp.port"):remove(default_settings.port, wire_proto)
    dprint2("Dissector disabled")
end

