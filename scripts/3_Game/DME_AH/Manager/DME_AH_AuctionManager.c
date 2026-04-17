// DME Auction House - Auction Manager (Server-Side Business Logic)

class DME_AH_AuctionManager
{
	protected ref DME_AH_Config m_Config;
	protected ref DME_AH_CategoryConfig m_CategoryConfig;
	protected ref DME_AH_DataStore m_DataStore;
	protected ref DME_AH_CurrencyAdapter m_CurrencyAdapter;
	protected float m_ExpiryCheckTimer;

	void DME_AH_AuctionManager()
	{
		m_ExpiryCheckTimer = 0;
	}

	void Init(DME_AH_Config config, DME_AH_CategoryConfig categoryConfig, DME_AH_DataStore dataStore, DME_AH_CurrencyAdapter currencyAdapter)
	{
		m_Config = config;
		m_CategoryConfig = categoryConfig;
		m_DataStore = dataStore;
		m_CurrencyAdapter = currencyAdapter;
		DME_AH_Logger.Info("AuctionManager initialized");
	}

	void Update(float deltaTime)
	{
		m_ExpiryCheckTimer += deltaTime;
		if (m_ExpiryCheckTimer >= DME_AH_EXPIRY_CHECK_INTERVAL)
		{
			m_ExpiryCheckTimer = 0;
			CheckExpiredListings();
		}
	}

	// --- Create Listing ---
	int CreateListing(string sellerUID, string sellerName, string itemClassName, int listingType, int startPrice, int buyNowPrice, int durationMinutes, int categoryID)
	{
		if (!m_Config || !m_DataStore)
			return EDME_AH_ResultCode.FailedServerError;

		if (startPrice < m_Config.MinPrice || startPrice > m_Config.MaxPrice)
			return EDME_AH_ResultCode.FailedInvalidPrice;

		if (buyNowPrice > 0 && (buyNowPrice < m_Config.MinPrice || buyNowPrice > m_Config.MaxPrice))
			return EDME_AH_ResultCode.FailedInvalidPrice;

		if (buyNowPrice > 0 && buyNowPrice <= startPrice && listingType == EDME_AH_ListingType.AuctionWithBuyNow)
			return EDME_AH_ResultCode.FailedInvalidPrice;

		if (!m_Config.IsDurationAllowed(durationMinutes))
			return EDME_AH_ResultCode.FailedInvalidPrice;

		int activeCount = m_DataStore.GetActiveListingCountForPlayer(sellerUID);
		if (activeCount >= m_Config.MaxListingsPerPlayer)
			return EDME_AH_ResultCode.FailedMaxListingsReached;

		int listingFee = m_Config.CalculateListingFee(startPrice);
		if (listingFee > 0 && !m_CurrencyAdapter.HasEnough(sellerUID, listingFee))
			return EDME_AH_ResultCode.FailedNotEnoughMoney;

		if (listingFee > 0)
		{
			bool deducted = m_CurrencyAdapter.Deduct(sellerUID, listingFee);
			if (!deducted)
				return EDME_AH_ResultCode.FailedServerError;
		}

		int currentTime = DME_AH_Util.GetTimestamp();
		int expiresTime = currentTime + (durationMinutes * 60);

		DME_AH_Listing listing = new DME_AH_Listing();
		listing.ListingID = DME_AH_Listing.GenerateID();
		listing.SellerUID = sellerUID;
		listing.SellerName = sellerName;
		listing.ItemClassName = itemClassName;
		listing.ItemDisplayName = itemClassName;
		listing.CategoryID = categoryID;
		listing.ListingType = listingType;
		listing.StartPrice = startPrice;
		listing.BuyNowPrice = buyNowPrice;
		listing.CurrentBid = 0;
		listing.CurrentBidderUID = "";
		listing.CurrentBidderName = "";
		listing.BidCount = 0;
		listing.CreatedTimestamp = currentTime;
		listing.ExpiresTimestamp = expiresTime;
		listing.Status = EDME_AH_ListingStatus.Active;

		m_DataStore.AddListing(listing);
		string feeStr = m_Config.CalculateListingFee(startPrice).ToString();
		DME_AH_Logger.Info("Listing created: " + listing.ListingID + " by " + sellerName + " for " + itemClassName + " (fee: " + feeStr + ")");
		return EDME_AH_ResultCode.Success;
	}

