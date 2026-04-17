# DME Auction House - GUI & System Design Spec

**Mod:** DME_Auction_House
**Prefix:** DME_AH
**Author:** Psyern | **Community:** DeadmansEcho
**Target:** DayZ 1.29 Experimental (EnforceScript)
**Date:** 2026-04-17

---

## 1. Overview

DME Auction House is a player-to-player trading mod for DayZ. Players can list items for sale (fixed price or auction with bidding) and buy from other players. The mod is accessible via placeable NPC traders and terminal objects. Currency is configurable by the server admin (Expansion Market, in-game items, or internal points).

### Design Decisions

| Topic | Decision |
|-------|----------|
| Access Point | NPC + Terminal (both, admin places either) |
| Trading Model | Hybrid: Auction (bidding + timer) + Buy Now (fixed price) |
| Currency | Configurable: Expansion / In-Game Items / Internal |
| UI Layout | Tab-based with list/table view |
| Features | Categories, Search, Sort, 3D Preview, History, Fees, Expiry, Notifications, Max Listings |
| Data Storage | One JSON file per data type ($profile:DME_AH/) |
| RPC System | Community Framework (CF_GetRPCManager) |

---

## 2. Architecture

```
CLIENT (5_Mission)                         SERVER (3_Game + 4_World)
+----------------------------------+       +----------------------------------+
| DME_AH_AuctionMenu              |       | DME_AH_Module (CF_ModuleWorld)   |
|   Tab: Marktplatz (Browse)       |       |   - OnInit: Load config, data    |
|   Tab: Meine Listings            | <---> |   - OnUpdate: Check expiry       |
|   Tab: Meine Gebote             |  CF   |   - RegisterRPCs                 |
|   Tab: Item Verkaufen            | RPCs  |                                  |
+----------------------------------+       | DME_AH_AuctionManager            |
| DME_AH_DetailPanel               |       |   - CreateListing()              |
|   3D ItemPreview + Info + Actions |       |   - PlaceBid()                   |
+----------------------------------+       |   - BuyNow()                     |
| DME_AH_CreateListingDialog       |       |   - CancelListing()              |
|   Item select, price, type, dur. |       |   - CheckExpiredListings()       |
+----------------------------------+       |   - ProcessCompletedAuctions()   |
| DME_AH_ConfirmDialog             |       +----------------------------------+
|   Confirm buy/bid/cancel         |       | DME_AH_CurrencyAdapter           |
+----------------------------------+       |   - GetBalance(playerUID)        |
| DME_AH_NotificationHandler       |       |   - Deduct(playerUID, amount)    |
|   Toast notifications            |       |   - Add(playerUID, amount)       |
+----------------------------------+       +----------------------------------+
                                           | DME_AH_DataStore                 |
                                           |   - LoadListings() / Save()      |
                                           |   - LoadHistory() / Save()       |
                                           |   - LoadPlayerData() / Save()    |
                                           |   - AutoSave (configurable)      |
                                           +----------------------------------+
```

### Layer Assignment

| Layer | Classes |
|-------|---------|
| Common | DME_AH_Debug |
| 3_Game | DME_AH_Module, DME_AH_AuctionManager, DME_AH_DataStore, DME_AH_CurrencyAdapter, DME_AH_CurrencyExpansion, DME_AH_CurrencyItem, DME_AH_Config, DME_AH_CategoryConfig, DME_AH_Listing, DME_AH_Bid, DME_AH_Transaction, DME_AH_Category, DME_AH_Constants, DME_AH_Enums, DME_AH_Logger, DME_AH_RPCHandler |
| 4_World | DME_AH_AuctionTerminal, DME_AH_AuctionNPC, DME_AH_ActionOpenAuction, ActionConstructor (modded) |
| 5_Mission | DME_AH_AuctionMenu, DME_AH_ListingRow, DME_AH_DetailPanel, DME_AH_CreateListingDialog, DME_AH_ConfirmDialog, DME_AH_NotificationHandler, MissionBase (modded), MissionGameplay (modded) |

---

## 3. Data Model

