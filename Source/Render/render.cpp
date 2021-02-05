//Fluffy: For blitting textures to 32-bit back buffer

#include "../all.h"
#include "../Textures/textures.h"

DEVILUTION_BEGIN_NAMESPACE

void Render_Reset32BitBuffer() //Fluffy: Fill 32bit buffer with nothing
{
	memset(gpBuffer_32bit, 0, BUFFER_WIDTH * BUFFER_HEIGHT * 4);
}

static void Render_32bitTextureTo32bitBuffer(int x, int y, BYTE *from, int width, int height)
{
	BYTE *to = &gpBuffer_32bit[(y * BUFFER_WIDTH * 4) + (x * 4)];
	for (int i = 0; i < height; i++) {
		memcpy(to, from, width * 4);
		from += width * 4;
		to += BUFFER_WIDTH * 4;
	}
	//TODO: Handle blending in case "to" already has image data
}

void Render_TextureTo32BitBuffer(int x, int y, int textureNum, int frameNum)
{
	if (!options_32bitRendering)
		return;
	texture_s *texture = &textures[textureNum];
	assert(texture->imgData);
	BYTE *from = texture->imgData[frameNum];
	assert(from);
	if (texture->channels == 4)
		Render_32bitTextureTo32bitBuffer(x, y, from, texture->width, texture->height);
	//TODO: Support 24-bit textures
}

void Render_TextureTo32BitBuffer_StartAtY(int x, int y, int yStart, int textureNum, int frameNum)
{
	if (!options_32bitRendering)
		return;
	texture_s *texture = &textures[textureNum];
	assert(texture->imgData);
	BYTE *from = texture->imgData[frameNum] + (yStart * texture->width * 4);
	assert(from);
	if (texture->channels == 4)
		Render_32bitTextureTo32bitBuffer(x, y, from, texture->width, texture->height - yStart);
	//TODO: Support 24-bit textures
}

DEVILUTION_END_NAMESPACE
