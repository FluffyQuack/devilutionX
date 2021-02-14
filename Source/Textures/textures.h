#pragma once

DEVILUTION_BEGIN_NAMESPACE

//List of textures
enum {
	//CELs as textures
	TEXTURE_CURSOR, //Mouse cursors and inventory items (data\inv\objcurs.cel)
	TEXTURE_CURSOR_OUTLINE, //Same as above, but outlined versions
	TEXTURE_CURSOR2, //Mouse cursors and inventory items (Hellfire) (data\inv\objcurs2.cel)
	TEXTURE_CURSOR2_OUTLINE, //Same as above, but outlined versions
	TEXTURE_INVENTORY, //Inventory screen (data\inv\inv.cel, data\inv\inv_rog.cel, or data\inv\inv_sor.cel)
	//Window for gold selection? (ctrlpan\golddrop.cel)
	//Empty HUD flasks (ctrlpan\p8bulbs.cel)
	//Multiplayer buttons on HUD (ctrlpan\P8But2.cel)
	//HUD panel (ctrlpan\panel8.cel)
	//Buttons pressed down on HUD panel (ctrlpan\panel8bu.cel)
	//Small font used for control panel (ctrlpan\smaltext.cel)
	//All spell icons (ctrlpan\spelicon.cel)
	//Mute and voice buttons on control panel (ctrlpan\talkbutt.cel)
	//Control panel with voice buttons (ctrlpan\talkpanl.cel)
	//Big font, golden colour (data\bigtgold.cel)
	//Character stat screen (data\char.cel)
	//Level up icon and quest log pressed down (data\charbut.cel)
	//Game logo (data\diabsmal.cel)
	//Game logo animated (data\hf_logo3.CEL)
	//Medium-sized text (data\medtexts.cel)
	//Options menu button? (data\optbar.cel)
	//Skulls next to menu button (data\option.cel)
	//Spinning pentagram (data\PentSpin.cel)
	//Tiny spinning pentagram (data\PentSpn2.cel)
	//Quest window (data\quest.cel)
	//Spell icons (data\SpelIcon.CEL)
	//Spellbook window (data\spellbk.CEL)
	//Spellbook window buttons pressed down (data\spellbkb.CEL)
	//Small spell icons (data\spelli2.cel)
	//Debug tile selection (data\square.cel)
	//NPC dialogue box (data\textbox.cel)
	//More narrow version of textbox (data\textbox2.cel)
	//Window of any size. Used for help screen? (data\textslid.cel)
	//Catacombs loading screen (gendata\cut2.cel)
	//Caves loading screen (gendata\cut3.cel)
	//Hell loading screen (gendata\cut4.cel)
	//Pentagram loading screen (gendata\cutgate.cel)
	//Hell loading screen (gendata\cut1d.cel)
	//Town portal loading screen (gendata\cutportl.cel)
	//Red portal loading screen (gendata\cutportr.cel)
	//Town loading screen (gendata\cutstart.cel)
	//Church loading screen (gendata\cuttt.cel)
	//Church loading screen (gendata\cuttt.cel)
	//Crypt loading screen (nlevels\cutl5.cel)
	//Hive loading screen (nlevels\cutl6.cel)

	//BillieJoe's animated HUD flasks
	TEXTURE_HEALTHFLASK,
	TEXTURE_MANAFLASK,

	TEXTURE_NUM
};

struct textureFrame_s {
	SDL_Texture *frame;
	int width;
	int height;
	int channels;
};

struct texture_s {
	bool loaded; //Whether or not texture is loaded
	textureFrame_s *frames;
	int frameCount; //Quantity of frames (imgData is an array of this length)
};

extern texture_s textures[TEXTURE_NUM];

void Texture_UnloadTexture(texture_s *texture);
void Textures_Init();
void Textures_Deinit();

DEVILUTION_END_NAMESPACE
