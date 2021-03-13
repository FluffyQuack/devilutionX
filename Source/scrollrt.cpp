/**
 * @file plrmsg.cpp
 *
 * Implementation of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#include "all.h"
#include "textures/textures.h" //Fluffy: For rendering 32-bit textures
#include "render/sdl-render.h" //Fluffy: For rendering 32-bit textures
#include "render/lightmap.h" //Fluffy: For lightmap generation

DEVILUTION_BEGIN_NAMESPACE

/**
 * Specifies the current light entry.
 */
int light_table_index; //0 means fully lit. Higher value means darker lighting (usually maxes out at 15)
DWORD sgdwCursWdtOld;
DWORD sgdwCursX;
DWORD sgdwCursY;
/**
 * Upper bound of back buffer.
 */
BYTE *gpBufStart;
/**
 * Lower bound of back buffer.
 */
BYTE *gpBufEnd;
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

/* data */

/* used in 1.00 debug */
const char *const szMonModeAssert[18] = {
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

const char *const szPlrModeAssert[12] = {
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

/**
 * @brief Remove the cursor from the back buffer
 */
static void scrollrt_draw_cursor_back_buffer()
{
	int i;
	BYTE *src, *dst;

	if (sgdwCursWdt == 0) {
		return;
	}

	assert(gpBuffer);
	src = sgSaveBack;
	dst = &gpBuffer[SCREENXY(sgdwCursX, sgdwCursY)];
	i = sgdwCursHgt;

	if (sgdwCursHgt != 0) {
		while (i--) {
			memcpy(dst, src, sgdwCursWdt);
			src += sgdwCursWdt;
			dst += BUFFER_WIDTH;
		}
	}

	sgdwCursXOld = sgdwCursX;
	sgdwCursYOld = sgdwCursY;
	sgdwCursWdtOld = sgdwCursWdt;
	sgdwCursHgtOld = sgdwCursHgt;
	sgdwCursWdt = 0;
}

/**
 * @brief Draw the cursor on the back buffer
 */
static void scrollrt_draw_cursor_item()
{
	int i, mx, my, col = 0;
	BYTE *src, *dst;

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
	} else if (mx > SCREEN_WIDTH - 1) {
		return;
	}
	my = MouseY - 1;
	if (my < 0 - cursH - 1) {
		return;
	} else if (my > SCREEN_HEIGHT - 1) {
		return;
	}

	if (!options_hwRendering) { //Fluffy: Only do backup of cursor if we're doing 8-bit rendering
		sgdwCursX = mx;
		sgdwCursWdt = sgdwCursX + cursW + 1;
		if (sgdwCursWdt > SCREEN_WIDTH - 1) {
			sgdwCursWdt = SCREEN_WIDTH - 1;
		}
		sgdwCursX &= ~3;
		sgdwCursWdt |= 3;
		sgdwCursWdt -= sgdwCursX;
		sgdwCursWdt++;

		sgdwCursY = my;
		sgdwCursHgt = sgdwCursY + cursH + 1;
		if (sgdwCursHgt > SCREEN_HEIGHT - 1) {
			sgdwCursHgt = SCREEN_HEIGHT - 1;
		}
		sgdwCursHgt -= sgdwCursY;
		sgdwCursHgt++;

		assert(sgdwCursWdt * sgdwCursHgt < sizeof sgSaveBack);
		assert(gpBuffer);
		dst = sgSaveBack;
		src = &gpBuffer[SCREENXY(sgdwCursX, sgdwCursY)];

		for (i = sgdwCursHgt; i != 0; i--, dst += sgdwCursWdt, src += BUFFER_WIDTH) {
			memcpy(dst, src, sgdwCursWdt);
		}
	}

	mx++;
	my++;
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_HEIGHT + SCREEN_Y) - cursW - 2];

	if (pcurs >= CURSOR_FIRSTITEM) {
		col = ICOL_WHITE;
		if (plr[myplr].HoldItem._iMagical != 0) {
			col = ICOL_BLUE;
		}
		if (!plr[myplr].HoldItem._iStatFlag) {
			col = ICOL_RED;
		}
	}
	DrawCursorItemWrapper(mx + SCREEN_X, my + cursH + SCREEN_Y - 1, pcurs, cursW, 1, col == ICOL_RED, pcurs >= CURSOR_FIRSTITEM, col);
}

/**
 * @brief Render a missile sprite
 * @param m Pointer to MissileStruct struct
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pre Is the sprite in the background
 */
void DrawMissilePrivate(MissileStruct *m, int sx, int sy, BOOL pre)
{
	int mx, my, nCel, frames;
	BYTE *pCelBuff;

	if (m->_miPreFlag != pre || !m->_miDrawFlag)
		return;

	mx = sx + m->_mixoff - m->_miAnimWidth2;
	my = sy + m->_miyoff;

	pCelBuff = m->_miAnimData;
	if (!pCelBuff) {
		// app_fatal("Draw Missile 2 type %d: NULL Cel Buffer", m->_mitype);
		return;
	}

	if (options_hwRendering) { //Fluffy: Render missile via SDL

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
		Render_Texture_FromBottom(mx - BORDER_LEFT, my - BORDER_TOP, textureNum, frameNum);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
		//TODO: Handle m->_miUniqTrans
		return;
	}
	
	nCel = m->_miAnimFrame;
	frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		// app_fatal("Draw Missile 2: frame %d of %d, missile type==%d", nCel, frames, m->_mitype);
		return;
	}
	if (m->_miUniqTrans)
		Cl2DrawLightTbl(mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth, m->_miUniqTrans + 3);
	else if (m->_miLightFlag)
		Cl2DrawLight(mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth);
	else
		Cl2Draw(mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth);
}

/**
 * @brief Render a missile sprites for a given tile
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pre Is the sprite in the background
 */
void DrawMissile(int x, int y, int sx, int sy, BOOL pre)
{
	int i;
	MissileStruct *m;

	if (!(dFlags[x][y] & BFLAG_MISSILE))
		return;

	if (dMissile[x][y] != -1) {
		m = &missile[dMissile[x][y] - 1];
		DrawMissilePrivate(m, sx, sy, pre);
		return;
	}

	for (i = 0; i < nummissiles; i++) {
		assert(missileactive[i] < MAXMISSILES);
		m = &missile[missileactive[i]];
		if (m->_mix != x || m->_miy != y)
			continue;
		DrawMissilePrivate(m, sx, sy, pre);
	}
}

