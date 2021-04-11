#pragma once

DEVILUTION_BEGIN_NAMESPACE

#define HOTBAR_SLOTS 8

struct hotbarSlot_s {
	Uint32 itemLink; //Links to something in player's inventory
	//Uint32 itemLink2; //Links to a second item in player inventory (used for weapon swaps)
	//Uint32 spellLink; //Links to a spell or skill
};

extern int selectedHotbarSlot;

void Hotbar_ResetSlots();
bool Hotbar_SlotSelection();
bool Hotbar_MouseDown(bool rightClick);
void Hotbar_Render(CelOutputBuffer out);

DEVILUTION_END_NAMESPACE
