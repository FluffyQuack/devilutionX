/**
 * @file control.cpp
 *
 * Implementation of the character and main control panels
 */
#include "all.h"
#include "Textures/textures.h" //Fluffy: For accessing SDL textures
#include "Render/render.h" //Fluffy: For rendering via SDL
#include "Textures/cel-convert.h" //Fluffy: For loading CELs as SDL textures

DEVILUTION_BEGIN_NAMESPACE

BYTE sgbNextTalkSave;
BYTE sgbTalkSavePos;
BYTE *pDurIcons;
BYTE *pChrButtons;
BOOL drawhpflag;
BOOL dropGoldFlag;
BOOL panbtn[8];
BOOL chrbtn[4];
BYTE *pMultiBtns;
BYTE *pPanelButtons;
BYTE *pChrPanel;
BOOL lvlbtndown;
char sgszTalkSave[8][80];
int dropGoldValue;
BOOL drawmanaflag;
BOOL chrbtnactive;
char sgszTalkMsg[MAX_SEND_STR_LEN];
BYTE *pPanelText;
BYTE *pLifeBuff;
BYTE *pBtmBuff;
BYTE *pTalkBtns;
BOOL pstrjust[4];
int pnumlines;
BOOL pinfoflag;
BOOL talkbtndown[3];
int pSpell;
BYTE *pManaBuff;
char infoclr;
int sgbPlrTalkTbl;
BYTE *pGBoxBuff;
BYTE *pSBkBtnCel;
char tempstr[256];
BOOLEAN whisper[MAX_PLRS];
int sbooktab;
int pSplType;
int initialDropGoldIndex;
BOOL talkflag;
BYTE *pSBkIconCels;
BOOL sbookflag;
BOOL chrflag;
BOOL drawbtnflag;
BYTE *pSpellBkCel;
char infostr[256];
int numpanbtns;
BYTE *pStatusPanel;
char panelstr[4][64];
BOOL panelflag;
BYTE SplTransTbl[256];
int initialDropGoldValue;
BYTE *pSpellCels;
BOOL panbtndown;
BYTE *pTalkPanel;
BOOL spselflag;

/** Maps from font index to smaltext.cel frame number. */
const BYTE fontframe[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 54, 44, 57, 58, 56, 55, 47, 40, 41, 59, 39, 50, 37, 51, 52,
	36, 27, 28, 29, 30, 31, 32, 33, 34, 35, 48, 49, 60, 38, 61, 53,
	62, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 42, 63, 43, 64, 65,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 40, 66, 41, 67, 0
};

/**
 * Maps from smaltext.cel frame number to character width. Note, the
 * character width may be distinct from the frame width, which is 13 for every
 * smaltext.cel frame.
 */
const BYTE fontkern[68] = {
	8, 10, 7, 9, 8, 7, 6, 8, 8, 3,
	3, 8, 6, 11, 9, 10, 6, 9, 9, 6,
	9, 11, 10, 13, 10, 11, 7, 5, 7, 7,
	8, 7, 7, 7, 7, 7, 10, 4, 5, 6,
	3, 3, 4, 3, 6, 6, 3, 3, 3, 3,
	3, 2, 7, 6, 3, 10, 10, 6, 6, 7,
	4, 4, 9, 6, 6, 12, 3, 7
};
/**
 * Line start position for info box text when displaying 1, 2, 3, 4 and 5 lines respectivly
 */
const int lineOffsets[5][5] = {
	{ 82 },
	{ 70, 94 },
	{ 64, 82, 100 },
	{ 60, 75, 89, 104 },
	{ 58, 70, 82, 94, 105 },
};

/**
 * Maps ASCII character code to font index, as used by the
 * small, medium and large sized fonts; which corresponds to smaltext.cel,
 * medtexts.cel and bigtgold.cel respectively.
 */
const BYTE gbFontTransTbl[256] = {
	// clang-format off
	'\0', 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	' ',  '!',  '\"', '#',  '$',  '%',  '&',  '\'', '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
	'@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '[',  '\\', ']',  '^',  '_',
	'`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x01,
	'C',  'u',  'e',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'i',  'i',  'i',  'A',  'A',
	'E',  'a',  'A',  'o',  'o',  'o',  'u',  'u',  'y',  'O',  'U',  'c',  'L',  'Y',  'P',  'f',
	'a',  'i',  'o',  'u',  'n',  'N',  'a',  'o',  '?',  0x01, 0x01, 0x01, 0x01, '!',  '<',  '>',
	'o',  '+',  '2',  '3',  '\'', 'u',  'P',  '.',  ',',  '1',  '0',  '>',  0x01, 0x01, 0x01, '?',
	'A',  'A',  'A',  'A',  'A',  'A',  'A',  'C',  'E',  'E',  'E',  'E',  'I',  'I',  'I',  'I',
	'D',  'N',  'O',  'O',  'O',  'O',  'O',  'X',  '0',  'U',  'U',  'U',  'U',  'Y',  'b',  'B',
	'a',  'a',  'a',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'e',  'i',  'i',  'i',  'i',
	'o',  'n',  'o',  'o',  'o',  'o',  'o',  '/',  '0',  'u',  'u',  'u',  'u',  'y',  'b',  'y',
	// clang-format on
};

/* data */

/** Maps from spell_id to spelicon.cel frame number. */
char SpellITbl[] = {
	27,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	28,
	13,
	12,
	18,
	16,
	14,
	18,
	19,
	11,
	20,
	15,
	21,
	23,
	24,
	25,
	22,
	26,
	29,
	37,
	38,
	39,
	42,
	41,
	40,
	10,
	36,
	30,
	51,
	51,
	50,
	46,
	47,
	43,
	45,
	48,
	49,
	44,
	35,
	35,
	35,
	35,
	35,
};
/** Maps from panel_button_id to the position and dimensions of a panel button. */
int PanBtnPos[8][5] = {
	// clang-format off
	{   9,   9, 71, 19, TRUE  }, // char button
	{   9,  35, 71, 19, FALSE }, // quests button
	{   9,  75, 71, 19, TRUE  }, // map button
	{   9, 101, 71, 19, FALSE }, // menu button
	{ 560,   9, 71, 19, TRUE  }, // inv button
	{ 560,  35, 71, 19, FALSE }, // spells button
	{  87,  91, 33, 32, TRUE  }, // chat button
	{ 527,  91, 33, 32, TRUE  }, // friendly fire button
	// clang-format on
};
/** Maps from panel_button_id to hotkey name. */
const char *const PanBtnHotKey[8] = { "'c'", "'q'", "Tab", "Esc", "'i'", "'b'", "Enter", NULL };
/** Maps from panel_button_id to panel button description. */
const char *const PanBtnStr[8] = {
	"Character Information",
	"Quests log",
	"Automap",
	"Main Menu",
	"Inventory",
	"Spell book",
	"Send Message",
	"Player Attack"
};
/** Maps from attribute_id to the rectangle on screen used for attribute increment buttons. */
RECT32 ChrBtnsRect[4] = {
	{ 137, 138, 41, 22 },
	{ 137, 166, 41, 22 },
	{ 137, 195, 41, 22 },
	{ 137, 223, 41, 22 }
};

/** Maps from spellbook page number and position to spell_id. */
int SpellPages[6][7] = {
	{ SPL_NULL, SPL_FIREBOLT, SPL_CBOLT, SPL_HBOLT, SPL_HEAL, SPL_HEALOTHER, SPL_FLAME },
	{ SPL_RESURRECT, SPL_FIREWALL, SPL_TELEKINESIS, SPL_LIGHTNING, SPL_TOWN, SPL_FLASH, SPL_STONE },
	{ SPL_RNDTELEPORT, SPL_MANASHIELD, SPL_ELEMENT, SPL_FIREBALL, SPL_WAVE, SPL_CHAIN, SPL_GUARDIAN },
	{ SPL_NOVA, SPL_GOLEM, SPL_TELEPORT, SPL_APOCA, SPL_BONESPIRIT, SPL_FLARE, SPL_ETHEREALIZE },
	{ SPL_LIGHTWALL, SPL_IMMOLAT, SPL_WARP, SPL_REFLECT, SPL_BERSERK, SPL_FIRERING, SPL_SEARCH },
	{ -1, -1, -1, -1, -1, -1, -1 }
};

#define SPLICONLENGTH 56
#define SPLSMALLICONSIZE 37
#define SPLROWICONLS 10
#define SPLICONLAST (gbIsHellfire ? 52 : 43)

/**
 * Draw spell cell onto the back buffer.
 * @param xp Back buffer coordinate
 * @param yp Back buffer coordinate
 * @param Trans Pointer to the cel buffer.
 * @param nCel Index of the cel frame to draw. 0 based.
 * @param w Width of the frame.
 */
void DrawSpellCel(int xp, int yp, int nCel, int type, bool spellBook) //Fluffy: Updated this so it can render via SDL too
{
	int width = SPLICONLENGTH;
	if (spellBook)
		width = SPLSMALLICONSIZE;

	if (options_hwRendering) {
		int textureNum = TEXTURE_SPELLICONS;
		if (spellBook)
			textureNum = TEXTURE_SMALLSPELLICONS;
		textureNum += type;
		/*if (spellBook) //This takes the big spell icon and scales it down to the same size of spell book icons. I like this dynamic solution, but the icons end up looking a bit different (the small icons in the CEL are a bit different than the normal spell icons, as they have a different border)
			Render_Texture_ScaleAndCrop(xp - BORDER_LEFT, yp - BORDER_TOP - width + 1, textureNum, width, width, 5, 4, textures[textureNum].frames[nCel - 1].width - 3, textures[textureNum].frames[nCel - 1].height - 4, nCel - 1);
		else*/
		Render_Texture_FromBottomLeft(xp - BORDER_LEFT, yp - BORDER_TOP, textureNum, nCel - 1);
		return;
	}

	BYTE *celData = pSpellCels;
	if (spellBook)
		celData = pSBkIconCels;

	CelDrawLight(xp, yp, celData, nCel, width, SplTransTbl);
}

void SetSpellTrans(char t)
{
	int i;

	if (t == RSPLTYPE_SKILL) {
		for (i = 0; i < 128; i++)
			SplTransTbl[i] = i;
	}
	for (i = 128; i < 256; i++)
		SplTransTbl[i] = i;
	SplTransTbl[255] = 0;

	switch (t) {
	case RSPLTYPE_SPELL:
		SplTransTbl[PAL8_YELLOW] = PAL16_BLUE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_BLUE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_BLUE + 5;
		for (i = PAL16_BLUE; i < PAL16_BLUE + 16; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_BLUE + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_BLUE + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_BLUE + i] = i;
		}
		break;
	case RSPLTYPE_SCROLL:
		SplTransTbl[PAL8_YELLOW] = PAL16_BEIGE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_BEIGE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_BEIGE + 5;
		for (i = PAL16_BEIGE; i < PAL16_BEIGE + 16; i++) {
			SplTransTbl[PAL16_YELLOW - PAL16_BEIGE + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_BEIGE + i] = i;
		}
		break;
	case RSPLTYPE_CHARGES:
		SplTransTbl[PAL8_YELLOW] = PAL16_ORANGE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_ORANGE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_ORANGE + 5;
		for (i = PAL16_ORANGE; i < PAL16_ORANGE + 16; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_ORANGE + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_ORANGE + i] = i;
		}
		break;
	case RSPLTYPE_INVALID:
		SplTransTbl[PAL8_YELLOW] = PAL16_GRAY + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_GRAY + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_GRAY + 5;
		for (i = PAL16_GRAY; i < PAL16_GRAY + 15; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_GRAY + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_GRAY + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_GRAY + i] = i;
		}
		SplTransTbl[PAL16_BEIGE + 15] = 0;
		SplTransTbl[PAL16_YELLOW + 15] = 0;
		SplTransTbl[PAL16_ORANGE + 15] = 0;
		break;
	}
}

/**
 * Sets the spell frame to draw and its position then draws it.
 */
