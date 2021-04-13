/**
 * @file plrmsg.cpp
 *
 * Implementation of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#include "all.h"
#include "textures/textures.h" //Fluffy: For rendering 32-bit textures
#include "render/sdl-render.h" //Fluffy: For rendering 32-bit textures
#include "render/lightmap.h" //Fluffy: For lightmap generation
#include "options.h" //Fluffy
#include "ui/hotbar.h" //Fluffy

DEVILUTION_BEGIN_NAMESPACE

/**
 * Specifies the current light entry.
 */
int light_table_index;
DWORD sgdwCursWdtOld;
DWORD sgdwCursX;
DWORD sgdwCursY;
/**
 * Lower bound of back buffer.
 */
DWORD sgdwCursHgt;

/**
 * Specifies the current MIN block of the level CEL file, as used during rendering of the level tiles.
 *
 * frameNum  := block & 0x0FFF
 * frameType := block & 0x7000 >> 12
 */
DWORD level_cel_block;
DWORD sgdwCursXOld;
DWORD sgdwCursYOld;
BOOLEAN AutoMapShowItems;
/**
 * Specifies the type of arches to render.
 */
char arch_draw_type;
/**
 * Specifies whether transparency is active for the current CEL file being decoded.
 */
int cel_transparency_active;
/**
 * Specifies whether foliage (tile has extra content that overlaps previous tile) being rendered.
 */
int cel_foliage_active = false;
/**
 * Specifies the current dungeon piece ID of the level, as used during rendering of the level tiles.
 */
int level_piece_id;
DWORD sgdwCursWdt;
void (*DrawPlrProc)(int, int, int, int, int, BYTE *, int, int, int, int);
BYTE sgSaveBack[8192];
DWORD sgdwCursHgtOld;

bool dRendered[MAXDUNX][MAXDUNY];

int frames;
BOOL frameflag;
int frameend;
int framerate;
unsigned long long framestart; //Fluffy: Gave this higher precision ( //Fluffy TODO merge: Should be deleted?)

/* data */

const char *const szMonModeAssert[] = {
	"standing",
	"walking (1)",
	"walking (2)",
	"walking (3)",
	"attacking",
	"getting hit",
	"dying",
	"attacking (special)",
	"fading in",
	"fading out",
	"attacking (ranged)",
	"standing (special)",
	"attacking (special ranged)",
	"delaying",
	"charging",
	"stoned",
	"healing",
	"talking"
};

const char *const szPlrModeAssert[] = {
	"standing",
	"walking (1)",
	"walking (2)",
	"walking (3)",
	"attacking (melee)",
	"attacking (ranged)",
	"blocking",
	"getting hit",
	"dying",
	"casting a spell",
	"changing levels",
	"quitting"
};

/**
 * @brief Clear cursor state
 */
void ClearCursor() // CODE_FIX: this was supposed to be in cursor.cpp
{
	sgdwCursWdt = 0;
	sgdwCursWdtOld = 0;
}

static void BlitCursor(BYTE *dst, int dst_pitch, BYTE *src, int src_pitch)
{
	const int h = std::min(sgdwCursY + 1, sgdwCursHgt);
	for (int i = 0; i < h; ++i, src += src_pitch, dst += dst_pitch) {
		memcpy(dst, src, sgdwCursWdt);
	}
}

/**
 * @brief Remove the cursor from the buffer
 */
static void scrollrt_draw_cursor_back_buffer(CelOutputBuffer out)
{
	if (sgdwCursWdt == 0) {
		return;
	}

	BlitCursor(out.at(sgdwCursX, sgdwCursY), out.pitch(), sgSaveBack, sgdwCursWdt);

	sgdwCursXOld = sgdwCursX;
	sgdwCursYOld = sgdwCursY;
	sgdwCursWdtOld = sgdwCursWdt;
	sgdwCursHgtOld = sgdwCursHgt;
	sgdwCursWdt = 0;
}

/**
 * @brief Draw the cursor on the given buffer
 */
static void scrollrt_draw_cursor_item(CelOutputBuffer out)
{
	int i, mx, my;
	BYTE col = 0; //Fluffy

	assert(!sgdwCursWdt);

	if (pcurs <= CURSOR_NONE || cursW == 0 || cursH == 0) {
		return;
	}

	if (sgbControllerActive && !IsMovingMouseCursorWithController() && pcurs != CURSOR_TELEPORT && !invflag && (!chrflag || plr[myplr]._pStatPts <= 0)) {
		return;
	}

	mx = MouseX - 1;
	if (mx < 0 - cursW - 1) {
		return;
	} else if (mx > gnScreenWidth - 1) {
		return;
	}
	my = MouseY - 1;
	if (my < 0 - cursH - 1) {
		return;
	} else if (my > gnScreenHeight - 1) {
		return;
	}

	if (!options_hwUIRendering) { //Fluffy: Only do backup of cursor if we're doing 8-bit rendering
		sgdwCursX = mx;
		sgdwCursWdt = sgdwCursX + cursW + 1;
		if (sgdwCursWdt > gnScreenWidth - 1) {
			sgdwCursWdt = gnScreenWidth - 1;
		}
		sgdwCursX &= ~3;
		sgdwCursWdt |= 3;
		sgdwCursWdt -= sgdwCursX;
		sgdwCursWdt++;

		sgdwCursY = my;
		sgdwCursHgt = sgdwCursY + cursH + 1;
		if (sgdwCursHgt > gnScreenHeight - 1) {
			sgdwCursHgt = gnScreenHeight - 1;
		}
		sgdwCursHgt -= sgdwCursY;
		sgdwCursHgt++;

		BlitCursor(sgSaveBack, sgdwCursWdt, out.at(sgdwCursX, sgdwCursY), out.pitch());
	}

	/*mx++;
	my++;

	out = out.subregion(0, 0, out.w() - 2, out.h());
	if (pcurs >= CURSOR_FIRSTITEM) {
		col = PAL16_YELLOW + 5;
		if (plr[myplr].HoldItem._iMagical != 0) {
			col = PAL16_BLUE + 5;
		}
		if (!plr[myplr].HoldItem._iStatFlag) {
			col = PAL16_RED + 5;
		}
		if (pcurs <= 179) {
			CelBlitOutlineTo(out, col, mx, my + cursH - 1, pCursCels, pcurs, cursW, false);
			if (col != PAL16_RED + 5) {
				CelClippedDrawSafeTo(out, mx, my + cursH - 1, pCursCels, pcurs, cursW);
			} else {
				CelDrawLightRedSafeTo(out, mx, my + cursH - 1, pCursCels, pcurs, cursW, 1);
			}
		} else {
			CelBlitOutlineTo(out, col, mx, my + cursH - 1, pCursCels2, pcurs - 179, cursW, false);
			if (col != PAL16_RED + 5) {
				CelClippedDrawSafeTo(out, mx, my + cursH - 1, pCursCels2, pcurs - 179, cursW);
			} else {
				CelDrawLightRedSafeTo(out, mx, my + cursH - 1, pCursCels2, pcurs - 179, cursW, 0);
			}
		}
	} else {
		CelClippedDrawSafeTo(out, mx, my + cursH - 1, pCursCels, pcurs, cursW);
	}*/

	mx++;
	my++;

	if (pcurs >= CURSOR_FIRSTITEM) {
		col = ICOL_WHITE;
		if (plr[myplr].HoldItem._iMagical != 0) {
			col = ICOL_BLUE;
		}
		if (!plr[myplr].HoldItem._iStatFlag) {
			col = ICOL_RED;
		}
	}
	DrawCursorItemWrapper(out, mx, my + cursH - 1, pcurs, cursW, 1, col == ICOL_RED, pcurs >= CURSOR_FIRSTITEM, col); //Fluffy
}

/**
 * @brief Render a missile sprite
 * @param out Output buffer
 * @param m Pointer to MissileStruct struct
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pre Is the sprite in the background
 */
void DrawMissilePrivate(CelOutputBuffer out, MissileStruct *m, int sx, int sy, BOOL pre)
{
	if (m->_miPreFlag != pre || !m->_miDrawFlag)
		return;

	BYTE *pCelBuff = m->_miAnimData;
	if (pCelBuff == NULL) {
		SDL_Log("Draw Missile 2 type %d: NULL Cel Buffer", m->_mitype);
		return;
	}

	int mx = sx + m->_mixoff - m->_miAnimWidth2;
	int my = sy + m->_miyoff;

	if (options_hwIngameRendering) { //Fluffy: Render missile via SDL

		//Figure out what texture to use for this missile
		int textureNum = -1;
		int frameNum = m->_miAnimFrame - 1;
		if (m->_mitype == MIS_RHINO) { //Some missiles will actually use a monster's texture (ie, Rhino's charge move)
			CMonster *mon = monster[m->_misource].MType;
			int monType = monster[m->_misource]._mMTidx;
			int anim;
			if (mon->mtype >= MT_HORNED && mon->mtype <= MT_OBLORD) {
				anim = MA_SPECIAL;
			} else {
				if (mon->mtype >= MT_NSNAKE && mon->mtype <= MT_GSNAKE)
					anim = MA_ATTACK;
				else
					anim = MA_WALK;
			}
			textureNum = TEXTURE_MONSTERS + (monType * MA_NUM) + anim;

			//Cycle through directions to find matching animation data
			for (int j = 0; j < 8; j++)
				if (m->_miAnimData == mon->Anims[anim].Data[j]) {
					frameNum += j * m->_miAnimLen; //TODO: Are we using the right animation length here?
					goto foundTexture;
				}
		}
		for (int j = 0; j < 16; j++) { //Missile texture
			if (m->_miAnimData == misfiledata[m->_miAnimType].mAnimData[j]) {
				textureNum = TEXTURE_MISSILES + (m->_miAnimType * 16) + j;
				goto foundTexture;
			}
		}
		assert(textureNum != -1);
	foundTexture:

		//Render missile
		int brightness;
		if (m->_miLightFlag)
			brightness = 255;
		else
			brightness = Render_IndexLightToBrightness();
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
		Render_Texture_FromBottom(mx, my, textureNum, frameNum);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
		//TODO: Handle m->_miUniqTrans
		return;
	}

	int nCel = m->_miAnimFrame;
	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		SDL_Log("Draw Missile 2: frame %d of %d, missile type==%d", nCel, frames, m->_mitype);
		return;
	}
	if (m->_miUniqTrans)
		Cl2DrawLightTbl(out, mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth, m->_miUniqTrans + 3);
	else if (m->_miLightFlag)
		Cl2DrawLight(out, mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth);
	else
		Cl2Draw(out, mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth);
}

