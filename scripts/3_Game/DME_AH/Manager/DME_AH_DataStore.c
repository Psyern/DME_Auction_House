// DME Auction House - Data Persistence

class DME_AH_PendingPickup
{
	string PendingID;
	string PlayerUID;
	string ItemClassName;
	int Amount;
	int Type;
	int Timestamp;
	// Item snapshot (used when IsItem == true)
	bool IsItem;
	float ItemHealth;
	float ItemQuantity;
	int ItemLiquidType;

	void DME_AH_PendingPickup()
	{
		PendingID = "";
		PlayerUID = "";
		ItemClassName = "";
		Amount = 0;
		Type = 0;
		Timestamp = 0;
		IsItem = false;
		ItemHealth = -1;
		ItemQuantity = -1;
		ItemLiquidType = 0;
	}
}

class DME_AH_PlayerData
{
	ref map<string, int> Balances;
	ref array<ref DME_AH_PendingPickup> PendingPickups;

	void DME_AH_PlayerData()
	{
		Balances = new map<string, int>;
		PendingPickups = new array<ref DME_AH_PendingPickup>;
	}
}

class DME_AH_DataStore
{
	ref DME_AH_ListingArray m_ActiveListings;
	ref DME_AH_TransactionArray m_CompletedListings;
	ref DME_AH_PlayerData m_PlayerData;
	protected float m_TimeSinceLastSave;
	protected float m_AutoSaveInterval;

	void DME_AH_DataStore()
	{
		m_ActiveListings = new DME_AH_ListingArray();
		m_CompletedListings = new DME_AH_TransactionArray();
		m_PlayerData = new DME_AH_PlayerData();
		m_TimeSinceLastSave = 0;
		m_AutoSaveInterval = 300;
	}

	void SetAutoSaveInterval(int seconds)
	{
		m_AutoSaveInterval = seconds;
	}

	void LoadAll()
	{
		DME_AH_Config.MakeDirectoryIfNotExists();
		LoadActiveListings();
		LoadCompletedListings();
		LoadPlayerData();
		DME_AH_Logger.Info("DataStore: All data loaded");
	}

	void SaveAll()
	{
		DME_AH_Config.MakeDirectoryIfNotExists();
		SaveActiveListings();
		SaveCompletedListings();
		SavePlayerData();
		m_TimeSinceLastSave = 0;
		DME_AH_Logger.Debug("DataStore: All data saved");
	}

	void Update(float deltaTime)
	{
		m_TimeSinceLastSave += deltaTime;
		if (m_TimeSinceLastSave >= m_AutoSaveInterval)
		{
			SaveAll();
		}
	}

	// Active Listings
	void LoadActiveListings()
	{
		if (FileExist(DME_AH_ACTIVE_LISTINGS_FILE))
		{
			JsonFileLoader<DME_AH_ListingArray>.JsonLoadFile(DME_AH_ACTIVE_LISTINGS_FILE, m_ActiveListings);
			DME_AH_Logger.Info("Loaded " + m_ActiveListings.Listings.Count().ToString() + " active listings");
		}
	}

	void SaveActiveListings()
	{
		JsonFileLoader<DME_AH_ListingArray>.JsonSaveFile(DME_AH_ACTIVE_LISTINGS_FILE, m_ActiveListings);
	}

	void AddListing(DME_AH_Listing listing)
	{
		if (!listing)
			return;
		m_ActiveListings.Listings.Insert(listing);
		SaveActiveListings();
	}

	void RemoveListing(string listingID)
	{
		for (int i = m_ActiveListings.Listings.Count() - 1; i >= 0; i--)
		{
			DME_AH_Listing listing = m_ActiveListings.Listings[i];
			if (listing && listing.ListingID == listingID)
			{
				m_ActiveListings.Listings.Remove(i);
				break;
			}
		}
		SaveActiveListings();
	}

	DME_AH_Listing GetListingByID(string listingID)
	{
		for (int i = 0; i < m_ActiveListings.Listings.Count(); i++)
		{
			DME_AH_Listing listing = m_ActiveListings.Listings[i];
			if (listing && listing.ListingID == listingID)
				return listing;
		}
		return null;
	}

