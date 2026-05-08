# DME_AH P2PMarket-Integration — Orchestrator State

Stand: 2026-05-08 — Phase 0 abgeschlossen

## Pfade

- Ziel-Repo: `c:\Users\Administrator\Desktop\DME_Auction_House`
- Referenz (read-only): `C:\Users\Administrator\Desktop\Mod Repositories\DayZExpansion\P2PMarket`
- DayZ-Doku-Referenz: `C:\Users\Administrator\Desktop\Mod Repositories\DAYZ_Enforce-Script-main`

## Phase 0 — Anker im Hauptmenü

Datei: `gui/DME_AH/layouts/auction_menu.layout`

| Anker (Widget-Name) | Position / Size | Zweck | Phase, die ihn nutzt |
|---|---|---|---|
| `RootPanel` | 0 0 / 1 1 | Wurzel | — |
| `MainFrame` | 0.05 0.03 / 0.9 0.94 | Inner Frame | — |
| `txtTitle` / `btnSellItem` / `txtBalance` / `btnClose` | Top-Header | bestehender Header | — |
| `TabBar` | 0.02 0.055 / 0.96 0.045 | Tabs (Marketplace/MyListings/MyBids) | — |
| `FilterBar` | 0.02 0.11 / 0.96 0.045 | Search + Combo + Sortbuttons (legacy) | (1A entkoppelt; bleibt vorerst) |
| **`ListHeader`** | 0.02 0.165 / 0.7 0.035 | statische Spalten-Header — **Anker für Phase 1A** | **Phase 1A** |
| **`ListContainer`** | 0.02 0.2 / 0.7 0.74 | wraps `lstListings` (TextListboxWidget) — **Anker für Phase 1B (Tooltip-Trigger)** | **Phase 1B**, **Phase 3** |
| **`DetailPanel`** | 0.73 0.165 / 0.25 0.775 | Right-side Detail — **Anker für Phase 2** | **Phase 2** |
| `BottomBar` | 0.02 0.95 / 0.96 0.04 | Pagination | — |
| `LoadingOverlay` | 0 0 / 1 1 | Loading | — |

## Fehlende Anker (von Phase 1C zu ergänzen)

- **CategorySidebar**: existiert NICHT. Phase 1C muss `auction_menu.layout` modifizieren (zwischen FilterBar und ListHeader eine neue `PanelWidgetClass CategorySidebar` einfügen) und `ListHeader` + `ListContainer` in der x-Position nach rechts schieben (0.02 → 0.18) und in der Breite kürzen (0.7 → 0.54). Existierender `comboCategory` in FilterBar wird auf `visible 0` gesetzt (nicht entfernt — das würde die Bind-Logik in `DME_AH_AuctionMenu.c` brechen).

## Bestehende Methoden in `DME_AH_AuctionMenu.c`, die Phase-1-Agents nutzen/erweitern

| Methode | Zweck | Erweitert durch |
|---|---|---|
| `RequestListings()` (L. 508) | Sendet Filter+Sort+Page-RPC | 1A (neue Sort-Modi triggern), 1C (CategoryID-Quelle wechseln) |
| `OnReceiveListings()` (L. 599) | Füllt `m_LstListings` + `m_ListingRows` | 1B (Tooltip-Hookup), 3 (per-row preview) |
| `Init()` (L. 86) | Bind-Phase | alle drei Phase-1-Agents (additive Bind-Calls) |
| `OnClick()` (L. 201) | Click-Dispatch | alle drei Phase-1-Agents (additive Branches) |
| `m_CurrentSortMode`, `m_ComboCategory` | State | 1A, 1C |

## Datenmodell, das Agents nutzen dürfen (NICHT neu erfinden)

