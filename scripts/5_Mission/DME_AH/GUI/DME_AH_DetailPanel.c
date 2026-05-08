// DME Auction House - Detail Panel Controller

class DME_AH_DetailPanel
{
	protected Widget m_Root;
	protected TextWidget m_TxtTitle;
	protected TextWidget m_TxtSeller;
	protected TextWidget m_TxtType;
	protected TextWidget m_TxtPrice;
	protected TextWidget m_TxtBuyNow;
	protected TextWidget m_TxtBids;
	protected TextWidget m_TxtTime;
	protected ItemPreviewWidget m_ItemPreview;
	protected Widget m_BidSection;
	protected ButtonWidget m_BtnBuyNow;
	protected ButtonWidget m_BtnCancelListing;
	protected EditBoxWidget m_EditBidAmount;

	protected Widget m_HealthBar;
	protected Widget m_HealthBarFill;
	protected TextWidget m_TxtHealthLabel;
	protected Widget m_QuantityBar;
	protected Widget m_QuantityBarFill;
	protected TextWidget m_TxtQuantityLabel;
	protected Widget m_CargoGrid;
	protected TextWidget m_TxtCargoLabel;

	protected ref DME_AH_ListingRow m_CurrentRow;
	// Client-only preview entity spawned to populate m_ItemPreview.
	// Must be deleted in Hide() to avoid leaks.
	protected EntityAI m_PreviewEntity;

	void Init(Widget root)
	{
		m_Root = root;
		if (!m_Root)
			return;

		m_TxtTitle = TextWidget.Cast(m_Root.FindAnyWidget("txtDetailTitle"));
		m_TxtSeller = TextWidget.Cast(m_Root.FindAnyWidget("txtDetailSeller"));
		m_TxtType = TextWidget.Cast(m_Root.FindAnyWidget("txtDetailType"));
		m_TxtPrice = TextWidget.Cast(m_Root.FindAnyWidget("txtDetailPrice"));
		m_TxtBuyNow = TextWidget.Cast(m_Root.FindAnyWidget("txtDetailBuyNow"));
		m_TxtBids = TextWidget.Cast(m_Root.FindAnyWidget("txtDetailBids"));
		m_TxtTime = TextWidget.Cast(m_Root.FindAnyWidget("txtDetailTime"));
		m_ItemPreview = ItemPreviewWidget.Cast(m_Root.FindAnyWidget("itemPreview"));
		m_BidSection = m_Root.FindAnyWidget("BidSection");
		m_BtnBuyNow = ButtonWidget.Cast(m_Root.FindAnyWidget("btnBuyNow"));
		m_BtnCancelListing = ButtonWidget.Cast(m_Root.FindAnyWidget("btnCancelListing"));
		m_EditBidAmount = EditBoxWidget.Cast(m_Root.FindAnyWidget("editBidAmount"));

		m_HealthBar = m_Root.FindAnyWidget("HealthBar");
		m_HealthBarFill = m_Root.FindAnyWidget("healthBarFill");
		m_TxtHealthLabel = TextWidget.Cast(m_Root.FindAnyWidget("txtHealthLabel"));
		m_QuantityBar = m_Root.FindAnyWidget("QuantityBar");
		m_QuantityBarFill = m_Root.FindAnyWidget("quantityBarFill");
		m_TxtQuantityLabel = TextWidget.Cast(m_Root.FindAnyWidget("txtQuantityLabel"));
		m_CargoGrid = m_Root.FindAnyWidget("CargoGrid");
		m_TxtCargoLabel = TextWidget.Cast(m_Root.FindAnyWidget("txtCargoLabel"));
	}

	void Show(DME_AH_ListingRow row, bool isOwnListing)
	{
		if (!m_Root || !row)
			return;

		m_CurrentRow = row;
		m_Root.Show(true);

		// Clean up any stale preview entity from a previous Show() that
		// wasn't followed by a Hide() (e.g. user clicked row-to-row).
		DestroyPreviewEntity();

		if (m_ItemPreview && row.ItemClassName != "" && g_Game)
		{
			Object obj = g_Game.CreateObjectEx(row.ItemClassName, vector.Zero, ECE_LOCAL | ECE_NOLIFETIME);
			EntityAI preview;
			if (Class.CastTo(preview, obj))
			{
				m_PreviewEntity = preview;
				m_ItemPreview.SetItem(preview);
			}
			else if (obj)
			{
				g_Game.ObjectDelete(obj);
			}
		}

		UpdateHealthBar();
		UpdateQuantityBar();
		UpdateCargoGrid();

		if (m_TxtTitle)
			m_TxtTitle.SetText(row.ItemName);
		if (m_TxtSeller)
			m_TxtSeller.SetText("Seller: " + row.SellerName);
		if (m_TxtType)
			m_TxtType.SetText("Type: " + row.GetTypeString());
		if (m_TxtPrice)
			m_TxtPrice.SetText("Price: " + row.Price.ToString());
		if (m_TxtBids)
			m_TxtBids.SetText("Bids: " + row.BidCount.ToString());
		if (m_TxtTime)
			m_TxtTime.SetText("Time: " + row.GetTimeRemainingString());

		bool showBuyNowText = row.ListingType == EDME_AH_ListingType.AuctionWithBuyNow;
		if (m_TxtBuyNow)
		{
			if (showBuyNowText)
				m_TxtBuyNow.SetText("Buy Now: " + row.BuyNowPrice.ToString());
			else
				m_TxtBuyNow.SetText("");
		}

		bool isAuction = row.ListingType == EDME_AH_ListingType.Auction;
		bool isAuctionBN = row.ListingType == EDME_AH_ListingType.AuctionWithBuyNow;
		bool showBidSection = (isAuction || isAuctionBN) && !isOwnListing;
		if (m_BidSection)
			m_BidSection.Show(showBidSection);

		bool showBuyNowBtn = (row.ListingType == EDME_AH_ListingType.BuyNow || isAuctionBN) && !isOwnListing;
		if (m_BtnBuyNow)
			m_BtnBuyNow.Show(showBuyNowBtn);

		if (m_BtnCancelListing)
			m_BtnCancelListing.Show(isOwnListing);
	}