### DME_AH_Listing
```
string ListingID            - Unique ID (timestamp + random)
string SellerUID            - Steam64 ID of seller
string SellerName           - Display name
string ItemClassName        - DayZ item classname
string ItemDisplayName      - Human-readable name
int CategoryID              - Category reference
int ListingType             - 0 = BuyNow, 1 = Auction, 2 = Auction+BuyNow
int StartPrice              - Starting price (auction) or fixed price (buy now)
int BuyNowPrice             - Buy-now price (0 = no buy now option)
int CurrentBid              - Current highest bid (0 = no bids)
string CurrentBidderUID     - UID of highest bidder
string CurrentBidderName    - Name of highest bidder
int BidCount                - Number of bids placed
int CreatedTimestamp        - Unix timestamp of creation
int ExpiresTimestamp        - Unix timestamp of expiry
int Status                  - 0=Active, 1=Sold, 2=Expired, 3=Cancelled
```

### DME_AH_Bid
```
string BidID                - Unique ID
string ListingID            - Reference to listing
string BidderUID            - Steam64 ID
string BidderName           - Display name
int Amount                  - Bid amount
int Timestamp               - Unix timestamp
```

### DME_AH_Transaction
```
string TransactionID        - Unique ID
string ListingID            - Reference to original listing
string SellerUID            - Seller Steam64
string BuyerUID             - Buyer Steam64
string SellerName           - Seller display name
string BuyerName            - Buyer display name
string ItemClassName        - Item classname
string ItemDisplayName      - Human-readable name
int FinalPrice              - Final sale price
int Fee                     - Fee deducted
int Timestamp               - Unix timestamp
int Type                    - 0=BuyNow, 1=AuctionWon, 2=Expired, 3=Cancelled
```

### DME_AH_Category
```
int CategoryID              - Unique ID
string DisplayName          - Localized display name (#STR_ key)
string Icon                 - Icon path for UI
array<string> ClassFilters  - Optional: restrict to specific classnames
```

---

## 4. Server Configuration

### Settings.json ($profile:DME_AH/Config/Settings.json)
```json
{
    "Enabled": true,
    "CurrencyType": "Expansion",
    "CurrencyItemClass": "MoneyRuble100",
    "MaxListingsPerPlayer": 10,
    "MaxActiveBidsPerPlayer": 20,
    "ListingFeePercent": 5,
    "SaleFeePercent": 10,
    "MinPrice": 1,
    "MaxPrice": 1000000,
    "AllowedDurations": [360, 720, 1440, 2880],
    "DefaultDuration": 1440,
    "MinBidIncrement": 1,
    "MinBidIncrementPercent": 5,
    "AutoSaveIntervalSeconds": 300,
    "MaxHistoryEntries": 500,
    "NotificationsEnabled": true,
    "LogLevel": 1
}
```

- `CurrencyType`: "Expansion" | "Item" | "Internal"
- `CurrencyItemClass`: Only used when CurrencyType = "Item"
- `AllowedDurations`: Minutes (360=6h, 720=12h, 1440=24h, 2880=48h)
- `MinBidIncrementPercent`: Minimum bid increase as % of current bid
- `ListingFeePercent`: Deducted from seller when listing (non-refundable)
- `SaleFeePercent`: Deducted from sale price on successful sale

### Categories.json ($profile:DME_AH/Config/Categories.json)
```json
{
    "Categories": [
        { "CategoryID": 1, "DisplayName": "Weapons", "Icon": "set:dayz_gui image:icon_weapon", "ClassFilters": [] },
        { "CategoryID": 2, "DisplayName": "Clothing", "Icon": "set:dayz_gui image:icon_clothing", "ClassFilters": [] },
        { "CategoryID": 3, "DisplayName": "Medical", "Icon": "set:dayz_gui image:icon_medical", "ClassFilters": [] },
        { "CategoryID": 4, "DisplayName": "Food", "Icon": "set:dayz_gui image:icon_food", "ClassFilters": [] },
        { "CategoryID": 5, "DisplayName": "Vehicles", "Icon": "set:dayz_gui image:icon_vehicle", "ClassFilters": [] },
        { "CategoryID": 6, "DisplayName": "Building", "Icon": "set:dayz_gui image:icon_build", "ClassFilters": [] },
        { "CategoryID": 7, "DisplayName": "Other", "Icon": "set:dayz_gui image:icon_gear", "ClassFilters": [] }
    ]
}
```