	array<ref DME_AH_Listing> GetActiveListings()
	{
		return m_ActiveListings.Listings;
	}

	array<ref DME_AH_Listing> GetListingsBySeller(string sellerUID)
	{
		array<ref DME_AH_Listing> result = new array<ref DME_AH_Listing>;
		for (int i = 0; i < m_ActiveListings.Listings.Count(); i++)
		{
			DME_AH_Listing listing = m_ActiveListings.Listings[i];
			if (listing && listing.SellerUID == sellerUID)
				result.Insert(listing);
		}
		return result;
	}

	int GetActiveListingCountForPlayer(string playerUID)
	{
		int count = 0;
		for (int i = 0; i < m_ActiveListings.Listings.Count(); i++)
		{
			DME_AH_Listing listing = m_ActiveListings.Listings[i];
			if (listing && listing.SellerUID == playerUID && listing.IsActive())
				count++;
		}
		return count;
	}

	// Completed Listings / History
	void LoadCompletedListings()
	{
		if (FileExist(DME_AH_COMPLETED_LISTINGS_FILE))
		{
			JsonFileLoader<DME_AH_TransactionArray>.JsonLoadFile(DME_AH_COMPLETED_LISTINGS_FILE, m_CompletedListings);
			DME_AH_Logger.Info("Loaded " + m_CompletedListings.Transactions.Count().ToString() + " transactions");
		}
	}

	void SaveCompletedListings()
	{
		JsonFileLoader<DME_AH_TransactionArray>.JsonSaveFile(DME_AH_COMPLETED_LISTINGS_FILE, m_CompletedListings);
	}

	void AddTransaction(DME_AH_Transaction transaction)
	{
		if (!transaction)
			return;
		m_CompletedListings.Transactions.Insert(transaction);

		int maxEntries = 500;
		while (m_CompletedListings.Transactions.Count() > maxEntries)
		{
			m_CompletedListings.Transactions.Remove(0);
		}
		SaveCompletedListings();
	}

	array<ref DME_AH_Transaction> GetTransactionHistory()
	{
		return m_CompletedListings.Transactions;
	}

	// Player Data
	void LoadPlayerData()
	{
		if (FileExist(DME_AH_PLAYER_DATA_FILE))
		{
			JsonFileLoader<DME_AH_PlayerData>.JsonLoadFile(DME_AH_PLAYER_DATA_FILE, m_PlayerData);
			DME_AH_Logger.Info("Loaded player data");
		}
	}

	void SavePlayerData()
	{
		JsonFileLoader<DME_AH_PlayerData>.JsonSaveFile(DME_AH_PLAYER_DATA_FILE, m_PlayerData);
	}

	int GetInternalBalance(string playerUID)
	{
		if (!m_PlayerData.Balances.Contains(playerUID))
			return 0;
		return m_PlayerData.Balances.Get(playerUID);
	}

	void SetInternalBalance(string playerUID, int balance)
	{
		m_PlayerData.Balances.Set(playerUID, balance);
		SavePlayerData();
	}

	void AddPendingPickup(DME_AH_PendingPickup pickup)
	{
		if (!pickup)
			return;
		m_PlayerData.PendingPickups.Insert(pickup);
		SavePlayerData();
	}

	array<ref DME_AH_PendingPickup> GetPendingPickupsForPlayer(string playerUID)
	{
		array<ref DME_AH_PendingPickup> result = new array<ref DME_AH_PendingPickup>;
		for (int i = 0; i < m_PlayerData.PendingPickups.Count(); i++)
		{
			DME_AH_PendingPickup pickup = m_PlayerData.PendingPickups[i];
			if (pickup && pickup.PlayerUID == playerUID)
				result.Insert(pickup);
		}
		return result;
	}

	void RemovePendingPickup(string pendingID)
	{
		for (int i = m_PlayerData.PendingPickups.Count() - 1; i >= 0; i--)
		{
			DME_AH_PendingPickup pickup = m_PlayerData.PendingPickups[i];
			if (pickup && pickup.PendingID == pendingID)
			{
				m_PlayerData.PendingPickups.Remove(i);
				break;
			}
		}
		SavePlayerData();
	}
}