void DrawSpell()
{
	char st;
	int spl, tlvl;

	spl = plr[myplr]._pRSpell;
	st = plr[myplr]._pRSplType;

	// BUGFIX: Move the next line into the if statement to avoid OOB (SPL_INVALID is -1) (fixed)
	if (st == RSPLTYPE_SPELL && spl != SPL_INVALID) {
		tlvl = plr[myplr]._pISplLvlAdd + plr[myplr]._pSplLvl[spl];
		if (!CheckSpell(myplr, spl, RSPLTYPE_SPELL, TRUE))
			st = RSPLTYPE_INVALID;
		if (tlvl <= 0)
			st = RSPLTYPE_INVALID;
	}

	//Fluffy: Grey out spell icon UI if we can't cast spells in town
	if (currlevel == 0 && st != RSPLTYPE_INVALID && !spelldata[spl].sTownSpell && gameSetup_allowAttacksInTown == false)
		st = RSPLTYPE_INVALID;

	if (plr[myplr]._pRSpell < 0)
		st = RSPLTYPE_INVALID;
	SetSpellTrans(st);
	if (spl != SPL_INVALID)
		DrawSpellCel(PANEL_X + 565, PANEL_Y + 119, SpellITbl[spl], st, false);
	else
		DrawSpellCel(PANEL_X + 565, PANEL_Y + 119, 27, st, false);
}

void DrawSpellList()
{
	int i, j, x, y, c, s, t, v, lx, ly, trans;
	unsigned __int64 mask, spl;

	pSpell = SPL_INVALID;
	infostr[0] = '\0';
	x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
	y = PANEL_Y - 17;
	ClearPanel();

	int maxSpells = gbIsHellfire ? MAX_SPELLS : 37;

	for (i = 0; i < 4; i++) {
		switch ((spell_type)i) {
		case RSPLTYPE_SKILL:
			SetSpellTrans(RSPLTYPE_SKILL);
			mask = plr[myplr]._pAblSpells;
			c = SPLICONLAST + 3;
			break;
		case RSPLTYPE_SPELL:
			mask = plr[myplr]._pMemSpells;
			c = SPLICONLAST + 4;
			break;
		case RSPLTYPE_SCROLL:
			SetSpellTrans(RSPLTYPE_SCROLL);
			mask = plr[myplr]._pScrlSpells;
			c = SPLICONLAST + 1;
			break;
		case RSPLTYPE_CHARGES:
			SetSpellTrans(RSPLTYPE_CHARGES);
			mask = plr[myplr]._pISpells;
			c = SPLICONLAST + 2;
			break;
		}
		for (spl = 1, j = 1; j < maxSpells; spl <<= 1, j++) {
			if (!(mask & spl))
				continue;
			trans = i;
			if (i == RSPLTYPE_SPELL) {
				s = plr[myplr]._pISplLvlAdd + plr[myplr]._pSplLvl[j];
				if (s < 0)
					s = 0;
				if (s > 0)
					trans = RSPLTYPE_SPELL;
				else
					trans = RSPLTYPE_INVALID;
				SetSpellTrans(trans);
			}

			//Fluffy: Grey out spells in spell selection if we can't cast in town
			if (currlevel == 0 && !spelldata[j].sTownSpell && gameSetup_allowAttacksInTown == false) {
				trans = RSPLTYPE_INVALID;
				SetSpellTrans(trans);
			}

			DrawSpellCel(x, y, SpellITbl[j], trans, false);
			lx = x - BORDER_LEFT;
			ly = y - BORDER_TOP - SPLICONLENGTH;
			if (MouseX >= lx && MouseX < lx + SPLICONLENGTH && MouseY >= ly && MouseY < ly + SPLICONLENGTH) {
				pSpell = j;
				pSplType = i;
				if (plr[myplr]._pClass == PC_MONK && j == SPL_SEARCH)
					pSplType = RSPLTYPE_SKILL;
				DrawSpellCel(x, y, c, trans, false);
				switch (pSplType) {
				case RSPLTYPE_SKILL:
					sprintf(infostr, "%s Skill", spelldata[pSpell].sSkillText);
					break;
				case RSPLTYPE_SPELL:
					sprintf(infostr, "%s Spell", spelldata[pSpell].sNameText);
					if (pSpell == SPL_HBOLT) {
						sprintf(tempstr, "Damages undead only");
						AddPanelString(tempstr, TRUE);
					}
					if (s == 0)
						sprintf(tempstr, "Spell Level 0 - Unusable");
					else
						sprintf(tempstr, "Spell Level %i", s);
					AddPanelString(tempstr, TRUE);
					break;
				case RSPLTYPE_SCROLL:
					sprintf(infostr, "Scroll of %s", spelldata[pSpell].sNameText);
					v = 0;
					for (t = 0; t < plr[myplr]._pNumInv; t++) {
						if (plr[myplr].InvList[t]._itype != ITYPE_NONE
						    && (plr[myplr].InvList[t]._iMiscId == IMISC_SCROLL || plr[myplr].InvList[t]._iMiscId == IMISC_SCROLLT)
						    && plr[myplr].InvList[t]._iSpell == pSpell) {
							v++;
						}
					}
					for (t = 0; t < MAXBELTITEMS; t++) {
						if (plr[myplr].SpdList[t]._itype != ITYPE_NONE
						    && (plr[myplr].SpdList[t]._iMiscId == IMISC_SCROLL || plr[myplr].SpdList[t]._iMiscId == IMISC_SCROLLT)
						    && plr[myplr].SpdList[t]._iSpell == pSpell) {
							v++;
						}
					}
					if (v == 1)
						strcpy(tempstr, "1 Scroll");
					else
						sprintf(tempstr, "%i Scrolls", v);
					AddPanelString(tempstr, TRUE);
					break;
				case RSPLTYPE_CHARGES:
					sprintf(infostr, "Staff of %s", spelldata[pSpell].sNameText);
					if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges == 1)
						strcpy(tempstr, "1 Charge");
					else
						sprintf(tempstr, "%i Charges", plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges);
					AddPanelString(tempstr, TRUE);
					break;
				}
				for (t = 0; t < 4; t++) {
					if (plr[myplr]._pSplHotKey[t] == pSpell && plr[myplr]._pSplTHotKey[t] == pSplType) {
						DrawSpellCel(x, y, t + SPLICONLAST + 5, trans, false);
						sprintf(tempstr, "Spell Hot Key #F%i", t + 5);
						AddPanelString(tempstr, TRUE);
					}
				}
			}
			x -= SPLICONLENGTH;
			if (x == PANEL_X + 12 - SPLICONLENGTH) {
				x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				y -= SPLICONLENGTH;
			}
		}
		if (mask != 0 && x != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS)
			x -= SPLICONLENGTH;
		if (x == PANEL_X + 12 - SPLICONLENGTH) {
			x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
			y -= SPLICONLENGTH;
		}
	}
}

void SetSpell()
{
	spselflag = FALSE;
	if (pSpell != SPL_INVALID) {
		ClearPanel();
		plr[myplr]._pRSpell = pSpell;
		plr[myplr]._pRSplType = pSplType;
		force_redraw = 255;
	}
}

void SetSpeedSpell(int slot)
{
	int i;

	if (pSpell != SPL_INVALID) {
		for (i = 0; i < 4; ++i) {
			if (plr[myplr]._pSplHotKey[i] == pSpell && plr[myplr]._pSplTHotKey[i] == pSplType)
				plr[myplr]._pSplHotKey[i] = SPL_INVALID;
		}
		plr[myplr]._pSplHotKey[slot] = pSpell;
		plr[myplr]._pSplTHotKey[slot] = pSplType;
	}
}

void ToggleSpell(int slot)
{
	unsigned __int64 spells;

	if (plr[myplr]._pSplHotKey[slot] == -1) {
		return;
	}

	switch (plr[myplr]._pSplTHotKey[slot]) {
	case RSPLTYPE_SKILL:
		spells = plr[myplr]._pAblSpells;
		break;
	case RSPLTYPE_SPELL:
		spells = plr[myplr]._pMemSpells;
		break;
	case RSPLTYPE_SCROLL:
		spells = plr[myplr]._pScrlSpells;
		break;
	case RSPLTYPE_CHARGES:
		spells = plr[myplr]._pISpells;
		break;
	}

	if (spells & SPELLBIT(plr[myplr]._pSplHotKey[slot])) {
		plr[myplr]._pRSpell = plr[myplr]._pSplHotKey[slot];
		plr[myplr]._pRSplType = plr[myplr]._pSplTHotKey[slot];
		force_redraw = 255;
	}
}

/**
 * @brief Print letter to the back buffer
 * @param sx Backbuffer offset
 * @param sy Backbuffer offset
 * @param nCel Number of letter in Windows-1252
 * @param col text_color color value
 */
void PrintChar(int sx, int sy, int nCel, char col)
{
	assert(gpBuffer);

	if (options_hwRendering) { //Fluffy: Render font using SDL
		int frame = nCel - 1;
		SDL_Texture *tex = textures[TEXTURE_SMALLFONT].frames[frame].frame;
		int x = sx - BORDER_LEFT;
		int y = sy - BORDER_TOP - textures[TEXTURE_SMALLFONT].frames[0].height + 1;
		if (col != COL_WHITE) {
			switch (col) {
			case COL_BLUE:
				SDL_SetTextureColorMod(tex, 200, 210, 255); //TODO: This colour should be slightly darker
				break;
			case COL_RED: //Even with rendering this twice, the colour is still slightly off (the edges of the font are supposed to be a strong red), but I think it's impossible to get the same look without texture changes
				SDL_SetTextureColorMod(tex, 255, 153, 153);
				Render_Texture(x, y, TEXTURE_SMALLFONT, frame);
				SDL_SetTextureColorMod(tex, 64, 8, 8);
				SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
				Render_Texture(x, y, TEXTURE_SMALLFONT, frame);
				break;
			default: //Gold colour
				SDL_SetTextureColorMod(tex, 255, 231, 148); //TODO: This colour should be ever so slightly darker
				break;
			}
		}
		Render_Texture(x, y, TEXTURE_SMALLFONT, frame);
		if (col != COL_WHITE) {
			SDL_SetTextureColorMod(tex, 255, 255, 255);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
		}
		return;
	}

	int i;
	BYTE pix;
	BYTE tbl[256];

	switch (col) {
	case COL_WHITE:
		CelDraw(sx, sy, pPanelText, nCel, 13);
		return;
	case COL_BLUE:
		for (i = 0; i < 256; i++) {
			pix = i;
			if (pix > PAL16_GRAY + 13)
				pix = PAL16_BLUE + 15;
			else if (pix >= PAL16_GRAY)
				pix -= PAL16_GRAY - (PAL16_BLUE + 2);
			tbl[i] = pix;
		}
		break;
	case COL_RED:
		for (i = 0; i < 256; i++) {
			pix = i;
			if (pix >= PAL16_GRAY)
				pix -= PAL16_GRAY - PAL16_RED;
			tbl[i] = pix;
		}
		break;
	default: //This is used for displaying the text "No automap available in town" which is shown in a gold colour
		for (i = 0; i < 256; i++) {
			pix = i;
			if (pix >= PAL16_GRAY) {
				if (pix >= PAL16_GRAY + 14)
					pix = PAL16_YELLOW + 15;
				else
					pix -= PAL16_GRAY - (PAL16_YELLOW + 2);
			}
			tbl[i] = pix;
		}
		break;
	}
	CelDrawLight(sx, sy, pPanelText, nCel, 13, tbl);
}

void AddPanelString(const char *str, BOOL just)
{
	strcpy(panelstr[pnumlines], str);
	pstrjust[pnumlines] = just;

	if (pnumlines < 4)
		pnumlines++;
}

void ClearPanel()
{
	pnumlines = 0;
	pinfoflag = FALSE;
}

void DrawPanelBox(int x, int y, int w, int h, int sx, int sy)
{
	if (options_hwRendering) { //Fluffy: Render HUD panel via SDL (we reference two different textures for this)
		//x, y = from
		//sx, sy = to
		//w, h = crop
		int texture = TEXTURE_HUDPANEL;
		if (y > 144) { //Original Diablo code stores two HUD panel images as one but they're referenced as 2 different textures when loaded via SDL
			texture = TEXTURE_HUDPANEL_VOICE;
			y -= 144;
		}
		Render_Texture_Crop(sx - BORDER_LEFT, sy - BORDER_TOP, texture, x, y, x + w, y + h);
		return;
	}

	int nSrcOff, nDstOff;

	assert(gpBuffer);

	nSrcOff = x + PANEL_WIDTH * y;
	nDstOff = sx + BUFFER_WIDTH * sy;

	int hgt;
	BYTE *src, *dst;

	src = &pBtmBuff[nSrcOff];
	dst = &gpBuffer[nDstOff];

	for (hgt = h; hgt; hgt--, src += PANEL_WIDTH, dst += BUFFER_WIDTH) {
		memcpy(dst, src, w);
	}
}

