// DME Auction House - Notification Handler

class DME_AH_NotificationHandler
{
	static void ShowNotification(string title, string message)
	{
		if (!g_Game)
			return;
		if (g_Game.IsDedicatedServer())
			return;

		NotificationSystem.AddNotificationExtended(5.0, title, message, "set:dayz_gui image:icon_gear");
	}

	static void HandleNotificationRPC(int notificationType, string message)
	{
		if (notificationType == EDME_AH_NotificationType.ItemSold)
			ShowNotification("Item Sold!", message);
		else if (notificationType == EDME_AH_NotificationType.AuctionWon)
			ShowNotification("Auction Won!", message);
		else if (notificationType == EDME_AH_NotificationType.Outbid)
			ShowNotification("Outbid!", message);
		else if (notificationType == EDME_AH_NotificationType.ListingExpired)
			ShowNotification("Listing Expired", message);
		else if (notificationType == EDME_AH_NotificationType.AuctionExpiredNoBids)
			ShowNotification("Auction Expired", message);
		else if (notificationType == EDME_AH_NotificationType.PendingPickup)
			ShowNotification("Auction House", "You have items to collect");
	}
}
