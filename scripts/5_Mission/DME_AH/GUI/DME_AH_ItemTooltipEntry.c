// DME Auction House - Tooltip Entry (single line: icon + label + value)

class DME_AH_ItemTooltipEntry
{
	protected Widget m_Root;
	protected ImageWidget m_Icon;
	protected TextWidget m_Label;
	protected TextWidget m_Value;

	void Init(Widget parent, string iconPath, string label, string value)
	{
		if (!g_Game)
			return;

		m_Root = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu_tooltip_entry.layout", parent);
		if (!m_Root)
			return;

		m_Icon = ImageWidget.Cast(m_Root.FindAnyWidget("imgIcon"));
		m_Label = TextWidget.Cast(m_Root.FindAnyWidget("txtLabel"));
		m_Value = TextWidget.Cast(m_Root.FindAnyWidget("txtValue"));

		if (m_Icon && iconPath != "")
			m_Icon.LoadImageFile(0, iconPath);

		if (m_Label)
			m_Label.SetText(label);

		if (m_Value)
			m_Value.SetText(value);
	}

	void Destroy()
	{
		if (m_Root)
		{
			m_Root.Unlink();
			m_Root = null;
		}
	}
}