/**
 * Draws a section of the empty flask cel on top of the panel to create the illusion
 * of the flask getting empty. This function takes a cel and draws a
 * horizontal stripe of height (max-min) onto the back buffer.
 * @param pCelBuff Buffer of the empty flask cel.
 * @param min Top of the flask cel section to draw.
 * @param max Bottom of the flask cel section to draw.
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
void SetFlaskHeight(BYTE *pCelBuff, int min, int max, int sx, int sy)
{
	int nSrcOff, nDstOff, w;

	assert(gpBuffer);

	nSrcOff = 88 * min;
	nDstOff = sx + BUFFER_WIDTH * sy;
	w = max - min;

	BYTE *src, *dst;

	src = &pCelBuff[nSrcOff];
	dst = &gpBuffer[nDstOff];

	for (; w; w--, src += 88, dst += BUFFER_WIDTH)
		memcpy(dst, src, 88);
}

/**
 * Draws the dome of the flask that protrudes above the panel top line.
 * It draws a rectangle of fixed width 59 and height 'h' from the source buffer
 * into the target buffer.
 * @param pCelBuff The flask cel buffer.
 * @param w Width of the cel.
 * @param nSrcOff Offset of the source buffer from where the bytes will start to be copied from.
 * @param pBuff Target buffer.
 * @param nDstOff Offset of the target buffer where the bytes will start to be copied to.
 * @param h How many lines of the source buffer that will be copied.
 */
void DrawFlask(BYTE *pCelBuff, int w, int nSrcOff, BYTE *pBuff, int nDstOff, int h)
{
	int wdt, hgt;
	BYTE *src, *dst;

	src = &pCelBuff[nSrcOff];
	dst = &pBuff[nDstOff];

	for (hgt = h; hgt; hgt--, src += w - 59, dst += BUFFER_WIDTH - 59) {
		for (wdt = 59; wdt; wdt--) {
			if (*src)
				*dst = *src;
			src++;
			dst++;
		}
	}
}

/**
 * Draws the top dome of the life flask (that part that protrudes out of the control panel).
 * First it draws the empty flask cel and then draws the filled part on top if needed.
 */
void DrawLifeFlask()
{
	if (options_hwRendering) //Fluffy: If true, we'll be handling everything to do with life flask rendering in UpdateLifeFlask()
		return;

	double p;
	int filled;

	p = 0.0;
	if (plr[myplr]._pMaxHP > 0) {
		p = (double)plr[myplr]._pHitPoints / (double)plr[myplr]._pMaxHP * 80.0;
	}
	plr[myplr]._pHPPer = p;
	filled = plr[myplr]._pHPPer;

	if (filled > 80)
		filled = 80;

	filled = 80 - filled;

	if (filled > 11)
		filled = 11;
	filled += 2;

	DrawFlask(pLifeBuff, 88, 88 * 3 + 13, gpBuffer, SCREENXY(PANEL_LEFT + 109, PANEL_TOP - 13), filled);
	if (filled != 13){
		DrawFlask(pBtmBuff, PANEL_WIDTH, PANEL_WIDTH * (filled + 3) + 109, gpBuffer, SCREENXY(PANEL_LEFT + 109, PANEL_TOP - 13 + filled), 13 - filled);
	}
}

static void DrawFlask_SDL(int x, int startY, int texture)
{
	int y = PANEL_TOP - 16 + startY;
	if (options_animatedUIFlasks) {
		Render_Texture_Crop(PANEL_LEFT + x, y, texture, -1, startY, -1, -1, gameplayTickCount % 48);
		if (gameplayTickCount_progress != 0) {
			int frameNum = (gameplayTickCount + 1) % 48;
			SDL_SetTextureAlphaMod(textures[texture].frames[frameNum].frame, (gameplayTickCount_progress * 255) / (gSpeedMod));
			Render_Texture_Crop(PANEL_LEFT + x, y, texture, -1, startY, -1, -1, frameNum);
			SDL_SetTextureAlphaMod(textures[texture].frames[frameNum].frame, 255);
		}
	} else
		Render_Texture_Crop(PANEL_LEFT + x, y, texture, 96, startY, 96 + 88, 88);
}

/**
 * Controls the drawing of the area of the life flask within the control panel.
 * First sets the fill amount then draws the empty flask cel portion then the filled
 * flask portion.
 */
void UpdateLifeFlask()
{
	if (options_hwRendering) { //Fluffy: Render via SDL
		//First, draw empty life flask
		Render_Texture(PANEL_LEFT + 96, PANEL_TOP - 16, TEXTURE_HUDPANEL_EMPTYFLASKS, 0);

		//Then, draw the full life flask depending on quantity of health
		int texture = TEXTURE_HUDPANEL;
		if (talkflag)
			texture = TEXTURE_HUDPANEL_VOICE;
		int startY = 3; //We skip the first 3 pixels as they're not part of the flask
		int filled = 0;
		if (plr[myplr]._pMaxHP > 0) {
			filled = (double)plr[myplr]._pHitPoints / (double)plr[myplr]._pMaxHP * (88 - startY);
		}
		startY += (88 - startY) - (int)filled;
		if (startY < 88)
			DrawFlask_SDL(96, startY, options_animatedUIFlasks ? TEXTURE_HEALTHFLASK : texture);
		return;
	}

	if (options_animatedUIFlasks) { //Fluffy: Draw the entire flask as empty
		SetFlaskHeight(pLifeBuff, 16, 85, 96 + PANEL_X, PANEL_Y);
		return;
	}

	double p;
	int filled;

	p = 0.0;
	if (plr[myplr]._pMaxHP > 0) {
		p = (double)plr[myplr]._pHitPoints / (double)plr[myplr]._pMaxHP * 80.0;
	}
	filled = p;
	plr[myplr]._pHPPer = filled;

	if (filled > 69)
		filled = 69;
	else if (filled < 0)
		filled = 0;
	if (filled != 69)
		SetFlaskHeight(pLifeBuff, 16, 85 - filled, 96 + PANEL_X, PANEL_Y);
	if (filled != 0)
		DrawPanelBox(96, 85 - filled, 88, filled, 96 + PANEL_X, PANEL_Y + 69 - filled);
}

void DrawManaFlask()
{
	if (options_hwRendering) //Fluffy: If true, we'll be handling everything to do with mana flask rendering in UpdateManaFlask()
		return;

	int filled = plr[myplr]._pManaPer;
	if (filled > 80)
		filled = 80;
	filled = 80 - filled;
	

	if (filled > 11)
		filled = 11;
	filled += 2;

	DrawFlask(pManaBuff, 88, 88 * 3 + 13, gpBuffer, SCREENXY(PANEL_LEFT + 475, PANEL_TOP - 13), filled);
	if (filled != 13)
		DrawFlask(pBtmBuff, PANEL_WIDTH, PANEL_WIDTH * (filled + 3) + 475, gpBuffer, SCREENXY(PANEL_LEFT + 475, PANEL_TOP - 13 + filled), 13 - filled);
}

void control_update_life_mana()
{
	int manaPer;
	int maxMana = plr[myplr]._pMaxMana;
	int mana = plr[myplr]._pMana;
	if (maxMana < 0)
		maxMana = 0;
	if (mana < 0)
		mana = 0;
	if (maxMana == 0)
		manaPer = 0;
	else
		manaPer = (double)mana / (double)maxMana * 80.0;
	plr[myplr]._pManaPer = manaPer;
	plr[myplr]._pHPPer = (double)plr[myplr]._pHitPoints / (double)plr[myplr]._pMaxHP * 80.0;
}

/**
 * Controls the drawing of the area of the life flask within the control panel.
 * Also for some reason draws the current right mouse button spell.
 */
void UpdateManaFlask()
{
	int maxMana = plr[myplr]._pMaxMana;
	int mana = plr[myplr]._pMana;
	if (maxMana < 0)
		maxMana = 0;
	if (mana < 0)
		mana = 0;

	if (options_hwRendering) { //Fluffy: Render via SDL
		//First, draw empty mana flask
		Render_Texture(PANEL_LEFT + 464, PANEL_TOP - 16, TEXTURE_HUDPANEL_EMPTYFLASKS, 1);

		//Then, draw the full mana flask depending on quantity of mana
		int texture = TEXTURE_HUDPANEL;
		if (talkflag)
			texture = TEXTURE_HUDPANEL_VOICE;
		int startY = 3; //We skip the first 3 pixels as they're not part of the flask
		int filled = 0;
		if (maxMana > 0) {
			filled = (double)mana / (double)maxMana * (88 - startY);
		}
		startY += (88 - startY) - (int)filled;
		if (startY < 88)
			DrawFlask_SDL(464, startY, options_animatedUIFlasks ? TEXTURE_MANAFLASK : texture);
		goto spellrender;
	}

	int filled;

	if (maxMana == 0)
		filled = 0;
	else
		filled = (double)mana / (double)maxMana * 80.0;

	plr[myplr]._pManaPer = filled;

	if (filled > 69)
		filled = 69;
	if (filled != 69)
		SetFlaskHeight(pManaBuff, 16, 85 - filled, PANEL_X + 464, PANEL_Y);
	if (filled != 0)
		DrawPanelBox(464, 85 - filled, 88, filled, PANEL_X + 464, PANEL_Y + 69 - filled);

	spellrender:
	DrawSpell();
}

