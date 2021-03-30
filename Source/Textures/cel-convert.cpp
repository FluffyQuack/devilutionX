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

static SDL_Surface *mask1 = 0; //Fluffy debug: Mask for modifying textures we're loading

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

static void ConvertOneLine_OneColour(unsigned char *dst, unsigned char colour, int length, bool alpha) //Convert one indexed colour to RGB (or RGBA)
{
	int srcPos = 0;
	int dstPos = 0;
	while (srcPos < length) {
		if (alpha) {
			dst[dstPos] = 255;
			dstPos++;
		}
		if (celConvert_TranslationTable) {
			dst[dstPos + 2] = orig_palette[celConvert_TranslationTable[colour]].r;
			dst[dstPos + 1] = orig_palette[celConvert_TranslationTable[colour]].g;
			dst[dstPos + 0] = orig_palette[celConvert_TranslationTable[colour]].b;
		} else {
			dst[dstPos + 2] = orig_palette[colour].r;
			dst[dstPos + 1] = orig_palette[colour].g;
			dst[dstPos + 0] = orig_palette[colour].b;
		}
		srcPos++;
		dstPos += 3;
	}
}

static void ConvertOneLine(unsigned char *dst, unsigned char *src, int length, bool alpha) //Convert one line of CEL image data to RGB (or RGBA)
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
				//TODO: For now, outline creation is only done for inventory items, and for those it's okay to include shadow pixels
				memset(bufferPtr, 1, width);
				src += width;
				bufferPtr += width;
				/*for (int j = 0; j < width; j++) { //We have to go pixel by pixel here because we want to skip shadow pixels
					if (*src != 0)
						*bufferPtr = 1;
					else
						*bufferPtr = 0;
					src++;
					bufferPtr += 1;
				}*/
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
	totalTextureSize += textureFrame->height * textureFrame->width * textureFrame->channels;

	//TODO: Should we save this as a different format? We only need alpha channel since colour is handled by game code.
}

void Texture_ConvertCEL_MultipleFrames_Outlined_VariableResolution(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights, bool frameHeader)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(textureNum); //Unload if it's already loaded

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
	texture->loaded = true;
	texture->usesAtlas = false;
}

static bool IsThisActuallyType1(unsigned char *src, unsigned char *end) //Check if it's valid to go through this as if it's a type 1 CEL frame
{
	int i = 32, w = 32;
	unsigned char width;
	int linesProcessed = 0;
	while (1) {
		width = *src++;
		if (width == 0 || width == 0x7F || width == 0x80) { //End of data, or lines longer than 127 bytes
			break;
		}
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
	return (/*i == 0 &&*/ linesProcessed == 32);
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

static void ConvertCELtoSDL(textureFrame_s *textureFrame, unsigned char *celData, unsigned int celDataOffsetPos, bool frameHeader, int frameWidth, int frameHeight = -1, int format = CELDATAFORMAT_TYPE1, unsigned char **rtrnImgData = 0)
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

	//Fluffy debug: Use mask for ceiling tiles
	unsigned char *pixels = 0;
	if (mask1) {
		pixels = (unsigned char *)mask1->pixels;

		unsigned int pos = 0;
		while (pos < textureFrame->width * textureFrame->height * textureFrame->channels) {
			if (imgData[pos + 0] > 0) {
				if (pixels[pos + 0] == 0) {
					imgData[pos + 0] = 0;
					imgData[pos + 1] = 0;
					imgData[pos + 2] = 0;
					imgData[pos + 3] = 0;
				} else if (pixels[pos + 3] < 255) {
					imgData[pos + 0] = pixels[pos + 3];
				}
			}
			pos += 4;
		}
	}

	if (rtrnImgData != 0) { //Return image data rather than making an SDL texture
		*rtrnImgData = imgData;
		return;
	}

	//Create SDL texture utilizing converted image data
	textureFrame->frame = SDL_CreateTexture(renderer, textureFrame->channels == 4 ? SDL_PIXELFORMAT_RGBA8888 : SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, textureFrame->width, textureFrame->height);
	if (textureFrame->frame == 0)
		ErrSdl();
	if (SDL_UpdateTexture(textureFrame->frame, NULL, imgData, textureFrame->width * textureFrame->channels) < 0) //TODO: We probably need to make sure the pitch is divisible by 4. Which is easy enough as long the texture is 32-bit
		ErrSdl();
	delete[] imgData;
	if (SDL_SetTextureBlendMode(textureFrame->frame, SDL_BLENDMODE_BLEND) < 0)
		ErrSdl();
	totalTextureSize += textureFrame->height * textureFrame->width * textureFrame->channels;
}

void Texture_ConvertCEL_MultipleFrames_VariableResolution(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights, bool frameHeader)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(textureNum); //Unload if it's already loaded

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
	texture->loaded = true;
	texture->usesAtlas = false;
}

