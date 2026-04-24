// DME Auction House - Create Listing Dialog

class DME_AH_CreateListingDialog : UIScriptedMenu
{
	protected TextListboxWidget m_LstInventory;
	protected ItemPreviewWidget m_ItemPreview;
	protected TextWidget m_TxtSelectedItem;
	protected TextWidget m_TxtBalance;
	protected XComboBoxWidget m_ComboListingType;
	protected XComboBoxWidget m_ComboCategoryCreate;
	protected XComboBoxWidget m_ComboDuration;
	protected EditBoxWidget m_EditStartPrice;
	protected EditBoxWidget m_EditBuyNowPrice;
	protected TextWidget m_TxtFeeInfo;
	protected ButtonWidget m_BtnCreateListing;
	protected ButtonWidget m_BtnCancelCreate;
	protected ButtonWidget m_BtnBackToMarket;

	protected EntityAI m_SelectedItem;
	protected string m_SelectedItemClass;
	// Maps category-combo index -> categoryID
	protected ref array<int> m_CategoryComboIDs;
	protected ref DME_AH_AuctionMenu m_ParentMenu;

	void DME_AH_CreateListingDialog()
	{
		m_CategoryComboIDs = new array<int>;
	}

	void SetParentMenu(DME_AH_AuctionMenu parentMenu)
	{
		m_ParentMenu = parentMenu;
	}

	// Called by the parent menu AFTER EnterScriptedMenu returned the dialog.
	void SetSelectedItem(EntityAI item)
	{
		if (!item)
			return;

		m_SelectedItem = item;
		m_SelectedItemClass = item.GetType();

		if (m_TxtSelectedItem)
		{
			string displayName = item.GetDisplayName();
			if (displayName == "")
				displayName = m_SelectedItemClass;
			m_TxtSelectedItem.SetText(displayName);
		}

		// Show the actual selected entity directly — do NOT delete it on close.
		if (m_ItemPreview)
			m_ItemPreview.SetItem(item);
	}