	// --- Buy Now ---
	int BuyNow(string buyerUID, string buyerName, string listingID)
	{
		if (!m_Config || !m_DataStore || !m_CurrencyAdapter)
			return EDME_AH_ResultCode.FailedServerError;

		DME_AH_Listing listing = m_DataStore.GetListingByID(listingID);
		if (!listing)
			return EDME_AH_ResultCode.FailedListingNotFound;

		if (!listing.IsActive())
			return EDME_AH_ResultCode.FailedListingExpired;

		if (!listing.HasBuyNow())
			return EDME_AH_ResultCode.FailedListingNotFound;

		if (listing.SellerUID == buyerUID)
			return EDME_AH_ResultCode.FailedOwnListing;

		int price = listing.StartPrice;
		if (listing.ListingType == EDME_AH_ListingType.AuctionWithBuyNow)
			price = listing.BuyNowPrice;

		if (!m_CurrencyAdapter.HasEnough(buyerUID, price))
			return EDME_AH_ResultCode.FailedNotEnoughMoney;

		bool deducted = m_CurrencyAdapter.Deduct(buyerUID, price);
		if (!deducted)
			return EDME_AH_ResultCode.FailedServerError;

		int saleFee = m_Config.CalculateSaleFee(price);
		int sellerReceives = price - saleFee;

		bool added = m_CurrencyAdapter.Add(listing.SellerUID, sellerReceives);
		if (!added)
		{
			DME_AH_PendingPickup pickup = new DME_AH_PendingPickup();
			pickup.PendingID = "PND_" + listing.ListingID;
			pickup.PlayerUID = listing.SellerUID;
			pickup.Amount = sellerReceives;
			pickup.Type = EDME_AH_TransactionType.BuyNow;
			pickup.Timestamp = DME_AH_Util.GetTimestamp();
			m_DataStore.AddPendingPickup(pickup);
		}

		if (listing.HasBids())
			RefundCurrentBidder(listing);

		listing.Status = EDME_AH_ListingStatus.Sold;

		DME_AH_Transaction transaction = new DME_AH_Transaction();
		transaction.TransactionID = DME_AH_Transaction.GenerateID();
		transaction.ListingID = listing.ListingID;
		transaction.SellerUID = listing.SellerUID;
		transaction.BuyerUID = buyerUID;
		transaction.SellerName = listing.SellerName;
		transaction.BuyerName = buyerName;
		transaction.ItemClassName = listing.ItemClassName;
		transaction.ItemDisplayName = listing.ItemDisplayName;
		transaction.FinalPrice = price;
		transaction.Fee = saleFee;
		transaction.Timestamp = DME_AH_Util.GetTimestamp();
		transaction.Type = EDME_AH_TransactionType.BuyNow;

		m_DataStore.AddTransaction(transaction);
		m_DataStore.RemoveListing(listing.ListingID);

		DME_AH_Logger.Info("BuyNow: " + buyerName + " bought " + listing.ItemClassName + " for " + price.ToString());
		return EDME_AH_ResultCode.Success;
	}

