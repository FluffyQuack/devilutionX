//Fluffy: Replacement for a lot of functions in engine.cpp so we can render CEL/CEL2 to the 32-bit buffer

#include "../all.h"
//#include "render.h"
//#include "../Textures/textures.h"

DEVILUTION_BEGIN_NAMESPACE

static void CelBlitSafe_To32BitBuffer(int x, int y, BYTE *pRLEBytes, int nDataSize, int nWidth)
{
	/*
	BYTE *dst = &gpBuffer_32bit[(y * BUFFER_WIDTH * 4) + (x * 4)];
	int i, w;
	BYTE width;
	BYTE *src;
	BYTE *buffEnd = &gpBuffer_32bit[BUFFER_WIDTH * BUFFER_HEIGHT * 4];

	assert(pRLEBytes != NULL);

	src = pRLEBytes;
	w = nWidth;

	for (; src != &pRLEBytes[nDataSize]; dst -= (BUFFER_WIDTH + w) * 4) {
		for (i = w; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst < buffEnd && dst > gpBuffer_32bit) {
					int srcPos = 0;
					int dstPos = 0;
					while (srcPos < width) {
						dst[dstPos + 3] = 255;
						dst[dstPos + 2] = orig_palette[src[srcPos]].b;
						dst[dstPos + 1] = orig_palette[src[srcPos]].g;
						dst[dstPos + 0] = orig_palette[src[srcPos]].r;
						srcPos++;
						dstPos += 4;
					}
				}
				src += width;
				dst += width * 4;
			} else {
				width = -(char)width;
				dst += width * 4;
				i -= width;
			}
		}
	}
	*/
}

void CelClippedDrawSafe_To32BitBuffer(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth)
{
	/*
	sx -= SCREEN_X;
	sy -= SCREEN_Y;
	BYTE *pRLEBytes;
	int nDataSize;

	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);

	CelBlitSafe_To32BitBuffer(
	    sx, sy,
	    pRLEBytes,
	    nDataSize,
	    nWidth);
	*/
}

DEVILUTION_END_NAMESPACE