/**
 * @brief Render a monster sprite
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param mx Back buffer coordinate
 * @param my Back buffer coordinate
 * @param m Id of monster
 */
static void DrawMonster(int x, int y, int mx, int my, int m)
{
	int nCel, frames;
	char trans;
	BYTE *pCelBuff;

	if ((DWORD)m >= MAXMONSTERS) {
		// app_fatal("Draw Monster: tried to draw illegal monster %d", m);
		return;
	}

	pCelBuff = monster[m]._mAnimData;
	if (!pCelBuff) {
		// app_fatal("Draw Monster \"%s\": NULL Cel Buffer", monster[m].mName);
		return;
	}

	nCel = monster[m]._mAnimFrame;
	frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		/*
		const char *szMode = "unknown action";
		if(monster[m]._mmode <= 17)
			szMode = szMonModeAssert[monster[m]._mmode];
		app_fatal(
			"Draw Monster \"%s\" %s: facing %d, frame %d of %d",
			monster[m].mName,
			szMode,
			monster[m]._mdir,
			nCel,
			frames);
		*/
		return;
	}

	if (!(dFlags[x][y] & BFLAG_LIT)) {
		Cl2DrawLightTbl(mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width, 1);
	} else {
		trans = 0;
		if (monster[m]._uniqtype)
			trans = monster[m]._uniqtrans + 4;
		if (monster[m]._mmode == MM_STONE)
			trans = 2;
		if (plr[myplr]._pInfraFlag && light_table_index > 8)
			trans = 1;
		if (trans)
			Cl2DrawLightTbl(mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width, trans);
		else
			Cl2DrawLight(mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width);
	}
}

static void DrawPlayer_SDL(int p, int x, int y, int px, int py)
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
	Render_Texture_FromBottom(px - BORDER_LEFT, py - BORDER_TOP, textureNum, frameNum);
	if (brightness < 255)
		SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);

	//TODO: Render outline (if player is selected) and Mana Shield if it's on
}

/**
 * @brief Render a player sprite
 * @param pnum Player id
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param px Back buffer coordinate
 * @param py Back buffer coordinate
 * @param pCelBuff sprite buffer
 * @param nCel frame
 * @param nWidth width
 */
static void DrawPlayer(int pnum, int x, int y, int px, int py, BYTE *pCelBuff, int nCel, int nWidth)
{
	int l, frames;

	if (dFlags[x][y] & BFLAG_LIT || plr[myplr]._pInfraFlag || !setlevel && currlevel == 0) {
		if (!pCelBuff) {
			// app_fatal("Drawing player %d \"%s\": NULL Cel Buffer", pnum, plr[pnum]._pName);
			return;
		}
		frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
		if (nCel < 1 || frames > 50 || nCel > frames) {
			/*
			const char *szMode = "unknown action";
			if(plr[pnum]._pmode <= PM_QUIT)
				szMode = szPlrModeAssert[plr[pnum]._pmode];
			app_fatal(
				"Drawing player %d \"%s\" %s: facing %d, frame %d of %d",
				pnum,
				plr[pnum]._pName,
				szMode,
				plr[pnum]._pdir,
				nCel,
				frames);
			*/
			return;
		}
		if (pnum == pcursplr)
			Cl2DrawOutline(165, px, py, pCelBuff, nCel, nWidth);
		if (pnum == myplr) {
			Cl2Draw(px, py, pCelBuff, nCel, nWidth);
			if (plr[pnum].pManaShield)
				Cl2Draw(
				    px + plr[pnum]._pAnimWidth2 - misfiledata[MFILE_MANASHLD].mAnimWidth2[0],
				    py,
				    misfiledata[MFILE_MANASHLD].mAnimData[0],
				    1,
				    misfiledata[MFILE_MANASHLD].mAnimWidth[0]);
		} else if (!(dFlags[x][y] & BFLAG_LIT) || plr[myplr]._pInfraFlag && light_table_index > 8) {
			Cl2DrawLightTbl(px, py, pCelBuff, nCel, nWidth, 1);
			if (plr[pnum].pManaShield)
				Cl2DrawLightTbl(
				    px + plr[pnum]._pAnimWidth2 - misfiledata[MFILE_MANASHLD].mAnimWidth2[0],
				    py,
				    misfiledata[MFILE_MANASHLD].mAnimData[0],
				    1,
				    misfiledata[MFILE_MANASHLD].mAnimWidth[0],
				    1);
		} else {
			l = light_table_index;
			if (light_table_index < 5)
				light_table_index = 0;
			else
				light_table_index -= 5;
			Cl2DrawLight(px, py, pCelBuff, nCel, nWidth);
			if (plr[pnum].pManaShield)
				Cl2DrawLight(
				    px + plr[pnum]._pAnimWidth2 - misfiledata[MFILE_MANASHLD].mAnimWidth2[0],
				    py,
				    misfiledata[MFILE_MANASHLD].mAnimData[0],
				    1,
				    misfiledata[MFILE_MANASHLD].mAnimWidth[0]);
			light_table_index = l;
		}
	}
}

