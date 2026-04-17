// DME Auction House - Listing Data Class
// JSON-serializable: NO prefix on member variables

class DME_AH_Listing
{
	string ListingID;
	string SellerUID;
	string SellerName;
	string ItemClassName;
	string ItemDisplayName;
	int CategoryID;
	int ListingType;
	int StartPrice;
	int BuyNowPrice;
	int CurrentBid;
	string CurrentBidderUID;
	string CurrentBidderName;
	int BidCount;
	int CreatedTimestamp;
	int ExpiresTimestamp;
	int Status;

	void DME_AH_Listing()
	{
		ListingID = "";
		SellerUID = "";
		SellerName = "";
		ItemClassName = "";
		ItemDisplayName = "";
		CategoryID = 0;
		ListingType = 0;
		StartPrice = 0;
		BuyNowPrice = 0;
		CurrentBid = 0;
		CurrentBidderUID = "";
		CurrentBidderName = "";
		BidCount = 0;
		CreatedTimestamp = 0;
		ExpiresTimestamp = 0;
		Status = 0;
	}

	bool IsActive()
	{
		return Status == EDME_AH_ListingStatus.Active;
	}

	bool IsAuction()
	{
		if (ListingType == EDME_AH_ListingType.Auction)
			return true;
		if (ListingType == EDME_AH_ListingType.AuctionWithBuyNow)
			return true;
		return false;
	}

	bool HasBuyNow()
	{
		if (ListingType == EDME_AH_ListingType.BuyNow)
			return true;
		if (ListingType == EDME_AH_ListingType.AuctionWithBuyNow)
			return true;
		return false;
	}

	bool HasBids()
	{
		return BidCount > 0;
	}

	int GetCurrentPrice()
	{
		if (IsAuction() && HasBids())
			return CurrentBid;
		if (IsAuction())
			return StartPrice;
		return StartPrice;
	}

	bool IsExpired()
	{
		if (!g_Game)
			return false;
		int currentTime = DME_AH_Util.GetTimestamp();
		return currentTime >= ExpiresTimestamp;
	}

	string GetRemainingTimeString()
	{
		if (!g_Game)
			return "N/A";
		int currentTime = DME_AH_Util.GetTimestamp();
		int remaining = ExpiresTimestamp - currentTime;
		if (remaining <= 0)
			return "Abgelaufen";
		int hours = remaining / 3600;
		int minutes = (remaining % 3600) / 60;
		if (hours > 0)
			return hours.ToString() + "h " + minutes.ToString() + "m";
		return minutes.ToString() + "m";
	}

	static string GenerateID()
	{
		int timestamp = 0;
		if (g_Game)
			timestamp = DME_AH_Util.GetTimestamp();
		int random = Math.RandomInt(10000, 99999);
		return timestamp.ToString() + "_" + random.ToString();
	}
}

class DME_AH_ListingArray
{
	ref array<ref DME_AH_Listing> Listings;

	void DME_AH_ListingArray()
	{
		Listings = new array<ref DME_AH_Listing>;
	}
}
