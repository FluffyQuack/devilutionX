//Fluffy: For loading CEL image data as SDL textures

#include "../all.h"
#include "textures.h"
#include <sdl_image.h>

DEVILUTION_BEGIN_NAMESPACE

BYTE *celConvert_TranslationTable = 0; //This is assumed to point to an array with 256 entries

static int GetCelHeight(unsigned char *src, unsigned char *dataEnd, int frameWidth)
{
	unsigned char width;
	int w = frameWidth;
	int i;
	int height = 0;
	while (src < dataEnd) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				src += width;
			} else {
				width = -(char)width;
			}
			i -= width;
		}
		height++;
	}
	return height;
}

static void ConvertCELtoSDL_Outline(textureFrame_s *textureFrame, unsigned char *celData, unsigned int celDataOffsetPos, bool frameHeader, int frameWidth, int frameHeight = -1)
{
	//TODO: In order to reduce texture size, we could detect if there are rows/columbs of fully transparent pixels along the edges and then crop baed on that

	//Handle offset
	unsigned int offsetStart = (unsigned int &)celData[celDataOffsetPos];
	celDataOffsetPos += 4;
	unsigned int offsetEnd = (unsigned int &)celData[celDataOffsetPos];
	unsigned char *src = &celData[offsetStart];

	//Handle frame header (frame header exist in CELs with multiple frames)
	if (frameHeader) {
		unsigned short skip = (unsigned short &)*src;
		src += skip;
	}

	//If height is unknown, then we calculate it from CEL data
	if (frameHeight == -1)
		frameHeight = GetCelHeight(src, &celData[offsetEnd], frameWidth);

	//Set texture properties
	textureFrame->width = frameWidth + 2;
	textureFrame->height = frameHeight + 2;
	textureFrame->channels = 4; //TODO: We could figure out if there's any transparency in the CEL, and then store it as 24-bit instead

	//Write sprite to buffer (we only care about what pixels are opaque or not, so we don't add the real colour values. We're treating this as a 1-bit buffer)
	BYTE *buffer = new BYTE[frameWidth * frameHeight];
	BYTE *bufferPtr = buffer;
	unsigned char *srcBack = src;
	unsigned char width;
	while (src != &celData[offsetEnd]) {
		for (int i = frameWidth; i;) {
			width = *src++;
			if (!(width & 0x80)) { //Run-length encoding. Positive signed byte means it defines quantity of bytes with image data
				for (int j = 0; j < width; j++) { //We have to go pixel by pixel here because we want to skip shadow pixels
					if (*src != 0)
						*bufferPtr = 1;
					else
						*bufferPtr = 0;
					src++;
					bufferPtr += 1;
				}
			} else { //Negative signed byte means it's defining quantity of skipped pixels
				width = -(char)width;
				memset(bufferPtr, 0, width);
				bufferPtr += width;
			}
			i -= width;
		}
	}
	assert(buffer + (frameWidth * frameHeight) == bufferPtr);
	src = srcBack;

	//Create imgData buffer
	unsigned char *imgData = new unsigned char[textureFrame->width * textureFrame->height * textureFrame->channels];
	memset(imgData, 0, textureFrame->width * textureFrame->height * textureFrame->channels);

	//Write outline into buffer
	unsigned char *dst = &imgData[textureFrame->width * (textureFrame->height - 2) * textureFrame->channels];
	dst += textureFrame->channels;
	unsigned int dstPitch = textureFrame->width * textureFrame->channels;

	//Draw outline
	bufferPtr = buffer;
	unsigned char *bufferEnd = buffer + (frameWidth * frameHeight);
	unsigned char *bufferOneRow = buffer + frameWidth;
	int x = 1, y = textureFrame->height - 2;
	//TODO: We should rewrite this so x and y values aren't necessary. Also, this isn't a precise outline. This is a bulky one
	while (bufferPtr < bufferEnd) {
		for (int i = frameWidth; i;) {
			if (*bufferPtr == 1) {
				if (x < 0 || x >= textureFrame->width || y < 0 || y >= textureFrame->height)
					ErrSdl(); //Quit due to error
				//unsigned char *newDst = &imgData[(textureFrame->width * y * textureFrame->channels) + (x * textureFrame->channels)];
				(int &)imgData[(textureFrame->width * y * textureFrame->channels) + (x * textureFrame->channels)] = 0xFFFFFFFF;
				if (bufferPtr < bufferOneRow || bufferPtr[-frameWidth] == 0)
					(int &)imgData[(textureFrame->width * (y + 1) * textureFrame->channels) + (x * textureFrame->channels)] = 0xFFFFFFFF;
					//(int &)dst[dstPitch] = 0xFFFFFFFF;
				if (i == frameWidth || bufferPtr <= buffer || bufferPtr[-1] == 0)
					(int &)imgData[(textureFrame->width * y * textureFrame->channels) + ((x - 1) * textureFrame->channels)] = 0xFFFFFFFF;
					//(int &)dst[-4] = 0xFFFFFFFF;
				if (i == 1 || bufferPtr + 1 >= bufferEnd || bufferPtr[1] == 0)
					(int &)imgData[(textureFrame->width * y * textureFrame->channels) + ((x + 1) * textureFrame->channels)] = 0xFFFFFFFF;
					//(int &)dst[4] = 0xFFFFFFFF;
				if (bufferPtr + frameWidth >= bufferEnd || bufferPtr[frameWidth] == 0)
					(int &)imgData[(textureFrame->width * (y - 1) * textureFrame->channels) + (x * textureFrame->channels)] = 0xFFFFFFFF;
					//(int &)dst[-dstPitch] = 0xFFFFFFFF;
			}
			bufferPtr += 1;
			dst += 4;
			x += 1;
			i--;
		}
		dst -= dstPitch * 2;
		y -= 1;
		x = 1;
	}
	delete[] buffer;

	//Create SDL texture with converted image data
	textureFrame->frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, textureFrame->width, textureFrame->height);
	if (textureFrame->frame == 0)
		ErrSdl();
	if (SDL_UpdateTexture(textureFrame->frame, NULL, imgData, textureFrame->width * textureFrame->channels) < 0) //TODO: We probably need to make sure the pitch is divisible by 4. Which is easy enough as long the texture is 32-bit
		ErrSdl();
	delete[] imgData;
	if (SDL_SetTextureBlendMode(textureFrame->frame, SDL_BLENDMODE_BLEND) < 0)
		ErrSdl();

	//TODO: Should we save this as a different format? We only need alpha channel since colour is handled by game code.
}