| Datei | Klasse | Inhalt |
|---|---|---|
| `scripts/3_Game/DME_AH/Data/DME_AH_Category.c` | `DME_AH_Category` | `int CategoryID, string DisplayName, string Icon, ref array<string> ClassFilters` |
| `scripts/3_Game/DME_AH/Config/DME_AH_CategoryConfig.c` | `DME_AH_CategoryConfig` | `array<ref DME_AH_Category> Categories`, `CreateDefaults()` mit 7 Vanilla-Kategorien (Weapons/Clothing/Medical/Food/Vehicles/Building/Other) |
| `scripts/5_Mission/DME_AH/GUI/DME_AH_ListingRow.c` | `DME_AH_ListingRow` | `string ItemClassName, string ItemName, int Price, int ListingType, string SellerName, int ExpiresTimestamp, int BuyNowPrice, int BidCount, string CurrentBidderName` |

## Konflikt-Vermeidung Phase 1 (parallele Sub-Agents)

Damit drei parallele Agents nicht in dieselben Dateien schreiben:

- **Phase 1A**: Erstellt NUR neue Dateien. Bearbeitet `DME_AH_AuctionMenu.c` additiv (4 neue `Listings_Filter_*`-Methoden; 1 neuer Member `m_ListingsHeader`; 1 neue Init-Zeile; 1 neuer OnClick-Branch). Bearbeitet **NICHT** `auction_menu.layout`.
- **Phase 1B**: Erstellt NUR neue Dateien. Bearbeitet `DME_AH_AuctionMenu.c` additiv (1 neuer Member `m_ItemTooltip`; 1 neue Init-Zeile; ggf. 1 neuer OnMouseEnter/OnSelect-Hook). Bearbeitet **NICHT** `auction_menu.layout`.
- **Phase 1C**: Erstellt neue Dateien UND bearbeitet `auction_menu.layout` (verschiebt ListHeader/ListContainer + fügt CategorySidebar ein). Bearbeitet `DME_AH_AuctionMenu.c` additiv (1 neuer Member `m_CategorySidebar`; 1 neue Init-Zeile; 1 neuer OnClick/OnChange-Branch; ersetzt die `GetSelectedCategoryID()`-Quelle so, dass sie zuerst Sidebar-Selection abfragt und dann auf Combo zurückfällt).

Wenn Konflikte auftreten, eskaliert der jeweilige Agent statt zu raten.

## Briefing-Block für Phase-1-Agents (gemeinsamer Kontext)

```
GEMEINSAMER PROJEKT-KONTEXT
===========================
Mod: DME_Auction_House (DME_AH), Author Psyern, Community DeadmansEcho.
Sprache: EnforceScript (DayZ Standalone). Engine: Enfusion.
Pfade:
  Ziel-Repo:         c:\Users\Administrator\Desktop\DME_Auction_House
  Referenz:          C:\Users\Administrator\Desktop\Mod Repositories\DayZExpansion\P2PMarket  (NUR LESEN)
  DayZ-Doku:         C:\Users\Administrator\Desktop\Mod Repositories\DAYZ_Enforce-Script-main (NUR LESEN)

HARTE CONSTRAINTS
=================
- STANDALONE: keine Expansion*-, JM_*-, CF_*-Klassen, kein requiredAddons ergaenzen.
- Re-Implementieren, nicht copy-pasten. P2PMarket nur als Vorlage.
- Naming: Klassen + Dateien mit DME_AH_-Prefix; Layouts snake_case unter gui/DME_AH/layouts/.
- EnforceScript: kein string.ToLower()-Returnwert zuweisen; Layer-Trennung 3_Game/4_World/5_Mission.
- Keine Doku/.md, keine TODO-Marker, keine // removed-Marker, keine Tests.
- Nicht zerstoeren: gui/DME_AH/layouts/auction_menu.layout, auction_menu_listing_row.layout,
  auction_menu_detail_panel.layout, auction_menu_create_listing.layout,
  auction_menu_confirm_dialog.layout, scripts/5_Mission/DME_AH/GUI/DME_AH_AuctionMenu.c,
  DME_AH_ListingRow.c, DME_AH_DetailPanel.c, DME_AH_CreateListingDialog.c.

ANKER IM HAUPTMENUE (auction_menu.layout)
=========================================
- ListHeader        (PanelWidget, position 0.02 0.165, size 0.7 0.035)
- ListContainer     (PanelWidget, enthaelt lstListings TextListboxWidget)
- DetailPanel       (PanelWidget, position 0.73 0.165, size 0.25 0.775)
- FilterBar         (enthaelt comboCategory XComboBoxWidget — wird ggf. fallback bleiben)
- TabBar            (Marketplace/MyListings/MyBids Tabs)

DATENMODELL (existiert; NICHT neu erfinden)
===========================================
- DME_AH_ListingRow: ItemClassName, ItemName, Price, ListingType, SellerName, ExpiresTimestamp,
  BuyNowPrice, BidCount, CurrentBidderName.
- DME_AH_Category: CategoryID, DisplayName, Icon, ClassFilters.
- DME_AH_CategoryConfig.CreateDefaults(): 7 Kategorien (Weapons/Clothing/Medical/Food/Vehicles/Building/Other).
- DME_AH_Module.GetInstance().GetCategoryConfig() liefert die Live-Config.

ESKALATION
==========
Wenn Daten fehlen, ein Anker fehlt, ein P2PMarket-Pattern eine Expansion-Klasse braucht
(z.B. ExpansionScrollerWidget) oder eine Bestandsdatei so geaendert werden muesste, dass
ein anderer Phase-1-Agent bricht: STOPPEN und im Result-Bericht "ESCALATION:" markieren.
```