void InitControlPan()
{
	int i;

	if (gbMaxPlayers == 1) {
		pBtmBuff = DiabloAllocPtr((PANEL_HEIGHT + 16) * PANEL_WIDTH);
		memset(pBtmBuff, 0, (PANEL_HEIGHT + 16) * PANEL_WIDTH);
	} else {
		pBtmBuff = DiabloAllocPtr((PANEL_HEIGHT + 16) * 2 * PANEL_WIDTH);
		memset(pBtmBuff, 0, (PANEL_HEIGHT + 16) * 2 * PANEL_WIDTH);
	}
	pManaBuff = DiabloAllocPtr(88 * 88);
	memset(pManaBuff, 0, 88 * 88);
	pLifeBuff = DiabloAllocPtr(88 * 88);
	memset(pLifeBuff, 0, 88 * 88);
	pPanelText = LoadFileInMem("CtrlPan\\SmalText.CEL", NULL);
	pChrPanel = LoadFileInMem("Data\\Char.CEL", NULL);
	if (!gbIsHellfire)
		pSpellCels = LoadFileInMem("CtrlPan\\SpelIcon.CEL", NULL);
	else
		pSpellCels = LoadFileInMem("Data\\SpelIcon.CEL", NULL);
	SetSpellTrans(RSPLTYPE_SKILL);
	BYTE *panel8 = LoadFileInMem("CtrlPan\\Panel8.CEL", NULL);
	CelBlitWidth(pBtmBuff, 0, (PANEL_HEIGHT + 16) - 1, PANEL_WIDTH, panel8, 1, PANEL_WIDTH);
	pStatusPanel = LoadFileInMem("CtrlPan\\P8Bulbs.CEL", NULL);
	CelBlitWidth(pLifeBuff, 0, 87, 88, pStatusPanel, 1, 88);
	CelBlitWidth(pManaBuff, 0, 87, 88, pStatusPanel, 2, 88);
	talkflag = FALSE;
	if (gbMaxPlayers != 1) {
		pTalkPanel = LoadFileInMem("CtrlPan\\TalkPanl.CEL", NULL);
		CelBlitWidth(pBtmBuff, 0, (PANEL_HEIGHT + 16) * 2 - 1, PANEL_WIDTH, pTalkPanel, 1, PANEL_WIDTH);
		pMultiBtns = LoadFileInMem("CtrlPan\\P8But2.CEL", NULL);
		pTalkBtns = LoadFileInMem("CtrlPan\\TalkButt.CEL", NULL);
		sgbPlrTalkTbl = 0;
		sgszTalkMsg[0] = '\0';
		for (i = 0; i < MAX_PLRS; i++)
			whisper[i] = TRUE;
		for (i = 0; i < sizeof(talkbtndown) / sizeof(talkbtndown[0]); i++)
			talkbtndown[i] = FALSE;
	}
	panelflag = FALSE;
	lvlbtndown = FALSE;
	pPanelButtons = LoadFileInMem("CtrlPan\\Panel8bu.CEL", NULL);
	for (i = 0; i < sizeof(panbtn) / sizeof(panbtn[0]); i++)
		panbtn[i] = FALSE;
	panbtndown = FALSE;
	if (gbMaxPlayers == 1)
		numpanbtns = 6;
	else
		numpanbtns = 8;
	pChrButtons = LoadFileInMem("Data\\CharBut.CEL", NULL);
	for (i = 0; i < sizeof(chrbtn) / sizeof(chrbtn[0]); i++)
		chrbtn[i] = FALSE;
	chrbtnactive = FALSE;
	pDurIcons = LoadFileInMem("Items\\DurIcons.CEL", NULL);
	strcpy(infostr, "");
	ClearPanel();
	drawhpflag = TRUE;
	drawmanaflag = TRUE;
	chrflag = FALSE;
	spselflag = FALSE;
	pSpellBkCel = LoadFileInMem("Data\\SpellBk.CEL", NULL);
	pSBkBtnCel = LoadFileInMem("Data\\SpellBkB.CEL", NULL);
	pSBkIconCels = LoadFileInMem("Data\\SpellI2.CEL", NULL);
	sbooktab = 0;
	sbookflag = FALSE;
	if (plr[myplr]._pClass == PC_WARRIOR) {
		SpellPages[0][0] = SPL_REPAIR;
	} else if (plr[myplr]._pClass == PC_ROGUE) {
		SpellPages[0][0] = SPL_DISARM;
	} else if (plr[myplr]._pClass == PC_SORCERER) {
		SpellPages[0][0] = SPL_RECHARGE;
	} else if (plr[myplr]._pClass == PC_MONK) {
		SpellPages[0][0] = SPL_SEARCH;
	} else if (plr[myplr]._pClass == PC_BARD) {
		SpellPages[0][0] = SPL_IDENTIFY;
	} else if (plr[myplr]._pClass == PC_BARBARIAN) {
		SpellPages[0][0] = SPL_BLODBOIL;
	}
	pQLogCel = LoadFileInMem("Data\\Quest.CEL", NULL);
	pGBoxBuff = LoadFileInMem("CtrlPan\\Golddrop.cel", NULL);
	dropGoldFlag = FALSE;
	dropGoldValue = 0;
	initialDropGoldValue = 0;
	initialDropGoldIndex = 0;

	if (options_hwRendering) { //Fluffy: Load the above CELs as SDL textures
		Texture_ConvertCEL_MultipleFrames(pPanelText, TEXTURE_SMALLFONT, 13);
		Texture_ConvertCEL_SingleFrame(pChrPanel, TEXTURE_STATWINDOW, SPANEL_WIDTH);
		int charButWidths[9] = { 95, 41, 41, 41, 41, 41, 41, 41, 41 };
		Texture_ConvertCEL_MultipleFrames_VariableResolution(pChrButtons, TEXTURE_STATWINDOW_BUTTONS, charButWidths);

		//Spell icons
		Texture_ConvertCEL_MultipleFrames(pSpellCels, TEXTURE_SPELLICONS, SPLICONLENGTH);
		Texture_ConvertCEL_MultipleFrames(pSBkIconCels, TEXTURE_SMALLSPELLICONS, SPLSMALLICONSIZE);
		celConvert_TranslationTable = SplTransTbl;
		SetSpellTrans(RSPLTYPE_SPELL);
		Texture_ConvertCEL_MultipleFrames(pSpellCels, TEXTURE_SPELLICONS_SPELL, SPLICONLENGTH);
		Texture_ConvertCEL_MultipleFrames(pSBkIconCels, TEXTURE_SMALLSPELLICONS_SPELL, SPLSMALLICONSIZE);
		SetSpellTrans(RSPLTYPE_SCROLL);
		Texture_ConvertCEL_MultipleFrames(pSpellCels, TEXTURE_SPELLICONS_SCROLL, SPLICONLENGTH);
		Texture_ConvertCEL_MultipleFrames(pSBkIconCels, TEXTURE_SMALLSPELLICONS_SCROLL, SPLSMALLICONSIZE);
		SetSpellTrans(RSPLTYPE_CHARGES);
		Texture_ConvertCEL_MultipleFrames(pSpellCels, TEXTURE_SPELLICONS_CHARGES, SPLICONLENGTH);
		Texture_ConvertCEL_MultipleFrames(pSBkIconCels, TEXTURE_SMALLSPELLICONS_CHARGES, SPLSMALLICONSIZE);
		SetSpellTrans(RSPLTYPE_INVALID);
		Texture_ConvertCEL_MultipleFrames(pSpellCels, TEXTURE_SPELLICONS_INVALID, SPLICONLENGTH);
		Texture_ConvertCEL_MultipleFrames(pSBkIconCels, TEXTURE_SMALLSPELLICONS_INVALID, SPLSMALLICONSIZE);
		celConvert_TranslationTable = 0;

		Texture_ConvertCEL_SingleFrame(pSpellBkCel, TEXTURE_SPELLBOOK, SPANEL_WIDTH);
		Texture_ConvertCEL_MultipleFrames(pSBkBtnCel, TEXTURE_SPELLBOOK_BUTTONS, 61); //TODO: There's one reference to this being width 76. What is the context of that?
		Texture_ConvertCEL_SingleFrame(pQLogCel, TEXTURE_QUESTLOG, SPANEL_WIDTH);
		Texture_ConvertCEL_SingleFrame(pGBoxBuff, TEXTURE_GOLDDROPSELECTION, 261);
		Texture_ConvertCEL_MultipleFrames(pDurIcons, TEXTURE_DURABILITYWARNING, 32);

		//Panel textures
		Texture_ConvertCEL_SingleFrame(panel8, TEXTURE_HUDPANEL, PANEL_WIDTH);
		Texture_ConvertCEL_MultipleFrames(pStatusPanel, TEXTURE_HUDPANEL_EMPTYFLASKS, 88);
		Texture_ConvertCEL_MultipleFrames(pPanelButtons, TEXTURE_HUDPANEL_BUTTONS, 71);
		if (gbMaxPlayers != 1) {
			Texture_ConvertCEL_MultipleFrames(pMultiBtns, TEXTURE_HUDPANEL_MPBUTTONS, 33);
			Texture_ConvertCEL_MultipleFrames(pTalkBtns, TEXTURE_HUDPANEL_TALKBUTTONS, 61);
			Texture_ConvertCEL_SingleFrame(pTalkPanel, TEXTURE_HUDPANEL_VOICE, PANEL_WIDTH);
		}
	}

	//Free up loaded CEL data we won't be referencing anymore
	MemFreeDbg(panel8);
	MemFreeDbg(pStatusPanel);
	if (gbMaxPlayers != 1)
		MemFreeDbg(pTalkPanel);
}

void DrawCtrlPan()
{
	DrawPanelBox(0, sgbPlrTalkTbl + 16, PANEL_WIDTH, PANEL_HEIGHT, PANEL_X, PANEL_Y);
	DrawInfoBox();
}

static void DrawCtrlButton(int x, int y, BYTE *celData, int width, int frame, int texture)
{
	if (options_hwRendering) //Fluffy: Render via SDL
		Render_Texture_FromBottomLeft(x + PANEL_LEFT, y + PANEL_TOP, texture, frame - 1);
	else
		CelDraw(x + PANEL_X, y + PANEL_Y, celData, frame, width);
}

/**
 * Draws the control panel buttons in their current state. If the button is in the default
 * state draw it from the panel cel(extract its sub-rect). Else draw it from the buttons cel.
 */
void DrawCtrlBtns()
{
	int i;

	for (i = 0; i < 6; i++) {
		if (!panbtn[i])
			DrawPanelBox(PanBtnPos[i][0], PanBtnPos[i][1] + 16, 71, 20, PanBtnPos[i][0] + PANEL_X, PanBtnPos[i][1] + PANEL_Y);
		else
			DrawCtrlButton(PanBtnPos[i][0], PanBtnPos[i][1] + 18, pPanelButtons, 71, i + 1, TEXTURE_HUDPANEL_BUTTONS);
	}
	if (numpanbtns == 8) {
		DrawCtrlButton(87, 122, pMultiBtns, 33, panbtn[6] + 1, TEXTURE_HUDPANEL_MPBUTTONS);
		DrawCtrlButton(527, 122, pMultiBtns, 33, FriendlyMode ? panbtn[7] + 3 : panbtn[7] + 5, TEXTURE_HUDPANEL_MPBUTTONS);
	}
}

/**
 * Draws the "Speed Book": the rows of known spells for quick-setting a spell that
 * show up when you click the spell slot at the control panel.
 */
void DoSpeedBook()
{
	unsigned __int64 spell, spells;
	int xo, yo, X, Y, i, j;

	spselflag = TRUE;
	xo = PANEL_X + 12 + SPLICONLENGTH * 10;
	yo = PANEL_Y - 17;
	X = xo - (BORDER_LEFT - SPLICONLENGTH / 2);
	Y = yo - (BORDER_TOP + SPLICONLENGTH / 2);

	int maxSpells = gbIsHellfire ? MAX_SPELLS : 37;

	if (plr[myplr]._pRSpell != SPL_INVALID) {
		for (i = 0; i < 4; i++) {
			switch (i) {
			case RSPLTYPE_SKILL:
				spells = plr[myplr]._pAblSpells;
				break;
			case RSPLTYPE_SPELL:
				spells = plr[myplr]._pMemSpells;
				break;
			case RSPLTYPE_SCROLL:
				spells = plr[myplr]._pScrlSpells;
				break;
			case RSPLTYPE_CHARGES:
				spells = plr[myplr]._pISpells;
				break;
			}
			spell = (__int64)1;
			for (j = 1; j < maxSpells; j++) {
				if (spell & spells) {
					if (j == plr[myplr]._pRSpell && i == plr[myplr]._pRSplType) {
						X = xo - (BORDER_LEFT - SPLICONLENGTH / 2);
						Y = yo - (BORDER_TOP + SPLICONLENGTH / 2);
					}
					xo -= SPLICONLENGTH;
					if (xo == PANEL_X + 12 - SPLICONLENGTH) {
						xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
						yo -= SPLICONLENGTH;
					}
				}
				spell <<= (__int64)1;
			}
			if (spells && xo != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS)
				xo -= SPLICONLENGTH;
			if (xo == PANEL_X + 12 - SPLICONLENGTH) {
				xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				yo -= SPLICONLENGTH;
			}
		}
	}

	SetCursorPos(X, Y);
}

/**
 * Checks if the mouse cursor is within any of the panel buttons and flag it if so.
 */
void DoPanBtn()
{
	int i;

	for (i = 0; i < numpanbtns; i++) {
		int x = PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2];
		int y = PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3];
		if (MouseX >= PanBtnPos[i][0] + PANEL_LEFT && MouseX <= x) {
			if (MouseY >= PanBtnPos[i][1] + PANEL_TOP && MouseY <= y) {
				panbtn[i] = TRUE;
				drawbtnflag = TRUE;
				panbtndown = TRUE;
			}
		}
	}
	if (!spselflag && MouseX >= 565 + PANEL_LEFT && MouseX < 621 + PANEL_LEFT && MouseY >= 64 + PANEL_TOP && MouseY < 120 + PANEL_TOP) {
		DoSpeedBook();
		gamemenu_off();
	}
}

void control_set_button_down(int btn_id)
{
	panbtn[btn_id] = TRUE;
	drawbtnflag = TRUE;
	panbtndown = TRUE;
}

void control_check_btn_press()
{
	int x, y;

	x = PanBtnPos[3][0] + PANEL_LEFT + PanBtnPos[3][2];
	y = PanBtnPos[3][1] + PANEL_TOP + PanBtnPos[3][3];
	if (MouseX >= PanBtnPos[3][0] + PANEL_LEFT
	    && MouseX <= x
	    && MouseY >= PanBtnPos[3][1] + PANEL_TOP
	    && MouseY <= y) {
		control_set_button_down(3);
	}
	x = PanBtnPos[6][0] + PANEL_LEFT + PanBtnPos[6][2];
	y = PanBtnPos[6][1] + PANEL_TOP + PanBtnPos[6][3];
	if (MouseX >= PanBtnPos[6][0] + PANEL_LEFT
	    && MouseX <= x
	    && MouseY >= PanBtnPos[6][1] + PANEL_TOP
	    && MouseY <= y) {
		control_set_button_down(6);
	}
}

void DoAutoMap()
{
	if (currlevel != 0 || gbMaxPlayers != 1) {
		if (!automapflag)
			StartAutomap();
		else
			automapflag = FALSE;
	} else {
		InitDiabloMsg(EMSG_NO_AUTOMAP_IN_TOWN);
	}
}

/**
 * Checks the mouse cursor position within the control panel and sets information
 * strings if needed.
 */