void Texture_ConvertCEL_MultipleFrames_Outlined_VariableResolution(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights, bool frameHeader)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(texture); //Unload if it's already loaded

	//Create textureFrame_s pointer array
	int frameCount = (int &)*celData;
	texture->frames = new textureFrame_s[frameCount];
	texture->frameCount = frameCount;
	unsigned int celDataOffsetPos = 4;
	for (int j = 0; j < frameCount; j++) {

		//Do the conversion
		textureFrame_s *textureFrame = &texture->frames[j];
		ConvertCELtoSDL_Outline(textureFrame, celData, celDataOffsetPos, frameHeader, frameWidths[j], frameHeights[j]);
		celDataOffsetPos += 4;
	}
}

static void ConvertCELtoSDL(textureFrame_s *textureFrame, unsigned char *celData, unsigned int celDataOffsetPos, bool frameHeader, int frameWidth, int frameHeight = -1)
{
	//Handle offset
	unsigned int offsetStart = (unsigned int &)celData[celDataOffsetPos];
	celDataOffsetPos += 4;
	unsigned int offsetEnd = (unsigned int &)celData[celDataOffsetPos];
	unsigned char *src = &celData[offsetStart];

	//Handle frame header (frame header exist in CELs with multiple frames)
	if (frameHeader) {
		unsigned short skip = (unsigned short &)*src;
		src += skip;
	}

	//If height is unknown, then we calculate it from CEL data
	if (frameHeight == -1)
		frameHeight = GetCelHeight(src, &celData[offsetEnd], frameWidth);

	//Set texture properties
	textureFrame->width = frameWidth;
	textureFrame->height = frameHeight;
	textureFrame->channels = 4; //TODO: We could figure out if there's any transparency in the CEL, and then store it as 24-bit instead

	//Create buffer
	unsigned char *imgData = new unsigned char[textureFrame->width * textureFrame->height * textureFrame->channels];

	//Write CEL data into buffer
	unsigned char *dst = &imgData[textureFrame->width * (textureFrame->height - 1) * textureFrame->channels];
	unsigned char width;
	int w = textureFrame->width;
	int i;
	while (src < &celData[offsetEnd]) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				{
					int srcPos = 0;
					int dstPos = 0;
					while (srcPos < width) {
						if (celConvert_TranslationTable) {
							dst[dstPos + 3] = orig_palette[celConvert_TranslationTable[src[srcPos]]].r;
							dst[dstPos + 2] = orig_palette[celConvert_TranslationTable[src[srcPos]]].g;
							dst[dstPos + 1] = orig_palette[celConvert_TranslationTable[src[srcPos]]].b;
						} else {
							dst[dstPos + 3] = orig_palette[src[srcPos]].r;
							dst[dstPos + 2] = orig_palette[src[srcPos]].g;
							dst[dstPos + 1] = orig_palette[src[srcPos]].b;
						}
						dst[dstPos + 0] = 255;
						srcPos++;
						dstPos += 4;
					}
				}
				src += width;
				dst += width * 4;
			} else {
				width = -(char)width;
				memset(dst, 0, width * 4);
				dst += width * 4;
				i -= width;
			}
		}
		dst -= textureFrame->width * textureFrame->channels * 2;
	}

	//Create SDL texture with converted image data
	textureFrame->frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, textureFrame->width, textureFrame->height);
	if (textureFrame->frame == 0)
		ErrSdl();
	if (SDL_UpdateTexture(textureFrame->frame, NULL, imgData, textureFrame->width * textureFrame->channels) < 0) //TODO: We probably need to make sure the pitch is divisible by 4. Which is easy enough as long the texture is 32-bit
		ErrSdl();
	delete[] imgData;
	if (SDL_SetTextureBlendMode(textureFrame->frame, SDL_BLENDMODE_BLEND) < 0)
		ErrSdl();
}

