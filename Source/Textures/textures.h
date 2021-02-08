#pragma once

DEVILUTION_BEGIN_NAMESPACE

//List of textures
enum {
	//CELs as textures
	TEXTURE_CURSOR, //Mouse cursors and inventory items
	TEXTURE_CURSOR_OUTLINE, //Same as above, but outlined versions
	TEXTURE_CURSOR2, //Mouse cursors and inventory items (Hellfire)
	TEXTURE_CURSOR2_OUTLINE, //Same as above, but outlined versions
	TEXTURE_INVENTORY, //Inventory screen

	//BillieJoe's animated HUD flasks
	TEXTURE_HEALTHFLASK,
	TEXTURE_MANAFLASK,

	TEXTURE_NUM
};

struct textureFrame_s {
	SDL_Texture *frame;
	int width;
	int height;
	int channels;
};

struct texture_s {
	bool loaded; //Whether or not texture is loaded
	textureFrame_s *frames;
	int frameCount; //Quantity of frames (imgData is an array of this length)
};

extern texture_s textures[TEXTURE_NUM];

void Texture_UnloadTexture(texture_s *texture);
void Textures_Init();
void Textures_Deinit();

DEVILUTION_END_NAMESPACE
