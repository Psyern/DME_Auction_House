---
name: create-rpc
description: Create a new RPC (Remote Procedure Call) for client-server communication in DayZ
---

# Create DayZ RPC

Creates RPCs for client-server communication following DayZ Native RPC system conventions.

## Required Input
- RPC name/purpose
- Direction: Client->Server, Server->Client, or Bidirectional
- Data to transmit (parameters)

## Steps

### 1. Define RPC Enum
File: `scripts/3_Game/DME_AH/Enums/DME_AH_Enums.c`
```c
enum EDME_AH_RPCs
{
	DME_AH_RPC_START = 10000,
	DME_AH_RPC_REQUEST_LISTINGS,
	DME_AH_RPC_SEND_LISTINGS,
	DME_AH_RPC_PLACE_BID,
	DME_AH_RPC_BID_RESULT,
	// ... add new RPCs here
	DME_AH_RPC_END
}
```

### 2. Create RPC Handler (3_Game layer)
File: `scripts/3_Game/DME_AH/RPC/DME_AH_RPCHandler.c`

Pattern for handler:
```c
void DME_AH_RPC_HandlerName(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
{
	if (type != CallType.Server) return;  // or CallType.Client

	// Read Param as SINGLE OBJECT (never individual fields!)
	Param2<string, int> data;
	if (!ctx.Read(data)) return;

	string value1 = data.param1;
	int value2 = data.param2;

	// Process...
}
```

### 3. Register RPCs
File: `scripts/5_Mission/DME_AH/MissionGameplay.c` (client-side)
File: `scripts/5_Mission/DME_AH/MissionServer.c` (server-side)

### 4. Send RPC
```c
if (!g_Game) return;

auto data = new Param2<string, int>("value", 42);
g_Game.RPC(null, EDME_AH_RPCs.DME_AH_RPC_REQUEST_LISTINGS, data, true, identity);
// true = guaranteed delivery
// identity = target player (null for broadcast)
```

## Critical Rules
- RPC enum values start at 10000+ (avoid vanilla conflicts)
- ALWAYS check `g_Game` for null before sending
- ALWAYS validate parameters in handlers
- Read Param as SINGLE object, never individual fields
- Check `CallType` to ensure correct execution side
- Use `true` (guaranteed) for important updates
- Use `false` (non-guaranteed) for frequent updates
- Handle singleplayer with appropriate `SingleplayerExecutionType`
- Prefix ALL RPC names with `DME_AH_`

## Checklist
- [ ] RPC enum defined at 10000+
- [ ] Handler validates CallType
- [ ] Handler reads Param as single object
- [ ] Sender checks g_Game for null
- [ ] Parameters validated in handler
- [ ] Registered in correct mission file
- [ ] Prefixed with DME_AH_
