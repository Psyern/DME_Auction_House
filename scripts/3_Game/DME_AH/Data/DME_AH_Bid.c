// DME Auction House - Bid Data Class
// JSON-serializable: NO prefix on member variables

class DME_AH_Bid
{
	string BidID;
	string ListingID;
	string BidderUID;
	string BidderName;
	int Amount;
	int Timestamp;

	void DME_AH_Bid()
	{
		BidID = "";
		ListingID = "";
		BidderUID = "";
		BidderName = "";
		Amount = 0;
		Timestamp = 0;
	}

	static string GenerateID()
	{
		int timestamp = 0;
		if (g_Game)
			timestamp = CF_Date.Now(true).GetTimestamp();
		int random = Math.RandomInt(10000, 99999);
		return "BID_" + timestamp.ToString() + "_" + random.ToString();
	}
}

class DME_AH_BidArray
{
	ref array<ref DME_AH_Bid> Bids;

	void DME_AH_BidArray()
	{
		Bids = new array<ref DME_AH_Bid>;
	}
}
