//Fluffy: For loading truecolour textures

#include "../all.h"
#include "textures.h"
#include "../render/lightmap.h"
#include "../options.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

DEVILUTION_BEGIN_NAMESPACE

//textureAtlas_s *textureAtlases;
texture_s textures[TEXTURE_NUM];
unsigned long long totalTextureSize = 0; //Total size of all texture data loaded into GPU ram

void Texture_UnloadTexture(int textureNum) //Unloads all frames for one texture
{
	texture_s *texture = &textures[textureNum];
	if (texture->loaded == false)
		return;

	if (texture->usesAtlas) { //This is stored as a texture atlas so we handle it differently
		SDL_DestroyTexture(texture->frames[0].frame);
		totalTextureSize -= texture->atlasSizeX * texture->atlasSizeY * 4;
		texture->loaded = false;
		texture->usesAtlas = false;
		delete[] texture->frames;
		return;
	}

	for (int i = 0; i < texture->frameCount; i++) {
		SDL_DestroyTexture(texture->frames[i].frame);
		totalTextureSize -= texture->frames[i].height * texture->frames[i].width * texture->frames[i].channels;
	}
	texture->loaded = false;
	texture->usesAtlas = false;
	delete[] texture->frames;
}

//Load one texture (a texture can have multiple frames)
void Textures_LoadTexture(int textureNum, char *filePath, int frameCount)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(textureNum); //Unload if it's already loaded

	//Create textureFrame_s pointer array
	texture->frames = new textureFrame_s[frameCount];

	for (int i = 0; i < frameCount; i++) {
		char newPath[MAX_PATH];
		char *pathToUse;
		textureFrame_s *textureFrame = &texture->frames[i];
		if (frameCount == 1)
			pathToUse = filePath;
		else {
			//Separate extension
			char pathWithoutExt[MAX_PATH];
			char ext[10];
			int dotPos = 0;
			{
				int pos = 0;
				while (filePath[pos] != 0) {
					if (filePath[pos] == '.')
						dotPos = pos;
					pos++;
				}
			}
			if (dotPos == 0) {
				ErrSdl(); //TODO Quit with proper error message
			}
			memcpy(pathWithoutExt, filePath, dotPos);
			pathWithoutExt[dotPos] = 0;
			strcpy_s(ext, &filePath[dotPos]);

			//New path
			sprintf_s(newPath, MAX_PATH, "%s_%04i%s", pathWithoutExt, i + 1, ext);
			pathToUse = newPath;
			//TODO: Alternate plan for the path creation is for the filePath to replace digits with # (ie, "health_####.png"), and then we dynamically replace instances of # with a number going up by one
		}

		//Load texture file
		int width, height, channels;
		unsigned char *imgData = stbi_load(pathToUse, &width, &height, &channels, 0);
		if (imgData == NULL) { //Texture load failed
			ErrSdl(); //TODO Quit with proper error message
		} else { //Successful load
			if(channels == 4)
				textureFrame->frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, width, height);
			else
				textureFrame->frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STATIC, width, height);
			if (textureFrame->frame == NULL) {
				ErrSdl(); //TODO Quit with proper error message
			}
			if (SDL_UpdateTexture(textureFrame->frame, NULL, imgData, width * channels) < 0)
				ErrSdl(); //TODO Quit with proper error message
			textureFrame->width = width;
			textureFrame->height = height;
			textureFrame->channels = channels;
			textureFrame->offsetX = 0;
			textureFrame->offsetY = 0;
			textureFrame->cropX1 = 0;
			textureFrame->cropX2 = 0;
			textureFrame->cropY1 = 0;
			textureFrame->cropY2 = 0;
			if (SDL_SetTextureBlendMode(textureFrame->frame, SDL_BLENDMODE_BLEND) < 0)
				ErrSdl();
			stbi_image_free(imgData);
			totalTextureSize += textureFrame->height * textureFrame->width * textureFrame->channels;
		}
	}

	texture->loaded = true;
	texture->usesAtlas = false;
	texture->frameCount = frameCount;

	//TODO: Use file loading in Devilution (which tries to load from file system, and then from MPQ)
	/*
	unsigned char *buffer;
	unsigned int fileSize;
	unsigned char *data = stbi_load_from_memory(buffer, fileSize, &texture->width, &texture->height, &texture->channels, 0);
	*/
}

static void GenerateRenderTarget(int textureNum, int x, int y, bool alpha)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(textureNum); //Unload if it's already loaded

	//Create textureFrame_s pointer array
	texture->frames = new textureFrame_s[1];
	textureFrame_s *textureFrame = &texture->frames[0];

	textureFrame->frame = SDL_CreateTexture(renderer, alpha ? SDL_PIXELFORMAT_RGBA8888 : SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, x, y);
	if (textureFrame->frame == 0)
		ErrSdl();
	textureFrame->channels = alpha ? 4 : 3;
	textureFrame->width = x;
	textureFrame->height = y;
	textureFrame->offsetX = 0;
	textureFrame->offsetY = 0;
	textureFrame->cropX1 = 0;
	textureFrame->cropX2 = 0;
	textureFrame->cropY1 = 0;
	textureFrame->cropY2 = 0;
	texture->frameCount = 1;
	texture->loaded = true;
	texture->usesAtlas = false;
	totalTextureSize += textureFrame->height * textureFrame->width * textureFrame->channels;
	if (SDL_SetTextureBlendMode(textureFrame->frame, SDL_BLENDMODE_BLEND) < 0)
		ErrSdl();
}

