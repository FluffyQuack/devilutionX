#include "../all.h"
#include "hotbar.h"
#include "../render/sdl-render.h"
#include "../textures/textures.h"
#include "../options.h"

DEVILUTION_BEGIN_NAMESPACE

hotbarSlot_s hotbarSlots[HOTBAR_SLOTS];
static Sint32 hotbarChanges[HOTBAR_SLOTS]; //For temporarily storing changes for the hotbar
int selectedHotbarSlot = -1;
int selectedHotbarSlot_forLinking = -1;
const InvXY hotbarSlotLocations[HOTBAR_SLOTS] = {
	{ 205, 33 },
	{ 234, 33 },
	{ 263, 33 },
	{ 292, 33 },
	{ 321, 33 },
	{ 350, 33 },
	{ 379, 33 },
	{ 408, 33 }
};

static int Hotbar_ReturnInvListPositionUsingItemLink(int slot)
{
	if (hotbarSlots[slot].itemLink < INVITEM_INV_FIRST || hotbarSlots[slot].itemLink > INVITEM_INV_LAST)
		return -1;
	int invGridValue = abs(plr[myplr].InvGrid[hotbarSlots[slot].itemLink - INVITEM_INV_FIRST]);
	if (invGridValue == 0)
		return -1;
	return invGridValue - 1;
}

void Hotbar_ResetSlots() //Reset everything to do with the hotbar
{
	for (int i = 0; i < HOTBAR_SLOTS; i++) {
		hotbarSlots[i].itemLink = -1;
		hotbarSlots[i].spellLink = -1;
		hotbarSlots[i].spellLinkType = 0;
	}
	selectedHotbarSlot = -1;
	selectedHotbarSlot_forLinking = -1;
}

bool Hotbar_SlotSelection() //Update hotbar selection based on mouse movement
{
	for (int i = 0; i < HOTBAR_SLOTS; i++) {
		int xo = PANEL_LEFT;
		int yo = PANEL_TOP;

		if (MouseX >= hotbarSlotLocations[i].X + xo
		    && MouseX < hotbarSlotLocations[i].X + xo + (INV_SLOT_SIZE_PX + 1)
		    && MouseY >= hotbarSlotLocations[i].Y + yo - (INV_SLOT_SIZE_PX + 1)
		    && MouseY < hotbarSlotLocations[i].Y + yo) {
			selectedHotbarSlot = i;
			return true;
		}
	}

	selectedHotbarSlot = -1;
	return false;
}

void Hotbar_UpdateItemLink(int oldLink, int newLink)
{
	for (int i = 0; i < HOTBAR_SLOTS; i++) {
		if (hotbarSlots[i].itemLink == oldLink)
			hotbarSlots[i].itemLink = newLink;
	}
}

void Hotbar_LinkSpellToHotbar(Uint32 spell, Uint32 spellType)
{
	if (selectedHotbarSlot_forLinking != -1) {
		hotbarSlots[selectedHotbarSlot_forLinking].itemLink = -1;
		hotbarSlots[selectedHotbarSlot_forLinking].spellLink = spell;
		hotbarSlots[selectedHotbarSlot_forLinking].spellLinkType = spellType;
		selectedHotbarSlot_forLinking = -1;
	}
}

bool Hotbar_LinkItemToHotbar(int invItem)
{
	if (invItem >= INVITEM_INV_FIRST && invItem <= INVITEM_INV_LAST) { //invItem argument is the item's position in invList array, but we want to use position for invGrid array
		invItem -= INVITEM_INV_FIRST;
		for (int i = 0; i < NUM_INV_GRID_ELEM; i++) {
			if (plr[myplr].InvGrid[i] == invItem + 1) {
				invItem = i + INVITEM_INV_FIRST;
				break;
			}

			if (i == NUM_INV_GRID_ELEM - 1) //If true, we couldn't find the item in invGrid. (This should never happen)
				invItem = -1;
		}
	}

	if (selectedHotbarSlot_forLinking != -1 && invItem != -1) {
		hotbarSlots[selectedHotbarSlot_forLinking].itemLink = invItem;
		hotbarSlots[selectedHotbarSlot_forLinking].spellLink = -1;
		hotbarSlots[selectedHotbarSlot_forLinking].spellLinkType = 0;
		selectedHotbarSlot_forLinking = -1;
		return true;
	}

	return false;
}