---

## 5. Data Storage

All data persisted as JSON in `$profile:DME_AH/`:

| File | Content | Write Frequency |
|------|---------|-----------------|
| `Config/Settings.json` | Server settings | On first run, admin edits |
| `Config/Categories.json` | Category definitions | On first run, admin edits |
| `Data/ActiveListings.json` | All active listings | On change + auto-save |
| `Data/CompletedListings.json` | Transaction history | On sale/expiry + auto-save |
| `Data/PlayerData.json` | Player balances (Internal currency), pending pickups | On change + auto-save |

### Auto-Save
- Configurable interval (default 300s)
- Save on every transaction (create/buy/bid/cancel/expire)
- Save on server shutdown (OnUnloadModule)

### Pending Pickup System
When a player is offline and their item sells or auction expires:
- Sale proceeds / returned items stored in PlayerData.json under `PendingPickups`
- On next login or next auction menu open, player is notified and can collect

---

## 6. RPC Communication (CF)

All RPCs registered via `CF_GetRPCManager().RegisterServerRPC()` / `RegisterClientRPC()`.

### Client → Server

| RPC | Parameters | Description |
|-----|-----------|-------------|
| `DME_AH_RPC_RequestListings` | int categoryID, string search, int sortMode, int page | Browse marketplace |
| `DME_AH_RPC_CreateListing` | string itemClassName, int listingType, int startPrice, int buyNowPrice, int durationMinutes | Create new listing |
| `DME_AH_RPC_PlaceBid` | string listingID, int bidAmount | Place bid on auction |
| `DME_AH_RPC_BuyNow` | string listingID | Buy immediately |
| `DME_AH_RPC_CancelListing` | string listingID | Cancel own listing (no active bids) |
| `DME_AH_RPC_RequestMyListings` | (none) | Get own listings |
| `DME_AH_RPC_RequestMyBids` | (none) | Get own active bids |
| `DME_AH_RPC_RequestHistory` | int page | Get transaction history |
| `DME_AH_RPC_CollectPending` | string pendingID | Collect pending money/items |

### Server → Client

| RPC | Parameters | Description |
|-----|-----------|-------------|
| `DME_AH_RPC_SendListings` | serialized array of listings, int totalPages | Listing data response |
| `DME_AH_RPC_SendMyListings` | serialized array of own listings | Own listings response |
| `DME_AH_RPC_SendMyBids` | serialized array of bids with listing info | Own bids response |
| `DME_AH_RPC_SendHistory` | serialized array of transactions | History response |
| `DME_AH_RPC_Callback` | int resultCode, string message | Action result (success/error) |
| `DME_AH_RPC_Notification` | int notificationType, string message | Push notification |
| `DME_AH_RPC_SendBalance` | int balance | Player balance update |

### Result Codes (DME_AH_RPC_Callback)
```
0 = Success
1 = FailedNotEnoughMoney
2 = FailedListingNotFound
3 = FailedListingExpired
4 = FailedBidTooLow
5 = FailedMaxListingsReached
6 = FailedMaxBidsReached
7 = FailedItemNotInInventory
8 = FailedCannotCancelWithBids
9 = FailedOwnListing
10 = FailedInvalidPrice
11 = FailedServerError
```

### Notification Types
```
0 = ItemSold (your listing was purchased)
1 = AuctionWon (you won an auction)
2 = Outbid (someone outbid you)
3 = ListingExpired (your listing expired)
4 = AuctionExpiredNoBids (auction ended with no bids)
5 = PendingPickup (you have items/money to collect)
```

---

## 7. UI Design

### Main Menu: DME_AH_AuctionMenu (UIScriptedMenu)

**Layout:** Full-screen overlay with tab navigation.

**4 Tabs at top:**
1. **Marktplatz** - Browse all active listings
2. **Meine Listings** - Own active/expired listings, collect pending
3. **Meine Gebote** - Active bids, status (highest/outbid)
4. **Item Verkaufen** - Create new listing from inventory

