#include "../all.h"
#include "../textures/textures.h" //Fluffy: For rendering 32-bit textures
#include "../render/sdl-render.h" //Fluffy: For rendering 32-bit textures
#include "lightmap.h"

DEVILUTION_BEGIN_NAMESPACE

#define SUBTILEDATAVERSION 1
#define SUBTILEDATAMAGIC 'PNDF'

static bool dRendered_lightmap[MAXDUNX][MAXDUNY];
unsigned char *lightmap_imgData = 0; //Same as TEXTURE_LIGHT_FRAMEBUFFER but in system RAM
int lightmap_lightx = 0; //What value to use for current tile we're processing
int lightmap_lighty = 0;
unsigned char *lightInfo_subTiles = 0;
unsigned int lightInfo_subTilesSize = 0;

int Lightmap_ReturnBrightness(int x, int y)
{
	if (x < 0 || y < 0 || x >= (SCREEN_WIDTH + LIGHTMAP_APPEND_X) || y >= (SCREEN_HEIGHT + LIGHTMAP_APPEND_Y))
		return 0;
	return lightmap_imgData[((SCREEN_WIDTH + LIGHTMAP_APPEND_X) * 4 * y) + (4 * x)];
}

void Lightmap_UnloadSubtileData()
{
	if (lightInfo_subTiles)
		delete[] lightInfo_subTiles;
	lightInfo_subTiles = 0;
}

static int LevelTypeNumber()
{
	int num = 0;
	if (leveltype == DTYPE_TOWN)
		num = 0;
	else if (leveltype == DTYPE_CATHEDRAL && currlevel < 21)
		num = 1;
	else if (leveltype == DTYPE_CATACOMBS)
		num = 2;
	else if (leveltype == DTYPE_CAVES && currlevel < 17)
		num = 3;
	else if (leveltype == DTYPE_HELL)
		num = 4;
	else if (leveltype == DTYPE_CAVES) //Nest
		num = 5;
	else if (leveltype == DTYPE_CATHEDRAL) //Crypt
		num = 6;
	return num;
}

void Lightmap_SaveSubtileData()
{
	if (lightInfo_subTiles) {
		FILE *file;
		char path[MAX_PATH];
		sprintf_s(path, MAX_PATH, "l%i.flf", LevelTypeNumber());
		fopen_s(&file, path, "wb");
		if (!file) {
			//TODO: Quit with error
			return;
		}

		int var = SUBTILEDATAMAGIC; //Magic
		fwrite(&var, sizeof(int), 1, file); 
		var = SUBTILEDATAVERSION; //Version
		fwrite(&var, sizeof(int), 1, file);
		fwrite(&lightInfo_subTilesSize, sizeof(int), 1, file);
		fwrite(lightInfo_subTiles, 1, lightInfo_subTilesSize, file);
		fclose(file);
	}
}

void Lightmap_LoadSubtileData()
{
#ifdef LIGHTMAP_SUBTILE_EDITOR
	subtileSelection = 0;
#endif
	bool loadedFromFile = false;

	FILE *file;
	char path[MAX_PATH];
	sprintf_s(path, MAX_PATH, "l%i.flf", LevelTypeNumber());
	fopen_s(&file, path, "rb");

	if (file) {
		int var = 0;
		fread(&var, sizeof(int), 1, file);
		if (var != SUBTILEDATAMAGIC) { //Check magic
			fclose(file);
			goto skipFileLoad;
		}
		fread(&var, sizeof(int), 1, file);
		if (var > SUBTILEDATAVERSION) { //Check version
			fclose(file);
			goto skipFileLoad;
		}

		fread(&lightInfo_subTilesSize, sizeof(int), 1, file);
		lightInfo_subTiles = new unsigned char[lightInfo_subTilesSize];
		fread(lightInfo_subTiles, 1, lightInfo_subTilesSize, file);
		fclose(file);
		loadedFromFile = true;
	}
	skipFileLoad:

	if (loadedFromFile == false) {
		lightInfo_subTilesSize = 452;
		if (leveltype == DTYPE_TOWN)
			lightInfo_subTilesSize = 1257;
		else if (leveltype == DTYPE_CATHEDRAL && currlevel < 21)
			lightInfo_subTilesSize = 452;
		else if (leveltype == DTYPE_CATACOMBS)
			lightInfo_subTilesSize = 558;
		else if (leveltype == DTYPE_CAVES && currlevel < 17)
			lightInfo_subTilesSize = 559;
		else if (leveltype == DTYPE_HELL)
			lightInfo_subTilesSize = 455;
		else if (leveltype == DTYPE_CAVES) //Nest
			lightInfo_subTilesSize = 605;
		else if (leveltype == DTYPE_CATHEDRAL) //Crypt
			lightInfo_subTilesSize = 649;
		lightInfo_subTilesSize += 1;
		lightInfo_subTiles = new unsigned char[lightInfo_subTilesSize];
		memset(lightInfo_subTiles, LIGHTING_SUBTILE_UNIFORM, lightInfo_subTilesSize);
	}
}

