//Fluffy: For rendering truecolour textures via SDL

#include "../all.h"
#include "../textures/textures.h"
#include "sdl-render.h"
#include "lightmap.h"

DEVILUTION_BEGIN_NAMESPACE

static unsigned char brightnessValues[16] = {255, 230, 210, 180, 140, 120, 100, 88, 74, 62, 50, 40, 30, 20, 10, 0};

int Render_IndexLightToBrightness()
{
	if (options_lightmapping && lightmap_lightx != -1 && lightmap_lighty != -1)
		return lightmap_imgData[(SCREEN_WIDTH * 4 * lightmap_lighty) + (4 * lightmap_lightx)];
	int brightness;
	brightness = 255 - ((light_table_index * 255) / lightmax);
	if (lightmax == 15) {
		return brightnessValues[light_table_index];
	} else {
		return brightness;
	}
}

//Render texture as a solid colour
void Render_Texture_SolidColor(int x, int y, unsigned char r, unsigned char g, unsigned char b, int textureNum, int frameNum)
{
	if (!options_hwRendering)
		return;
	if (textures[textureNum].frameCount <= frameNum) {
		ErrSdl(); //TODO Quit with proper error message
	}

	//Switch to a intermediate render target and render a solid colour to it
	SDL_SetRenderTarget(renderer, textures[TEXTURE_TILE_INTERMEDIATE_BIG].frames[0].frame);
	SDL_SetRenderDrawColor(renderer, r, g, b, 0);
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = textures[textureNum].frames[frameNum].width;
	rect.h = textures[textureNum].frames[frameNum].height;
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_RenderFillRect(renderer, &rect);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	//Create alpha
	SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
	SDL_SetTextureBlendMode(textures[textureNum].frames[frameNum].frame, blendMode);
	Render_Texture(0, 0, textureNum, frameNum);
	SDL_SetTextureBlendMode(textures[textureNum].frames[frameNum].frame, SDL_BLENDMODE_BLEND);

	//Switch render target back to intermediate texture and render generated texture
	SDL_SetRenderTarget(renderer, texture_intermediate);
	Render_Texture_Crop(x, y, TEXTURE_TILE_INTERMEDIATE_BIG, 0, 0, rect.w, rect.h);
}

void Render_TextureOutline_FromBottom(int x, int y, unsigned char r, unsigned char g, unsigned char b, int textureNum, int frameNum)
{
	Render_TextureOutline(x, y - textures[textureNum].frames[frameNum].height + 1, r, g, b, textureNum, frameNum);
}

//Render texture as an outline (this assumes the normal version of the texture is rendered afterwards)
void Render_TextureOutline(int x, int y, unsigned char r, unsigned char g, unsigned char b, int textureNum, int frameNum)
{
	if (!options_hwRendering)
		return;
	if (textures[textureNum].frameCount <= frameNum) {
		ErrSdl(); //TODO Quit with proper error message
	}

	//Switch to a intermediate render target and render a solid colour to it
	SDL_SetRenderTarget(renderer, textures[TEXTURE_TILE_INTERMEDIATE_BIG].frames[0].frame);
	SDL_SetRenderDrawColor(renderer, r, g, b, 0);
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = textures[textureNum].frames[frameNum].width + 2;
	rect.h = textures[textureNum].frames[frameNum].height + 2;
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_RenderFillRect(renderer, &rect);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	//Create thicc alpha by rendering frame 4 times in different directions
	SDL_BlendMode blendMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
	SDL_SetTextureBlendMode(textures[textureNum].frames[frameNum].frame, blendMode);
	Render_Texture(1, 0, textureNum, frameNum);
	Render_Texture(0, 1, textureNum, frameNum);
	Render_Texture(2, 1, textureNum, frameNum);
	Render_Texture(1, 2, textureNum, frameNum);
	SDL_SetTextureBlendMode(textures[textureNum].frames[frameNum].frame, SDL_BLENDMODE_BLEND);

	//Switch render target back to intermediate texture and render the outline we created
	SDL_SetRenderTarget(renderer, texture_intermediate);
	Render_Texture_Crop(x - 1, y - 1, TEXTURE_TILE_INTERMEDIATE_BIG, 0, 0, rect.w, rect.h);
}

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

//Same as Render_Texture() but anchor point is bottomleft rather than topright (this is how most stuff is rendered in original Diablo code)
void Render_Texture_FromBottom(int x, int y, int textureNum, int frameNum)
{
	Render_Texture(x, y - textures[textureNum].frames[frameNum].height + 1, textureNum, frameNum);
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
