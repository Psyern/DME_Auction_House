# Orchestrator-Auftrag: P2PMarket-Features in DME_Auction_House integrieren

Du bist Orchestrator. Deine Aufgabe: 5 voneinander unabhängige UI-Features
aus dem DayZExpansion-P2PMarket-Mod als **eigenständige, standalone**
Implementierungen in das DME_Auction_House (DME_AH) integrieren.

## Pfade

- **Ziel-Repo (schreiben):** `c:\Users\Administrator\Desktop\DME_Auction_House`
- **Referenz-Repo (NUR LESEN, nicht kopieren!):**
  `C:\Users\Administrator\Desktop\Mod Repositories\DayZExpansion\P2PMarket`
- **DayZ-Doku-Referenz (read-only):**
  `C:\Users\Administrator\Desktop\Mod Repositories\DAYZ_Enforce-Script-main`

## Harte Constraints (NICHT VERHANDELBAR)

1. **STANDALONE**: DME_AH darf **keinerlei** Abhängigkeit zu DayZExpansion,
   CFTools-Framework oder anderen Mods bekommen. Keine `Expansion*`-Klassen
   referenzieren, keine `requiredAddons` ergänzen außer DayZ-Vanilla.
2. **Re-Implementieren, nicht kopieren**: P2PMarket-Code dient nur als
   Architektur-Vorlage. Jede Klasse, jedes Layout-Widget, jede Methode wird
   unter DME_AH-Namenskonventionen **neu geschrieben**. Kein Copy-Paste von
   Expansion-Code.
3. **Naming**:
   - Prefix aller Klassen/Dateien: `DME_AH_`
   - Author: Psyern, Community: DeadmansEcho
   - Layout-Dateien: snake_case unter `gui/DME_AH/layouts/`
   - Script-Klassen: PascalCase mit `DME_AH_` Prefix
4. **EnforceScript-Regeln**: `string.ToLower()` modifiziert in-place, niemals
   den Rückgabewert zuweisen. Layer-Trennung 3_Game / 4_World / 5_Mission
   einhalten.
5. **Keine Premature Abstraction**: Keine Helper-Klassen "für später", kein
   feature-flag-Code, keine ungenutzten Hooks. Nur das, was die jeweilige
   Aufgabe braucht.

## Bestehender DME_AH-Stand (NICHT zerstören)

Diese Dateien existieren bereits und werden **erweitert**, nicht neu geschrieben:
- `gui/DME_AH/layouts/auction_menu.layout` (Hauptmenü)
- `gui/DME_AH/layouts/auction_menu_listing_row.layout`
- `gui/DME_AH/layouts/auction_menu_detail_panel.layout`
- `gui/DME_AH/layouts/auction_menu_create_listing.layout`
- `gui/DME_AH/layouts/auction_menu_confirm_dialog.layout`
- `scripts/5_Mission/DME_AH/GUI/DME_AH_AuctionMenu.c` (Hauptcontroller)
- `scripts/5_Mission/DME_AH/GUI/DME_AH_ListingRow.c`
- `scripts/5_Mission/DME_AH/GUI/DME_AH_DetailPanel.c`
- `scripts/5_Mission/DME_AH/GUI/DME_AH_CreateListingDialog.c`

## Phasen

### Phase 0: Setup & Kontext (sequenziell, du selbst)

1. Lies vollständig:
   - `gui/DME_AH/layouts/auction_menu.layout`
   - `scripts/5_Mission/DME_AH/GUI/DME_AH_AuctionMenu.c`
   - `scripts/5_Mission/DME_AH/GUI/DME_AH_ListingRow.c`
   - `scripts/5_Mission/DME_AH/GUI/DME_AH_DetailPanel.c`
2. Identifiziere die existierenden Anker-Widgets im Hauptmenü (Container-
   Namen, in die später neue Sub-Layouts eingehängt werden müssen — z.B.
   Listings-Container, Sidebar-Container, Detail-Container).
3. Schreibe die gefundenen Anker-Namen in den Briefing-Block für die Sub-Agents.
4. Erstelle einen Status-Report unter `tmp/orchestrator_state.md` mit:
   - Welche Anker existieren bereits
   - Welche fehlen und in welcher Phase ergänzt werden
   - Welche bestehenden Methoden in `DME_AH_AuctionMenu.c` von Phase-1-Agents
     gebraucht/erweitert werden müssen

