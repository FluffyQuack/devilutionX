#pragma once

DEVILUTION_BEGIN_NAMESPACE

//#define MAXCELPATH 260

//Player animation types
enum {
	PLAYERANIM_STAND,
	PLAYERANIM_WALK,
	PLAYERANIM_ATTACK,
	PLAYERANIM_SPELL_LIGHTNING,
	PLAYERANIM_SPELL_FIRE,
	PLAYERANIM_SPELL_GENERIC,
	PLAYERANIM_GETHIT,
	PLAYERANIM_DEATH,
	PLAYERANIM_BLOCK,
	PLAYERANIM_STAND_CASUAL,
	PLAYERANIM_WALK_CASUAL,

	PLAYERANIM_NUM
};

/*//List of CEL files
enum {
	//CEL_CURSOR, //Mouse cursors and inventory items (data\inv\objcurs.cel)
	//CEL_CURSOR_HELLFIRE,         //Mouse cursors and inventory items (Hellfire) (data\inv\objcurs2.cel)
	CEL_INVENTORY,       //Inventory screen (data\inv\inv.cel, data\inv\inv_rog.cel, or data\inv\inv_sor.cel)
	CEL_INVENTORY_ROGUE,
	CEL_INVENTORY_SORCERER,

	/*
	TEXTURE_GOLDDROPSELECTION, //Window for gold selection? (ctrlpan\golddrop.cel)
	TEXTURE_HUDPANEL_EMPTYFLASKS, //Empty HUD flasks (ctrlpan\p8bulbs.cel)
	TEXTURE_HUDPANEL_MPBUTTONS,//Multiplayer buttons on HUD (ctrlpan\P8But2.cel)
	TEXTURE_HUDPANEL, //HUD panel (ctrlpan\panel8.cel)
	TEXTURE_HUDPANEL_BUTTONS, //Buttons pressed down on HUD panel (ctrlpan\panel8bu.cel)
	TEXTURE_SMALLFONT, //Small font (ctrlpan\smaltext.cel)
	TEXTURE_SPELLICONS, //All spell icons (ctrlpan\spelicon.cel or data\SpelIcon.CEL, depending on it being Diablo or Hellfire)
	TEXTURE_SPELLICONS_SPELL, //This and the following are TEXTURE_SPELLICONS with colours pre-modified
	TEXTURE_SPELLICONS_SCROLL,
	TEXTURE_SPELLICONS_CHARGES,
	TEXTURE_SPELLICONS_INVALID,
	TEXTURE_SMALLSPELLICONS, //Small spell icons (data\spelli2.cel)
	TEXTURE_SMALLSPELLICONS_SPELL, 
	TEXTURE_SMALLSPELLICONS_SCROLL,
	TEXTURE_SMALLSPELLICONS_CHARGES,
	TEXTURE_SMALLSPELLICONS_INVALID,
	TEXTURE_HUDPANEL_TALKBUTTONS, //Mute and voice buttons on control panel (ctrlpan\talkbutt.cel)
	TEXTURE_HUDPANEL_VOICE, //Control panel with voice buttons (ctrlpan\talkpanl.cel)
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
	TEXTURE_QUESTLOG, //Quest window (data\quest.cel)
	TEXTURE_SPELLBOOK, //Spellbook window (data\spellbk.CEL)
	TEXTURE_SPELLBOOK_BUTTONS, //Spellbook window buttons pressed down (data\spellbkb.CEL)
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
	TEXTURE_DURABILITYWARNING, //Durability icons on HUD (Items\DurIcons.CEL)

	TEXTURE_DUNGEONTILES, //Texture for current town/dungeon tileset (ie, levels\l1data\l1.cel)*/

	/*CEL_NUM
};*/

