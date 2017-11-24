------------------------------------------------------------------------------
-- author: Sergei Fedorov <sergei.a.fedorov@gmail.com>
-- Copyright (c) 2016, 2017, Sergei Fedorov
------------------------------------------------------------------------------

local function load_core_lib()
    -- body
    local path = persconffile_path('wire_plugins/')
    dofile(path .. 'wire-core.lua')
    dofile(path .. 'wire-core-lib.lua')
    info('WIRE:  Core lib loaded')
end
local function load_generated_protos()
    local path = persconffile_path('wire_plugins/')
    for fname in Dir.open(path) do
        if fname:find('wire%-core', 0) == nil then
            dprint2('Generated proto', path .. fname)
            dofile(path .. fname)
        end
    end
    for k, v in pairs(wire.function_dictionary) do
        dprint2('Registered function', v.name)
    end
end


load_core_lib()
load_generated_protos()
-- if pcall(load_generated_protos) then
--     dprint2("Protos loaded OK")
-- else
--     dprint2("Failed to load protos")
-- end

--wire_functions['test'].run('test the func')

--------------------------------------------------------------------------------
-- creates a Proto object, but doesn't register it yet
local wire_proto = Proto("wire", "Wire Protocol");

wire_proto.fields = wire.hdr_fields

dprint2("wire_proto ProtoFields registered")

-- due to a bug in older (prior to 1.12) wireshark versions, we need to keep newly created
-- Tvb's for longer than the duration of the dissect function (see bug 10888)
-- this bug only affects dissectors that create new Tvb's, which is not that common
-- but this FPM dissector happens to do it in order to create the fake SLL header
-- to pass on to the Netlink dissector
local tvbs = {}

--wire_functions = {}

---------------------------------------
-- This function will be invoked by Wireshark during initialization, such as
-- at program start and loading a new file
function wire_proto.init()
    -- reset the save Tvbs
    tvbs = {}
end

-- some forward "declarations" of helper functions we use in the dissector
local dissectWire

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
            pktinfo.desegment_offset = Int64(bytes_consumed):tonumber()

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
    return Int64(bytes_consumed):tonumber()
end

--------------------------------------------------------------------------------
dissectWire = function(tvbuf, pktinfo, root, offset)
    dprint2("Wire dissect function called. offset = " .. offset)

    local consumed, msg_header = wire.protocol.read_header(tvbuf, offset)

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

    local tree = root:add(wire_proto, range64(tvbuf, offset, consumed + msg_header.size))
    local hdr_tree = tree:add(wire.hdr_fields.header, range64(tvbuf, offset, consumed),
        "Size: " .. consumed)
    -- magic number
    hdr_tree:add(wire.hdr_fields.magic_type, range64(tvbuf, offset, 4))
    -- flags
    hdr_tree:add(wire.hdr_fields.msg_type, range64(tvbuf, offset + 4, 1))
    hdr_tree:add(wire.hdr_fields.proto_flag, range64(tvbuf, offset + 4, 1))
    hdr_tree:add(wire.hdr_fields.enc_flag, range64(tvbuf, offset + 4, 1))

    local data_size = msg_header.size
    dprint2("Tree added, adding info")
    if msg_header.type == 0 then
        local rn = wire.protocol.dissect_request(tvbuf, pktinfo, tree, offset + consumed)
        if rn == 0 then
            dprint("Failed to dissect request")
            return 0
        end
        consumed = consumed + rn
        data_size = data_size - rn
    elseif msg_header.type == 2 then
        local rn = wire.protocol.dissect_reply(tvbuf, pktinfo, tree, offset + consumed)
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
        hdr_tree:add(wire.hdr_fields.proto_version,
            range64(tvbuf, msg_header.proto_version.offset, msg_header.proto_version.size), msg_header.proto_version.value)
    end
    if msg_header.encoding_version then
        pktinfo.cols.info:append(" encoding " .. msg_header.encoding_version.major ..
            "." .. msg_header.encoding_version.minor)
        hdr_tree:add(wire.hdr_fields.enc_version,
            range64(tvbuf, msg_header.encoding_version.offset, msg_header.encoding_version.size), msg_header.encoding_version.value)
    end
    pktinfo.cols.info:append(" size " .. msg_header.size)
    hdr_tree:add(wire.hdr_fields.msg_size,
          range64(tvbuf, msg_header.size_offset, msg_header.size_length), msg_header.size:tonumber())

    if data_size > 0 then
        local tvb = tvbuf(offset + consumed, data_size):tvb()
        data:call(tvb, pktinfo, tree)
    end

    return consumed + data_size
end

--------------------------------------------------------------------------------
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