/**
 * @brief Render a missile sprites for a given tile
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pre Is the sprite in the background
 */
void DrawMissile(CelOutputBuffer out, int x, int y, int sx, int sy, BOOL pre)
{
	int i;
	MissileStruct *m;

	if (!(dFlags[x][y] & BFLAG_MISSILE))
		return;

	if (dMissile[x][y] != -1) {
		m = &missile[dMissile[x][y] - 1];
		DrawMissilePrivate(out, m, sx, sy, pre);
		return;
	}

	for (i = 0; i < nummissiles; i++) {
		assert(missileactive[i] < MAXMISSILES);
		m = &missile[missileactive[i]];
		if (m->_mix != x || m->_miy != y)
			continue;
		DrawMissilePrivate(out, m, sx, sy, pre);
	}
}

/**
 * @brief Render a monster sprite
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param mx Output buffer coordinate
 * @param my Output buffer coordinate
 * @param m Id of monster
 */
static void DrawMonster(CelOutputBuffer out, int x, int y, int mx, int my, int m)
{
	if (m < 0 || m >= MAXMONSTERS) {
		SDL_Log("Draw Monster: tried to draw illegal monster %d", m);
		return;
	}

	BYTE *pCelBuff = monster[m]._mAnimData;
	if (pCelBuff == NULL) {
		SDL_Log("Draw Monster \"%s\": NULL Cel Buffer", monster[m].mName);
		return;
	}

	int nCel = monster[m]._mAnimFrame;
	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		const char *szMode = "unknown action";
		if (monster[m]._mmode <= 17)
			szMode = szMonModeAssert[monster[m]._mmode];
		SDL_Log(
		    "Draw Monster \"%s\" %s: facing %d, frame %d of %d",
		    monster[m].mName,
		    szMode,
		    monster[m]._mdir,
		    nCel,
		    frames);
		return;
	}

	if (!(dFlags[x][y] & BFLAG_LIT)) {
		Cl2DrawLightTbl(out, mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width, 1);
		return;
	}

	char trans = 0;
	if (monster[m]._uniqtype)
		trans = monster[m]._uniqtrans + 4;
	if (monster[m]._mmode == MM_STONE)
		trans = 2;
	if (plr[myplr]._pInfraFlag && light_table_index > 8)
		trans = 1;
	if (trans)
		Cl2DrawLightTbl(out, mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width, trans);
	else
		Cl2DrawLight(out, mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width);
}

/**
 * @brief Helper for rendering player a Mana Shield
 * @param out Output buffer
 * @param pnum Player id
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param lighting Should lighting be applied
 */
static void DrawManaShield(CelOutputBuffer out, int pnum, int x, int y, bool lighting)
{
	if (!plr[pnum].pManaShield)
		return;

	x += plr[pnum]._pAnimWidth2 - misfiledata[MFILE_MANASHLD].mAnimWidth2[0];

	int width = misfiledata[MFILE_MANASHLD].mAnimWidth[0];
	BYTE *pCelBuff = misfiledata[MFILE_MANASHLD].mAnimData[0];

	if (pnum == myplr) {
		Cl2Draw(out, x, y, pCelBuff, 1, width);
		return;
	}

	if (lighting) {
		Cl2DrawLightTbl(out, x, y, pCelBuff, 1, width, 1);
		return;
	}

	Cl2DrawLight(out, x, y, pCelBuff, 1, width);
}

static void DrawPlayer_SDL(int p, int x, int y, int px, int py) //Fluffy
{
	if (!(dFlags[x][y] & BFLAG_LIT || plr[myplr]._pInfraFlag || !setlevel && currlevel == 0))
		return;

	PlayerStruct *pPlayer = &plr[p];
	//Figure out what animation the player is in
	int textureNum = TEXTURE_PLAYERS + (p * PLAYERANIM_NUM), brightness, facing = 0;
	if (p == myplr)
		brightness = 255;
	else
		brightness = Render_IndexLightToBrightness();
	for (int i = 0; i < 8; i++) { //TODO: We should probably use a way better way to figure out what animation we're in. A better system would be for the player struct to store current animation and facing, and have both normal and SDL rendering code reference that rather than using player->_pAnimData
		facing = i;
		if (pPlayer->_pAnimData == pPlayer->_pNAnim[i])
			textureNum += PLAYERANIM_STAND;
		else if (pPlayer->_pAnimData == pPlayer->_pWAnim[i])
			textureNum += PLAYERANIM_WALK;
		else if (pPlayer->_pAnimData == pPlayer->_pAAnim[i])
			textureNum += PLAYERANIM_ATTACK;
		else if (pPlayer->_pAnimData == pPlayer->_pLAnim[i])
			textureNum += PLAYERANIM_SPELL_LIGHTNING;
		else if (pPlayer->_pAnimData == pPlayer->_pFAnim[i])
			textureNum += PLAYERANIM_SPELL_FIRE;
		else if (pPlayer->_pAnimData == pPlayer->_pTAnim[i])
			textureNum += PLAYERANIM_SPELL_GENERIC;
		else if (pPlayer->_pAnimData == pPlayer->_pHAnim[i])
			textureNum += PLAYERANIM_GETHIT;
		else if (pPlayer->_pAnimData == pPlayer->_pDAnim[i])
			textureNum += PLAYERANIM_DEATH;
		else if (pPlayer->_pAnimData == pPlayer->_pBAnim[i])
			textureNum += PLAYERANIM_BLOCK;
		else if (pPlayer->_pAnimData == pPlayer->_pNAnim_c[i])
			textureNum += PLAYERANIM_STAND_CASUAL;
		else if (pPlayer->_pAnimData == pPlayer->_pWAnim_c[i])
			textureNum += PLAYERANIM_WALK_CASUAL;
		else
			continue;
		break;
	}
	int frameNum = (pPlayer->_pAnimFrame - 1) + (facing * pPlayer->_pAnimLen);
	if (brightness < 255)
		SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
	Render_Texture_FromBottom(px, py, textureNum, frameNum);
	if (brightness < 255)
		SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);

	//TODO: Render outline (if player is selected) and Mana Shield if it's on
}


/**
 * @brief Render a player sprite
 * @param out Output buffer
 * @param pnum Player id
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param px Output buffer coordinate
 * @param py Output buffer coordinate
 * @param pCelBuff sprite buffer
 * @param nCel frame
 * @param nWidth width
 */
static void DrawPlayer(CelOutputBuffer out, int pnum, int x, int y, int px, int py, BYTE *pCelBuff, int nCel, int nWidth)
{
	if ((dFlags[x][y] & BFLAG_LIT) == 0 && !plr[myplr]._pInfraFlag && leveltype != DTYPE_TOWN) {
		return;
	}

	if (pCelBuff == NULL) {
		SDL_Log("Drawing player %d \"%s\": NULL Cel Buffer", pnum, plr[pnum]._pName);
		return;
	}

	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		const char *szMode = "unknown action";
		if (plr[pnum]._pmode <= PM_QUIT)
			szMode = szPlrModeAssert[plr[pnum]._pmode];
		SDL_Log(
		    "Drawing player %d \"%s\" %s: facing %d, frame %d of %d",
		    pnum,
		    plr[pnum]._pName,
		    szMode,
		    plr[pnum]._pdir,
		    nCel,
		    frames);
		return;
	}

	if (pnum == pcursplr)
		Cl2DrawOutline(out, 165, px, py, pCelBuff, nCel, nWidth);

	if (pnum == myplr) {
		Cl2Draw(out, px, py, pCelBuff, nCel, nWidth);
		DrawManaShield(out, pnum, px, py, true);
		return;
	}

	if (!(dFlags[x][y] & BFLAG_LIT) || (plr[myplr]._pInfraFlag && light_table_index > 8)) {
		Cl2DrawLightTbl(out, px, py, pCelBuff, nCel, nWidth, 1);
		DrawManaShield(out, pnum, px, py, true);
		return;
	}

	int l = light_table_index;
	if (light_table_index < 5)
		light_table_index = 0;
	else
		light_table_index -= 5;

	Cl2DrawLight(out, px, py, pCelBuff, nCel, nWidth);
	DrawManaShield(out, pnum, px, py, false);

	light_table_index = l;
}

/**
 * @brief Render a player sprite
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 */
void DrawDeadPlayer(CelOutputBuffer out, int x, int y, int sx, int sy)
{
	int i, px, py;
	PlayerStruct *p;

	dFlags[x][y] &= ~BFLAG_DEAD_PLAYER;

	for (i = 0; i < MAX_PLRS; i++) {
		p = &plr[i];
		if (p->plractive && p->_pHitPoints == 0 && p->plrlevel == (BYTE)currlevel && p->_px == x && p->_py == y) {
			dFlags[x][y] |= BFLAG_DEAD_PLAYER;
			px = sx + p->_pxoff - p->_pAnimWidth2;
			py = sy + p->_pyoff;
			DrawPlayer(out, i, x, y, px, py, p->_pAnimData, p->_pAnimFrame, p->_pAnimWidth);
		}
	}
}

