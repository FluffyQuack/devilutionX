//Fluffy: For rendering truecolour textures via SDL

#include "../all.h"
#include "../Textures/textures.h"

DEVILUTION_BEGIN_NAMESPACE

void Render_Texture_ScaleAndCrop(int x, int y, int textureNum, int width, int height, int startX, int startY, int endX, int endY, int frameNum)
{
	if (!options_hwRendering)
		return;

	if (textures[textureNum].frameCount <= frameNum) {
		ErrSdl(); //TODO Quit with proper error message
	}
	textureFrame_s *textureFrame = &textures[textureNum].frames[frameNum];
	if (startX == -1)
		startX = 0;
	if (startY == -1)
		startY = 0;
	if (endX == -1)
		endX = textureFrame->width;
	if (endY == -1)
		endY = textureFrame->height;
	SDL_Rect srcR, dstR;
	dstR.x = x;
	dstR.y = y;
	dstR.w = width;
	dstR.h = height;

	srcR.x = startX;
	srcR.y = startY;
	srcR.w = endX - startX;
	srcR.h = endY - startY;
	SDL_RenderCopy(renderer, textureFrame->frame, &srcR, &dstR);
}

void Render_Texture_Scale(int x, int y, int textureNum, int width, int height, int frameNum)
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
	dstR.w = width;
	dstR.h = height;

	SDL_RenderCopy(renderer, textureFrame->frame, NULL, &dstR);
}

void Render_Texture_Crop(int x, int y, int textureNum, int startX, int startY, int endX, int endY, int frameNum)
{
	if (!options_hwRendering)
		return;
	
	if (textures[textureNum].frameCount <= frameNum) {
		ErrSdl(); //TODO Quit with proper error message
	}
	textureFrame_s *textureFrame = &textures[textureNum].frames[frameNum];
	if (startX == -1) startX = 0;
	if (startY == -1) startY = 0;
	if (endX == -1) endX = textureFrame->width;
	if (endY == -1) endY = textureFrame->height;
	SDL_Rect srcR, dstR;
	dstR.x = x;
	dstR.y = y;
	dstR.w = endX - startX;
	dstR.h = endY - startY;

	srcR.x = startX;
	srcR.y = startY;
	srcR.w = endX - startX;
	srcR.h = endY - startY;
	SDL_RenderCopy(renderer, textureFrame->frame, &srcR, &dstR);
}

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

DEVILUTION_END_NAMESPACE