### Phase 1: Parallel — 3 unabhängige Features

Dispatche in **einer Message** drei Sub-Agents (subagent_type=general-purpose).
Jeder Agent bekommt das volle Briefing aus Phase 0 plus seinen Sonderauftrag.

#### Agent A — Sortier-Header
**Vorlage:**
- `Mod Repositories\DayZExpansion\P2PMarket\GUI\layouts\expansion_p2pmarket_listings_header.layout`
- `Mod Repositories\DayZExpansion\P2PMarket\Scripts\5_Mission\DayZExpansion_P2PMarket\GUI\P2PMarket\ExpansionP2PMarketMenuListHeader.c`

**Liefert:**
- `gui/DME_AH/layouts/auction_menu_listings_header.layout` — Klickbare
  Spalten-Header (Name, Zeit, Preis, Verkäufer) mit Toggle-Sort-Pfeil
- `scripts/5_Mission/DME_AH/GUI/DME_AH_ListingsHeader.c` — Toggle-State pro
  Spalte, ruft Filter-Methoden im AuctionMenu auf
- Erweitert `DME_AH_AuctionMenu.c` um 4 Filter-Methoden:
  `Listings_Filter_NameAZ/ZA`, `Listings_Filter_PriceLowHigh/HighLow`,
  `Listings_Filter_TimeOldNew/NewOld`, `Listings_Filter_SellerAZ/ZA`
- Hängt das Header-Layout in den existierenden Listings-Container von
  `auction_menu.layout` ein (Anker-Name aus Phase 0)

**Definition of Done:** Layout lädt ohne Fehler; Klick auf jeden Header
toggelt sichtbar das Sort-Icon und ändert die Listings-Reihenfolge.

#### Agent B — Tooltip-System
**Vorlage:**
- `Mod Repositories\DayZExpansion\P2PMarket\GUI\layouts\expansion_p2pmarket_item_element_tooltip.layout`
- `Mod Repositories\DayZExpansion\P2PMarket\GUI\layouts\expansion_p2pmarket_item_element_tooltip_entry.layout`
- `...\GUI\P2PMarket\expansionp2pmarketmenuitemtooltip.c`
- `...\GUI\P2PMarket\expansionp2pmarketmenuitemtooltipentryiteminfo.c`

**Liefert:**
- `gui/DME_AH/layouts/auction_menu_tooltip.layout`
- `gui/DME_AH/layouts/auction_menu_tooltip_entry.layout`
- `scripts/5_Mission/DME_AH/GUI/DME_AH_ItemTooltip.c`
- `scripts/5_Mission/DME_AH/GUI/DME_AH_ItemTooltipEntry.c`

Tooltip wird beim Hover über einen Listing-Row-Hauptbereich angezeigt und
zeigt Icon-Entries für: Health-Stage, Quantity, Cargo-Item-Count,
ggf. Magazine-Inhalt.

**Definition of Done:** Hover über eine bestehende Listing-Row öffnet Tooltip
neben dem Cursor mit mindestens Health + Quantity Entries.

#### Agent C — Kategorie-Tree mit Count-Badge
**Vorlage:**
- `Mod Repositories\DayZExpansion\P2PMarket\GUI\layouts\expansion_p2pmarket_category.layout`
- `Mod Repositories\DayZExpansion\P2PMarket\GUI\layouts\expansion_p2pmarket_subcategory.layout`
- `...\GUI\P2PMarket\ExpansionP2PMarketMenuCategoryElement.c`

**Liefert:**
- `gui/DME_AH/layouts/auction_menu_category.layout` (expandierbar, mit Count-Badge)
- `gui/DME_AH/layouts/auction_menu_subcategory.layout`
- `scripts/5_Mission/DME_AH/GUI/DME_AH_CategoryElement.c`
- `scripts/5_Mission/DME_AH/GUI/DME_AH_SubCategoryElement.c`

Kategorie-Daten kommen aus dem bestehenden DME_AH-Datenmodell. Wenn keine
Kategorie-Struktur existiert, lies dafür `scripts/3_Game/DME_AH/Config/`
und nutze, was da ist — **nicht** ein neues Datenmodell erfinden, sondern
eskaliere an den Orchestrator wenn Daten fehlen.

