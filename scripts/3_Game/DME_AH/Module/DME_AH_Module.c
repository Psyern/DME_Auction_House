// DME Auction House - Main Module (CF_ModuleWorld)

class DME_AH_Module : CF_ModuleWorld
{
	protected ref DME_AH_Config m_Config;
	protected ref DME_AH_CategoryConfig m_CategoryConfig;
	protected ref DME_AH_DataStore m_DataStore;
	protected ref DME_AH_AuctionManager m_AuctionManager;
	protected ref DME_AH_CurrencyAdapter m_CurrencyAdapter;
	protected ref DME_AH_RPCHandler m_RPCHandler;

	static ref DME_AH_Module s_Instance;

	static DME_AH_Module GetInstance()
	{
		return s_Instance;
	}

	override void OnInit()
	{
		super.OnInit();
		s_Instance = this;

		DME_AH_Logger.Info("DME Auction House v" + DME_AH_VERSION + " initializing...");

		m_Config = DME_AH_Config.Load();
		DME_AH_Logger.SetLogLevel(m_Config.LogLevel);
		DME_AH_Logger.Info("Config loaded (CurrencyType: " + m_Config.CurrencyType + ")");

		m_CategoryConfig = DME_AH_CategoryConfig.Load();
		DME_AH_Logger.Info("Categories loaded (" + m_CategoryConfig.Categories.Count().ToString() + " categories)");

		m_DataStore = new DME_AH_DataStore();
		m_DataStore.SetAutoSaveInterval(m_Config.AutoSaveIntervalSeconds);

		InitCurrencyAdapter();

		if (g_Game && g_Game.IsDedicatedServer())
		{
			m_DataStore.LoadAll();
		}

		m_AuctionManager = new DME_AH_AuctionManager();
		m_AuctionManager.Init(m_Config, m_CategoryConfig, m_DataStore, m_CurrencyAdapter);

		m_RPCHandler = new DME_AH_RPCHandler();
		m_RPCHandler.Init(m_AuctionManager, m_Config, m_DataStore, m_CurrencyAdapter, m_CategoryConfig);
		m_RPCHandler.RegisterRPCs();

		EnableUpdate();
		DME_AH_Logger.Info("DME Auction House initialized successfully");
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
			m_CurrencyAdapter = new DME_AH_CurrencyExpansion();
			DME_AH_Logger.Info("Currency adapter: Expansion");
		}
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (!g_Game)
			return;

		if (g_Game.IsDedicatedServer())
		{
			if (m_AuctionManager)
				m_AuctionManager.Update(timeslice);
			if (m_DataStore)
				m_DataStore.Update(timeslice);
		}
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();
		DME_AH_Logger.Info("Mission started");
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

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