static void DrawPlayerLightmap(int x, int y, int oy, int sx, int sy)
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
		if (options_lightmapping) { //Fluffy: Render light for player
			int width = 1024;
			int height = width - (width / 2);

			if (currlevel == 0) {
				width *= 2;
				height *= 2;
			}

			//int lightX = px - (pPlayer->_pAnimWidth / 2);
			//int lightY = py - BORDER_TOP;
			//int lightY = py - (SPANEL_HEIGHT / 2);
			int lightX = px - 23;
			int lightY = py - 171;
			Render_Texture_Scale(lightX - (width / 2), lightY - (height / 2), TEXTURE_LIGHT_HALFGRADIENT_HALFGREY, width, height);
		}
	}
}

static void DrawObjectLightmap(int x, int y, int ox, int oy)
{
	int sx, sy, xx, yy, nCel, frames;
	char bv;

	if (dObject[x][y] == 0)
		return;

	if (dObject[x][y] > 0) {
		bv = dObject[x][y] - 1;
		sx = ox - object[bv]._oAnimWidth2;
		sy = oy;
	} else {
		bv = -(dObject[x][y] + 1);
		xx = object[bv]._ox - x;
		yy = object[bv]._oy - y;
		sx = (xx << 5) + ox - object[bv]._oAnimWidth2 - (yy << 5);
		sy = oy + (yy << 4) + (xx << 4);
	}

	assert((unsigned char)bv < MAXOBJECTS);

	if (options_hwRendering) {

		int lightRadius = 0;
		switch (object[bv]._otype) {
		case OBJ_L1LIGHT:
		case OBJ_SKFIRE:
		case OBJ_CANDLE1:
		case OBJ_CANDLE2:
		case OBJ_BOOKCANDLE:
		case OBJ_BCROSS:
		case OBJ_TBCROSS:
			lightRadius = 5;
			break;
		case OBJ_STORYCANDLE:
			lightRadius = 3;
			break;
		case OBJ_TORCHL:
		case OBJ_TORCHR:
		case OBJ_TORCHL2:
		case OBJ_TORCHR2:
			lightRadius = 8;
			break;
		}
		
		if (options_lightmapping && lightRadius) { //Fluffy: Generate lightmap for light
			int width = 512;
			width = (width * 5) / lightRadius;

			//width += random_(0, 40); //TODO: Add nice flickering for flame objects

			int height = width - (width / 2);
			int lightX = ox - 23;
			int lightY = oy - 171;
			//SDL_SetTextureColorMod(textures[TEXTURE_LIGHT_SMOOTHGRADIENT].frames[0].frame, 255, 214, 173);
			//Render_Texture_Scale(lightX - (width / 2), lightY - (height / 2), TEXTURE_LIGHT_SMOOTHGRADIENT, width, height);
			Render_Texture_Scale(lightX - (width / 2), lightY - (height / 2), TEXTURE_LIGHT_HALFGRADIENT_HALFGREY, width, height);
			//SDL_SetTextureColorMod(textures[TEXTURE_LIGHT_SMOOTHGRADIENT].frames[0].frame, 255, 255, 255);
		}
	}
}

static void ProcessTile(int sx, int sy, int dx, int dy)
{
	int mi, px, py, nCel, nMon, negMon, frames;
	char bFlag, bDead, bObj, bItem, bPlr, bArch, bMap, negPlr, dd;
	DeadStruct *pDeadGuy;
	BYTE *pCelBuff;
	DWORD *pFrameTable;


	assert((DWORD)sx < MAXDUNX);
	assert((DWORD)sy < MAXDUNY);

	if (dRendered_lightmap[sx][sy])
		return;
	dRendered_lightmap[sx][sy] = true;

	bFlag = dFlags[sx][sy];

	DrawObjectLightmap(sx, sy, dx, dy);
	if (bFlag & BFLAG_PLAYERLR) {
		assert((DWORD)(sy - 1) < MAXDUNY);
		DrawPlayerLightmap(sx, sy, -1, dx, dy);
	}
	if (dPlayer[sx][sy] > 0) {
		DrawPlayerLightmap(sx, sy, 0, dx, dy);
	}

	/*
	light_table_index = dLight[sx][sy];
	if (options_hwRendering && options_lightmapping) //Fluffy: Force brightness to max for lightmapping
		light_table_index = 0;

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
	
	DrawItem(sx, sy, dx, dy, 1);
	
	if (bFlag & BFLAG_MONSTLR && negMon < 0) {
		DrawMonsterHelper(sx, sy, -1, dx, dy);
	}
	if (bFlag & BFLAG_DEAD_PLAYER) {
		DrawDeadPlayer(sx, sy, dx, dy);
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
	*/
}

