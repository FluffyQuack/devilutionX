#pragma once

DEVILUTION_BEGIN_NAMESPACE

#define HOTBAR_SLOTS 8
#define HOLDITEM_LINK 55 //This is one higher than INVITEM_BELT_LAST

struct hotbarSlot_s {

	/*
	* The item links correspond to this enum list: NUM_INVELEM
	* Equipped items link to invBody[]
	* Items in inventory link to invGrid[]
	* Items in belt link to SpdList[]
	*/
	Sint32 itemLink; //Links to something in player's inventory
	//Sint32 itemLink2; //Links to a second item in player inventory (used for weapon swaps)
	Sint32 spellLink; //Links to a spell or skill
	Sint32 spellLinkType; //What type of type is linked (spell, charge, or skill)
};

extern hotbarSlot_s hotbarSlots[HOTBAR_SLOTS];
extern int selectedHotbarSlot;
extern int selectedHotbarSlot_forLinking;

int FindItemOnInvGridUsingInvListIndex(int invListIndex);
void Hotbar_ResetSlots();
bool Hotbar_SlotSelection();
void Hotbar_RemoveItemLinkToInventory(int invGridIndex);
void Hotbar_UpdateItemLink(int oldLink, int newLink);
void Hotbar_SwapItemLinks(int itemLink1, int itemLink2);
void Hotbar_LinkSpellToHotbar(Uint32 spell, Uint32 spellType);
bool Hotbar_LinkItemToHotbar(int invItem);
bool Hotbar_LeftMouseDown();
void Hotbar_UseSlot(int slot);
bool Hotbar_RightMouseDown();
void Hotbar_Render(CelOutputBuffer out);

DEVILUTION_END_NAMESPACE