/**
 * @brief Render a player sprite
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
void DrawDeadPlayer(int x, int y, int sx, int sy)
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

			if (options_hwRendering)
				DrawPlayer_SDL(i, x, y, px, py);
			else
				DrawPlayer(i, x, y, px, py, p->_pAnimData, p->_pAnimFrame, p->_pAnimWidth);
		}
	}
}

/**
 * @brief Render an object sprite
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param ox Back buffer coordinate
 * @param oy Back buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawObject(int x, int y, int ox, int oy, BOOL pre)
{
	int sx, sy, xx, yy, nCel, frames;
	char bv;
	BYTE *pCelBuff;

	if (dObject[x][y] == 0 || light_table_index >= lightmax)
		return;

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
		xx = object[bv]._ox - x;
		yy = object[bv]._oy - y;
		sx = (xx << 5) + ox - object[bv]._oAnimWidth2 - (yy << 5);
		sy = oy + (yy << 4) + (xx << 4);
	}

	assert((unsigned char)bv < MAXOBJECTS);

	pCelBuff = object[bv]._oAnimData;
	if (!pCelBuff) {
		// app_fatal("Draw Object type %d: NULL Cel Buffer", object[bv]._otype);
		return;
	}

	nCel = object[bv]._oAnimFrame;
	frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > (int)frames) {
		// app_fatal("Draw Object: frame %d of %d, object type==%d", nCel, frames, object[bv]._otype);
		return;
	}

	if (options_hwRendering) {  //Fluffy: Render object via SDL
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
			Render_TextureOutline_FromBottom(sx - BORDER_LEFT, sy - BORDER_TOP, 221, 196, 126, TEXTURE_OBJECTS + AllObjects[objectType].ofindex, nCel - 1);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
		Render_Texture_FromBottom(sx - BORDER_LEFT, sy - BORDER_TOP, TEXTURE_OBJECTS + AllObjects[objectType].ofindex, nCel - 1);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
		if (!object[bv]._oLight)
			SDL_SetTextureBlendMode(textures[textureNum].frames[frameNum].frame, SDL_BLENDMODE_BLEND);
		return;
	}

	if (bv == pcursobj) {
		CelBlitOutline(194, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
		if (options_opaqueWallsWithSilhouette) //Fluffy
			CelDrawToImportant(194, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth, true);
	}
	if (object[bv]._oLight) {
		CelClippedDrawLight(sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
	} else {
		CelClippedDraw(sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
	}
	if (object[bv]._oSelFlag >= 1) {
		if (options_opaqueWallsWithBlobs) //Fluffy
			CelDrawToImportant_Ellipse(sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
		if (options_opaqueWallsWithSilhouette) //Fluffy
			CelDrawToImportant(pLightTbl[(256 * light_table_index) + 198], sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth, false);
	}
}

static void scrollrt_draw_dungeon(int sx, int sy, int dx, int dy);

/**
 * @brief Render a cell
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param importantObjectNaarby Whether or not a player, monster, item, etc is nearby (Fluffy)
 */
static void drawCell(int x, int y, int sx, int sy, bool importantObjectNearby)
{
	BYTE *dst;
	MICROS *pMap;

	dst = &gpBuffer[sx + sy * BUFFER_WIDTH];
	pMap = &dpiece_defs_map_2[x][y];
	level_piece_id = dPiece[x][y]; //This corresponds to "sub-tile" in Diablo 1 graphics tool and, same as the tool, the first tile is 1

	//Fluffy
	int lightx = -1;
	int lighty = -1;
	int lightType = LIGHTING_SUBTILE_NONE;
	if (options_hwRendering && options_lightmapping) {
		if (lightInfo_subTiles && level_piece_id < lightInfo_subTilesSize)
			lightType = lightInfo_subTiles[level_piece_id - 1];

		if (lightType == LIGHTING_SUBTILE_UNIFORM) {
			lightx = lightmap_lightx;
			lighty = lightmap_lighty;
		}
	}

	if (options_opaqueWallsUnlessObscuring && !importantObjectNearby && !options_opaqueWallsWithBlobs && !options_opaqueWallsWithSilhouette) //Fluffy: Make this opaque if there's nothing important nearby
		cel_transparency_active = 0;
	else {
		if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) //Fluffy
			cel_transparency_active = 1;
		else
			cel_transparency_active = (BYTE)(nTransTable[level_piece_id] & TransList[dTransVal[x][y]]);
	}

	cel_foliage_active = !nSolidTable[level_piece_id];
	for (int i = 0; i < (MicroTileLen >> 1); i++) {

		//Fluffy
		int curType = lightType;
		if (options_hwRendering && options_lightmapping) {
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
			if (options_hwRendering) //Fluffy
				RenderTileViaSDL(sx, sy, lightx, lighty, curType);
			else
				RenderTile(dst);
		}

		//Fluffy
		if (options_hwRendering && options_lightmapping) {
			if (lightType == LIGHTING_SUBTILE_DIAGONALFORWARD || lightType == LIGHTING_SUBTILE_MIXEDFOREGROUND) {
				lightx = lightmap_lightx;
				lighty = lightmap_lighty;
				curType = LIGHTING_SUBTILE_DIAGONALFORWARD;
			} else if (lightType == LIGHTING_SUBTILE_DIAGONALBACKWARD || lightType == LIGHTING_SUBTILE_MIXEDBACKGROUND) {
				lightx = lightmap_lightx;
				lighty = lightmap_lighty;
				curType = LIGHTING_SUBTILE_DIAGONALBACKWARD;
			}
		}

		level_cel_block = pMap->mt[2 * i + 1];
		if (level_cel_block != 0) {
			arch_draw_type = i == 0 ? 2 : 0;
			if (options_hwRendering) //Fluffy
				RenderTileViaSDL(sx + TILE_WIDTH / 2, sy, lightx, lighty, curType);
			else
				RenderTile(dst + TILE_WIDTH / 2);
		}
		dst -= BUFFER_WIDTH * TILE_HEIGHT;
		sy -= TILE_HEIGHT; //Fluffy
	}
	cel_foliage_active = false;
}