void CheckPanelInfo()
{
	int i, c, v, s, xend, yend;

	panelflag = FALSE;
	ClearPanel();
	for (i = 0; i < numpanbtns; i++) {
		xend = PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2];
		yend = PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3];
		if (MouseX >= PanBtnPos[i][0] + PANEL_LEFT && MouseX <= xend && MouseY >= PanBtnPos[i][1] + PANEL_TOP && MouseY <= yend) {
			if (i != 7) {
				strcpy(infostr, PanBtnStr[i]);
			} else {
				if (FriendlyMode)
					strcpy(infostr, "Player friendly");
				else
					strcpy(infostr, "Player attack");
			}
			if (PanBtnHotKey[i] != NULL) {
				sprintf(tempstr, "Hotkey : %s", PanBtnHotKey[i]);
				AddPanelString(tempstr, TRUE);
			}
			infoclr = COL_WHITE;
			panelflag = TRUE;
			pinfoflag = TRUE;
		}
	}
	if (!spselflag && MouseX >= 565 + PANEL_LEFT && MouseX < 621 + PANEL_LEFT && MouseY >= 64 + PANEL_TOP && MouseY < 120 + PANEL_TOP) {
		strcpy(infostr, "Select current spell button");
		infoclr = COL_WHITE;
		panelflag = TRUE;
		pinfoflag = TRUE;
		strcpy(tempstr, "Hotkey : 's'");
		AddPanelString(tempstr, TRUE);
		v = plr[myplr]._pRSpell;
		if (v != SPL_INVALID) {
			switch (plr[myplr]._pRSplType) {
			case RSPLTYPE_SKILL:
				sprintf(tempstr, "%s Skill", spelldata[v].sSkillText);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_SPELL:
				sprintf(tempstr, "%s Spell", spelldata[v].sNameText);
				AddPanelString(tempstr, TRUE);
				c = plr[myplr]._pISplLvlAdd + plr[myplr]._pSplLvl[v];
				if (c < 0)
					c = 0;
				if (c == 0)
					sprintf(tempstr, "Spell Level 0 - Unusable");
				else
					sprintf(tempstr, "Spell Level %i", c);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_SCROLL:
				sprintf(tempstr, "Scroll of %s", spelldata[v].sNameText);
				AddPanelString(tempstr, TRUE);
				s = 0;
				for (i = 0; i < plr[myplr]._pNumInv; i++) {
					if (plr[myplr].InvList[i]._itype != ITYPE_NONE
					    && (plr[myplr].InvList[i]._iMiscId == IMISC_SCROLL || plr[myplr].InvList[i]._iMiscId == IMISC_SCROLLT)
					    && plr[myplr].InvList[i]._iSpell == v) {
						s++;
					}
				}
				for (i = 0; i < MAXBELTITEMS; i++) {
					if (plr[myplr].SpdList[i]._itype != ITYPE_NONE
					    && (plr[myplr].SpdList[i]._iMiscId == IMISC_SCROLL || plr[myplr].SpdList[i]._iMiscId == IMISC_SCROLLT)
					    && plr[myplr].SpdList[i]._iSpell == v) {
						s++;
					}
				}
				if (s == 1)
					strcpy(tempstr, "1 Scroll");
				else
					sprintf(tempstr, "%i Scrolls", s);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_CHARGES:
				sprintf(tempstr, "Staff of %s", spelldata[v].sNameText);
				AddPanelString(tempstr, TRUE);
				if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges == 1)
					strcpy(tempstr, "1 Charge");
				else
					sprintf(tempstr, "%i Charges", plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges);
				AddPanelString(tempstr, TRUE);
				break;
			}
		}
	}
	if (MouseX > 190 + PANEL_LEFT && MouseX < 437 + PANEL_LEFT && MouseY > 4 + PANEL_TOP && MouseY < 33 + PANEL_TOP)
		pcursinvitem = CheckInvHLight();
}

/**
 * Check if the mouse is within a control panel button that's flagged.
 * Takes apropiate action if so.
 */
void CheckBtnUp()
{
	int i;
	BOOLEAN gamemenuOff;

	gamemenuOff = TRUE;
	drawbtnflag = TRUE;
	panbtndown = FALSE;

	for (i = 0; i < 8; i++) {
		if (!panbtn[i]) {
			continue;
		}

		panbtn[i] = FALSE;

		if (MouseX < PanBtnPos[i][0] + PANEL_LEFT
		    || MouseX > PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2]
		    || MouseY < PanBtnPos[i][1] + PANEL_TOP
		    || MouseY > PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3]) {
			continue;
		}

		switch (i) {
		case PANBTN_CHARINFO:
			questlog = FALSE;
			chrflag = !chrflag;
			break;
		case PANBTN_QLOG:
			chrflag = FALSE;
			if (!questlog)
				StartQuestlog();
			else
				questlog = FALSE;
			break;
		case PANBTN_AUTOMAP:
			DoAutoMap();
			break;
		case PANBTN_MAINMENU:
			qtextflag = FALSE;
			gamemenu_handle_previous();
			gamemenuOff = FALSE;
			break;
		case PANBTN_INVENTORY:
			sbookflag = FALSE;
			invflag = !invflag;
			if (dropGoldFlag) {
				dropGoldFlag = FALSE;
				dropGoldValue = 0;
			}
			break;
		case PANBTN_SPELLBOOK:
			invflag = FALSE;
			if (dropGoldFlag) {
				dropGoldFlag = FALSE;
				dropGoldValue = 0;
			}
			sbookflag = !sbookflag;
			break;
		case PANBTN_SENDMSG:
			if (talkflag)
				control_reset_talk();
			else
				control_type_message();
			break;
		case PANBTN_FRIENDLY:
			FriendlyMode = !FriendlyMode;
			break;
		}
	}

	if (gamemenuOff)
		gamemenu_off();
}

void FreeControlPan()
{
	MemFreeDbg(pBtmBuff);
	MemFreeDbg(pManaBuff);
	MemFreeDbg(pLifeBuff);
	MemFreeDbg(pPanelText);
	MemFreeDbg(pChrPanel);
	MemFreeDbg(pSpellCels);
	MemFreeDbg(pPanelButtons);
	MemFreeDbg(pMultiBtns);
	MemFreeDbg(pTalkBtns);
	MemFreeDbg(pChrButtons);
	MemFreeDbg(pDurIcons);
	MemFreeDbg(pQLogCel);
	MemFreeDbg(pSpellBkCel);
	MemFreeDbg(pSBkBtnCel);
	MemFreeDbg(pSBkIconCels);
	MemFreeDbg(pGBoxBuff);
}

BOOL control_WriteStringToBuffer(BYTE *str)
{
	int k;
	BYTE ichar;

	k = 0;
	while (*str) {
		ichar = gbFontTransTbl[*str];
		str++;
		k += fontkern[fontframe[ichar]];
		if (k >= 125)
			return FALSE;
	}

	return TRUE;
}

static void CPrintString(int y, const char *str, BOOL center, int lines)
{
	BYTE c;
	const char *tmp;
	int lineOffset, strWidth, sx, sy;

	lineOffset = 0;
	sx = 177 + PANEL_X;
	sy = lineOffsets[lines][y] + PANEL_Y;
	if (center == TRUE) {
		strWidth = 0;
		tmp = str;
		while (*tmp) {
			c = gbFontTransTbl[(BYTE)*tmp++];
			strWidth += fontkern[fontframe[c]] + 2;
		}
		if (strWidth < 288)
			lineOffset = (288 - strWidth) >> 1;
		sx += lineOffset;
	}
	while (*str) {
		c = gbFontTransTbl[(BYTE)*str++];
		c = fontframe[c];
		lineOffset += fontkern[c] + 2;
		if (c) {
			if (lineOffset < 288) {
				PrintChar(sx, sy, c, infoclr);
			}
		}
		sx += fontkern[c] + 2;
	}
}

static void PrintInfo()
{
	int yo, lo, i;

	if (!talkflag) {
		yo = 0;
		lo = 1;
		if (infostr[0] != '\0') {
			CPrintString(0, infostr, TRUE, pnumlines);
			yo = 1;
			lo = 0;
		}

		for (i = 0; i < pnumlines; i++) {
			CPrintString(i + yo, panelstr[i], pstrjust[i], pnumlines - lo);
		}
	}
}

/**
 * Sets a string to be drawn in the info box and then draws it.
 */
void DrawInfoBox()
{
	int nGold;

	DrawPanelBox(177, 62, 288, 60, PANEL_X + 177, PANEL_Y + 46);
	if (!panelflag && !trigflag && pcursinvitem == -1 && !spselflag) {
		infostr[0] = '\0';
		infoclr = COL_WHITE;
		ClearPanel();
	}
	if (spselflag || trigflag) {
		infoclr = COL_WHITE;
	} else if (pcurs >= CURSOR_FIRSTITEM) {
		if (plr[myplr].HoldItem._itype == ITYPE_GOLD) {
			nGold = plr[myplr].HoldItem._ivalue;
			sprintf(infostr, "%i gold %s", nGold, get_pieces_str(nGold));
		} else if (!plr[myplr].HoldItem._iStatFlag) {
			ClearPanel();
			AddPanelString("Requirements not met", TRUE);
			pinfoflag = TRUE;
		} else {
			if (plr[myplr].HoldItem._iIdentified)
				strcpy(infostr, plr[myplr].HoldItem._iIName);
			else
				strcpy(infostr, plr[myplr].HoldItem._iName);
			if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_MAGIC)
				infoclr = COL_BLUE;
			if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_UNIQUE)
				infoclr = COL_GOLD;
		}
	} else {
		if (pcursitem != -1)
			GetItemStr(pcursitem);
		else if (pcursobj != -1)
			GetObjectStr(pcursobj);
		if (pcursmonst != -1) {
			if (leveltype != DTYPE_TOWN) {
				infoclr = COL_WHITE;
				strcpy(infostr, monster[pcursmonst].mName);
				ClearPanel();
				if (monster[pcursmonst]._uniqtype != 0) {
					infoclr = COL_GOLD;
					PrintUniqueHistory();
				} else {
					PrintMonstHistory(monster[pcursmonst].MType->mtype);
				}
			} else if (pcursitem == -1) {
				strcpy(infostr, towner[pcursmonst]._tName);
			}
		}
		if (pcursplr != -1) {
			infoclr = COL_GOLD;
			strcpy(infostr, plr[pcursplr]._pName);
			ClearPanel();
			sprintf(tempstr, "%s, Level : %i", ClassStrTblOld[plr[pcursplr]._pClass], plr[pcursplr]._pLevel);
			AddPanelString(tempstr, TRUE);
			sprintf(tempstr, "Hit Points %i of %i", plr[pcursplr]._pHitPoints >> 6, plr[pcursplr]._pMaxHP >> 6);
			AddPanelString(tempstr, TRUE);
		}
	}
	if (infostr[0] != '\0' || pnumlines != 0)
		PrintInfo();
}

#define ADD_PlrStringXY(x, y, width, pszStr, col) MY_PlrStringXY(x, y, width, pszStr, col, 1)

void PrintGameStr(int x, int y, const char *str, int color)
{
	BYTE c;
	int sx, sy;
	sx = x + SCREEN_X;
	sy = y + SCREEN_Y;
	while (*str) {
		c = gbFontTransTbl[(BYTE)*str++];
		c = fontframe[c];
		if (c)
			PrintChar(sx, sy, c, color);
		sx += fontkern[c] + 1;
	}
}

/**
 * @brief Render text string to back buffer
 * @param x Screen coordinate
 * @param y Screen coordinate
 * @param endX End of line in screen coordinate
 * @param pszStr String to print, in Windows-1252 encoding
 * @param col text_color color value
 * @param base Letter spacing
 */
static void MY_PlrStringXY(int x, int y, int endX, const char *pszStr, char col, int base)
{
	BYTE c;
	const char *tmp;
	int sx, sy, screen_x, line, widthOffset;

	sx = x + SCREEN_X;
	sy = y + SCREEN_Y;
	widthOffset = endX - x + 1;
	line = 0;
	screen_x = 0;
	tmp = pszStr;
	while (*tmp) {
		c = gbFontTransTbl[(BYTE)*tmp++];
		screen_x += fontkern[fontframe[c]] + base;
	}
	if (screen_x < widthOffset)
		line = (widthOffset - screen_x) >> 1;
	sx += line;
	while (*pszStr) {
		c = gbFontTransTbl[(BYTE)*pszStr++];
		c = fontframe[c];
		line += fontkern[c] + base;
		if (c) {
			if (line < widthOffset)
				PrintChar(sx, sy, c, col);
		}
		sx += fontkern[c] + base;
	}
}

