// DME Auction House - Sub-Category Sidebar Element (placeholder for future use)

class DME_AH_SubCategoryElement
{
	protected Widget m_Root;
	protected ImageWidget m_Icon;
	protected TextWidget m_Name;
	protected TextWidget m_Count;
	protected bool m_Selected;

	void Init(Widget parentContainer, string displayName, string iconPath)
	{
		if (!parentContainer || !g_Game)
			return;

		m_Selected = false;

		m_Root = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu_subcategory.layout", parentContainer);
		if (!m_Root)
			return;

		m_Icon = ImageWidget.Cast(m_Root.FindAnyWidget("imgSubIcon"));
		m_Name = TextWidget.Cast(m_Root.FindAnyWidget("txtSubName"));
		m_Count = TextWidget.Cast(m_Root.FindAnyWidget("txtSubCount"));

		if (m_Icon && iconPath != "")
			m_Icon.LoadImageFile(0, iconPath);
		if (m_Name)
			m_Name.SetText(displayName);
		if (m_Count)
			m_Count.SetText("0");
	}

	void SetCount(int count)
	{
		if (m_Count)
			m_Count.SetText(count.ToString());
	}

	void SetSelected(bool selected)
	{
		m_Selected = selected;
		if (!m_Root)
			return;
		if (m_Selected)
			m_Root.SetColor(ARGB(220, 80, 40, 0));
		else
			m_Root.SetColor(ARGB(0, 0, 0, 0));
	}

	Widget GetRootWidget()
	{
		return m_Root;
	}
}
