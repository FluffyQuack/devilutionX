#include "../all.h"
#include "../textures/textures.h"
#include "../render/sdl-render.h"

DEVILUTION_BEGIN_NAMESPACE

#define PAPERDOLLX 86
#define PAPERDOLLY 3

void Paperdoll_Render(int x, int y)
{
	if (!options_hwUIRendering || plr[myplr]._pClass != PC_ROGUE)
		return;

	int textureNum, textureNum2 = -1, opacity = 255;
	if (plr[myplr].InvBody[INVLOC_CHEST].isEmpty())
		textureNum = TEXTURE_ROGUE_NOTHING;
	else {
		ItemStruct *item = &plr[myplr].InvBody[INVLOC_CHEST];
		if (item->_itype == ITYPE_LARMOR)
			textureNum = TEXTURE_ROGUE_LIGHT2;
		else if (item->_itype == ITYPE_MARMOR)
			textureNum = TEXTURE_ROGUE_MEDIUM;
		else if (item->_itype == ITYPE_HARMOR)
			textureNum = TEXTURE_ROGUE_HEAVY;
		else
			textureNum = TEXTURE_ROGUE_LIGHT;

		if (item->_iDurability < item->_iMaxDur) {
			textureNum2 = textureNum + 1;
			opacity = 255 - ((item->_iDurability * 255) / item->_iMaxDur);
		}
	}

	Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, textureNum);
	if (textureNum2 != -1) {
		SDL_SetTextureAlphaMod(textures[textureNum2].frames[0].frame, opacity);
		Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, textureNum2);
		SDL_SetTextureAlphaMod(textures[textureNum2].frames[0].frame, 255);
	}
}

DEVILUTION_END_NAMESPACE
