#pragma once

DEVILUTION_BEGIN_NAMESPACE

//#define LIGHTMAP_SUBTILE_EDITOR

enum {
	LIGHTING_SUBTILE_NONE,             //If this is referenced, we don't apply any lightmapping value to tile render
	LIGHTING_SUBTILE_UNIFORM,          //The same light value affects the entire wall evenly
	LIGHTING_SUBTILE_DIAGONALFORWARD,  //Draw columns of lighting starting from bottomleft going to topright
	LIGHTING_SUBTILE_DIAGONALBACKWARD, //Draw columns of lighting starting from topleft going to bottomright
	LIGHTING_SUBTILE_MIXEDFOREGROUND,  //Draw columns of lighting starting from topleft going to bottom and then to topright
	LIGHTING_SUBTILE_MIXEDBACKGROUND,  //Draw columns of lighting starting from bottomleft going to top and then to bottomright
	LIGHTING_SUBTILE_LIGHTMAP, //Straightforwardly apply the lightmap with an offset (this is meant for ceiling tiles)
};

extern unsigned char *lightmap_imgData;
extern int lightmap_lightx;
extern int lightmap_lighty;
extern unsigned char *lightInfo_subTiles;
extern unsigned int lightInfo_subTilesSize;

void Lightmap_UnloadSubtileData();
void Lightmap_SaveSubtileData();
void Lightmap_LoadSubtileData();
void Lightmap_MakeLightmap(int x, int y, int sx, int sy, int rows, int columns);
void Lightmap_RenderCeiling(int x, int y, int sx, int sy, int rows, int columns);

#ifdef LIGHTMAP_SUBTILE_EDITOR
extern int subtileSelection;
void Lightmap_SubtilePreview();
#endif

DEVILUTION_END_NAMESPACE