bool Hotbar_LeftMouseDown()
{
	if (selectedHotbarSlot != -1) {
		if (selectedHotbarSlot_forLinking == selectedHotbarSlot)
			selectedHotbarSlot_forLinking = -1;
		else
			selectedHotbarSlot_forLinking = selectedHotbarSlot;
		return true;
	}
	return false;
}

static bool IsWeapon(ItemStruct *item)
{
	if (item->_itype == ITYPE_SWORD || item->_itype == ITYPE_AXE || item->_itype == ITYPE_BOW || item->_itype == ITYPE_MACE || item->_itype == ITYPE_SHIELD || item->_itype == ITYPE_STAFF)
		return true;
	return false;
}

static bool IsRing(ItemStruct *item)
{
	return item->_itype == ITYPE_RING;
}

static bool IsAmulet(ItemStruct *item)
{
	return item->_itype == ITYPE_AMULET;
}

static bool IsHelmet(ItemStruct *item)
{
	return item->_itype == ITYPE_HELM;
}

static bool IsArmour(ItemStruct *item)
{
	if (item->_itype == ITYPE_LARMOR || item->_itype == ITYPE_MARMOR || item->_itype == ITYPE_HARMOR)
		return true;
	return false;
}

static bool IsEquippableItem(ItemStruct *item)
{
	if (IsWeapon(item) || IsRing(item) || IsAmulet(item) || IsHelmet(item) || IsArmour(item))
		return true;
	return false;
}

static int TargetBodySlot(int from, int to, bool *targetSlotIsOccupied) 
{
	for (int i = from; i <= to; i++) {
		ItemStruct *bodySlot = &plr[myplr].InvBody[i];
		if (bodySlot->isEmpty()) {
			*targetSlotIsOccupied = false;
			return i;
		}
	}
	*targetSlotIsOccupied = true;
	return from;
}

static bool IsTwoHanded(ItemStruct *item)
{
	if (item->_iLoc == ILOC_TWOHAND && !(plr[myplr]._pClass == PC_BARBARIAN && (item->_itype == ITYPE_SWORD || item->_itype == ITYPE_MACE))) //Barbarian always hold swords and maces in one hand
		return true;
	return false;
}

static void RemoveItemFromInvGrid(Sint8 *invGrid, int item)
{
	for (int i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (invGrid[i] == item || invGrid[i] == -item)
			invGrid[i] = 0;
	}
}

static int FindSpotForItemInInvGrid(Sint8 *invGrid, int *positionPointer, int sizeX, int sizeY)
{
	//Position 0 equals to topleft. NUM_INV_GRID_ELEM - 1 is bottomright
	//This function returns the "real" inventory grid position of an item, which is at the item's bottleft-most grid position

	int startPos = 0;
	if (positionPointer != nullptr)
		startPos = *positionPointer;

	const int pitch = 10; //TODO: Put this somewhere else
	for (int i = startPos; i < NUM_INV_GRID_ELEM; i++) {
		
		if ((i + (sizeX - 1)) % pitch < i % pitch) //Check if item's horizontal size would exceed x boundary when on this grid tile
			continue;
		if (i + (sizeX - 1) + ((sizeY - 1) * pitch) >= NUM_INV_GRID_ELEM) //Check if item would exceed vertical and total boundary if on this grid tile
			break;

		int verticalOffset = 0;
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				if (invGrid[i + x + verticalOffset] != 0) //Check if slot is already occupied
					goto skip;
			}
			if (y == sizeY - 1) { //If we get this far, then this position is a valid spot for the item
				if (positionPointer != nullptr)
					*positionPointer = i;
				return i + verticalOffset;
			}
			verticalOffset += pitch;
		}
	skip:
		continue;
	}
	return -1;
}