static void RenderLevelupIconOnStatWindow(int x, int y, int frame)
{
	if (options_hwRendering) //Fluffy: Render via SDL
		Render_Texture_FromBottomLeft(x, y, TEXTURE_STATWINDOW_BUTTONS, frame - 1);
	else
		CelDraw(x + SCREEN_X, y + SCREEN_Y, pChrButtons, frame, 41);
}

void DrawChr()
{
	char col;
	char chrstr[64];
	int pc, mindam, maxdam;

	if (options_hwRendering) //Fluffy: Render character stat window via SDL
		Render_Texture(0, 0, TEXTURE_STATWINDOW);
	else
		CelDraw(SCREEN_X, 351 + SCREEN_Y, pChrPanel, 1, SPANEL_WIDTH);

	ADD_PlrStringXY(20, 32, 151, plr[myplr]._pName, COL_WHITE);

	ADD_PlrStringXY(168, 32, 299, ClassStrTblOld[plr[myplr]._pClass], COL_WHITE);

	sprintf(chrstr, "%i", plr[myplr]._pLevel);
	ADD_PlrStringXY(66, 69, 109, chrstr, COL_WHITE);

	sprintf(chrstr, "%li", plr[myplr]._pExperience);
	ADD_PlrStringXY(216, 69, 300, chrstr, COL_WHITE);

	if (plr[myplr]._pLevel == MAXCHARLEVEL - 1) {
		strcpy(chrstr, "None");
		col = COL_GOLD;
	} else {
		sprintf(chrstr, "%li", plr[myplr]._pNextExper);
		col = COL_WHITE;
	}
	ADD_PlrStringXY(216, 97, 300, chrstr, col);

	sprintf(chrstr, "%i", plr[myplr]._pGold);
	ADD_PlrStringXY(216, 146, 300, chrstr, COL_WHITE);

	col = COL_WHITE;
	if (plr[myplr]._pIBonusAC > 0)
		col = COL_BLUE;
	if (plr[myplr]._pIBonusAC < 0)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pIBonusAC + plr[myplr]._pIAC + plr[myplr]._pDexterity / 5);
	ADD_PlrStringXY(258, 183, 301, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pIBonusToHit > 0)
		col = COL_BLUE;
	if (plr[myplr]._pIBonusToHit < 0)
		col = COL_RED;
	sprintf(chrstr, "%i%%", (plr[myplr]._pDexterity >> 1) + plr[myplr]._pIBonusToHit + 50);
	ADD_PlrStringXY(258, 211, 301, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pIBonusDam > 0)
		col = COL_BLUE;
	if (plr[myplr]._pIBonusDam < 0)
		col = COL_RED;
	mindam = plr[myplr]._pIMinDam;
	mindam += plr[myplr]._pIBonusDam * mindam / 100;
	mindam += plr[myplr]._pIBonusDamMod;
	if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_BOW) {
		if (plr[myplr]._pClass == PC_ROGUE)
			mindam += plr[myplr]._pDamageMod;
		else
			mindam += plr[myplr]._pDamageMod >> 1;
	} else {
		mindam += plr[myplr]._pDamageMod;
	}
	maxdam = plr[myplr]._pIMaxDam;
	maxdam += plr[myplr]._pIBonusDam * maxdam / 100;
	maxdam += plr[myplr]._pIBonusDamMod;
	if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_BOW) {
		if (plr[myplr]._pClass == PC_ROGUE)
			maxdam += plr[myplr]._pDamageMod;
		else
			maxdam += plr[myplr]._pDamageMod >> 1;
	} else {
		maxdam += plr[myplr]._pDamageMod;
	}
	sprintf(chrstr, "%i-%i", mindam, maxdam);
	if (mindam >= 100 || maxdam >= 100)
		MY_PlrStringXY(254, 239, 305, chrstr, col, -1);
	else
		MY_PlrStringXY(258, 239, 301, chrstr, col, 0);

	if (plr[myplr]._pMagResist == 0)
		col = COL_WHITE;
	else
		col = COL_BLUE;
	if (plr[myplr]._pMagResist < MAXRESIST) {
		sprintf(chrstr, "%i%%", plr[myplr]._pMagResist);
	} else {
		col = COL_GOLD;
		sprintf(chrstr, "MAX");
	}
	ADD_PlrStringXY(257, 276, 300, chrstr, col);

	if (plr[myplr]._pFireResist == 0)
		col = COL_WHITE;
	else
		col = COL_BLUE;
	if (plr[myplr]._pFireResist < MAXRESIST) {
		sprintf(chrstr, "%i%%", plr[myplr]._pFireResist);
	} else {
		col = COL_GOLD;
		sprintf(chrstr, "MAX");
	}
	ADD_PlrStringXY(257, 304, 300, chrstr, col);

	if (plr[myplr]._pLghtResist == 0)
		col = COL_WHITE;
	else
		col = COL_BLUE;
	if (plr[myplr]._pLghtResist < MAXRESIST) {
		sprintf(chrstr, "%i%%", plr[myplr]._pLghtResist);
	} else {
		col = COL_GOLD;
		sprintf(chrstr, "MAX");
	}
	ADD_PlrStringXY(257, 332, 300, chrstr, col);

	col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pBaseStr);
	if (MaxStats[plr[myplr]._pClass][ATTRIB_STR] == plr[myplr]._pBaseStr)
		col = COL_GOLD;
	ADD_PlrStringXY(95, 155, 126, chrstr, col);

	col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pBaseMag);
	if (MaxStats[plr[myplr]._pClass][ATTRIB_MAG] == plr[myplr]._pBaseMag)
		col = COL_GOLD;
	ADD_PlrStringXY(95, 183, 126, chrstr, col);

	col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pBaseDex);
	if (MaxStats[plr[myplr]._pClass][ATTRIB_DEX] == plr[myplr]._pBaseDex)
		col = COL_GOLD;
	ADD_PlrStringXY(95, 211, 126, chrstr, col);

	col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pBaseVit);
	if (MaxStats[plr[myplr]._pClass][ATTRIB_VIT] == plr[myplr]._pBaseVit)
		col = COL_GOLD;
	ADD_PlrStringXY(95, 239, 126, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pStrength > plr[myplr]._pBaseStr)
		col = COL_BLUE;
	if (plr[myplr]._pStrength < plr[myplr]._pBaseStr)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pStrength);
	ADD_PlrStringXY(143, 155, 173, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pMagic > plr[myplr]._pBaseMag)
		col = COL_BLUE;
	if (plr[myplr]._pMagic < plr[myplr]._pBaseMag)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pMagic);
	ADD_PlrStringXY(143, 183, 173, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pDexterity > plr[myplr]._pBaseDex)
		col = COL_BLUE;
	if (plr[myplr]._pDexterity < plr[myplr]._pBaseDex)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pDexterity);
	ADD_PlrStringXY(143, 211, 173, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pVitality > plr[myplr]._pBaseVit)
		col = COL_BLUE;
	if (plr[myplr]._pVitality < plr[myplr]._pBaseVit)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pVitality);
	ADD_PlrStringXY(143, 239, 173, chrstr, col);

	if (plr[myplr]._pStatPts > 0) {
		if (CalcStatDiff(myplr) < plr[myplr]._pStatPts) {
			plr[myplr]._pStatPts = CalcStatDiff(myplr);
		}
	}
	if (plr[myplr]._pStatPts > 0) {
		sprintf(chrstr, "%i", plr[myplr]._pStatPts);
		ADD_PlrStringXY(95, 266, 126, chrstr, COL_RED);
		pc = plr[myplr]._pClass;

		//Fluffy: Draw the level-up icon in the character stat window
		int xCoord = 137;
		if (plr[myplr]._pBaseStr < MaxStats[pc][ATTRIB_STR])
			RenderLevelupIconOnStatWindow(xCoord, 159, chrbtn[ATTRIB_STR] + 2);
		if (plr[myplr]._pBaseMag < MaxStats[pc][ATTRIB_MAG])
			RenderLevelupIconOnStatWindow(xCoord, 187, chrbtn[ATTRIB_MAG] + 4);
		if (plr[myplr]._pBaseDex < MaxStats[pc][ATTRIB_DEX])
			RenderLevelupIconOnStatWindow(xCoord, 216, chrbtn[ATTRIB_DEX] + 6);
		if (plr[myplr]._pBaseVit < MaxStats[pc][ATTRIB_VIT])
			RenderLevelupIconOnStatWindow(xCoord, 244, chrbtn[ATTRIB_VIT] + 8);
	}

	if (plr[myplr]._pMaxHP > plr[myplr]._pMaxHPBase)
		col = COL_BLUE;
	else
		col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pMaxHP >> 6);
	ADD_PlrStringXY(95, 304, 126, chrstr, col);
	if (plr[myplr]._pHitPoints != plr[myplr]._pMaxHP)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pHitPoints >> 6);
	ADD_PlrStringXY(143, 304, 174, chrstr, col);

	if (plr[myplr]._pMaxMana > plr[myplr]._pMaxManaBase)
		col = COL_BLUE;
	else
		col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pMaxMana >> 6);
	ADD_PlrStringXY(95, 332, 126, chrstr, col);
	if (plr[myplr]._pMana != plr[myplr]._pMaxMana)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pMana >> 6);
	ADD_PlrStringXY(143, 332, 174, chrstr, col);
}

void CheckLvlBtn()
{
	if (!lvlbtndown && MouseX >= 40 + PANEL_LEFT && MouseX <= 81 + PANEL_LEFT && MouseY >= -39 + PANEL_TOP && MouseY <= -17 + PANEL_TOP)
		lvlbtndown = TRUE;
}

void ReleaseLvlBtn()
{
	if (MouseX >= 40 + PANEL_LEFT && MouseX <= 81 + PANEL_LEFT && MouseY >= -39 + PANEL_TOP && MouseY <= -17 + PANEL_TOP)
		chrflag = TRUE;
	lvlbtndown = FALSE;
}

void DrawLevelUpIcon()
{
	int nCel;

	if (stextflag == STORE_NONE) {
		nCel = lvlbtndown ? 3 : 2;
		ADD_PlrStringXY(PANEL_LEFT + 0, PANEL_TOP - 49, PANEL_LEFT + 120, "Level Up", COL_WHITE);

		if (options_hwRendering) //Fluffy: Render via SDL
			Render_Texture_FromBottomLeft(40 + PANEL_LEFT, -17 + PANEL_TOP, TEXTURE_STATWINDOW_BUTTONS, nCel - 1);
		else
			CelDraw(40 + PANEL_X, -17 + PANEL_Y, pChrButtons, nCel, 41);
	}
}

void CheckChrBtns()
{
	int pc, i, x, y;

	if (!chrbtnactive && plr[myplr]._pStatPts) {
		pc = plr[myplr]._pClass;
		for (i = 0; i < 4; i++) {
			switch (i) {
			case ATTRIB_STR:
				if (plr[myplr]._pBaseStr >= MaxStats[pc][ATTRIB_STR])
					continue;
				break;
			case ATTRIB_MAG:
				if (plr[myplr]._pBaseMag >= MaxStats[pc][ATTRIB_MAG])
					continue;
				break;
			case ATTRIB_DEX:
				if (plr[myplr]._pBaseDex >= MaxStats[pc][ATTRIB_DEX])
					continue;
				break;
			case ATTRIB_VIT:
				if (plr[myplr]._pBaseVit >= MaxStats[pc][ATTRIB_VIT])
					continue;
				break;
			default:
				continue;
			}
			x = ChrBtnsRect[i].x + ChrBtnsRect[i].w;
			y = ChrBtnsRect[i].y + ChrBtnsRect[i].h;
			if (MouseX >= ChrBtnsRect[i].x
			    && MouseX <= x
			    && MouseY >= ChrBtnsRect[i].y
			    && MouseY <= y) {
				chrbtn[i] = TRUE;
				chrbtnactive = TRUE;
			}
		}
	}
}

void ReleaseChrBtns()
{
	int i;

	chrbtnactive = FALSE;
	for (i = 0; i < 4; ++i) {
		if (chrbtn[i]) {
			chrbtn[i] = FALSE;
			if (MouseX >= ChrBtnsRect[i].x
			    && MouseX <= ChrBtnsRect[i].x + ChrBtnsRect[i].w
			    && MouseY >= ChrBtnsRect[i].y
			    && MouseY <= ChrBtnsRect[i].y + ChrBtnsRect[i].h) {
				switch (i) {
				case 0:
					NetSendCmdParam1(TRUE, CMD_ADDSTR, 1);
					plr[myplr]._pStatPts--;
					break;
				case 1:
					NetSendCmdParam1(TRUE, CMD_ADDMAG, 1);
					plr[myplr]._pStatPts--;
					break;
				case 2:
					NetSendCmdParam1(TRUE, CMD_ADDDEX, 1);
					plr[myplr]._pStatPts--;
					break;
				case 3:
					NetSendCmdParam1(TRUE, CMD_ADDVIT, 1);
					plr[myplr]._pStatPts--;
					break;
				}
			}
		}
	}
}

