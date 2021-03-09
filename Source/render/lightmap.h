#pragma once

DEVILUTION_BEGIN_NAMESPACE

enum {
	LIGHTING_SUBTILE_NONE, //If this is referenced, we don't apply any lightmapping value to tile render
	LIGHTING_SUBTILE_UNIFORM, //The same light value affects the entire wall evenly
	LIGHTING_SUBTILE_DIAGONALFORWARD, //Draw columns of lighting starting from bottomleft going to topright
	LIGHTING_SUBTILE_DIAGONALBACKWARD, //Draw columns of lighting starting from topleft going to bottomright
	LIGHTING_SUBTILE_MIXEDFOREGROUND,  //Draw columns of lighting starting from topleft going to bottom and then to topright
	LIGHTING_SUBTILE_MIXEDBACKGROUND, //Draw columns of lighting starting from bottomleft going to top and then to bottomright
};

extern unsigned char *lightmap_imgData;
extern int lightmap_lightx;
extern int lightmap_lighty;

void Lightmap_MakeLightmap(int x, int y, int sx, int sy, int rows, int columns);

DEVILUTION_END_NAMESPACE