static void AddItemToInvGrid(Sint8 *invGrid, int slot, int sizeX, int sizeY, int invListLink) //Slot corresponds to the bottomleft position of the item we're adding
{
	const int pitch = 10; //TODO: Put this somewhere else
	int startingPos = slot;
	for (int y = 0; y < sizeY; y++) {
		for (int x = 0; x < sizeX; x++) {
			int position = startingPos + x - (y * pitch);
			if (x == 0 & y == 0)
				invGrid[position] = (invListLink + 1);
			else
				invGrid[position] = -(invListLink + 1);
		}
	}
}

static int TargetHandSlot(bool *targetSlotIsOccupied, ItemStruct *replacingItem)
{
	//If the weapon we're replacing with is two-handed, it always goes into the left slot
	if (IsTwoHanded(replacingItem)) {
		*targetSlotIsOccupied = !plr[myplr].InvBody[INVLOC_HAND_LEFT].isEmpty();
		return INVLOC_HAND_LEFT;
	}

	//Check if either slot contains a two-handed weapon, if so, we replace that slot
	for (int i = INVLOC_HAND_LEFT; i <= INVLOC_HAND_RIGHT; i++) {
		ItemStruct *hand = &plr[myplr].InvBody[i];
		if (!hand->isEmpty() && IsTwoHanded(hand)) {
			*targetSlotIsOccupied = true;
			return i;
		}
	}

	//Check if the type of item is weapon or shield. If equal to replacing item, we replace that slot. If different, we replace the other slot. 
	if (plr[myplr]._pClass != PC_BARD) { //Bards can wield two weapons, so this check doesn't apply
		bool replacingItemIsAShield = replacingItem->_itype == ITYPE_SHIELD;
		for (int i = INVLOC_HAND_LEFT; i <= INVLOC_HAND_RIGHT; i++) {
			ItemStruct *hand = &plr[myplr].InvBody[i];
			if (!hand->isEmpty()) {
				bool equippedItemIsShield = hand->_itype == ITYPE_SHIELD;
				int targetSlot;
				if (replacingItemIsAShield != equippedItemIsShield) { //These are differing types, so we should replace the other slot
					if (i == INVLOC_HAND_LEFT)
						targetSlot = INVLOC_HAND_RIGHT;
					else
						targetSlot = INVLOC_HAND_LEFT;
				} else //Types match, so we replace this slot
					targetSlot = i;

				*targetSlotIsOccupied = !plr[myplr].InvBody[targetSlot].isEmpty();
				return targetSlot;
			}
		}
	}

	//If all the above fail then we go with default behaviour
	return TargetBodySlot(INVLOC_HAND_LEFT, INVLOC_HAND_RIGHT, targetSlotIsOccupied);

}

static void SaveHotBarItemLinkChange(int oldLink, int newLink)
{
	for (int i = 0; i < HOTBAR_SLOTS; i++) {
		if (hotbarSlots[i].itemLink == oldLink)
			hotbarChanges[i] = newLink;
	}
}

static void ResetHotbarChangesArray()
{
	for (int i = 0; i < HOTBAR_SLOTS; i++)
		hotbarChanges[i] = -1;
}

static void UpdateHotbarWithTemporaryArray()
{
	for (int i = 0; i < HOTBAR_SLOTS; i++) {
		if (hotbarChanges[i] != -1)
			hotbarSlots[i].itemLink = hotbarChanges[i];
	}
}

struct migratingItem_s {
	bool active; //Whether or not this is an item we need to migrate
	int bodyLoc; //Location on body the item currently resides (-1 means there's nothing to migrate)
	int invGridTarget; //Target location we want the item to reside in invGrid
	InvXY size; //Size of item on inventory grid
};