	override Widget Init()
	{
		if (!g_Game)
			return null;

		layoutRoot = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu_create_listing.layout");
		if (!layoutRoot)
			return null;

		m_LstInventory = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("lstInventory"));
		m_ItemPreview = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("itemPreviewCreate"));
		m_TxtSelectedItem = TextWidget.Cast(layoutRoot.FindAnyWidget("txtSelectedItem"));
		m_TxtBalance = TextWidget.Cast(layoutRoot.FindAnyWidget("txtBalance"));
		m_ComboListingType = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("comboListingType"));
		m_ComboCategoryCreate = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("comboCategoryCreate"));
		m_ComboDuration = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("comboDuration"));
		m_EditStartPrice = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("editStartPrice"));
		m_EditBuyNowPrice = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("editBuyNowPrice"));
		m_TxtFeeInfo = TextWidget.Cast(layoutRoot.FindAnyWidget("txtFeeInfo"));
		m_BtnCreateListing = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnCreateListing"));
		m_BtnCancelCreate = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnCancelCreate"));
		m_BtnBackToMarket = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnBackToMarket"));

		PopulateComboBoxes();
		PopulateInventory();
		UpdateBalanceFromParent();

		return layoutRoot;
	}

	protected void UpdateBalanceFromParent()
	{
		if (!m_TxtBalance || !m_ParentMenu)
			return;
		int balance = m_ParentMenu.GetPlayerBalance();
		m_TxtBalance.SetText("Balance: " + balance.ToString());
	}

	override bool UseKeyboard()
	{
		return true;
	}

	override bool UseMouse()
	{
		return true;
	}

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

	protected void PopulateComboBoxes()
	{
		if (m_ComboListingType)
		{
			m_ComboListingType.AddItem("Buy Now");
			m_ComboListingType.AddItem("Auction");
			m_ComboListingType.AddItem("Auction + Buy Now");
			m_ComboListingType.SetCurrentItem(0);
		}

		if (m_ComboDuration)
		{
			m_ComboDuration.AddItem("6 Hours");
			m_ComboDuration.AddItem("12 Hours");
			m_ComboDuration.AddItem("24 Hours");
			m_ComboDuration.AddItem("48 Hours");
			m_ComboDuration.SetCurrentItem(2);
		}

		if (m_ComboCategoryCreate)
		{
			m_CategoryComboIDs.Clear();

			DME_AH_CategoryConfig catConfig;
			DME_AH_Module module = DME_AH_Module.GetInstance();
			if (module)
				catConfig = module.GetCategoryConfig();

			if (!catConfig || !catConfig.Categories || catConfig.Categories.Count() == 0)
			{
				catConfig = new DME_AH_CategoryConfig();
				catConfig.CreateDefaults();
			}

			int defaultIdx = 0;
			for (int i = 0; i < catConfig.Categories.Count(); i++)
			{
				DME_AH_Category cat = catConfig.Categories[i];
				if (!cat)
					continue;
				m_ComboCategoryCreate.AddItem(cat.DisplayName);
				m_CategoryComboIDs.Insert(cat.CategoryID);
				// Default to "Other" (ID 7) if present
				if (cat.CategoryID == 7)
					defaultIdx = m_CategoryComboIDs.Count() - 1;
			}

			m_ComboCategoryCreate.SetCurrentItem(defaultIdx);
		}
	}

	protected void PopulateInventory()
	{
		if (!m_LstInventory)
			return;
		if (!g_Game)
			return;

		m_LstInventory.ClearItems();

		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
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

			string itemName = item.GetDisplayName();
			if (itemName == "")
				itemName = item.GetType();

			m_LstInventory.AddItem(itemName, item, 0);
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		if (w == m_BtnCancelCreate || w == m_BtnBackToMarket)
		{
			Close();
			return true;
		}

		if (w == m_BtnCreateListing)
		{
			OnCreateListingClick();
			return true;
		}

		if (w == m_LstInventory)
		{
			OnInventoryItemSelected();
			return true;
		}

		return false;
	}

	// EditBoxWidget fires OnChange when text changes — keep the fee preview live.
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (w == m_EditStartPrice)
		{
			UpdateFeeInfo();
			return true;
		}
		return super.OnChange(w, x, y, finished);
	}

	protected void UpdateFeeInfo()
	{
		if (!m_TxtFeeInfo)
			return;

		int startPrice = 0;
		if (m_EditStartPrice)
			startPrice = m_EditStartPrice.GetText().ToInt();

		int fee = 0;
		if (m_ParentMenu)
		{
			DME_AH_Module module = DME_AH_Module.GetInstance();
			if (module)
			{
				DME_AH_Config cfg = module.GetConfig();
				if (cfg)
					fee = cfg.CalculateListingFee(startPrice);
			}
		}

		m_TxtFeeInfo.SetColor(ARGB(255, 204, 153, 0));
		m_TxtFeeInfo.SetText("Listing Fee: " + fee.ToString());
	}

	protected void OnInventoryItemSelected()
	{
		if (!m_LstInventory)
			return;

		int selectedRow = m_LstInventory.GetSelectedRow();
		if (selectedRow < 0)
			return;

		EntityAI item;
		m_LstInventory.GetItemData(selectedRow, 0, item);
		if (!item)
			return;

		SetSelectedItem(item);
	}

	protected void ShowInlineError(string message)
	{
		if (!m_TxtFeeInfo)
			return;
		m_TxtFeeInfo.SetColor(ARGB(255, 220, 60, 60));
		m_TxtFeeInfo.SetText(message);
	}

	protected void OnCreateListingClick()
	{
		if (!m_ParentMenu)
			return;

		if (!m_SelectedItem || m_SelectedItemClass == "")
		{
			ShowInlineError("Select an item first");
			return;
		}

		int listingType = 0;
		if (m_ComboListingType)
			listingType = m_ComboListingType.GetCurrentItem();

		int categoryID = 7;
		if (m_ComboCategoryCreate && m_CategoryComboIDs)
		{
			int catIdx = m_ComboCategoryCreate.GetCurrentItem();
			if (catIdx >= 0 && catIdx < m_CategoryComboIDs.Count())
				categoryID = m_CategoryComboIDs[catIdx];
		}

		string startPriceStr = "";
		if (m_EditStartPrice)
			startPriceStr = m_EditStartPrice.GetText();
		int startPrice = startPriceStr.ToInt();

		string buyNowPriceStr = "";
		if (m_EditBuyNowPrice)
			buyNowPriceStr = m_EditBuyNowPrice.GetText();
		int buyNowPrice = buyNowPriceStr.ToInt();

		// Validate by listing type.
		// Pure BuyNow: startPrice IS the buy-now price; require > 0.
		// Auction: require startPrice > 0.
		// AuctionWithBuyNow: require both > 0 and buyNowPrice > startPrice.
		if (listingType == EDME_AH_ListingType.BuyNow)
		{
			if (startPrice <= 0)
			{
				ShowInlineError("Enter a Buy Now price greater than 0");
				return;
			}
			// Unused field for pure BuyNow.
			buyNowPrice = 0;
		}
		else if (listingType == EDME_AH_ListingType.Auction)
		{
			if (startPrice <= 0)
			{
				ShowInlineError("Enter a Start price greater than 0");
				return;
			}
			buyNowPrice = 0;
		}
		else if (listingType == EDME_AH_ListingType.AuctionWithBuyNow)
		{
			if (startPrice <= 0)
			{
				ShowInlineError("Enter a Start price greater than 0");
				return;
			}
			if (buyNowPrice <= 0)
			{
				ShowInlineError("Enter a Buy Now price greater than 0");
				return;
			}
			if (buyNowPrice <= startPrice)
			{
				ShowInlineError("Buy Now price must be greater than Start price");
				return;
			}
		}
		else
		{
			ShowInlineError("Unknown listing type");
			return;
		}

		int durationMinutes = 1440;
		if (m_ComboDuration)
		{
			int durationIdx = m_ComboDuration.GetCurrentItem();
			if (durationIdx == 0)
				durationMinutes = 360;
			else if (durationIdx == 1)
				durationMinutes = 720;
			else if (durationIdx == 2)
				durationMinutes = 1440;
			else if (durationIdx == 3)
				durationMinutes = 2880;
		}

		int networkLow = 0;
		int networkHigh = 0;
		m_SelectedItem.GetNetworkID(networkLow, networkHigh);

		m_ParentMenu.SendCreateListing(m_SelectedItemClass, listingType, startPrice, buyNowPrice, durationMinutes, categoryID, networkLow, networkHigh);
		Close();
	}
}