	// --- Place Bid ---
	int PlaceBid(string bidderUID, string bidderName, string listingID, int bidAmount)
	{
		if (!m_Config || !m_DataStore || !m_CurrencyAdapter)
			return EDME_AH_ResultCode.FailedServerError;

		DME_AH_Listing listing = m_DataStore.GetListingByID(listingID);
		if (!listing)
			return EDME_AH_ResultCode.FailedListingNotFound;

		if (!listing.IsActive())
			return EDME_AH_ResultCode.FailedListingExpired;

		if (!listing.IsAuction())
			return EDME_AH_ResultCode.FailedListingNotFound;

		if (listing.SellerUID == bidderUID)
			return EDME_AH_ResultCode.FailedOwnListing;

		int minBid = m_Config.GetMinBidForListing(listing);
		if (bidAmount < minBid)
			return EDME_AH_ResultCode.FailedBidTooLow;

		if (!m_CurrencyAdapter.HasEnough(bidderUID, bidAmount))
			return EDME_AH_ResultCode.FailedNotEnoughMoney;

		if (listing.HasBids())
			RefundCurrentBidder(listing);

		bool deducted = m_CurrencyAdapter.Deduct(bidderUID, bidAmount);
		if (!deducted)
			return EDME_AH_ResultCode.FailedServerError;

		string previousBidderUID = listing.CurrentBidderUID;

		listing.CurrentBid = bidAmount;
		listing.CurrentBidderUID = bidderUID;
		listing.CurrentBidderName = bidderName;
		listing.BidCount = listing.BidCount + 1;

		m_DataStore.SaveActiveListings();

		DME_AH_Logger.Info("Bid placed: " + bidderName + " bid " + bidAmount.ToString() + " on " + listing.ItemClassName);
		return EDME_AH_ResultCode.Success;
	}

	// --- Cancel Listing ---
	int CancelListing(string playerUID, string listingID)
	{
		if (!m_DataStore)
			return EDME_AH_ResultCode.FailedServerError;

		DME_AH_Listing listing = m_DataStore.GetListingByID(listingID);
		if (!listing)
			return EDME_AH_ResultCode.FailedListingNotFound;

		if (listing.SellerUID != playerUID)
			return EDME_AH_ResultCode.FailedListingNotFound;

		if (!listing.IsActive())
			return EDME_AH_ResultCode.FailedListingExpired;

		if (listing.HasBids())
			return EDME_AH_ResultCode.FailedCannotCancelWithBids;

		listing.Status = EDME_AH_ListingStatus.Cancelled;

		DME_AH_Transaction transaction = new DME_AH_Transaction();
		transaction.TransactionID = DME_AH_Transaction.GenerateID();
		transaction.ListingID = listing.ListingID;
		transaction.SellerUID = listing.SellerUID;
		transaction.SellerName = listing.SellerName;
		transaction.ItemClassName = listing.ItemClassName;
		transaction.ItemDisplayName = listing.ItemDisplayName;
		transaction.Timestamp = DME_AH_Util.GetTimestamp();
		transaction.Type = EDME_AH_TransactionType.Cancelled;

		m_DataStore.AddTransaction(transaction);
		m_DataStore.RemoveListing(listing.ListingID);

		DME_AH_Logger.Info("Listing cancelled: " + listing.ListingID + " by " + listing.SellerName);
		return EDME_AH_ResultCode.Success;
	}

	// --- Check Expired Listings ---
	void CheckExpiredListings()
	{
		if (!m_DataStore)
			return;

		array<ref DME_AH_Listing> listings = m_DataStore.GetActiveListings();
		int currentTime = DME_AH_Util.GetTimestamp();

		for (int i = listings.Count() - 1; i >= 0; i--)
		{
			DME_AH_Listing listing = listings[i];
			if (!listing || !listing.IsActive())
				continue;

			bool isExpired = currentTime >= listing.ExpiresTimestamp;
			if (!isExpired)
				continue;

			if (listing.IsAuction() && listing.HasBids())
				CompleteAuction(listing);
			else
				ExpireListing(listing);
		}
	}

