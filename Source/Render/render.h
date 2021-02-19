#pragma once

DEVILUTION_BEGIN_NAMESPACE

void Render_Texture(int x, int y, int textureNum, int frameNum = 0);
void Render_Texture_Crop(int x, int y, int textureNum, int startX = -1, int startY = -1, int endX = -1, int endY = -1, int frameNum = 0);

DEVILUTION_END_NAMESPACE