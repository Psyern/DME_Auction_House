// DME Auction House - RPC Handler (CF RPC System)

class DME_AH_RPCHandler
{
	protected ref DME_AH_AuctionManager m_AuctionManager;
	protected ref DME_AH_Config m_Config;
	protected ref DME_AH_DataStore m_DataStore;
	protected ref DME_AH_CurrencyAdapter m_CurrencyAdapter;
	protected ref DME_AH_CategoryConfig m_CategoryConfig;

	void Init(DME_AH_AuctionManager auctionManager, DME_AH_Config config, DME_AH_DataStore dataStore, DME_AH_CurrencyAdapter currencyAdapter, DME_AH_CategoryConfig categoryConfig)
	{
		m_AuctionManager = auctionManager;
		m_Config = config;
		m_DataStore = dataStore;
		m_CurrencyAdapter = currencyAdapter;
		m_CategoryConfig = categoryConfig;
	}

	void RegisterRPCs()
	{
		if (!GetRPCManager())
			return;

		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_RequestListings", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_CreateListing", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_PlaceBid", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_BuyNow", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_CancelListing", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_RequestMyListings", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_RequestMyBids", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_RequestHistory", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_RequestBalance", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_RequestCategories", this, SingleplayerExecutionType.Server);

		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_SendListings", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_SendMyListings", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_SendMyBids", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_SendHistory", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_Callback", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_Notification", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_SendBalance", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_SendCategories", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC("DME_AH_RPCHandler", "RPC_SendConfig", this, SingleplayerExecutionType.Client);

