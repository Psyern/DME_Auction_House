# DME Auction House - Implementation Plan

Based on: `docs/superpowers/specs/2026-04-17-auction-house-gui-design.md`

## Phase 1: Foundation (config.cpp + Constants + Enums + Data Classes)
1. Create `config.cpp` with CfgPatches and CfgMods
2. Create `scripts/Common/DME_AH_Debug.c`
3. Create `scripts/3_Game/DME_AH/Constants/DME_AH_Constants.c`
4. Create `scripts/3_Game/DME_AH/Enums/DME_AH_Enums.c`
5. Create `scripts/3_Game/DME_AH/Data/DME_AH_Listing.c`
6. Create `scripts/3_Game/DME_AH/Data/DME_AH_Bid.c`
7. Create `scripts/3_Game/DME_AH/Data/DME_AH_Transaction.c`
8. Create `scripts/3_Game/DME_AH/Data/DME_AH_Category.c`

## Phase 2: Config + Logger + DataStore
9. Create `scripts/3_Game/DME_AH/Config/DME_AH_Config.c`
10. Create `scripts/3_Game/DME_AH/Config/DME_AH_CategoryConfig.c`
11. Create `scripts/3_Game/DME_AH/Logger/DME_AH_Logger.c`
12. Create `scripts/3_Game/DME_AH/Manager/DME_AH_DataStore.c`

## Phase 3: Currency Adapters
13. Create `scripts/3_Game/DME_AH/Currency/DME_AH_CurrencyAdapter.c`
14. Create `scripts/3_Game/DME_AH/Currency/DME_AH_CurrencyExpansion.c`
15. Create `scripts/3_Game/DME_AH/Currency/DME_AH_CurrencyItem.c`
16. Create `scripts/3_Game/DME_AH/Currency/DME_AH_CurrencyInternal.c`

## Phase 4: Auction Manager + RPC Handler + Module
17. Create `scripts/3_Game/DME_AH/Manager/DME_AH_AuctionManager.c`
18. Create `scripts/3_Game/DME_AH/RPC/DME_AH_RPCHandler.c`
19. Create `scripts/3_Game/DME_AH/Module/DME_AH_Module.c`

## Phase 5: World Entities + Actions (4_World)
20. Create `scripts/4_World/DME_AH/Entities/DME_AH_AuctionTerminal.c`
21. Create `scripts/4_World/DME_AH/Entities/DME_AH_AuctionNPC.c`
22. Create `scripts/4_World/DME_AH/Actions/DME_AH_ActionOpenAuction.c`
23. Create `scripts/4_World/DME_AH/Actions/ActionConstructor.c`

## Phase 6: Layout Files (GUI)
24. Create `gui/DME_AH/layouts/auction_menu.layout`
25. Create `gui/DME_AH/layouts/auction_menu_listing_row.layout`
26. Create `gui/DME_AH/layouts/auction_menu_detail_panel.layout`
27. Create `gui/DME_AH/layouts/auction_menu_create_listing.layout`
28. Create `gui/DME_AH/layouts/auction_menu_confirm_dialog.layout`

## Phase 7: GUI Classes (5_Mission)
29. Create `scripts/5_Mission/DME_AH/GUI/DME_AH_ListingRow.c`
30. Create `scripts/5_Mission/DME_AH/GUI/DME_AH_DetailPanel.c`
31. Create `scripts/5_Mission/DME_AH/GUI/DME_AH_CreateListingDialog.c`
32. Create `scripts/5_Mission/DME_AH/GUI/DME_AH_ConfirmDialog.c`
33. Create `scripts/5_Mission/DME_AH/GUI/DME_AH_NotificationHandler.c`
34. Create `scripts/5_Mission/DME_AH/GUI/DME_AH_AuctionMenu.c`

## Phase 8: Mission Integration (5_Mission)
35. Create `scripts/5_Mission/DME_AH/MissionBase.c`
36. Create `scripts/5_Mission/DME_AH/MissionGameplay.c`

## Phase 9: Review
37. EnforceScript compliance check on all files
38. Script layer placement verification