/**
 * @brief Render an object sprite
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param ox Output buffer coordinate
 * @param oy Output buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawObject(CelOutputBuffer out, int x, int y, int ox, int oy, BOOL pre)
{
	if (dObject[x][y] == 0 || light_table_index >= lightmax)
		return;

	int sx, sy;
	char bv;
	if (dObject[x][y] > 0) {
		bv = dObject[x][y] - 1;
		if (object[bv]._oPreFlag != pre)
			return;
		sx = ox - object[bv]._oAnimWidth2;
		sy = oy;
	} else {
		bv = -(dObject[x][y] + 1);
		if (object[bv]._oPreFlag != pre)
			return;
		int xx = object[bv]._ox - x;
		int yy = object[bv]._oy - y;
		sx = (xx << 5) + ox - object[bv]._oAnimWidth2 - (yy << 5);
		sy = oy + (yy << 4) + (xx << 4);
	}

	assert(bv >= 0 && bv < MAXOBJECTS);

	BYTE *pCelBuff = object[bv]._oAnimData;
	if (pCelBuff == NULL) {
		SDL_Log("Draw Object type %d: NULL Cel Buffer", object[bv]._otype);
		return;
	}

	int nCel = object[bv]._oAnimFrame;
	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		SDL_Log("Draw Object: frame %d of %d, object type==%d", nCel, frames, object[bv]._otype);
		return;
	}

	if (options_hwIngameRendering) { //Fluffy: Render object via SDL
		int brightness;
		if (!object[bv]._oLight)
			brightness = 255;
		else
			brightness = Render_IndexLightToBrightness();
		int objectType = object[bv]._otype;
		int textureNum = TEXTURE_OBJECTS + AllObjects[objectType].ofindex;
		int frameNum = nCel - 1;
		if (!object[bv]._oLight)
			SDL_SetTextureBlendMode(textures[textureNum].frames[frameNum].frame, SDL_BLENDMODE_ADD);
		if (bv == pcursobj)
			Render_TextureOutline_FromBottom(sx, sy, 221, 196, 126, TEXTURE_OBJECTS + AllObjects[objectType].ofindex, nCel - 1);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
		Render_Texture_FromBottom(sx, sy, TEXTURE_OBJECTS + AllObjects[objectType].ofindex, nCel - 1);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
		if (!object[bv]._oLight)
			SDL_SetTextureBlendMode(textures[textureNum].frames[frameNum].frame, SDL_BLENDMODE_BLEND);
		return;
	}

	if (bv == pcursobj) {
		CelBlitOutlineTo(out, 194, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
		if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
			CelDrawToImportant(194, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth, true);
	}
	if (object[bv]._oLight) {
		CelClippedDrawLightTo(out, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
	} else {
		CelClippedDrawTo(out, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
	}
	if (object[bv]._oSelFlag >= 1) {
		if (sgOptions.Graphics.bOpaqueWallsWithBlobs) //Fluffy
			CelDrawToImportant_Ellipse(sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
		if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
			CelDrawToImportant(pLightTbl[(256 * light_table_index) + 198], sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth, false);
	}
}

static void scrollrt_draw_dungeon(CelOutputBuffer, int, int, int, int);

/**
 * @brief Render a cell
 * @param out Target buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param importantObjectNearby Whether or not a player, monster, item, etc is nearby (Fluffy)
 */
static void drawCell(CelOutputBuffer out, int x, int y, int sx, int sy, bool importantObjectNearby)
{
	MICROS *pMap = &dpiece_defs_map_2[x][y];
	level_piece_id = dPiece[x][y];

	//Fluffy
	int lightx = -1;
	int lighty = -1;
	int lightType = LIGHTING_SUBTILE_NONE;
	if (options_hwIngameRendering && options_lightmapping) {
		if (lightInfo_subTiles && level_piece_id < lightInfo_subTilesSize)
			lightType = lightInfo_subTiles[level_piece_id - 1];

		if (lightType == LIGHTING_SUBTILE_UNIFORM) {
			lightx = lightmap_lightx;
			lighty = lightmap_lighty;
		}
	}

	if (sgOptions.Graphics.bOpaqueWallsUnlessObscuring && !importantObjectNearby && !sgOptions.Graphics.bOpaqueWallsWithBlobs && !sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy: Make this opaque if there's nothing important nearby
		cel_transparency_active = 0;
	else {
		if (sgOptions.Graphics.bOpaqueWallsWithBlobs || sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
			cel_transparency_active = 1;
		else
			cel_transparency_active = (BYTE)(nTransTable[level_piece_id] & TransList[dTransVal[x][y]]);
	}

	//Fluffy: Render cell as one whole dungeon piece
	if (1 && nSolidTable[level_piece_id] && options_hwIngameRendering && options_lightmapping) {
		level_piece_id--;
		SDL_Texture *tex = textures[TEXTURE_DUNGEONTILES_DUNGEONPIECES].frames[0].frame;
		textureFrame_s *textureFrame = &textures[TEXTURE_DUNGEONTILES_DUNGEONPIECES].frames[level_piece_id];
		int brightness;

		if (1 && (lightType == LIGHTING_SUBTILE_DIAGONALFORWARD || lightType == LIGHTING_SUBTILE_DIAGONALBACKWARD || lightType == LIGHTING_SUBTILE_MIXEDFOREGROUND || lightType == LIGHTING_SUBTILE_MIXEDBACKGROUND)) {

			if (1) { //Render target render
				//Switch to the intermediate tile render target
				SDL_SetRenderTarget(renderer, textures[TEXTURE_TILE_INTERMEDIATE_PIECE].frames[0].frame);
				SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE); //Switch to "none" blend mode so we overwrite everything in the render target
				Render_Texture(0, 0, TEXTURE_DUNGEONTILES_DUNGEONPIECES, level_piece_id);
				SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND); //Revert blend mode for the texture

				if (lightType == LIGHTING_SUBTILE_DIAGONALFORWARD || lightType == LIGHTING_SUBTILE_MIXEDBACKGROUND) { //Start bottomleft
					lightx = lightmap_lightx - (TILE_WIDTH / 2);
					lighty = lightmap_lighty + (TILE_HEIGHT / 2);
				} else if (lightType == LIGHTING_SUBTILE_DIAGONALBACKWARD || lightType == LIGHTING_SUBTILE_MIXEDFOREGROUND) { //Start topleft
					lightx = lightmap_lightx - (TILE_WIDTH / 2);
					lighty = lightmap_lighty - (TILE_HEIGHT / 2);
				}

				//TODO: We need to handle cropX1/cropX2 if it's ever non-0 for dungeon pieces
				SDL_Rect dstRect, srcRect;
				dstRect.x = 0;
				dstRect.y = textureFrame->cropY1;
				srcRect.w = dstRect.w = 2;
				dstRect.h = textureFrame->height - (textureFrame->cropY1 + textureFrame->cropY2);
				srcRect.x = lightx;
				srcRect.y = lighty;
				srcRect.h = 1;

				SDL_Texture *texLight = textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame;
				for (int i = 0; i < textureFrame->width; i += 2) {
					SDL_RenderCopy(renderer, texLight, &srcRect, &dstRect);
					dstRect.x += 2;
					srcRect.x += 2;
					if ((lightType == LIGHTING_SUBTILE_DIAGONALFORWARD)
					    || (lightType == LIGHTING_SUBTILE_MIXEDBACKGROUND && i < textureFrame->width / 2)
					    || (lightType == LIGHTING_SUBTILE_MIXEDFOREGROUND && i >= textureFrame->width / 2)) {
						srcRect.y -= 1;
					} else {
						srcRect.y += 1;
					}
				}

				if (0) { //Go through nearby tiles and render any important entity as a silhouette
					//TODO: Finish this code. Right now it's only a basic stress test
					//Render player as solid colour
					Render_Texture(0, 0, TEXTURE_PLAYERS, 0);

					//Render alpha from wall texture
					SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ZERO, SDL_BLENDOPERATION_ADD); // (dstColor = dstColor; dstAlpha = srcAlpha)
					SDL_SetTextureBlendMode(tex, blendMode);
					Render_Texture(0, 0, TEXTURE_DUNGEONTILES_DUNGEONPIECES, level_piece_id);
					SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
				}

				//Switch render target back to intermediate texture and render final result
				SDL_SetRenderTarget(renderer, texture_intermediate);
				dstRect.x = sx;
				dstRect.y = (sy - (textures[TEXTURE_DUNGEONTILES_DUNGEONPIECES].frames[level_piece_id].height - 1)) + textureFrame->cropY1;
				srcRect.w = dstRect.w = textureFrame->width;
				srcRect.h = dstRect.h = textureFrame->height - (textureFrame->cropY1 + textureFrame->cropY2);
				srcRect.x = 0;
				srcRect.y = textureFrame->cropY1;
				SDL_RenderCopy(renderer, textures[TEXTURE_TILE_INTERMEDIATE_PIECE].frames[0].frame, &srcRect, &dstRect);
			} else {                                                                                                  //Render using light info from lightmap in RAM
				if (lightType == LIGHTING_SUBTILE_DIAGONALFORWARD || lightType == LIGHTING_SUBTILE_MIXEDBACKGROUND) { //Start bottomleft
					lightx = lightmap_lightx - (TILE_WIDTH / 2);
					lighty = lightmap_lighty + (TILE_HEIGHT / 2);
				} else if (lightType == LIGHTING_SUBTILE_DIAGONALBACKWARD || lightType == LIGHTING_SUBTILE_MIXEDFOREGROUND) { //Start topleft
					lightx = lightmap_lightx - (TILE_WIDTH / 2);
					lighty = lightmap_lighty - (TILE_HEIGHT / 2);
				}

				//TODO: We need to handle cropX1/cropX2 if it's ever non-0 for dungeon pieces
				SDL_Rect dstRect, srcRect;
				dstRect.x = sx;
				dstRect.y = (sy - (textureFrame->height - 1)) + textureFrame->cropY1;
				srcRect.w = dstRect.w = 1;
				srcRect.h = dstRect.h = textureFrame->height - (textureFrame->cropY1 + textureFrame->cropY2);
				srcRect.x = textureFrame->offsetX;
				srcRect.y = textureFrame->offsetY + textureFrame->cropY1;

				for (int i = 0; i < textureFrame->width; i++) {
					brightness = Lightmap_ReturnBrightness(lightx, lighty);
					SDL_SetTextureColorMod(tex, brightness, brightness, brightness);
					SDL_RenderCopy(renderer, tex, &srcRect, &dstRect);
					dstRect.x += 1;
					srcRect.x += 1;
					lightx += 1;
					if (i > 0 && i % 2 == 0) {
						if ((lightType == LIGHTING_SUBTILE_DIAGONALFORWARD)
						    || (lightType == LIGHTING_SUBTILE_MIXEDBACKGROUND && i < textureFrame->width / 2)
						    || (lightType == LIGHTING_SUBTILE_MIXEDFOREGROUND && i >= textureFrame->width / 2)) {
							lighty -= 1;
						} else {
							lighty += 1;
						}
					}
				}
				SDL_SetTextureColorMod(tex, 255, 255, 255);
			}
		} else if (1 || lightType == LIGHTING_SUBTILE_LIGHTMAP) {
			//Switch to the intermediate tile render target
			SDL_SetRenderTarget(renderer, textures[TEXTURE_TILE_INTERMEDIATE_PIECE].frames[0].frame);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE); //Switch to "none" blend mode so we overwrite everything in the render target
			Render_Texture(0, 0, TEXTURE_DUNGEONTILES_DUNGEONPIECES, level_piece_id);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND); //Revert blend mode for the texture

			//TODO: We need to handle cropX1/cropX2 if it's ever non-0 for dungeon pieces
			//TODO: Handle Y cropping
			int x = sx;
			int y = sy - (textureFrame->height - 1);
			y += textureFrame->width;
			if (x < 0)
				x = 0;
			if (y < 0)
				y = 0;
			Render_Texture_Crop(0, 0, TEXTURE_LIGHT_FRAMEBUFFER, x, y, x + textureFrame->width, y + textureFrame->height);

			//Switch render target back to intermediate texture and render final result
			SDL_SetRenderTarget(renderer, texture_intermediate);
			SDL_Rect srcRect, dstRect;
			dstRect.x = sx;
			dstRect.y = (sy - (textureFrame->height - 1)) + textureFrame->cropY1;
			srcRect.w = dstRect.w = textureFrame->width;
			srcRect.h = dstRect.h = textureFrame->height - (textureFrame->cropY1 + textureFrame->cropY2);
			srcRect.x = 0;
			srcRect.y = textureFrame->cropY1;
			SDL_RenderCopy(renderer, textures[TEXTURE_TILE_INTERMEDIATE_PIECE].frames[0].frame, &srcRect, &dstRect);
		} else {
			Render_Texture(sx, sy - (textureFrame->height - 1), TEXTURE_DUNGEONTILES_DUNGEONPIECES, level_piece_id);
		}

		return;
	}

	cel_foliage_active = !nSolidTable[level_piece_id];
	for (int i = 0; i < (MicroTileLen >> 1); i++) {

		//Fluffy
		int curType = lightType;
		if (options_hwIngameRendering && options_lightmapping) {
			if (lightType == LIGHTING_SUBTILE_DIAGONALFORWARD || lightType == LIGHTING_SUBTILE_MIXEDBACKGROUND) { //Start bottomleft
				lightx = lightmap_lightx - (TILE_WIDTH / 2);
				lighty = lightmap_lighty + (TILE_HEIGHT / 2);
				curType = LIGHTING_SUBTILE_DIAGONALFORWARD;
			} else if (lightType == LIGHTING_SUBTILE_DIAGONALBACKWARD || lightType == LIGHTING_SUBTILE_MIXEDFOREGROUND) { //Start topleft
				lightx = lightmap_lightx - (TILE_WIDTH / 2);
				lighty = lightmap_lighty - (TILE_HEIGHT / 2);
				curType = LIGHTING_SUBTILE_DIAGONALBACKWARD;
			}
		}

		level_cel_block = pMap->mt[2 * i];
		if (level_cel_block != 0) {
			arch_draw_type = i == 0 ? 1 : 0;
			if (options_hwIngameRendering) //Fluffy
				RenderTileViaSDL(sx, sy, lightx, lighty, curType);
			else
				RenderTile(out, sx, sy);
		}
		level_cel_block = pMap->mt[2 * i + 1];
		if (level_cel_block != 0) {
			arch_draw_type = i == 0 ? 2 : 0;
			if (options_hwIngameRendering) //Fluffy
				RenderTileViaSDL(sx + TILE_WIDTH / 2, sy, lightx, lighty, curType);
			else
				RenderTile(out, sx + TILE_WIDTH / 2, sy);
		}
		sy -= TILE_HEIGHT;
	}
	cel_foliage_active = false;
}