		DME_AH_Logger.Info("RPCs registered");
	}

	// ===== SERVER-SIDE RPC HANDLERS =====

	void RPC_RequestListings(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_AuctionManager)
			return;

		Param4<int, string, int, int> data;
		if (!ctx.Read(data))
			return;

		int categoryID = data.param1;
		string searchText = data.param2;
		int sortMode = data.param3;
		int page = data.param4;

		array<ref DME_AH_Listing> listings = m_AuctionManager.GetFilteredListings(categoryID, searchText, sortMode, page);
		int totalCount = m_AuctionManager.GetFilteredListingsCount(categoryID, searchText);
		int totalPages = totalCount / DME_AH_LISTINGS_PER_PAGE;
		if (totalCount % DME_AH_LISTINGS_PER_PAGE > 0)
			totalPages = totalPages + 1;

		string listingsJson = SerializeListings(listings);
		Param3<string, int, int> response = new Param3<string, int, int>(listingsJson, totalPages, page);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_SendListings", response, true, sender);
	}

	void RPC_CreateListing(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_AuctionManager)
			return;

		Param6<string, int, int, int, int, int> data;
		if (!ctx.Read(data))
			return;

		string itemClassName = data.param1;
		int listingType = data.param2;
		int startPrice = data.param3;
		int buyNowPrice = data.param4;
		int durationMinutes = data.param5;
		int categoryID = data.param6;

		string playerUID = sender.GetPlainId();
		string playerName = sender.GetName();

		int result = m_AuctionManager.CreateListing(playerUID, playerName, itemClassName, listingType, startPrice, buyNowPrice, durationMinutes, categoryID);

		Param2<int, string> response = new Param2<int, string>(result, itemClassName);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_Callback", response, true, sender);
	}

	void RPC_PlaceBid(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_AuctionManager)
			return;

		Param2<string, int> data;
		if (!ctx.Read(data))
			return;

		string listingID = data.param1;
		int bidAmount = data.param2;

		string playerUID = sender.GetPlainId();
		string playerName = sender.GetName();

		int result = m_AuctionManager.PlaceBid(playerUID, playerName, listingID, bidAmount);

		Param2<int, string> response = new Param2<int, string>(result, listingID);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_Callback", response, true, sender);
	}

	void RPC_BuyNow(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_AuctionManager)
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		string listingID = data.param1;
		string playerUID = sender.GetPlainId();
		string playerName = sender.GetName();

		int result = m_AuctionManager.BuyNow(playerUID, playerName, listingID);

		Param2<int, string> response = new Param2<int, string>(result, listingID);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_Callback", response, true, sender);
	}

	void RPC_CancelListing(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_AuctionManager)
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		string listingID = data.param1;
		string playerUID = sender.GetPlainId();

		int result = m_AuctionManager.CancelListing(playerUID, listingID);

		Param2<int, string> response = new Param2<int, string>(result, listingID);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_Callback", response, true, sender);
	}

	void RPC_RequestMyListings(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_DataStore)
			return;

		string playerUID = sender.GetPlainId();
		array<ref DME_AH_Listing> myListings = m_DataStore.GetListingsBySeller(playerUID);

		string listingsJson = SerializeListings(myListings);
		Param1<string> response = new Param1<string>(listingsJson);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_SendMyListings", response, true, sender);
	}

	void RPC_RequestMyBids(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_DataStore)
			return;

		string playerUID = sender.GetPlainId();
		array<ref DME_AH_Listing> allListings = m_DataStore.GetActiveListings();
		array<ref DME_AH_Listing> myBids = new array<ref DME_AH_Listing>;

		for (int i = 0; i < allListings.Count(); i++)
		{
			DME_AH_Listing listing = allListings[i];
			if (listing && listing.CurrentBidderUID == playerUID && listing.IsActive())
				myBids.Insert(listing);
		}

		string listingsJson = SerializeListings(myBids);
		Param1<string> response = new Param1<string>(listingsJson);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_SendMyBids", response, true, sender);
	}

	void RPC_RequestHistory(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_DataStore)
			return;

		Param1<int> data;
		if (!ctx.Read(data))
			return;

		int page = data.param1;
		array<ref DME_AH_Transaction> history = m_DataStore.GetTransactionHistory();

		int startIdx = page * DME_AH_HISTORY_PER_PAGE;
		int endIdx = startIdx + DME_AH_HISTORY_PER_PAGE;
		if (endIdx > history.Count())
			endIdx = history.Count();

		string historyJson = "[]";
		Param2<string, int> response = new Param2<string, int>(historyJson, history.Count());
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_SendHistory", response, true, sender);
	}

	void RPC_RequestBalance(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_CurrencyAdapter)
			return;

		string playerUID = sender.GetPlainId();
		int balance = m_CurrencyAdapter.GetBalance(playerUID);

		Param1<int> response = new Param1<int>(balance);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_SendBalance", response, true, sender);
	}

	void RPC_RequestCategories(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		if (!sender || !m_CategoryConfig)
			return;

		string categoriesJson = "[]";
		Param1<string> response = new Param1<string>(categoriesJson);
		GetRPCManager().SendRPC("DME_AH_RPCHandler", "RPC_SendCategories", response, true, sender);
	}

	// ===== CLIENT-SIDE RPC HANDLERS (stubs - handled by menu) =====

	void RPC_SendListings(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_AuctionMenu
	}

	void RPC_SendMyListings(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_AuctionMenu
	}

	void RPC_SendMyBids(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_AuctionMenu
	}

	void RPC_SendHistory(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_AuctionMenu
	}

	void RPC_Callback(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_AuctionMenu
	}

	void RPC_Notification(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_NotificationHandler
	}

	void RPC_SendBalance(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_AuctionMenu
	}

	void RPC_SendCategories(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_AuctionMenu
	}

	void RPC_SendConfig(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		// Handled by DME_AH_AuctionMenu
	}

	// ===== Serialization Helpers =====

	protected string SerializeListings(array<ref DME_AH_Listing> listings)
	{
		string result = "";
		for (int i = 0; i < listings.Count(); i++)
		{
			DME_AH_Listing listing = listings[i];
			if (!listing)
				continue;
			if (i > 0)
				result = result + "|";
			result = result + listing.ListingID + ";" + listing.SellerName + ";" + listing.ItemClassName + ";" + listing.ItemDisplayName + ";" + listing.CategoryID.ToString() + ";" + listing.ListingType.ToString() + ";" + listing.StartPrice.ToString() + ";" + listing.BuyNowPrice.ToString() + ";" + listing.CurrentBid.ToString() + ";" + listing.CurrentBidderName + ";" + listing.BidCount.ToString() + ";" + listing.ExpiresTimestamp.ToString() + ";" + listing.Status.ToString();
		}
		return result;
	}
}