## Status

- [x] Phase 0 (Setup & Kontext)
- [x] Phase 1A (Sortier-Header)
- [x] Phase 1B (Tooltip)
- [x] Phase 1C (Kategorie-Tree)
- [ ] Phase 2 (Detail-Panel-Upgrade) — laeuft als naechstes
- [ ] Phase 3 (Item-Preview in Listing-Row) — sequentiell

## Phase 1 — Integration Notes

Alle drei Phase-1-Agents komplett, Patches integriert in:
- `scripts/3_Game/DME_AH/Enums/DME_AH_Enums.c` (+ SellerAsc/SellerDesc)
- `scripts/3_Game/DME_AH/Manager/DME_AH_AuctionManager.c` (+ Seller-Sort-Branches)
- `scripts/5_Mission/DME_AH/GUI/DME_AH_AuctionMenu.c` (Members, Init-Block, OnClick-Top-Branch, OnListingSelected-Tooltip-Show, SwitchTab/Destruktor-Tooltip-Hide, 8 Listings_Filter_*-Methoden, GetSelectedCategoryID-Replace, PopulateCategorySidebar/RefreshCategoryCounts/SelectCategory, RefreshCategoryCounts-Call in OnReceiveListings)

Neue Dateien:
- 5 Layouts: auction_menu_listings_header / _tooltip / _tooltip_entry / _category / _subcategory
- 5 Skripte: DME_AH_ListingsHeader / _ItemTooltip / _ItemTooltipEntry / _CategoryElement / _SubCategoryElement

Layout-Aenderung an `auction_menu.layout`: CategorySidebar eingefuegt; ListHeader/ListContainer x-Position nach rechts geschoben; comboCategory `visible 0` (Bind-Logik bleibt).

Compliance-Checks bestanden:
- Keine `Expansion`/`JM_`/`CF_`-Tokens in neuen Dateien.
- Keine `ref` als Methodenparameter, keine `string.ToLower()`-Returnwert-Zuweisung.
- AuctionMenu.c top-level Klammern balanciert (2 Klassen).

Verbleibende Punkte (vom Phase-1C-Agent eskaliert, nicht-blockierend):
- `DME_AH_Category` hat kein `ParentID`-Feld. Die `auction_menu_subcategory.layout` + `DME_AH_SubCategoryElement.c` sind ausgeliefert aber INERT (kein Parent-Mapping vorhanden). User-Entscheidung in spaeterer Phase, ob ein `ParentID` zum Datenmodell ergaenzt wird.
