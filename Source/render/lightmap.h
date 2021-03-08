#pragma once

DEVILUTION_BEGIN_NAMESPACE

extern unsigned char *lightmap_imgData;
extern int lightmap_lightx;
extern int lightmap_lighty;

void Lightmap_MakeLightmap(int x, int y, int sx, int sy, int rows, int columns);

DEVILUTION_END_NAMESPACE
