#pragma once

DEVILUTION_BEGIN_NAMESPACE

void Texture_ConvertCEL_MultipleFrames_Outlined(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights);
void Texture_ConvertCEL_MultipleFrames(BYTE *celData, int textureNum, int *frameWidths, int *frameHeights);
void Texture_ConvertCEL_SingleFrame(BYTE *celData, int textureNum, int frameWidth);

DEVILUTION_END_NAMESPACE