/**
 * @brief Render a floor tiles
 * @param out Target buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 */
static void drawFloor(CelOutputBuffer out, int x, int y, int sx, int sy)
{
	cel_transparency_active = 0;
	if (options_hwIngameRendering && options_lightmapping) //Fluffy: Force brightness to max for lightmapping
		light_table_index = 0;
	else
		light_table_index = dLight[x][y];

	arch_draw_type = 1; // Left
	level_cel_block = dpiece_defs_map_2[x][y].mt[0];
	if (level_cel_block != 0) {
		if (options_hwIngameRendering) //Fluffy
			RenderTileViaSDL(sx, sy);
		else
			RenderTile(out, sx, sy);
	}
	arch_draw_type = 2; // Right
	level_cel_block = dpiece_defs_map_2[x][y].mt[1];
	if (level_cel_block != 0) {
		if (options_hwIngameRendering) //Fluffy
			RenderTileViaSDL(sx + TILE_WIDTH / 2, sy);
		else
			RenderTile(out, sx + TILE_WIDTH / 2, sy);
	}
}

/**
 * @brief Draw item for a given tile
 * @param out Output buffer
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawItem(CelOutputBuffer out, int x, int y, int sx, int sy, BOOL pre)
{
	char bItem = dItem[x][y];

	assert((unsigned char)bItem <= MAXITEMS);

	if (bItem > MAXITEMS || bItem <= 0)
		return;

	ItemStruct *pItem = &item[bItem - 1];
	if (pItem->_iPostDraw == pre)
		return;

	BYTE *pCelBuff = pItem->_iAnimData;
	if (pCelBuff == NULL) {
		SDL_Log("Draw Item \"%s\" 1: NULL Cel Buffer", pItem->_iIName);
		return;
	}

	int nCel = pItem->_iAnimFrame;
	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		SDL_Log("Draw \"%s\" Item 1: frame %d of %d, item type==%d", pItem->_iIName, nCel, frames, pItem->_itype);
		return;
	}

	int px = sx - pItem->_iAnimWidth2;

	if (options_hwIngameRendering) { //Fluffy: Render item via SDL
		int textureNum = TEXTURE_ITEMS + ItemCAnimTbl[pItem->_iCurs];
		int frameNum = nCel - 1;
		if (bItem - 1 == pcursitem || AutoMapShowItems)
			Render_TextureOutline_FromBottom(px, sy, 121, 127, 160, textureNum, frameNum);
		int brightness = Render_IndexLightToBrightness();
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
		Render_Texture_FromBottom(px, sy, textureNum, frameNum);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
		return;
	}

	if (bItem - 1 == pcursitem || AutoMapShowItems) {
		CelBlitOutlineTo(out, 181, px, sy, pCelBuff, nCel, pItem->_iAnimWidth);
		if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
			CelDrawToImportant(181, px, sy, pItem->_iAnimData, nCel, pItem->_iAnimWidth, true);
	}
	CelClippedDrawLightTo(out, px, sy, pCelBuff, nCel, pItem->_iAnimWidth);

	if (sgOptions.Graphics.bOpaqueWallsWithBlobs) //Fluffy
		CelDrawToImportant_Ellipse(px, sy, pCelBuff, nCel, pItem->_iAnimWidth);
	if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
		CelDrawToImportant(pLightTbl[(256 * light_table_index) + 185], px, sy, pCelBuff, nCel, pItem->_iAnimWidth, false);
}

/**
 * @brief Check if and how a monster should be rendered
 * @param out Output buffer
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param oy dPiece Y offset
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 */
static void DrawMonsterHelper(CelOutputBuffer out, int x, int y, int oy, int sx, int sy)
{
	int mi, px, py;
	MonsterStruct *pMonster;

	mi = dMonster[x][y + oy];
	mi = mi > 0 ? mi - 1 : -(mi + 1);

	if (leveltype == DTYPE_TOWN) {
		px = sx - towner[mi]._tAnimWidth2;

		if (options_hwIngameRendering) { //Fluffy: Render NPC via SDL
			int textureNum = TEXTURE_SMITH;
			switch (towner[mi]._ttype) {
			case TOWN_SMITH:
				textureNum = TEXTURE_SMITH;
				break;
			case TOWN_HEALER:
				textureNum = TEXTURE_HEALER;
				break;
			case TOWN_DEADGUY:
				textureNum = TEXTURE_DEADGUY;
				break;
			case TOWN_TAVERN:
				textureNum = TEXTURE_BAROWNER;
				break;
			case TOWN_WITCH:
				textureNum = TEXTURE_WITCH;
				break;
			case TOWN_STORY:
				textureNum = TEXTURE_STORYTELLER;
				break;
			case TOWN_DRUNK:
				textureNum = TEXTURE_DRUNK;
				break;
			case TOWN_BMAID:
				textureNum = TEXTURE_BARMAID;
				break;
			case TOWN_PEGBOY:
				textureNum = TEXTURE_BOY;
				break;
			case TOWN_COW:
				textureNum = TEXTURE_COWS;
				break;
			case TOWN_FARMER:
				textureNum = TEXTURE_FARMER;
				break;
			case TOWN_GIRL:
				textureNum = TEXTURE_GIRL;
				break;
			case TOWN_COWFARM:
				textureNum = TEXTURE_COWFARMER;
				break;
			}
			int frameNum = towner[mi]._tAnimFrame - 1;
			if (towner[mi]._ttype == TOWN_COW) {
				for (int j = 0; j < 8; j++) //Figure out facing for the cow
					if (towner[mi]._tAnimData == towner[mi]._tNAnim[j]) {
						frameNum += towner[mi]._tAnimLen * j;
						break;
					}
			}
			if (mi == pcursmonst)
				Render_TextureOutline_FromBottom(px, sy, 165, 90, 90, textureNum, frameNum);
			int brightness = Render_IndexLightToBrightness();
			if (brightness < 255)
				SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
			Render_Texture_FromBottom(px, sy, textureNum, frameNum);
			if (brightness < 255)
				SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
			return;
		}

		if (mi == pcursmonst) {
			CelBlitOutlineTo(out, 166, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth);
			if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
				CelDrawToImportant(166, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth, true);
		}
		assert(towner[mi]._tAnimData);
		CelClippedDrawTo(out, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth);
		if (sgOptions.Graphics.bOpaqueWallsWithBlobs) //Fluffy
			CelDrawToImportant_Ellipse(px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth);
		if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
			CelDrawToImportant(170, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth, false);
		return;
	}

	if (!(dFlags[x][y] & BFLAG_LIT) && !plr[myplr]._pInfraFlag)
		return;

	if (mi < 0 || mi >= MAXMONSTERS) {
		SDL_Log("Draw Monster: tried to draw illegal monster %d", mi);
		return;
	}

	pMonster = &monster[mi];
	if (pMonster->_mFlags & MFLAG_HIDDEN) {
		return;
	}

	if (pMonster->MType == NULL) {
		SDL_Log("Draw Monster \"%s\": uninitialized monster", pMonster->mName);
		return;
	}

	px = sx + pMonster->_mxoff - pMonster->MType->width2;
	py = sy + pMonster->_myoff;

	if (options_hwIngameRendering) { //Fluffy: Render monster via SDL
		int brightness = Render_IndexLightToBrightness();
		int textureNum = TEXTURE_MONSTERS + (pMonster->_mMTidx * MA_NUM);
		int facing = -1;
		for (int i = 0; i < MA_NUM; i++) {
			for (int j = 0; j < 8; j++)
				if (pMonster->_mAnimData == pMonster->MType->Anims[i].Data[j]) {
					facing = j;
					textureNum += i;
					goto foundAnim;
				}
		}
		assert(facing != -1);
	foundAnim:
		int frameNum = (pMonster->_mAnimFrame - 1) + (facing * pMonster->_mAnimLen);
		if (mi == pcursmonst)
			Render_TextureOutline_FromBottom(px, py, 147, 30, 30, textureNum, frameNum);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
		Render_Texture_FromBottom(px, py, textureNum, frameNum);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
		//TODO: Do rendering differently if trans is non-zero
		return;
	}

	if (mi == pcursmonst) {
		Cl2DrawOutline(out, 233, px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width);
		if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
			Cl2DrawToImportant(233, px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width, true);
	}
	DrawMonster(out, x, y, px, py, mi);
	if (sgOptions.Graphics.bOpaqueWallsWithBlobs) //Fluffy
		Cl2DrawToImportant_Ellipse(px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width);
	if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
		Cl2DrawToImportant(pLightTbl[(256 * light_table_index) + 234], px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width, false);
}

