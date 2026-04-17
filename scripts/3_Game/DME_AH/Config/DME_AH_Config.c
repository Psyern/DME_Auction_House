// DME Auction House - Server Configuration
// JSON-serializable: NO prefix on member variables

class DME_AH_Config
{
	bool Enabled = true;
	string CurrencyType = "Expansion";
	string CurrencyItemClass = "MoneyRuble100";
	int MaxListingsPerPlayer = 10;
	int MaxActiveBidsPerPlayer = 20;
	int ListingFeePercent = 5;
	int SaleFeePercent = 10;
	int MinPrice = 1;
	int MaxPrice = 1000000;
	ref array<int> AllowedDurations;
	int DefaultDuration = 1440;
	int MinBidIncrement = 1;
	int MinBidIncrementPercent = 5;
	int AutoSaveIntervalSeconds = 300;
	int MaxHistoryEntries = 500;
	bool NotificationsEnabled = true;
	int DebugLogLevel = 3;

	void DME_AH_Config()
	{
		AllowedDurations = new array<int>;
		AllowedDurations.Insert(360);
		AllowedDurations.Insert(720);
		AllowedDurations.Insert(1440);
		AllowedDurations.Insert(2880);
	}

	void Validate()
	{
		MaxListingsPerPlayer = Math.Clamp(MaxListingsPerPlayer, 1, 100);
		MaxActiveBidsPerPlayer = Math.Clamp(MaxActiveBidsPerPlayer, 1, 100);
		ListingFeePercent = Math.Clamp(ListingFeePercent, 0, 50);
		SaleFeePercent = Math.Clamp(SaleFeePercent, 0, 50);
		MinPrice = Math.Clamp(MinPrice, 1, 999999);
		MaxPrice = Math.Clamp(MaxPrice, MinPrice + 1, 99999999);
		DefaultDuration = Math.Clamp(DefaultDuration, 60, 10080);
		MinBidIncrement = Math.Clamp(MinBidIncrement, 1, 10000);
		MinBidIncrementPercent = Math.Clamp(MinBidIncrementPercent, 1, 50);
		AutoSaveIntervalSeconds = Math.Clamp(AutoSaveIntervalSeconds, 60, 3600);
		MaxHistoryEntries = Math.Clamp(MaxHistoryEntries, 10, 10000);
		DebugLogLevel = Math.Clamp(DebugLogLevel, 0, 4);

		if (!AllowedDurations || AllowedDurations.Count() == 0)
		{
			AllowedDurations = new array<int>;
			AllowedDurations.Insert(360);
			AllowedDurations.Insert(720);
			AllowedDurations.Insert(1440);
			AllowedDurations.Insert(2880);
		}

		if (CurrencyType != "Expansion" && CurrencyType != "Item" && CurrencyType != "Internal")
			CurrencyType = "Expansion";

		if (CurrencyType == "Item" && CurrencyItemClass == "")
			CurrencyItemClass = "MoneyRuble100";
	}

	EDME_AH_CurrencyType GetCurrencyTypeEnum()
	{
		if (CurrencyType == "Item")
			return EDME_AH_CurrencyType.Item;
		if (CurrencyType == "Internal")
			return EDME_AH_CurrencyType.Internal;
		return EDME_AH_CurrencyType.Expansion;
	}

	bool IsDurationAllowed(int durationMinutes)
	{
		if (!AllowedDurations)
			return false;

		for (int i = 0; i < AllowedDurations.Count(); i++)
		{
			if (AllowedDurations[i] == durationMinutes)
				return true;
		}
		return false;
	}

	int CalculateListingFee(int price)
	{
		if (ListingFeePercent <= 0)
			return 0;
		int fee = (price * ListingFeePercent) / 100;
		if (fee < 1)
			fee = 1;
		return fee;
	}

	int CalculateSaleFee(int price)
	{
		if (SaleFeePercent <= 0)
			return 0;
		int fee = (price * SaleFeePercent) / 100;
		if (fee < 1)
			fee = 1;
		return fee;
	}

	int GetMinBidForListing(DME_AH_Listing listing)
	{
		if (!listing)
			return 0;

		int currentPrice = listing.GetCurrentPrice();
		int percentIncrement = (currentPrice * MinBidIncrementPercent) / 100;
		int increment = MinBidIncrement;
		if (percentIncrement > increment)
			increment = percentIncrement;
		return currentPrice + increment;
	}

	static void MakeDirectoryIfNotExists()
	{
		if (!FileExist(DME_AH_ROOT_FOLDER))
			MakeDirectory(DME_AH_ROOT_FOLDER);
		if (!FileExist(DME_AH_CONFIG_DIR))
			MakeDirectory(DME_AH_CONFIG_DIR);
		if (!FileExist(DME_AH_DATA_DIR))
			MakeDirectory(DME_AH_DATA_DIR);
	}

	void Save()
	{
		MakeDirectoryIfNotExists();
		JsonFileLoader<DME_AH_Config>.JsonSaveFile(DME_AH_SETTINGS_FILE, this);
	}

	static ref DME_AH_Config Load()
	{
		MakeDirectoryIfNotExists();
		DME_AH_Config config = new DME_AH_Config();

		if (FileExist(DME_AH_SETTINGS_FILE))
		{
			JsonFileLoader<DME_AH_Config>.JsonLoadFile(DME_AH_SETTINGS_FILE, config);
			config.Validate();
			return config;
		}

		config.Save();
		return config;
	}
}