static int DrawDurIcon4Item(ItemStruct *pItem, int x, int c)
{
	if (pItem->_itype == ITYPE_NONE)
		return x;
	if (pItem->_iDurability > 5)
		return x;
	if (c == 0) {
		if (pItem->_iClass == ICLASS_WEAPON) {
			switch (pItem->_itype) {
			case ITYPE_SWORD:
				c = 2;
				break;
			case ITYPE_AXE:
				c = 6;
				break;
			case ITYPE_BOW:
				c = 7;
				break;
			case ITYPE_MACE:
				c = 5;
				break;
			case ITYPE_STAFF:
				c = 8;
				break;
			}
		} else {
			c = 1;
		}
	}
	if (pItem->_iDurability > 2)
		c += 8;
	if (options_hwRendering) //Fluffy: Render via SDL rendering
		Render_Texture_FromBottomLeft(x - BORDER_LEFT, -17 + PANEL_TOP, TEXTURE_DURABILITYWARNING, c - 1);
	else
		CelDraw(x, -17 + PANEL_Y, pDurIcons, c, 32);
	return x - 32 - 8;
}

void DrawDurIcon()
{
	PlayerStruct *p;
	int x;

	bool hasRoomBetweenPanels = SCREEN_WIDTH >= PANEL_WIDTH + 16 + (32 + 8 + 32 + 8 + 32 + 8 + 32) + 16;
	bool hasRoomUnderPanels = SCREEN_HEIGHT >= SPANEL_HEIGHT + PANEL_HEIGHT + 16 + 32 + 16;

	if (!hasRoomBetweenPanels && !hasRoomUnderPanels) {
		if ((chrflag || questlog) && (invflag || sbookflag))
			return;
	}

	x = PANEL_X + PANEL_WIDTH - 32 - 16;
	if (!hasRoomUnderPanels) {
		if (invflag || sbookflag)
			x -= SPANEL_WIDTH - (SCREEN_WIDTH - PANEL_WIDTH) / 2;
	}

	p = &plr[myplr];
	x = DrawDurIcon4Item(&p->InvBody[INVLOC_HEAD], x, 4);
	x = DrawDurIcon4Item(&p->InvBody[INVLOC_CHEST], x, 3);
	x = DrawDurIcon4Item(&p->InvBody[INVLOC_HAND_LEFT], x, 0);
	DrawDurIcon4Item(&p->InvBody[INVLOC_HAND_RIGHT], x, 0);
}

void RedBack()
{
	int idx;

	idx = light4flag ? 1536 : 4608;

	assert(gpBuffer);

	int w, h;
	BYTE *dst, *tbl;

	if (leveltype != DTYPE_HELL) {
		dst = &gpBuffer[SCREENXY(0, 0)];
		tbl = &pLightTbl[idx];
		for (h = VIEWPORT_HEIGHT; h; h--, dst += BUFFER_WIDTH - SCREEN_WIDTH) {
			for (w = SCREEN_WIDTH; w; w--) {
				*dst = tbl[*dst];
				dst++;
			}
		}
	} else {
		dst = &gpBuffer[SCREENXY(0, 0)];
		tbl = &pLightTbl[idx];
		for (h = VIEWPORT_HEIGHT; h; h--, dst += BUFFER_WIDTH - SCREEN_WIDTH) {
			for (w = SCREEN_WIDTH; w; w--) {
				if (*dst >= 32)
					*dst = tbl[*dst];
				dst++;
			}
		}
	}
}

static void PrintSBookStr(int x, int y, BOOL cjustflag, const char *pszStr, char col)
{
	BYTE c;
	const char *tmp;
	int screen_x, line, sx;

	sx = x + RIGHT_PANEL_X + SPLICONLENGTH;
	line = 0;
	if (cjustflag) {
		screen_x = 0;
		tmp = pszStr;
		while (*tmp) {
			c = gbFontTransTbl[(BYTE)*tmp++];
			screen_x += fontkern[fontframe[c]] + 1;
		}
		if (screen_x < 222)
			line = (222 - screen_x) >> 1;
		sx += line;
	}
	while (*pszStr) {
		c = gbFontTransTbl[(BYTE)*pszStr++];
		c = fontframe[c];
		line += fontkern[c] + 1;
		if (c) {
			if (line <= 222)
				PrintChar(sx, y, c, col);
		}
		sx += fontkern[c] + 1;
	}
}

char GetSBookTrans(int ii, BOOL townok)
{
	char st;

	if ((plr[myplr]._pClass == PC_MONK) && (ii == SPL_SEARCH))
		return RSPLTYPE_SKILL;
	st = RSPLTYPE_SPELL;
	if (plr[myplr]._pISpells & SPELLBIT(ii)) {
		st = RSPLTYPE_CHARGES;
	}
	if (plr[myplr]._pAblSpells & SPELLBIT(ii)) {
		st = RSPLTYPE_SKILL;
	}
	if (st == RSPLTYPE_SPELL) {
		if (!CheckSpell(myplr, ii, RSPLTYPE_SPELL, TRUE)) {
			st = RSPLTYPE_INVALID;
		}
		if ((char)(plr[myplr]._pSplLvl[ii] + plr[myplr]._pISplLvlAdd) <= 0) {
			st = RSPLTYPE_INVALID;
		}

		//Fluffy: Grey out spells in spell book if we can't cast in town
		if (townok && currlevel == 0 && st != RSPLTYPE_INVALID && !spelldata[ii].sTownSpell && gameSetup_allowAttacksInTown == false)
			st = RSPLTYPE_INVALID;
	}

	return st;
}

void DrawSpellBook()
{
	int i, sn, mana, lvl, yp, min, max;
	char st;
	unsigned __int64 spl;

	if (options_hwRendering) { //Fluffy: Render spellbook window and buttons via SDL
		Render_Texture(RIGHT_PANEL, 0, TEXTURE_SPELLBOOK);
		if (gbIsHellfire && sbooktab < 5)
			Render_Texture_FromBottomLeft(RIGHT_PANEL + 61 * sbooktab + 7, 348, TEXTURE_SPELLBOOK_BUTTONS, sbooktab);
		else if (gbIsHellfire && sbooktab < 4)
			Render_Texture_FromBottomLeft(RIGHT_PANEL + 76 * sbooktab + 7, 348, TEXTURE_SPELLBOOK_BUTTONS, sbooktab); //Probably needs the same fix as described below, and also... this probably causes a crash as I haven't seen these buttons with 76 as resolution
	} else {
		CelDraw(RIGHT_PANEL_X, 351 + SCREEN_Y, pSpellBkCel, 1, SPANEL_WIDTH);
		if (gbIsHellfire && sbooktab < 5)
			CelDraw(RIGHT_PANEL_X + 61 * sbooktab + 7, 348 + SCREEN_Y, pSBkBtnCel, sbooktab + 1, 61);
		else if (gbIsHellfire && sbooktab < 4)
			// BUGFIX: rendering of page 3 and page 4 buttons are both off-by-one pixel.
			// The fix would look as follows:
			//
			//    int sx = RIGHT_PANEL_X + 76 * sbooktab + 7;
			//    if (sbooktab == 2 || sbooktab == 3) {
			//       sx++;
			//    }
			//    CelDraw(sx, 348 + SCREEN_Y, pSBkBtnCel, sbooktab + 1, 76);
			CelDraw(RIGHT_PANEL_X + 76 * sbooktab + 7, 348 + SCREEN_Y, pSBkBtnCel, sbooktab + 1, 76);
	}

	spl = plr[myplr]._pMemSpells | plr[myplr]._pISpells | plr[myplr]._pAblSpells;

	yp = 55 + SCREEN_Y;
	for (i = 1; i < 8; i++) {
		sn = SpellPages[sbooktab][i - 1];
		if (sn != -1 && spl & SPELLBIT(sn)) {
			st = GetSBookTrans(sn, TRUE);
			SetSpellTrans(st);
			DrawSpellCel(RIGHT_PANEL_X + 11, yp, SpellITbl[sn], st, true);
			if (sn == plr[myplr]._pRSpell && st == plr[myplr]._pRSplType) {
				SetSpellTrans(RSPLTYPE_SKILL);
				DrawSpellCel(RIGHT_PANEL_X + 11, yp, SPLICONLAST, RSPLTYPE_SKILL, true);
			}
			PrintSBookStr(10, yp - 23, FALSE, spelldata[sn].sNameText, COL_WHITE);
			switch (GetSBookTrans(sn, FALSE)) {
			case RSPLTYPE_SKILL:
				strcpy(tempstr, "Skill");
				break;
			case RSPLTYPE_CHARGES:
				sprintf(tempstr, "Staff (%i charges)", plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges);
				break;
			default:
				mana = GetManaAmount(myplr, sn) >> 6;
				GetDamageAmt(sn, &min, &max);
				if (min != -1) {
					sprintf(tempstr, "Mana: %i  Dam: %i - %i", mana, min, max);
				} else {
					sprintf(tempstr, "Mana: %i   Dam: n/a", mana);
				}
				if (sn == SPL_BONESPIRIT) {
					sprintf(tempstr, "Mana: %i  Dam: 1/3 tgt hp", mana);
				}
				PrintSBookStr(10, yp - 1, FALSE, tempstr, COL_WHITE);
				lvl = plr[myplr]._pSplLvl[sn] + plr[myplr]._pISplLvlAdd;
				if (lvl < 0) {
					lvl = 0;
				}
				if (lvl == 0) {
					sprintf(tempstr, "Spell Level 0 - Unusable");
				} else {
					sprintf(tempstr, "Spell Level %i", lvl);
				}
				break;
			}
			PrintSBookStr(10, yp - 12, FALSE, tempstr, COL_WHITE);
		}
		yp += 43;
	}
}

void CheckSBook()
{
	int sn;
	char st;
	unsigned __int64 spl;

	if (MouseX >= RIGHT_PANEL + 11 && MouseX < RIGHT_PANEL + 48 && MouseY >= 18 && MouseY < 314) {
		sn = SpellPages[sbooktab][(MouseY - 18) / 43];
		spl = plr[myplr]._pMemSpells | plr[myplr]._pISpells | plr[myplr]._pAblSpells;
		if (sn != -1 && spl & SPELLBIT(sn)) {
			st = RSPLTYPE_SPELL;
			if (plr[myplr]._pISpells & SPELLBIT(sn)) {
				st = RSPLTYPE_CHARGES;
			}
			if (plr[myplr]._pAblSpells & SPELLBIT(sn)) {
				st = RSPLTYPE_SKILL;
			}
			plr[myplr]._pRSpell = sn;
			plr[myplr]._pRSplType = st;
			force_redraw = 255;
		}
	}
	if (MouseX >= RIGHT_PANEL + 7 && MouseX < RIGHT_PANEL + 311 && MouseY >= SPANEL_WIDTH && MouseY < 349) {
		sbooktab = (MouseX - (RIGHT_PANEL + 7)) / (gbIsHellfire ? 61 : 76);
	}
}

const char *get_pieces_str(int nGold)
{
	const char *result;

	result = "piece";
	if (nGold != 1)
		result = "pieces";
	return result;
}

void DrawGoldSplit(int amount)
{
	int screen_x, i;

	screen_x = 0;
	if (options_hwRendering) //Fluffy: Draw via SDL rendering
		Render_Texture_FromBottomLeft(351, 178, TEXTURE_GOLDDROPSELECTION);
	else
		CelDraw(351 + SCREEN_X, 178 + SCREEN_Y, pGBoxBuff, 1, 261);
	sprintf(tempstr, "You have %u gold", initialDropGoldValue);
	ADD_PlrStringXY(366, 87, 600, tempstr, COL_GOLD);
	sprintf(tempstr, "%s.  How many do", get_pieces_str(initialDropGoldValue));
	ADD_PlrStringXY(366, 103, 600, tempstr, COL_GOLD);
	ADD_PlrStringXY(366, 121, 600, "you want to remove?", COL_GOLD);
	if (amount > 0) {
		sprintf(tempstr, "%u", amount);
		PrintGameStr(388, 140, tempstr, COL_WHITE);
	}
	if (amount > 0) {
		for (i = 0; i < tempstr[i]; i++) {
			BYTE c = fontframe[gbFontTransTbl[(BYTE)tempstr[i]]];
			screen_x += fontkern[c] + 1;
		}
		screen_x += 452;
	} else {
		screen_x = 450;
	}
	if (options_hwRendering) //Fluffy: Render via SDL
		Render_Texture_FromBottomLeft(screen_x - BORDER_LEFT, 140, TEXTURE_SPINNINGPENTAGRAM2, PentSpn2Spin() - 1);
	else
		CelDraw(screen_x, 140 + SCREEN_Y, pSPentSpn2Cels, PentSpn2Spin(), 12);
}