**Below tabs:** Filter bar (search input, category dropdown, sort buttons)

**Main area:** Table/list view with columns:
- Item icon + name
- Price (current bid or buy-now price)
- Type indicator (Auction / Buy Now / Both)
- Seller name
- Remaining time (countdown)
- Action button (Buy / Bid / View)

**Bottom bar:** Player balance display, page navigation (< Page X/Y >)

**Clicking a listing row** opens the Detail Panel on the right side.

### Detail Panel: DME_AH_DetailPanel

Slides in from right when a listing is selected:
- 3D ItemPreviewWidget (item model)
- Item name + classname
- Seller name
- Listing type badge (Auction / Buy Now)
- Current price / Current bid + bid count
- Buy-now price (if available)
- Time remaining (countdown)
- Buy Now button (green)
- Place Bid section: input field + Bid button (orange)
- Close button

### Create Listing Dialog: DME_AH_CreateListingDialog

Opened from "Item Verkaufen" tab:
- Item selection (list of player inventory items)
- 3D preview of selected item
- Listing type selector: Buy Now / Auction / Auction + Buy Now
- Price input(s): Start price, Buy-now price
- Duration selector: dropdown (6h / 12h / 24h / 48h)
- Fee display: "Listing fee: X (Y%)"
- Category selector: dropdown
- Create Listing button
- Cancel button

### Confirm Dialog: DME_AH_ConfirmDialog

Modal overlay for:
- Confirm purchase: "Buy [item] for [price]?"
- Confirm bid: "Bid [amount] on [item]?"
- Confirm cancel: "Cancel listing for [item]?"
- Yes / No buttons

### Notification Handler: DME_AH_NotificationHandler

Receives DME_AH_RPC_Notification and shows:
- Toast notification (top-right, fades after 5s)
- Optional chat message
- Queues multiple notifications

### Layout Files

| File | Purpose |
|------|---------|
| `auction_menu.layout` | Main menu: tabs, filter bar, table, bottom bar |
| `auction_menu_listing_row.layout` | Single row in the listings table (instantiated per listing) |
| `auction_menu_detail_panel.layout` | Right-side detail panel with preview and actions |
| `auction_menu_create_listing.layout` | Create listing dialog with item selection |
| `auction_menu_confirm_dialog.layout` | Confirmation modal (buy/bid/cancel) |

---

## 8. Access Points

### DME_AH_AuctionTerminal (4_World)
- Inherits from `HouseNoDestruct` (or similar static object)
- Admin-placeable via server tools (e.g., Expansion Object Spawner)
- Has `DME_AH_ActionOpenAuction` registered via `SetActions()`

### DME_AH_AuctionNPC (4_World)
- Inherits from `SurvivorBase` or Expansion trader base
- Static NPC, non-hostile, non-lootable
- Same action: `DME_AH_ActionOpenAuction`

### DME_AH_ActionOpenAuction (4_World)
- `ActionInteractBase` subclass
- Condition: target is `DME_AH_AuctionTerminal` or `DME_AH_AuctionNPC`
- OnExecuteClient: opens `DME_AH_AuctionMenu`
- Registered in `ActionConstructor`

---

## 9. Currency Adapter

Abstract base class `DME_AH_CurrencyAdapter` with three implementations:

### DME_AH_CurrencyExpansion
- Uses Expansion Market money system
- `GetBalance()`: Queries Expansion player wallet
- `Deduct()` / `Add()`: Modifies Expansion wallet

### DME_AH_CurrencyItem
- Uses physical in-game items as currency
- `CurrencyItemClass` from config (e.g., "MoneyRuble100")
- `GetBalance()`: Counts items in player inventory
- `Deduct()`: Removes items from inventory
- `Add()`: Spawns items in inventory

### DME_AH_CurrencyInternal
- Self-contained point system
- Balance stored in `PlayerData.json`
- No external dependencies

### Selection
Module reads `CurrencyType` from config and instantiates the correct adapter on init.

---

## 10. Server-Side Logic

