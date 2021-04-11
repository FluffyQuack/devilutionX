#include "../all.h"
#include "hotbar.h"
#include "../render/sdl-render.h"
#include "../textures/textures.h"

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

bool Hotbar_MouseDown(bool rightClick) //Returns true if we completed an action related to a hotbar slot
{
	if (!rightClick) { //Left click
		if(selectedHotbarSlot != -1)
		{
			if (selectedHotbarSlot_forLinking == selectedHotbarSlot)
				selectedHotbarSlot_forLinking = -1;
			else
				selectedHotbarSlot_forLinking = selectedHotbarSlot;
			return true;
		} else if (selectedHotbarSlot_forLinking != -1) { //Try to link this hotbar slot to something (or set to -1 if we're selecting nothing or something invalid)
			if (pcursinvitem != -1) {
				hotbarSlots[selectedHotbarSlot_forLinking].itemLink = pcursinvitem;
				selectedHotbarSlot_forLinking = -1;
				return true;
			}
		}
	} else { //Right click
		if (selectedHotbarSlot != -1) {
			if (selectedHotbarSlot_forLinking == selectedHotbarSlot) {
				hotbarSlots[selectedHotbarSlot_forLinking].itemLink = -1;
				selectedHotbarSlot_forLinking = -1;
			}
			else if (hotbarSlots[selectedHotbarSlot].itemLink != -1) {
				//Fluffy TODO: Verify the slot isn't empty
				ItemStruct *item;
				if (hotbarSlots[selectedHotbarSlot].itemLink <= INVITEM_INV_LAST) {
					item = &plr[myplr].InvList[hotbarSlots[selectedHotbarSlot].itemLink - INVITEM_INV_FIRST];
				} else {
					item = &plr[myplr].SpdList[hotbarSlots[selectedHotbarSlot].itemLink - INVITEM_BELT_FIRST];
				}

				int miscId = item->_iMiscId;
				int spellId = item->_iSpell;
				if (UseInvItem(myplr, hotbarSlots[selectedHotbarSlot].itemLink)) { //Fluffy: If item was consumed, then try to find another slot containing an item of the same or similar type
					bool found = false;
					for (int i = 0; i < MAXBELTITEMS; i++) {
						if (!plr[myplr].SpdList[i].isEmpty()) {
							if (plr[myplr].SpdList[i]._iMiscId == miscId && plr[myplr].SpdList[i]._iSpell == spellId) {
								hotbarSlots[selectedHotbarSlot].itemLink = i + INVITEM_BELT_FIRST;
								found = true;
							}
						}
					}

					if (!found) {
						for (int i = 0; i < NUM_INV_GRID_ELEM; i++) {
							if (plr[myplr].InvGrid[i] > 0) {
								int invSlot = abs(plr[myplr].InvGrid[i]) - 1;
								if (plr[myplr].InvList[invSlot]._iMiscId == miscId && plr[myplr].InvList[invSlot]._iSpell == spellId) {
									hotbarSlots[selectedHotbarSlot].itemLink = invSlot + INVITEM_INV_FIRST;
									found = true;
								}
							}
						}
					}

					if (!found)
						hotbarSlots[selectedHotbarSlot].itemLink = -1;
				}
			}
			return true;
		}
	}
	return false;
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
			if (hotbarSlots[i].itemLink <= INVITEM_CHEST) {
				int itemSlotNum = hotbarSlots[i].itemLink;

				if (plr[myplr].InvBody[itemSlotNum].isEmpty())
					continue;

				frame = plr[myplr].InvBody[itemSlotNum]._iCurs + CURSOR_FIRSTITEM;
			} else if (hotbarSlots[i].itemLink >= INVITEM_INV_FIRST && hotbarSlots[i].itemLink <= INVITEM_INV_LAST) {
				int itemSlotNum = hotbarSlots[i].itemLink - INVITEM_INV_FIRST;

				if (plr[myplr].InvList[itemSlotNum].isEmpty())
					continue;

				/*bool found = false;
				for (int j = 0; j < NUM_INV_GRID_ELEM; j++) {
					if (plr[myplr].InvGrid[j] != 0 && abs(plr[myplr].InvGrid[j]) == itemSlotNum + 1) {
						found = true;
						break;
					}
				}
				if (!found)
					continue;*/

				frame = plr[myplr].InvList[itemSlotNum]._iCurs + CURSOR_FIRSTITEM;
			} else if (hotbarSlots[i].itemLink >= INVITEM_BELT_FIRST && hotbarSlots[i].itemLink <= INVITEM_BELT_LAST) {
				int itemSlotNum = hotbarSlots[i].itemLink - INVITEM_BELT_FIRST;

				if (plr[myplr].SpdList[itemSlotNum].isEmpty())
					continue;

				frame = plr[myplr].SpdList[itemSlotNum]._iCurs + CURSOR_FIRSTITEM;
			}

			frame_width = InvItemWidth[frame];
			InvDrawSlotBack(out, x, y, INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX);

			if (options_hwUIRendering) {
				int textureNum = TEXTURE_CURSOR;
				if (frame > 179) {
					textureNum = TEXTURE_CURSOR2;
					frame -= 179;
				}
				Render_Texture_FromBottom(x, y, textureNum, frame - 1);
			} else {
				//Fluffy TODO
			}
		}

		//Draw hotkey
		if (activeLink) {
			Uint8 ff = fontframe[gbFontTransTbl[i + 49]];
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