void Lightmap_MakeLightmap(int x, int y, int sx, int sy, int rows, int columns)
{
	//Expand the search of tiles we'll be going through so we can add lights which are out of view (TODO: We should calculate how big this search expansion actually needs to be)
	{
		int expandBy = 10;
		sx -= (TILE_WIDTH / 2) * expandBy;
		sy -= (TILE_HEIGHT / 2) * expandBy;
		x -= expandBy;
		columns += expandBy * 2;
		rows += expandBy * 2;
	}

	SDL_SetRenderTarget(renderer, textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame); //Render target
	memset(dRendered_lightmap, 0, sizeof(dRendered_lightmap));
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (x >= 0 && x < MAXDUNX && y >= 0 && y < MAXDUNY) {
				if (x + 1 < MAXDUNX && y - 1 >= 0 && sx + TILE_WIDTH <= SCREEN_X + SCREEN_WIDTH) {
					// Render objects behind walls first to prevent sprites, that are moving
					// between tiles, from poking through the walls as they exceed the tile bounds.
					// A proper fix for this would probably be to layout the sceen and render by
					// sprite screen position rather than tile position.
					/*if (IsWall(x, y) && (IsWall(x + 1, y) || (x > 0 && IsWall(x - 1, y)))) { // Part of a wall aligned on the x-axis
						if (IsWalkable(x + 1, y - 1) && IsWalkable(x, y - 1)) {              // Has walkable area behind it
							scrollrt_draw_dungeon(x + 1, y - 1, sx + TILE_WIDTH, sy);
						}
					}*/
				}

				ProcessTile(x, y, sx, sy);

				/*if (dPiece[x][y] != 0) {
					scrollrt_draw_dungeon(x, y, sx, sy);
				}*/
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

	SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888, lightmap_imgData, (SCREEN_WIDTH + LIGHTMAP_APPEND_X) * 4); //Read lightmap into system RAM
	SDL_SetRenderTarget(renderer, texture_intermediate); //Revert render target to intermediate texture
}

#ifdef LIGHTMAP_SUBTILE_EDITOR
int subtileSelection = 0;
void Lightmap_SubtilePreview()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect rect;
	rect.x = SCREEN_WIDTH - 160;
	rect.y = 148;
	rect.w = 100;
	rect.h = 210;
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_RenderFillRect(renderer, &rect);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	char *message = "INVALID";
	if (lightInfo_subTiles && subtileSelection < lightInfo_subTilesSize) {
		if (lightInfo_subTiles[subtileSelection] == LIGHTING_SUBTILE_NONE)
			message = "NONE";
		else if (lightInfo_subTiles[subtileSelection] == LIGHTING_SUBTILE_UNIFORM)
			message = "UNIFORM";
		else if (lightInfo_subTiles[subtileSelection] == LIGHTING_SUBTILE_DIAGONALFORWARD)
			message = "DIAGONALF";
		else if (lightInfo_subTiles[subtileSelection] == LIGHTING_SUBTILE_DIAGONALBACKWARD)
			message = "DIAGONALB";
		else if (lightInfo_subTiles[subtileSelection] == LIGHTING_SUBTILE_MIXEDFOREGROUND)
			message = "MIXEDF";
		else if (lightInfo_subTiles[subtileSelection] == LIGHTING_SUBTILE_MIXEDBACKGROUND)
			message = "MIXEDB";
		else if (lightInfo_subTiles[subtileSelection] == LIGHTING_SUBTILE_LIGHTMAP)
			message = "LIGHTMAP";
	}
	PrintGameStr(rect.x + 10, rect.y + 190, message, 255);
	char str[100];
	sprintf_s(str, 100, "%i / %i", subtileSelection + 1, lightInfo_subTilesSize);
	PrintGameStr(rect.x + 10, rect.y + 205, str, 255);

	light_table_index = 0;
	lightmap_lightx = -1;
	lightmap_lighty = -1;
	arch_draw_type = 3;
	int subTileSize = 10;
	if (leveltype == DTYPE_TOWN || leveltype == DTYPE_HELL)
		subTileSize = 16;
	int x = rect.x + 64 + 20;
	int y = rect.y + 200;
	for (int i = 0; i < subTileSize; i++) {
		unsigned short piece = (unsigned short &)pLevelPieces[(subtileSelection * subTileSize * 2) + (i * 2)];
		if (piece != 0) {
			level_cel_block = piece;
			RenderTileViaSDL(x, y, 0, 0, 0);
		}
		if (i % 2 == 0) {
			x += TILE_WIDTH / 2;
		} else {
			x -= TILE_WIDTH / 2;
			y += TILE_HEIGHT;
		}
	}
}
#endif

DEVILUTION_END_NAMESPACE
