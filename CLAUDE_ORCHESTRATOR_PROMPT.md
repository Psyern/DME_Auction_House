# DME Auction House — Multi-Agent Orchestrator Prompt

> Paste this entire file into a fresh Claude Code / Claude Console session.
> You are the **orchestrator**. Read the whole brief, then dispatch the subagents
> listed at the bottom. Parallelize where phases are independent; serialize where
> noted. After each phase, stop and show the user a diff summary before moving on.

---

## 1. Project context (you walk in cold — read this first)

You are helping fix the **DME Auction House** DayZ mod (prefix `DME_AH`, author
Psyern, community DeadmansEcho).

**Primary working directory:** `c:\Users\Administrator\Desktop\DME_Auction_House`

**Reference mods (read-only, for pattern copying):**
- Expansion Market (bank, sell/buy flows, preview entities):
  `c:\Users\Administrator\Desktop\Mod Repositories\DayZExpansion\Market`
- DayZ EnforceScript docs:
  `c:\Users\Administrator\Desktop\Mod Repositories\DAYZ_Enforce-Script-main`

**DayZ-specific gotchas (do NOT forget these):**
- EnforceScript `string.ToLower()` mutates in place — never write `s = s.ToLower()`.
- Script layers: `3_Game` / `4_World` / `5_Mission`. `3_Game` cannot reference
  `PlayerBase` or anything from `4_World`; use `Man` + `EntityAI` there.
- Types declared in one layer are only visible to equal or later layers.
- After editing a `.c` file, verify it still parses with the server logs
  (`C:\Program Files (x86)\CFTools Software GmbH\Architect\Agent\deployments\DME_Test\log_storage\<instance>\script_*.log`).
- No `#pragma` / `#include` — files are auto-compiled per `config.cpp` `CfgMods`.

---

## 2. Bugs reported by the user (what we're fixing)

1. **Geld kommt nur aus Inventar, nicht aus Expansion-Bank.** Expected: use both,
   inventory first, bank as fallback (exactly like Expansion Trader's
   `AcceptWalletMoney`).
2. **Listing kann nicht normal angelegt werden.** Player click on inventory item
   fires a hardcoded RPC (`price=100, category=7`) instead of opening the
   existing `DME_AH_CreateListingDialog`.
3. **Kategorie-Sortierung funktioniert nicht.** Combo has no `OnChange` handler;
   value is read only on sort-button clicks.
4. **Items werden beim Listing NICHT aus dem Inventar entfernt** (and on BuyNow
   they are NOT spawned for the buyer either). Business logic half-missing.
