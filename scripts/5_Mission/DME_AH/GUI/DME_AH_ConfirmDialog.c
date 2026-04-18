// DME Auction House - Confirm Dialog

class DME_AH_ConfirmDialog : UIScriptedMenu
{
	protected ButtonWidget m_BtnYes;
	protected ButtonWidget m_BtnNo;
	protected TextWidget m_TxtTitle;
	protected TextWidget m_TxtMessage;

	protected string m_ConfirmAction;
	protected string m_ConfirmData;
	protected ref DME_AH_AuctionMenu m_ParentMenu;

	void SetParentMenu(DME_AH_AuctionMenu parentMenu)
	{
		m_ParentMenu = parentMenu;
	}

	void SetConfirmData(string title, string message, string action, string data)
	{
		m_ConfirmAction = action;
		m_ConfirmData = data;
		if (m_TxtTitle)
			m_TxtTitle.SetText(title);
		if (m_TxtMessage)
			m_TxtMessage.SetText(message);
	}

	override Widget Init()
	{
		if (!g_Game)
			return null;

		layoutRoot = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu_confirm_dialog.layout");
		if (!layoutRoot)
			return null;

		m_BtnYes = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnConfirmYes"));
		m_BtnNo = ButtonWidget.Cast(layoutRoot.FindAnyWidget("btnConfirmNo"));
		m_TxtTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("txtConfirmTitle"));
		m_TxtMessage = TextWidget.Cast(layoutRoot.FindAnyWidget("txtConfirmMessage"));

		return layoutRoot;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		if (w == m_BtnYes)
		{
			if (m_ParentMenu)
				m_ParentMenu.OnConfirmAction(m_ConfirmAction, m_ConfirmData);
			Close();
			return true;
		}

		if (w == m_BtnNo)
		{
			Close();
			return true;
		}

		return false;
	}
}
