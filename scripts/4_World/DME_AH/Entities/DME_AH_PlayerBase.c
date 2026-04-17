// DME Auction House - RPC Handling via PlayerBase.OnRPC

modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type < EDME_AH_RPC.DME_AH_RPC_REQUEST_LISTINGS)
			return;
		if (rpc_type > EDME_AH_RPC.DME_AH_RPC_COLLECT_PENDING)
			return;

		// Server-side: forward to RPCHandler
		if (g_Game && g_Game.IsDedicatedServer())
		{
			DME_AH_Module module = DME_AH_Module.GetInstance();
			if (!module)
				return;

			DME_AH_RPCHandler rpcHandler = module.GetRPCHandler();
			if (!rpcHandler)
				return;

			rpcHandler.OnRPC(sender, this, rpc_type, ctx);
			return;
		}

		// Client-side: store for menu to pick up
		DME_AH_RPCQueue.Enqueue(rpc_type, ctx);
	}
}