### Expiry Check (DME_AH_AuctionManager)
- Runs every 60 seconds via module OnUpdate
- Iterates active listings where `ExpiresTimestamp <= currentTime`
- Auction with bids: Complete sale to highest bidder, notify both parties
- Auction without bids: Mark expired, return item to seller (pending pickup)
- Buy Now expired: Mark expired, return to seller (pending pickup)

### Transaction Processing
1. **Buy Now:** Deduct price from buyer → Add (price - fee) to seller → Mark sold → Create transaction → Notify both
2. **Place Bid:** Validate bid > current + increment → Refund previous bidder → Reserve bid amount from new bidder → Update listing → Notify outbid player
3. **Auction Complete:** Transfer reserved bid to seller (minus fee) → Mark sold → Create transaction → Notify both
4. **Cancel:** Only if no active bids → Mark cancelled → Return item to seller → Refund listing fee: No (non-refundable)

### Bid Reservation
When placing a bid, the amount is reserved (deducted) from the bidder immediately. If outbid, the full amount is refunded. This prevents players from spending money they've committed to bids.

---

## 11. File Structure

```
DME_Auction_House/
├── config.cpp
├── gui/
│   └── DME_AH/
│       └── layouts/
│           ├── auction_menu.layout
│           ├── auction_menu_listing_row.layout
│           ├── auction_menu_detail_panel.layout
│           ├── auction_menu_create_listing.layout
│           └── auction_menu_confirm_dialog.layout
├── scripts/
│   ├── Common/
│   │   └── DME_AH_Debug.c
│   ├── 3_Game/
│   │   └── DME_AH/
│   │       ├── Constants/
│   │       │   └── DME_AH_Constants.c
│   │       ├── Enums/
│   │       │   └── DME_AH_Enums.c
│   │       ├── Config/
│   │       │   ├── DME_AH_Config.c
│   │       │   └── DME_AH_CategoryConfig.c
│   │       ├── Data/
│   │       │   ├── DME_AH_Listing.c
│   │       │   ├── DME_AH_Bid.c
│   │       │   ├── DME_AH_Transaction.c
│   │       │   └── DME_AH_Category.c
│   │       ├── Module/
│   │       │   └── DME_AH_Module.c
│   │       ├── Manager/
│   │       │   ├── DME_AH_AuctionManager.c
│   │       │   └── DME_AH_DataStore.c
│   │       ├── Currency/
│   │       │   ├── DME_AH_CurrencyAdapter.c
│   │       │   ├── DME_AH_CurrencyExpansion.c
│   │       │   ├── DME_AH_CurrencyItem.c
│   │       │   └── DME_AH_CurrencyInternal.c
│   │       ├── Logger/
│   │       │   └── DME_AH_Logger.c
│   │       └── RPC/
│   │           └── DME_AH_RPCHandler.c
│   ├── 4_World/
│   │   └── DME_AH/
│   │       ├── Entities/
│   │       │   ├── DME_AH_AuctionTerminal.c
│   │       │   └── DME_AH_AuctionNPC.c
│   │       └── Actions/
│   │           ├── DME_AH_ActionOpenAuction.c
│   │           └── ActionConstructor.c
│   └── 5_Mission/
│       └── DME_AH/
│           ├── GUI/
│           │   ├── DME_AH_AuctionMenu.c
│           │   ├── DME_AH_ListingRow.c
│           │   ├── DME_AH_DetailPanel.c
│           │   ├── DME_AH_CreateListingDialog.c
│           │   ├── DME_AH_ConfirmDialog.c
│           │   └── DME_AH_NotificationHandler.c
│           ├── MissionBase.c
│           └── MissionGameplay.c
└── docs/
    └── superpowers/
        └── specs/
            └── 2026-04-17-auction-house-gui-design.md
```

---

## 12. Dependencies

| Dependency | Required | Purpose |
|-----------|----------|---------|
| Community Framework (CF) | Yes | RPC system, module system |
| DayZ Expansion | Optional | Currency adapter (if CurrencyType = "Expansion") |

`config.cpp` only lists `requiredAddons[] = {"DZ_Data"};` — CF dependency is runtime, not compile-time.