void Texture_ConvertCEL_MultipleFrames(BYTE *celData, int textureNum, int frameWidth, int frameHeight, bool frameHeader, int groupNum)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(textureNum); //Unload if it's already loaded

	//Go through header data to attain total frame count
	unsigned int totalFrameCount = 0;
	unsigned int pos = 0;
	for (int i = 0; i < groupNum; i++) {
		if (groupNum > 1)
			pos = (unsigned int &)celData[i * 4];
		totalFrameCount += (unsigned int &)celData[pos];
	}

	//Create textureFrame_s pointer array
	texture->frames = new textureFrame_s[totalFrameCount];
	texture->frameCount = totalFrameCount;

	//Parse CEL data
	pos = 0;
	unsigned int curFrameNum = 0;
	for (int i = 0; i < groupNum; i++) {
		//Go to CEL
		if (groupNum > 1)
			pos = (unsigned int &)celData[i * 4];

		//Handle CEL
		int frameCount = (unsigned int &)celData[pos];
		unsigned int celDataOffsetPos = 4;
		for (int j = 0; j < frameCount; j++) {
			ConvertCELtoSDL(&texture->frames[curFrameNum], &celData[pos], celDataOffsetPos, frameHeader, frameWidth, -1);
			celDataOffsetPos += 4;
			curFrameNum++;
		}
	}
	texture->loaded = true;
	texture->usesAtlas = false;
}

void Texture_ConvertCEL_SingleFrame(BYTE *celData, int textureNum, int frameWidth)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(textureNum); //Unload if it's already loaded

	//Create textureFrame_s pointer array
	int frameCount = (int &)*celData;
	texture->frames = new textureFrame_s[1];
	texture->frameCount = 1;
	
	//Do the conversion
	textureFrame_s *textureFrame = &texture->frames[0];
	ConvertCELtoSDL(textureFrame, celData, 4, 0, frameWidth);
	texture->loaded = true;
	texture->usesAtlas = false;
}

unsigned char *atlasImgData = 0;
unsigned int atlasCurPosX = 0;
unsigned int atlasCurPosY = 0;
unsigned int atlasSizeX = 0;
unsigned int atlasSizeY = 0;
#define TILE_SIZE 32
static void CopyImgData(unsigned char *from, unsigned char *to, int fromX, int fromY, int fromStride, int toX, int toY, int toStride, int horSize, int verSize)
{
	unsigned int fromPos = ((fromY * fromStride) + fromX) * 4;
	unsigned int toPos = ((toY * toStride) + toX) * 4;
	for (int i = 0; i < verSize; i++) {
		memcpy(&to[toPos], &from[fromPos], horSize * 4);
		fromPos += fromStride * 4;
		toPos += toStride * 4;
	}
}

static void SaveAsTGA(int sizeX, int sizeY, unsigned char *imgData, char *name)
{
	//Create TGA header
	struct TargaHeader {
		BYTE IDLength;
		BYTE ColormapType;
		BYTE ImageType;
		BYTE ColormapSpecification[5];
		WORD XOrigin;
		WORD YOrigin;
		WORD ImageWidth;
		WORD ImageHeight;
		BYTE PixelDepth;
		BYTE ImageDescriptor;
	} tgaHeader;
	memset(&tgaHeader, 0, sizeof(tgaHeader));
	tgaHeader.IDLength = 0;
	tgaHeader.ImageType = 2;
	tgaHeader.ImageWidth = sizeX;
	tgaHeader.ImageHeight = sizeY;
	tgaHeader.PixelDepth = 32;
	tgaHeader.ImageDescriptor = 0x28;

	unsigned char *tgaData = new unsigned char[sizeX * sizeY * 4];
	for (unsigned int i = 0; i < sizeX * sizeY * 4; i += 4) {
		tgaData[i + 3] = imgData[i + 0];
		tgaData[i + 0] = imgData[i + 1];
		tgaData[i + 1] = imgData[i + 2];
		tgaData[i + 2] = imgData[i + 3];
	}

	FILE *file;
	fopen_s(&file, name, "wb");
	fwrite(&tgaHeader, sizeof(TargaHeader), 1, file);
	fwrite(tgaData, 1, sizeX * sizeY * 4, file);
	fclose(file);
	delete[] tgaData;
}

