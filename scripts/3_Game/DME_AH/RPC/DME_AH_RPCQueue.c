// DME Auction House - Client RPC Receiver (3_Game layer)
// Bridges 4_World (PlayerBase.OnRPC) to the menu system
// Uses static listener pattern - menu registers itself

class DME_AH_RPCListener
{
	void OnReceiveListings(string data, int totalPages, int page);
	void OnReceiveBalance(int balance);
	void OnReceiveCallback(int resultCode, string data);
}

class DME_AH_RPCQueue
{
	static DME_AH_RPCListener s_Listener;

	static void SetListener(DME_AH_RPCListener listener)
	{
		s_Listener = listener;
	}

	static void Enqueue(int rpc_type, ParamsReadContext ctx)
	{
		if (rpc_type == EDME_AH_RPC.DME_AH_RPC_SEND_LISTINGS)
		{
			string listingsData;
			if (!ctx.Read(listingsData))
				return;
			int totalPages;
			if (!ctx.Read(totalPages))
				return;
			int page;
			if (!ctx.Read(page))
				return;
			if (s_Listener)
				s_Listener.OnReceiveListings(listingsData, totalPages, page);
		}
		else if (rpc_type == EDME_AH_RPC.DME_AH_RPC_SEND_MY_LISTINGS)
		{
			string myListingsData;
			if (!ctx.Read(myListingsData))
				return;
			if (s_Listener)
				s_Listener.OnReceiveListings(myListingsData, 1, 0);
		}
		else if (rpc_type == EDME_AH_RPC.DME_AH_RPC_SEND_MY_BIDS)
		{
			string myBidsData;
			if (!ctx.Read(myBidsData))
				return;
			if (s_Listener)
				s_Listener.OnReceiveListings(myBidsData, 1, 0);
		}
		else if (rpc_type == EDME_AH_RPC.DME_AH_RPC_SEND_BALANCE)
		{
			int balance;
			if (!ctx.Read(balance))
				return;
			if (s_Listener)
				s_Listener.OnReceiveBalance(balance);
		}
		else if (rpc_type == EDME_AH_RPC.DME_AH_RPC_CALLBACK)
		{
			int resultCode;
			if (!ctx.Read(resultCode))
				return;
			string callbackData;
			if (!ctx.Read(callbackData))
				return;
			if (s_Listener)
				s_Listener.OnReceiveCallback(resultCode, callbackData);
		}
		else if (rpc_type == EDME_AH_RPC.DME_AH_RPC_NOTIFICATION)
		{
			int notifType;
			if (!ctx.Read(notifType))
				return;
			string notifMessage;
			if (!ctx.Read(notifMessage))
				return;
			DME_AH_Logger.Info("Notification: " + notifMessage);
		}
	}
}
