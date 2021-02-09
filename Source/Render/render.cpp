//Fluffy: For rendering truecolour textures via SDL

#include "../all.h"
#include "../Textures/textures.h"

DEVILUTION_BEGIN_NAMESPACE

void Render_Texture(int x, int y, int textureNum, int frameNum)
{
	if (!options_hwRendering)
		return;
	if (textures[textureNum].frameCount <= frameNum) {
		ErrSdl(); //TODO Quit with proper error message
	}
	textureFrame_s *textureFrame = &textures[textureNum].frames[frameNum];
	SDL_Rect dstR;
	dstR.x = x;
	dstR.y = y;
	dstR.w = textureFrame->width;
	dstR.h = textureFrame->height;

	SDL_RenderCopy(renderer, textureFrame->frame, NULL, &dstR);
}

void Render_Texture_StartAtY(int x, int y, int yStart, int textureNum, int frameNum)
{
	if (!options_hwRendering)
		return;
	if (textures[textureNum].frameCount <= frameNum) {
		ErrSdl(); //TODO Quit with proper error message
	}
	textureFrame_s *textureFrame = &textures[textureNum].frames[frameNum];
	SDL_Rect srcR, dstR;
	dstR.x = x;
	dstR.y = y;
	dstR.w = textureFrame->width;
	dstR.h = textureFrame->height - yStart;

	srcR.x = 0;
	srcR.y = yStart;
	srcR.w = textureFrame->width;
	srcR.h = textureFrame->height - yStart;
	SDL_RenderCopy(renderer, textureFrame->frame, &srcR, &dstR);
}

DEVILUTION_END_NAMESPACE
