//Fluffy: For loading truecolour textures

#include "../all.h"
#include "textures.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <sdl_image.h>

DEVILUTION_BEGIN_NAMESPACE

texture_s textures[TEXTURE_NUM];

static void UnloadTexture(texture_s *texture) //Unloads all frames for one texture
{
	if (texture->loaded == 0)
		return;
	for (int i = 0; i < texture->frameCount; i++) {
		SDL_DestroyTexture(texture->frames[i]);
	}
	texture->loaded = 0;
	delete[] texture->frames;
}

//Load one texture (a texture can have multiple frames)
static void LoadTexture(int textureNum, char *filePath, int frameCount = 1)
{
	texture_s *texture = &textures[textureNum];
	UnloadTexture(texture); //Unload if it's already loaded

	//Create imgData pointer array
	texture->frames = new SDL_Texture *[frameCount];

	for (int i = 0; i < frameCount; i++) {
		char newPath[MAX_PATH];
		char *pathToUse;
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
			texture->loaded = 1;
			texture->frames[i] = SDL_CreateTextureFromSurface(renderer, loadedSurface);
			if (texture->frames[i] == 0) {
				ErrSdl(); //TODO Quit with proper error message
			}
			if (SDL_SetTextureBlendMode(texture->frames[i], SDL_BLENDMODE_BLEND) < 0)
				ErrSdl();
			if (i == 0) { //Set texture properties if this is first frame
				texture->width = loadedSurface->w;
				texture->height = loadedSurface->h;
				texture->channels = loadedSurface->format->BytesPerPixel;
			} else if (texture->width != loadedSurface->w || texture->height != loadedSurface->h || texture->channels != loadedSurface->format->BytesPerPixel) { //Check if properties of this frame is the same as first frame
				ErrSdl(); //TODO Quit with proper error message
			}
			SDL_FreeSurface(loadedSurface);
		}
	}

	texture->frameCount = frameCount;

	//TODO: Use file loading in Devilution (which tries to load from file system, and then from MPQ)
	/*
	unsigned char *buffer;
	unsigned int fileSize;
	unsigned char *data = stbi_load_from_memory(buffer, fileSize, &texture->width, &texture->height, &texture->channels, 0);
	*/
}

//Load textures
void Textures_Init()
{
	memset(textures, 0, sizeof(texture_s) * TEXTURE_NUM);

	//Load textures
	if (options_animatedUIFlasks) {
		LoadTexture(TEXTURE_HEALTHFLASK, "Data/Textures/UI/Flasks/health.png", 48);
		LoadTexture(TEXTURE_MANAFLASK, "Data/Textures/UI/Flasks/mana.png", 48);
	}
	
}

//Unload all textures
void Textures_Deinit()
{
	for (int i = 0; i < TEXTURE_NUM; i++) {
		UnloadTexture(&textures[i]);
	}
}

DEVILUTION_END_NAMESPACE