//Load textures
void Textures_Init()
{
	//TODO: Use this for figuring out maxiumum texture size:
	//https://wiki.libsdl.org/SDL_RendererInfo

	memset(textures, 0, sizeof(texture_s) * TEXTURE_NUM);

	if (!sgOptions.Graphics.bInitHwUIRendering)
		return;

	//Load textures
	if (sgOptions.Graphics.bAnimatedUIFlasks) {
		Textures_LoadTexture(TEXTURE_HEALTHFLASK, "data/textures/ui/flasks/health.png", 48);
		Textures_LoadTexture(TEXTURE_MANAFLASK, "data/textures/ui/flasks/mana.png", 48);
	}

	//Paperdoll textures
	if (sgOptions.Graphics.bPaperdoll) {
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_AXE, "data/textures/ui/paperdoll/Rogue-axe.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_SWORD, "data/textures/ui/paperdoll/Rogue-sword.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_STAFF, "data/textures/ui/paperdoll/Rogue-staff.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_MACE, "data/textures/ui/paperdoll/Rogue-mace.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_BOW_BACK, "data/textures/ui/paperdoll/Rogue-bow-back.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_BASE, "data/textures/ui/paperdoll/Rogue-base.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_TOPLESS, "data/textures/ui/paperdoll/Rogue-topless.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_PANTY, "data/textures/ui/paperdoll/Rogue-panty.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_BASEOUTFIT, "data/textures/ui/paperdoll/Rogue-baseoutfit.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_BASEOUTFIT_DAMAGELAYER1, "data/textures/ui/paperdoll/Rogue-baseoutfit-damagelayer1.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_BASEOUTFIT_DAMAGELAYER2, "data/textures/ui/paperdoll/Rogue-baseoutfit-damagelayer2.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_LEATHEROUTFIT, "data/textures/ui/paperdoll/Rogue-leather.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_LEATHEROUTFIT_DAMAGELAYER1, "data/textures/ui/paperdoll/Rogue-leather-damagelayer1.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_LEATHEROUTFIT_DAMAGELAYER2, "data/textures/ui/paperdoll/Rogue-leather-damagelayer2.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_BELTS, "data/textures/ui/paperdoll/Rogue-belts.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_GLOVES, "data/textures/ui/paperdoll/Rogue-gloves.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_BOOTS, "data/textures/ui/paperdoll/Rogue-boots.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_CHAINMAIL, "data/textures/ui/paperdoll/Rogue-chainmail.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_CHAINMAIL_DAMAGELAYER1, "data/textures/ui/paperdoll/Rogue-chainmail-damagelayer1.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_CHAINMAIL_DAMAGELAYER2, "data/textures/ui/paperdoll/Rogue-chainmail-damagelayer2.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_PLATEMAIL, "data/textures/ui/paperdoll/Rogue-platemail.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_PLATEMAIL_DAMAGELAYER1, "data/textures/ui/paperdoll/Rogue-platemail-damagelayer1.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_PLATEMAIL_DAMAGELAYER2, "data/textures/ui/paperdoll/Rogue-platemail-damagelayer2.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_FACE, "data/textures/ui/paperdoll/Rogue-face.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_LEATHERHELM, "data/textures/ui/paperdoll/Rogue-leatherhelm.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_LEATHERHELM_DAMAGELAYER1, "data/textures/ui/paperdoll/Rogue-leatherhelm-damagelayer1.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_LEATHERHELM_DAMAGELAYER2, "data/textures/ui/paperdoll/Rogue-leatherhelm-damagelayer2.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_CHAINHELM, "data/textures/ui/paperdoll/Rogue-chainhelm.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_CHAINHELM_DAMAGELAYER1, "data/textures/ui/paperdoll/Rogue-chainhelm-damagelayer1.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_CHAINHELM_DAMAGELAYER2, "data/textures/ui/paperdoll/Rogue-chainhelm-damagelayer2.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_PLATEHELM, "data/textures/ui/paperdoll/Rogue-platehelm.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_PLATEHELM_DAMAGELAYER1, "data/textures/ui/paperdoll/Rogue-platehelm-damagelayer1.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_PLATEHELM_DAMAGELAYER2, "data/textures/ui/paperdoll/Rogue-platehelm-damagelayer2.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_ROGUE_BOW_FRONT, "data/textures/ui/paperdoll/Rogue-bow-front.png");
		Textures_LoadTexture(TEXTURE_PAPERDOLL_CENSORED, "data/textures/ui/paperdoll/Rogue-censored.png");
	}

	//Generate alpha masks used during tile rendering. These are all given a custom blending mode
	if (sgOptions.Graphics.bInitHwIngameRendering) {
		SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDOPERATION_ADD); // (dstColor = dstColor; dstAlpha *= srcAlpha)
		Textures_LoadTexture(TEXTURE_TILE_LEFTFOLIAGEMASK, "data/textures/tiles/LeftFoliageMask.png");
		Textures_LoadTexture(TEXTURE_TILE_RIGHTFOLIAGEMASK, "data/textures/tiles/RightFoliageMask.png");
		Textures_LoadTexture(TEXTURE_TILE_LEFTMASK, "data/textures/tiles/LeftMaskTransparent.png");
		Textures_LoadTexture(TEXTURE_TILE_RIGHTMASK, "data/textures/tiles/RightMaskTransparent.png");
		if (sgOptions.Graphics.bInitLightmapping) {
			Textures_LoadTexture(TEXTURE_TILE_LEFTMASKINVERTED_OPAQUE, "data/textures/tiles/LeftMaskNulls-Invert.png");
			Textures_LoadTexture(TEXTURE_TILE_RIGHTMASKINVERTED_OPAQUE, "data/textures/tiles/RightMaskNulls-Invert-OneRowTaller.png");
			Textures_LoadTexture(TEXTURE_TILE_LEFTMASK_OPAQUE, "data/textures/tiles/LeftMaskNulls.png");
			Textures_LoadTexture(TEXTURE_TILE_RIGHTMASK_OPAQUE, "data/textures/tiles/RightMaskNulls.png");

			SDL_SetTextureBlendMode(textures[TEXTURE_TILE_LEFTMASKINVERTED_OPAQUE].frames[0].frame, blendMode);
			SDL_SetTextureBlendMode(textures[TEXTURE_TILE_RIGHTMASKINVERTED_OPAQUE].frames[0].frame, blendMode);
			SDL_SetTextureBlendMode(textures[TEXTURE_TILE_LEFTMASK_OPAQUE].frames[0].frame, blendMode);
			SDL_SetTextureBlendMode(textures[TEXTURE_TILE_RIGHTMASK_OPAQUE].frames[0].frame, blendMode);
		}
		SDL_SetTextureBlendMode(textures[TEXTURE_TILE_LEFTFOLIAGEMASK].frames[0].frame, blendMode);
		SDL_SetTextureBlendMode(textures[TEXTURE_TILE_RIGHTFOLIAGEMASK].frames[0].frame, blendMode);
		SDL_SetTextureBlendMode(textures[TEXTURE_TILE_LEFTMASK].frames[0].frame, blendMode);
		SDL_SetTextureBlendMode(textures[TEXTURE_TILE_RIGHTMASK].frames[0].frame, blendMode);

		if (sgOptions.Graphics.bInitLightmapping) {
			Textures_LoadTexture(TEXTURE_LIGHT_SMOOTHGRADIENT, "data/textures/light-smooth-gradient.png");
			Textures_LoadTexture(TEXTURE_LIGHT_HALFGRADIENT_HALFGREY, "data/textures/light-half-gradient-half-grey.png");
			SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD); //Basically normal blending
			//SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD); //Same as normal additive blending
			//SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_MAXIMUM, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_MAXIMUM);
			//SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
			SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_SMOOTHGRADIENT].frames[0].frame, blendMode);
			SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_HALFGRADIENT_HALFGREY].frames[0].frame, blendMode);
		}

		//Generate tile intermediate render target
		GenerateRenderTarget(TEXTURE_TILE_INTERMEDIATE, 32, 32, true);
		GenerateRenderTarget(TEXTURE_TILE_INTERMEDIATE_PIECE, 64, 256, true); //Most dungeon pieces have a height of 160, but they're 256 in Hell and Town tilesets, so we set this height to that
		GenerateRenderTarget(TEXTURE_TILE_INTERMEDIATE_BIG, gnScreenWidth, gnScreenHeight, true);

		if (sgOptions.Graphics.bInitLightmapping) {
			GenerateRenderTarget(TEXTURE_LIGHT_FRAMEBUFFER, gnScreenWidth + LIGHTMAP_APPEND_X, gnScreenHeight + LIGHTMAP_APPEND_Y, true);
			SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame, SDL_BLENDMODE_MOD);
			lightmap_imgData = new unsigned char[(gnScreenWidth + LIGHTMAP_APPEND_X) * (gnScreenHeight + LIGHTMAP_APPEND_Y) * 4];
			memset(lightmap_imgData, 0, (gnScreenWidth + LIGHTMAP_APPEND_X) * (gnScreenHeight + LIGHTMAP_APPEND_Y) * 4);
		}
	}
}

//Unload all textures
void Textures_Deinit()
{
	for (int i = 0; i < TEXTURE_NUM; i++) {
		Texture_UnloadTexture(i);
	}
	if (lightmap_imgData)
		delete[] lightmap_imgData;
	lightmap_imgData = 0;
}

DEVILUTION_END_NAMESPACE