/**
 * @brief Check if and how a player should be rendered
 * @param out Output buffer
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 */
static void DrawPlayerHelper(CelOutputBuffer out, int x, int y, int sx, int sy)
{
	int p = dPlayer[x][y];
	p = p > 0 ? p - 1 : -(p + 1);

	if (p < 0 || p >= MAX_PLRS) {
		SDL_Log("draw player: tried to draw illegal player %d", p);
		return;
	}

	PlayerStruct *pPlayer = &plr[p];
	int px = sx + pPlayer->_pxoff - pPlayer->_pAnimWidth2;
	int py = sy + pPlayer->_pyoff;

	if (options_hwIngameRendering) { //Fluffy: Render player via SDL
		DrawPlayer_SDL(p, x, y, px, py);
		return;
	}

	DrawPlayer(out, p, x, y, px, py, pPlayer->_pAnimData, pPlayer->_pAnimFrame, pPlayer->_pAnimWidth);
	if (sgOptions.Graphics.bOpaqueWallsWithBlobs) //Fluffy
		Cl2DrawToImportant_Ellipse(px, py, pPlayer->_pAnimData, pPlayer->_pAnimFrame, pPlayer->_pAnimWidth);
	else if (sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
		Cl2DrawToImportant(165, px, py, pPlayer->_pAnimData, pPlayer->_pAnimFrame, pPlayer->_pAnimWidth, false);
}

static void RenderArchViaSDL(int x, int y, int archNum, bool transparent) //Fluffy
{
	if (options_lightmapping) {
		bool forward = true;
		if (leveltype == DTYPE_CATHEDRAL && currlevel < 21) { //Cathedral
			if (archNum == 2 || archNum == 3 || archNum == 8)
				forward = false;
		}
		archNum -= 1;

		SDL_Rect srcRect;
		srcRect.x = 0;
		srcRect.y = 0;
		srcRect.w = 1;
		srcRect.h = textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].height;

		SDL_Rect dstRect;
		dstRect.x = x;
		dstRect.y = y - (srcRect.h - 1);
		dstRect.w = 1;
		dstRect.h = srcRect.h;

		if (transparent)
			SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 191);

		int lightx, lighty, brightness;
		if (forward) {
			lightx = lightmap_lightx - (TILE_WIDTH / 2);
			lighty = lightmap_lighty + (TILE_HEIGHT / 2);
		} else {
			lightx = lightmap_lightx - (TILE_WIDTH / 2);
			lighty = lightmap_lighty - (TILE_HEIGHT / 2);
		}

		for (int i = 0; i < textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].width; i++) {
			brightness = Lightmap_ReturnBrightness(lightx, lighty);
			SDL_SetTextureColorMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, brightness, brightness, brightness);
			SDL_RenderCopy(renderer, textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, &srcRect, &dstRect);
			dstRect.x += 1;
			srcRect.x += 1;
			lightx += 1;
			if (i > 0 && i % 2 == 0) {
				if (forward) {
					lighty -= 1;
				} else {
					lighty += 1;
				}
			}
		}
		if (transparent)
			SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 255);
		return;
	}

	archNum -= 1;
	int brightness = Render_IndexLightToBrightness();
	if (brightness < 255)
		SDL_SetTextureColorMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, brightness, brightness, brightness);
	if (transparent)
		SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 191);
	Render_Texture_FromBottom(x, y, TEXTURE_DUNGEONTILES_SPECIAL, archNum);
	if (brightness < 255)
		SDL_SetTextureColorMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 255, 255, 255);
	if (transparent)
		SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 255);
}

/**
 * @brief Render object sprites
 * @param out Target buffer
 * @param sx dPiece coordinate
 * @param sy dPiece coordinate
 * @param dx Target buffer coordinate
 * @param dy Target buffer coordinate
 */
static void scrollrt_draw_dungeon(CelOutputBuffer out, int sx, int sy, int dx, int dy)
{
	assert((DWORD)sx < MAXDUNX);
	assert((DWORD)sy < MAXDUNY);

	if (dRendered[sx][sy])
		return;
	dRendered[sx][sy] = true;

	light_table_index = dLight[sx][sy];

	if (options_hwIngameRendering && options_lightmapping) { //Fluffy: Set coordinates for light value to use
		lightmap_lightx = (dx + (TILE_WIDTH / 2));
		lightmap_lighty = dy;
		if (lightmap_lightx >= gnScreenWidth)
			lightmap_lightx = gnScreenWidth - 1;
		else if (lightmap_lightx < 0)
			lightmap_lightx = 0;
		if (lightmap_lighty >= gnScreenHeight)
			lightmap_lighty = gnScreenHeight - 1;
		else if (lightmap_lighty < 0)
			lightmap_lighty = 0;
	}

	//Fluffy: In case we are to render a wall here, figure out if there's an important object nearby so we know if it should be opaque or not
	bool importantObjectNearby = 0;
	if (sgOptions.Graphics.bOpaqueWallsUnlessObscuring && !sgOptions.Graphics.bOpaqueWallsWithBlobs && !sgOptions.Graphics.bOpaqueWallsWithSilhouette && light_table_index < lightmax) {
		for (int i = -3; i < 1; i++)
			for (int j = -3; j < 2; j++) {
				int x = i + sx;
				int y = j + sy;
				if (x < 0 || x >= MAXDUNX || y < 0 || y >= MAXDUNY)
					continue;

				//Check for interactable object
				if (dObject[x][y] != 0) {
					int ob = dObject[x][y] > 0 ? dObject[x][y] - 1 : -(dObject[x][y] + 1);
					if (object[ob]._oSelFlag >= 1) {
						importantObjectNearby = 1;
						break;
					}
				}

				//Check for player, monster, item, and missile
				if (dPlayer[x][y] != 0 || dFlags[x][y] & BFLAG_PLAYERLR || dItem[x][y] > 0
				    || dMonster[x][y] != 0 || dFlags[x][y] & BFLAG_MONSTLR || dFlags[x][y] & BFLAG_MISSILE) {
					importantObjectNearby = 1;
					break;
				}
			}
	}

	drawCell(out, sx, sy, dx, dy, importantObjectNearby);

	char bFlag = dFlags[sx][sy];
	char bDead = dDead[sx][sy];
	char bMap = dTransVal[sx][sy];

	int negMon = 0;
	if (sy > 0) // check for OOB
		negMon = dMonster[sx][sy - 1];

#ifdef _DEBUG
	if (visiondebug && bFlag & BFLAG_LIT) {
		CelClippedDrawTo(out, dx, dy, pSquareCel, 1, 64);
	}
#endif

	if (MissilePreFlag) {
		DrawMissile(out, sx, sy, dx, dy, TRUE);
	}

	if (light_table_index < lightmax && bDead != 0) {
		do {
			DeadStruct *pDeadGuy = &dead[(bDead & 0x1F) - 1];
			char dd = (bDead >> 5) & 7;
			int px = dx - pDeadGuy->_deadWidth2;

			if (options_hwIngameRendering) { //Render dead enemy via SDL
				//Figure out which monster in the Monsters array this body belongs to
				int textureNum = -1;
				int frameNum = -1;
				for (int i = 0; i < nummtypes; i++) {
					if (pDeadGuy->_deadData[0] == Monsters[i].Anims[MA_DEATH].Data[0]) {
						textureNum = TEXTURE_MONSTERS + (i * MA_NUM) + MA_DEATH;
						frameNum = (pDeadGuy->_deadFrame - 1) + (dd * Monsters[i].Anims[MA_DEATH].Frames);
						break;
					}
				}
				assert(textureNum != -1);

				//Render
				int brightness = Render_IndexLightToBrightness();
				if (brightness < 255)
					SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
				Render_Texture_FromBottom(px, dy, textureNum, frameNum);
				if (brightness < 255)
					SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
				//TODO: Do rendering differently if pDeadGuy->_deadtrans is non-zero
				break;
			} else {
				BYTE *pCelBuff = pDeadGuy->_deadData[dd];
				assert(pCelBuff != NULL);
				if (pCelBuff == NULL)
					break;
				int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
				int nCel = pDeadGuy->_deadFrame;
				if (nCel < 1 || frames > 50 || nCel > frames) {
					SDL_Log("Unclipped dead: frame %d of %d, deadnum==%d", nCel, frames, (bDead & 0x1F) - 1);
					break;
				}
				if (pDeadGuy->_deadtrans != 0) {
					Cl2DrawLightTbl(out, px, dy, pCelBuff, nCel, pDeadGuy->_deadWidth, pDeadGuy->_deadtrans);
				} else {
					Cl2DrawLight(out, px, dy, pCelBuff, nCel, pDeadGuy->_deadWidth);
				}
			}
		} while (0);
	}
	DrawObject(out, sx, sy, dx, dy, 1);
	DrawItem(out, sx, sy, dx, dy, 1);
	if (bFlag & BFLAG_PLAYERLR) {
		assert((DWORD)(sy - 1) < MAXDUNY);
		DrawPlayerHelper(out, sx, sy - 1, dx, dy);
	}
	if (bFlag & BFLAG_MONSTLR && negMon < 0) {
		DrawMonsterHelper(out, sx, sy, -1, dx, dy);
	}
	if (bFlag & BFLAG_DEAD_PLAYER) {
		DrawDeadPlayer(out, sx, sy, dx, dy);
	}
	if (dPlayer[sx][sy] > 0) {
		DrawPlayerHelper(out, sx, sy, dx, dy);
	}
	if (dMonster[sx][sy] > 0) {
		DrawMonsterHelper(out, sx, sy, 0, dx, dy);
	}
	DrawMissile(out, sx, sy, dx, dy, FALSE);
	DrawObject(out, sx, sy, dx, dy, 0);
	DrawItem(out, sx, sy, dx, dy, 0);

	if (leveltype != DTYPE_TOWN) {
		char bArch = dSpecial[sx][sy];
		if (bArch != 0) {
			if (sgOptions.Graphics.bOpaqueWallsUnlessObscuring && !sgOptions.Graphics.bOpaqueWallsWithBlobs && !sgOptions.Graphics.bOpaqueWallsWithSilhouette && !importantObjectNearby) //Fluffy: Make this opaque if nothing important is nearby
				cel_transparency_active = 0;
			else {
				if (sgOptions.Graphics.bOpaqueWallsWithBlobs || sgOptions.Graphics.bOpaqueWallsWithSilhouette) //Fluffy
					cel_transparency_active = true;
				else
					cel_transparency_active = TransList[bMap];
			}
#ifdef _DEBUG
			if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000) {
				cel_transparency_active = 0; // Turn transparency off here for debugging
			}
#endif
			if (options_hwIngameRendering) //Fluffy: Render via SDL
				RenderArchViaSDL(dx, dy, bArch, cel_transparency_active != 0);
			else
				CelClippedBlitLightTransTo(out, dx, dy, pSpecialCels, bArch, 64);
#ifdef _DEBUG
			if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000) {
				cel_transparency_active = TransList[bMap]; // Turn transparency back to its normal state
			}
#endif
		}
	} else {
		// Tree leaves should always cover player when entering or leaving the tile,
		// So delay the rendering until after the next row is being drawn.
		// This could probably have been better solved by sprites in screen space.
		if (sx > 0 && sy > 0 && dy > TILE_HEIGHT) {
			char bArch = dSpecial[sx - 1][sy - 1];
			if (bArch != 0) {
				if (options_hwIngameRendering) //Fluffy: Render via SDL
					RenderArchViaSDL(dx, dy - TILE_HEIGHT, bArch, 0);
				else
					CelDrawTo(out, dx, dy - TILE_HEIGHT, pSpecialCels, bArch, 64);
			}
		}
	}
}

