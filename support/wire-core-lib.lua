
dprint2("Add core functions")

wire.types.alias("::wire::core::identity::id_type", wire.types.variant( { wire.types.type("string"), wire.types.type("uuid") } ))

local identity = wire.types.structure("::wire::core::identity", {
	fields			= {
		{ 
			name	= "category",
			type 	= wire.types.type("string"), 
		},
		{ 
			name    = "id", 
			type 	= wire.types.type("::wire::core::identity::id_type") 
		},
	}
})

local ep_data = wire.types.structure("::wire::core::inet_endpoint_data", {
	fields 			= {
		{
			name 	= "host",
			type 	= wire.types.type("string")
		},
		{
			name 	= "port",
			type 	= wire.types.type("uint16") 
		}
	}
})

local socket_endpoint = wire.types.structure("::wire::core::socket_endpoint", {
	fields 			= {
		{
			name 	= "path",
			type 	= wire.types.type("string")
		},
	}
})

local tcp_endpoint 		= wire.types.alias("::wire::core::tcp_endpoint", ep_data)
local ssl_endpoint 		= wire.types.alias("::wire::core::ssl_endpoint", ep_data)
local udp_endpoint 		= wire.types.alias("::wire::core::udp_endpoint", ep_data)

local endpoint_type = wire.types.enum("::wire::core::endpoint_type",
{
	empty	= 0x00,
	tcp		= 0x01,
	ssl		= 0x03,
	udp		= 0x04,
	socket  = 0x05,
})

local endpoint = wire.types.alias("::wire::core::endpoint", 
	wire.types.variant( { ep_data, tcp_endpoint, ssl_endpoint, udp_endpoint, socket_endpoint } , 
	endpoint_type))

local reference_data = wire.types.structure("::wire::core::reference_data", {
	fields 			= {
		{
			name 	= "object_id", 
			type 	= identity,
		},
		{
			name 	= "facet",
			type 	= wire.types.type("string") 
		},
		{
			name 	= "adapter",
			type	= wire.types.optional(identity),
		},
		{
			name 	= "endpoints",
			type 	= wire.types.sequence(endpoint)
		}
	}
})

wire.types.alias("proxy", reference_data)

wire.types.interface("::wire::core::object", {
	functions 		= {
		wire_is_a 	= {
			hash	= "0x1cdc304b",
			params  = {
				{
					name = "id",
					type = wire.types.type("string") 
				}
			},
		},
		wire_ping 	= {
			hash 	= "0x7052047d",
			params  = {},
		},
		wire_type   = {
			hash	= "0x82d9cb3a",
			params  = {},
			ret 	= wire.types.type("string")
		},
		wire_types	= {
			hash	= "0x452e5af9",
			params  = {},
			ret 	= wire.types.sequence( wire.types.type("string") )
		},
	}
})