void Texture_ConvertCEL_DungeonTiles(BYTE *celData, int textureNum, int textureNumDungeonPieces, unsigned char *dungeonPieceInfo)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(textureNum); //Unload if it's already loaded

	//Fluffy debug: Load masks
	if (textureNum == TEXTURE_DUNGEONTILES_LEFTFOLIAGE)
		mask1 = IMG_Load("data/textures/tiles/LeftFoliageMask.png");
	else if (textureNum == TEXTURE_DUNGEONTILES_RIGHTFOLIAGE)
		mask1 = IMG_Load("data/textures/tiles/RightFoliageMask.png");
	else if (textureNum == TEXTURE_DUNGEONTILES_LEFTMASK)
		mask1 = IMG_Load("data/textures/tiles/LeftMaskTransparent.png");
	else if (textureNum == TEXTURE_DUNGEONTILES_RIGHTMASK)
		mask1 = IMG_Load("data/textures/tiles/RightMaskTransparent.png");
	else if (textureNum == TEXTURE_DUNGEONTILES_LEFTMASKINVERTED)
		mask1 = IMG_Load("data/textures/tiles/LeftMaskNulls-Invert.png");
	else if (textureNum == TEXTURE_DUNGEONTILES_RIGHTMASKINVERTED)
		mask1 = IMG_Load("data/textures/tiles/RightMaskNulls-Invert-OneRowTaller.png");
	else if (textureNum == TEXTURE_DUNGEONTILES_LEFTMASKOPAQUE)
		mask1 = IMG_Load("data/textures/tiles/LeftMaskNulls.png");
	else if (textureNum == TEXTURE_DUNGEONTILES_RIGHTMASKOPAQUE)
		mask1 = IMG_Load("data/textures/tiles/RightMaskNulls.png");

	//Create textureFrame_s pointer array
	int frameCount = (int &)*celData;
	texture->frames = new textureFrame_s[frameCount];
	texture->frameCount = frameCount;
	unsigned int celDataOffsetPos = 4;
	int width = 32, height = 32; //All frames in a dungeon CELs are 32x32

	//Create image data for compositing texture atlas
	if (atlasImgData == 0) {

		//Calculate resolution needed for atlas
		unsigned long long totalPixels = frameCount * width * height;
		atlasSizeX = (unsigned int) sqrt(totalPixels);
		//Pad resolution so it's divisible by 32
		if (atlasSizeX % TILE_SIZE != 0) {
			atlasSizeX += TILE_SIZE - (atlasSizeX % TILE_SIZE);
		}
		atlasSizeY = atlasSizeX;
		atlasSizeY += TILE_SIZE;

		atlasImgData = new unsigned char[atlasSizeX * atlasSizeY * 4];
		atlasCurPosX = 0;
		atlasCurPosY = 0;
	}

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

		unsigned char *imgData;
		ConvertCELtoSDL(textureFrame, celData, celDataOffsetPos, false, width, height, format, &imgData);
		celDataOffsetPos += 4;

		//Copy image data to texture atlas
		CopyImgData(imgData, atlasImgData, 0, 0, TILE_SIZE, atlasCurPosX, atlasCurPosY, atlasSizeX, TILE_SIZE, TILE_SIZE);
		textureFrame->offsetX = atlasCurPosX;
		textureFrame->offsetY = atlasCurPosY;
		atlasCurPosX += TILE_SIZE;
		if (atlasCurPosX >= atlasSizeX) {
			atlasCurPosX = 0;
			atlasCurPosY += TILE_SIZE;
			if (atlasCurPosY >= atlasSizeY)
				atlasCurPosY = atlasCurPosY;
		}
		delete[] imgData;
	}

	//Debug: Output micro-tile atlas as TGA
	if (mask1 == 0 && 0)
		SaveAsTGA(atlasSizeX, atlasSizeY, atlasImgData, "MicroTiles.tga");

	//Fluffy debug: Unload masks
	if (mask1)
		SDL_FreeSurface(mask1);
	mask1 = 0;

	texture->loaded = true;
	texture->usesAtlas = true;

	//Turn rest of texture atlas into zero data
	unsigned int toPos = ((atlasCurPosY * atlasSizeX) + atlasCurPosX) * 4;
	unsigned int toEnd = atlasSizeX * atlasSizeY * 4;
	memset(&atlasImgData[toPos], 0, toEnd - toPos);

	//Turn atlas into an SDL texture
	texture->frames[0].frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, atlasSizeX, atlasSizeY);
	if (texture->frames[0].frame == 0)
		ErrSdl();
	if (SDL_UpdateTexture(texture->frames[0].frame, NULL, atlasImgData, atlasSizeX * 4) < 0) //TODO: We probably need to make sure the pitch is divisible by 4. Which is easy enough as long the texture is 32-bit
		ErrSdl();
	if (SDL_SetTextureBlendMode(texture->frames[0].frame, SDL_BLENDMODE_BLEND) < 0)
		ErrSdl();
	totalTextureSize += atlasSizeX * atlasSizeY * 4;
	texture->atlasSizeX = atlasSizeX;
	texture->atlasSizeY = atlasSizeY;

	if (dungeonPieceInfo && textureNumDungeonPieces) { //If these are true, then we also create a texture containing all of the tiles as dungeon pieces which are 64x160

		//TODO: The size for hell and town dungeon pieces are different

		//Define basic size info
		width = 64;
		height = 160;
		int frame2Count = 452;
		if (leveltype == DTYPE_TOWN)
			frame2Count = 1257;
		else if (leveltype == DTYPE_CATHEDRAL && currlevel < 21)
			frame2Count = 452;
		else if (leveltype == DTYPE_CATACOMBS)
			frame2Count = 558;
		else if (leveltype == DTYPE_CAVES && currlevel < 17)
			frame2Count = 559;
		else if (leveltype == DTYPE_HELL)
			frame2Count = 455;
		else if (leveltype == DTYPE_CAVES) //Nest
			frame2Count = 605;
		else if (leveltype == DTYPE_CATHEDRAL) //Crypt
			frame2Count = 649;
		frame2Count += 1;

		//Calculate resolution needed for atlas
		unsigned long long totalPixels = frame2Count * width * height;
		unsigned int atlas2SizeX = (unsigned int)sqrt(totalPixels);
		unsigned int atlas2SizeY = atlas2SizeX;
		//Pad atlas resolution so it's divisible by tile resolution
		if (atlas2SizeX % width != 0) {
			atlas2SizeX += width - (atlas2SizeX % width);
		}
		if (atlas2SizeY % height != 0) {
			atlas2SizeY += height - (atlas2SizeY % height);
		}
		atlas2SizeY += height;

		unsigned char *atlas2ImgData = new unsigned char[atlas2SizeX * atlas2SizeY * 4];
		unsigned int atlas2CurPosX = 0;
		unsigned int atlas2CurPosY = 0;
		memset(atlas2ImgData, 0, atlas2SizeX * atlas2SizeY * 4);

		//Texture pointer
		texture_s *texture2 = &textures[textureNumDungeonPieces];
		Texture_UnloadTexture(textureNumDungeonPieces); //Unload if it's already loaded

		//Create textureFrame_s pointer array
		texture2->frames = new textureFrame_s[frame2Count];
		texture2->frameCount = frame2Count;

		int subTileSize = 10;
		if (leveltype == DTYPE_TOWN || leveltype == DTYPE_HELL)
			subTileSize = 16;
		for (int i = 0; i < frame2Count; i++) {
			textureFrame_s *textureFrame = &texture2->frames[i];

			if (i) {
				atlas2CurPosX += width;
				if (atlas2CurPosX >= atlas2SizeX) {
					atlas2CurPosX = 0;
					atlas2CurPosY += height;
					if (atlas2CurPosY >= atlas2SizeY)
						atlas2CurPosY = atlas2CurPosY;
				}
			}

			textureFrame->channels = 4;
			textureFrame->height = height;
			textureFrame->width = width;
			textureFrame->offsetX = atlas2CurPosX;
			textureFrame->offsetY = atlas2CurPosY;

			int curPosY = 0;
			for (int j = 0; j < subTileSize; j++) {
				if (j > 1 && j % 2 == 0)
					curPosY += TILE_SIZE;
				unsigned short piece = (unsigned short &)pLevelPieces[(i * subTileSize * 2) + (j * 2)];
				if (piece != 0) {
					int frame = (piece & 0xFFF) - 1;
					if (frame >= frameCount)
						frame = frame;
					CopyImgData(atlasImgData, atlas2ImgData, texture->frames[frame].offsetX, texture->frames[frame].offsetY, atlasSizeX, atlas2CurPosX + (j % 2 != 0 ? TILE_SIZE : 0), atlas2CurPosY + curPosY, atlas2SizeX, TILE_SIZE, TILE_SIZE);
				}
			}
		}

		//Debug: Output dungeon piece atlas as TGA
		if (0)
			SaveAsTGA(atlas2SizeX, atlas2SizeY, atlas2ImgData, "DungeonPieces.tga");

		//Turn atlas into an SDL texture
		texture2->frames[0].frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, atlas2SizeX, atlas2SizeY);
		if (texture2->frames[0].frame == 0)
			ErrSdl();
		if (SDL_UpdateTexture(texture2->frames[0].frame, NULL, atlas2ImgData, atlas2SizeX * 4) < 0) //TODO: We probably need to make sure the pitch is divisible by 4. Which is easy enough as long the texture is 32-bit
			ErrSdl();
		if (SDL_SetTextureBlendMode(texture2->frames[0].frame, SDL_BLENDMODE_BLEND) < 0)
			ErrSdl();

		texture2->loaded = true;
		texture2->usesAtlas = true;

		totalTextureSize += atlas2SizeX * atlas2SizeY * 4;
		texture2->atlasSizeX = atlas2SizeX;
		texture2->atlasSizeY = atlas2SizeY;
		delete[] atlas2ImgData;
		atlas2ImgData = 0;
	}

	delete[] atlasImgData;
	atlasImgData = 0;
}