static bool TryToEquipItem(int invGridPosition, ItemStruct *item)
{
	//TODO: During this process, we also need to check if we meet the stat requirement for the item we're replacing (in case the item we're replacing increases the stat needed to equip this item)

	//Initialize migrating item variables
	migratingItem_s migratingItems[2];
	migratingItems[0].active = false;
	migratingItems[0].bodyLoc = -1;
	migratingItems[0].invGridTarget = -1;
	migratingItems[0].size.X = 0;
	migratingItems[0].size.Y = 0;
	migratingItems[1] = migratingItems[0];

	//Figure out which slot this goes into
	int invBodyTarget = -1;
	bool targetSlotIsOccupied = false;
	if (IsRing(item))
		invBodyTarget = TargetBodySlot(INVLOC_RING_LEFT, INVLOC_RING_RIGHT, &targetSlotIsOccupied);
	else if (IsHelmet(item))
		invBodyTarget = TargetBodySlot(INVLOC_HEAD, INVLOC_HEAD, &targetSlotIsOccupied);
	else if (IsAmulet(item))
		invBodyTarget = TargetBodySlot(INVLOC_AMULET, INVLOC_AMULET, &targetSlotIsOccupied);
	else if (IsArmour(item))
		invBodyTarget = TargetBodySlot(INVLOC_CHEST, INVLOC_CHEST, &targetSlotIsOccupied);
	else if (IsWeapon(item)) {
		invBodyTarget = TargetHandSlot(&targetSlotIsOccupied, item);

		//When equipping a weapon, it's possible we need to move what's in the second hand
		if (invBodyTarget != -1 && IsTwoHanded(item)) {
			if (invBodyTarget == INVLOC_HAND_LEFT)
				migratingItems[1].bodyLoc = plr[myplr].InvBody[INVLOC_HAND_RIGHT].isEmpty() ? -1 : INVLOC_HAND_RIGHT;
			else
				migratingItems[1].bodyLoc = plr[myplr].InvBody[INVLOC_HAND_LEFT].isEmpty() ? -1 : INVLOC_HAND_LEFT;
			if (migratingItems[1].bodyLoc != -1)
				migratingItems[1].active = true;
		}
	}

	if (invBodyTarget == -1) //We failed to find a slot to equip item in
		return false;

	//Set up items which need to be migrated
	if (targetSlotIsOccupied) {
		migratingItems[0].active = true;
		migratingItems[0].bodyLoc = invBodyTarget;
	}
	if (!migratingItems[0].active && migratingItems[1].active) { //We have a second migrating item but not a first one, so make second the first
		migratingItems[0] = migratingItems[1];
		migratingItems[1].active = false;
	}

	//Attain sizes of the items we need to move out of the way
	for (int i = 0; i < 2; i++) {
		if (migratingItems[i].active)
			migratingItems[i].size = GetInventorySize(plr[myplr].InvBody[migratingItems[i].bodyLoc]);
	}
	
	if (migratingItems[0].active) {

		//Create alternate version of invGrid which has "item" removed
		Sint8 InvGrid_withoutReplacingItem[NUM_INV_GRID_ELEM];
		memcpy(InvGrid_withoutReplacingItem, plr[myplr].InvGrid, NUM_INV_GRID_ELEM);
		RemoveItemFromInvGrid(InvGrid_withoutReplacingItem, plr[myplr].InvGrid[invGridPosition]);

		//We scan through invGrid for free space for the item (prefer slots which are the most topleft)
		int curSearchPosition = -1;
		while (1) {
			curSearchPosition += 1;
			int gridTargetSlot = FindSpotForItemInInvGrid(InvGrid_withoutReplacingItem, &curSearchPosition, migratingItems[0].size.X, migratingItems[0].size.Y);
			if (gridTargetSlot == -1) //If -1, then we have failed to find a slot for this item
				break;

			if (migratingItems[1].active) { //We have two items we need to find space for, so let's find something for item 2

				//We make yet another clone of invGrid, but this time with first migrating item added in
				Sint8 InvGrid_withMigratingItem[NUM_INV_GRID_ELEM];
				memcpy(InvGrid_withMigratingItem, InvGrid_withoutReplacingItem, NUM_INV_GRID_ELEM);
				AddItemToInvGrid(InvGrid_withMigratingItem, gridTargetSlot, migratingItems[0].size.X, migratingItems[0].size.Y, 1); //The last parameter isn't important so we use a dummy value

				//Search for a fitting slot for migratingSlot2
				int freeSlot = FindSpotForItemInInvGrid(InvGrid_withMigratingItem, 0, migratingItems[1].size.X, migratingItems[1].size.Y);

				if (freeSlot == -1) //If -1, we didn't find a free slot, so we continue the loop
					continue;

				//Set slot for migrating item 2
				migratingItems[1].invGridTarget = freeSlot;
			}

			//Set slot for migrating item 1
			migratingItems[0].invGridTarget = gridTargetSlot;
			break;
		}
	}

	if ((migratingItems[0].active && migratingItems[0].invGridTarget == -1)
		|| (migratingItems[1].active && migratingItems[1].invGridTarget == -1)) { //We failed to find inventory space for the migrating items
		if (plr[myplr]._pClass == PC_WARRIOR || plr[myplr]._pClass == PC_BARBARIAN) {
			PlaySFX(random_(0, 3) + PS_WARR14);
		} else if (plr[myplr]._pClass == PC_ROGUE || plr[myplr]._pClass == PC_BARD) {
			PlaySFX(random_(0, 3) + PS_ROGUE14);
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			PlaySFX(random_(0, 3) + PS_MAGE14);
		} else if (plr[myplr]._pClass == PC_MONK) {
			PlaySFX(random_(0, 3) + PS_MONK14);
		}
		return false;
	}

	//Now as we've figured out target slot and everything, we do the actual swap. First off, remove item from inventory
	ItemStruct itemTemp = *item; //Save info about item before removing it
	RemoveItemFromInventory(plr[myplr], plr[myplr].InvGrid[invGridPosition] - 1); //Remove item from inventory (note: this invalidates the function's "item" pointer)

	//Move "migrating" items from body to inventory
	ResetHotbarChangesArray();
	for (int i = 0; i < 2; i++) {
		migratingItem_s *migratingItem = &migratingItems[i];
		if (migratingItem->active) {
			plr[myplr].InvList[plr[myplr]._pNumInv] = plr[myplr].InvBody[migratingItem->bodyLoc]; //Move item to end of invList array
			int invListPosition = plr[myplr]._pNumInv;                                 //Remember position it now has in invList
			plr[myplr]._pNumInv++; //Increase quantity of items held
			AddItemToInvGrid(plr[myplr].InvGrid, migratingItem->invGridTarget, migratingItem->size.X, migratingItem->size.Y, invListPosition); //Place item in invGrid

			if (migratingItem->bodyLoc != invBodyTarget) {
				NetSendCmdDelItem(FALSE, migratingItem->bodyLoc); //Let other clients know that the item in this body inventory slot is now getting deleted (TODO: Check if this works)
				plr[myplr].InvBody[migratingItem->bodyLoc]._itype = ITYPE_NONE; //Remove item for local client
			}

			SaveHotBarItemLinkChange(migratingItem->bodyLoc, migratingItem->invGridTarget + INVITEM_INV_FIRST); //Update hotbar link for this item
		}
	}
	
	plr[myplr].InvBody[invBodyTarget] = itemTemp; //Place item into slot
	PlaySFX(ItemInvSnds[ItemCAnimTbl[itemTemp._iCurs]]); //Play sound for item being equipped
	NetSendCmdChItem_ItemPointer(FALSE, invBodyTarget, &plr[myplr].InvBody[invBodyTarget]); //Send network command letting other players know about the new item we just equipped (TODO: Check if this works)
	CalcPlrInv(myplr, TRUE); //Calculate player stats and item requirements now as we're wearing different gear
	SaveHotBarItemLinkChange(invGridPosition + INVITEM_INV_FIRST, invBodyTarget);
	UpdateHotbarWithTemporaryArray();
	return true;
}

