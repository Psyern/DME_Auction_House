// DME Auction House - Listing Row Widget (per-row controller with 3D item preview)

class DME_AH_ListingRowWidget
{
	protected Widget m_Root;
	protected ItemPreviewWidget m_ItemPreview;
	protected TextWidget m_TxtItem;
	protected TextWidget m_TxtPrice;
	protected TextWidget m_TxtType;
	protected TextWidget m_TxtSeller;
	protected TextWidget m_TxtTime;
	protected ref DME_AH_ListingRow m_Row;
	protected EntityAI m_PreviewEntity;
	protected DME_AH_AuctionMenu m_MenuRef;

	void Init(Widget parentList, DME_AH_ListingRow row, DME_AH_AuctionMenu menuRef)
	{
		if (!parentList || !g_Game)
			return;

		m_MenuRef = menuRef;

		m_Root = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu_listing_row.layout", parentList);
		if (!m_Root)
			return;

		m_ItemPreview = ItemPreviewWidget.Cast(m_Root.FindAnyWidget("itemPreviewRow"));
		m_TxtItem = TextWidget.Cast(m_Root.FindAnyWidget("txtRowItem"));
		m_TxtPrice = TextWidget.Cast(m_Root.FindAnyWidget("txtRowPrice"));
		m_TxtType = TextWidget.Cast(m_Root.FindAnyWidget("txtRowType"));
		m_TxtSeller = TextWidget.Cast(m_Root.FindAnyWidget("txtRowSeller"));
		m_TxtTime = TextWidget.Cast(m_Root.FindAnyWidget("txtRowTime"));

		SetRow(row);
	}

	void SetRow(DME_AH_ListingRow row)
	{
		m_Row = row;

		if (m_PreviewEntity)
		{
			if (m_ItemPreview)
				m_ItemPreview.SetItem(null);
			g_Game.ObjectDelete(m_PreviewEntity);
			m_PreviewEntity = null;
		}

		if (!m_Row)
			return;

		if (m_TxtItem)
			m_TxtItem.SetText(m_Row.ItemName);
		if (m_TxtPrice)
			m_TxtPrice.SetText(m_Row.Price.ToString());
		if (m_TxtType)
			m_TxtType.SetText(m_Row.GetTypeString());
		if (m_TxtSeller)
			m_TxtSeller.SetText(m_Row.SellerName);
		if (m_TxtTime)
			m_TxtTime.SetText(m_Row.GetTimeRemainingString());

		if (m_ItemPreview && m_Row.ItemClassName != "")
		{
			Object obj = g_Game.CreateObjectEx(m_Row.ItemClassName, vector.Zero, ECE_LOCAL | ECE_NOLIFETIME);
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
	}

	bool OnClick(Widget w, int x, int y, int button)
	{
		if (!w || !m_Root)
			return false;

		if (w == m_Root)
		{
			if (m_MenuRef)
				m_MenuRef.OnRowClicked(this);
			return true;
		}

		return false;
	}

	DME_AH_ListingRow GetRow()
	{
		return m_Row;
	}

	Widget GetRoot()
	{
		return m_Root;
	}

	void Destroy()
	{
		if (m_PreviewEntity)
		{
			if (m_ItemPreview)
				m_ItemPreview.SetItem(null);
			g_Game.ObjectDelete(m_PreviewEntity);
			m_PreviewEntity = null;
		}

		if (m_Root)
		{
			m_Root.Unlink();
			m_Root = null;
		}
	}

	void ~DME_AH_ListingRowWidget()
	{
		Destroy();
	}
}
