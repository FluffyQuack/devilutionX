#pragma once

DEVILUTION_BEGIN_NAMESPACE

void Texture_ConvertCEL_MultipleFrames_Outlined_VariableResolution(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights);
void Texture_ConvertCEL_MultipleFrames_VariableResolution(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights);
void Texture_ConvertCEL_MultipleFrames(BYTE *celData, int textureNum, int frameWidth, int frameHeight = -1);
void Texture_ConvertCEL_SingleFrame(BYTE *celData, int textureNum, int frameWidth);

DEVILUTION_END_NAMESPACE