static void SetSpellBasedOnWeapon(ItemStruct *item)
{
	if (item->_iCharges > 0) { //If item has charges and is equipped, then equip its spell
		plr[myplr]._pRSpell = item->_iSpell;
		plr[myplr]._pRSplType = RSPLTYPE_CHARGES;
		force_redraw = 255;
	} else if (sgOptions.Gameplay.bNoEquippedSpellIsAttack && IsWeapon(item)) {
		ClearReadiedSpell(plr[myplr]);
	}
}

static ItemStruct *ReturnItemBasedOnItemLink(int slot)
{
	if (hotbarSlots[slot].itemLink <= INVITEM_CHEST) {
		return &plr[myplr].InvBody[hotbarSlots[slot].itemLink];
	} else if (hotbarSlots[slot].itemLink <= INVITEM_INV_LAST) {
		return &plr[myplr].InvList[Hotbar_ReturnInvListPositionUsingItemLink(slot)];
	} else if (hotbarSlots[slot].itemLink <= INVITEM_BELT_LAST) {
		return &plr[myplr].SpdList[hotbarSlots[slot].itemLink - INVITEM_BELT_FIRST];
	} else
		return nullptr;
}

void Hotbar_UseSlot(int slot)
{
	if (hotbarSlots[slot].itemLink != -1) {
		//Fluffy TODO: Verify the slot isn't empty
		ItemStruct *item = ReturnItemBasedOnItemLink(slot);
		assert(item != nullptr);

		if (hotbarSlots[slot].itemLink <= INVITEM_CHEST) { //Item linked is an equipped item
			SetSpellBasedOnWeapon(item);
		}
		else if (IsEquippableItem(item)) { //Item is something which can be equipped to a body slot
			if (plr[myplr]._pmode <= PM_WALK3) { //Only equip items if player is standing or walking
				if (TryToEquipItem(hotbarSlots[slot].itemLink - INVITEM_INV_FIRST, item)) {
					item = ReturnItemBasedOnItemLink(slot); //Re-acquire item as the pointer we created above would have changed during TryToEquipItem()
					SetSpellBasedOnWeapon(item);
				}
				//TryToEquipItem(hotbarSlots[slot].itemLink2, item2); //TODO: This should avoid replacing the item we just moved
			} else { //I'm not sure how to handle behaviour if it's not allowed to equip. I think the ideal solution would be to "queue" up the action but that would take a lot of work to implement
				if (plr[myplr]._pClass == PC_WARRIOR || plr[myplr]._pClass == PC_BARBARIAN) {
					PlaySFX(random_(0, 2) + PS_WARR37);
				} else if (plr[myplr]._pClass == PC_ROGUE || plr[myplr]._pClass == PC_BARD) {
					PlaySFX(random_(0, 2) + PS_ROGUE37);
				} else if (plr[myplr]._pClass == PC_SORCERER) {
					PlaySFX(random_(0, 2) + PS_MAGE37);
				} else if (plr[myplr]._pClass == PC_MONK) {
					PlaySFX(random_(0, 2) + PS_MONK37);
				}
			}
		} else if (item->_iMiscId == IMISC_SCROLL || item->_iMiscId == IMISC_SCROLLT) { //Equip as spell rather than use directly
			plr[myplr]._pRSpell = item->_iSpell;
			plr[myplr]._pRSplType = RSPLTYPE_SCROLL;
			force_redraw = 255;
		} else {
			int miscId = item->_iMiscId;
			int spellId = item->_iSpell;

			//Convert itemLink value into what UseInvItem expects
			int useItemValue;
			if (hotbarSlots[slot].itemLink >= INVITEM_INV_FIRST && hotbarSlots[slot].itemLink <= INVITEM_INV_LAST)
				useItemValue = Hotbar_ReturnInvListPositionUsingItemLink(slot) + INVITEM_INV_FIRST;
			else
				useItemValue = hotbarSlots[slot].itemLink;

			if (UseInvItem(myplr, useItemValue)) { //Fluffy: If item was consumed, then try to find another slot containing an item of the same or similar type
				bool found = false;
				for (int i = 0; i < MAXBELTITEMS; i++) {
					if (!plr[myplr].SpdList[i].isEmpty()) {
						if (plr[myplr].SpdList[i]._iMiscId == miscId && plr[myplr].SpdList[i]._iSpell == spellId) {
							hotbarSlots[slot].itemLink = i + INVITEM_BELT_FIRST;
							found = true;
						}
					}
				}

				if (!found) {
					for (int i = 0; i < NUM_INV_GRID_ELEM; i++) {
						if (plr[myplr].InvGrid[i] > 0) {
							int invSlot = abs(plr[myplr].InvGrid[i]) - 1;
							if (plr[myplr].InvList[invSlot]._iMiscId == miscId && plr[myplr].InvList[invSlot]._iSpell == spellId) {
								hotbarSlots[slot].itemLink = i + INVITEM_INV_FIRST;
								found = true;
							}
						}
					}
				}

				if (!found)
					hotbarSlots[slot].itemLink = -1;
			}
		}
	} else if (hotbarSlots[slot].spellLink != -1) {
		plr[myplr]._pRSpell = (spell_id) hotbarSlots[slot].spellLink;
		plr[myplr]._pRSplType = (spell_type) hotbarSlots[slot].spellLinkType;
		force_redraw = 255;
	}
}

