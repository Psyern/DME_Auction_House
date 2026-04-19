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
	protected ButtonWidget m_BtnTabSellItem;
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

	void DME_AH_AuctionMenu()
	{
		m_CurrentTab = EDME_AH_MenuTab.Marketplace;
		m_CurrentPage = 0;
		m_TotalPages = 1;
		m_CurrentSortMode = EDME_AH_SortMode.NewestFirst;
		m_PlayerBalance = 0;
		m_ListingRows = new array<ref DME_AH_ListingRow>;
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
		m_BtnTabSellItem = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnTabSellItem"));

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

	override void OnHide()
	{
		super.OnHide();

		if (!g_Game)
			return;

		PPEffects.SetBlurMenu(0);
		g_Game.GetInput().ResetGameFocus();
		g_Game.GetUIManager().ShowUICursor(false);
		g_Game.GetMission().GetHud().Show(true);
		Close();
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
		if (w == m_BtnTabSellItem)
		{
			SwitchTab(EDME_AH_MenuTab.SellItem);
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
			if (m_CurrentTab == EDME_AH_MenuTab.SellItem)
				OnInventoryItemSelected();
			else
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
		else if (tab == EDME_AH_MenuTab.SellItem)
			PopulateInventoryList();
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
		if (m_BtnTabSellItem)
		{
			int siColor = inactiveColor;
			if (m_CurrentTab == EDME_AH_MenuTab.SellItem)
				siColor = activeColor;
			m_BtnTabSellItem.SetTextColor(siColor);
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

		m_ComboCategory.AddItem("All Categories");
		m_ComboCategory.AddItem("Weapons");
		m_ComboCategory.AddItem("Clothing");
		m_ComboCategory.AddItem("Medical");
		m_ComboCategory.AddItem("Food");
		m_ComboCategory.AddItem("Vehicles");
		m_ComboCategory.AddItem("Building");
		m_ComboCategory.AddItem("Other");
		m_ComboCategory.SetCurrentItem(0);
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

	// --- Sell Item Tab ---
	protected ref array<string> m_InventoryItemClasses;

	protected void PopulateInventoryList()
	{
		if (!m_LstListings)
			return;
		if (!g_Game)
			return;

		m_LstListings.ClearItems();
		m_ListingRows.Clear();

		if (!m_InventoryItemClasses)
			m_InventoryItemClasses = new array<string>;
		m_InventoryItemClasses.Clear();

		Man player = g_Game.GetPlayer();
		if (!player)
			return;

		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int i = 0; i < items.Count(); i++)
		{
			EntityAI item = items[i];
			if (!item)
				continue;
			if (item == player)
				continue;

			string displayName = item.GetDisplayName();
			if (displayName == "")
				displayName = item.GetType();

			string className = item.GetType();
			m_InventoryItemClasses.Insert(className);

			string displayStr = displayName + "    (" + className + ")";
			m_LstListings.AddItem(displayStr, null, 0);
		}

		UpdatePageInfo();
	}

	protected void OnInventoryItemSelected()
	{
		if (!m_LstListings || !m_InventoryItemClasses)
			return;

		int selectedRow = m_LstListings.GetSelectedRow();
		if (selectedRow < 0 || selectedRow >= m_InventoryItemClasses.Count())
			return;

		string selectedClass = m_InventoryItemClasses[selectedRow];
		DME_AH_Logger.Info("Selected inventory item: " + selectedClass);

		// For now, create a simple buy-now listing with a default price
		// TODO: Add price input in detail panel
		SendCreateListing(selectedClass, EDME_AH_ListingType.BuyNow, 100, 0, 1440, 7);
	}

	// --- RPC Sending (Native ScriptRPC) ---
	void RequestListings()
	{
		if (!g_Game)
			return;

		int categoryID = 0;
		if (m_ComboCategory)
			categoryID = m_ComboCategory.GetCurrentItem();

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

	void SendCreateListing(string itemClassName, int listingType, int startPrice, int buyNowPrice, int durationMinutes, int categoryID)
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
