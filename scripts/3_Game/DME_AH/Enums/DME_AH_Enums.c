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

// Native RPC IDs (10000+ to avoid vanilla conflicts)
enum EDME_AH_RPC
{
	DME_AH_RPC_REQUEST_LISTINGS = 10100,
	DME_AH_RPC_SEND_LISTINGS,
	DME_AH_RPC_CREATE_LISTING,
	DME_AH_RPC_PLACE_BID,
	DME_AH_RPC_BUY_NOW,
	DME_AH_RPC_CANCEL_LISTING,
	DME_AH_RPC_REQUEST_MY_LISTINGS,
	DME_AH_RPC_SEND_MY_LISTINGS,
	DME_AH_RPC_REQUEST_MY_BIDS,
	DME_AH_RPC_SEND_MY_BIDS,
	DME_AH_RPC_REQUEST_HISTORY,
	DME_AH_RPC_SEND_HISTORY,
	DME_AH_RPC_REQUEST_BALANCE,
	DME_AH_RPC_SEND_BALANCE,
	DME_AH_RPC_REQUEST_CATEGORIES,
	DME_AH_RPC_SEND_CATEGORIES,
	DME_AH_RPC_CALLBACK,
	DME_AH_RPC_NOTIFICATION,
	DME_AH_RPC_COLLECT_PENDING
}
