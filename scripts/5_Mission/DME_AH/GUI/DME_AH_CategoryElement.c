// DME Auction House - Category Sidebar Element

class DME_AH_CategoryElement
{
	protected Widget m_Root;
	protected ButtonWidget m_BtnExpand;
	protected ImageWidget m_Icon;
	protected TextWidget m_Name;
	protected TextWidget m_Count;
	protected Widget m_SubContainer;
	protected ref DME_AH_Category m_Category;
	protected DME_AH_AuctionMenu m_MenuRef;
	protected bool m_Expanded;
	protected bool m_Selected;

	void Init(Widget parentSidebar, DME_AH_Category category, DME_AH_AuctionMenu menuRef)
	{
		if (!parentSidebar || !g_Game)
			return;

		m_Category = category;
		m_MenuRef = menuRef;
		m_Expanded = false;
		m_Selected = false;

		m_Root = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu_category.layout", parentSidebar);
		if (!m_Root)
			return;

		m_BtnExpand = ButtonWidget.Cast(m_Root.FindAnyWidget("btnExpand"));
		m_Icon = ImageWidget.Cast(m_Root.FindAnyWidget("imgIcon"));
		m_Name = TextWidget.Cast(m_Root.FindAnyWidget("txtName"));
		m_Count = TextWidget.Cast(m_Root.FindAnyWidget("txtCount"));
		m_SubContainer = m_Root.FindAnyWidget("subCategoryContainer");

		if (m_Category)
		{
			if (m_Icon && m_Category.Icon != "")
				m_Icon.LoadImageFile(0, m_Category.Icon);
			if (m_Name)
				m_Name.SetText(m_Category.DisplayName);
		}
		if (m_Count)
			m_Count.SetText("0");
		if (m_SubContainer)
			m_SubContainer.Show(false);

		ApplySelectedColor();
	}

	bool OnClick(Widget w, int x, int y, int button)
	{
		if (!w || !m_Root)
			return false;

		if (m_BtnExpand && w == m_BtnExpand)
		{
			m_Expanded = !m_Expanded;
			if (m_SubContainer)
				m_SubContainer.Show(m_Expanded);
			if (m_BtnExpand)
			{
				if (m_Expanded)
					m_BtnExpand.SetText("v");
				else
					m_BtnExpand.SetText(">");
			}
			return true;
		}

		if (w == m_Root || w == m_Name || w == m_Icon)
		{
			if (m_MenuRef && m_Category)
				m_MenuRef.SelectCategory(m_Category.CategoryID);
			return true;
		}

		return false;
	}

	void SetCount(int count)
	{
		if (m_Count)
			m_Count.SetText(count.ToString());
	}

	void SetSelected(bool selected)
	{
		m_Selected = selected;
		ApplySelectedColor();
	}

	protected void ApplySelectedColor()
	{
		if (!m_Root)
			return;
		if (m_Selected)
			m_Root.SetColor(ARGB(220, 80, 40, 0));
		else
			m_Root.SetColor(ARGB(0, 0, 0, 0));
	}

	int GetCategoryID()
	{
		if (!m_Category)
			return 0;
		return m_Category.CategoryID;
	}

	Widget GetRootWidget()
	{
		return m_Root;
	}
}