/**
 * @brief Render a row of tiles
 * @param out Buffer to render to
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param rows Number of rows
 * @param columns Tile in a row
 */
static void scrollrt_drawFloor(CelOutputBuffer out, int x, int y, int sx, int sy, int rows, int columns)
{
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (x >= 0 && x < MAXDUNX && y >= 0 && y < MAXDUNY) {
				level_piece_id = dPiece[x][y];
				if (level_piece_id != 0) {
					if (!nSolidTable[level_piece_id])
						drawFloor(out, x, y, sx, sy);
				} else {
					world_draw_black_tile(out, sx, sy);
				}
			} else {
				world_draw_black_tile(out, sx, sy);
			}
			ShiftGrid(&x, &y, 1, 0);
			sx += TILE_WIDTH;
		}
		// Return to start of row
		ShiftGrid(&x, &y, -columns, 0);
		sx -= columns * TILE_WIDTH;

		// Jump to next row
		sy += TILE_HEIGHT / 2;
		if (i & 1) {
			x++;
			columns--;
			sx += TILE_WIDTH / 2;
		} else {
			y++;
			columns++;
			sx -= TILE_WIDTH / 2;
		}
	}
}

#define IsWall(x, y) (dPiece[x][y] == 0 || nSolidTable[dPiece[x][y]] || dSpecial[x][y] != 0)
#define IsWalkable(x, y) (dPiece[x][y] != 0 && !nSolidTable[dPiece[x][y]])

/**
 * @brief Render a row of tile
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Buffer coordinate
 * @param sy Buffer coordinate
 * @param rows Number of rows
 * @param columns Tile in a row
 */
static void scrollrt_draw(CelOutputBuffer out, int x, int y, int sx, int sy, int rows, int columns)
{
	// Keep evaluating until MicroTiles can't affect screen
	rows += MicroTileLen;
	memset(dRendered, 0, sizeof(dRendered));

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (x >= 0 && x < MAXDUNX && y >= 0 && y < MAXDUNY) {
				if (x + 1 < MAXDUNX && y - 1 >= 0 && sx + TILE_WIDTH <= gnScreenWidth) {
					// Render objects behind walls first to prevent sprites, that are moving
					// between tiles, from poking through the walls as they exceed the tile bounds.
					// A proper fix for this would probably be to layout the sceen and render by
					// sprite screen position rather than tile position.
					if (IsWall(x, y) && (IsWall(x + 1, y) || (x > 0 && IsWall(x - 1, y)))) { // Part of a wall aligned on the x-axis
						if (IsWalkable(x + 1, y - 1) && IsWalkable(x, y - 1)) {              // Has walkable area behind it
							scrollrt_draw_dungeon(out, x + 1, y - 1, sx + TILE_WIDTH, sy);
						}
					}
				}
				if (dPiece[x][y] != 0) {
					scrollrt_draw_dungeon(out, x, y, sx, sy);
				}
			}
			ShiftGrid(&x, &y, 1, 0);
			sx += TILE_WIDTH;
		}
		// Return to start of row
		ShiftGrid(&x, &y, -columns, 0);
		sx -= columns * TILE_WIDTH;

		// Jump to next row
		sy += TILE_HEIGHT / 2;
		if (i & 1) {
			x++;
			columns--;
			sx += TILE_WIDTH / 2;
		} else {
			y++;
			columns++;
			sx -= TILE_WIDTH / 2;
		}
	}
}

/**
 * @brief Scale up the rendered part of the back buffer to take up the full view
 */
static void Zoom(CelOutputBuffer out)
{
	int wdt = gnScreenWidth / 2;

	int src_x = gnScreenWidth / 2 - 1;
	int dst_x = gnScreenWidth - 1;

	if (PANELS_COVER) {
		if (chrflag || questlog) {
			wdt >>= 1;
			src_x -= wdt;
		} else if (invflag || sbookflag) {
			wdt >>= 1;
			src_x -= wdt;
			dst_x -= SPANEL_WIDTH;
		}
	}

	BYTE *src = out.at(src_x, gnViewportHeight / 2 - 1);
	BYTE *dst = out.at(dst_x, gnViewportHeight - 1);

	for (int hgt = 0; hgt < gnViewportHeight / 2; hgt++) {
		for (int i = 0; i < wdt; i++) {
			*dst-- = *src;
			*dst-- = *src;
			src--;
		}
		memcpy(dst - out.pitch(), dst, wdt * 2 + 1);
		src -= out.pitch() - wdt;
		dst -= 2 * (out.pitch() - wdt);
	}
}

/**
 * @brief Shifting the view area along the logical grid
 *        Note: this won't allow you to shift between even and odd rows
 * @param horizontal Shift the screen left or right
 * @param vertical Shift the screen up or down
 */
void ShiftGrid(int *x, int *y, int horizontal, int vertical)
{
	*x += vertical + horizontal;
	*y += vertical - horizontal;
}

/**
 * @brief Gets the number of rows covered by the main panel
 */
int RowsCoveredByPanel()
{
	if (gnScreenWidth <= PANEL_WIDTH) {
		return 0;
	}

	int rows = PANEL_HEIGHT / TILE_HEIGHT;
	if (!zoomflag) {
		rows /= 2;
	}

	return rows;
}

/**
 * @brief Calculate the offset needed for centering tiles in view area
 * @param offsetX Offset in pixels
 * @param offsetY Offset in pixels
 */
void CalcTileOffset(int *offsetX, int *offsetY)
{
	int x, y;

	if (zoomflag) {
		x = gnScreenWidth % TILE_WIDTH;
		y = gnViewportHeight % TILE_HEIGHT;
	} else {
		x = (gnScreenWidth / 2) % TILE_WIDTH;
		y = (gnViewportHeight / 2) % TILE_HEIGHT;
	}

	if (x)
		x = (TILE_WIDTH - x) / 2;
	if (y)
		y = (TILE_HEIGHT - y) / 2;

	*offsetX = x;
	*offsetY = y;
}

/**
 * @brief Calculate the needed diamond tile to cover the view area
 * @param columns Tiles needed per row
 * @param rows Both even and odd rows
 */
void TilesInView(int *rcolumns, int *rrows)
{
	int columns = gnScreenWidth / TILE_WIDTH;
	if (gnScreenWidth % TILE_WIDTH) {
		columns++;
	}
	int rows = gnViewportHeight / TILE_HEIGHT;
	if (gnViewportHeight % TILE_HEIGHT) {
		rows++;
	}

	if (!zoomflag) {
		// Half the number of tiles, rounded up
		if (columns & 1) {
			columns++;
		}
		columns /= 2;
		if (rows & 1) {
			rows++;
		}
		rows /= 2;
	}

	*rcolumns = columns;
	*rrows = rows;
}

int tileOffsetX;
int tileOffsetY;
int tileShiftX;
int tileShiftY;
int tileColums;
int tileRows;

void CalcViewportGeometry()
{
	int xo, yo;
	tileShiftX = 0;
	tileShiftY = 0;

	// Adjust by player offset and tile grid alignment
	CalcTileOffset(&xo, &yo);
	tileOffsetX = 0 - xo;
	tileOffsetY = 0 - yo - 1 + TILE_HEIGHT / 2;

	TilesInView(&tileColums, &tileRows);
	int lrow = tileRows - RowsCoveredByPanel();

	// Center player tile on screen
	ShiftGrid(&tileShiftX, &tileShiftY, -tileColums / 2, -lrow / 2);

	tileRows *= 2;

	// Align grid
	if ((tileColums & 1) == 0) {
		tileShiftY--; // Shift player row to one that can be centered with out pixel offset
		if ((lrow & 1) == 0) {
			// Offset tile to vertically align the player when both rows and colums are even
			tileRows++;
			tileOffsetY -= TILE_HEIGHT / 2;
		}
	} else if (tileColums & 1 && lrow & 1) {
		// Offset tile to vertically align the player when both rows and colums are odd
		ShiftGrid(&tileShiftX, &tileShiftY, 0, -1);
		tileRows++;
		tileOffsetY -= TILE_HEIGHT / 2;
	}

	// Slightly lower the zoomed view
	if (!zoomflag) {
		tileOffsetY += TILE_HEIGHT / 4;
		if (yo < TILE_HEIGHT / 4)
			tileRows++;
	}

	tileRows++; // Cover lower edge saw tooth, right edge accounted for in scrollrt_draw()
}

