#include "../all.h"
#include "hotbar.h"

DEVILUTION_BEGIN_NAMESPACE

hotbarSlot_s hotbarSlots[HOTBAR_SLOTS];
int selectedHotbarSlot = -1;
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

void Hotbar_Render()
{
	if (selectedHotbarSlot != -1) { //Draw an outline for the selected hotbar slot
		if (options_hwUIRendering) {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			SDL_Rect rect;
			rect.x = PANEL_LEFT + hotBarSlotLocations[selectedHotbarSlot].X;
			rect.y = PANEL_TOP + hotBarSlotLocations[selectedHotbarSlot].Y - INV_SLOT_SIZE_PX;
			rect.w = INV_SLOT_SIZE_PX;
			rect.h = INV_SLOT_SIZE_PX;
			SDL_RenderDrawRect(renderer, &rect);
		} else {
			//Fluffy TODO
		}
	}
}

DEVILUTION_END_NAMESPACE