**Definition of Done:** Sidebar zeigt mindestens 2 Kategorien mit Icon, Name
und korrekter Listing-Count-Anzeige; Klick filtert die Listings-Liste.

### Phase 2: Sequenziell nach Phase 1 — Detail-Panel-Erweiterung

**Erst starten, wenn alle 3 Phase-1-Agents fertig und integriert sind.**

Ein Agent (Agent D), der die existierende `auction_menu_detail_panel.layout`
und `DME_AH_DetailPanel.c` aufwertet:

**Vorlage:**
- `Mod Repositories\DayZExpansion\P2PMarket\GUI\layouts\expansion_p2pmarket_menu_details_view.layout`
- `...\GUI\P2PMarket\ExpansionP2PMarketMenuDetailsView.c`

**Liefert (Edits, keine neue Datei):**
- `auction_menu_detail_panel.layout` ergänzt um: ItemPreviewWidget (3D),
  Health-Bar, Quantity-Bar, Cargo-Count-Grid (zeigt Tooltip-Entries inline)
- `DME_AH_DetailPanel.c` ergänzt um die Bind-Methoden für die neuen Widgets

**Definition of Done:** Auswahl einer Listing zeigt rechts 3D-Item-Preview
+ Health-Bar; Werte stimmen mit der ausgewählten Listing überein.

### Phase 3: Polish — Item-Preview in Listing-Row (sequenziell)

**Nur wenn Phase 2 fehlerfrei läuft.**

Agent E erweitert die existierende `auction_menu_listing_row.layout` um ein
46x46px ItemPreviewWidget (Pattern aus
`expansion_p2pmarket_listing.layout` Z. 73-89 als Inspiration).

**Definition of Done:** Listings-Liste zeigt pro Zeile ein echtes 3D-Icon
des Items statt Text-only.

## Anti-Patterns (was Sub-Agents NICHT tun dürfen)

- Keine `Expansion*` Klassen, keine `JM_CF_*`-Referenzen
- Keine neuen Settings-Files, wenn eine bestehende DME_AH-Konfig sie
  aufnehmen kann
- Keine Refaktorierung von Code, der nicht zur Aufgabe gehört
- Keine Tests/Mocks erfinden — DayZ-Modding hat keine Unit-Test-Infrastruktur
- Keine Doku-Dateien (`.md`) erzeugen, außer der Status-Report des
  Orchestrators
- Keine `// TODO`-Kommentare, keine `// removed: ...`-Marker
- `string.ToLower()`-Rückgabewert NIEMALS zuweisen

## Verification & Übergabe

Nach jeder Phase:

1. Orchestrator führt aus:
   - `Skill check-enforce` (alle .c-Dateien auf EnforceScript-Compliance)
   - `Skill check-layers` (Layer-Trennung)
2. Liest die geänderten/neuen Dateien stichprobenartig auf:
   - Standalone-Constraint (kein `Expansion`-Token irgendwo)
   - Naming (alle neuen Klassen `DME_AH_*`)
3. Aktualisiert `tmp/orchestrator_state.md`
4. Berichtet Phasenabschluss in 2-3 Sätzen an User: was fertig ist, was als
   nächstes kommt, ob Blocker existieren

## Eskalationsregeln

Sub-Agents melden zurück (nicht selbst entscheiden), wenn:
- Ein Anker-Widget im Hauptmenü fehlt
- Daten für ein Feature im DME_AH-Modell nicht existieren (z.B. keine
  Kategorien definiert)
- Ein P2PMarket-Pattern eine Expansion-spezifische Klasse braucht, die
  in Vanilla nicht existiert (z.B. `ExpansionScrollerWidget`)
- Eine Bestandsdatei so geändert werden müsste, dass das andere
  Phase-1-Agents brechen würde

Der Orchestrator entscheidet bei Eskalation — wenn unklar, an den User fragen.

## Start

Beginne mit Phase 0. Berichte am Ende von Phase 0 die gefundenen Anker-Namen
und den Briefing-Block, BEVOR du Phase-1-Agents dispatchst.
