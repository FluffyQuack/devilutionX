#pragma once

DEVILUTION_BEGIN_NAMESPACE

extern BYTE *celConvert_TranslationTable;

void Texture_ConvertCEL_MultipleFrames_Outlined_VariableResolution(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights = 0, bool frameHeader = 0);
void Texture_ConvertCEL_MultipleFrames_VariableResolution(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights = 0, bool frameHeader = 0);
void Texture_ConvertCEL_MultipleFrames(BYTE *celData, int textureNum, int frameWidth, int frameHeight = -1, bool frameHeader = 0, int groupNum = 1);
void Texture_ConvertCEL_SingleFrame(BYTE *celData, int textureNum, int frameWidth);
void Texture_ConvertCEL_DungeonTiles(BYTE *celData, int textureNum, int textureNumDungeonPieces = 0, unsigned char *dungeonPieceInfo = 0);
void Texture_ConvertCL2_MultipleFrames(BYTE *celData, int textureNum, int groupNum = 1);

DEVILUTION_END_NAMESPACE
