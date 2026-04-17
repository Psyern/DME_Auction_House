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

	protected ref DME_AH_ListingRow m_CurrentRow;

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
	}

	void Show(DME_AH_ListingRow row, bool isOwnListing)
	{
		if (!m_Root || !row)
			return;

		m_CurrentRow = row;
		m_Root.Show(true);

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
		m_CurrentRow = null;
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
