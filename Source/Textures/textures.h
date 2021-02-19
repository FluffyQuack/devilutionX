#pragma once

DEVILUTION_BEGIN_NAMESPACE

//List of textures (many of these correspond to CEL files from the original game)
enum {
	//CELs as textures
	TEXTURE_CURSOR,          //Mouse cursors and inventory items (data\inv\objcurs.cel)
	TEXTURE_CURSOR_OUTLINE,  //Same as above, but outlined versions
	TEXTURE_CURSOR2,         //Mouse cursors and inventory items (Hellfire) (data\inv\objcurs2.cel)
	TEXTURE_CURSOR2_OUTLINE, //Same as above, but outlined versions
	TEXTURE_INVENTORY,       //Inventory screen (data\inv\inv.cel, data\inv\inv_rog.cel, or data\inv\inv_sor.cel)
	//Window for gold selection? (ctrlpan\golddrop.cel)
	//Empty HUD flasks (ctrlpan\p8bulbs.cel)
	//Multiplayer buttons on HUD (ctrlpan\P8But2.cel)
	//HUD panel (ctrlpan\panel8.cel)
	//Buttons pressed down on HUD panel (ctrlpan\panel8bu.cel)
	TEXTURE_SMALLFONT, //Small font (ctrlpan\smaltext.cel)
	TEXTURE_SPELLICONS, //All spell icons (ctrlpan\spelicon.cel or data\SpelIcon.CEL, depending on it being Diablo or Hellfire)
	TEXTURE_SPELLICONS_SPELL, //This as the following are TEXTURE_SPELLICONS with colours pre-modified
	TEXTURE_SPELLICONS_SCROLL,
	TEXTURE_SPELLICONS_CHARGES,
	TEXTURE_SPELLICONS_INVALID,
	TEXTURE_SMALLSPELLICONS, //Small spell icons (data\spelli2.cel)
	TEXTURE_SMALLSPELLICONS_SPELL, 
	TEXTURE_SMALLSPELLICONS_SCROLL,
	TEXTURE_SMALLSPELLICONS_CHARGES,
	TEXTURE_SMALLSPELLICONS_INVALID,
	//Mute and voice buttons on control panel (ctrlpan\talkbutt.cel)
	//Control panel with voice buttons (ctrlpan\talkpanl.cel)
	//Big font, golden colour (data\bigtgold.cel)
	TEXTURE_STATWINDOW, //Character stat screen (data\char.cel)
	TEXTURE_STATWINDOW_BUTTONS, //Level up icon and quest log pressed down (data\charbut.cel)
	//Game logo (data\diabsmal.cel)
	//Game logo animated (data\hf_logo3.CEL)
	TEXTURE_MEDIUMFONT, //Medium-sized font (data\medtexts.cel)
	//Options menu button? (data\optbar.cel)
	//Skulls next to menu button (data\option.cel)
	//Spinning pentagram (data\PentSpin.cel)
	TEXTURE_SPINNINGPENTAGRAM2, //Tiny spinning pentagram (data\PentSpn2.cel)
	//Quest window (data\quest.cel)
	//Spellbook window (data\spellbk.CEL)
	//Spellbook window buttons pressed down (data\spellbkb.CEL)
	
	//Debug tile selection (data\square.cel)
	TEXTURE_TEXTBOX, //NPC dialogue box (data\textbox.cel)
	TEXTURE_TEXTBOX2, //More narrow version of textbox (data\textbox2.cel)
	TEXTURE_DYNAMICWINDOW, //Window of any size. Used for error message and scroll bars (data\textslid.cel)
	//Catacombs loading screen (gendata\cut2.cel)
	//Caves loading screen (gendata\cut3.cel)
	//Hell loading screen (gendata\cut4.cel)
	//Pentagram loading screen (gendata\cutgate.cel)
	//Hell loading screen (gendata\cut1d.cel)
	//Town portal loading screen (gendata\cutportl.cel)
	//Red portal loading screen (gendata\cutportr.cel)
	//Town loading screen (gendata\cutstart.cel)
	//Church loading screen (gendata\cuttt.cel)
	//Crypt loading screen (nlevels\cutl5.cel)
	//Hive loading screen (nlevels\cutl6.cel)

	//BillieJoe's animated HUD flasks
	TEXTURE_HEALTHFLASK,
	TEXTURE_MANAFLASK,

	TEXTURE_NUM
};

/*
struct textureAtlas_s {
	SDL_Texture *tex;
	int width;
	int height;
	int channels;
	bool loaded;
};
*/

struct textureFrame_s {
	SDL_Texture *frame;
	//int textureAtlas; //Corresponds to an entry in textureAtlases array
	int width;
	int height;
	int channels;
	//int offsetX; //Offset for texture in the SDL_Texture (this is used for texture atlases)
	//int offsetY;
};

struct texture_s {
	bool loaded; //Whether or not texture is loaded
	textureFrame_s *frames;
	int frameCount; //Quantity of frames (imgData is an array of this length)
};

//extern textureAtlas_s *textureAtlases;
extern texture_s textures[TEXTURE_NUM];

void Texture_UnloadTexture(texture_s *texture);
void Textures_Init();
void Textures_Deinit();

DEVILUTION_END_NAMESPACE