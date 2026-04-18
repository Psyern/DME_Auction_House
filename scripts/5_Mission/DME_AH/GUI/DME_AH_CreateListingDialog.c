// DME Auction House - Create Listing Dialog

class DME_AH_CreateListingDialog : UIScriptedMenu
{
	protected TextListboxWidget m_LstInventory;
	protected ItemPreviewWidget m_ItemPreview;
	protected TextWidget m_TxtSelectedItem;
	protected XComboBoxWidget m_ComboListingType;
	protected XComboBoxWidget m_ComboCategoryCreate;
	protected XComboBoxWidget m_ComboDuration;
	protected EditBoxWidget m_EditStartPrice;
	protected EditBoxWidget m_EditBuyNowPrice;
	protected TextWidget m_TxtFeeInfo;
	protected ButtonWidget m_BtnCreateListing;
	protected ButtonWidget m_BtnCancelCreate;

	protected string m_SelectedItemClass;
	protected ref DME_AH_AuctionMenu m_ParentMenu;

	void SetParentMenu(DME_AH_AuctionMenu parentMenu)
	{
		m_ParentMenu = parentMenu;
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
		m_ComboListingType = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("comboListingType"));
		m_ComboCategoryCreate = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("comboCategoryCreate"));
		m_ComboDuration = XComboBoxWidget.Cast(layoutRoot.FindAnyWidget("comboDuration"));
		m_EditStartPrice = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("editStartPrice"));
		m_EditBuyNowPrice = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("editBuyNowPrice"));
		m_TxtFeeInfo = TextWidget.Cast(layoutRoot.FindAnyWidget("txtFeeInfo"));
		m_BtnCreateListing = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnCreateListing"));
		m_BtnCancelCreate = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnCancelCreate"));

		PopulateComboBoxes();
		PopulateInventory();

		return layoutRoot;
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
			m_ComboCategoryCreate.AddItem("Weapons");
			m_ComboCategoryCreate.AddItem("Clothing");
			m_ComboCategoryCreate.AddItem("Medical");
			m_ComboCategoryCreate.AddItem("Food");
			m_ComboCategoryCreate.AddItem("Vehicles");
			m_ComboCategoryCreate.AddItem("Building");
			m_ComboCategoryCreate.AddItem("Other");
			m_ComboCategoryCreate.SetCurrentItem(6);
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

		if (w == m_BtnCancelCreate)
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

		m_SelectedItemClass = item.GetType();

		if (m_TxtSelectedItem)
		{
			string displayName = item.GetDisplayName();
			if (displayName == "")
				displayName = m_SelectedItemClass;
			m_TxtSelectedItem.SetText(displayName);
		}

		if (m_ItemPreview)
			m_ItemPreview.SetItem(item);
	}

	protected void OnCreateListingClick()
	{
		if (m_SelectedItemClass == "")
			return;
		if (!m_ParentMenu)
			return;

		int listingType = 0;
		if (m_ComboListingType)
			listingType = m_ComboListingType.GetCurrentItem();

		int categoryID = 7;
		if (m_ComboCategoryCreate)
			categoryID = m_ComboCategoryCreate.GetCurrentItem() + 1;

		string startPriceStr = "";
		if (m_EditStartPrice)
			startPriceStr = m_EditStartPrice.GetText();
		int startPrice = startPriceStr.ToInt();

		string buyNowPriceStr = "";
		if (m_EditBuyNowPrice)
			buyNowPriceStr = m_EditBuyNowPrice.GetText();
		int buyNowPrice = buyNowPriceStr.ToInt();

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

		m_ParentMenu.SendCreateListing(m_SelectedItemClass, listingType, startPrice, buyNowPrice, durationMinutes, categoryID);
		Close();
	}
}