/**
 * @brief Render a floor tiles
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void drawFloor(int x, int y, int sx, int sy)
{
	cel_transparency_active = 0;
	light_table_index = dLight[x][y];
	if (options_hwRendering && options_lightmapping) //Fluffy: Force brightness to max for lightmapping
		light_table_index = 0;

	BYTE *dst = &gpBuffer[sx + sy * BUFFER_WIDTH];
	arch_draw_type = 1; // Left
	level_cel_block = dpiece_defs_map_2[x][y].mt[0];
	if (level_cel_block != 0) {
		if (options_hwRendering) //Fluffy
			RenderTileViaSDL(sx, sy);
		else
			RenderTile(dst);
	}
	arch_draw_type = 2; // Right
	level_cel_block = dpiece_defs_map_2[x][y].mt[1];
	if (level_cel_block != 0) {
		if (options_hwRendering) //Fluffy
			RenderTileViaSDL(sx + TILE_WIDTH / 2, sy);
		else
			RenderTile(dst + TILE_WIDTH / 2);
	}
}

/**
 * @brief Draw item for a given tile
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawItem(int x, int y, int sx, int sy, BOOL pre)
{
	int nCel;
	char bItem = dItem[x][y];
	ItemStruct *pItem;
	BYTE *pCelBuff;
	DWORD *pFrameTable;

	assert((unsigned char)bItem <= MAXITEMS);

	if (bItem > MAXITEMS || bItem <= 0)
		return;

	pItem = &item[bItem - 1];
	if (pItem->_iPostDraw == pre)
		return;

	pCelBuff = pItem->_iAnimData;
	if (pCelBuff == NULL) {
		// app_fatal("Draw Item \"%s\" 1: NULL Cel Buffer", pItem->_iIName);
		return;
	}
	pFrameTable = (DWORD *)pCelBuff;
	nCel = pItem->_iAnimFrame;
	if (nCel < 1 || pFrameTable[0] > 50 || nCel > (int)pFrameTable[0]) {
		// app_fatal("Draw \"%s\" Item 1: frame %d of %d, item type==%d", pItem->_iIName, nCel, pFrameTable[0], pItem->_itype);
		return;
	}
	int px = sx - pItem->_iAnimWidth2;

	if (options_hwRendering) { //Fluffy: Render item via SDL
		int textureNum = TEXTURE_ITEMS + ItemCAnimTbl[pItem->_iCurs];
		int frameNum = nCel - 1;
		if (bItem - 1 == pcursitem || AutoMapShowItems)
			Render_TextureOutline_FromBottom(px - BORDER_LEFT, sy - BORDER_TOP, 121, 127, 160, textureNum, frameNum);
		int brightness = Render_IndexLightToBrightness();
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
		Render_Texture_FromBottom(px - BORDER_LEFT, sy - BORDER_TOP, textureNum, frameNum);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
		return;
	}

	if (bItem - 1 == pcursitem || AutoMapShowItems) {
		CelBlitOutline(181, px, sy, pCelBuff, nCel, pItem->_iAnimWidth);
		if (options_opaqueWallsWithSilhouette) //Fluffy
			CelDrawToImportant(181, px, sy, pItem->_iAnimData, nCel, pItem->_iAnimWidth, true);
	}
	CelClippedDrawLight(px, sy, pCelBuff, nCel, pItem->_iAnimWidth);
	if (options_opaqueWallsWithBlobs) //Fluffy
		CelDrawToImportant_Ellipse(px, sy, pCelBuff, nCel, pItem->_iAnimWidth);
	if (options_opaqueWallsWithSilhouette) //Fluffy
		CelDrawToImportant(pLightTbl[(256 * light_table_index) + 185], px, sy, pCelBuff, nCel, pItem->_iAnimWidth, false);
}

/**
 * @brief Check if and how a monster should be rendered
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param oy dPiece Y offset
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void DrawMonsterHelper(int x, int y, int oy, int sx, int sy)
{
	int mi, px, py;
	MonsterStruct *pMonster;

	mi = dMonster[x][y + oy];
	mi = mi > 0 ? mi - 1 : -(mi + 1);

	if (leveltype == DTYPE_TOWN) {
		px = sx - towner[mi]._tAnimWidth2;

		if (options_hwRendering) { //Fluffy: Render NPC via SDL
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
			case TOWN_PRIEST:
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
				Render_TextureOutline_FromBottom(px - BORDER_LEFT, sy - BORDER_TOP, 165, 90, 90, textureNum, frameNum);
			int brightness = Render_IndexLightToBrightness();
			if (brightness < 255)
				SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
			Render_Texture_FromBottom(px - BORDER_LEFT, sy - BORDER_TOP, textureNum, frameNum);
			if (brightness < 255)
				SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
			return;
		}
		if (mi == pcursmonst) {
			CelBlitOutline(166, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth);
			if (options_opaqueWallsWithSilhouette) //Fluffy
				CelDrawToImportant(166, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth, true);
		}
		assert(towner[mi]._tAnimData);
		CelClippedDraw(px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth);
		if (options_opaqueWallsWithBlobs) //Fluffy
			CelDrawToImportant_Ellipse(px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth);
		if (options_opaqueWallsWithSilhouette) //Fluffy
			CelDrawToImportant(170, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth, false);
		return;
	}

	if (!(dFlags[x][y] & BFLAG_LIT) && !plr[myplr]._pInfraFlag)
		return;

	if ((DWORD)mi >= MAXMONSTERS) {
		// app_fatal("Draw Monster: tried to draw illegal monster %d", mi);
		return;
	}

	pMonster = &monster[mi];
	if (pMonster->_mFlags & MFLAG_HIDDEN) {
		return;
	}

	if (pMonster->MType == NULL) {
		// app_fatal("Draw Monster \"%s\": uninitialized monster", pMonster->mName);
		return;
	}

	px = sx + pMonster->_mxoff - pMonster->MType->width2;
	py = sy + pMonster->_myoff;

	if (options_hwRendering) { //Fluffy: Render monster via SDL
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
			Render_TextureOutline_FromBottom(px - BORDER_LEFT, py - BORDER_TOP, 147, 30, 30, textureNum, frameNum);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, brightness, brightness, brightness);
		Render_Texture_FromBottom(px - BORDER_LEFT, py - BORDER_TOP, textureNum, frameNum);
		if (brightness < 255)
			SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
		//TODO: Do rendering differently if trans is non-zero
		return;
	}

	if (mi == pcursmonst) {
		Cl2DrawOutline(233, px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width);
		if (options_opaqueWallsWithSilhouette) //Fluffy
			Cl2DrawToImportant(233, px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width, true);
	}
	DrawMonster(x, y, px, py, mi);
	if (options_opaqueWallsWithBlobs) //Fluffy
		Cl2DrawToImportant_Ellipse(px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width);
	if (options_opaqueWallsWithSilhouette) //Fluffy
		Cl2DrawToImportant(pLightTbl[(256 * light_table_index) + 234], px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width, false);
}

/**
 * @brief Check if and how a player should be rendered
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param oy dPiece Y offset
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
static void DrawPlayerHelper(int x, int y, int oy, int sx, int sy)
{
	int p = dPlayer[x][y + oy];
	p = p > 0 ? p - 1 : -(p + 1);

	if ((DWORD)p >= MAX_PLRS) {
		// app_fatal("draw player: tried to draw illegal player %d", p);
		return;
	}

	PlayerStruct *pPlayer = &plr[p];
	int px = sx + pPlayer->_pxoff - pPlayer->_pAnimWidth2;
	int py = sy + pPlayer->_pyoff;


	if (options_hwRendering) { //Fluffy: Render player via SDL
		DrawPlayer_SDL(p, x, y, px, py);
		return;
	}

	DrawPlayer(p, x, y + oy, px, py, pPlayer->_pAnimData, pPlayer->_pAnimFrame, pPlayer->_pAnimWidth);
	if (options_opaqueWallsWithBlobs) //Fluffy
		Cl2DrawToImportant_Ellipse(px, py, pPlayer->_pAnimData, pPlayer->_pAnimFrame, pPlayer->_pAnimWidth);
	else if (options_opaqueWallsWithSilhouette) //Fluffy
		Cl2DrawToImportant(165, px, py, pPlayer->_pAnimData, pPlayer->_pAnimFrame, pPlayer->_pAnimWidth, false);
}

static void RenderArchViaSDL(int x, int y, int archNum, bool transparent)
{
	if (options_lightmapping) {
		bool forward = true;
		if (leveltype == DTYPE_CATHEDRAL && currlevel < 21) { //Cathedral
			if (archNum == 2 || archNum == 3 || archNum == 8)
				forward == false;
		}
		archNum -= 1;

		SDL_Rect srcRect;
		srcRect.x = 0;
		srcRect.y = 0;
		srcRect.w = 1;
		srcRect.h = textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].height;

		SDL_Rect dstRect;
		dstRect.x = x - BORDER_LEFT;
		dstRect.y = (y - BORDER_TOP) - (srcRect.h - 1);
		dstRect.w = 1;
		dstRect.h = srcRect.h;

		if (transparent)
			SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 127);

		int lightx, lighty, brightness;
		if (forward) {
			lightx = lightmap_lightx - (TILE_WIDTH / 2);
			lighty = lightmap_lighty + (TILE_HEIGHT / 2);
		} else {
			//TODO: I think these values should be different to avoid seams
			lightx = lightmap_lightx - (TILE_WIDTH / 2);
			lighty = lightmap_lighty - (TILE_HEIGHT); 
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
		SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 127);
	Render_Texture_FromBottom(x - BORDER_LEFT, y - BORDER_TOP, TEXTURE_DUNGEONTILES_SPECIAL, archNum);
	if (brightness < 255)
		SDL_SetTextureColorMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 255, 255, 255);
	if (transparent)
		SDL_SetTextureAlphaMod(textures[TEXTURE_DUNGEONTILES_SPECIAL].frames[archNum].frame, 255);
}

/**
 * @brief Render object sprites
 * @param sx dPiece coordinate
 * @param sy dPiece coordinate
 * @param dx Back buffer coordinate
 * @param dy Back buffer coordinate
 */
