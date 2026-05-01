// DME Auction House - RPC Handler (Native ScriptRPC)
// Uses ScriptRPC.Send() for sending, OnRPC() for receiving

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
		DME_AH_Logger.Info("RPCHandler initialized");
	}

	// Called from modded MissionServer OnRPC
	void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		switch (rpc_type)
		{
			case EDME_AH_RPC.DME_AH_RPC_REQUEST_LISTINGS:
				RPC_RequestListings(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_CREATE_LISTING:
				RPC_CreateListing(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_PLACE_BID:
				RPC_PlaceBid(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_BUY_NOW:
				RPC_BuyNow(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_CANCEL_LISTING:
				RPC_CancelListing(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_REQUEST_MY_LISTINGS:
				RPC_RequestMyListings(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_REQUEST_MY_BIDS:
				RPC_RequestMyBids(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_REQUEST_HISTORY:
				RPC_RequestHistory(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_REQUEST_BALANCE:
				RPC_RequestBalance(ctx, sender, target);
				break;
			case EDME_AH_RPC.DME_AH_RPC_REQUEST_CATEGORIES:
				RPC_RequestCategories(ctx, sender, target);
				break;
		}
	}

	// ===== SERVER-SIDE HANDLERS =====

	protected void RPC_RequestListings(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_AuctionManager)
			return;

		int categoryID;
		if (!ctx.Read(categoryID))
			return;
		string searchText;
		if (!ctx.Read(searchText))
			return;
		int sortMode;
		if (!ctx.Read(sortMode))
			return;
		int page;
		if (!ctx.Read(page))
			return;

		array<ref DME_AH_Listing> listings = m_AuctionManager.GetFilteredListings(categoryID, searchText, sortMode, page);
		int totalCount = m_AuctionManager.GetFilteredListingsCount(categoryID, searchText);
		int totalPages = totalCount / DME_AH_LISTINGS_PER_PAGE;
		if (totalCount % DME_AH_LISTINGS_PER_PAGE > 0)
			totalPages = totalPages + 1;

		string listingsData = SerializeListings(listings);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(listingsData);
		rpc.Write(totalPages);
		rpc.Write(page);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_SEND_LISTINGS, true, sender);
	}

	protected void RPC_CreateListing(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_AuctionManager)
			return;

		string itemClassName;
		if (!ctx.Read(itemClassName))
			return;
		int listingType;
		if (!ctx.Read(listingType))
			return;
		int startPrice;
		if (!ctx.Read(startPrice))
			return;
		int buyNowPrice;
		if (!ctx.Read(buyNowPrice))
			return;
		int durationMinutes;
		if (!ctx.Read(durationMinutes))
			return;
		int categoryID;
		if (!ctx.Read(categoryID))
			return;
		int networkLow;
		if (!ctx.Read(networkLow))
			return;
		int networkHigh;
		if (!ctx.Read(networkHigh))
			return;

		string playerUID = sender.GetPlainId();
		string playerName = sender.GetName();
		int result = EDME_AH_ResultCode.FailedServerError;

		// Resolve the entity from NetworkID, validate ownership + no attachments/cargo
		Object obj = null;
		if (g_Game)
			obj = g_Game.GetObjectByNetworkId(networkLow, networkHigh);
		EntityAI item;
		if (!Class.CastTo(item, obj))
		{
			result = EDME_AH_ResultCode.FailedItemNotInInventory;
		}
		else
		{
			// Validate item is in sender's inventory (hierarchy root is the player)
			Man rootMan = item.GetHierarchyRootPlayer();
			if (!rootMan || !rootMan.GetIdentity() || rootMan.GetIdentity().GetPlainId() != playerUID)
			{
				result = EDME_AH_ResultCode.FailedItemNotInInventory;
			}
			else if (item.GetInventory().AttachmentCount() > 0)
			{
				result = EDME_AH_ResultCode.FailedItemHasAttachments;
			}
			else
			{
				// Check cargo contents (if any)
				bool hasCargo = false;
				CargoBase cargo = item.GetInventory().GetCargo();
				if (cargo && cargo.GetItemCount() > 0)
					hasCargo = true;
				if (hasCargo)
				{
					result = EDME_AH_ResultCode.FailedItemHasAttachments;
				}
				else
				{
					// All validation passed — delegate to manager with the resolved entity
					result = m_AuctionManager.CreateListing(playerUID, playerName, itemClassName, listingType, startPrice, buyNowPrice, durationMinutes, categoryID, item);
				}
			}
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(result);
		rpc.Write(itemClassName);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_CALLBACK, true, sender);
	}

	protected void RPC_PlaceBid(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_AuctionManager)
			return;

		string listingID;
		if (!ctx.Read(listingID))
			return;
		int bidAmount;
		if (!ctx.Read(bidAmount))
			return;

		string playerUID = sender.GetPlainId();
		string playerName = sender.GetName();

		int result = m_AuctionManager.PlaceBid(playerUID, playerName, listingID, bidAmount);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(result);
		rpc.Write(listingID);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_CALLBACK, true, sender);
	}

	protected void RPC_BuyNow(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_AuctionManager)
			return;

		string listingID;
		if (!ctx.Read(listingID))
			return;

		string playerUID = sender.GetPlainId();
		string playerName = sender.GetName();

		int result = m_AuctionManager.BuyNow(playerUID, playerName, listingID);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(result);
		rpc.Write(listingID);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_CALLBACK, true, sender);
	}

	protected void RPC_CancelListing(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_AuctionManager)
			return;

		string listingID;
		if (!ctx.Read(listingID))
			return;

		string playerUID = sender.GetPlainId();

		int result = m_AuctionManager.CancelListing(playerUID, listingID);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(result);
		rpc.Write(listingID);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_CALLBACK, true, sender);
	}

	protected void RPC_RequestMyListings(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_DataStore)
			return;

		string playerUID = sender.GetPlainId();
		array<ref DME_AH_Listing> myListings = m_DataStore.GetListingsBySeller(playerUID);

		string listingsData = SerializeListings(myListings);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(listingsData);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_SEND_MY_LISTINGS, true, sender);
	}

	protected void RPC_RequestMyBids(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
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

		string listingsData = SerializeListings(myBids);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(listingsData);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_SEND_MY_BIDS, true, sender);
	}

	protected void RPC_RequestHistory(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_DataStore)
			return;

		string historyData = "";

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(historyData);
		rpc.Write(0);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_SEND_HISTORY, true, sender);
	}

	protected void RPC_RequestBalance(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_CurrencyAdapter)
			return;

		string playerUID = sender.GetPlainId();
		int balance = m_CurrencyAdapter.GetBalance(playerUID);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(balance);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_SEND_BALANCE, true, sender);
	}

	protected void RPC_RequestCategories(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender || !m_CategoryConfig)
			return;

		string categoriesData = "";

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(categoriesData);
		rpc.Send(target, EDME_AH_RPC.DME_AH_RPC_SEND_CATEGORIES, true, sender);
	}

	// ===== Serialization =====

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
			string row = listing.ListingID;
			row = row + ";" + listing.SellerName;
			row = row + ";" + listing.ItemClassName;
			row = row + ";" + listing.ItemDisplayName;
			row = row + ";" + listing.CategoryID.ToString();
			row = row + ";" + listing.ListingType.ToString();
			row = row + ";" + listing.StartPrice.ToString();
			row = row + ";" + listing.BuyNowPrice.ToString();
			row = row + ";" + listing.CurrentBid.ToString();
			row = row + ";" + listing.CurrentBidderName;
			row = row + ";" + listing.BidCount.ToString();
			row = row + ";" + listing.ExpiresTimestamp.ToString();
			row = row + ";" + listing.Status.ToString();
			result = result + row;
		}
		return result;
	}
}
