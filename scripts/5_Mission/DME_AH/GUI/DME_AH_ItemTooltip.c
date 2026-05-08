// DME Auction House - Item Tooltip (floating panel with item info entries)

class DME_AH_ItemTooltip
{
	protected Widget m_Root;
	protected TextWidget m_Title;
	protected Widget m_Spacer;
	protected ref array<ref DME_AH_ItemTooltipEntry> m_Entries;
	protected bool m_Visible;

	void DME_AH_ItemTooltip()
	{
		m_Entries = new array<ref DME_AH_ItemTooltipEntry>;
		m_Visible = false;
	}

	void Init(Widget parentMenuRoot)
	{
		if (!g_Game)
			return;

		m_Root = g_Game.GetWorkspace().CreateWidgets("DME_Auction_House/gui/DME_AH/layouts/auction_menu_tooltip.layout", parentMenuRoot);
		if (!m_Root)
			return;

		m_Title = TextWidget.Cast(m_Root.FindAnyWidget("txtTooltipTitle"));
		m_Spacer = m_Root.FindAnyWidget("spacerEntries");
		m_Root.Show(false);
	}

	void ClearEntries()
	{
		if (!m_Entries)
			return;

		for (int i = 0; i < m_Entries.Count(); i++)
		{
			DME_AH_ItemTooltipEntry entry = m_Entries[i];
			if (entry)
				entry.Destroy();
		}
		m_Entries.Clear();
	}

	void AddEntry(string iconPath, string label, string value)
	{
		if (!m_Spacer)
			return;

		DME_AH_ItemTooltipEntry entry = new DME_AH_ItemTooltipEntry();
		entry.Init(m_Spacer, iconPath, label, value);
		m_Entries.Insert(entry);
	}

	void ShowFor(DME_AH_ListingRow row, int mouseX, int mouseY)
	{
		if (!m_Root)
			return;

		if (!row)
		{
			Hide();
			return;
		}

		ClearEntries();

		if (m_Title)
			m_Title.SetText(row.ItemName);

		if (row.ItemClassName != "")
		{
			Object previewObj = g_Game.CreateObjectEx(row.ItemClassName, vector.Zero, ECE_LOCAL | ECE_NOLIFETIME);
			if (previewObj)
			{
				EntityAI entity = EntityAI.Cast(previewObj);
				if (entity)
					AddEntry("set:dayz_gui image:icon_health", "Health", computeHealthStageString(entity));

				ItemBase itemBase = ItemBase.Cast(previewObj);
				if (itemBase && itemBase.HasQuantity())
					AddEntry("set:dayz_gui image:icon_quantity", "Quantity", computeQuantityString(itemBase));

				if (entity && entity.GetInventory() && entity.GetInventory().GetCargo())
				{
					int cargoCount = entity.GetInventory().GetCargo().GetItemCount();
					if (cargoCount > 0)
						AddEntry("set:dayz_gui image:icon_cargo", "Items", cargoCount.ToString());
				}

				g_Game.ObjectDelete(previewObj);
			}
		}

		int screenW, screenH;
		GetScreenSize(screenW, screenH);

		float tw, th;
		m_Root.GetScreenSize(tw, th);

		float posX = mouseX + 16;
		float posY = mouseY + 16;

		if (posX + tw > screenW)
			posX = screenW - tw - 4;
		if (posY + th > screenH)
			posY = screenH - th - 4;
		if (posX < 0)
			posX = 0;
		if (posY < 0)
			posY = 0;

		m_Root.SetPos(posX, posY, true);
		m_Root.Show(true);
		m_Visible = true;
	}

	void Hide()
	{
		ClearEntries();
		if (m_Root)
			m_Root.Show(false);
		m_Visible = false;
	}

	protected string computeHealthStageString(EntityAI entity)
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

	protected string computeQuantityString(ItemBase item)
	{
		return item.GetQuantity().ToString() + "/" + item.GetQuantityMax().ToString();
	}
}
