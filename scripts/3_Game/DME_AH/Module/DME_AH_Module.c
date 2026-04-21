// DME Auction House - Main Module (Standalone Singleton)
// No CF compile-time dependency - uses native DayZ patterns

class DME_AH_Module
{
	protected ref DME_AH_Config m_Config;
	protected ref DME_AH_CategoryConfig m_CategoryConfig;
	protected ref DME_AH_NPCConfig m_NPCConfig;
	protected ref DME_AH_DataStore m_DataStore;
	protected ref DME_AH_AuctionManager m_AuctionManager;
	protected ref DME_AH_CurrencyAdapter m_CurrencyAdapter;
	protected ref DME_AH_RPCHandler m_RPCHandler;
	protected bool m_Initialized;

	static ref DME_AH_Module s_Instance;

	static DME_AH_Module GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DME_AH_Module();
		return s_Instance;
	}

	void DME_AH_Module()
	{
		m_Initialized = false;
	}

	void Init()
	{
		if (m_Initialized)
			return;

		m_Initialized = true;
		s_Instance = this;

		bool isServer = false;
		if (g_Game)
			isServer = g_Game.IsDedicatedServer();

		DME_AH_Logger.Info("DME Auction House v" + DME_AH_VERSION + " initializing...");

		// Server: full init with file I/O
		if (isServer)
		{
			m_Config = DME_AH_Config.Load();
			DME_AH_Logger.SetLogLevel(m_Config.DebugLogLevel);
			DME_AH_Logger.Info("Config loaded (CurrencyType: " + m_Config.CurrencyType + ")");

			m_CategoryConfig = DME_AH_CategoryConfig.Load();
			DME_AH_Logger.Info("Categories loaded (" + m_CategoryConfig.Categories.Count().ToString() + " categories)");

			m_NPCConfig = DME_AH_NPCConfig.Load();
			DME_AH_Logger.Info("NPC config loaded (" + m_NPCConfig.NPCs.Count().ToString() + " NPCs defined)");

			m_DataStore = new DME_AH_DataStore();
			m_DataStore.SetAutoSaveInterval(m_Config.AutoSaveIntervalSeconds);

			InitCurrencyAdapter();

			m_DataStore.LoadAll();

			m_AuctionManager = new DME_AH_AuctionManager();
			m_AuctionManager.Init(m_Config, m_CategoryConfig, m_DataStore, m_CurrencyAdapter);

			m_RPCHandler = new DME_AH_RPCHandler();
			m_RPCHandler.Init(m_AuctionManager, m_Config, m_DataStore, m_CurrencyAdapter, m_CategoryConfig);

			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OnUpdate, 1000, true);
		}
		else
		{
			// Client: only load NPC config for action detection (no file creation)
			m_Config = new DME_AH_Config();
			m_NPCConfig = new DME_AH_NPCConfig();

			if (FileExist(DME_AH_SETTINGS_FILE))
				JsonFileLoader<DME_AH_Config>.JsonLoadFile(DME_AH_SETTINGS_FILE, m_Config);

			if (FileExist(DME_AH_NPC_FILE))
				JsonFileLoader<DME_AH_NPCConfig>.JsonLoadFile(DME_AH_NPC_FILE, m_NPCConfig);
		}

		string sideStr = "client";
		if (isServer)
			sideStr = "server";
		DME_AH_Logger.Info("DME Auction House initialized successfully (" + sideStr + ")");
	}

	protected void InitCurrencyAdapter()
	{
		EDME_AH_CurrencyType currencyType = m_Config.GetCurrencyTypeEnum();

		if (currencyType == EDME_AH_CurrencyType.Item)
		{
			DME_AH_CurrencyItem itemAdapter = new DME_AH_CurrencyItem();
			itemAdapter.SetItemClassName(m_Config.CurrencyItemClass);
			m_CurrencyAdapter = itemAdapter;
			DME_AH_Logger.Info("Currency adapter: Item (" + m_Config.CurrencyItemClass + ")");
		}
		else if (currencyType == EDME_AH_CurrencyType.Internal)
		{
			DME_AH_CurrencyInternal internalAdapter = new DME_AH_CurrencyInternal();
			internalAdapter.SetDataStore(m_DataStore);
			m_CurrencyAdapter = internalAdapter;
			DME_AH_Logger.Info("Currency adapter: Internal");
		}
		else
		{
			// DME_AH_CurrencyExpansion lives in 4_World (it references
			// ExpansionMarketModule types). We cannot instantiate it directly
			// from 3_Game; a modded_class hook in 4_World provides it.
			m_CurrencyAdapter = CreateExpansionCurrencyAdapter();
			if (!m_CurrencyAdapter)
			{
				DME_AH_Logger.Warning("Currency adapter: Expansion factory returned null, falling back to internal");
				DME_AH_CurrencyInternal fallback = new DME_AH_CurrencyInternal();
				fallback.SetDataStore(m_DataStore);
				m_CurrencyAdapter = fallback;
			}
			else
			{
				DME_AH_Logger.Info("Currency adapter: Expansion");
			}
		}
	}

	// Factory hook. Overridden by modded class in 4_World to construct
	// DME_AH_CurrencyExpansion (which cannot be referenced from 3_Game).
	protected DME_AH_CurrencyAdapter CreateExpansionCurrencyAdapter()
	{
		return null;
	}

	void OnUpdate()
	{
		if (!g_Game)
			return;

		if (g_Game.IsDedicatedServer())
		{
			if (m_AuctionManager)
				m_AuctionManager.Update(1.0);
			if (m_DataStore)
				m_DataStore.Update(1.0);
		}
	}

	void OnMissionFinish()
	{
		if (g_Game && g_Game.IsDedicatedServer() && m_DataStore)
		{
			m_DataStore.SaveAll();
			DME_AH_Logger.Info("Data saved on mission finish");
		}
	}

	// Public accessors
	DME_AH_Config GetConfig()
	{
		return m_Config;
	}

	DME_AH_CategoryConfig GetCategoryConfig()
	{
		return m_CategoryConfig;
	}

	DME_AH_NPCConfig GetNPCConfig()
	{
		return m_NPCConfig;
	}

	DME_AH_DataStore GetDataStore()
	{
		return m_DataStore;
	}

	DME_AH_AuctionManager GetAuctionManager()
	{
		return m_AuctionManager;
	}

	DME_AH_CurrencyAdapter GetCurrencyAdapter()
	{
		return m_CurrencyAdapter;
	}

	DME_AH_RPCHandler GetRPCHandler()
	{
		return m_RPCHandler;
	}
}