static int GetCL2PixelCount(unsigned char *src, unsigned char *dataEnd)
{
	unsigned char width;
	unsigned int totalPixels = 0;
	while (src < dataEnd) {
		width = *src++;
		if (width == 0)
			break;
		else if (width >= 0x01 && width <= 0x7F) {
			width = width;
		} else if (width >= 0x80 && width <= 0xBE) {
			width = 191 - width;
			src += 1;
		} else {
			width = 256 - width;
			src += width;
		}
		totalPixels += width;
	}
	return totalPixels;
}

static void ConvertCL2toSDL(textureFrame_s *textureFrame, unsigned char *celData, unsigned int celDataOffsetPos, unsigned int groupOffset)
{
	//TODO: In order to reduce texture size, we could detect if there are rows/columns of fully transparent pixels along the edges and then crop baed on that

	//Handle offset
	unsigned int offsetStart = (unsigned int &)celData[celDataOffsetPos] + groupOffset;
	celDataOffsetPos += 4;
	unsigned int offsetEnd = (unsigned int &)celData[celDataOffsetPos] + groupOffset;
	unsigned char *src = &celData[offsetStart];

	//Handle frame header and attain frame resolution
	int frameWidth, frameHeight;
	{
		unsigned short offset1 = (unsigned short &)src[0];
		unsigned short offset2 = (unsigned short &)src[2];
		frameWidth = GetCL2PixelCount(&src[offset1], &src[offset2]) / 32;
		src += offset1;
		frameHeight = GetCL2PixelCount(src, &celData[offsetEnd]) / frameWidth;
	}

	//Set texture properties
	textureFrame->width = frameWidth;
	textureFrame->height = frameHeight;
	textureFrame->channels = 4; //TODO: We could figure out if there's any transparency in the CEL, and then store it as 24-bit instead

	//Create buffer
	unsigned char *imgData = new unsigned char[textureFrame->width * textureFrame->height * textureFrame->channels];
	memset(imgData, 0, textureFrame->width * textureFrame->height * textureFrame->channels); //Fluffy debug: Remove when CL2 conversion code is done

	//Write CL2 data to buffer
	unsigned char *dst = &imgData[textureFrame->width * (textureFrame->height - 1) * textureFrame->channels];
	unsigned char *dataEnd = &celData[offsetEnd];
	unsigned char width;
	unsigned int dstWidthPos = 0, rest = 0, type;
	while (src < dataEnd) {
		width = *src++;
		if (width == 0) //End of data
			break;
		else if (width >= 0x01 && width <= 0x7F) { //Transparent pixels (no image data)
			type = 0;
		} else if (width >= 0x80 && width <= 0xBE) { //Pixels of one colour (image data is one byte)
			width = 191 - width;
			type = 1;
		} else { //Pixels of various colours (image data length is the same as line length)
			width = 256 - width;
			type = 2;
		}

		while (width) {
			if (width + dstWidthPos > frameWidth)
				rest = frameWidth - dstWidthPos;
			else
				rest = width;

			if (type == 0) {
				memset(dst, 0, rest * 4);
			} else if (type == 1) {
				ConvertOneLine_OneColour(dst, src[0], rest, true);
			} else if (type == 2) {
				ConvertOneLine(dst, src, rest, true);
				src += rest;
			}
				
			dst += rest * 4;
			dstWidthPos += rest;
			width -= rest;
			if (dstWidthPos == frameWidth) {
				dstWidthPos -= frameWidth;
				dst -= frameWidth * textureFrame->channels * 2;
			}
			assert(dstWidthPos < frameWidth);
		}

		if (type == 1)
			src++;
	}

	//Create SDL texture utilizing converted image data
	textureFrame->frame = SDL_CreateTexture(renderer, textureFrame->channels == 4 ? SDL_PIXELFORMAT_RGBA8888 : SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, textureFrame->width, textureFrame->height);
	if (textureFrame->frame == 0)
		ErrSdl();
	if (SDL_UpdateTexture(textureFrame->frame, NULL, imgData, textureFrame->width * textureFrame->channels) < 0) //TODO: We probably need to make sure the pitch is divisible by 4. Which is easy enough as long the texture is 32-bit
		ErrSdl();
	delete[] imgData;
	if (SDL_SetTextureBlendMode(textureFrame->frame, SDL_BLENDMODE_BLEND) < 0)
		ErrSdl();
	totalTextureSize += textureFrame->height * textureFrame->width * textureFrame->channels;
}

