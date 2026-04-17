// DME Auction House - Transaction Data Class
// JSON-serializable: NO prefix on member variables

class DME_AH_Transaction
{
	string TransactionID;
	string ListingID;
	string SellerUID;
	string BuyerUID;
	string SellerName;
	string BuyerName;
	string ItemClassName;
	string ItemDisplayName;
	int FinalPrice;
	int Fee;
	int Timestamp;
	int Type;

	void DME_AH_Transaction()
	{
		TransactionID = "";
		ListingID = "";
		SellerUID = "";
		BuyerUID = "";
		SellerName = "";
		BuyerName = "";
		ItemClassName = "";
		ItemDisplayName = "";
		FinalPrice = 0;
		Fee = 0;
		Timestamp = 0;
		Type = 0;
	}

	static string GenerateID()
	{
		int timestamp = 0;
		if (g_Game)
			timestamp = DME_AH_Util.GetTimestamp();
		int random = Math.RandomInt(10000, 99999);
		return "TXN_" + timestamp.ToString() + "_" + random.ToString();
	}
}

class DME_AH_TransactionArray
{
	ref array<ref DME_AH_Transaction> Transactions;

	void DME_AH_TransactionArray()
	{
		Transactions = new array<ref DME_AH_Transaction>;
	}
}
