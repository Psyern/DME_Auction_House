// DME Auction House - Notification Handler

class DME_AH_NotificationHandler
{
	static void ShowNotification(string title, string message)
	{
		if (!g_Game)
			return;
		if (g_Game.IsDedicatedServer())
			return;

		NotificationSystem.AddNotification(NotificationType.GENERIC, title, message, "set:dayz_gui image:icon_gear");
	}

	static void ShowSoldNotification(string itemName, int price)
	{
		string message = "Your " + itemName + " was sold for " + price.ToString();
		ShowNotification("Item Sold!", message);
	}

	static void ShowAuctionWonNotification(string itemName, int price)
	{
		string message = "You won " + itemName + " for " + price.ToString();
		ShowNotification("Auction Won!", message);
	}

	static void ShowOutbidNotification(string itemName)
	{
		string message = "You have been outbid on " + itemName;
		ShowNotification("Outbid!", message);
	}

	static void ShowExpiredNotification(string itemName)
	{
		string message = "Your listing for " + itemName + " has expired";
		ShowNotification("Listing Expired", message);
	}

	static void ShowPendingPickupNotification()
	{
		ShowNotification("Auction House", "You have items or money to collect");
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
			ShowPendingPickupNotification();
	}
}