void Texture_ConvertCL2_MultipleFrames(BYTE *celData, int textureNum, int groupNum)
{
	texture_s *texture = &textures[textureNum];
	Texture_UnloadTexture(textureNum); //Unload if it's already loaded

	//Go through header data to attain total frame count
	unsigned int totalFrameCount = 0;
	unsigned int pos = 0;
	for (int i = 0; i < groupNum; i++) {
		if (groupNum > 1)
			pos = (unsigned int &)celData[i * 4];
		totalFrameCount += (unsigned int &)celData[pos];
	}

	//Create textureFrame_s pointer array
	texture->frames = new textureFrame_s[totalFrameCount];
	texture->frameCount = totalFrameCount;

	//Parse CL2 data
	pos = 0;
	unsigned int curFrameNum = 0;
	for (int i = 0; i < groupNum; i++) {
		//Go to CL2
		if (groupNum > 1)
			pos = (unsigned int &) celData[i * 4];

		//Handle CL2
		int frameCount = (unsigned int &)celData[pos];
		for (int j = 0; j < frameCount; j++) {
			ConvertCL2toSDL(&texture->frames[curFrameNum], celData, pos + 4 + (j * 4), pos); //Convert CEL
			curFrameNum++;
		}
	}
	texture->loaded = true;
	texture->usesAtlas = false;
}

DEVILUTION_END_NAMESPACE