	protected void CompleteAuction(DME_AH_Listing listing)
	{
		if (!listing || !m_CurrencyAdapter || !m_DataStore)
			return;

		int saleFee = m_Config.CalculateSaleFee(listing.CurrentBid);
		int sellerReceives = listing.CurrentBid - saleFee;

		bool added = m_CurrencyAdapter.Add(listing.SellerUID, sellerReceives);
		if (!added)
		{
			DME_AH_PendingPickup pickup = new DME_AH_PendingPickup();
			pickup.PendingID = "PND_" + listing.ListingID;
			pickup.PlayerUID = listing.SellerUID;
			pickup.Amount = sellerReceives;
			pickup.Type = EDME_AH_TransactionType.AuctionWon;
			pickup.Timestamp = DME_AH_Util.GetTimestamp();
			m_DataStore.AddPendingPickup(pickup);
		}

		listing.Status = EDME_AH_ListingStatus.Sold;

		DME_AH_Transaction transaction = new DME_AH_Transaction();
		transaction.TransactionID = DME_AH_Transaction.GenerateID();
		transaction.ListingID = listing.ListingID;
		transaction.SellerUID = listing.SellerUID;
		transaction.BuyerUID = listing.CurrentBidderUID;
		transaction.SellerName = listing.SellerName;
		transaction.BuyerName = listing.CurrentBidderName;
		transaction.ItemClassName = listing.ItemClassName;
		transaction.ItemDisplayName = listing.ItemDisplayName;
		transaction.FinalPrice = listing.CurrentBid;
		transaction.Fee = saleFee;
		transaction.Timestamp = DME_AH_Util.GetTimestamp();
		transaction.Type = EDME_AH_TransactionType.AuctionWon;

		m_DataStore.AddTransaction(transaction);
		m_DataStore.RemoveListing(listing.ListingID);

		DME_AH_Logger.Info("Auction completed: " + listing.ItemClassName + " sold to " + listing.CurrentBidderName + " for " + listing.CurrentBid.ToString());
	}

	protected void ExpireListing(DME_AH_Listing listing)
	{
		if (!listing || !m_DataStore)
			return;

		listing.Status = EDME_AH_ListingStatus.Expired;

		DME_AH_Transaction transaction = new DME_AH_Transaction();
		transaction.TransactionID = DME_AH_Transaction.GenerateID();
		transaction.ListingID = listing.ListingID;
		transaction.SellerUID = listing.SellerUID;
		transaction.SellerName = listing.SellerName;
		transaction.ItemClassName = listing.ItemClassName;
		transaction.ItemDisplayName = listing.ItemDisplayName;
		transaction.Timestamp = DME_AH_Util.GetTimestamp();
		transaction.Type = EDME_AH_TransactionType.Expired;

		m_DataStore.AddTransaction(transaction);
		m_DataStore.RemoveListing(listing.ListingID);

		DME_AH_Logger.Info("Listing expired: " + listing.ListingID + " (" + listing.ItemClassName + ")");
	}

	protected void RefundCurrentBidder(DME_AH_Listing listing)
	{
		if (!listing || !m_CurrencyAdapter)
			return;

		if (listing.CurrentBidderUID == "" || listing.CurrentBid <= 0)
			return;

		bool refunded = m_CurrencyAdapter.Add(listing.CurrentBidderUID, listing.CurrentBid);
		if (!refunded)
		{
			DME_AH_PendingPickup pickup = new DME_AH_PendingPickup();
			pickup.PendingID = "REFUND_" + listing.ListingID + "_" + listing.CurrentBidderUID;
			pickup.PlayerUID = listing.CurrentBidderUID;
			pickup.Amount = listing.CurrentBid;
			pickup.Timestamp = DME_AH_Util.GetTimestamp();
			m_DataStore.AddPendingPickup(pickup);
		}

		DME_AH_Logger.Debug("Refunded " + listing.CurrentBid.ToString() + " to " + listing.CurrentBidderName);
	}