bool Hotbar_RightMouseDown()
{
	if (selectedHotbarSlot != -1) {
		if (selectedHotbarSlot_forLinking == selectedHotbarSlot) {
			hotbarSlots[selectedHotbarSlot_forLinking].itemLink = -1;
			hotbarSlots[selectedHotbarSlot_forLinking].spellLink = -1;
			hotbarSlots[selectedHotbarSlot_forLinking].spellLinkType = 0;
			selectedHotbarSlot_forLinking = -1;
		} else
			Hotbar_UseSlot(selectedHotbarSlot);

		return true;
	}

	return false;
}

static void RenderSpellIcon(CelOutputBuffer out, int x, int y, int spell, int spellType)
{
	if (options_hwUIRendering) {
		int textureNum = TEXTURE_SMALLSPELLICONS;
		int frame = SpellITbl[spell] - 1;
		textureNum += spellType;
		Render_Texture_Scale(x, y - INV_SLOT_SIZE_PX, textureNum, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX, frame);
	} else {
		//Fluffy TODO
	}
}

void Hotbar_Render(CelOutputBuffer out)
{
	//Render hotbar slot linking and hotkey
	for (int i = 0; i < HOTBAR_SLOTS; i++) {
		bool activeLink = false;
		int x = PANEL_LEFT + hotbarSlotLocations[i].X;
		int y = PANEL_TOP + hotbarSlotLocations[i].Y;

		if (hotbarSlots[i].itemLink != -1) {
			activeLink = true;
			int frame, frame_width;
			bool meetRequirements = true;

			ItemStruct *item;
			if (hotbarSlots[i].itemLink <= INVITEM_CHEST)
				item = &plr[myplr].InvBody[hotbarSlots[i].itemLink];
			else if (hotbarSlots[i].itemLink >= INVITEM_INV_FIRST && hotbarSlots[i].itemLink <= INVITEM_INV_LAST)
				item = &plr[myplr].InvList[Hotbar_ReturnInvListPositionUsingItemLink(i)];
			else if (hotbarSlots[i].itemLink >= INVITEM_BELT_FIRST && hotbarSlots[i].itemLink <= INVITEM_BELT_LAST)
				item = &plr[myplr].SpdList[hotbarSlots[i].itemLink - INVITEM_BELT_FIRST];
			else
				assert(false);

			if (item->isEmpty())
				continue;

			InvDrawSlotBack(out, x, y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

			meetRequirements = item->_iStatFlag;

			bool skipRenderingIcon = false;
			if (hotbarSlots[i].itemLink <= INVITEM_CHEST) {
				if (item->_iCharges > 0) { //This equipped item has spell charges, so render its spell icon on the hotbar instead of item icon //TODO: Should we use meetRequirements in this check?
					RenderSpellIcon(out, x, y, item->_iSpell, RSPLTYPE_CHARGES);
					skipRenderingIcon = true;
				}
			} else {
				if (meetRequirements && (item->_iMiscId == IMISC_SCROLL || item->_iMiscId == IMISC_SCROLLT)) { //This is a scroll so render its spell icon on the hotbar instead of item icon
					RenderSpellIcon(out, x, y, item->_iSpell, RSPLTYPE_SCROLL);
					skipRenderingIcon = true;
				}
			}

			if (!skipRenderingIcon) {
				frame = item->_iCurs + CURSOR_FIRSTITEM;
				frame_width = InvItemWidth[frame];

				if (options_hwUIRendering) {
					int textureNum = TEXTURE_CURSOR;
					if (frame > 179) {
						textureNum = TEXTURE_CURSOR2;
						frame -= 179;
					}
					frame -= 1;

					if (!meetRequirements) //Render item as red if we can't use it
						SDL_SetTextureColorMod(textures[textureNum].frames[frame].frame, 207, 0, 0);

					int width = textures[textureNum].frames[frame].width;
					int height = textures[textureNum].frames[frame].height;
					if (width == INV_SLOT_SIZE_PX && height == INV_SLOT_SIZE_PX)
						Render_Texture_FromBottom(x, y, textureNum, frame);
					else { //Scale item render
						int renderX = INV_SLOT_SIZE_PX, renderY = INV_SLOT_SIZE_PX, offsetX = 0, offsetY = 0;
						if (width > height) {
							float scale = (float)height / width;
							renderY = INV_SLOT_SIZE_PX * scale;
							offsetY = (INV_SLOT_SIZE_PX / 2) * (1.0f - scale);
						} else if (width < height) {
							float scale = (float)width / height;
							renderX = INV_SLOT_SIZE_PX * scale;
							offsetX = (INV_SLOT_SIZE_PX / 2) * (1.0f - scale);
						}
						Render_Texture_Scale(x + offsetX, y + offsetY - INV_SLOT_SIZE_PX, textureNum, renderX, renderY, frame);
					}

					if (!meetRequirements)
						SDL_SetTextureColorMod(textures[textureNum].frames[frame].frame, 255, 255, 255);
				} else {
					//Fluffy TODO
				}

			}
		} else if (hotbarSlots[i].spellLink != -1) {
			activeLink = true;
			RenderSpellIcon(out, x, y, hotbarSlots[i].spellLink, hotbarSlots[i].spellLinkType);
		}
		doneRenderingIcon:

		//Draw hotkey
		if (activeLink) {
			Uint8 ff = fontframe[gbFontTransTbl[i + 49]];
			PrintChar(out, x + INV_SLOT_SIZE_PX - fontkern[ff] - 1, y - 1, ff, COL_BLACK);
			PrintChar(out, x + INV_SLOT_SIZE_PX - fontkern[ff], y, ff, COL_WHITE);
		}
	}

	//Draw an outline for the selected hotbar slot
	for (int i = 0; i < 2; i++) {
		int slotNum;
		if (i == 0)
			slotNum = selectedHotbarSlot;
		else
			slotNum = selectedHotbarSlot_forLinking;
		if (slotNum == -1)
			continue;

		if (options_hwUIRendering) {
			if (i == 0)
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			else
				SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			SDL_Rect rect;
			rect.x = PANEL_LEFT + hotbarSlotLocations[slotNum].X;
			rect.y = PANEL_TOP + hotbarSlotLocations[slotNum].Y - INV_SLOT_SIZE_PX;
			rect.w = INV_SLOT_SIZE_PX;
			rect.h = INV_SLOT_SIZE_PX;
			SDL_RenderDrawRect(renderer, &rect);
		} else {
			//Fluffy TODO
		}
	}
}

DEVILUTION_END_NAMESPACE
