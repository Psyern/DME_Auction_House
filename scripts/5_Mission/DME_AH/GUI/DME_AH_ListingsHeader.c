// DME Auction House - Clickable Listings Header (Sort Column Buttons)

class DME_AH_ListingsHeader
{
	protected DME_AH_AuctionMenu m_MenuRef;
	protected Widget m_Root;

	protected ButtonWidget m_BtnHeaderName;
	protected ButtonWidget m_BtnHeaderPrice;
	protected ButtonWidget m_BtnHeaderSeller;
	protected ButtonWidget m_BtnHeaderTime;

	protected TextWidget m_TxtArrowName;
	protected TextWidget m_TxtArrowPrice;
	protected TextWidget m_TxtArrowSeller;
	protected TextWidget m_TxtArrowTime;

	protected bool m_NameAsc;
	protected bool m_PriceLow;
	protected bool m_TimeOld;
	protected bool m_SellerAsc;

	void DME_AH_ListingsHeader()
	{
		m_NameAsc = true;
		m_PriceLow = true;
		m_TimeOld = true;
		m_SellerAsc = true;
	}

	void Init(Widget headerAnchor, DME_AH_AuctionMenu menuRef)
	{
		if (!headerAnchor || !g_Game)
			return;

		m_MenuRef = menuRef;

		Widget txtItem = headerAnchor.FindAnyWidget("txtHeaderItem");
		if (txtItem)
			TextWidget.Cast(txtItem).Show(false);
		Widget txtPrice = headerAnchor.FindAnyWidget("txtHeaderPrice");
		if (txtPrice)
			TextWidget.Cast(txtPrice).Show(false);
		Widget txtType = headerAnchor.FindAnyWidget("txtHeaderType");
		if (txtType)
			TextWidget.Cast(txtType).Show(false);
		Widget txtSeller = headerAnchor.FindAnyWidget("txtHeaderSeller");
		if (txtSeller)
			TextWidget.Cast(txtSeller).Show(false);
		Widget txtTime = headerAnchor.FindAnyWidget("txtHeaderTime");
		if (txtTime)
			TextWidget.Cast(txtTime).Show(false);

		m_Root = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu_listings_header.layout", headerAnchor);
		if (!m_Root)
			return;

		m_BtnHeaderName = ButtonWidget.Cast(m_Root.FindAnyWidget("btnHeaderName"));
		m_BtnHeaderPrice = ButtonWidget.Cast(m_Root.FindAnyWidget("btnHeaderPrice"));
		m_BtnHeaderSeller = ButtonWidget.Cast(m_Root.FindAnyWidget("btnHeaderSeller"));
		m_BtnHeaderTime = ButtonWidget.Cast(m_Root.FindAnyWidget("btnHeaderTime"));

		m_TxtArrowName = TextWidget.Cast(m_Root.FindAnyWidget("txtArrowName"));
		m_TxtArrowPrice = TextWidget.Cast(m_Root.FindAnyWidget("txtArrowPrice"));
		m_TxtArrowSeller = TextWidget.Cast(m_Root.FindAnyWidget("txtArrowSeller"));
		m_TxtArrowTime = TextWidget.Cast(m_Root.FindAnyWidget("txtArrowTime"));

		ClearAllArrows();
	}

	bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_MenuRef || !w)
			return false;

		if (w == m_BtnHeaderName)
		{
			ClearAllArrows();
			m_NameAsc = !m_NameAsc;
			if (m_NameAsc)
			{
				SetArrow(m_TxtArrowName, "^");
				m_MenuRef.Listings_Filter_NameAZ();
			}
			else
			{
				SetArrow(m_TxtArrowName, "v");
				m_MenuRef.Listings_Filter_NameZA();
			}
			return true;
		}

		if (w == m_BtnHeaderPrice)
		{
			ClearAllArrows();
			m_PriceLow = !m_PriceLow;
			if (m_PriceLow)
			{
				SetArrow(m_TxtArrowPrice, "^");
				m_MenuRef.Listings_Filter_PriceLowHigh();
			}
			else
			{
				SetArrow(m_TxtArrowPrice, "v");
				m_MenuRef.Listings_Filter_PriceHighLow();
			}
			return true;
		}

		if (w == m_BtnHeaderTime)
		{
			ClearAllArrows();
			m_TimeOld = !m_TimeOld;
			if (m_TimeOld)
			{
				SetArrow(m_TxtArrowTime, "^");
				m_MenuRef.Listings_Filter_TimeOldNew();
			}
			else
			{
				SetArrow(m_TxtArrowTime, "v");
				m_MenuRef.Listings_Filter_TimeNewOld();
			}
			return true;
		}

		if (w == m_BtnHeaderSeller)
		{
			ClearAllArrows();
			m_SellerAsc = !m_SellerAsc;
			if (m_SellerAsc)
			{
				SetArrow(m_TxtArrowSeller, "^");
				m_MenuRef.Listings_Filter_SellerAZ();
			}
			else
			{
				SetArrow(m_TxtArrowSeller, "v");
				m_MenuRef.Listings_Filter_SellerZA();
			}
			return true;
		}

		return false;
	}

	protected void ClearAllArrows()
	{
		SetArrow(m_TxtArrowName, "");
		SetArrow(m_TxtArrowPrice, "");
		SetArrow(m_TxtArrowSeller, "");
		SetArrow(m_TxtArrowTime, "");
	}

	protected void SetArrow(TextWidget arrow, string symbol)
	{
		if (arrow)
			arrow.SetText(symbol);
	}
}
