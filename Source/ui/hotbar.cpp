#include "../all.h"
#include "hotbar.h"
#include "../render/sdl-render.h"
#include "../textures/textures.h"
#include "../options.h"

DEVILUTION_BEGIN_NAMESPACE

hotbarSlot_s hotbarSlots[HOTBAR_SLOTS];
int selectedHotbarSlot = -1;
int selectedHotbarSlot_forLinking = -1;
const InvXY hotBarSlotLocations[HOTBAR_SLOTS] = {
	{ 205, 33 },
	{ 234, 33 },
	{ 263, 33 },
	{ 292, 33 },
	{ 321, 33 },
	{ 350, 33 },
	{ 379, 33 },
	{ 408, 33 }
};

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

		if (MouseX >= hotBarSlotLocations[i].X + xo
		    && MouseX < hotBarSlotLocations[i].X + xo + (INV_SLOT_SIZE_PX + 1)
		    && MouseY >= hotBarSlotLocations[i].Y + yo - (INV_SLOT_SIZE_PX + 1)
		    && MouseY < hotBarSlotLocations[i].Y + yo) {
			selectedHotbarSlot = i;
			return true;
		}
	}

	selectedHotbarSlot = -1;
	return false;
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

static int FindSpotForItemInInvGrid(Sint8 *invGrid, int startAt, int sizeX, int sizeY)
{
	//Position 0 equals to topleft. NUM_INV_GRID_ELEM - 1 is bottomright
	//The "real" position of an item is at their bottomleft-most grid position
	const int pitch = 10; //TODO: Put this somewhere else
	for (int i = startAt; i < NUM_INV_GRID_ELEM; i++) {
		int verticalOffset = 0;
		for (int y = 0; y < sizeY; y++) {
			for (int x = 0; x < sizeX; x++) {
				if ((x + i) % pitch < i % pitch) //Check if we're exceeding X boundary
					goto skip;
				if (i + x + verticalOffset >= NUM_INV_GRID_ELEM) //Check if we're exceeding Y boundary
					goto skip;
				if (invGrid[i + x + verticalOffset] != 0) //Check if slot is already occupied
					goto skip;
			}
			if (y == sizeY - 1) //If we get this far, then this position is a valid spot for the item
				return i /*+ verticalOffset*/;
			verticalOffset += pitch;
		}
	skip:
		continue;
	}
	return -1;
}