	// --- Query Methods ---
	array<ref DME_AH_Listing> GetFilteredListings(int categoryID, string searchText, int sortMode, int page)
	{
		if (!m_DataStore)
			return new array<ref DME_AH_Listing>;

		array<ref DME_AH_Listing> allListings = m_DataStore.GetActiveListings();
		array<ref DME_AH_Listing> filtered = new array<ref DME_AH_Listing>;

		string searchLower = searchText;
		searchLower.ToLower();

		for (int i = 0; i < allListings.Count(); i++)
		{
			DME_AH_Listing listing = allListings[i];
			if (!listing || !listing.IsActive())
				continue;

			if (categoryID > 0 && listing.CategoryID != categoryID)
				continue;

			if (searchLower != "")
			{
				string itemNameLower = listing.ItemDisplayName;
				itemNameLower.ToLower();
				if (itemNameLower.IndexOf(searchLower) == -1)
					continue;
			}

			filtered.Insert(listing);
		}

		SortListings(filtered, sortMode);

		int startIdx = page * DME_AH_LISTINGS_PER_PAGE;
		int endIdx = startIdx + DME_AH_LISTINGS_PER_PAGE;
		if (endIdx > filtered.Count())
			endIdx = filtered.Count();

		array<ref DME_AH_Listing> pageResults = new array<ref DME_AH_Listing>;
		for (int j = startIdx; j < endIdx; j++)
		{
			pageResults.Insert(filtered[j]);
		}

		return pageResults;
	}

	int GetFilteredListingsCount(int categoryID, string searchText)
	{
		if (!m_DataStore)
			return 0;

		array<ref DME_AH_Listing> allListings = m_DataStore.GetActiveListings();
		int count = 0;

		string searchLower = searchText;
		searchLower.ToLower();

		for (int i = 0; i < allListings.Count(); i++)
		{
			DME_AH_Listing listing = allListings[i];
			if (!listing || !listing.IsActive())
				continue;
			if (categoryID > 0 && listing.CategoryID != categoryID)
				continue;
			if (searchLower != "")
			{
				string itemNameLower = listing.ItemDisplayName;
				itemNameLower.ToLower();
				if (itemNameLower.IndexOf(searchLower) == -1)
					continue;
			}
			count++;
		}
		return count;
	}

	protected void SortListings(array<ref DME_AH_Listing> listings, int sortMode)
	{
		if (!listings || listings.Count() <= 1)
			return;

		for (int i = 0; i < listings.Count() - 1; i++)
		{
			for (int j = 0; j < listings.Count() - i - 1; j++)
			{
				DME_AH_Listing a = listings[j];
				DME_AH_Listing b = listings[j + 1];
				if (!a || !b)
					continue;

				bool shouldSwap = false;

				if (sortMode == EDME_AH_SortMode.PriceAsc)
					shouldSwap = a.GetCurrentPrice() > b.GetCurrentPrice();
				else if (sortMode == EDME_AH_SortMode.PriceDesc)
					shouldSwap = a.GetCurrentPrice() < b.GetCurrentPrice();
				else if (sortMode == EDME_AH_SortMode.NameAsc)
					shouldSwap = a.ItemDisplayName > b.ItemDisplayName;
				else if (sortMode == EDME_AH_SortMode.NameDesc)
					shouldSwap = a.ItemDisplayName < b.ItemDisplayName;
				else if (sortMode == EDME_AH_SortMode.TimeAsc)
					shouldSwap = a.ExpiresTimestamp > b.ExpiresTimestamp;
				else if (sortMode == EDME_AH_SortMode.TimeDesc)
					shouldSwap = a.ExpiresTimestamp < b.ExpiresTimestamp;
				else if (sortMode == EDME_AH_SortMode.NewestFirst)
					shouldSwap = a.CreatedTimestamp < b.CreatedTimestamp;

				if (shouldSwap)
					listings.SwapItems(j, j + 1);
			}
		}
	}
}
