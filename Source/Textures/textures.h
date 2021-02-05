#pragma once

DEVILUTION_BEGIN_NAMESPACE

//List of textures
enum {
	TEXTURE_HEALTHFLASK,
	TEXTURE_MANAFLASK,

	TEXTURE_NUM
};

struct texture_s {
	bool loaded; //Whether or not texture is loaded
	unsigned char **imgData; //Array of image datas
	int channels; //Channel count
	int width;
	int height;
	int frames; //Quantity of frames (imgData is an array of this length)
};

extern texture_s textures[TEXTURE_NUM];

void Textures_Init();
void Textures_Deinit();

DEVILUTION_END_NAMESPACE
