//Fluffy: For loading truecolour textures

#include "../all.h"
#include "textures.h"
#include <sdl_image.h>

DEVILUTION_BEGIN_NAMESPACE

//textureAtlas_s *textureAtlases;
texture_s textures[TEXTURE_NUM];

void Texture_UnloadTexture(texture_s *texture) //Unloads all frames for one texture
{
	if (texture->loaded == 0)
		return;
	for (int i = 0; i < texture->frameCount; i++) {
		SDL_DestroyTexture(texture->frames[i].frame);
	}
	texture->loaded = 0;
	delete[] texture->frames;
}

//Load one texture (a texture can have multiple frames)
static void LoadTexture(int textureNum, char *filePath, int frameCount = 1)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(texture); //Unload if it's already loaded

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
		}
	}

	texture->loaded = 1;
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
		LoadTexture(TEXTURE_HEALTHFLASK, "data/textures/ui/flasks/health.png", 48);
		LoadTexture(TEXTURE_MANAFLASK, "data/textures/ui/flasks/mana.png", 48);

		//Fluffy: Basic lighting test
		LoadTexture(TEXTURE_LIGHTTEST, "data/textures/light-test.png");
		SDL_SetTextureBlendMode(textures[TEXTURE_LIGHTTEST].frames[0].frame, SDL_BLENDMODE_MOD);
	}
}

//Unload all textures
void Textures_Deinit()
{
	for (int i = 0; i < TEXTURE_NUM; i++) {
		Texture_UnloadTexture(&textures[i]);
	}
}

DEVILUTION_END_NAMESPACE