void Texture_ConvertCEL_MultipleFrames_VariableResolution(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights, bool frameHeader)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(texture); //Unload if it's already loaded

	//Create textureFrame_s pointer array
	int frameCount = (int &) *celData;
	texture->frames = new textureFrame_s[frameCount];
	texture->frameCount = frameCount;
	unsigned int celDataOffsetPos = 4;
	for (int j = 0; j < frameCount; j++) {

		//Do the conversion
		textureFrame_s *textureFrame = &texture->frames[j];
		ConvertCELtoSDL(textureFrame, celData, celDataOffsetPos, frameHeader, frameWidths[j], frameHeights ? frameHeights[j] : -1);
		celDataOffsetPos += 4;
	}
}

void Texture_ConvertCEL_MultipleFrames(BYTE *celData, int textureNum, int frameWidth, int frameHeight, bool frameHeader)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(texture); //Unload if it's already loaded

	//Create textureFrame_s pointer array
	int frameCount = (int &)*celData;
	texture->frames = new textureFrame_s[frameCount];
	texture->frameCount = frameCount;
	unsigned int celDataOffsetPos = 4;
	for (int j = 0; j < frameCount; j++) {

		//Do the conversion
		textureFrame_s *textureFrame = &texture->frames[j];
		ConvertCELtoSDL(textureFrame, celData, celDataOffsetPos, frameHeader, frameWidth, frameHeight);
		celDataOffsetPos += 4;
	}
}

void Texture_ConvertCEL_SingleFrame(BYTE *celData, int textureNum, int frameWidth)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(texture); //Unload if it's already loaded

	//Create textureFrame_s pointer array
	int frameCount = (int &)*celData;
	texture->frames = new textureFrame_s[1];
	texture->frameCount = 1;
	
	//Do the conversion
	textureFrame_s *textureFrame = &texture->frames[0];
	ConvertCELtoSDL(textureFrame, celData, 4, 0, frameWidth);
}

DEVILUTION_END_NAMESPACE