void control_drop_gold(char vkey)
{
	char input[6];

	if (plr[myplr]._pHitPoints >> 6 <= 0) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
		return;
	}

	memset(input, 0, sizeof(input));
	snprintf(input, sizeof(input), "%d", dropGoldValue);
	if (vkey == DVL_VK_RETURN) {
		if (dropGoldValue > 0)
			control_remove_gold(myplr, initialDropGoldIndex);
		dropGoldFlag = FALSE;
	} else if (vkey == DVL_VK_ESCAPE) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	} else if (vkey == DVL_VK_BACK) {
		input[strlen(input) - 1] = '\0';
		dropGoldValue = atoi(input);
	} else if (vkey - '0' >= 0 && vkey - '0' <= 9) {
		if (dropGoldValue != 0 || atoi(input) <= initialDropGoldValue) {
			input[strlen(input)] = vkey;
			if (atoi(input) > initialDropGoldValue)
				return;
			if (strlen(input) > strlen(input))
				return;
		} else {
			input[0] = vkey;
		}
		dropGoldValue = atoi(input);
	}
}

void control_remove_gold(int pnum, int gold_index)
{
	int gi;

	if (gold_index <= INVITEM_INV_LAST) {
		gi = gold_index - INVITEM_INV_FIRST;
		plr[pnum].InvList[gi]._ivalue -= dropGoldValue;
		if (plr[pnum].InvList[gi]._ivalue > 0)
			SetGoldCurs(pnum, gi);
		else
			RemoveInvItem(pnum, gi);
	} else {
		gi = gold_index - INVITEM_BELT_FIRST;
		plr[pnum].SpdList[gi]._ivalue -= dropGoldValue;
		if (plr[pnum].SpdList[gi]._ivalue > 0)
			SetSpdbarGoldCurs(pnum, gi);
		else
			RemoveSpdBarItem(pnum, gi);
	}
	SetPlrHandItem(&plr[pnum].HoldItem, IDI_GOLD);
	GetGoldSeed(pnum, &plr[pnum].HoldItem);
	plr[pnum].HoldItem._ivalue = dropGoldValue;
	plr[pnum].HoldItem._iStatFlag = TRUE;
	control_set_gold_curs(pnum);
	plr[pnum]._pGold = CalculateGold(pnum);
	dropGoldValue = 0;
}

void control_set_gold_curs(int pnum)
{
	if (plr[pnum].HoldItem._ivalue >= GOLD_MEDIUM_LIMIT)
		plr[pnum].HoldItem._iCurs = ICURS_GOLD_LARGE;
	else if (plr[pnum].HoldItem._ivalue <= GOLD_SMALL_LIMIT)
		plr[pnum].HoldItem._iCurs = ICURS_GOLD_SMALL;
	else
		plr[pnum].HoldItem._iCurs = ICURS_GOLD_MEDIUM;

	NewCursor(plr[pnum].HoldItem._iCurs + CURSOR_FIRSTITEM);
}

static char *control_print_talk_msg(char *msg, int *x, int y, int color)
{
	BYTE c;
	int width;

	*x += 200 + SCREEN_X;
	y += 22 + PANEL_Y;
	width = *x;
	while (*msg) {

		c = gbFontTransTbl[(BYTE)*msg];
		c = fontframe[c];
		width += fontkern[c] + 1;
		if (width > 450 + PANEL_X)
			return msg;
		msg++;
		if (c != 0) {
			PrintChar(*x, y, c, color);
		}
		*x += fontkern[c] + 1;
	}
	return NULL;
}

void DrawTalkPan()
{
	int i, off, talk_btn, color, nCel, x;
	char *msg;

	if (!talkflag)
		return;

	DrawPanelBox(175, sgbPlrTalkTbl + 20, 294, 5, PANEL_X + 175, PANEL_Y + 4); //Renders a thin line near top of panel
	off = 0;
	for (i = 293; i > 283; off++, i--) { //Renders behind the wing of the demon next to health flask, and similar position for the mana flask
		DrawPanelBox((off >> 1) + 175, sgbPlrTalkTbl + off + 25, i, 1, (off >> 1) + PANEL_X + 175, off + PANEL_Y + 9);
	}
	DrawPanelBox(185, sgbPlrTalkTbl + 35, 274, 30, PANEL_X + 185, PANEL_Y + 19);
	DrawPanelBox(180, sgbPlrTalkTbl + 65, 284, 5, PANEL_X + 180, PANEL_Y + 49);
	for (i = 0; i < 10; i++) {
		DrawPanelBox(180, sgbPlrTalkTbl + i + 70, i + 284, 1, PANEL_X + 180, i + PANEL_Y + 54);
	}
	DrawPanelBox(170, sgbPlrTalkTbl + 80, 310, 55, PANEL_X + 170, PANEL_Y + 64);
	msg = sgszTalkMsg;
	for (i = 0; i < 39; i += 13) {
		x = 0 + PANEL_LEFT;
		msg = control_print_talk_msg(msg, &x, i, 0);
		if (!msg)
			break;
	}
	if (msg)
		*msg = '\0';
	if (options_hwRendering) //Fluffy: Render via SDL
		Render_Texture_FromBottomLeft(x - BORDER_LEFT, i + 22 + PANEL_TOP, TEXTURE_SPINNINGPENTAGRAM2, PentSpn2Spin() - 1);
	else
		CelDraw(x, i + 22 + PANEL_Y, pSPentSpn2Cels, PentSpn2Spin(), 12);
	talk_btn = 0;
	for (i = 0; i < 4; i++) {
		if (i == myplr)
			continue;
		if (whisper[i]) {
			color = COL_GOLD;
			if (talkbtndown[talk_btn]) {
				if (talk_btn != 0)
					nCel = 4;
				else
					nCel = 3;
				if (options_hwRendering) //Fluffy: Render via SDL
					Render_Texture_FromBottomLeft(172 + PANEL_LEFT, 84 + 18 * talk_btn + PANEL_TOP, TEXTURE_HUDPANEL_TALKBUTTONS, nCel - 1);
				else
					CelDraw(172 + PANEL_X, 84 + 18 * talk_btn + PANEL_Y, pTalkBtns, nCel, 61);
			}
		} else {
			color = COL_RED;
			if (talk_btn != 0)
				nCel = 2;
			else
				nCel = 1;
			if (talkbtndown[talk_btn])
				nCel += 4;
			if (options_hwRendering) //Fluffy: Render via SDL
				Render_Texture_FromBottomLeft(172 + PANEL_LEFT, 84 + 18 * talk_btn + PANEL_TOP, TEXTURE_HUDPANEL_TALKBUTTONS, nCel - 1);
			else
				CelDraw(172 + PANEL_X, 84 + 18 * talk_btn + PANEL_Y, pTalkBtns, nCel, 61);
		}
		if (plr[i].plractive) {
			x = 46 + PANEL_LEFT;
			control_print_talk_msg(plr[i]._pName, &x, 60 + talk_btn * 18, color);
		}

		talk_btn++;
	}
}

BOOL control_check_talk_btn()
{
	int i;

	if (!talkflag)
		return FALSE;

	if (MouseX < 172 + PANEL_LEFT)
		return FALSE;
	if (MouseY < 69 + PANEL_TOP)
		return FALSE;
	if (MouseX > 233 + PANEL_LEFT)
		return FALSE;
	if (MouseY > 123 + PANEL_TOP)
		return FALSE;

	for (i = 0; i < sizeof(talkbtndown) / sizeof(talkbtndown[0]); i++) {
		talkbtndown[i] = FALSE;
	}

	talkbtndown[(MouseY - (69 + PANEL_TOP)) / 18] = TRUE;

	return TRUE;
}

void control_release_talk_btn()
{
	int i, p, off;

	if (talkflag) {
		for (i = 0; i < sizeof(talkbtndown) / sizeof(talkbtndown[0]); i++)
			talkbtndown[i] = FALSE;
		if (MouseX >= 172 + PANEL_LEFT && MouseY >= 69 + PANEL_TOP && MouseX <= 233 + PANEL_LEFT && MouseY <= 123 + PANEL_TOP) {
			off = (MouseY - (69 + PANEL_TOP)) / 18;

			for (p = 0; p < MAX_PLRS && off != -1; p++) {
				if (p != myplr)
					off--;
			}
			if (p <= MAX_PLRS)
				whisper[p - 1] = !whisper[p - 1];
		}
	}
}

void control_reset_talk_msg(char *msg)
{
	int i, pmask;
	pmask = 0;

	for (i = 0; i < MAX_PLRS; i++) {
		if (whisper[i])
			pmask |= 1 << i;
	}
	NetSendCmdString(pmask, sgszTalkMsg);
}

void control_type_message()
{
	int i;

	if (gbMaxPlayers == 1) {
		return;
	}

	talkflag = TRUE;
	sgszTalkMsg[0] = '\0';
	for (i = 0; i < 3; i++) {
		talkbtndown[i] = FALSE;
	}
	sgbPlrTalkTbl = PANEL_HEIGHT + 16;
	force_redraw = 255;
	sgbTalkSavePos = sgbNextTalkSave;
}

void control_reset_talk()
{
	talkflag = FALSE;
	sgbPlrTalkTbl = 0;
	force_redraw = 255;
}

static void control_press_enter()
{
	int i;
	BYTE talk_save;

	if (sgszTalkMsg[0] != 0) {
		control_reset_talk_msg(sgszTalkMsg);
		for (i = 0; i < 8; i++) {
			if (!strcmp(sgszTalkSave[i], sgszTalkMsg))
				break;
		}
		if (i >= 8) {
			strcpy(sgszTalkSave[sgbNextTalkSave], sgszTalkMsg);
			sgbNextTalkSave++;
			sgbNextTalkSave &= 7;
		} else {
			talk_save = sgbNextTalkSave - 1;
			talk_save &= 7;
			if (i != talk_save) {
				strcpy(sgszTalkSave[i], sgszTalkSave[talk_save]);
				strcpy(sgszTalkSave[talk_save], sgszTalkMsg);
			}
		}
		sgszTalkMsg[0] = '\0';
		sgbTalkSavePos = sgbNextTalkSave;
	}
	control_reset_talk();
}

BOOL control_talk_last_key(int vkey)
{
	int result;

	if (gbMaxPlayers == 1)
		return FALSE;

	if (!talkflag)
		return FALSE;

	if ((DWORD)vkey < DVL_VK_SPACE)
		return FALSE;

	result = strlen(sgszTalkMsg);
	if (result < 78) {
		sgszTalkMsg[result] = vkey;
		sgszTalkMsg[result + 1] = '\0';
	}
	return TRUE;
}

static void control_up_down(int v)
{
	int i;

	for (i = 0; i < 8; i++) {
		sgbTalkSavePos = (v + sgbTalkSavePos) & 7;
		if (sgszTalkSave[sgbTalkSavePos][0]) {
			strcpy(sgszTalkMsg, sgszTalkSave[sgbTalkSavePos]);
			return;
		}
	}
}

BOOL control_presskeys(int vkey)
{
	int len;
	BOOL ret;

	if (gbMaxPlayers != 1) {
		if (!talkflag) {
			ret = FALSE;
		} else {
			if (vkey == DVL_VK_SPACE) {
			} else if (vkey == DVL_VK_ESCAPE) {
				control_reset_talk();
			} else if (vkey == DVL_VK_RETURN) {
				control_press_enter();
			} else if (vkey == DVL_VK_BACK) {
				len = strlen(sgszTalkMsg);
				if (len > 0)
					sgszTalkMsg[len - 1] = '\0';
			} else if (vkey == DVL_VK_DOWN) {
				control_up_down(1);
			} else if (vkey == DVL_VK_UP) {
				control_up_down(-1);
			} else {
				return FALSE;
			}
			ret = TRUE;
		}
	} else {
		ret = FALSE;
	}
	return ret;
}

DEVILUTION_END_NAMESPACE
