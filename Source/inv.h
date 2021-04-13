/**
 * @file inv.h
 *
 * Interface of player inventory.
 */
#ifndef __INV_H__
#define __INV_H__

#include "items.h"
#include "player.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

typedef enum item_color {
	// clang-format off
	ICOL_WHITE = PAL16_YELLOW + 5,
	ICOL_BLUE  = PAL16_BLUE + 5,
	ICOL_RED   = PAL16_RED + 5,
	// clang-format on
} item_color;

extern BOOL invflag;
extern BYTE *pInvCels; //Fluffy: Added extern of this so Diablo.cpp can make SDL texture out of this
extern BOOL drawsbarflag;
extern InvXY InvRect[73]; //Fluffy: Changed from const to non-const for dynamic belt slot positions

void CalculateBeltSlotPositions(); //Fluffy
void FreeInvGFX();
void InitInv();
void InvDrawSlotBack(CelOutputBuffer out, int X, int Y, int W, int H); //Fluffy
void DrawCursorItemWrapper(CelOutputBuffer out, int x, int y, int frame, int frameWidth, bool cursorRender, bool red, bool outline = 0, int outlineColor = 0, bool transparent = 0);

/**
 * @brief Render the inventory panel to the given buffer.
 */
void DrawInv(CelOutputBuffer out);

void DrawInvBelt(CelOutputBuffer out);
InvXY GetInventorySize(const ItemStruct &item); //Fluffy: Added this to header file
bool AutoEquipEnabled(const PlayerStruct &player, const ItemStruct &item);
bool AutoEquip(int playerNumber, const ItemStruct &item, bool persistItem = true);
BOOL AutoPlace(int pnum, int ii, int sx, int sy, BOOL saveflag);
BOOL SpecialAutoPlace(int pnum, int ii, const ItemStruct &item);
BOOL GoldAutoPlace(int pnum);
void CheckInvSwap(int pnum, BYTE bLoc, int idx, WORD wCI, int seed, BOOL bId, uint32_t dwBuff);
void RemoveItemFromInventory(PlayerStruct &player, int iv); //Fluffy
void inv_update_rem_item(int pnum, BYTE iv);
void RemoveInvItem(int pnum, int iv);
void RemoveSpdBarItem(int pnum, int iv);
void CheckInvItem(bool isShiftHeld = false);
void CheckInvScrn(bool isShiftHeld);
void CheckItemStats(int pnum);
void InvGetItem(int pnum, int ii);
void AutoGetItem(int pnum, int ii);
int FindGetItem(int idx, WORD ci, int iseed);
void SyncGetItem(int x, int y, int idx, WORD ci, int iseed);
BOOL CanPut(int x, int y);
BOOL TryInvPut();
void DrawInvMsg(const char *msg);
int InvPutItem(int pnum, int x, int y);
int SyncPutItem(int pnum, int x, int y, int idx, WORD icreateinfo, int iseed, int Id, int dur, int mdur, int ch, int mch, int ivalue, DWORD ibuff, int to_hit, int max_dam, int min_str, int min_mag, int min_dex, int ac);
char CheckInvHLight();
void RemoveScroll(int pnum);
BOOL UseScroll();
void UseStaffCharge(int pnum);
BOOL UseStaff();
BOOL UseInvItem(int pnum, int cii);
void DoTelekinesis();
int CalculateGold(int pnum);
BOOL DropItemBeforeTrig();

/* data */

extern int AP2x2Tbl[10];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INV_H__ */