5. **Kein Preis-Eingabefeld** (same root cause as #2 — dialog never opens).
6. **Keine Item-Vorschau**, nur Text. `ItemPreviewWidget` exists in layouts but
   `.SetItem()` is never called.

---

## 3. Decisions the user has already made (do NOT re-ask)

| # | Decision |
|---|----------|
| A | Create-Listing dialog takes **both Start Price AND Buy-Now Price** inputs (existing fields `editStartPrice` / `editBuyNowPrice` are already in layout). |
| B | **Item health and quantity MUST be preserved** across listing → sale. Otherwise exploit: list ruined item, buyer gets pristine. Store snapshot on listing creation, restore on delivery. |
| C | **Single items only** — no attachments, no cargo. If the seller's item has attachments or contents, REJECT the listing with a clear error message; force them to strip it first. |
| D | **Expansion currency uses inventory + bank** (like Expansion Trader with `AcceptWalletMoney=true`). Order: check inventory first, then top up from ATM. Deduct inventory first, then ATM. Balance = inventory + ATM. |

---

## 4. Architecture map (files you will touch)

| Concern | File |
|---------|------|
| Main auction menu (GUI controller) | `scripts/5_Mission/DME_AH/GUI/DME_AH_AuctionMenu.c` |
| Create-listing dialog (exists, unused) | `scripts/5_Mission/DME_AH/GUI/DME_AH_CreateListingDialog.c` |
| Detail panel | `scripts/5_Mission/DME_AH/GUI/DME_AH_DetailPanel.c` |
| Listing data class | `scripts/3_Game/DME_AH/Data/DME_AH_Listing.c` |
| Server business logic | `scripts/3_Game/DME_AH/Manager/DME_AH_AuctionManager.c` |
| Data persistence / pending pickups | `scripts/3_Game/DME_AH/Manager/DME_AH_DataStore.c` |
| RPC handler | `scripts/3_Game/DME_AH/RPC/DME_AH_RPCHandler.c` |
| Currency adapters | `scripts/3_Game/DME_AH/Currency/DME_AH_Currency*.c` |
| Category config | `scripts/3_Game/DME_AH/Config/DME_AH_CategoryConfig.c` |
| Server config (incl. `UseExpansionBank` flag to add) | `scripts/3_Game/DME_AH/Config/DME_AH_Config.c` |
| Layouts | `gui/DME_AH/layouts/auction_menu*.layout` |

**Reference patterns to copy from Expansion Market:**
- Bank read/write: `4_World/DayZExpansion_Market/Systems/Market/ExpansionMarketModule.c`
  around lines 4500–5230 (`GetPlayerATMData`, `RequestDepositMoney`, `RequestWithdrawMoney`).
- Item destroy with health/quantity preservation: `ExpansionMarketSellItem.c::DestroyItem()`.
- Item preview entity spawn: search for `ItemPreviewWidget.SetItem` in
  `ExpansionMarketMenu.c` / `ExpansionMarketMenuItem.c`.

---

## 5. Implementation phases (dispatch these)

### Phase 1 — Listing data + server business logic (BLOCKS Phase 2,3,4)

Do this **serially first, alone**. No parallel work until Phase 1 is done and
verified by user.

**Subagent: `phase1-backend`**
- Extend `DME_AH_Listing` with: `float ItemHealth`, `float ItemQuantity`,
  `int ItemLiquidType` (reserved, set 0 for now).
- Extend `DME_AH_PendingPickup` to also carry an item (rename `Type` semantics
  or add `bool IsItem`, `string ItemClassName`, `float ItemHealth`, `float ItemQuantity`).
- In `DME_AH_RPCHandler.RPC_CreateListing`:
  - Read an additional `int networkLow, int networkHigh` from the client (so we
    can look up the exact entity on the server — see DayZ `GetNetworkIDString`
    / `GetGame().GetObjectByNetworkId`).
  - Server-side: resolve the entity from the player's inventory. Validate it
    belongs to the sender, is in their inventory, has no attachments and no
    cargo. If any check fails, reply `FailedItemNotInInventory` or a new code
    `FailedItemHasAttachments`.
- `DME_AH_AuctionManager.CreateListing`:
  - Accept the resolved `EntityAI` argument.
  - Snapshot `GetHealth("", "")` and `GetQuantity()` into the listing.
  - After the listing record is persisted successfully, call
    `g_Game.ObjectDelete(item)` to remove it from the seller's inventory.
  - On any failure before deletion, make sure the item is NOT lost.
- `DME_AH_AuctionManager.BuyNow` and `CompleteAuction`:
  - After money transfer succeeds, spawn the item for the buyer:
    `buyer.GetInventory().CreateInInventory(listing.ItemClassName)`.
    Restore health via `item.SetHealth("", "", listing.ItemHealth)` and quantity
    via `item.SetQuantity(listing.ItemQuantity)` if > 0.
  - If the buyer is offline OR `CreateInInventory` returns null (inventory
    full), create a `DME_AH_PendingPickup` that carries the full item snapshot
    and deliver on next pickup claim.
- `DME_AH_AuctionManager.CancelListing`:
  - Must also return the item to the seller (same pattern — inventory or pending pickup).
- Add `EDME_AH_ResultCode.FailedItemHasAttachments` to the enum and translate
  it in `DME_AH_AuctionMenu.GetErrorMessage`.

**Verification:** Ask the user to reboot the test server, list an item, buy it
from a second account, cancel a listing — verify item + health + quantity
survive round trip, and that listing with attachments is rejected.

---

### Phase 2 — GUI (can run in PARALLEL with Phase 3 after Phase 1 is merged)

**Subagent: `phase2-gui`**
- In `DME_AH_AuctionMenu.OnInventoryItemSelected()` (currently lines ~469-484):
  Stop sending the hardcoded RPC. Instead:
  1. Instantiate `DME_AH_CreateListingDialog` via
     `g_Game.GetUIManager().EnterScriptedMenu()`.
  2. Pass the selected `EntityAI` to the dialog via a setter
     (`dialog.SetSelectedItem(item)`), so it can show preview + read networkID
     when creating.
  3. Dialog's "Create" button sends the RPC with user-entered price, category,
     duration — and the item's NetworkID.
- `DME_AH_AuctionMenu.PopulateCategoryCombo()`:
  - Replace hardcoded list with iteration over
    `DME_AH_Module.GetInstance().GetCategoryConfig().Categories`.
- Hook category-combo change:
  - `XComboBoxWidget` fires `OnChange` — handle it in `OnChange` override of
    the menu, call `m_CurrentPage = 0; RequestListings();`.
- `DME_AH_CreateListingDialog.SendCreateListing`:
  - Write both `startPrice` and `buyNowPrice` from the two edit boxes.
  - Write the selected item's NetworkID (2 ints) into the RPC.
  - Validate: if listing type = BuyNow, require `startPrice > 0` (that IS the
    buy-now price in BuyNow-only mode). If Auction, require `startPrice > 0`.
    If AuctionWithBuyNow, require both and `buyNowPrice > startPrice`.
- `DME_AH_DetailPanel.Show()`:
  - After showing text, spawn a client-only preview entity at a hidden
    location (e.g. `"0 0 0"` with `ECE_LOCAL | ECE_NOLIFETIME`) of
    `row.ItemClassName`, call `m_ItemPreview.SetItem(previewEntity)`.
  - Delete the previous preview entity in `Hide()` to avoid leaks.
  - Same pattern in `DME_AH_CreateListingDialog` when a row is selected.

**Verification:** User opens dialog, enters prices, picks category, sees preview.

---

### Phase 3 — Expansion bank currency (can run in PARALLEL with Phase 2)

**Subagent: `phase3-currency`**
- `DME_AH_CurrencyExpansion.c` must call into `ExpansionMarketModule`, which
  lives in `4_World`. Two options — pick the cleaner one:
  - **Option A (preferred):** Move `DME_AH_CurrencyExpansion.c` from `3_Game`
    to `4_World/DME_AH/Currency/`. Update `config.cpp` script path list.
  - **Option B:** Keep it in `3_Game` but guard every Expansion call with
    `#ifdef EXPANSIONMOD_MARKET` — only works if EXPANSIONMOD_MARKET is a real
    preprocessor define in Expansion's config. Check
    `DayZExpansion/Market/config.cpp` for `defines=`. If not present, take
    Option A.
- Add `bool UseExpansionBank = true;` to `DME_AH_Config`.
- `GetBalance(uid)`:
  ```
  int total = CountMoneyInInventory(player);
  #ifdef EXPANSIONMOD_MARKET
  if (m_Config.UseExpansionBank) {
      ExpansionMarketATM_Data atm = ExpansionMarketModule.GetInstance().GetPlayerATMData(uid);
      if (atm) total += atm.GetMoney();
  }
  #endif
  return total;
  ```
- `Deduct(uid, amount)`:
  - First try inventory (existing code).
  - If inventory had less than requested, deduct remainder from ATM:
    `atm.RemoveMoney(remainder); atm.Save();`.
  - If still not enough, roll back inventory deduction (re-spawn banknotes) and
    return false.
- `Add(uid, amount)`:
  - Keep current behavior (spawn banknotes in inventory). Optional: if
    inventory spawn fails (full), fall back to ATM deposit.
- Make sure `DME_AH_CurrencyExpansion` still compiles when Expansion is NOT
  loaded — all Expansion type references must be inside `#ifdef` blocks, and
  the file must not `import` Expansion at top level.

**Verification:** Set `UseExpansionBank=true`, player has 50 in inventory + 500
in bank, lists item with 200 fee → inventory drained to 0 + 150 deducted from
bank. Reverse: sell for 300 → 300 banknotes spawn in inventory.

---

### Phase 4 — Polish & logging (after Phase 1+2+3 merged)

**Subagent: `phase4-polish`**
- Cleaner error messages for all new result codes in
  `DME_AH_AuctionMenu.GetErrorMessage`.
- Info log on item delivery: seller UID, buyer UID, classname, health, quantity.
- Unit test checklist doc saved next to the prompt — `TEST_PLAN.md` — with
  every manual scenario the user should run before calling it done.
- Optional: a small "Balance: X (Y bank)" split display in the menu if the
  balance can be decomposed.

---

## 6. Orchestration rules (for you, the top-level Claude)

1. **Do Phase 1 alone and stop.** Report diffs, wait for user OK.
2. Once Phase 1 is merged, dispatch Phase 2 and Phase 3 **in parallel** as two
   separate subagents (single message, two `Agent` tool calls).
3. After both return, run Phase 4.
4. Never amend a commit unless the user asks. New commits per phase.
5. Use the `Explore` subagent for any file you haven't seen yet — don't guess.
6. After every phase, show a concise "what changed / how to test" summary. No
   narrated thinking, no essays.
7. If a subagent returns an uncertain result ("I think this might work"),
   re-dispatch with a sharper prompt rather than trusting it.

---

## 7. Subagent prompt templates (copy-paste ready)

### Template: `phase1-backend`

```
You are implementing Phase 1 of the DME Auction House fix. Read section 4-5
of c:\Users\Administrator\Desktop\DME_Auction_House\CLAUDE_ORCHESTRATOR_PROMPT.md
for the full spec. Your scope:

- Add ItemHealth and ItemQuantity to DME_AH_Listing.
- Extend DME_AH_PendingPickup to carry item snapshots.
- Add NetworkID-based entity lookup to RPC_CreateListing.
- Implement item removal on CreateListing, item delivery on BuyNow /
  CompleteAuction / CancelListing, with offline-pickup fallback.
- Reject listings whose item has attachments or cargo.
- Add EDME_AH_ResultCode.FailedItemHasAttachments.

Do NOT touch: GUI files, currency files, layouts. Those are other subagents.

When done, list the exact files you changed and the one-line reason for each.
```

### Template: `phase2-gui`

```
You are implementing Phase 2 of the DME Auction House fix. Read section 4-5
of c:\Users\Administrator\Desktop\DME_Auction_House\CLAUDE_ORCHESTRATOR_PROMPT.md
for the full spec. Your scope:

- Open DME_AH_CreateListingDialog from DME_AH_AuctionMenu.OnInventoryItemSelected()
  instead of firing the hardcoded RPC.
- Wire category combo OnChange to RequestListings.
- Populate category combo from DME_AH_CategoryConfig.
- Send NetworkID + both prices in RPC_CreateListing from the dialog.
- Add client-only preview entity spawn in DME_AH_DetailPanel.Show and
  DME_AH_CreateListingDialog inventory select; clean up on Hide/Close.

Do NOT touch: server-side auction manager, currency, data store. Those are
other subagents. Phase 1 has already extended the RPC write format — match it
exactly.

When done, list the exact files you changed.
```

### Template: `phase3-currency`

```
You are implementing Phase 3 of the DME Auction House fix. Read section 4-5
of c:\Users\Administrator\Desktop\DME_Auction_House\CLAUDE_ORCHESTRATOR_PROMPT.md
for the full spec. Your scope:

- Give DME_AH_CurrencyExpansion access to the Expansion ATM data. Pick
  Option A (move to 4_World) or Option B (#ifdef guards) — justify your choice
  in 1 sentence. Verify the macro exists before picking B.
- Implement inventory-first, bank-fallback logic for GetBalance / Deduct /
  Add, with full rollback on partial deduct failure.
- Add UseExpansionBank flag to DME_AH_Config with default true.
- File must still compile without Expansion loaded.

Do NOT touch: GUI, auction manager, listing data. Those are other subagents.

When done, list the exact files you changed.
```

### Template: `phase4-polish`

```
You are implementing Phase 4. Read CLAUDE_ORCHESTRATOR_PROMPT.md section 5.
Scope: error-message strings for new result codes, delivery log lines, and a
TEST_PLAN.md covering every scenario (create, list with attachments, buy,
cancel, offline buyer, empty bank, full inventory, Expansion not loaded).

When done, list the files you changed.
```

---

## 8. Quick smoke-test commands (user-facing, after each phase)

```
1. Restart test server.
2. Join with account A, spawn some money + a gun.
3. Open auction NPC → Sell tab → pick gun → enter prices → Create.
4. Confirm gun is gone from inventory.
5. Join with account B, open auction NPC → Buy.
6. Confirm gun arrives with same health/quantity.
7. Confirm money flow: inventory drained first, then bank.
```

---

*End of orchestrator prompt.*
