#pragma once

DEVILUTION_BEGIN_NAMESPACE

#define HOTBAR_SLOTS 8

struct hotbarSlot_s {
	Uint32 itemLink; //Links to something in player's inventory
	//Uint32 itemLink2; //Links to a second item in player inventory (used for weapon swaps)
	Uint32 spellLink; //Links to a spell or skill
	Uint32 spellLinkType; //What type of type is linked (spell, charge, or skill)
};

extern int selectedHotbarSlot;
extern int selectedHotbarSlot_forLinking;

void Hotbar_ResetSlots();
bool Hotbar_SlotSelection();
void Hotbar_LinkSpellToHotbar(Uint32 spell, Uint32 spellType);
bool Hotbar_LinkItemToHotbar(int invItem);
bool Hotbar_LeftMouseDown();
bool Hotbar_RightMouseDown();
void Hotbar_Render(CelOutputBuffer out);

DEVILUTION_END_NAMESPACE
