//Fluffy: For loading truecolour textures

#include "../all.h"
#include "textures.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

DEVILUTION_BEGIN_NAMESPACE

texture_s textures[TEXTURE_NUM];

static void UnloadTexture(texture_s *texture) //Unloads all frames for one texture
{
	if (texture->loaded == 0)
		return;
	for (int i = 0; i < texture->frames; i++) {
		stbi_image_free(texture->imgData[i]);
	}
	texture->loaded = 0;
	delete[] texture->imgData;
}

//Load one texture (a texture can have multiple frames)
static void LoadTexture(int textureNum, char *filePath, int frameCount = 1)
{
	texture_s *texture = &textures[textureNum];
	UnloadTexture(texture); //Unload if it's already loaded

	//Create imgData pointer array
	texture->imgData = new unsigned char *[frameCount];

	for (int i = 0; i < frameCount; i++) {
		char newPath[MAX_PATH];
		char *pathToUse;
		if (frameCount == 1)
			pathToUse = filePath;
		else {
			sprintf_s(newPath, MAX_PATH, "%s_%04i.png", filePath, i + 1); //TODO: This should automatically separate extension from filePath and then append it
			pathToUse = newPath;
		}

		//Load texture file
		int width, height, channels;
		unsigned char *imgData = stbi_load(pathToUse, &width, &height, &channels, 0);

		if (imgData == 0) { //Texture load failed
			//TODO: Quit out with an error message
			texture->loaded = 0;
		} else { //Successful load
			texture->loaded = 1;
			texture->imgData[i] = imgData;
			if (i == 0) { //Set texture properties if this is first frame
				texture->width = width;
				texture->height = height;
				texture->channels = channels;
			} else if (texture->width != width || texture->height != height || texture->channels != channels) { //Check if properties of this frame is the same as first frame
				//TODO: Quit out with an error message because one frame has the wrong resolution or colour depth
			}
		}
	}

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
		//LoadTexture(TEXTURE_HEALTHFLASK, "Data/Textures/UI/Flasks/health_0001.png", 48);
		LoadTexture(TEXTURE_HEALTHFLASK, "Data/Textures/UI/Flasks/health", 48);
		//LoadTexture(TEXTURE_MANAFLASK, "Data/Textures/UI/Flasks/mana_0001.png", 48);
		LoadTexture(TEXTURE_MANAFLASK, "Data/Textures/UI/Flasks/mana", 48);
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
