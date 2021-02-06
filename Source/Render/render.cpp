//Fluffy: For rendering truecolour textures via SDL

#include "../all.h"
#include "../Textures/textures.h"

DEVILUTION_BEGIN_NAMESPACE

void Render_Texture(int x, int y, int textureNum, int frameNum)
{
	if (!options_32bitRendering)
		return;
	texture_s *texture = &textures[textureNum];
	SDL_Rect dstR;
	dstR.x = x;
	dstR.y = y;
	dstR.w = texture->width;
	dstR.h = texture->height;

	SDL_RenderCopy(renderer, texture->frames[frameNum], NULL, &dstR);
}

void Render_Texture_StartAtY(int x, int y, int yStart, int textureNum, int frameNum)
{
	if (!options_32bitRendering)
		return;
	texture_s *texture = &textures[textureNum];
	SDL_Rect srcR, dstR;
	dstR.x = x;
	dstR.y = y;
	dstR.w = texture->width;
	dstR.h = texture->height - yStart;

	srcR.x = 0;
	srcR.y = yStart;
	srcR.w = texture->width;
	srcR.h = texture->height - yStart;
	SDL_RenderCopy(renderer, texture->frames[frameNum], &srcR, &dstR);
}

DEVILUTION_END_NAMESPACE
