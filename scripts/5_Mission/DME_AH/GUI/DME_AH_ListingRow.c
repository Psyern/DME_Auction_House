// DME Auction House - Listing Row Data (for listbox entries)

class DME_AH_ListingRow
{
	string ListingID;
	string ItemName;
	int Price;
	int ListingType;
	string SellerName;
	int ExpiresTimestamp;
	int BuyNowPrice;
	int BidCount;
	string CurrentBidderName;

	void DME_AH_ListingRow()
	{
		ListingID = "";
		ItemName = "";
		Price = 0;
		ListingType = 0;
		SellerName = "";
		ExpiresTimestamp = 0;
		BuyNowPrice = 0;
		BidCount = 0;
		CurrentBidderName = "";
	}

	string GetTypeString()
	{
		if (ListingType == EDME_AH_ListingType.BuyNow)
			return "Buy Now";
		if (ListingType == EDME_AH_ListingType.Auction)
			return "Auction";
		if (ListingType == EDME_AH_ListingType.AuctionWithBuyNow)
			return "Auction+BN";
		return "Unknown";
	}

	string GetTimeRemainingString()
	{
		int currentTime = CF_Date.Now(true).GetTimestamp();
		int remaining = ExpiresTimestamp - currentTime;
		if (remaining <= 0)
			return "Expired";
		int hours = remaining / 3600;
		int minutes = (remaining % 3600) / 60;
		if (hours > 0)
			return hours.ToString() + "h " + minutes.ToString() + "m";
		return minutes.ToString() + "m";
	}

	static DME_AH_ListingRow FromListing(DME_AH_Listing listing)
	{
		if (!listing)
			return null;

		DME_AH_ListingRow row = new DME_AH_ListingRow();
		row.ListingID = listing.ListingID;
		row.ItemName = listing.ItemDisplayName;
		row.Price = listing.GetCurrentPrice();
		row.ListingType = listing.ListingType;
		row.SellerName = listing.SellerName;
		row.ExpiresTimestamp = listing.ExpiresTimestamp;
		row.BuyNowPrice = listing.BuyNowPrice;
		row.BidCount = listing.BidCount;
		row.CurrentBidderName = listing.CurrentBidderName;
		return row;
	}
}
