//Fluffy: For loading truecolour textures

#include "../all.h"
#include "textures.h"
#include "../render/lightmap.h"
#include <sdl_image.h>

DEVILUTION_BEGIN_NAMESPACE

//textureAtlas_s *textureAtlases;
texture_s textures[TEXTURE_NUM];
unsigned long long totalTextureSize = 0; //Total size of all texture data loaded into GPU ram

void Texture_UnloadTexture(int textureNum) //Unloads all frames for one texture
{
	texture_s *texture = &textures[textureNum];
	if (texture->loaded == false)
		return;
	for (int i = 0; i < texture->frameCount; i++) {
		SDL_DestroyTexture(texture->frames[i].frame);
		totalTextureSize -= texture->frames[i].height * texture->frames[i].width * texture->frames[i].channels;
	}
	texture->loaded = false;
	delete[] texture->frames;
}

//Load one texture (a texture can have multiple frames)
static void LoadTexture(int textureNum, char *filePath, int frameCount = 1)
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
		SDL_Surface *loadedSurface = IMG_Load(pathToUse);
		//TODO: Use IMG_LoadTexture() instead?

		if (loadedSurface == NULL) { //Texture load failed
			ErrSdl(); //TODO Quit with proper error message
		} else { //Successful load
			textureFrame->frame = SDL_CreateTextureFromSurface(renderer, loadedSurface);
			if (textureFrame->frame == 0) {
				ErrSdl(); //TODO Quit with proper error message
			}
			textureFrame->width = loadedSurface->w;
			textureFrame->height = loadedSurface->h;
			textureFrame->channels = loadedSurface->format->BytesPerPixel;
			if (SDL_SetTextureBlendMode(textureFrame->frame, SDL_BLENDMODE_BLEND) < 0)
				ErrSdl();
			SDL_FreeSurface(loadedSurface);
			totalTextureSize += textureFrame->height * textureFrame->width * textureFrame->channels;
		}
	}

	texture->loaded = true;
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
	texture->frameCount = 1;
	texture->loaded = true;
	totalTextureSize += textureFrame->height * textureFrame->width * textureFrame->channels;
	if (SDL_SetTextureBlendMode(textureFrame->frame, SDL_BLENDMODE_BLEND) < 0)
		ErrSdl();
}

//Load textures
void Textures_Init()
{
	memset(textures, 0, sizeof(texture_s) * TEXTURE_NUM);

	if (!options_initHwRendering)
		return;

	//Load textures
	if (options_animatedUIFlasks) {
		LoadTexture(TEXTURE_HEALTHFLASK, "data/textures/ui/flasks/health.png", 48);
		LoadTexture(TEXTURE_MANAFLASK, "data/textures/ui/flasks/mana.png", 48);
	}

	//Generate alpha masks used during tile rendering. These are all given a custom blending mode
	LoadTexture(TEXTURE_TILE_LEFTFOLIAGEMASK, "data/textures/tiles/LeftFoliageMask.png");
	LoadTexture(TEXTURE_TILE_RIGHTFOLIAGEMASK, "data/textures/tiles/RightFoliageMask.png");
	LoadTexture(TEXTURE_TILE_LEFTMASK, "data/textures/tiles/LeftMaskTransparent.png");
	LoadTexture(TEXTURE_TILE_RIGHTMASK, "data/textures/tiles/RightMaskTransparent.png");
	SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDOPERATION_ADD); // (dstColor = dstColor; dstAlpha *= srcAlpha) 
	SDL_SetTextureBlendMode(textures[TEXTURE_TILE_LEFTFOLIAGEMASK].frames[0].frame, blendMode);
	SDL_SetTextureBlendMode(textures[TEXTURE_TILE_RIGHTFOLIAGEMASK].frames[0].frame, blendMode);
	SDL_SetTextureBlendMode(textures[TEXTURE_TILE_LEFTMASK].frames[0].frame, blendMode);
	SDL_SetTextureBlendMode(textures[TEXTURE_TILE_RIGHTMASK].frames[0].frame, blendMode);

	if (options_initLightmapping) {
		LoadTexture(TEXTURE_LIGHT_SMOOTHGRADIENT, "data/textures/light-smooth-gradient.png");
		LoadTexture(TEXTURE_LIGHT_HALFGRADIENT_HALFGREY, "data/textures/light-half-gradient-half-grey.png");
		SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD); //Basically normal blending
		//SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD); //Same as normal additive blending
		//SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_MAXIMUM, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_MAXIMUM);
		//SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
		SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_SMOOTHGRADIENT].frames[0].frame, blendMode);
		SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_HALFGRADIENT_HALFGREY].frames[0].frame, blendMode);
	}

	//Generate tile intermediate render target
	GenerateRenderTarget(TEXTURE_TILE_INTERMEDIATE, 32, 32, true);
	GenerateRenderTarget(TEXTURE_TILE_INTERMEDIATE_BIG, SCREEN_WIDTH, SCREEN_HEIGHT, true);

	if (options_initLightmapping) {
		GenerateRenderTarget(TEXTURE_LIGHT_FRAMEBUFFER, SCREEN_WIDTH, SCREEN_HEIGHT, true);
		SDL_SetTextureBlendMode(textures[TEXTURE_LIGHT_FRAMEBUFFER].frames[0].frame, SDL_BLENDMODE_MOD);
		lightmap_imgData = new unsigned char[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
		memset(lightmap_imgData, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
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