	void Hide()
	{
		if (m_Root)
			m_Root.Show(false);
		if (m_HealthBar)
			m_HealthBar.Show(false);
		if (m_QuantityBar)
			m_QuantityBar.Show(false);
		if (m_CargoGrid)
			m_CargoGrid.Show(false);
		m_CurrentRow = null;
		DestroyPreviewEntity();
	}

	protected void UpdateHealthBar()
	{
		if (!m_HealthBar)
			return;

		if (!m_PreviewEntity)
		{
			m_HealthBar.Show(false);
			return;
		}

		float maxHealth = m_PreviewEntity.GetMaxHealth("", "");
		float pct = 0;
		if (maxHealth > 0)
			pct = m_PreviewEntity.GetHealth("", "") / maxHealth;
		if (pct < 0)
			pct = 0;
		if (pct > 1)
			pct = 1;

		if (m_HealthBarFill)
			m_HealthBarFill.SetSize(pct, 1);

		if (m_TxtHealthLabel)
			m_TxtHealthLabel.SetText("Health: " + ComputeHealthStageString(m_PreviewEntity));

		m_HealthBar.Show(true);
	}

	protected void UpdateQuantityBar()
	{
		if (!m_QuantityBar)
			return;

		ItemBase ib;
		if (!m_PreviewEntity || !Class.CastTo(ib, m_PreviewEntity) || !ib.HasQuantity())
		{
			m_QuantityBar.Show(false);
			return;
		}

		float qty = ib.GetQuantity();
		float qtyMax = ib.GetQuantityMax();
		float pct = 0;
		if (qtyMax > 0)
			pct = qty / qtyMax;
		if (pct < 0)
			pct = 0;
		if (pct > 1)
			pct = 1;

		if (m_QuantityBarFill)
			m_QuantityBarFill.SetSize(pct, 1);

		if (m_TxtQuantityLabel)
			m_TxtQuantityLabel.SetText("Quantity: " + ib.GetQuantity().ToString() + "/" + ib.GetQuantityMax().ToString());

		m_QuantityBar.Show(true);
	}

	protected void UpdateCargoGrid()
	{
		if (!m_CargoGrid)
			return;

		if (!m_PreviewEntity || !m_PreviewEntity.GetInventory() || !m_PreviewEntity.GetInventory().GetCargo())
		{
			m_CargoGrid.Show(false);
			return;
		}

		CargoBase cargo = m_PreviewEntity.GetInventory().GetCargo();
		int count = cargo.GetItemCount();
		int capacity = cargo.GetWidth() * cargo.GetHeight();

		if (m_TxtCargoLabel)
			m_TxtCargoLabel.SetText("Cargo: " + count.ToString() + "/" + capacity.ToString());

		m_CargoGrid.Show(true);
	}

	protected string ComputeHealthStageString(EntityAI entity)
	{
		int level = entity.GetHealthLevel();
		if (level == GameConstants.STATE_PRISTINE)
			return "Pristine";
		if (level == GameConstants.STATE_WORN)
			return "Worn";
		if (level == GameConstants.STATE_DAMAGED)
			return "Damaged";
		if (level == GameConstants.STATE_BADLY_DAMAGED)
			return "Badly Damaged";
		if (level == GameConstants.STATE_RUINED)
			return "Ruined";
		return "Unknown";
	}

	protected void DestroyPreviewEntity()
	{
		if (m_ItemPreview)
			m_ItemPreview.SetItem(null);
		if (m_PreviewEntity && g_Game)
			g_Game.ObjectDelete(m_PreviewEntity);
		m_PreviewEntity = null;
	}

	bool IsVisible()
	{
		if (!m_Root)
			return false;
		return m_Root.IsVisible();
	}

	DME_AH_ListingRow GetCurrentRow()
	{
		return m_CurrentRow;
	}

	string GetBidAmountText()
	{
		if (!m_EditBidAmount)
			return "0";
		return m_EditBidAmount.GetText();
	}
}
