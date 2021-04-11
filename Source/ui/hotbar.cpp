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

void Hotbar_ResetSlots()
{
	for (int i = 0; i < HOTBAR_SLOTS; i++) {
		hotbarSlots[i].itemLink = -1;
	}
	selectedHotbarSlot = -1;
	selectedHotbarSlot_forLinking = -1;
}

bool Hotbar_SlotSelection()
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

bool Hotbar_MouseDown(bool rightClick)
{
	if (!rightClick) {
		if(selectedHotbarSlot != -1)
		{
			if (selectedHotbarSlot_forLinking == selectedHotbarSlot)
				selectedHotbarSlot_forLinking = -1;
			else
				selectedHotbarSlot_forLinking = selectedHotbarSlot;
			return true;
		} else if (selectedHotbarSlot_forLinking != -1) {
			if (pcursinvitem != -1) {
				hotbarSlots[selectedHotbarSlot_forLinking].itemLink = pcursinvitem;
				selectedHotbarSlot_forLinking = -1;
				return true;
			} else {
				hotbarSlots[selectedHotbarSlot_forLinking].itemLink = -1;
				selectedHotbarSlot_forLinking = -1;
			}
		}
	} 
	return false;
}

void Hotbar_Render(CelOutputBuffer out)
{
	//Render hotbar slot linking
	for (int i = 0; i < HOTBAR_SLOTS; i++) {
		if (hotbarSlots[i].itemLink != -1) {
			int frame;
			int frame_width;
			int x = PANEL_LEFT + hotBarSlotLocations[i].X;
			int y = PANEL_TOP + hotBarSlotLocations[i].Y;

			if (hotbarSlots[i].itemLink <= INVITEM_CHEST) {

				int itemSlotNum = hotbarSlots[i].itemLink;
				if (plr[myplr].InvBody[itemSlotNum].isEmpty())
					continue;
				frame = plr[myplr].InvBody[itemSlotNum]._iCurs + CURSOR_FIRSTITEM;

			} else if (hotbarSlots[i].itemLink >= INVITEM_INV_FIRST && hotbarSlots[i].itemLink <= INVITEM_INV_LAST) {
				int itemSlotNum = hotbarSlots[i].itemLink - INVITEM_INV_FIRST;
				if (plr[myplr].InvList[itemSlotNum].isEmpty())
					continue;
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
