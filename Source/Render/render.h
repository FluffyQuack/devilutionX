#pragma once

DEVILUTION_BEGIN_NAMESPACE

void Render_Reset32BitBuffer();
void Render_TextureTo32BitBuffer(int x, int y, int textureNum, int frameNum = 0);
void Render_TextureTo32BitBuffer_StartAtY(int x, int y, int yStart, int textureNum, int frameNum = 0);

DEVILUTION_END_NAMESPACE
