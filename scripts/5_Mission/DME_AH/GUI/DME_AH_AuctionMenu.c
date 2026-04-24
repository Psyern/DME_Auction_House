// DME Auction House - Main Auction Menu (UIScriptedMenu)

class DME_AH_AuctionMenuListener : DME_AH_RPCListener
{
	ref DME_AH_AuctionMenu m_Menu;

	void DME_AH_AuctionMenuListener(DME_AH_AuctionMenu menu)
	{
		m_Menu = menu;
	}

	override void OnReceiveListings(string data, int totalPages, int page)
	{
		if (m_Menu)
			m_Menu.OnReceiveListings(data, totalPages, page);
	}

	override void OnReceiveBalance(int balance)
	{
		if (m_Menu)
			m_Menu.OnReceiveBalance(balance);
	}

	override void OnReceiveCallback(int resultCode, string data)
	{
		if (m_Menu)
			m_Menu.OnReceiveCallback(resultCode, data);
	}
}

class DME_AH_AuctionMenu : UIScriptedMenu
{
	protected ref DME_AH_AuctionMenuListener m_RPCListener;
	// Tab buttons
	protected ButtonWidget m_BtnTabMarketplace;
	protected ButtonWidget m_BtnTabMyListings;
	protected ButtonWidget m_BtnTabMyBids;
	protected ButtonWidget m_BtnSellItem;
	// Filter bar
	protected EditBoxWidget m_EditSearch;
	protected XComboBoxWidget m_ComboCategory;
	protected ButtonWidget m_BtnSortPrice;
	protected ButtonWidget m_BtnSortName;
	protected ButtonWidget m_BtnSortTime;

	// Listings
	protected TextListboxWidget m_LstListings;
	protected TextWidget m_TxtBalance;
	protected TextWidget m_TxtPageInfo;
	protected ButtonWidget m_BtnPrevPage;
	protected ButtonWidget m_BtnNextPage;
	protected ButtonWidget m_BtnClose;

	// Detail panel
	protected ref DME_AH_DetailPanel m_DetailPanel;
	protected Widget m_DetailPanelRoot;
	protected ButtonWidget m_BtnBuyNow;
	protected ButtonWidget m_BtnPlaceBid;
	protected ButtonWidget m_BtnCancelListing;
	protected ButtonWidget m_BtnCloseDetail;

	// Loading
	protected Widget m_LoadingOverlay;

	// State
	protected int m_CurrentTab;
	protected int m_CurrentPage;
	protected int m_TotalPages;
	protected int m_CurrentSortMode;
	protected int m_PlayerBalance;
	protected ref array<ref DME_AH_ListingRow> m_ListingRows;
	// Maps category-combo index -> categoryID (index 0 == 0 == All)
	protected ref array<int> m_CategoryComboIDs;

	void DME_AH_AuctionMenu()
	{
		m_CurrentTab = EDME_AH_MenuTab.Marketplace;
		m_CurrentPage = 0;
		m_TotalPages = 1;
		m_CurrentSortMode = EDME_AH_SortMode.NewestFirst;
		m_PlayerBalance = 0;
		m_ListingRows = new array<ref DME_AH_ListingRow>;
		m_CategoryComboIDs = new array<int>;
	}