/**
 * @brief Configure render and process screen rows
 * @param full_out Buffer to render to
 * @param x Center of view in dPiece coordinate
 * @param y Center of view in dPiece coordinate
 */
static void DrawGame(CelOutputBuffer full_out, int x, int y)
{
	int sx, sy, columns, rows;

	// Limit rendering to the view area
	CelOutputBuffer out = zoomflag
	    ? full_out.subregionY(0, gnViewportHeight)
	    : full_out.subregionY(0, gnViewportHeight / 2);

	// Adjust by player offset and tile grid alignment
	sx = ScrollInfo._sxoff + tileOffsetX;
	sy = ScrollInfo._syoff + tileOffsetY;

	columns = tileColums;
	rows = tileRows;

	x += tileShiftX;
	y += tileShiftY;

	// Skip rendering parts covered by the panels
	if (PANELS_COVER) {
		if (zoomflag) {
			if (chrflag || questlog) {
				ShiftGrid(&x, &y, 2, 0);
				columns -= 4;
				sx += SPANEL_WIDTH - TILE_WIDTH / 2;
			}
			if (invflag || sbookflag) {
				ShiftGrid(&x, &y, 2, 0);
				columns -= 4;
				sx += -TILE_WIDTH / 2;
			}
		} else {
			if (chrflag || questlog) {
				ShiftGrid(&x, &y, 1, 0);
				columns -= 2;
				sx += -TILE_WIDTH / 2 / 2; // SPANEL_WIDTH accounted for in Zoom()
			}
			if (invflag || sbookflag) {
				ShiftGrid(&x, &y, 1, 0);
				columns -= 2;
				sx += -TILE_WIDTH / 2 / 2;
			}
		}
	}

	// Draw areas moving in and out of the screen
	switch (ScrollInfo._sdir) {
	case SDIR_N:
		sy -= TILE_HEIGHT;
		ShiftGrid(&x, &y, 0, -1);
		rows += 2;
		break;
	case SDIR_NE:
		sy -= TILE_HEIGHT;
		ShiftGrid(&x, &y, 0, -1);
		columns++;
		rows += 2;
		break;
	case SDIR_E:
		columns++;
		break;
	case SDIR_SE:
		columns++;
		rows++;
		break;
	case SDIR_S:
		rows += 2;
		break;
	case SDIR_SW:
		sx -= TILE_WIDTH;
		ShiftGrid(&x, &y, -1, 0);
		columns++;
		rows++;
		break;
	case SDIR_W:
		sx -= TILE_WIDTH;
		ShiftGrid(&x, &y, -1, 0);
		columns++;
		break;
	case SDIR_NW:
		sx -= TILE_WIDTH / 2;
		sy -= TILE_HEIGHT / 2;
		x--;
		columns++;
		rows++;
		break;
	}

	//Fluffy TODO Merge: Rewrite this
	/*if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
		memset(gpBuffer_important, 0, BUFFER_WIDTH * BUFFER_HEIGHT); //Fluffy: Reset "important" buffer before drawing stuff*/

	if (options_hwIngameRendering && options_lightmapping) { //Fluffy: Process all entities with a light source and have them add lights to the lightmap buffer
		Lightmap_MakeLightmap(x, y, sx, sy, rows, columns);
		lightmap_lightx = -1;
		lightmap_lighty = -1;
	}

	scrollrt_drawFloor(out, x, y, sx, sy, rows, columns);

	if (options_hwIngameRendering && options_lightmapping) { //Fluffy: Render lightmap on top of floor tiles

		/*
		//This applies more of a contrast to the lighting. Maybe it has some potential?
		SDL_SetRenderTarget(renderer, textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame);
		SDL_SetTextureBlendMode(texture_intermediate, SDL_BLENDMODE_MOD);
		SDL_RenderCopy(renderer, texture_intermediate, NULL, NULL);
		SDL_SetTextureBlendMode(texture_intermediate, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(renderer, texture_intermediate);
		Render_Texture(0, 0, TEXTURE_LIGHT_FRAMEBUFFER);
		*/

		//SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame, SDL_BLENDMODE_NONE);
		//if (currlevel != 0) //We don't apply lightmap to the town
		Render_Texture_Crop(0, 0, TEXTURE_LIGHT_FRAMEBUFFER, 0, 0, gnScreenWidth, gnScreenHeight);
		//SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame, SDL_BLENDMODE_MOD);
	}

	scrollrt_draw(out, x, y, sx, sy, rows, columns);

	if (options_hwIngameRendering && !zoomflag) { //Fluffy: Scale up the render if we're zooming in. TODO: Implement integer scaling for this?
		SDL_SetRenderTarget(renderer, textures[TEXTURE_TILE_INTERMEDIATE_BIG].frames[0].frame);
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = gnScreenWidth / 2;
		rect.h = gnScreenHeight / 2;
		SDL_RenderCopy(renderer, texture_intermediate, &rect, NULL);
		SDL_SetRenderTarget(renderer, texture_intermediate);
		SDL_RenderCopy(renderer, textures[TEXTURE_TILE_INTERMEDIATE_BIG].frames[0].frame, NULL, NULL);
	}

#ifdef LIGHTMAP_SUBTILE_EDITOR
	//Fluffy sub-tile editor
	Lightmap_SubtilePreview();
#endif

	if (!zoomflag) {
		Zoom(full_out.subregionY(0, gnScreenHeight));
	}
}

// DevilutionX extension.
extern void DrawControllerModifierHints(CelOutputBuffer out);

void DrawView(CelOutputBuffer out, int StartX, int StartY)
{
	DrawGame(out, StartX, StartY);
	if (automapflag) {
		DrawAutomap(out.subregionY(0, gnViewportHeight));
	}
	DrawMonsterHealthBar(out);

	if (stextflag && !qtextflag)
		DrawSText(out);
	if (invflag) {
		DrawInv(out);
	} else if (sbookflag) {
		DrawSpellBook(out);
	}

	DrawDurIcon(out);

	if (chrflag) {
		DrawChr(out);
	} else if (questlog) {
		DrawQuestLog(out);
	}
	if (!chrflag && plr[myplr]._pStatPts != 0 && !spselflag
	    && (!questlog || gnScreenHeight >= SPANEL_HEIGHT + PANEL_HEIGHT + 74 || gnScreenWidth >= 4 * SPANEL_WIDTH)) {
		DrawLevelUpIcon(out);
	}
	if (uitemflag) {
		DrawUniqueInfo(out);
	}
	if (qtextflag) {
		DrawQText(out);
	}
	if (spselflag) {
		DrawSpellList(out);
	}
	if (dropGoldFlag) {
		DrawGoldSplit(out, dropGoldValue);
	}
	if (helpflag) {
		DrawHelp(out);
	}
	if (msgflag) {
		DrawDiabloMsg(out);
	}
	if (deathflag) {
		RedBack(out);
	} else if (PauseMode != 0) {
		gmenu_draw_pause(out);
	}

	DrawControllerModifierHints(out);
	DrawPlrMsg(out);
	gmenu_draw(out);
	doom_draw(out);
	DrawInfoBox(out);
	DrawLifeFlask(out);
	DrawManaFlask(out);
}

extern SDL_Surface *pal_surface;

/**
 * @brief Render the whole screen black
 */