static void scrollrt_draw_dungeon(int sx, int sy, int dx, int dy)
{
	int mi, px, py, nCel, nMon, negMon, frames;
	char bFlag, bDead, bObj, bItem, bPlr, bArch, bMap, negPlr, dd;
	DeadStruct *pDeadGuy;
	BYTE *pCelBuff;
	DWORD *pFrameTable;

	assert((DWORD)sx < MAXDUNX);
	assert((DWORD)sy < MAXDUNY);

	if (dRendered[sx][sy])
		return;
	dRendered[sx][sy] = true;

	light_table_index = dLight[sx][sy];
	if (options_hwRendering && options_lightmapping) { //Fluffy: Set coordinates for light value to use
		lightmap_lightx = (dx + (TILE_WIDTH / 2)) - BORDER_LEFT;
		lightmap_lighty = dy - BORDER_TOP;
		if (lightmap_lightx >= SCREEN_WIDTH)
			lightmap_lightx = SCREEN_WIDTH - 1;
		else if (lightmap_lightx < 0)
			lightmap_lightx = 0;
		if (lightmap_lighty >= SCREEN_HEIGHT)
			lightmap_lighty = SCREEN_HEIGHT - 1;
		else if (lightmap_lighty < 0)
			lightmap_lighty = 0;
	}

	//Fluffy: In case we are to render a wall here, figure out if there's an important object nearby so we know if it should be opaque or not
	bool importantObjectNearby = 0;
	if (options_opaqueWallsUnlessObscuring && !options_opaqueWallsWithBlobs && !options_opaqueWallsWithSilhouette && light_table_index < lightmax) {
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

	drawCell(sx, sy, dx, dy, importantObjectNearby);

	bFlag = dFlags[sx][sy];
	bDead = dDead[sx][sy];
	bMap = dTransVal[sx][sy];

	negMon = 0;
	if (sy > 0) // check for OOB
		negMon = dMonster[sx][sy - 1];

	if (visiondebug && bFlag & BFLAG_LIT) {
		CelClippedDraw(dx, dy, pSquareCel, 1, 64);
	}

	if (MissilePreFlag) {
		DrawMissile(sx, sy, dx, dy, TRUE);
	}

	if (light_table_index < lightmax && bDead != 0) {
		do {
			pDeadGuy = &dead[(bDead & 0x1F) - 1];
			dd = (bDead >> 5) & 7;
			px = dx - pDeadGuy->_deadWidth2;

			if (options_hwRendering) { //Render dead enemy via SDL
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
				Render_Texture_FromBottom(px - BORDER_LEFT, dy - BORDER_TOP, textureNum, frameNum);
				if (brightness < 255)
					SDL_SetTextureColorMod(textures[textureNum].frames[frameNum].frame, 255, 255, 255);
				//TODO: Do rendering differently if pDeadGuy->_deadtrans is non-zero
				break;
			}
			pCelBuff = pDeadGuy->_deadData[dd];
			assert(pCelBuff != NULL);
			if (pCelBuff == NULL)
				break;
			pFrameTable = (DWORD *)pCelBuff;
			nCel = pDeadGuy->_deadFrame;
			if (nCel < 1 || pFrameTable[0] > 50 || nCel > (int)pFrameTable[0]) {
				// app_fatal("Unclipped dead: frame %d of %d, deadnum==%d", nCel, pFrameTable[0], (bDead & 0x1F) - 1);
				break;
			}
			if (pDeadGuy->_deadtrans != 0) {
				Cl2DrawLightTbl(px, dy, pCelBuff, nCel, pDeadGuy->_deadWidth, pDeadGuy->_deadtrans);
			} else {
				Cl2DrawLight(px, dy, pCelBuff, nCel, pDeadGuy->_deadWidth);
			}
		} while (0);
	}
	DrawObject(sx, sy, dx, dy, 1);
	DrawItem(sx, sy, dx, dy, 1);
	if (bFlag & BFLAG_PLAYERLR) {
		assert((DWORD)(sy - 1) < MAXDUNY);
		DrawPlayerHelper(sx, sy, -1, dx, dy);
	}
	if (bFlag & BFLAG_MONSTLR && negMon < 0) {
		DrawMonsterHelper(sx, sy, -1, dx, dy);
	}
	if (bFlag & BFLAG_DEAD_PLAYER) {
		DrawDeadPlayer(sx, sy, dx, dy);
	}
	if (dPlayer[sx][sy] > 0) {
		DrawPlayerHelper(sx, sy, 0, dx, dy);
	}
	if (dMonster[sx][sy] > 0) {
		DrawMonsterHelper(sx, sy, 0, dx, dy);
	}
	DrawMissile(sx, sy, dx, dy, FALSE);
	DrawObject(sx, sy, dx, dy, 0);
	DrawItem(sx, sy, dx, dy, 0);

	if (leveltype != DTYPE_TOWN) {
		bArch = dSpecial[sx][sy];

		if (bArch != 0) {

			if (options_opaqueWallsUnlessObscuring && !options_opaqueWallsWithBlobs && !options_opaqueWallsWithSilhouette && !importantObjectNearby) //Fluffy: Make this opaque if nothing important is nearby
				cel_transparency_active = 0;
			else {
				if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette) //Fluffy
					cel_transparency_active = true;
				else
					cel_transparency_active = TransList[bMap];
			}
#ifdef _DEBUG
			if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000) {
				cel_transparency_active = 0; //Fluffy: Turn transparency off here for debugging
			}
#endif

			if (options_hwRendering) //Fluffy: Render via SDL
				RenderArchViaSDL(dx, dy, bArch, cel_transparency_active != 0);
			else
				CelClippedBlitLightTrans(&gpBuffer[dx + BUFFER_WIDTH * dy], pSpecialCels, bArch, 64);
		}
	} else {
		// Tree leaves should always cover player when entering or leaving the tile,
		// So delay the rendering until after the next row is being drawn.
		// This could probably have been better solved by sprites in screen space.

		if (sx > 0 && sy > 0 && dy > TILE_HEIGHT + SCREEN_Y) {
			bArch = dSpecial[sx - 1][sy - 1];
			if (bArch != 0) {
				if (options_hwRendering) //Fluffy: Render via SDL
					RenderArchViaSDL(dx, dy - TILE_HEIGHT, bArch, 0);
				else
					CelBlitFrame(&gpBuffer[dx + BUFFER_WIDTH * (dy - TILE_HEIGHT)], pSpecialCels, bArch, 64);
			}
		}
	}
}