//List of textures (many of these correspond to CEL files from the original game)
enum {
	//CELs as textures
	TEXTURE_CURSOR,          //Mouse cursors and inventory items (data\inv\objcurs.cel)
	TEXTURE_CURSOR_OUTLINE,  //Same as above, but outlined versions
	TEXTURE_CURSOR2,         //Mouse cursors and inventory items (Hellfire) (data\inv\objcurs2.cel)
	TEXTURE_CURSOR2_OUTLINE, //Same as above, but outlined versions
	TEXTURE_INVENTORY,       //Inventory screen (data\inv\inv.cel, data\inv\inv_rog.cel, or data\inv\inv_sor.cel)
	TEXTURE_GOLDDROPSELECTION, //Window for gold selection? (ctrlpan\golddrop.cel)
	TEXTURE_HUDPANEL_EMPTYFLASKS, //Empty HUD flasks (ctrlpan\p8bulbs.cel)
	TEXTURE_HUDPANEL_MPBUTTONS,//Multiplayer buttons on HUD (ctrlpan\P8But2.cel)
	TEXTURE_HUDPANEL, //HUD panel (ctrlpan\panel8.cel)
	TEXTURE_HUDPANEL_BUTTONS, //Buttons pressed down on HUD panel (ctrlpan\panel8bu.cel)
	TEXTURE_SMALLFONT, //Small font (ctrlpan\smaltext.cel)
	TEXTURE_SPELLICONS, //All spell icons (ctrlpan\spelicon.cel or data\SpelIcon.CEL, depending on it being Diablo or Hellfire)
	TEXTURE_SPELLICONS_SPELL, //This and the following are TEXTURE_SPELLICONS with colours pre-modified
	TEXTURE_SPELLICONS_SCROLL,
	TEXTURE_SPELLICONS_CHARGES,
	TEXTURE_SPELLICONS_INVALID,
	TEXTURE_SMALLSPELLICONS, //Small spell icons (data\spelli2.cel)
	TEXTURE_SMALLSPELLICONS_SPELL, 
	TEXTURE_SMALLSPELLICONS_SCROLL,
	TEXTURE_SMALLSPELLICONS_CHARGES,
	TEXTURE_SMALLSPELLICONS_INVALID,
	TEXTURE_HUDPANEL_TALKBUTTONS, //Mute and voice buttons on control panel (ctrlpan\talkbutt.cel)
	TEXTURE_HUDPANEL_VOICE, //Control panel with voice buttons (ctrlpan\talkpanl.cel)
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
	TEXTURE_QUESTLOG, //Quest window (data\quest.cel)
	TEXTURE_SPELLBOOK, //Spellbook window (data\spellbk.CEL)
	TEXTURE_SPELLBOOK_BUTTONS, //Spellbook window buttons pressed down (data\spellbkb.CEL)
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
	TEXTURE_DURABILITYWARNING, //Durability icons on HUD (Items\DurIcons.CEL)
	TEXTURE_DUNGEONTILES, //Texture for current town/dungeon tileset (ie, levels\l1data\l1.cel)

	//Player textures from CEL data
	TEXTURE_PLAYERS,
	TEXTURE_PLAYERS_LAST = TEXTURE_PLAYERS + (PLAYERANIM_NUM * MAX_PLRS),

	/*//Animations of items on ground from CEL data
	TEXTURE_ITEMS,
	TEXTURE_ITEMS_LAST = TEXTURE_ITEMS + ITEMTYPES,*/

	//Missile textures from CEL data
	TEXTURE_MISSILES,
	TEXTURE_MISSILES_LAST = TEXTURE_MISSILES + (16 * MFILE_NONE), //16 is the quantity of animations one missile can have. TODO: Replace number with reference

	//Monster textures from CEL data
	TEXTURE_MONSTERS,
	TEXTURE_MONSTERS_LAST = TEXTURE_MONSTERS + (MA_NUM * 138), //138 is the quantity of unique monsters in the game. TODO: Replace number with a reference

	/*
	//Object textures from CEL data
	TEXTURE_OBJECTS,
	TEXTURE_OBJECTS_LAST = TEXTURE_OBJECTS + 40, //40 is the quantity of unique objects in the game. TODO: Replace number with a reference
	*/

	//Towners
	TEXTURE_SMITH, //Towners\\Smith\\SmithN.CEL
	TEXTURE_BAROWNER, //Towners\\TwnF\\TwnFN.CEL
	TEXTURE_DEADGUY, //Towners\\Butch\\Deadguy.CEL
	TEXTURE_WITCH, //Towners\\TownWmn1\\Witch.CEL
	TEXTURE_BARMAID, //Towners\\TownWmn1\\WmnN.CEL
	TEXTURE_BOY, //Towners\\TownBoy\\PegKid1.CEL
	TEXTURE_HEALER, //Towners\\Healer\\Healer.CEL
	TEXTURE_STORYTELLER, //Towners\\Strytell\\Strytell.CEL
	TEXTURE_DRUNK, //Towners\\Drunk\\TwnDrunk.CEL
	TEXTURE_COWS, //Towners\\Animals\\Cow.CEL
	TEXTURE_FARMER, //Towners\\Farmer\\Farmrn2.CEL
	TEXTURE_COWFARMER, //Towners\\Farmer\\cfrmrn2.CEL (or mfrmrn2.CEL)
	TEXTURE_GIRL, //Towners\\Girl\\Girlw1.CEL (or Girls1.CEL)

	//Texture versions of various masks in render.cpp
	TEXTURE_TILE_LEFTFOLIAGEMASK,
	TEXTURE_TILE_RIGHTFOLIAGEMASK,
	TEXTURE_TILE_LEFTMASK,
	TEXTURE_TILE_RIGHTMASK,

	//This is used for multiple render passes for tiles needing a different alpha mask
	TEXTURE_TILE_INTERMEDIATE, //32x32
	TEXTURE_TILE_INTERMEDIATE_BIG, //SCREEN_WIDTH * SCREEN_HEIGHT

	//BillieJoe's animated HUD flasks
	TEXTURE_HEALTHFLASK,
	TEXTURE_MANAFLASK,

	TEXTURE_NUM
};

/*
struct celInfo_s {
	char path[MAXCELPATH]; //Full path to the cel file
	int width;
	int height;
};
*/

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

//extern celInfo_s celInfo[CEL_NUM];
//extern textureAtlas_s *textureAtlases;
extern texture_s textures[TEXTURE_NUM];
extern unsigned long long totalTextureSize;

void Texture_UnloadTexture(int textureNum);
void Textures_Init();
void Textures_Deinit();

DEVILUTION_END_NAMESPACE