void ClearScreenBuffer()
{
	lock_buf(3);

	assert(pal_surface != NULL);

	SDL_Rect SrcRect = {
		BUFFER_BORDER_LEFT,
		BUFFER_BORDER_TOP,
		gnScreenWidth,
		gnScreenHeight,
	};
	SDL_FillRect(pal_surface, &SrcRect, 0);

	unlock_buf(3);

	if (options_hwUIRendering) //Fluffy: Also clear the intermediate texture
	{
		if (options_lightmapping) { //Clear lightmap
			SDL_SetRenderTarget(renderer, textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
			SDL_RenderClear(renderer);
		}

		//I'm pretty sure this function is only called outside of ingame rendering, so it should be fine to switch to and from render-to-texture
		SDL_SetRenderTarget(renderer, texture_intermediate);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
		SDL_RenderClear(renderer);
		SDL_SetRenderTarget(renderer, NULL);
	}
}

#ifdef _DEBUG
/**
 * @brief Scroll the screen when mouse is close to the edge
 */
void ScrollView()
{
	BOOL scroll;

	if (pcurs >= CURSOR_FIRSTITEM)
		return;

	scroll = FALSE;

	if (MouseX < 20) {
		if (dmaxy - 1 <= ViewY || dminx >= ViewX) {
			if (dmaxy - 1 > ViewY) {
				ViewY++;
				scroll = TRUE;
			}
			if (dminx < ViewX) {
				ViewX--;
				scroll = TRUE;
			}
		} else {
			ViewY++;
			ViewX--;
			scroll = TRUE;
		}
	}
	if (MouseX > gnScreenWidth - 20) {
		if (dmaxx - 1 <= ViewX || dminy >= ViewY) {
			if (dmaxx - 1 > ViewX) {
				ViewX++;
				scroll = TRUE;
			}
			if (dminy < ViewY) {
				ViewY--;
				scroll = TRUE;
			}
		} else {
			ViewY--;
			ViewX++;
			scroll = TRUE;
		}
	}
	if (MouseY < 20) {
		if (dminy >= ViewY || dminx >= ViewX) {
			if (dminy < ViewY) {
				ViewY--;
				scroll = TRUE;
			}
			if (dminx < ViewX) {
				ViewX--;
				scroll = TRUE;
			}
		} else {
			ViewX--;
			ViewY--;
			scroll = TRUE;
		}
	}
	if (MouseY > gnScreenHeight - 20) {
		if (dmaxy - 1 <= ViewY || dmaxx - 1 <= ViewX) {
			if (dmaxy - 1 > ViewY) {
				ViewY++;
				scroll = TRUE;
			}
			if (dmaxx - 1 > ViewX) {
				ViewX++;
				scroll = TRUE;
			}
		} else {
			ViewX++;
			ViewY++;
			scroll = TRUE;
		}
	}

	if (scroll)
		ScrollInfo._sdir = SDIR_NONE;
}
#endif

/**
 * @brief Initialize the FPS meter
 */
void EnableFrameCount()
{
	frameflag = frameflag == 0;
	framestart = SDL_GetPerformanceCounter(); //Fluffy
}

static void RenderDebugLine(CelOutputBuffer out, int *x, int *y, char *line)
{
	PrintGameStr(out, *x, *y, line, COL_RED);
	*y += 15;
}

/**
 * @brief Display the current average FPS over 1 sec
 */
static void DrawFPS(CelOutputBuffer out)
{
	//Fluffy: Updated this code to use high precision timer
	unsigned long long timeDiff;
	char String[100];
	HDC hdc;
	unsigned long long tc;

	if (frameflag && gbActive && pPanelText) {
		frameend++;
		tc = SDL_GetPerformanceCounter();
		timeDiff = tc - framestart;
		if (timeDiff >= SDL_GetPerformanceFrequency()) {
			framestart = tc;
			framerate = frameend;
			frameend = 0;
		}
		int x = 8, y = 10;
		snprintf(String, 100, "FPS: %d", framerate);
		RenderDebugLine(out, &x, &y, String);

		//Fluffy: Additional debug output
		snprintf(String, 100, "gametick delta: %0.2f", frame_gameplayTickDelta);
		RenderDebugLine(out, &x, &y, String);
		snprintf(String, 100, "render delta: %0.2f ", frame_renderDelta);
		RenderDebugLine(out, &x, &y, String);

		if (myplr == 0) {
			snprintf(String, 100, "playerXY: %i %i", plr[myplr]._px, plr[myplr]._py);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "playerOffsetXY: %i %i", plr[myplr]._pxoff, plr[myplr]._pyoff);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "cameraXY: %i %i", ViewX, ViewY);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "cameraOffsetXY: %i %i", ScrollInfo._sxoff / gSpeedMod, ScrollInfo._syoff / gSpeedMod);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "mouseXY: %i %i", MouseX, MouseY);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "lastLeftMouseButtonAction: %i", lastLeftMouseButtonAction);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "lastRightMouseButtonAction: %i", lastRightMouseButtonAction);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "pcurs: %i", pcurs);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "sgbMouseDown: %i", sgbMouseDown);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "safetyCounter: %i", plr[myplr].safetyCounter);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "hotbar: %i", sgOptions.Gameplay.bHotbar);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "selectedHotbarSlot: %i", selectedHotbarSlot);
			RenderDebugLine(out, &x, &y, String);

			snprintf(String, 100, "hotBarItemLinks: %i %i %i %i %i %i %i %i", hotbarSlots[0].itemLink, hotbarSlots[1].itemLink, hotbarSlots[2].itemLink, hotbarSlots[3].itemLink, hotbarSlots[4].itemLink, hotbarSlots[5].itemLink, hotbarSlots[6].itemLink, hotbarSlots[7].itemLink);
			RenderDebugLine(out, &x, &y, String);

			if (sgOptions.Graphics.bInitHwUIRendering) {
				if (totalTextureSize < 1 << 10)
					snprintf(String, 100, "loadedTextures: %u", totalTextureSize);
				else if (totalTextureSize < (1 << 20) * 10)
					snprintf(String, 100, "loadedTextures: %uKB", totalTextureSize >> 10);
				else if (totalTextureSize < (1 << 30) * 10)
					snprintf(String, 100, "loadedTextures: %uMB", totalTextureSize >> 20);
				else
					snprintf(String, 100, "loadedTextures: %uGB", totalTextureSize >> 30);
				RenderDebugLine(out, &x, &y, String);

				snprintf(String, 100, "hwUIRendering: %s", options_hwUIRendering ? "ON" : "OFF");
				RenderDebugLine(out, &x, &y, String);

				snprintf(String, 100, "hwIngameRendering: %s", options_hwIngameRendering ? "ON" : "OFF");
				RenderDebugLine(out, &x, &y, String);

				snprintf(String, 100, "lightMapping: %s", options_lightmapping ? "ON" : "OFF");
				RenderDebugLine(out, &x, &y, String);
			}
		}
	}
}

/**
 * @brief Update part of the screen from the back buffer
 * @param dwX Back buffer coordinate
 * @param dwY Back buffer coordinate
 * @param dwWdt Back buffer coordinate
 * @param dwHgt Back buffer coordinate
 */
static void DoBlitScreen(Sint16 dwX, Sint16 dwY, Uint16 dwWdt, Uint16 dwHgt)
{
	// In SDL1 SDL_Rect x and y are Sint16. Cast explicitly to avoid a compiler warning.
	using CoordType = decltype(SDL_Rect {}.x);
	SDL_Rect src_rect {
		static_cast<CoordType>(BUFFER_BORDER_LEFT + dwX),
		static_cast<CoordType>(BUFFER_BORDER_TOP + dwY),
		dwWdt, dwHgt
	};
	SDL_Rect dst_rect { dwX, dwY, dwWdt, dwHgt };

	BltFast(&src_rect, &dst_rect);
}

/**
 * @brief Check render pipeline and blit individual screen parts
 * @param dwHgt Section of screen to update from top to bottom
 * @param draw_desc Render info box
 * @param draw_hp Render health bar
 * @param draw_mana Render mana bar
 * @param draw_sbar Render belt
 * @param draw_btn Render panel buttons
 */
static void DrawMain(int dwHgt, BOOL draw_desc, BOOL draw_hp, BOOL draw_mana, BOOL draw_sbar, BOOL draw_btn)
{
	if (!gbActive) {
		return;
	}

	assert(dwHgt >= 0 && dwHgt <= gnScreenHeight);

	if (dwHgt > 0) {
		DoBlitScreen(0, 0, gnScreenWidth, dwHgt);
	}
	if (dwHgt < gnScreenHeight) {
		if (draw_sbar) {
			DoBlitScreen(PANEL_LEFT + 204, PANEL_TOP + 5, 232, 28);
		}
		if (draw_desc) {
			DoBlitScreen(PANEL_LEFT + 176, PANEL_TOP + 46, 288, 60);
		}
		if (draw_mana) {
			DoBlitScreen(PANEL_LEFT + 460, PANEL_TOP, 88, 72);
			DoBlitScreen(PANEL_LEFT + 564, PANEL_TOP + 64, 56, 56);
		}
		if (draw_hp) {
			DoBlitScreen(PANEL_LEFT + 96, PANEL_TOP, 88, 72);
		}
		if (draw_btn) {
			DoBlitScreen(PANEL_LEFT + 8, PANEL_TOP + 5, 72, 119);
			DoBlitScreen(PANEL_LEFT + 556, PANEL_TOP + 5, 72, 48);
			if (gbIsMultiplayer) {
				DoBlitScreen(PANEL_LEFT + 84, PANEL_TOP + 91, 36, 32);
				DoBlitScreen(PANEL_LEFT + 524, PANEL_TOP + 91, 36, 32);
			}
		}
		if (sgdwCursWdtOld != 0) {
			DoBlitScreen(sgdwCursXOld, sgdwCursYOld, sgdwCursWdtOld, sgdwCursHgtOld);
		}
		if (sgdwCursWdt != 0) {
			DoBlitScreen(sgdwCursX, sgdwCursY, sgdwCursWdt, sgdwCursHgt);
		}
	}
}

/**
 * @brief Redraw screen
 * @param draw_cursor
 */
void scrollrt_draw_game_screen(BOOL draw_cursor)
{
	int hgt = 0;

	if (force_redraw == 255) {
		force_redraw = 0;
		hgt = gnScreenHeight;
	}

	if (draw_cursor) {
		lock_buf(0);
		scrollrt_draw_cursor_item(GlobalBackBuffer());
		unlock_buf(0);
	}

	DrawMain(hgt, FALSE, FALSE, FALSE, FALSE, FALSE);

	if (draw_cursor) {
		if (!options_hwUIRendering) { //Fluffy: Only remove cursor from buffer if we're doing 8-bit rendering
			lock_buf(0);
			scrollrt_draw_cursor_back_buffer(GlobalBackBuffer());
			unlock_buf(0);
		}
	}
	RenderPresent();
}

/**
 * @brief Render the game
 */
void DrawAndBlit()
{
	if (!gbRunGame) {
		return;
	}

	if (options_hwUIRendering) //Fluffy: Change render target to texture
	{
		if (options_hwIngameRendering && options_lightmapping) { //Clear lightmap
			SDL_SetRenderTarget(renderer, textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
			SDL_RenderClear(renderer);
		}

		SDL_SetRenderTarget(renderer, texture_intermediate);

		//Clear the render target
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
		SDL_RenderClear(renderer);
	}

	int hgt = 0;
	bool ddsdesc = false;
	bool ctrlPan = false;

	if (gnScreenWidth > PANEL_WIDTH || force_redraw == 255) {
		drawhpflag = TRUE;
		drawmanaflag = TRUE;
		drawbtnflag = TRUE;
		drawsbarflag = TRUE;
		ddsdesc = false;
		ctrlPan = true;
		hgt = gnScreenHeight;
	} else if (force_redraw == 1) {
		ddsdesc = true;
		ctrlPan = false;
		hgt = gnViewportHeight;
	}

	force_redraw = 0;

	lock_buf(0);
	CelOutputBuffer out = GlobalBackBuffer();

	DrawView(out, ViewX, ViewY);
	if (ctrlPan) {
		DrawCtrlPan(out);
	}
	if (drawhpflag) {
		UpdateLifeFlask(out);
	}
	if (drawmanaflag) {
		UpdateManaFlask(out);
	}
	if (drawbtnflag) {
		DrawCtrlBtns(out);
	}
	if (drawsbarflag) {
		 if(!sgOptions.Gameplay.bHotbar) //Fluffy: Draw hotbar rather than belt if hotbar is on
			 DrawInvBelt(out);
		 else
			 Hotbar_Render(out);
	}

	if (talkflag) {
		DrawTalkPan(out);
		hgt = gnScreenHeight;
	}
	DrawXPBar(out);
	scrollrt_draw_cursor_item(out);

	DrawFPS(out);

	unlock_buf(0);

	DrawMain(hgt, ddsdesc, drawhpflag, drawmanaflag, drawsbarflag, drawbtnflag);

	if (!options_hwUIRendering) { //Fluffy: Only remove cursor from buffer if we're doing 8-bit rendering
		lock_buf(0);
		scrollrt_draw_cursor_back_buffer(GlobalBackBuffer());
		unlock_buf(0);
	}

	if (options_hwUIRendering) //Fluffy: Reset render target
	{
		SDL_SetRenderTarget(renderer, NULL);
	}

	RenderPresent();

	drawhpflag = FALSE;
	drawmanaflag = FALSE;
	drawbtnflag = FALSE;
	drawsbarflag = FALSE;
}

DEVILUTION_END_NAMESPACE