/**
 * @brief Render a row of tiles
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param rows Number of rows
 * @param columns Tile in a row
 */
static void scrollrt_drawFloor(int x, int y, int sx, int sy, int rows, int columns)
{
	assert(gpBuffer);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (x >= 0 && x < MAXDUNX && y >= 0 && y < MAXDUNY) {
				level_piece_id = dPiece[x][y];
				if (level_piece_id != 0) {
					if (!nSolidTable[level_piece_id])
						drawFloor(x, y, sx, sy);
				} else {
					world_draw_black_tile(sx, sy);
				}
			} else {
				world_draw_black_tile(sx, sy);
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
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param rows Number of rows
 * @param columns Tile in a row
 */
static void scrollrt_draw(int x, int y, int sx, int sy, int rows, int columns)
{
	assert(gpBuffer);

	// Keep evaluating until MicroTiles can't affect screen
	rows += MicroTileLen;
	memset(dRendered, 0, sizeof(dRendered));

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (x >= 0 && x < MAXDUNX && y >= 0 && y < MAXDUNY) {
				if (x + 1 < MAXDUNX && y - 1 >= 0 && sx + TILE_WIDTH <= SCREEN_X + SCREEN_WIDTH) {
					// Render objects behind walls first to prevent sprites, that are moving
					// between tiles, from poking through the walls as they exceed the tile bounds.
					// A proper fix for this would probably be to layout the sceen and render by
					// sprite screen position rather than tile position.
					if (IsWall(x, y) && (IsWall(x + 1, y) || (x > 0 && IsWall(x - 1, y)))) { // Part of a wall aligned on the x-axis
						if (IsWalkable(x + 1, y - 1) && IsWalkable(x, y - 1)) {              // Has walkable area behind it
							scrollrt_draw_dungeon(x + 1, y - 1, sx + TILE_WIDTH, sy);
						}
					}
				}
				if (dPiece[x][y] != 0) {
					scrollrt_draw_dungeon(x, y, sx, sy);
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
static void Zoom()
{
	int wdt = SCREEN_WIDTH / 2;
	int nSrcOff = SCREENXY(SCREEN_WIDTH / 2 - 1, VIEWPORT_HEIGHT / 2 - 1);
	int nDstOff = SCREENXY(SCREEN_WIDTH - 1, VIEWPORT_HEIGHT - 1);

	if (PANELS_COVER) {
		if (chrflag || questlog) {
			wdt >>= 1;
			nSrcOff -= wdt;
		} else if (invflag || sbookflag) {
			wdt >>= 1;
			nSrcOff -= wdt;
			nDstOff -= SPANEL_WIDTH;
		}
	}

	BYTE *src = &gpBuffer[nSrcOff];
	BYTE *dst = &gpBuffer[nDstOff];

	for (int hgt = 0; hgt < VIEWPORT_HEIGHT / 2; hgt++) {
		for (int i = 0; i < wdt; i++) {
			*dst-- = *src;
			*dst-- = *src;
			src--;
		}
		memcpy(dst - BUFFER_WIDTH, dst, wdt * 2 + 1);
		src -= BUFFER_WIDTH - wdt;
		dst -= 2 * (BUFFER_WIDTH - wdt);
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
	if (SCREEN_WIDTH <= PANEL_WIDTH) {
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
		x = SCREEN_WIDTH % TILE_WIDTH;
		y = VIEWPORT_HEIGHT % TILE_HEIGHT;
	} else {
		x = (SCREEN_WIDTH / 2) % TILE_WIDTH;
		y = (VIEWPORT_HEIGHT / 2) % TILE_HEIGHT;
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
	int columns = SCREEN_WIDTH / TILE_WIDTH;
	if (SCREEN_WIDTH % TILE_WIDTH) {
		columns++;
	}
	int rows = VIEWPORT_HEIGHT / TILE_HEIGHT;
	if (VIEWPORT_HEIGHT % TILE_HEIGHT) {
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
 * @param x Center of view in dPiece coordinate
 * @param y Center of view in dPiece coordinate
 */
static void DrawGame(int x, int y)
{
	int sx, sy, columns, rows;

	// Limit rendering to the view area
	if (zoomflag)
		gpBufEnd = &gpBuffer[BUFFER_WIDTH * (VIEWPORT_HEIGHT + SCREEN_Y)];
	else
		gpBufEnd = &gpBuffer[BUFFER_WIDTH * (VIEWPORT_HEIGHT / 2 + SCREEN_Y)];

	// Adjust by player offset and tile grid alignment
	sx = ScrollInfo._sxoff + tileOffsetX + SCREEN_X;
	sy = ScrollInfo._syoff + tileOffsetY + SCREEN_Y;

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

	if (options_opaqueWallsWithBlobs || options_opaqueWallsWithSilhouette)
		memset(gpBuffer_important, 0, BUFFER_WIDTH * BUFFER_HEIGHT); //Fluffy: Reset "important" buffer before drawing stuff

	if (options_hwRendering && options_lightmapping) { //Fluffy: Process all entities with a light source and have them add lights to the lightmap buffer
		Lightmap_MakeLightmap(x, y, sx, sy, rows, columns);
		lightmap_lightx = -1;
		lightmap_lighty = -1;
	}

	scrollrt_drawFloor(x, y, sx, sy, rows, columns); //Render floor tiles

	if (options_hwRendering && options_lightmapping) { //Fluffy: Render lightmap on top of floor tiles

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
			Render_Texture_Crop(0, 0, TEXTURE_LIGHT_FRAMEBUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		//SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame, SDL_BLENDMODE_MOD);
	}

	scrollrt_draw(x, y, sx, sy, rows, columns); //Render walls, ceilings (if lightmapping is off), objects, player, objects and more

	if (options_hwRendering && !zoomflag) { //Fluffy: Scale up the render if we're zooming in. TODO: Implement integer scaling for this?
		SDL_SetRenderTarget(renderer, textures[TEXTURE_TILE_INTERMEDIATE_BIG].frames[0].frame);
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = SCREEN_WIDTH / 2;
		rect.h = SCREEN_HEIGHT / 2;
		SDL_RenderCopy(renderer, texture_intermediate, &rect, NULL);
		SDL_SetRenderTarget(renderer, texture_intermediate);
		SDL_RenderCopy(renderer, textures[TEXTURE_TILE_INTERMEDIATE_BIG].frames[0].frame, NULL, NULL);
	}

#ifdef LIGHTMAP_SUBTILE_EDITOR
	//Fluffy sub-tile editor
	Lightmap_SubtilePreview();
#endif

	// Allow rendering to the whole screen
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * (SCREEN_HEIGHT + SCREEN_Y)];

	if (!zoomflag) {
		Zoom();
	}
}

// DevilutionX extension.
extern void DrawControllerModifierHints();

/**
 * @brief Start rendering of screen, town variation
 * @param StartX Center of view in dPiece coordinate
 * @param StartY Center of view in dPiece coordinate
 */
void DrawView(int StartX, int StartY)
{
	DrawGame(StartX, StartY);
	if (automapflag) {
		DrawAutomap();
	}
	if (stextflag && !qtextflag)
		DrawSText();
	if (invflag) {
		DrawInv();
	} else if (sbookflag) {
		DrawSpellBook();
	}

	DrawDurIcon();

	if (chrflag) {
		DrawChr();
	} else if (questlog) {
		DrawQuestLog();
	}
	if (!chrflag && plr[myplr]._pStatPts != 0 && !spselflag
	    && (!questlog || SCREEN_HEIGHT >= SPANEL_HEIGHT + PANEL_HEIGHT + 74 || SCREEN_WIDTH >= 4 * SPANEL_WIDTH)) {
		DrawLevelUpIcon();
	}
	if (uitemflag) {
		DrawUniqueInfo();
	}
	if (qtextflag) {
		DrawQText();
	}
	if (spselflag) {
		DrawSpellList();
	}
	if (dropGoldFlag) {
		DrawGoldSplit(dropGoldValue);
	}
	if (helpflag) {
		DrawHelp();
	}
	if (msgflag) {
		DrawDiabloMsg();
	}
	if (deathflag) {
		RedBack();
	} else if (PauseMode != 0) {
		gmenu_draw_pause();
	}

	DrawControllerModifierHints();
	DrawPlrMsg();
	gmenu_draw();
	doom_draw();
	DrawInfoBox();
	DrawLifeFlask();
	DrawManaFlask();
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
		SCREEN_X,
		SCREEN_Y,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};
	SDL_FillRect(pal_surface, &SrcRect, 0);

	unlock_buf(3);

	if (options_hwRendering) //Fluffy: Also clear the intermediate texture
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
	if (MouseX > SCREEN_WIDTH - 20) {
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
	if (MouseY > SCREEN_HEIGHT - 20) {
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

static void RenderDebugLine(int *x, int *y, char *line)
{
	PrintGameStr(*x, *y, line, COL_RED);
	*y += 15;
}

/**
 * @brief Display the current average FPS over 1 sec
 */
static void DrawFPS()
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
		RenderDebugLine(&x, &y, String);
		snprintf(String, 100, "gametick delta: %0.2f", frame_gameplayTickDelta);
		RenderDebugLine(&x, &y, String);
		snprintf(String, 100, "render delta: %0.2f ", frame_renderDelta);
		RenderDebugLine(&x, &y, String);

		if (myplr == 0) {
			snprintf(String, 100, "playerXY: %i %i", plr[myplr]._px, plr[myplr]._py);
			RenderDebugLine(&x, &y, String);

			snprintf(String, 100, "playerOffsetXY: %i %i", plr[myplr]._pxoff, plr[myplr]._pyoff);
			RenderDebugLine(&x, &y, String);

			snprintf(String, 100, "cameraXY: %i %i", ViewX, ViewY);
			RenderDebugLine(&x, &y, String);

			snprintf(String, 100, "cameraOffsetXY: %i %i", ScrollInfo._sxoff / gSpeedMod, ScrollInfo._syoff / gSpeedMod);
			RenderDebugLine(&x, &y, String);

			snprintf(String, 100, "mouseXY: %i %i", MouseX, MouseY);
			RenderDebugLine(&x, &y, String);

			if (options_initHwRendering) {
				if (totalTextureSize < 1 << 10)
					snprintf(String, 100, "loadedTextures: %u", totalTextureSize);
				else if (totalTextureSize < (1 << 20) * 10)
					snprintf(String, 100, "loadedTextures: %uKB", totalTextureSize >> 10);
				else if (totalTextureSize < (1 << 30) * 10)
					snprintf(String, 100, "loadedTextures: %uMB", totalTextureSize >> 20);
				else
					snprintf(String, 100, "loadedTextures: %uGB", totalTextureSize >> 30);
				RenderDebugLine(&x, &y, String);
			
				snprintf(String, 100, "hwRendering: %s", options_hwRendering ? "ON" : "OFF");
				RenderDebugLine(&x, &y, String);

				snprintf(String, 100, "lightMapping: %s", options_lightmapping ? "ON" : "OFF");
				RenderDebugLine(&x, &y, String);
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
static void DoBlitScreen(DWORD dwX, DWORD dwY, DWORD dwWdt, DWORD dwHgt)
{
	SDL_Rect SrcRect = {
		dwX + SCREEN_X,
		dwY + SCREEN_Y,
		dwWdt,
		dwHgt,
	};
	SDL_Rect DstRect = {
		dwX,
		dwY,
		dwWdt,
		dwHgt,
	};

	BltFast(&SrcRect, &DstRect);
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
	int ysize;
	DWORD dwTicks;
	BOOL retry;

	ysize = dwHgt;

	if (!gbActive) {
		return;
	}

	assert(ysize >= 0 && ysize <= SCREEN_HEIGHT);

	if (ysize > 0) {
		DoBlitScreen(0, 0, SCREEN_WIDTH, ysize);
	}
	if (ysize < SCREEN_HEIGHT) {
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
			if (gbMaxPlayers > 1) {
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
	int hgt;

	if (force_redraw == 255) {
		force_redraw = 0;
		hgt = SCREEN_HEIGHT;
	} else {
		hgt = 0;
	}

	if (draw_cursor) {
		lock_buf(0);
		scrollrt_draw_cursor_item();
		unlock_buf(0);
	}

	DrawMain(hgt, FALSE, FALSE, FALSE, FALSE, FALSE);

	if (draw_cursor) {
		if (!options_hwRendering) { //Fluffy: Only remove cursor from buffer if we're doing 8-bit rendering
			lock_buf(0);
			scrollrt_draw_cursor_back_buffer();
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
	int hgt;
	BOOL ddsdesc, ctrlPan;

	if (!gbRunGame) {
		return;
	}

	if (options_hwRendering) //Fluffy: Change render target to texture
	{
		if (options_lightmapping) { //Clear lightmap
			SDL_SetRenderTarget(renderer, textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
			SDL_RenderClear(renderer);
		}

		SDL_SetRenderTarget(renderer, texture_intermediate);

		//Clear the render target
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
		SDL_RenderClear(renderer);
	}

	if (SCREEN_WIDTH > PANEL_WIDTH || SCREEN_HEIGHT > VIEWPORT_HEIGHT + PANEL_HEIGHT || force_redraw == 255) {
		drawhpflag = TRUE;
		drawmanaflag = TRUE;
		drawbtnflag = TRUE;
		drawsbarflag = TRUE;
		ddsdesc = FALSE;
		ctrlPan = TRUE;
		hgt = SCREEN_HEIGHT;
	} else {
		ddsdesc = TRUE;
		ctrlPan = FALSE;
		hgt = VIEWPORT_HEIGHT;
	}

	force_redraw = 0;

	lock_buf(0);
	DrawView(ViewX, ViewY);
	if (ctrlPan) {
		DrawCtrlPan();
	}
	if (drawhpflag) {
		UpdateLifeFlask();
	}
	if (drawmanaflag) {
		UpdateManaFlask();
	}
	if (drawbtnflag) {
		DrawCtrlBtns();
	}
	if (drawsbarflag) {
		DrawInvBelt();
	}
	if (talkflag) {
		DrawTalkPan();
		hgt = SCREEN_HEIGHT;
	}
	scrollrt_draw_cursor_item();

	DrawFPS();

	unlock_buf(0);

	DrawMain(hgt, ddsdesc, drawhpflag, drawmanaflag, drawsbarflag, drawbtnflag);

	if (!options_hwRendering) { //Fluffy: Only remove cursor from buffer if we're doing 8-bit rendering
		lock_buf(0);
		scrollrt_draw_cursor_back_buffer();
		unlock_buf(0);
	}

	if (options_hwRendering) //Fluffy: Reset render target
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