	override Widget Init()
	{
		if (!g_Game)
			return null;

		layoutRoot = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu.layout");
		if (!layoutRoot)
			return null;

		// Tab buttons
		m_BtnTabMarketplace = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnTabMarketplace"));
		m_BtnTabMyListings = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnTabMyListings"));
		m_BtnTabMyBids = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnTabMyBids"));
		m_BtnSellItem = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnSellItem"));

		// Filter bar
		m_EditSearch = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("editSearch"));
		m_ComboCategory = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("comboCategory"));
		m_BtnSortPrice = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnSortPrice"));
		m_BtnSortName = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnSortName"));
		m_BtnSortTime = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnSortTime"));

		// Listings
		m_LstListings = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("lstListings"));
		m_TxtBalance = TextWidget.Cast(layoutRoot.FindAnyWidget("txtBalance"));
		m_TxtPageInfo = TextWidget.Cast(layoutRoot.FindAnyWidget("txtPageInfo"));
		m_BtnPrevPage = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnPrevPage"));
		m_BtnNextPage = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnNextPage"));
		m_BtnClose = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnClose"));

		// Detail panel
		m_DetailPanelRoot = layoutRoot.FindAnyWidget("DetailPanel");
		m_DetailPanel = new DME_AH_DetailPanel();
		m_DetailPanel.Init(m_DetailPanelRoot);
		m_BtnBuyNow = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnBuyNow"));
		m_BtnPlaceBid = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnPlaceBid"));
		m_BtnCancelListing = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnCancelListing"));
		m_BtnCloseDetail = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnCloseDetail"));

		// Loading
		m_LoadingOverlay = layoutRoot.FindAnyWidget("LoadingOverlay");

		PopulateCategoryCombo();
		UpdateTabHighlight();

		m_RPCListener = new DME_AH_AuctionMenuListener(this);
		DME_AH_RPCQueue.SetListener(m_RPCListener);

		RequestListings();
		RequestBalance();

		return layoutRoot;
	}

	override void OnHide()
	{
		super.OnHide();
		// NOTE: do NOT Close() here. OnHide fires every time a child menu
		// (e.g. the sell dialog) is pushed on top of us. Closing on hide
		// would destroy the parent marketplace menu the moment the user
		// clicks SELL ITEM. Cleanup of hud/input is in the destructor.
	}

	override bool UseKeyboard()
	{
		return true;
	}

	override bool UseMouse()
	{
		return true;
	}

	// ESC → close the whole marketplace. Without this override the default
	// ingame ESC handler would pop up the pause menu behind us.
	override bool OnKeyPress(Widget w, int x, int y, int key)
	{
		super.OnKeyPress(w, x, y, key);
		if (key == KeyCode.KC_ESCAPE)
		{
			Close();
			return true;
		}
		return false;
	}

	// Actual cleanup — called by the UIManager when the menu is being removed
	// (user pressed close, ESC, or Close() was called explicitly).
	override void OnShow()
	{
		super.OnShow();

		if (!g_Game)
			return;
		if (!layoutRoot)
			return;

		PPEffects.SetBlurMenu(0.5);
		g_Game.GetInput().ChangeGameFocus(1);
		g_Game.GetUIManager().ShowUICursor(true);
		g_Game.GetMission().GetHud().Show(false);
	}

	void ~DME_AH_AuctionMenu()
	{
		if (!g_Game)
			return;

		PPEffects.SetBlurMenu(0);
		g_Game.GetInput().ResetGameFocus();
		g_Game.GetUIManager().ShowUICursor(false);
		if (g_Game.GetMission() && g_Game.GetMission().GetHud())
			g_Game.GetMission().GetHud().Show(true);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		// Close
		if (w == m_BtnClose)
		{
			Close();
			return true;
		}

		// Tabs
		if (w == m_BtnTabMarketplace)
		{
			SwitchTab(EDME_AH_MenuTab.Marketplace);
			return true;
		}
		if (w == m_BtnTabMyListings)
		{
			SwitchTab(EDME_AH_MenuTab.MyListings);
			return true;
		}
		if (w == m_BtnTabMyBids)
		{
			SwitchTab(EDME_AH_MenuTab.MyBids);
			return true;
		}

		// Header "SELL ITEM" button — opens the dedicated sell layout on top.
		if (w == m_BtnSellItem)
		{
			OpenSellMenu();
			return true;
		}

		// Sort buttons
		if (w == m_BtnSortPrice)
		{
			ToggleSortMode(EDME_AH_SortMode.PriceAsc, EDME_AH_SortMode.PriceDesc);
			return true;
		}
		if (w == m_BtnSortName)
		{
			ToggleSortMode(EDME_AH_SortMode.NameAsc, EDME_AH_SortMode.NameDesc);
			return true;
		}
		if (w == m_BtnSortTime)
		{
			ToggleSortMode(EDME_AH_SortMode.TimeAsc, EDME_AH_SortMode.TimeDesc);
			return true;
		}

		// Pagination
		if (w == m_BtnPrevPage)
		{
			if (m_CurrentPage > 0)
			{
				m_CurrentPage = m_CurrentPage - 1;
				RequestListings();
			}
			return true;
		}
		if (w == m_BtnNextPage)
		{
			if (m_CurrentPage < m_TotalPages - 1)
			{
				m_CurrentPage = m_CurrentPage + 1;
				RequestListings();
			}
			return true;
		}

		// Listing selection
		if (w == m_LstListings)
		{
			OnListingSelected();
			return true;
		}

		// Detail panel buttons
		if (w == m_BtnBuyNow)
		{
			OnBuyNowClick();
			return true;
		}
		if (w == m_BtnPlaceBid)
		{
			OnPlaceBidClick();
			return true;
		}
		if (w == m_BtnCancelListing)
		{
			OnCancelListingClick();
			return true;
		}
		if (w == m_BtnCloseDetail)
		{
			if (m_DetailPanel)
				m_DetailPanel.Hide();
			return true;
		}

		return false;
	}

	// --- Tab Management ---
	protected void SwitchTab(int tab)
	{
		m_CurrentTab = tab;
		m_CurrentPage = 0;
		if (m_DetailPanel)
			m_DetailPanel.Hide();
		UpdateTabHighlight();

		if (tab == EDME_AH_MenuTab.Marketplace)
			RequestListings();
		else if (tab == EDME_AH_MenuTab.MyListings)
			RequestMyListings();
		else if (tab == EDME_AH_MenuTab.MyBids)
			RequestMyBids();
	}

	protected void UpdateTabHighlight()
	{
		int activeColor = ARGB(255, 255, 128, 0);
		int inactiveColor = ARGB(255, 153, 153, 153);

		if (m_BtnTabMarketplace)
		{
			int mpColor = inactiveColor;
			if (m_CurrentTab == EDME_AH_MenuTab.Marketplace)
				mpColor = activeColor;
			m_BtnTabMarketplace.SetTextColor(mpColor);
		}
		if (m_BtnTabMyListings)
		{
			int mlColor = inactiveColor;
			if (m_CurrentTab == EDME_AH_MenuTab.MyListings)
				mlColor = activeColor;
			m_BtnTabMyListings.SetTextColor(mlColor);
		}
		if (m_BtnTabMyBids)
		{
			int mbColor = inactiveColor;
			if (m_CurrentTab == EDME_AH_MenuTab.MyBids)
				mbColor = activeColor;
			m_BtnTabMyBids.SetTextColor(mbColor);
		}
	}

	// --- Sorting ---
	protected void ToggleSortMode(int ascMode, int descMode)
	{
		if (m_CurrentSortMode == ascMode)
			m_CurrentSortMode = descMode;
		else
			m_CurrentSortMode = ascMode;
		m_CurrentPage = 0;
		RequestListings();
	}

	// --- Category Combo ---
	protected void PopulateCategoryCombo()
	{
		if (!m_ComboCategory)
			return;

		m_CategoryComboIDs.Clear();

		// Always keep "All Categories" at index 0 (category ID 0 == no filter)
		m_ComboCategory.AddItem("All Categories");
		m_CategoryComboIDs.Insert(0);

		DME_AH_CategoryConfig catConfig;
		DME_AH_Module module = DME_AH_Module.GetInstance();
		if (module)
			catConfig = module.GetCategoryConfig();

		// Client may not have a config loaded — fall back to defaults so the
		// combo still reflects the real category IDs.
		if (!catConfig || !catConfig.Categories || catConfig.Categories.Count() == 0)
		{
			catConfig = new DME_AH_CategoryConfig();
			catConfig.CreateDefaults();
		}

		for (int i = 0; i < catConfig.Categories.Count(); i++)
		{
			DME_AH_Category cat = catConfig.Categories[i];
			if (!cat)
				continue;
			m_ComboCategory.AddItem(cat.DisplayName);
			m_CategoryComboIDs.Insert(cat.CategoryID);
		}

		m_ComboCategory.SetCurrentItem(0);
	}

	protected int GetSelectedCategoryID()
	{
		if (!m_ComboCategory || !m_CategoryComboIDs)
			return 0;
		int idx = m_ComboCategory.GetCurrentItem();
		if (idx < 0 || idx >= m_CategoryComboIDs.Count())
			return 0;
		return m_CategoryComboIDs[idx];
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		super.OnChange(w, x, y, finished);

		if (w == m_ComboCategory)
		{
			m_CurrentPage = 0;
			RequestListings();
			return true;
		}
		return false;
	}

	// --- Listing Selection ---
	protected void OnListingSelected()
	{
		if (!m_LstListings || !m_DetailPanel)
			return;

		int selectedRow = m_LstListings.GetSelectedRow();
		if (selectedRow < 0 || selectedRow >= m_ListingRows.Count())
			return;

		DME_AH_ListingRow row = m_ListingRows[selectedRow];
		if (!row)
			return;

		string playerUID = "";
		if (g_Game)
		{
			PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
			if (player)
			{
				PlayerIdentity identity = player.GetIdentity();
				if (identity)
					playerUID = identity.GetPlainId();
			}
		}

		bool isOwn = false;
		m_DetailPanel.Show(row, isOwn);
	}

	// --- Action Buttons ---
	protected void OnBuyNowClick()
	{
		if (!m_DetailPanel)
			return;
		DME_AH_ListingRow row = m_DetailPanel.GetCurrentRow();
		if (!row)
			return;

		SendBuyNow(row.ListingID);
	}

	protected void OnPlaceBidClick()
	{
		if (!m_DetailPanel)
			return;
		DME_AH_ListingRow row = m_DetailPanel.GetCurrentRow();
		if (!row)
			return;

		string bidText = m_DetailPanel.GetBidAmountText();
		int bidAmount = bidText.ToInt();
		if (bidAmount <= 0)
			return;

		SendPlaceBid(row.ListingID, bidAmount);
	}

	protected void OnCancelListingClick()
	{
		if (!m_DetailPanel)
			return;
		DME_AH_ListingRow row = m_DetailPanel.GetCurrentRow();
		if (!row)
			return;

		SendCancelListing(row.ListingID);
	}

	// Opens the full-screen sell menu as a separate scripted menu on top of this one.
	// Inventory list + preview + price/duration fields all live in that menu's own layout.
	protected void OpenSellMenu()
	{
		if (!g_Game)
			return;
		UIManager uiManager = g_Game.GetUIManager();
		if (!uiManager)
			return;

		UIScriptedMenu menu = uiManager.EnterScriptedMenu(MENU_DME_AH_CREATE_LISTING, null);
		DME_AH_CreateListingDialog dialog;
		if (Class.CastTo(dialog, menu))
			dialog.SetParentMenu(this);
	}

	// --- RPC Sending (Native ScriptRPC) ---
	void RequestListings()
	{
		if (!g_Game)
			return;

		int categoryID = GetSelectedCategoryID();

		string searchText = "";
		if (m_EditSearch)
			searchText = m_EditSearch.GetText();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(categoryID);
		rpc.Write(searchText);
		rpc.Write(m_CurrentSortMode);
		rpc.Write(m_CurrentPage);
		rpc.Send(g_Game.GetPlayer(), EDME_AH_RPC.DME_AH_RPC_REQUEST_LISTINGS, true, null);
	}

	void RequestMyListings()
	{
		if (!g_Game)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(0);
		rpc.Send(g_Game.GetPlayer(), EDME_AH_RPC.DME_AH_RPC_REQUEST_MY_LISTINGS, true, null);
	}

	void RequestMyBids()
	{
		if (!g_Game)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(0);
		rpc.Send(g_Game.GetPlayer(), EDME_AH_RPC.DME_AH_RPC_REQUEST_MY_BIDS, true, null);
	}

	void RequestBalance()
	{
		if (!g_Game)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(0);
		rpc.Send(g_Game.GetPlayer(), EDME_AH_RPC.DME_AH_RPC_REQUEST_BALANCE, true, null);
	}

	void SendCreateListing(string itemClassName, int listingType, int startPrice, int buyNowPrice, int durationMinutes, int categoryID, int networkLow, int networkHigh)
	{
		if (!g_Game)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(itemClassName);
		rpc.Write(listingType);
		rpc.Write(startPrice);
		rpc.Write(buyNowPrice);
		rpc.Write(durationMinutes);
		rpc.Write(categoryID);
		rpc.Write(networkLow);
		rpc.Write(networkHigh);
		rpc.Send(g_Game.GetPlayer(), EDME_AH_RPC.DME_AH_RPC_CREATE_LISTING, true, null);
	}

	void SendBuyNow(string listingID)
	{
		if (!g_Game)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(listingID);
		rpc.Send(g_Game.GetPlayer(), EDME_AH_RPC.DME_AH_RPC_BUY_NOW, true, null);
	}

	void SendPlaceBid(string listingID, int bidAmount)
	{
		if (!g_Game)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(listingID);
		rpc.Write(bidAmount);
		rpc.Send(g_Game.GetPlayer(), EDME_AH_RPC.DME_AH_RPC_PLACE_BID, true, null);
	}

	void SendCancelListing(string listingID)
	{
		if (!g_Game)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(listingID);
		rpc.Send(g_Game.GetPlayer(), EDME_AH_RPC.DME_AH_RPC_CANCEL_LISTING, true, null);
	}

	// --- RPC Response Handlers ---
	void OnReceiveListings(string listingsData, int totalPages, int page)
	{
		m_TotalPages = totalPages;
		m_CurrentPage = page;
		m_ListingRows.Clear();

		if (m_LstListings)
			m_LstListings.ClearItems();

		if (listingsData == "")
			return;

		array<string> rows = new array<string>;
		listingsData.Split("|", rows);

		for (int i = 0; i < rows.Count(); i++)
		{
			string rowStr = rows[i];
			if (rowStr == "")
				continue;

			array<string> fields = new array<string>;
			rowStr.Split(";", fields);
			if (fields.Count() < 13)
				continue;

			DME_AH_ListingRow row = new DME_AH_ListingRow();
			row.ListingID = fields[0];
			row.SellerName = fields[1];
			row.ItemClassName = fields[2];
			row.ItemName = fields[3];
			row.ListingType = fields[5].ToInt();
			row.Price = fields[6].ToInt();
			row.BuyNowPrice = fields[7].ToInt();
			int currentBid = fields[8].ToInt();
			if (currentBid > 0)
				row.Price = currentBid;
			row.CurrentBidderName = fields[9];
			row.BidCount = fields[10].ToInt();
			row.ExpiresTimestamp = fields[11].ToInt();

			m_ListingRows.Insert(row);

			if (m_LstListings)
			{
				string displayStr = row.ItemName + "    " + row.Price.ToString() + "    " + row.GetTypeString() + "    " + row.SellerName + "    " + row.GetTimeRemainingString();
				m_LstListings.AddItem(displayStr, null, 0);
			}
		}

		UpdatePageInfo();
	}

	void OnReceiveBalance(int balance)
	{
		m_PlayerBalance = balance;
		if (m_TxtBalance)
			m_TxtBalance.SetText("Balance: " + balance.ToString());
	}

	int GetPlayerBalance()
	{
		return m_PlayerBalance;
	}

	void OnReceiveCallback(int resultCode, string data)
	{
		if (resultCode == EDME_AH_ResultCode.Success)
		{
			DME_AH_NotificationHandler.ShowNotification("Success", "Action completed successfully");
			RequestListings();
			RequestBalance();
			if (m_DetailPanel)
				m_DetailPanel.Hide();
		}
		else
		{
			string errorMsg = GetErrorMessage(resultCode);
			DME_AH_NotificationHandler.ShowNotification("Error", errorMsg);
		}
	}

	void OnConfirmAction(string action, string data)
	{
		if (action == "buynow")
			SendBuyNow(data);
		else if (action == "cancel")
			SendCancelListing(data);
	}

	// --- Helpers ---
	protected void UpdatePageInfo()
	{
		if (!m_TxtPageInfo)
			return;

		int displayPage = m_CurrentPage + 1;
		int displayTotal = m_TotalPages;
		if (displayTotal < 1)
			displayTotal = 1;

		m_TxtPageInfo.SetText("Page " + displayPage.ToString() + "/" + displayTotal.ToString());
	}

	protected string GetErrorMessage(int resultCode)
	{
		if (resultCode == EDME_AH_ResultCode.FailedNotEnoughMoney)
			return "Not enough money";
		if (resultCode == EDME_AH_ResultCode.FailedListingNotFound)
			return "Listing not found";
		if (resultCode == EDME_AH_ResultCode.FailedListingExpired)
			return "Listing has expired";
		if (resultCode == EDME_AH_ResultCode.FailedBidTooLow)
			return "Bid too low";
		if (resultCode == EDME_AH_ResultCode.FailedMaxListingsReached)
			return "Max listings reached";
		if (resultCode == EDME_AH_ResultCode.FailedMaxBidsReached)
			return "Max bids reached";
		if (resultCode == EDME_AH_ResultCode.FailedItemNotInInventory)
			return "Item not in inventory";
		if (resultCode == EDME_AH_ResultCode.FailedItemHasAttachments)
			return "Remove attachments and cargo before listing";
		if (resultCode == EDME_AH_ResultCode.FailedCannotCancelWithBids)
			return "Cannot cancel with active bids";
		if (resultCode == EDME_AH_ResultCode.FailedOwnListing)
			return "Cannot buy your own listing";
		if (resultCode == EDME_AH_ResultCode.FailedInvalidPrice)
			return "Invalid price";
		if (resultCode == EDME_AH_ResultCode.FailedServerError)
			return "Server error";
		return "Unknown error";
	}
}
