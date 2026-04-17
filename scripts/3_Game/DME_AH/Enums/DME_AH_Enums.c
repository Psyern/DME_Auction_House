// DME Auction House - Enums

enum EDME_AH_ListingType
{
	BuyNow = 0,
	Auction,
	AuctionWithBuyNow
}

enum EDME_AH_ListingStatus
{
	Active = 0,
	Sold,
	Expired,
	Cancelled
}

enum EDME_AH_TransactionType
{
	BuyNow = 0,
	AuctionWon,
	Expired,
	Cancelled
}

enum EDME_AH_SortMode
{
	PriceAsc = 0,
	PriceDesc,
	NameAsc,
	NameDesc,
	TimeAsc,
	TimeDesc,
	NewestFirst
}

enum EDME_AH_ResultCode
{
	Success = 0,
	FailedNotEnoughMoney,
	FailedListingNotFound,
	FailedListingExpired,
	FailedBidTooLow,
	FailedMaxListingsReached,
	FailedMaxBidsReached,
	FailedItemNotInInventory,
	FailedCannotCancelWithBids,
	FailedOwnListing,
	FailedInvalidPrice,
	FailedServerError
}

enum EDME_AH_NotificationType
{
	ItemSold = 0,
	AuctionWon,
	Outbid,
	ListingExpired,
	AuctionExpiredNoBids,
	PendingPickup
}

enum EDME_AH_MenuTab
{
	Marketplace = 0,
	MyListings,
	MyBids,
	SellItem
}

enum EDME_AH_CurrencyType
{
	Expansion = 0,
	Item,
	Internal
}
