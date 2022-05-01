#include "../all.h"
#include "../textures/textures.h"
#include "../render/sdl-render.h"

DEVILUTION_BEGIN_NAMESPACE

#define PAPERDOLLX 86
#define PAPERDOLLY 11

static void DrawPaperdollArmour(int x, int y, ItemStruct *item, int texture)
{
	Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, texture);

	if (item->_iDurability < item->_iMaxDur) {
		//TODO: Draw damage layers based on damage
		//opacity = 255 - ((item->_iDurability * 255) / item->_iMaxDur);
		//SDL_SetTextureAlphaMod(textures[textureNum2].frames[0].frame, opacity);
		//Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_ROGUE_BASE);
	}
}

void Paperdoll_Render(int x, int y)
{
	if (!options_hwUIRendering || plr[myplr]._pClass != PC_ROGUE)
		return;

	//Draw weapon on the back
	//TODO
	/*TEXTURE_PAPERDOLL_ROGUE_AXE,
	    TEXTURE_PAPERDOLL_ROGUE_SWORD,
	    TEXTURE_PAPERDOLL_ROGUE_STAFF,
	    TEXTURE_PAPERDOLL_ROGUE_MACE,
	    TEXTURE_PAPERDOLL_ROGUE_BOW_BACK,*/

	//Draw base artwork
	Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_ROGUE_BASE);

	//Draw topless texture if character is wearing no armour
	if (plr[myplr].InvBody[INVLOC_CHEST].isEmpty())
		Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_ROGUE_TOPLESS);

	//Draw panty
	Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_ROGUE_PANTY);

	//Draw armour (if it's being worn)
	if (!plr[myplr].InvBody[INVLOC_CHEST].isEmpty()) {
		ItemStruct *item = &plr[myplr].InvBody[INVLOC_CHEST];
		if (item->_itype == ITYPE_LARMOR) { //TODO: Draw TEXTURE_PAPERDOLL_ROGUE_BASEOUTFIT instead if it's certain armour items
			DrawPaperdollArmour(x, y, item, TEXTURE_PAPERDOLL_ROGUE_LEATHEROUTFIT);
		}

		//Draw belts (TODO: Should this always be rendered?)
		Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_ROGUE_BELTS);

		//Draw gloves (TODO: Should this be skipped for certain armours?)
		Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_ROGUE_GLOVES);

		//Draw boots (TODO: Should this be skipped for certain armours?)
		Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_ROGUE_BOOTS);

		//Draw chainmail or plate armour if worn
		if (item->_itype == ITYPE_MARMOR) {
			DrawPaperdollArmour(x, y, item, TEXTURE_PAPERDOLL_ROGUE_CHAINMAIL);
		} else if (item->_itype == ITYPE_HARMOR)
		{
			DrawPaperdollArmour(x, y, item, TEXTURE_PAPERDOLL_ROGUE_PLATEMAIL);
		}
	}

	//Draw face
	Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_ROGUE_FACE);

	//Draw helmet
	//TODO:
	/*TEXTURE_PAPERDOLL_ROGUE_LEATHERHELM,
	    TEXTURE_PAPERDOLL_ROGUE_LEATHERHELM_DAMAGELAYER1,
	    TEXTURE_PAPERDOLL_ROGUE_LEATHERHELM_DAMAGELAYER2,
	    TEXTURE_PAPERDOLL_ROGUE_CHAINHELM,
	    TEXTURE_PAPERDOLL_ROGUE_CHAINHELM_DAMAGELAYER1,
	    TEXTURE_PAPERDOLL_ROGUE_CHAINHELM_DAMAGELAYER2,
	    TEXTURE_PAPERDOLL_ROGUE_PLATEHELM,
	    TEXTURE_PAPERDOLL_ROGUE_PLATEHELM_DAMAGELAYER1,
	    TEXTURE_PAPERDOLL_ROGUE_PLATEHELM_DAMAGELAYER2,*/

	//Draw foreground bow if equipped
	//TODO: TEXTURE_PAPERDOLL_ROGUE_BOW_FRONT

	//Draw censor bar if Rogue is wearing no armour
	if (plr[myplr].InvBody[INVLOC_CHEST].isEmpty())
		Render_Texture(x + PAPERDOLLX, y + PAPERDOLLY, TEXTURE_PAPERDOLL_CENSORED);
}

DEVILUTION_END_NAMESPACE