static void AddItemToInvGrid(Sint8 *invGrid, int slot, int sizeX, int sizeY, int invListLink)
{
	const int pitch = 10; //TODO: Put this somewhere else
	int startingPos = slot + (pitch * sizeY);
	for (int y = 0; y < sizeY; y++) {
		for (int x = 0; x < sizeX; x++) {
			if (x == 0 & y == 0)
				invGrid[startingPos + x - (y * pitch)] = invListLink;
			else
				invGrid[startingPos + x - (y * pitch)] = -invListLink;
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

static void TryToEquipItem(int invListIndex, ItemStruct *item)
{
	//TODO: During this process, we also need to check if we meet the stat requirement for the item we're replacing (in case the item we're replacing increases the stat needed to equip this item)
	// 
	//Figure out which slot this goes into
	int targetSlot = -1;
	bool targetSlotIsOccupied = false;
	int migratingItem2_from = -1;
	if (IsRing(item))
		targetSlot = TargetBodySlot(INVLOC_RING_LEFT, INVLOC_RING_RIGHT, &targetSlotIsOccupied);
	else if (IsHelmet(item))
		targetSlot = TargetBodySlot(INVLOC_HEAD, INVLOC_HEAD, &targetSlotIsOccupied);
	else if (IsAmulet(item))
		targetSlot = TargetBodySlot(INVLOC_AMULET, INVLOC_AMULET, &targetSlotIsOccupied);
	else if (IsArmour(item))
		targetSlot = TargetBodySlot(INVLOC_CHEST, INVLOC_CHEST, &targetSlotIsOccupied);
	else if (IsWeapon(item)) {
		targetSlot = TargetHandSlot(&targetSlotIsOccupied, item);

		//When equipping a weapon, it's possible we need to move what's in the second hand
		if (targetSlot != -1 && IsTwoHanded(item)) {
			if (targetSlot == INVLOC_HAND_LEFT)
				migratingItem2_from = plr[myplr].InvBody[INVLOC_HAND_RIGHT].isEmpty() ? -1 : INVLOC_HAND_RIGHT;
			else
				migratingItem2_from = plr[myplr].InvBody[INVLOC_HAND_LEFT].isEmpty() ? -1 : INVLOC_HAND_LEFT;
		}
	}

	if (targetSlot == -1) //We failed to find a slot to equip item in
		return;

	//Initialize variables used for items we need to move out of the way
	int migratingItem1_from = -1, migratingItem1_to = -1, migratingItem2_to = -1;
	if (targetSlotIsOccupied)
		migratingItem1_from = targetSlot;
	if (migratingItem1_from == -1 && migratingItem2_from != -1) { //If we have a slot2 to move but no slot1, then move that info over to slot1
		migratingItem1_from = migratingItem2_from;
		migratingItem2_from = -1;
	}

	//Attain information about the sizes of the items we need to move out of the way
	InvXY migratingItem1_size = { 0, 0 }, migratingItem2_size = { 0, 0 };
	if (migratingItem1_from != -1)
		migratingItem1_size = GetInventorySize(plr[myplr].InvBody[migratingItem1_from]);
	if (migratingItem2_from != -1)
		migratingItem2_size = GetInventorySize(plr[myplr].InvBody[migratingItem2_from]);
	
	if (migratingItem1_from != -1) {

		//Create alternate version of invGrid which has "item" removed
		Sint8 InvGrid_withoutReplacingItem[NUM_INV_GRID_ELEM];
		memcpy(InvGrid_withoutReplacingItem, plr[myplr].InvGrid, NUM_INV_GRID_ELEM);
		RemoveItemFromInvGrid(InvGrid_withoutReplacingItem, invListIndex - INVITEM_INV_FIRST + 1);

		//We scan through invGrid for free space for the item (prefer slots which are the most topleft)
		int curSlot = -1;
		while (1) {
			curSlot = FindSpotForItemInInvGrid(InvGrid_withoutReplacingItem, curSlot + 1, migratingItem1_size.X, migratingItem1_size.Y);
			if (curSlot == -1) //If -1, then we have failed to find a slot for this item
				break;

			if (migratingItem2_from != -1) { //We have two items we need to find space for, so let's find something for item 2

				//We make yet another clone of invGrid, but this time with migratingSlot1 added in
				Sint8 InvGrid_withMigratingSlot1[NUM_INV_GRID_ELEM];
				memcpy(InvGrid_withMigratingSlot1, InvGrid_withoutReplacingItem, NUM_INV_GRID_ELEM);
				AddItemToInvGrid(InvGrid_withMigratingSlot1, curSlot, migratingItem1_size.X, migratingItem1_size.Y, 1); //The last parameter isn't important so we use a dummy value

				//Search for a fitting slot for migratingSlot2
				int freeSlot = FindSpotForItemInInvGrid(InvGrid_withMigratingSlot1, 0, migratingItem2_size.X, migratingItem2_size.Y);

				if (freeSlot == -1) //If -1, we didn't find a free slot, so we continue the loop
					continue;

				//Set slot for migrating item 2
				migratingItem2_to = freeSlot;
			}

			//Set slot for migrating item 1
			migratingItem1_to = curSlot;
			break;
		}
	}

	if ((migratingItem1_from != -1 && migratingItem1_to == -1)
	    || (migratingItem2_from != -1 && migratingItem2_to == -1)) { //We failed to find inventory space for the migrating items
		if (plr[myplr]._pClass == PC_WARRIOR) {
			PlaySFX(random_(0, 3) + PS_WARR14);
		} else if (plr[myplr]._pClass == PC_ROGUE) {
			PlaySFX(random_(0, 3) + PS_ROGUE14);
		} else if (plr[myplr]._pClass == PC_SORCERER) {
			PlaySFX(random_(0, 3) + PS_MAGE14);
		} else if (plr[myplr]._pClass == PC_MONK) {
			PlaySFX(random_(0, 3) + PS_MONK14);
		} else if (plr[myplr]._pClass == PC_BARD) {
			PlaySFX(random_(0, 3) + PS_ROGUE14);
		} else if (plr[myplr]._pClass == PC_BARBARIAN) {
			PlaySFX(random_(0, 3) + PS_WARR14);
		}
		return;

	}

	//TODO: Now as we've figured out target slot and everything, we do the actual swap
	return;
}

void Hotbar_UseSlot(int slot)
{
	if (hotbarSlots[slot].itemLink != -1) {
		//Fluffy TODO: Verify the slot isn't empty
		ItemStruct *item;
		if (hotbarSlots[slot].itemLink <= INVITEM_INV_LAST) {
			item = &plr[myplr].InvList[hotbarSlots[slot].itemLink - INVITEM_INV_FIRST];
		} else {
			item = &plr[myplr].SpdList[hotbarSlots[slot].itemLink - INVITEM_BELT_FIRST];
		}

		if (hotbarSlots[slot].itemLink <= INVITEM_CHEST) { //Item linked is an equipped item
			if (item->_iCharges > 0) { //If item has charges and is equipped, then equip its spell
				plr[myplr]._pRSpell = item->_iSpell;
				plr[myplr]._pRSplType = RSPLTYPE_CHARGES;
				force_redraw = 255;
			} else if (sgOptions.Gameplay.bNoEquippedSpellIsAttack && IsWeapon(item)) {
				ClearReadiedSpell(plr[myplr]);
			}
		}
		else if (IsEquippableItem(item)) { //Item is something which can be equipped to a body slot
			TryToEquipItem(hotbarSlots[slot].itemLink, item);
			//TryToEquipItem(hotbarSlots[slot].itemLink2, item2); //TODO: This should avoid replacing the item we just moved

			/*if (AutoEquip(myplr, *item))
				RemoveItemFromInventory(plr[myplr], hotbarSlots[slot].itemLink - INVITEM_INV_FIRST);*/
			//TODO
		} else if (item->_iMiscId == IMISC_SCROLL || item->_iMiscId == IMISC_SCROLLT) { //Equip as spell rather than use directly
			plr[myplr]._pRSpell = item->_iSpell;
			plr[myplr]._pRSplType = RSPLTYPE_SCROLL;
			force_redraw = 255;
		} else {
			int miscId = item->_iMiscId;
			int spellId = item->_iSpell;
			if (UseInvItem(myplr, hotbarSlots[slot].itemLink)) { //Fluffy: If item was consumed, then try to find another slot containing an item of the same or similar type
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
								hotbarSlots[slot].itemLink = invSlot + INVITEM_INV_FIRST;
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
		int x = PANEL_LEFT + hotBarSlotLocations[i].X;
		int y = PANEL_TOP + hotBarSlotLocations[i].Y;

		if (hotbarSlots[i].itemLink != -1) {
			activeLink = true;
			int frame, frame_width;
			bool meetRequirements = true;

			ItemStruct *item;
			if (hotbarSlots[i].itemLink <= INVITEM_CHEST)
				item = &plr[myplr].InvBody[hotbarSlots[i].itemLink];
			else if (hotbarSlots[i].itemLink >= INVITEM_INV_FIRST && hotbarSlots[i].itemLink <= INVITEM_INV_LAST)
				item = &plr[myplr].InvList[hotbarSlots[i].itemLink - INVITEM_INV_FIRST];
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
			rect.x = PANEL_LEFT + hotBarSlotLocations[slotNum].X;
			rect.y = PANEL_TOP + hotBarSlotLocations[slotNum].Y - INV_SLOT_SIZE_PX;
			rect.w = INV_SLOT_SIZE_PX;
			rect.h = INV_SLOT_SIZE_PX;
			SDL_RenderDrawRect(renderer, &rect);
		} else {
			//Fluffy TODO
		}
	}
}

DEVILUTION_END_NAMESPACE
