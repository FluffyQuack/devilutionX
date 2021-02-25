//Fluffy: For loading CEL image data as SDL textures

#include "../all.h"
#include "textures.h"
#include <sdl_image.h>

DEVILUTION_BEGIN_NAMESPACE

BYTE *celConvert_TranslationTable = 0; //This is assumed to point to an array with 256 entries
enum {
	CELDATAFORMAT_TYPE0,
	CELDATAFORMAT_TYPE1,
	CELDATAFORMAT_TYPE2,
	//CELDATAFORMAT_TYPE3,
	CELDATAFORMAT_TYPE4,
	//CELDATAFORMAT_TYPE5,
};

static int GetCelHeight(unsigned char *src, unsigned char *dataEnd, int frameWidth)
{
	unsigned char width;
	int w = frameWidth;
	int i;
	int height = 0;
	while (src < dataEnd) {
		for (i = w; i;) {
			width = *src++;
			if (width == 0)
				break;
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

static void ConvertOneLine(unsigned char *dst, unsigned char *src, int length, bool alpha)
{
	int srcPos = 0;
	int dstPos = 0;
	while (srcPos < length) {
		if (alpha) {
			dst[dstPos] = 255;
			dstPos++;
		}
		if (celConvert_TranslationTable) {
			dst[dstPos + 2] = orig_palette[celConvert_TranslationTable[src[srcPos]]].r;
			dst[dstPos + 1] = orig_palette[celConvert_TranslationTable[src[srcPos]]].g;
			dst[dstPos + 0] = orig_palette[celConvert_TranslationTable[src[srcPos]]].b;
		} else {
			dst[dstPos + 2] = orig_palette[src[srcPos]].r;
			dst[dstPos + 1] = orig_palette[src[srcPos]].g;
			dst[dstPos + 0] = orig_palette[src[srcPos]].b;
		}
		srcPos++;
		dstPos += 3;
	}
}

static void ConvertCELtoSDL_Outline(textureFrame_s *textureFrame, unsigned char *celData, unsigned int celDataOffsetPos, bool frameHeader, int frameWidth, int frameHeight = -1)
{
	//TODO: In order to reduce texture size, we could detect if there are rows/columns of fully transparent pixels along the edges and then crop baed on that

	//Handle offset
	unsigned int offsetStart = (unsigned int &)celData[celDataOffsetPos];
	celDataOffsetPos += 4;
	unsigned int offsetEnd = (unsigned int &)celData[celDataOffsetPos];
	unsigned char *src = &celData[offsetStart];

	//Handle frame header
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
			if (width == 0) // Indicates end of data. I think this is only used in dungeon tile CELs
				break;
			if (!(width & 0x80)) { //Run-length encoding. Positive signed byte means it's defining quantity of bytes with image data
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
	while (bufferPtr < bufferEnd) {
		for (int i = frameWidth; i;) {
			if (*bufferPtr == 1) {
				if (bufferPtr < bufferOneRow || bufferPtr[-frameWidth] == 0)
					(int &)dst[dstPitch] = 0xFFFFFFFF;
				if (i == frameWidth || bufferPtr <= buffer || bufferPtr[-1] == 0)
					(int &)dst[-4] = 0xFFFFFFFF;
				if (i == 1 || bufferPtr + 1 >= bufferEnd || bufferPtr[1] == 0)
					(int &)dst[4] = 0xFFFFFFFF;
				if (bufferPtr + frameWidth >= bufferEnd || bufferPtr[frameWidth] == 0)
					(int &)dst[-dstPitch] = 0xFFFFFFFF;
			}
			bufferPtr += 1;
			dst += 4;
			i--;
		}
		dst -= dstPitch * 2;
		dst += textureFrame->channels * 2;
	}
	delete[] buffer;

	//Create SDL texture utilizing converted image data
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

static bool IsThisActuallyType1(unsigned char *src, unsigned char *end) //Check if it's valid to go through this as if it's a type 1 CEL frame
{
	int i = 32, w = 32;
	unsigned char width;
	int linesProcessed = 0;
	while (1) {
		width = *src++;
		if (width == 0) { //End of data
			break;
		}
		if (width == 0x7F)
			break;
		if (width == 0x80)
			break;
		if (width & 0x80)
			width = -(char)width;
		else
			src += width;
		i -= width;
		if (i == 0) {
			i = 32;
			linesProcessed++;
		}
		else if (i < 0)
			break;
		if (src >= end)
			break;
	}
	return (i == 0 && linesProcessed == 32);
}

static bool IsThisActuallyType2(unsigned char *src, bool type4, bool *right) //Check for the existance of double nulls. If they don't exist, assume this is type 1
{
	int i;

	unsigned char *srcBack = src;
	*right = false;
	if (src[0] == 0 && src[1] == 0) { //Transparency is on left side
		int size = 2;
		for (i = 0; i < 16; i++) {
			if (i % 2 == 0) {
				if (src[0] != 0 || src[1] != 0)
					goto next;
				src += 2; //Skip double nulls
			}
			src += size;
			if (i < 15)
				size += 2;
		}

		if (type4)
			return true;

		size -= 2;
		for (i = 0; i < 16; i++) {
			if (i % 2 == 0) {
				if (src[0] != 0 || src[1] != 0)
					goto next;
				src += 2; //Skip double nulls
			}
			src += size;
			size -= 2;
		}
		return true;
	}

next:
	src = srcBack;
	*right = true;
	if(src[2] == 0 && src[3] == 0) { //Transparency is on right side
		int size = 2;
		for (i = 0; i < 16; i++) {
			src += size;
			if (i < 15) {
				size += 2;
				if (i % 2 == 0) {
					if (src[0] != 0 || src[1] != 0)
						return false;
					src += 2; //Skip double nulls
				}
			}
		}

		if (type4)
			return true;

		size -= 2;
		for (i = 0; i < 16; i++) {
			src += size;
			size -= 2;
			if (i % 2 == 0) {
				if (src[0] != 0 || src[1] != 0)
					return false;
				src += 2; //Skip double nulls
			}
		}
		return true;
	}
	return false;
}

static void ConvertCELtoSDL(textureFrame_s *textureFrame, unsigned char *celData, unsigned int celDataOffsetPos, bool frameHeader, int frameWidth, int frameHeight = -1, int format = CELDATAFORMAT_TYPE1)
{
	//TODO: In order to reduce texture size, we could detect if there are rows/columns of fully transparent pixels along the edges and then crop baed on that

	//Handle offset
	unsigned int offsetStart = (unsigned int &)celData[celDataOffsetPos];
	celDataOffsetPos += 4;
	unsigned int offsetEnd = (unsigned int &)celData[celDataOffsetPos];
	unsigned char *src = &celData[offsetStart];

	//Handle frame header
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

	//Some CEL frames matching the size of a special type are actually of the ordinary type. In other words, I need to slap the person who created the CEL format
	bool rightSidedTransparency = 0;
	if (format == CELDATAFORMAT_TYPE2 || format == CELDATAFORMAT_TYPE4) {
		if (!IsThisActuallyType2(src, format == CELDATAFORMAT_TYPE4, &rightSidedTransparency)) { //We also get side of transparency for types 2 to 5 here
			format = CELDATAFORMAT_TYPE1;
		}
	} else if (format == CELDATAFORMAT_TYPE0) {
		if (IsThisActuallyType1(src, &celData[offsetEnd])) {
			format = CELDATAFORMAT_TYPE1;
		}
	}

	//Write CEL data into buffer
	unsigned char *dst = &imgData[textureFrame->width * (textureFrame->height - 1) * textureFrame->channels];
	unsigned char width;
	int w = textureFrame->width;
	int i;
	if (format == CELDATAFORMAT_TYPE0) {
		for (int y = 0; y < frameHeight; y++) {
			ConvertOneLine(dst, src, frameWidth, true);
			src += frameWidth;
			dst -= frameWidth * textureFrame->channels;
		}
	} else if (format == CELDATAFORMAT_TYPE1) {
		while (src < &celData[offsetEnd]) {
			for (i = w; i;) {
				width = *src++;
				if (width == 0) // Indicates end of data. I think this is only used in dungeon tile CELs
					break;
				if (!(width & 0x80)) { // width defines quantity of pixels to read
					i -= width;
					ConvertOneLine(dst, src, width, true);
					src += width;
					dst += width * 4;
				} else { // -width defines quantity of pixels to skip
					width = -(char)width;
					memset(dst, 0, width * 4);
					dst += width * 4;
					i -= width;
				}
			}
			dst -= frameWidth * textureFrame->channels * 2;
		}
	} else if (format == CELDATAFORMAT_TYPE2 || format == CELDATAFORMAT_TYPE4) { //Types 2 to 5
		if (!rightSidedTransparency) {
			//Process lower half (type 2 and 4)
			int size = 2;
			for (i = 0; i < 16; i++) {
				if (i % 2 == 0) {
					src += 2; //Skip double nulls
				}
				memset(dst, 0, (32 - size) * textureFrame->channels);
				dst += (32 - size) * textureFrame->channels;
				ConvertOneLine(dst, src, size, true);
				src += size;
				dst += size * textureFrame->channels;
				dst -= frameWidth * textureFrame->channels * 2;
				if (i < 15)
					size += 2;
			}

			if (format == CELDATAFORMAT_TYPE2) { //Process upper half of type 2
				size -= 2;
				for (i = 0; i < 16; i++) {
					if (i % 2 == 0) {
						src += 2; //Skip double nulls
					}
					memset(dst, 0, (32 - size) * textureFrame->channels);
					dst += (32 - size) * textureFrame->channels;
					if (size > 0)
						ConvertOneLine(dst, src, size, true);
					src += size;
					dst += size * textureFrame->channels;
					dst -= frameWidth * textureFrame->channels * 2;
					size -= 2;
				}
			}
		} else { //Transparency is on right side
			//Process lower half (types 3 and 5)
			int size = 2;
			for (i = 0; i < 16; i++) {
				ConvertOneLine(dst, src, size, true);
				src += size;
				dst += size * textureFrame->channels;
				memset(dst, 0, (32 - size) * textureFrame->channels);
				dst += (32 - size) * textureFrame->channels;
				dst -= frameWidth * textureFrame->channels * 2;
				if (i < 15) {
					size += 2;
					if (i % 2 == 0) {
						src += 2; //Skip double nulls
					}
				}
			}

			if (format == CELDATAFORMAT_TYPE2) { //Process upper half of type 3
				size -= 2;
				for (i = 0; i < 16; i++) {
					if (size > 0)
						ConvertOneLine(dst, src, size, true);
					src += size;
					dst += size * textureFrame->channels;
					memset(dst, 0, (32 - size) * textureFrame->channels);
					dst += (32 - size) * textureFrame->channels;
					dst -= frameWidth * textureFrame->channels * 2;
					size -= 2;
					if (i % 2 == 0) {
						src += 2; //Skip double nulls
					}
				}
			}
		}

		if (format == CELDATAFORMAT_TYPE4) { //Upper half of types 4 and 5
			for (int y = 0; y < 16; y++) {
				ConvertOneLine(dst, src, frameWidth, true);
				src += frameWidth;
				dst -= frameWidth * textureFrame->channels;
			}
		}
	}

	//Create SDL texture utilizing converted image data
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

void Texture_ConvertCEL_DungeonTiles(BYTE *celData, int textureNum)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(texture); //Unload if it's already loaded

	//Create textureFrame_s pointer array
	int frameCount = (int &)*celData;
	texture->frames = new textureFrame_s[frameCount];
	texture->frameCount = frameCount;
	unsigned int celDataOffsetPos = 4;
	int width = 32, height = 32; //All frames in a dungeon CELs are 32x32
	for (int j = 0; j < frameCount; j++) {
		//Do the conversion
		textureFrame_s *textureFrame = &texture->frames[j];

		//Figure out CEL size (specific sizes indicate unique encoding, which only happens in dungeon CEL files)
		unsigned int offsetStart = (unsigned int &)celData[celDataOffsetPos];
		unsigned int offsetEnd = (unsigned int &)celData[celDataOffsetPos + 4];
		int size = offsetEnd - offsetStart;
		int format;

		if (size == 0x400) { //Type 0 (upper wall)
			format = CELDATAFORMAT_TYPE0;
		} else if (size == 0x220) { //Type 2 and 3 (floor; diamond shaped)
			format = CELDATAFORMAT_TYPE2;
		} else if (size == 0x320) { //Type 4 and 5 (wall bottom)
			format = CELDATAFORMAT_TYPE4;
		} else { //Type 1 (normal CEL data)
			format = CELDATAFORMAT_TYPE1;
		}

		ConvertCELtoSDL(textureFrame, celData, celDataOffsetPos, false, width, height, format);
		celDataOffsetPos += 4;
	}
}

DEVILUTION_END_NAMESPACE
