/**
 * @file diablo.cpp
 *
 * Implementation of the main game initialization functions.
 */
#include "all.h"
#include "paths.h"
#include "console.h"
#include "options.h"
#include "../3rdParty/Storm/Source/storm.h"
#include "../DiabloUI/diabloui.h"
#include <config.h>
#include "textures/textures.h" //Fluffy: For texture init and deinit
#include "textures/cel-convert.h" //Fluffy: For loading CELs as SDL textures
#include "render/lightmap.h" //Fluffy: For lightmap debugging
#include "ui/hotbar.h" //Fluffy: For hotbar input

DEVILUTION_BEGIN_NAMESPACE

SDL_Window *ghMainWnd;
DWORD glSeedTbl[NUMLEVELS];
dungeon_type gnLevelTypeTbl[NUMLEVELS];
int glEndSeed[NUMLEVELS];
int glMid1Seed[NUMLEVELS];
int glMid2Seed[NUMLEVELS];
int glMid3Seed[NUMLEVELS];
int MouseX;
int MouseY;
BOOL gbGameLoopStartup;
BOOL gbRunGame;
BOOL gbRunGameResult;
BOOL zoomflag;
/** Enable updating of player character, set to false once Diablo dies */
BOOL gbProcessPlayers;
BOOL gbLoadGame;
BOOLEAN cineflag;
int force_redraw;
BOOL light4flag;
int setseed;
int PauseMode;
bool forceSpawn;
bool forceDiablo;
bool gbTheoQuest;
bool gbCowQuest;
bool gbNestArt;
bool gbBard;
bool gbBarbarian;
int sgnTimeoutCurs;
char sgbMouseDown;
int color_cycle_timer;
int gnTickRate;
WORD gnTickDelay = 50;
/** Game options */
Options sgOptions;

unsigned long long tick_delay_highResolution = 50 * 10000; //Fluffy: High resolution tick delay. The value we set here shouldn't matter as it gets calculated in other code
unsigned int gameplayTickCount = 0; ///Fluffy: How many gameplay ticks have elapsed in total. We use for animating the new UI flasks
unsigned int gameplayTickCount_progress = 0; //Fluffy: Progress towards the above gameplayTickCount (related to gSpeedMod)

/*
  Fluffy: This value modifies the speed of the game (it's supposed to be used in tandem with ticks_per_sec so we can increase framerate while also changing game simulation speed by a corresponding value)

  This will essentially divide the game simulation by whatever gSpeedMod is (2 would be 2 times slower, 4 would be 4 times slower, etc).

  It works by multiplying the "goal" values which iterating values are compared against. And those iterating values are divided by the same amount when referenced for rendering and other functions.

  An example: One of the camera offsets can range from to 0 to 16 and it's supposed to be increased by 2 each gameplay tick (if we're at the standard 20fps). We multiply the 16 (goal value) by gSpeedMod
  and then when using the camera offset for rendering we divide it by the same amount. The result is that we modify game speed with minimal changes to the code with everything behaving almost exactly the same.

  The above is done for a lot of values related to rendering. Another thing done a lot is using a value called "tickCount" which cycles between 0 and the max value of gSpeedMod - 1. Whenever it is 0 we see that as a true gameplayTick (aka matching original timing) and we only do certain actions then.

  Here is a complete list of all the variables which can have their value affected due to gSpeedMod:
  PlayerStruct->_pVar6
  PlayerStruct->_pVar7
  PlayerStruct->_pVar8
  PlayerStruct->_pAnimCnt
  TownerStruct->tAnimCnt
  ItemStruct->iAnimCnt //New variable
  ObjectStruct->_oAnimCnt
  MissileStruct->_miAnimCnt
  MissileStruct->_mitxoff
  MissileStruct->_mityoff
  MissileStruct->tickCount //New variable

  And the ones affected by gMonsterSpeedMod:
  MonsterStruct->_mVar6
  MonsterStruct->_mVar7
  MonsterStruct->_mVar8
  MonsterStruct->_mAnimCnt
  MonsterStruct->tickCount //New variable
*/
int gSpeedMod = 1; //Fluffy
int gMonsterSpeedMod = 1; //Same as above, but specifically for monsters

//Fluffy: New global variables which are updated when loading config file (gameplay-changing ones are updated via network if we joined a network game)
BOOL gameSetup_allowAttacksInTown = false; //Fluffy: Allow attacking in town
BOOL gameSetup_safetyJog = false; //Fluffy: If true, player will jog whenever it is safe (this overrides gbRunInTown)
BOOL options_hwIngameRendering = false;               //Fluffy: If true, we render all ingame graphics via SDL (this requires options_hwUIRendering to be true)
BOOL options_hwUIRendering = false; //Fluffy: If true, we render everything via SDL (aka truecolour rendering)
BOOL options_lightmapping = false;              //Fluffy: If true, we render ingame graphics at full brightness and then generate a light map for lighting
int lastLeftMouseButtonAction = MOUSEACTION_NONE;  //Fluffy: These are for supporting repeating attacks with leftclick
int lastRightMouseButtonAction = MOUSEACTION_NONE; //Fluffy: These are for supporting repeating actions with rightclick
unsigned long long lastLeftMouseButtonTime = 0;
unsigned long long lastRightMouseButtonTime = 0;

/* rdata */

bool gbForceWindowed = false;
bool gbShowIntro = true;
bool leveldebug = false;
#ifdef _DEBUG
bool monstdebug = false;
_monster_id DebugMonsters[10];
int debugmonsttypes = 0;
bool visiondebug = false;
int questdebug = -1;
bool debug_mode_key_s = false;
bool debug_mode_key_w = false;
bool debug_mode_key_inverted_v = false;
bool debug_mode_dollar_sign = false;
bool debug_mode_key_d = false;
bool debug_mode_key_i = false;
int debug_mode_key_j = 0;
int arrowdebug = 0;
#endif
unsigned long long frame_timeOfPreviousGamePlayTick = 0; //Fluffy: For tracking gameplay tick deltas
unsigned long long frame_timeOfPreviousFrameRender = 0; //Fluffy For tracking frame render deltas
double frame_gameplayTickDelta = 0; //Fluffy
double frame_renderDelta = 0; //Fluffy
/** Specifies whether players are in non-PvP mode. */
bool gbFriendlyMode = true;
/** Specifies players will still damage other players in non-PvP mode. */
bool gbFriendlyFire;
/** Default quick messages */
const char *const spszMsgTbl[] = {
	"I need help! Come Here!",
	"Follow me.",
	"Here's something for you.",
	"Now you DIE!"
};
/** INI files variable names for quick message keys */
const char *const spszMsgHotKeyTbl[] = { "F9", "F10", "F11", "F12" };

/** To know if these things have been done when we get to the diablo_deinit() function */
bool was_archives_init = false;
/** To know if surfaces have been initialized or not */
bool was_window_init = false;
bool was_ui_init = false;
bool was_snd_init = false;
bool sbWasOptionsLoaded = false;

// Controller support:
extern void plrctrls_every_frame();
extern void plrctrls_after_game_logic();

[[noreturn]] static void print_help_and_exit()
{
	printInConsole("Options:\n");
	printInConsole("    %-20s %-30s\n", "-h, --help", "Print this message and exit");
	printInConsole("    %-20s %-30s\n", "--version", "Print the version and exit");
	printInConsole("    %-20s %-30s\n", "--data-dir", "Specify the folder of diabdat.mpq");
	printInConsole("    %-20s %-30s\n", "--save-dir", "Specify the folder of save files");
	printInConsole("    %-20s %-30s\n", "--config-dir", "Specify the location of diablo.ini");
	printInConsole("    %-20s %-30s\n", "--ttf-dir", "Specify the location of the .ttf font");
	printInConsole("    %-20s %-30s\n", "--ttf-name", "Specify the name of a custom .ttf font");
	printInConsole("    %-20s %-30s\n", "-n", "Skip startup videos");
	printInConsole("    %-20s %-30s\n", "-f", "Display frames per second");
	printInConsole("    %-20s %-30s\n", "-x", "Run in windowed mode");
	printInConsole("    %-20s %-30s\n", "--spawn", "Force spawn mode even if diabdat.mpq is found");
	printInConsole("\nHellfire options:\n");
	printInConsole("    %-20s %-30s\n", "--diablo", "Force diablo mode even if hellfire.mpq is found");
	printInConsole("    %-20s %-30s\n", "--nestart", "Use alternate nest palette");
#ifdef _DEBUG
	printInConsole("\nDebug options:\n");
	printInConsole("    %-20s %-30s\n", "-d", "Increaased item drops");
	printInConsole("    %-20s %-30s\n", "-w", "Enable cheats");
	printInConsole("    %-20s %-30s\n", "-$", "Enable god mode");
	printInConsole("    %-20s %-30s\n", "-^", "Enable god mode and debug tools");
	printInConsole("    %-20s %-30s\n", "-v", "Highlight visibility");
	printInConsole("    %-20s %-30s\n", "-i", "Ignore network timeout");
	printInConsole("    %-20s %-30s\n", "-j <##>", "Mausoleum warps to given level");
	printInConsole("    %-20s %-30s\n", "-l <##> <##>", "Start in level as type");
	printInConsole("    %-20s %-30s\n", "-m <##>", "Add debug monster, up to 10 allowed");
	printInConsole("    %-20s %-30s\n", "-q <#>", "Force a certain quest");
	printInConsole("    %-20s %-30s\n", "-r <##########>", "Set map seed");
	printInConsole("    %-20s %-30s\n", "-t <##>", "Set current quest level");
#endif
	printInConsole("\nReport bugs at https://github.com/diasurgical/devilutionX/\n");
	diablo_quit(0);
}

static void diablo_parse_flags(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		if (strcasecmp("-h", argv[i]) == 0 || strcasecmp("--help", argv[i]) == 0) {
			print_help_and_exit();
		} else if (strcasecmp("--version", argv[i]) == 0) {
			printInConsole("%s v%s\n", PROJECT_NAME, PROJECT_VERSION);
			diablo_quit(0);
		} else if (strcasecmp("--data-dir", argv[i]) == 0) {
			SetBasePath(argv[++i]);
		} else if (strcasecmp("--save-dir", argv[i]) == 0) {
			SetPrefPath(argv[++i]);
		} else if (strcasecmp("--config-dir", argv[i]) == 0) {
			SetConfigPath(argv[++i]);
		} else if (strcasecmp("--ttf-dir", argv[i]) == 0) {
			SetTtfPath(argv[++i]);
		} else if (strcasecmp("--ttf-name", argv[i]) == 0) {
			SetTtfName(argv[++i]);
		} else if (strcasecmp("-n", argv[i]) == 0) {
			gbShowIntro = false;
		} else if (strcasecmp("-f", argv[i]) == 0) {
			EnableFrameCount();
		} else if (strcasecmp("-x", argv[i]) == 0) {
			gbForceWindowed = true;
		} else if (strcasecmp("--spawn", argv[i]) == 0) {
			forceSpawn = true;
		} else if (strcasecmp("--diablo", argv[i]) == 0) {
			forceDiablo = true;
		} else if (strcasecmp("--nestart", argv[i]) == 0) {
			gbNestArt = true;
		} else if (strcasecmp("--vanilla", argv[i]) == 0) {
			gbVanilla = true;
#ifdef _DEBUG
		} else if (strcasecmp("-^", argv[i]) == 0) {
			debug_mode_key_inverted_v = true;
		} else if (strcasecmp("-$", argv[i]) == 0) {
			debug_mode_dollar_sign = true;
		} else if (strcasecmp("-d", argv[i]) == 0) {
			debug_mode_key_d = true;
		} else if (strcasecmp("-i", argv[i]) == 0) {
			debug_mode_key_i = true;
		} else if (strcasecmp("-j", argv[i]) == 0) {
			debug_mode_key_j = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-l", argv[i]) == 0) {
			setlevel = FALSE;
			leveldebug = true;
			leveltype = (dungeon_type)SDL_atoi(argv[++i]);
			currlevel = SDL_atoi(argv[++i]);
			plr[0].plrlevel = currlevel;
		} else if (strcasecmp("-m", argv[i]) == 0) {
			monstdebug = true;
			DebugMonsters[debugmonsttypes++] = (_monster_id)SDL_atoi(argv[++i]);
		} else if (strcasecmp("-q", argv[i]) == 0) {
			questdebug = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-r", argv[i]) == 0) {
			setseed = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-s", argv[i]) == 0) {
			debug_mode_key_s = true;
		} else if (strcasecmp("-t", argv[i]) == 0) {
			leveldebug = true;
			setlevel = true;
			setlvlnum = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-v", argv[i]) == 0) {
			visiondebug = true;
		} else if (strcasecmp("-w", argv[i]) == 0) {
			debug_mode_key_w = true;
#endif
		} else {
			printInConsole("unrecognized option '%s'\n", argv[i]);
			print_help_and_exit();
		}
	}
}

void FreeGameMem()
{
	music_stop();

	MemFreeDbg(pDungeonCels);
	MemFreeDbg(pMegaTiles);
	MemFreeDbg(pLevelPieces);
	MemFreeDbg(pSpecialCels);

	FreeMissiles();
	FreeMonsters();
	FreeObjectGFX();
	FreeMonsterSnd();
	FreeTownerGFX();

	if (sgOptions.Graphics.bInitLightmapping) //Fluffy: Unload lighting information for subtiles
		Lightmap_UnloadSubtileData();

	//Fluffy: Also delete equivalent SDL textures
	if (sgOptions.Graphics.bInitHwIngameRendering) {
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_SPECIAL);

		//Fluffy debug: Testing optimization
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_LEFTFOLIAGE);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_RIGHTFOLIAGE);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_LEFTMASK);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_RIGHTMASK);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_LEFTMASKINVERTED);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_RIGHTMASKINVERTED);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_LEFTMASKOPAQUE);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_RIGHTMASKOPAQUE);
		Texture_UnloadTexture(TEXTURE_DUNGEONTILES_DUNGEONPIECES);
	}
}

static void start_game(interface_mode uMsg)
{
	zoomflag = TRUE;
	CalcViewportGeometry();
	cineflag = FALSE;
	InitCursor();
	InitLightTable();
#ifdef _DEBUG
	LoadDebugGFX();
#endif
	assert(ghMainWnd);
	music_stop();
	InitQol();
	ShowProgress(uMsg);
	gmenu_init_menu();
	InitLevelCursor();
	sgnTimeoutCurs = CURSOR_NONE;
	sgbMouseDown = CLICK_NONE;
	track_repeat_walk(FALSE);

	//Fluffy: Load various CELs as SDL textures here
	if (sgOptions.Graphics.bInitHwUIRendering) {
		//Cursors
		if (!textures[TEXTURE_CURSOR].loaded) {
			Texture_ConvertCEL_MultipleFrames_VariableResolution(pCursCels, TEXTURE_CURSOR, (int *)&InvItemWidth[1], (int *)&InvItemHeight[1], true);
			Texture_ConvertCEL_MultipleFrames_Outlined_VariableResolution(pCursCels, TEXTURE_CURSOR_OUTLINE, (int *)&InvItemWidth[1], (int *)&InvItemHeight[1], true);
		}
		if (!textures[TEXTURE_CURSOR2].loaded && gbIsHellfire) {
			Texture_ConvertCEL_MultipleFrames_VariableResolution(pCursCels2, TEXTURE_CURSOR2, (int *)&InvItemWidth[180], (int *)&InvItemHeight[180], true);
			Texture_ConvertCEL_MultipleFrames_Outlined_VariableResolution(pCursCels2, TEXTURE_CURSOR2_OUTLINE, (int *)&InvItemWidth[180], (int *)&InvItemHeight[180], true);
		}
	}
}

static void free_game()
{
	int i;

	FreeQol();
	FreeControlPan();
	FreeInvGFX();
	FreeGMenu();
	FreeQuestText();
	FreeStoreMem();

	for (i = 0; i < MAX_PLRS; i++)
		FreePlayerGFX(i);

	FreeItemGFX();
	FreeCursor();
	FreeLightTable();
#ifdef _DEBUG
	FreeDebugGFX();
#endif
	FreeGameMem();
}

// Controller support: Actions to run after updating the cursor state.
// Defined in SourceX/controls/plctrls.cpp.
extern void finish_simulated_mouse_clicks(int current_mouse_x, int current_mouse_y);
extern void plrctrls_after_check_curs_move();

static bool ProcessInput()
{
	if (PauseMode == 2) {
		return false;
	}

	plrctrls_every_frame();

	if (!gbIsMultiplayer && gmenu_is_active()) {
		force_redraw |= 1;
		return false;
	}

	if (!gmenu_is_active() && sgnTimeoutCurs == CURSOR_NONE) {
#ifndef USE_SDL1
		finish_simulated_mouse_clicks(MouseX, MouseY);
#endif
		CheckCursMove();
		plrctrls_after_check_curs_move();
		track_process();
	}

	return true;
}

static void run_game_loop(interface_mode uMsg)
{
	WNDPROC saveProc;
	MSG msg;

	nthread_ignore_mutex(TRUE);
	start_game(uMsg);
	assert(ghMainWnd);
	saveProc = SetWindowProc(GM_Game);
	control_update_life_mana();
	run_delta_info();
	gbRunGame = TRUE;
	gbProcessPlayers = TRUE;
	gbRunGameResult = TRUE;
	force_redraw = 255;
	DrawAndBlit();
	LoadPWaterPalette();
	PaletteFadeIn(8);
	force_redraw = 255;
	gbGameLoopStartup = TRUE;
	nthread_ignore_mutex(FALSE);

	while (gbRunGame) {
		while (FetchMessage(&msg)) {
			if (msg.message == DVL_WM_QUIT) {
				gbRunGameResult = FALSE;
				gbRunGame = FALSE;
				break;
			}
			TranslateMessage(&msg);
			PushMessage(&msg);
		}
		if (!gbRunGame)
			break;
		if (!nthread_has_500ms_passed()) {
			ProcessInput();
			force_redraw |= 1;
			DrawAndBlit();
			continue;
		}
		diablo_color_cyc_logic();
		multi_process_network_packets();
		game_loop(gbGameLoopStartup);
		gbGameLoopStartup = FALSE;
		DrawAndBlit();
	}

	if (gbIsMultiplayer) {
		pfile_write_hero();
	}

	pfile_flush_W();
	PaletteFadeOut(8);
	NewCursor(CURSOR_NONE);
	ClearScreenBuffer();
	if (options_hwUIRendering) //Fluffy: dx_face would be 255 here after a fade out, but we're not doing a fade back in, so we set it to 0 here
		dx_fade = 0;
	force_redraw = 255;
	scrollrt_draw_game_screen(TRUE);
	saveProc = SetWindowProc(saveProc);
	assert(saveProc == GM_Game);
	free_game();

	if (cineflag) {
		cineflag = FALSE;
		DoEnding();
	}
}

BOOL StartGame(BOOL bNewGame, BOOL bSinglePlayer)
{
	BOOL fExitProgram;

	gbSelectProvider = TRUE;

	do {
		fExitProgram = FALSE;
		gbLoadGame = FALSE;

		if (!NetInit(bSinglePlayer, &fExitProgram)) {
			gbRunGameResult = !fExitProgram;
			break;
		}

		gbSelectProvider = FALSE;

		if (bNewGame || !gbValidSaveFile) {
			InitLevels();
			InitQuests();
			InitPortals();
			InitDungMsgs(myplr);
		}
		interface_mode uMsg = WM_DIABNEWGAME;
		if (gbValidSaveFile && gbLoadGame) {
			uMsg = WM_DIABLOADGAME;
		}
		run_game_loop(uMsg);
		NetClose();
		pfile_create_player_description(NULL, 0);
	} while (gbRunGameResult);

	SNetDestroy();
	return gbRunGameResult;
}

/**
 * @brief Save game configurations to ini file
 */
static void SaveOptions()
{
	setIniInt("Diablo", "Intro", sgOptions.Diablo.bIntro);
	setIniInt("Hellfire", "Intro", sgOptions.Hellfire.bIntro);
	setIniValue("Hellfire", "SItem", sgOptions.Hellfire.szItem);

	setIniInt("Audio", "Sound Volume", sgOptions.Audio.nSoundVolume);
	setIniInt("Audio", "Music Volume", sgOptions.Audio.nMusicVolume);
	setIniInt("Audio", "Walking Sound", sgOptions.Audio.bWalkingSound);
	setIniInt("Audio", "Auto Equip Sound", sgOptions.Audio.bAutoEquipSound);

#ifndef __vita__
	setIniInt("Graphics", "Width", sgOptions.Graphics.nWidth);
	setIniInt("Graphics", "Height", sgOptions.Graphics.nHeight);
#endif
	setIniInt("Graphics", "Fullscreen", sgOptions.Graphics.bFullscreen);
#ifndef __vita__
	setIniInt("Graphics", "Upscale", sgOptions.Graphics.bUpscale);
#endif
	setIniInt("Graphics", "Fit to Screen", sgOptions.Graphics.bFitToScreen);
	setIniValue("Graphics", "Scaling Quality", sgOptions.Graphics.szScaleQuality);
	setIniInt("Graphics", "Integer Scaling", sgOptions.Graphics.bIntegerScaling);
	setIniInt("Graphics", "Vertical Sync", sgOptions.Graphics.bVSync);
	setIniInt("Graphics", "Blended Transparency", sgOptions.Graphics.bBlendedTransparancy);
	setIniInt("Graphics", "Gamma Correction", sgOptions.Graphics.nGammaCorrection);
	setIniInt("Graphics", "Color Cycling", sgOptions.Graphics.bColorCycling);
	setIniInt("Graphics", "FPS Limiter", sgOptions.Graphics.bFPSLimit);

	//Fluffy
	setIniInt("Graphics", "Nonobscuring Walls Are Opaque", sgOptions.Graphics.bOpaqueWallsUnlessObscuring);
	setIniInt("Graphics", "Opaque Walls With Blobs", sgOptions.Graphics.bOpaqueWallsWithBlobs);
	setIniInt("Graphics", "Opaque Walls With Silhouettes", sgOptions.Graphics.bOpaqueWallsWithSilhouette);
	setIniInt("Graphics", "Hardware Ingame Rendering", sgOptions.Graphics.bInitHwIngameRendering);
	setIniInt("Graphics", "Hardware UI Rendering", sgOptions.Graphics.bInitHwUIRendering);
	setIniInt("Graphics", "Lightmapping", sgOptions.Graphics.bInitLightmapping);
	setIniInt("Graphics", "Animated UI Flasks", sgOptions.Graphics.bAnimatedUIFlasks);
	setIniInt("Graphics", "Durability Icon Gradual Change", sgOptions.Graphics.bDurabilityIconGradualChange);
	setIniInt("Graphics", "Durability Icon Gold Value", sgOptions.Graphics.nDurabilityIconGold);
	setIniInt("Graphics", "Durability Icon Red Value", sgOptions.Graphics.nDurabilityIconRed);

	setIniInt("Game", "Speed", sgOptions.Gameplay.nTickRate);
	setIniInt("Game", "Run in Town", sgOptions.Gameplay.bRunInTown);
	setIniInt("Game", "Grab Input", sgOptions.Gameplay.bGrabInput);
	setIniInt("Game", "Theo Quest", sgOptions.Gameplay.bTheoQuest);
	setIniInt("Game", "Cow Quest", sgOptions.Gameplay.bCowQuest);
	setIniInt("Game", "Friendly Fire", sgOptions.Gameplay.bFriendlyFire);
	setIniInt("Game", "Test Bard", sgOptions.Gameplay.bTestBard);
	setIniInt("Game", "Test Barbarian", sgOptions.Gameplay.bTestBarbarian);
	setIniInt("Game", "Experience Bar", sgOptions.Gameplay.bExperienceBar);
	setIniInt("Game", "Enemy Health Bar", sgOptions.Gameplay.bEnemyHealthBar);
	setIniInt("Game", "Auto Gold Pickup", sgOptions.Gameplay.bAutoGoldPickup);
	setIniInt("Game", "Adria Refills Mana", sgOptions.Gameplay.bAdriaRefillsMana);
	setIniInt("Game", "Auto Equip Weapons", sgOptions.Gameplay.bAutoEquipWeapons);
	setIniInt("Game", "Auto Equip Armor", sgOptions.Gameplay.bAutoEquipArmor);
	setIniInt("Game", "Auto Equip Helms", sgOptions.Gameplay.bAutoEquipHelms);
	setIniInt("Game", "Auto Equip Shields", sgOptions.Gameplay.bAutoEquipShields);
	setIniInt("Game", "Auto Equip Jewelry", sgOptions.Gameplay.bAutoEquipJewelry);
	setIniInt("Game", "Randomize Quests", sgOptions.Gameplay.bRandomizeQuests);
	setIniInt("Game", "Show Monster Type", sgOptions.Gameplay.bShowMonsterType);

	//Fluffy
	setIniInt("Game", "Allow Attacks In Town", sgOptions.Gameplay.bAllowAttacksInTown);
	setIniInt("Game", "Jog When Safe", sgOptions.Gameplay.bSafetyJog);
	setIniInt("Game", "No Equipped Spell Is Attack", sgOptions.Gameplay.bNoEquippedSpellIsAttack);
	setIniInt("Game", "Hold To Attack", sgOptions.Gameplay.bHoldToAttack);
	setIniInt("Game", "Hotbar", sgOptions.Gameplay.bHotbar);

	setIniValue("Network", "Bind Address", sgOptions.Network.szBindAddress);
	setIniInt("Network", "Port", sgOptions.Network.nPort);
	setIniValue("Network", "Previous Host", sgOptions.Network.szPreviousHost);

	for (int i = 0; i < sizeof(spszMsgTbl) / sizeof(spszMsgTbl[0]); i++)
		setIniValue("NetMsg", spszMsgHotKeyTbl[i], sgOptions.Chat.szHotKeyMsgs[i]);

	setIniValue("Controller", "Mapping", sgOptions.Controller.szMapping);
	setIniInt("Controller", "Swap Shoulder Button Mode", sgOptions.Controller.bSwapShoulderButtonMode);
	setIniInt("Controller", "Dpad Hotkeys", sgOptions.Controller.bDpadHotkeys);
#ifdef __vita__
	setIniInt("Controller", "Enable Rear Touchpad", sgOptions.Controller.bRearTouch);
#endif

	SaveIni();
}

/**
 * @brief Load game configurations from ini file
 */
static void LoadOptions()
{
	sgOptions.Diablo.bIntro = getIniBool("Diablo", "Intro", true);
	sgOptions.Hellfire.bIntro = getIniBool("Hellfire", "Intro", true);
	getIniValue("Hellfire", "SItem", sgOptions.Hellfire.szItem, sizeof(sgOptions.Hellfire.szItem), "");

	sgOptions.Audio.nSoundVolume = getIniInt("Audio", "Sound Volume", VOLUME_MAX);
	sgOptions.Audio.nMusicVolume = getIniInt("Audio", "Music Volume", VOLUME_MAX);
	sgOptions.Audio.bWalkingSound = getIniBool("Audio", "Walking Sound", true);
	sgOptions.Audio.bAutoEquipSound = getIniBool("Audio", "Auto Equip Sound", false);

#ifndef __vita__
	sgOptions.Graphics.nWidth = getIniInt("Graphics", "Width", DEFAULT_WIDTH);
	sgOptions.Graphics.nHeight = getIniInt("Graphics", "Height", DEFAULT_HEIGHT);
#else
	sgOptions.Graphics.nWidth = DEFAULT_WIDTH;
	sgOptions.Graphics.nHeight = DEFAULT_HEIGHT;
#endif
	sgOptions.Graphics.bFullscreen = getIniBool("Graphics", "Fullscreen", true);
#if !defined(USE_SDL1) && !defined(__vita__)
	sgOptions.Graphics.bUpscale = getIniBool("Graphics", "Upscale", true);
#else
	sgOptions.Graphics.bUpscale = false;
#endif
	sgOptions.Graphics.bFitToScreen = getIniBool("Graphics", "Fit to Screen", true);
	getIniValue("Graphics", "Scaling Quality", sgOptions.Graphics.szScaleQuality, sizeof(sgOptions.Graphics.szScaleQuality), "2");
	sgOptions.Graphics.bIntegerScaling = getIniBool("Graphics", "Integer Scaling", false);
	sgOptions.Graphics.bVSync = getIniBool("Graphics", "Vertical Sync", true);
	sgOptions.Graphics.bBlendedTransparancy = getIniBool("Graphics", "Blended Transparency", true);
	sgOptions.Graphics.nGammaCorrection = getIniInt("Graphics", "Gamma Correction", 100);
	sgOptions.Graphics.bColorCycling = getIniBool("Graphics", "Color Cycling", true);
	sgOptions.Graphics.bFPSLimit = getIniBool("Graphics", "FPS Limiter", true);

	//Fluffy
	sgOptions.Graphics.bOpaqueWallsUnlessObscuring = getIniBool("Graphics", "Nonobscuring Walls Are Opaque", false);
	sgOptions.Graphics.bOpaqueWallsWithBlobs = getIniBool("Graphics", "Opaque Walls With Blobs", false);
	sgOptions.Graphics.bOpaqueWallsWithSilhouette = getIniBool("Graphics", "Opaque Walls With Silhouettes", false);
	sgOptions.Graphics.bInitHwIngameRendering = getIniBool("Graphics", "Hardware Ingame Rendering", true);
	sgOptions.Graphics.bInitHwUIRendering = getIniBool("Graphics", "Hardware UI Rendering", true);
	sgOptions.Graphics.bInitLightmapping = getIniBool("Graphics", "Lightmapping", true);
	sgOptions.Graphics.bAnimatedUIFlasks = getIniBool("Graphics", "Animated UI Flasks", true);
	sgOptions.Graphics.bDurabilityIconGradualChange = getIniBool("Graphics", "Durability Icon Gradual Change", true);
	sgOptions.Graphics.nDurabilityIconGold = getIniInt("Graphics", "Durability Icon Gold Value", 5);
	sgOptions.Graphics.nDurabilityIconRed = getIniInt("Graphics", "Durability Icon Red Value", 2);

	//Fluffy: HW ingame rendering can't be true unless HW UI rendering is also true
	if (sgOptions.Graphics.bInitHwIngameRendering && !sgOptions.Graphics.bInitHwUIRendering)
		sgOptions.Graphics.bInitHwIngameRendering = false;

	//Fluffy TODO
	//options_hwRendering = options_initHwRendering;
	//options_lightmapping = options_initLightmapping;

	sgOptions.Gameplay.nTickRate = getIniInt("Game", "Speed", 20);
	sgOptions.Gameplay.bRunInTown = getIniBool("Game", "Run in Town", false);
	sgOptions.Gameplay.bGrabInput = getIniBool("Game", "Grab Input", false);
	sgOptions.Gameplay.bTheoQuest = getIniBool("Game", "Theo Quest", false);
	sgOptions.Gameplay.bCowQuest = getIniBool("Game", "Cow Quest", false);
	sgOptions.Gameplay.bFriendlyFire = getIniBool("Game", "Friendly Fire", true);
	sgOptions.Gameplay.bTestBard = getIniBool("Game", "Test Bard", false);
	sgOptions.Gameplay.bTestBarbarian = getIniBool("Game", "Test Barbarian", false);
	sgOptions.Gameplay.bExperienceBar = getIniBool("Game", "Experience Bar", false);
	sgOptions.Gameplay.bEnemyHealthBar = getIniBool("Game", "Enemy Health Bar", false);
	sgOptions.Gameplay.bAutoGoldPickup = getIniBool("Game", "Auto Gold Pickup", false);
	sgOptions.Gameplay.bAdriaRefillsMana = getIniBool("Game", "Adria Refills Mana", false);
	sgOptions.Gameplay.bAutoEquipWeapons = getIniBool("Game", "Auto Equip Weapons", true);
	sgOptions.Gameplay.bAutoEquipArmor = getIniBool("Game", "Auto Equip Armor", false);
	sgOptions.Gameplay.bAutoEquipHelms = getIniBool("Game", "Auto Equip Helms", false);
	sgOptions.Gameplay.bAutoEquipShields = getIniBool("Game", "Auto Equip Shields", false);
	sgOptions.Gameplay.bAutoEquipJewelry = getIniBool("Game", "Auto Equip Jewelry", false);
	sgOptions.Gameplay.bRandomizeQuests = getIniBool("Game", "Randomize Quests", true);
	sgOptions.Gameplay.bShowMonsterType = getIniBool("Game", "Show Monster Type", false);

	//Fluffy
	sgOptions.Gameplay.bAllowAttacksInTown = getIniBool("Game", "Allow Attacks In Town", true);
	sgOptions.Gameplay.bSafetyJog = getIniBool("Game", "Jog When Safe", true);
	sgOptions.Gameplay.bNoEquippedSpellIsAttack = getIniBool("Game", "No Equipped Spell Is Attack", true);
	sgOptions.Gameplay.bHoldToAttack = getIniBool("Game", "Hold To Attack", true);
	sgOptions.Gameplay.bHotbar = getIniBool("Game", "Hotbar", true);

	getIniValue("Network", "Bind Address", sgOptions.Network.szBindAddress, sizeof(sgOptions.Network.szBindAddress), "0.0.0.0");
	sgOptions.Network.nPort = getIniInt("Network", "Port", 6112);
	getIniValue("Network", "Previous Host", sgOptions.Network.szPreviousHost, sizeof(sgOptions.Network.szPreviousHost), "");

	for (int i = 0; i < sizeof(spszMsgTbl) / sizeof(spszMsgTbl[0]); i++)
		getIniValue("NetMsg", spszMsgHotKeyTbl[i], sgOptions.Chat.szHotKeyMsgs[i], MAX_SEND_STR_LEN, spszMsgTbl[i]);

	getIniValue("Controller", "Mapping", sgOptions.Controller.szMapping, sizeof(sgOptions.Controller.szMapping), "");
	sgOptions.Controller.bSwapShoulderButtonMode = getIniBool("Controller", "Swap Shoulder Button Mode", false);
	sgOptions.Controller.bDpadHotkeys = getIniBool("Controller", "Dpad Hotkeys", false);
#ifdef __vita__
	sgOptions.Controller.bRearTouch = getIniBool("Controller", "Enable Rear Touchpad", true);
#endif

	sbWasOptionsLoaded = true;
}

static void diablo_init_screen()
{
	MouseX = gnScreenWidth / 2;
	MouseY = gnScreenHeight / 2;
	if (!sgbControllerActive)
		SetCursorPos(MouseX, MouseY);
	ScrollInfo._sdx = 0;
	ScrollInfo._sdy = 0;
	ScrollInfo._sxoff = 0;
	ScrollInfo._syoff = 0;
	ScrollInfo._sdir = SDIR_NONE;

	ClrDiabloMsg();
}

static void diablo_init()
{
	init_create_window();
	Textures_Init(); //Fluffy: Load textures here since SDL has finished its init
	was_window_init = true;

	SFileEnableDirectAccess(TRUE);
	init_archives();
	was_archives_init = true;

	if (forceSpawn)
		gbIsSpawn = true;
	if (forceDiablo)
		gbIsHellfire = false;

	UiInitialize();
	UiSetSpawned(gbIsSpawn);
	was_ui_init = true;

	ReadOnlyTest();

	InitHash();

	diablo_init_screen();

	snd_init();
	was_snd_init = true;

	ui_sound_init();
}

static void diablo_splash()
{
	if (!gbShowIntro)
		return;

	play_movie("gendata\\logo.smk", TRUE);

	if (gbIsHellfire && sgOptions.Hellfire.bIntro) {
		play_movie("gendata\\Hellfire.smk", TRUE);
		sgOptions.Hellfire.bIntro = false;
	}
	if (!gbIsHellfire && !gbIsSpawn && sgOptions.Diablo.bIntro) {
		play_movie("gendata\\diablo1.smk", TRUE);
		sgOptions.Diablo.bIntro = false;
	}

	UiTitleDialog();
}

static void diablo_deinit()
{
	if (sbWasOptionsLoaded)
		SaveOptions();
	if (was_snd_init) {
		effects_cleanup_sfx();
	}
	if (was_ui_init)
		UiDestroy();
	if (was_archives_init)
		init_cleanup();
	
	if (was_window_init) {
		Textures_Deinit(); //Fluffy: Unload SDL textures
		dx_cleanup(); // Cleanup SDL surfaces stuff, so we have to do it before SDL_Quit().
	}
	if (was_fonts_init)
		FontsCleanup();
	if (SDL_WasInit(SDL_INIT_EVERYTHING & ~SDL_INIT_HAPTIC))
		SDL_Quit();
}

void diablo_quit(int exitStatus)
{
	diablo_deinit();
	exit(exitStatus);
}

int DiabloMain(int argc, char **argv)
{
	diablo_parse_flags(argc, argv);
	LoadOptions();
	diablo_init();
	diablo_splash();
	mainmenu_loop();
	diablo_deinit();

	return 0;
}

static BOOL LeftMouseCmd(BOOL bShift)
{
	//If this function returns false, then mouse click becomes walk
	BOOL bNear;

	assert(MouseY < PANEL_TOP || MouseX < PANEL_LEFT || MouseX >= PANEL_LEFT + PANEL_WIDTH);

	if (leveltype == DTYPE_TOWN) {
		if (bShift && gameSetup_allowAttacksInTown) { //Fluffy: Attack if shift is held down and if attacking in town is allowed
			lastLeftMouseButtonAction = MOUSEACTION_ATTACK;
			if (plr[myplr]._pwtype == WT_RANGED)
				NetSendCmdLoc(TRUE, CMD_RATTACKXY, cursmx, cursmy);
			else
				NetSendCmdLoc(TRUE, CMD_SATTACKXY, cursmx, cursmy);
		}
		else if (pcursitem != -1 && pcurs == CURSOR_HAND)
			NetSendCmdLocParam1(TRUE, invflag ? CMD_GOTOGETITEM : CMD_GOTOAGETITEM, cursmx, cursmy, pcursitem);
		else if (pcursmonst != -1)
			NetSendCmdLocParam1(TRUE, CMD_TALKXY, cursmx, cursmy, pcursmonst);
			
		if ((!bShift || !gameSetup_allowAttacksInTown) && pcursitem == -1 && pcursmonst == -1 && pcursplr == -1) //Fluffy
			return TRUE;
	} else {
		bNear = abs(plr[myplr]._px - cursmx) < 2 && abs(plr[myplr]._py - cursmy) < 2;
		if (pcursitem != -1 && pcurs == CURSOR_HAND && !bShift) {
			NetSendCmdLocParam1(TRUE, invflag ? CMD_GOTOGETITEM : CMD_GOTOAGETITEM, cursmx, cursmy, pcursitem);
		} else if (pcursobj != -1 && (!bShift || (bNear && object[pcursobj]._oBreak == 1))) {
			NetSendCmdLocParam1(TRUE, pcurs == CURSOR_DISARM ? CMD_DISARMXY : CMD_OPOBJXY, cursmx, cursmy, pcursobj);
		} else if (plr[myplr]._pwtype == WT_RANGED) {
			if (bShift) {
				lastLeftMouseButtonAction = MOUSEACTION_ATTACK; //Fluffy
				NetSendCmdLoc(TRUE, CMD_RATTACKXY, cursmx, cursmy);
			} else if (pcursmonst != -1) {
				if (CanTalkToMonst(pcursmonst)) {
					NetSendCmdParam1(TRUE, CMD_ATTACKID, pcursmonst);
				} else {
					lastLeftMouseButtonAction = MOUSEACTION_ATTACK_MONSTERTARGET; //Fluffy
					NetSendCmdParam1(TRUE, CMD_RATTACKID, pcursmonst);
				}
			} else if (pcursplr != -1 && !gbFriendlyMode) {
				lastLeftMouseButtonAction = MOUSEACTION_ATTACK_PLAYERTARGET; //Fluffy
				NetSendCmdParam1(TRUE, CMD_RATTACKPID, pcursplr);
			}
		} else {
			if (bShift) {
				if (pcursmonst != -1) {
					if (CanTalkToMonst(pcursmonst)) {
						NetSendCmdParam1(TRUE, CMD_ATTACKID, pcursmonst);
					} else {
						lastLeftMouseButtonAction = MOUSEACTION_ATTACK; //Fluffy
						NetSendCmdLoc(TRUE, CMD_SATTACKXY, cursmx, cursmy);
					}
				} else {
					lastLeftMouseButtonAction = MOUSEACTION_ATTACK; //Fluffy
					NetSendCmdLoc(TRUE, CMD_SATTACKXY, cursmx, cursmy);
				}
			} else if (pcursmonst != -1) {
				lastLeftMouseButtonAction = MOUSEACTION_ATTACK_MONSTERTARGET; //Fluffy
				NetSendCmdParam1(TRUE, CMD_ATTACKID, pcursmonst);
			} else if (pcursplr != -1 && !gbFriendlyMode) {
				lastLeftMouseButtonAction = MOUSEACTION_ATTACK_PLAYERTARGET; //Fluffy
				NetSendCmdParam1(TRUE, CMD_ATTACKPID, pcursplr);
			}
		}
		if (!bShift && pcursitem == -1 && pcursobj == -1 && pcursmonst == -1 && pcursplr == -1)
			return TRUE;
	}

	return FALSE;
}

BOOL TryIconCurs()
{
	if (pcurs == CURSOR_RESURRECT) {
		NetSendCmdParam1(TRUE, CMD_RESURRECT, pcursplr);
		return TRUE;
	}

	if (pcurs == CURSOR_HEALOTHER) {
		NetSendCmdParam1(TRUE, CMD_HEALOTHER, pcursplr);
		return TRUE;
	}

	if (pcurs == CURSOR_TELEKINESIS) {
		DoTelekinesis();
		return TRUE;
	}

	if (pcurs == CURSOR_IDENTIFY) {
		if (pcursinvitem != -1)
			CheckIdentify(myplr, pcursinvitem);
		else
			NewCursor(CURSOR_HAND);
		return TRUE;
	}

	if (pcurs == CURSOR_REPAIR) {
		if (pcursinvitem != -1)
			DoRepair(myplr, pcursinvitem);
		else
			NewCursor(CURSOR_HAND);
		return TRUE;
	}

	if (pcurs == CURSOR_RECHARGE) {
		if (pcursinvitem != -1)
			DoRecharge(myplr, pcursinvitem);
		else
			NewCursor(CURSOR_HAND);
		return TRUE;
	}

	if (pcurs == CURSOR_OIL) {
		if (pcursinvitem != -1)
			DoOil(myplr, pcursinvitem);
		else
			NewCursor(CURSOR_HAND);
		return TRUE;
	}

	if (pcurs == CURSOR_TELEPORT) {
		if (pcursmonst != -1)
			NetSendCmdParam3(TRUE, CMD_TSPELLID, pcursmonst, plr[myplr]._pTSpell, GetSpellLevel(myplr, plr[myplr]._pTSpell));
		else if (pcursplr != -1)
			NetSendCmdParam3(TRUE, CMD_TSPELLPID, pcursplr, plr[myplr]._pTSpell, GetSpellLevel(myplr, plr[myplr]._pTSpell));
		else
			NetSendCmdLocParam2(TRUE, CMD_TSPELLXY, cursmx, cursmy, plr[myplr]._pTSpell, GetSpellLevel(myplr, plr[myplr]._pTSpell));
		NewCursor(CURSOR_HAND);
		return TRUE;
	}

	if (pcurs == CURSOR_DISARM && pcursobj == -1) {
		NewCursor(CURSOR_HAND);
		return TRUE;
	}

	return FALSE;
}

static BOOL LeftMouseDown(int wParam)
{
	lastLeftMouseButtonAction = MOUSEACTION_OTHER; //Fluffy
	lastLeftMouseButtonTime = SDL_GetPerformanceCounter(); //Fluffy
	if (gmenu_left_mouse(TRUE))
		return FALSE;

	if (control_check_talk_btn())
		return FALSE;

	if (sgnTimeoutCurs != CURSOR_NONE)
		return FALSE;

	if (deathflag) {
		control_check_btn_press();
		return FALSE;
	}

	if (PauseMode == 2) {
		return FALSE;
	}
	if (doomflag) {
		doom_close();
		return FALSE;
	}

	if (spselflag) {
		SetSpell();
		return FALSE;
	}

	if (stextflag != STORE_NONE) {
		CheckStoreBtn();
		return FALSE;
	}

	bool isShiftHeld = wParam & DVL_MK_SHIFT;

	if (MouseY < PANEL_TOP || MouseX < PANEL_LEFT || MouseX >= PANEL_LEFT + PANEL_WIDTH) {
		if (!gmenu_is_active() && !TryIconCurs()) {
			if (questlog && MouseX > 32 && MouseX < 288 && MouseY > 32 && MouseY < 308) {
				QuestlogESC();
			} else if (qtextflag) {
				qtextflag = FALSE;
				stream_stop();
			} else if (chrflag && MouseX < SPANEL_WIDTH && MouseY < SPANEL_HEIGHT) {
				CheckChrBtns();
			} else if (IsMouseOnInventoryScreen()) { //Fluffy
				if (!dropGoldFlag)
					CheckInvItem(isShiftHeld);
			} else if (sbookflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
				CheckSBook();
			} else if (pcurs >= CURSOR_FIRSTITEM) {
				if (TryInvPut()) {
					NetSendCmdPItem(TRUE, CMD_PUTITEM, cursmx, cursmy);
					NewCursor(CURSOR_HAND);
				}
			} else {
				if (plr[myplr]._pStatPts != 0 && !spselflag)
					CheckLvlBtn();
				if (!lvlbtndown)
					return LeftMouseCmd(isShiftHeld);
			}
		}
	} else {
		if (!sgOptions.Gameplay.bHotbar && !talkflag && !dropGoldFlag && !gmenu_is_active()) { //Fluffy: Added hotbar check
			CheckInvScrn(isShiftHeld);
		} else if (sgOptions.Gameplay.bHotbar && pcurs == CURSOR_HAND && Hotbar_LeftMouseDown()) { //Fluffy
		} else 
			DoPanBtn();
		if (pcurs > CURSOR_HAND && pcurs < CURSOR_FIRSTITEM)
			NewCursor(CURSOR_HAND);
	}

	return FALSE;
}

static void LeftMouseUp(int wParam)
{
	gmenu_left_mouse(FALSE);
	control_release_talk_btn();
	bool isShiftHeld = wParam & (DVL_MK_SHIFT | DVL_MK_LBUTTON);
	if (panbtndown)
		CheckBtnUp();
	if (chrbtnactive)
		ReleaseChrBtns(isShiftHeld);
	if (lvlbtndown)
		ReleaseLvlBtn();
	if (stextflag != STORE_NONE)
		ReleaseStoreBtn();
}

static void RightMouseDown()
{
	lastRightMouseButtonAction = MOUSEACTION_OTHER; //Fluffy
	lastRightMouseButtonTime = SDL_GetPerformanceCounter(); //Fluffy
	if (!gmenu_is_active() && sgnTimeoutCurs == CURSOR_NONE && PauseMode != 2 && !plr[myplr]._pInvincible) {
		if (doomflag) {
			doom_close();
		} else if (stextflag == STORE_NONE) {
			if (spselflag) {
				SetSpell();
			} else if (MouseY >= SPANEL_HEIGHT
			    || (!sbookflag || MouseX <= RIGHT_PANEL)
			        && !TryIconCurs()
			        && (pcursinvitem == -1 || !UseInvItem(myplr, pcursinvitem))) {
				if (pcurs == CURSOR_HAND) {
					if (sgOptions.Gameplay.bHotbar && Hotbar_RightMouseDown()) { //Fluffy
					} else if (sgOptions.Gameplay.bNoEquippedSpellIsAttack && IsMouseOnRightSpellIcon()) { //Fluffy: Unselect "spell"
						ClearReadiedSpell(plr[myplr]);
					} else if(pcursinvitem == -1 || !UseInvItem(myplr, pcursinvitem))
						CheckPlrSpell(true);
				} else if (pcurs > CURSOR_HAND && pcurs < CURSOR_FIRSTITEM) {
					NewCursor(CURSOR_HAND);
				}
			}
		}
	}
}

void diablo_pause_game()
{
	if (!gbIsMultiplayer) {
		if (PauseMode) {
			PauseMode = 0;
		} else {
			PauseMode = 2;
			sound_stop();
			track_repeat_walk(FALSE);
		}
		force_redraw = 255;
	}
}

static void diablo_hotkey_msg(DWORD dwMsg)
{
	if (!gbIsMultiplayer) {
		return;
	}

	assert(dwMsg < sizeof(spszMsgTbl) / sizeof(spszMsgTbl[0]));

	NetSendCmdString(-1, sgOptions.Chat.szHotKeyMsgs[dwMsg]);
}

static BOOL PressSysKey(int wParam)
{
	if (gmenu_is_active() || wParam != DVL_VK_F10)
		return FALSE;
	diablo_hotkey_msg(1);
	return TRUE;
}

static void ReleaseKey(int vkey)
{
	if (vkey == DVL_VK_SNAPSHOT)
		CaptureScreen();
}

static void ClosePanels()
{
	if (PANELS_COVER) {
		if (!chrflag && !questlog && (invflag || sbookflag) && MouseX < 480 && MouseY < PANEL_TOP) {
			SetCursorPos(MouseX + 160, MouseY);
		} else if (!invflag && !sbookflag && (chrflag || questlog) && MouseX > 160 && MouseY < PANEL_TOP) {
			SetCursorPos(MouseX - 160, MouseY);
		}
	}
	invflag = FALSE;
	chrflag = FALSE;
	sbookflag = FALSE;
	questlog = FALSE;
}

bool PressEscKey()
{
	bool rv = false;

	if (doomflag) {
		doom_close();
		rv = true;
	}

	if (helpflag) {
		helpflag = FALSE;
		rv = true;
	}

	if (qtextflag) {
		qtextflag = FALSE;
		stream_stop();
		rv = true;
	}

	if (stextflag) {
		STextESC();
		rv = true;
	}

	if (msgflag) {
		msgdelay = 0;
		rv = true;
	}

	if (talkflag) {
		control_reset_talk();
		rv = true;
	}

	if (dropGoldFlag) {
		control_drop_gold(DVL_VK_ESCAPE);
		rv = true;
	}

	if (spselflag) {
		spselflag = FALSE;
		rv = true;
	}

	if (invflag || chrflag || sbookflag || questlog) {
		ClosePanels();
		rv = true;
	}

	return rv;
}

static void PressKey(int vkey)
{
	if (gmenu_presskeys(vkey) || control_presskeys(vkey)) {
		return;
	}

	if (deathflag) {
		if (sgnTimeoutCurs != CURSOR_NONE) {
			return;
		}
		if (vkey == DVL_VK_F9) {
			diablo_hotkey_msg(0);
		}
		if (vkey == DVL_VK_F10) {
			diablo_hotkey_msg(1);
		}
		if (vkey == DVL_VK_F11) {
			diablo_hotkey_msg(2);
		}
		if (vkey == DVL_VK_F12) {
			diablo_hotkey_msg(3);
		}
		if (vkey == DVL_VK_RETURN) {
			if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000)
				dx_reinit();
			else
				control_type_message();
		}
		if (vkey != DVL_VK_ESCAPE) {
			return;
		}
	}
	if (vkey == DVL_VK_ESCAPE) {
		if (!PressEscKey()) {
			track_repeat_walk(FALSE);
			gamemenu_on();
		}
		return;
	}

	if (sgnTimeoutCurs != CURSOR_NONE || dropGoldFlag) {
		return;
	}
	if (vkey == DVL_VK_PAUSE) {
		diablo_pause_game();
		return;
	}
	if (PauseMode == 2) {
		if ((vkey == DVL_VK_RETURN) && (GetAsyncKeyState(DVL_VK_MENU) & 0x8000))
			dx_reinit();
		return;
	}

	if (vkey == DVL_VK_RETURN) {
		if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000) {
			dx_reinit();
		} else if (stextflag) {
			STextEnter();
		} else if (questlog) {
			QuestlogEnter();
		} else {
			control_type_message();
		}
	} else if (vkey == DVL_VK_F1) {
		if (helpflag) {
			helpflag = FALSE;
		} else if (stextflag != STORE_NONE) {
			ClearPanel();
			AddPanelString("No help available", TRUE); /// BUGFIX: message isn't displayed
			AddPanelString("while in stores", TRUE);
			track_repeat_walk(FALSE);
		} else {
			invflag = FALSE;
			chrflag = FALSE;
			sbookflag = FALSE;
			spselflag = FALSE;
			if (qtextflag && leveltype == DTYPE_TOWN) {
				qtextflag = FALSE;
				stream_stop();
			}
			questlog = FALSE;
			automapflag = FALSE;
			msgdelay = 0;
			gamemenu_off();
			DisplayHelp();
			doom_close();
		}
	}
#ifdef _DEBUG
	else if (vkey == DVL_VK_F2) {
	}
#endif
#ifdef _DEBUG
	else if (vkey == DVL_VK_F3) {
		if (pcursitem != -1) {
			sprintf(
			    tempstr,
			    "IDX = %i  :  Seed = %i  :  CF = %i",
			    item[pcursitem].IDidx,
			    item[pcursitem]._iSeed,
			    item[pcursitem]._iCreateInfo);
			NetSendCmdString(1 << myplr, tempstr);
		}
		sprintf(tempstr, "Numitems : %i", numitems);
		NetSendCmdString(1 << myplr, tempstr);
	}
#endif
#ifdef _DEBUG
	else if (vkey == DVL_VK_F4) {
		PrintDebugQuest();
	}
#endif
	else if (vkey == DVL_VK_F5) {
		if (spselflag) {
			SetSpeedSpell(0);
			return;
		}
		ToggleSpell(0);
		return;
	} else if (vkey == DVL_VK_F6) {
		if (spselflag) {
			SetSpeedSpell(1);
			return;
		}
		ToggleSpell(1);
		return;
	} else if (vkey == DVL_VK_F7) {
		if (spselflag) {
			SetSpeedSpell(2);
			return;
		}
		ToggleSpell(2);
		return;
	} else if (vkey == DVL_VK_F8) {
		if (spselflag) {
			SetSpeedSpell(3);
			return;
		}
		ToggleSpell(3);
		return;
	} else if (vkey == DVL_VK_F9) {
		diablo_hotkey_msg(0);
	} else if (vkey == DVL_VK_F10) {
		diablo_hotkey_msg(1);
	} else if (vkey == DVL_VK_F11) {
		diablo_hotkey_msg(2);
	} else if (vkey == DVL_VK_F12) {
		diablo_hotkey_msg(3);
	} else if (vkey == DVL_VK_UP) {
		if (stextflag) {
			STextUp();
		} else if (questlog) {
			QuestlogUp();
		} else if (helpflag) {
			HelpScrollUp();
		} else if (automapflag) {
			AutomapUp();
		}
	} else if (vkey == DVL_VK_DOWN) {
		if (stextflag) {
			STextDown();
		} else if (questlog) {
			QuestlogDown();
		} else if (helpflag) {
			HelpScrollDown();
		} else if (automapflag) {
			AutomapDown();
		}
	} else if (vkey == DVL_VK_PRIOR) {
		if (stextflag) {
			STextPrior();
		}
	} else if (vkey == DVL_VK_NEXT) {
		if (stextflag) {
			STextNext();
		}
	} else if (vkey == DVL_VK_LEFT) {
		if (automapflag && !talkflag) {
			AutomapLeft();
		}
	} else if (vkey == DVL_VK_RIGHT) {
		if (automapflag && !talkflag) {
			AutomapRight();
		}
	} else if (vkey == DVL_VK_TAB) {
		DoAutoMap();
	} else if (vkey == DVL_VK_SPACE) {
		ClosePanels();
		helpflag = FALSE;
		spselflag = FALSE;
		if (qtextflag && leveltype == DTYPE_TOWN) {
			qtextflag = FALSE;
			stream_stop();
		}
		automapflag = FALSE;
		msgdelay = 0;
		gamemenu_off();
		doom_close();
	}
}

/**
 * @internal `return` must be used instead of `break` to be bin exact as C++
 */
static void PressChar(WPARAM vkey)
{
	if (gmenu_is_active() || control_talk_last_key(vkey) || sgnTimeoutCurs != CURSOR_NONE || deathflag) {
		return;
	}
	if ((char)vkey == 'p' || (char)vkey == 'P') {
		diablo_pause_game();
		return;
	}
	if (PauseMode == 2) {
		return;
	}
	if (doomflag) {
		doom_close();
		return;
	}
	if (dropGoldFlag) {
		control_drop_gold(vkey);
		return;
	}

	switch (vkey) {

	//Fluffy: For lightmap debugging
#ifdef LIGHTMAP_SUBTILE_EDITOR
	case 'w':
		if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000)
			subtileSelection -= 100;
		else if (GetAsyncKeyState(DVL_VK_CONTROL) & 0x8000)
			subtileSelection -= 10;
		else
			subtileSelection--;
		if (subtileSelection < 0)
			subtileSelection = 0;
		break;
	case 'e': {
		if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000)
			subtileSelection += 100;
		else if (GetAsyncKeyState(DVL_VK_CONTROL) & 0x8000)
			subtileSelection += 10;
		else
			subtileSelection++;
		if (subtileSelection >= lightInfo_subTilesSize)
			subtileSelection = lightInfo_subTilesSize - 1;
		break;
	}
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		if (lightInfo_subTiles)
			lightInfo_subTiles[subtileSelection] = vkey - '1';
		break;
	case 'S':
	case 's':
		Lightmap_SaveSubtileData();
		break;
#endif

	case 'G':
	case 'g':
		DecreaseGamma();
		return;
	case 'F':
	case 'f':
		IncreaseGamma();
		return;
	case 'I':
	case 'i':
		if (stextflag == STORE_NONE) {
			invflag = !invflag;
			if (!chrflag && !questlog && PANELS_COVER) {
				if (!invflag) { // We closed the invetory
					if (MouseX < 480 && MouseY < PANEL_TOP) {
						SetCursorPos(MouseX + 160, MouseY);
					}
				} else if (!sbookflag) { // We opened the invetory
					if (MouseX > 160 && MouseY < PANEL_TOP) {
						SetCursorPos(MouseX - 160, MouseY);
					}
				}
			}
			sbookflag = FALSE;
		}
		return;
	case 'C':
	case 'c':
		if (stextflag == STORE_NONE) {
			chrflag = !chrflag;
			if (!invflag && !sbookflag && PANELS_COVER) {
				if (!chrflag) { // We closed the character sheet
					if (MouseX > 160 && MouseY < PANEL_TOP) {
						SetCursorPos(MouseX - 160, MouseY);
					}
				} else if (!questlog) { // We opened the character sheet
					if (MouseX < 480 && MouseY < PANEL_TOP) {
						SetCursorPos(MouseX + 160, MouseY);
					}
				}
			}
			questlog = FALSE;
		}
		return;
	case 'Q':
	case 'q':
		if (stextflag == STORE_NONE) {
			if (!questlog) {
				StartQuestlog();
			} else {
				questlog = FALSE;
			}
			if (!invflag && !sbookflag && PANELS_COVER) {
				if (!questlog) { // We closed the quest log
					if (MouseX > 160 && MouseY < PANEL_TOP) {
						SetCursorPos(MouseX - 160, MouseY);
					}
				} else if (!chrflag) { // We opened the character quest log
					if (MouseX < 480 && MouseY < PANEL_TOP) {
						SetCursorPos(MouseX + 160, MouseY);
					}
				}
			}
			chrflag = FALSE;
		}
		return;
	case 'Z':
	case 'z':
		zoomflag = !zoomflag;
		CalcViewportGeometry();
		return;
#ifndef LIGHTMAP_SUBTILE_EDITOR
	case 'S':
	case 's':
		if (stextflag == STORE_NONE) {
			chrflag = FALSE;
			questlog = FALSE;
			invflag = FALSE;
			sbookflag = FALSE;
			if (!spselflag) {
				DoSpeedBook();
			} else {
				spselflag = FALSE;
			}
			track_repeat_walk(FALSE);
		}
		return;
#endif
	case 'B':
	case 'b':
		if (stextflag == STORE_NONE) {
			sbookflag = !sbookflag;
			if (!chrflag && !questlog && PANELS_COVER) {
				if (!sbookflag) { // We closed the invetory
					if (MouseX < 480 && MouseY < PANEL_TOP) {
						SetCursorPos(MouseX + 160, MouseY);
					}
				} else if (!invflag) { // We opened the invetory
					if (MouseX > 160 && MouseY < PANEL_TOP) {
						SetCursorPos(MouseX - 160, MouseY);
					}
				}
			}
			invflag = FALSE;
		}
		return;
	case '+':
	case '=':
		if (automapflag) {
			AutomapZoomIn();
		}
		return;
	case '-':
	case '_':
		if (automapflag) {
			AutomapZoomOut();
		}
		return;
	case 'v': {
		char pszStr[120];
		const char *difficulties[3] = {
			"Normal",
			"Nightmare",
			"Hell",
		};
		sprintf(pszStr, "%s, mode = %s", gszProductName, difficulties[gnDifficulty]);
		NetSendCmdString(1 << myplr, pszStr);
		return;
	}
	case 'V':
		NetSendCmdString(1 << myplr, gszVersionNumber);
		return;
#ifndef LIGHTMAP_SUBTILE_EDITOR
	case '!':
	case '1':
		if (sgOptions.Gameplay.bHotbar) //Fluffy
			Hotbar_UseSlot(0);
		else if (!plr[myplr].SpdList[0].isEmpty() && plr[myplr].SpdList[0]._itype != ITYPE_GOLD) {
			UseInvItem(myplr, INVITEM_BELT_FIRST);
		}
		return;
	case '@':
	case '2':
		if (sgOptions.Gameplay.bHotbar) //Fluffy
			Hotbar_UseSlot(1);
		else if (!plr[myplr].SpdList[1].isEmpty() && plr[myplr].SpdList[1]._itype != ITYPE_GOLD) {
			UseInvItem(myplr, INVITEM_BELT_FIRST + 1);
		}
		return;
	case '#':
	case '3':
		if (sgOptions.Gameplay.bHotbar) //Fluffy
			Hotbar_UseSlot(2);
		else if (!plr[myplr].SpdList[2].isEmpty() && plr[myplr].SpdList[2]._itype != ITYPE_GOLD) {
			UseInvItem(myplr, INVITEM_BELT_FIRST + 2);
		}
		return;
	case '$':
	case '4':
		if (sgOptions.Gameplay.bHotbar) //Fluffy
			Hotbar_UseSlot(3);
		else if (!plr[myplr].SpdList[3].isEmpty() && plr[myplr].SpdList[3]._itype != ITYPE_GOLD) {
			UseInvItem(myplr, INVITEM_BELT_FIRST + 3);
		}
		return;
	case '%':
	case '5':
		if (sgOptions.Gameplay.bHotbar) //Fluffy
			Hotbar_UseSlot(4);
		else if (!plr[myplr].SpdList[4].isEmpty() && plr[myplr].SpdList[4]._itype != ITYPE_GOLD) {
			UseInvItem(myplr, INVITEM_BELT_FIRST + 4);
		}
		return;
	case '^':
	case '6':
		if (sgOptions.Gameplay.bHotbar) //Fluffy
			Hotbar_UseSlot(5);
		else if (!plr[myplr].SpdList[5].isEmpty() && plr[myplr].SpdList[5]._itype != ITYPE_GOLD) {
			UseInvItem(myplr, INVITEM_BELT_FIRST + 5);
		}
		return;
	case '&':
	case '7':
		if (sgOptions.Gameplay.bHotbar) //Fluffy
			Hotbar_UseSlot(6);
		else if (!plr[myplr].SpdList[6].isEmpty() && plr[myplr].SpdList[6]._itype != ITYPE_GOLD) {
			UseInvItem(myplr, INVITEM_BELT_FIRST + 6);
		}
		return;
	case '*':
	case '8':
#ifdef _DEBUG
		if (debug_mode_key_inverted_v || debug_mode_key_w) {
			NetSendCmd(TRUE, CMD_CHEAT_EXPERIENCE);
			return;
		}
#endif
		if (sgOptions.Gameplay.bHotbar) //Fluffy
			Hotbar_UseSlot(7);
		else if (!plr[myplr].SpdList[7].isEmpty() && plr[myplr].SpdList[7]._itype != ITYPE_GOLD) {
			UseInvItem(myplr, INVITEM_BELT_FIRST + 7);
		}
		return;
#endif
#ifdef _DEBUG
	case ')':
	case '0':
		if (debug_mode_key_inverted_v) {
			if (arrowdebug > 2) {
				arrowdebug = 0;
			}
			if (arrowdebug == 0) {
				plr[myplr]._pIFlags &= ~ISPL_FIRE_ARROWS;
				plr[myplr]._pIFlags &= ~ISPL_LIGHT_ARROWS;
			}
			if (arrowdebug == 1) {
				plr[myplr]._pIFlags |= ISPL_FIRE_ARROWS;
			}
			if (arrowdebug == 2) {
				plr[myplr]._pIFlags |= ISPL_LIGHT_ARROWS;
			}
			arrowdebug++;
		}
		return;
	case ':':
		if (currlevel == 0 && debug_mode_key_w) {
			SetAllSpellsCheat();
		}
		return;
	case '[':
		if (currlevel == 0 && debug_mode_key_w) {
			TakeGoldCheat();
		}
		return;
	case ']':
		if (currlevel == 0 && debug_mode_key_w) {
			MaxSpellsCheat();
		}
		return;
	case 'a':
		if (debug_mode_key_inverted_v) {
			spelldata[SPL_TELEPORT].sTownSpell = 1;
			plr[myplr]._pSplLvl[plr[myplr]._pSpell]++;
		}
		return;
	case 'D':
		PrintDebugPlayer(TRUE);
		return;
	case 'd':
		PrintDebugPlayer(FALSE);
		return;
	case 'L':
	case 'l':
		if (debug_mode_key_inverted_v) {
			ToggleLighting();
		}
		return;
	case 'M':
		NextDebugMonster();
		return;
	case 'm':
		GetDebugMonster();
		return;
	case 'R':
	case 'r':
		sprintf(tempstr, "seed = %i", glSeedTbl[currlevel]);
		NetSendCmdString(1 << myplr, tempstr);
		sprintf(tempstr, "Mid1 = %i : Mid2 = %i : Mid3 = %i", glMid1Seed[currlevel], glMid2Seed[currlevel], glMid3Seed[currlevel]);
		NetSendCmdString(1 << myplr, tempstr);
		sprintf(tempstr, "End = %i", glEndSeed[currlevel]);
		NetSendCmdString(1 << myplr, tempstr);
		return;
	case 'T':
	case 't':
		if (debug_mode_key_inverted_v) {
			sprintf(tempstr, "PX = %i  PY = %i", plr[myplr]._px, plr[myplr]._py);
			NetSendCmdString(1 << myplr, tempstr);
			sprintf(tempstr, "CX = %i  CY = %i  DP = %i", cursmx, cursmy, dungeon[cursmx][cursmy]);
			NetSendCmdString(1 << myplr, tempstr);
		}
		return;
	case '|':
		if (currlevel == 0 && debug_mode_key_w) {
			GiveGoldCheat();
		}
		return;
#endif
	case 'h': //Fluffy: Toggle between normal and SDL UI rendering
		if (sgOptions.Graphics.bInitHwUIRendering)
			options_hwUIRendering = !options_hwUIRendering;
		if (!options_hwUIRendering)
			options_hwIngameRendering = false;
		return;
	case 'j': //Fluffy: Toggle between normal and SDL ingame rendering (this requires HW UI rendering to be true)
		if (sgOptions.Graphics.bInitHwIngameRendering)
			options_hwIngameRendering = !options_hwIngameRendering;
		if (!options_hwUIRendering)
			options_hwIngameRendering = false;
		return;
	case 'k': //Fluffy: Toggle between normal and lightmap lighting
		if (sgOptions.Graphics.bInitLightmapping)
			options_lightmapping = !options_lightmapping;
		return;
	case 'u': //Fluffy: Toggle hotbar on and off
		sgOptions.Gameplay.bHotbar = !sgOptions.Gameplay.bHotbar;
		CalculateBeltSlotPositions();
		return;
	}
}

static void GetMousePos(LPARAM lParam)
{
	MouseX = (short)(lParam & 0xffff);
	MouseY = (short)((lParam >> 16) & 0xffff);
}

void DisableInputWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case DVL_WM_KEYDOWN:
	case DVL_WM_KEYUP:
	case DVL_WM_CHAR:
	case DVL_WM_SYSKEYDOWN:
	case DVL_WM_SYSCOMMAND:
		return;
	case DVL_WM_MOUSEMOVE:
		GetMousePos(lParam);
		return;
	case DVL_WM_LBUTTONDOWN:
		if (sgbMouseDown != CLICK_NONE)
			return;
		sgbMouseDown = CLICK_LEFT;
		return;
	case DVL_WM_LBUTTONUP:
		if (sgbMouseDown != CLICK_LEFT)
			return;
		sgbMouseDown = CLICK_NONE;
		return;
	case DVL_WM_RBUTTONDOWN:
		if (sgbMouseDown != CLICK_NONE)
			return;
		sgbMouseDown = CLICK_RIGHT;
		return;
	case DVL_WM_RBUTTONUP:
		if (sgbMouseDown != CLICK_RIGHT)
			return;
		sgbMouseDown = CLICK_NONE;
		return;
	case DVL_WM_CAPTURECHANGED:
		sgbMouseDown = CLICK_NONE;
		return;
	}

	MainWndProc(uMsg, wParam, lParam);
}

void GM_Game(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case DVL_WM_KEYDOWN:
		PressKey(wParam);
		return;
	case DVL_WM_KEYUP:
		ReleaseKey(wParam);
		return;
	case DVL_WM_CHAR:
		PressChar(wParam);
		return;
	case DVL_WM_SYSKEYDOWN:
		if (PressSysKey(wParam))
			return;
		break;
	case DVL_WM_SYSCOMMAND:
		if (wParam == DVL_SC_CLOSE) {
			gbRunGame = FALSE;
			gbRunGameResult = FALSE;
			return;
		}
		break;
	case DVL_WM_MOUSEMOVE:
		GetMousePos(lParam);
		gmenu_on_mouse_move();
		return;
	case DVL_WM_LBUTTONDOWN:
		GetMousePos(lParam);
		if (sgbMouseDown == CLICK_NONE) {
			sgbMouseDown = CLICK_LEFT;
			track_repeat_walk(LeftMouseDown(wParam));
		}
		return;
	case DVL_WM_LBUTTONUP:
		GetMousePos(lParam);
		lastLeftMouseButtonAction = MOUSEACTION_NONE; //Fluffy
		if (sgbMouseDown == CLICK_LEFT) {
			sgbMouseDown = CLICK_NONE;
			LeftMouseUp(wParam);
			track_repeat_walk(FALSE);
		}
		return;
	case DVL_WM_RBUTTONDOWN:
		GetMousePos(lParam);
		if (sgbMouseDown == CLICK_NONE) {
			sgbMouseDown = CLICK_RIGHT;
			RightMouseDown();
		}
		return;
	/*case 'e': //Fluffy
		if (currlevel == 0 && debug_mode_key_w) {
			GiveGoldCheat();
			SetAllSpellsCheat();
		}
		return;*/
	case DVL_WM_RBUTTONUP:
		GetMousePos(lParam);
		lastRightMouseButtonAction = MOUSEACTION_NONE; //Fluffy
		if (sgbMouseDown == CLICK_RIGHT) {
			sgbMouseDown = CLICK_NONE;
		}
		return;
	case DVL_WM_CAPTURECHANGED:
		sgbMouseDown = CLICK_NONE;
		track_repeat_walk(FALSE);
		break;
	case WM_DIABNEXTLVL:
	case WM_DIABPREVLVL:
	case WM_DIABRTNLVL:
	case WM_DIABSETLVL:
	case WM_DIABWARPLVL:
	case WM_DIABTOWNWARP:
	case WM_DIABTWARPUP:
	case WM_DIABRETOWN:
		if (gbIsMultiplayer)
			pfile_write_hero();
		nthread_ignore_mutex(TRUE);
		PaletteFadeOut(8);
		sound_stop();
		music_stop();
		track_repeat_walk(FALSE);
		sgbMouseDown = CLICK_NONE;
		ShowProgress((interface_mode)uMsg);
		force_redraw = 255;
		DrawAndBlit();
		LoadPWaterPalette();
		if (gbRunGame)
			PaletteFadeIn(8);
		nthread_ignore_mutex(FALSE);
		gbGameLoopStartup = TRUE;
		return;
	}

	MainWndProc(uMsg, wParam, lParam);
}

void LoadLvlGFX()
{
	assert(!pDungeonCels);

	switch (leveltype) {
	case DTYPE_TOWN:
		if (gbIsHellfire) {
			pDungeonCels = LoadFileInMem("NLevels\\TownData\\Town.CEL", NULL);
			pMegaTiles = LoadFileInMem("NLevels\\TownData\\Town.TIL", NULL);
			pLevelPieces = LoadFileInMem("NLevels\\TownData\\Town.MIN", NULL);
		} else {
			pDungeonCels = LoadFileInMem("Levels\\TownData\\Town.CEL", NULL);
			pMegaTiles = LoadFileInMem("Levels\\TownData\\Town.TIL", NULL);
			pLevelPieces = LoadFileInMem("Levels\\TownData\\Town.MIN", NULL);
		}
		pSpecialCels = LoadFileInMem("Levels\\TownData\\TownS.CEL", NULL);
		break;
	case DTYPE_CATHEDRAL:
		if (currlevel < 21) {
			pDungeonCels = LoadFileInMem("Levels\\L1Data\\L1.CEL", NULL);
			pMegaTiles = LoadFileInMem("Levels\\L1Data\\L1.TIL", NULL);
			pLevelPieces = LoadFileInMem("Levels\\L1Data\\L1.MIN", NULL);
			pSpecialCels = LoadFileInMem("Levels\\L1Data\\L1S.CEL", NULL);
		} else {
			pDungeonCels = LoadFileInMem("NLevels\\L5Data\\L5.CEL", NULL);
			pMegaTiles = LoadFileInMem("NLevels\\L5Data\\L5.TIL", NULL);
			pLevelPieces = LoadFileInMem("NLevels\\L5Data\\L5.MIN", NULL);
			pSpecialCels = LoadFileInMem("NLevels\\L5Data\\L5S.CEL", NULL);
		}
		break;
	case DTYPE_CATACOMBS:
		pDungeonCels = LoadFileInMem("Levels\\L2Data\\L2.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\L2Data\\L2.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L2Data\\L2.MIN", NULL);
		pSpecialCels = LoadFileInMem("Levels\\L2Data\\L2S.CEL", NULL);
		break;
	case DTYPE_CAVES:
		if (currlevel < 17) {
			pDungeonCels = LoadFileInMem("Levels\\L3Data\\L3.CEL", NULL);
			pMegaTiles = LoadFileInMem("Levels\\L3Data\\L3.TIL", NULL);
			pLevelPieces = LoadFileInMem("Levels\\L3Data\\L3.MIN", NULL);
		} else {
			pDungeonCels = LoadFileInMem("NLevels\\L6Data\\L6.CEL", NULL);
			pMegaTiles = LoadFileInMem("NLevels\\L6Data\\L6.TIL", NULL);
			pLevelPieces = LoadFileInMem("NLevels\\L6Data\\L6.MIN", NULL);
		}
		pSpecialCels = LoadFileInMem("Levels\\L1Data\\L1S.CEL", NULL);
		break;
	case DTYPE_HELL:
		pDungeonCels = LoadFileInMem("Levels\\L4Data\\L4.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\L4Data\\L4.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L4Data\\L4.MIN", NULL);
		pSpecialCels = LoadFileInMem("Levels\\L2Data\\L2S.CEL", NULL);
		break;
	default:
		app_fatal("LoadLvlGFX");
	}
}

void LoadAllGFX()
{
	IncProgress();
	IncProgress();
	InitObjectGFX();
	IncProgress();
	InitMissileGFX();
	IncProgress();
}

/**
 * @param lvldir method of entry
 */
void CreateLevel(int lvldir)
{
	switch (leveltype) {
	case DTYPE_TOWN:
		CreateTown(lvldir);
		InitTownTriggers();
		LoadRndLvlPal(0);
		break;
	case DTYPE_CATHEDRAL:
		CreateL5Dungeon(glSeedTbl[currlevel], lvldir);
		InitL1Triggers();
		Freeupstairs();
		if (currlevel < 21) {
			LoadRndLvlPal(1);
		} else {
			LoadRndLvlPal(5);
		}
		break;
	case DTYPE_CATACOMBS:
		CreateL2Dungeon(glSeedTbl[currlevel], lvldir);
		InitL2Triggers();
		Freeupstairs();
		LoadRndLvlPal(2);
		break;
	case DTYPE_CAVES:
		CreateL3Dungeon(glSeedTbl[currlevel], lvldir);
		InitL3Triggers();
		Freeupstairs();
		if (currlevel < 17) {
			LoadRndLvlPal(3);
		} else {
			LoadRndLvlPal(6);
		}
		break;
	case DTYPE_HELL:
		CreateL4Dungeon(glSeedTbl[currlevel], lvldir);
		InitL4Triggers();
		Freeupstairs();
		LoadRndLvlPal(4);
		break;
	default:
		app_fatal("CreateLevel");
	}
}

static void UpdateMonsterLights()
{
	for (int i = 0; i < nummonsters; i++) {
		MonsterStruct *mon = &monster[monstactive[i]];
		if (mon->mlid != NO_LIGHT) {
			if (mon->mlid == plr[myplr]._plid) { // Fix old saves where some monsters had 0 instead of NO_LIGHT
				mon->mlid = NO_LIGHT;
				continue;
			}

			LightListStruct *lid = &LightList[mon->mlid];
			if (mon->_mx != lid->_lx || mon->_my != lid->_ly) {
				ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
			}
		}
	}
}

void LoadGameLevel(BOOL firstflag, int lvldir)
{
	int i, j;
	BOOL visited;

	if (setseed)
		glSeedTbl[currlevel] = setseed;

	music_stop();
	if (pcurs > CURSOR_HAND && pcurs < CURSOR_FIRSTITEM) {
		NewCursor(CURSOR_HAND);
	}
	SetRndSeed(glSeedTbl[currlevel]);
	IncProgress();
	MakeLightTable();
	LoadLvlGFX();
	IncProgress();

	if (firstflag) {
		InitInv();
		InitItemGFX();
		InitQuestText();

		int players = gbIsMultiplayer ? MAX_PLRS : 1;
		for (i = 0; i < players; i++)
			InitPlrGFXMem(i);

		InitStores();
		InitAutomapOnce();
		InitHelp();
	}

	SetRndSeed(glSeedTbl[currlevel]);

	if (leveltype == DTYPE_TOWN)
		SetupTownStores();

	IncProgress();
	InitAutomap();

	if (leveltype != DTYPE_TOWN && lvldir != ENTRY_LOAD) {
		InitLighting();
		InitVision();
	}

	InitLevelMonsters();
	IncProgress();

	if (!setlevel) {
		CreateLevel(lvldir);
		IncProgress();
		FillSolidBlockTbls();
		SetRndSeed(glSeedTbl[currlevel]);

		if (leveltype != DTYPE_TOWN) {
			GetLevelMTypes();
			InitThemes();
			LoadAllGFX();
		} else {
			IncProgress();
			IncProgress();
			InitMissileGFX();
			IncProgress();
			IncProgress();
		}

		IncProgress();

		if (lvldir == ENTRY_RTNLVL)
			GetReturnLvlPos();
		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();

		IncProgress();

		for (i = 0; i < MAX_PLRS; i++) {
			if (plr[i].plractive && currlevel == plr[i].plrlevel) {
				InitPlayerGFX(i);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(i, firstflag);
			}
		}

		PlayDungMsgs();
		InitMultiView();
		IncProgress();

		visited = FALSE;
		int players = gbIsMultiplayer ? MAX_PLRS : 1;
		for (i = 0; i < players; i++) {
			if (plr[i].plractive)
				visited = visited || plr[i]._pLvlVisited[currlevel];
		}

		SetRndSeed(glSeedTbl[currlevel]);

		if (leveltype != DTYPE_TOWN) {
			if (firstflag || lvldir == ENTRY_LOAD || !plr[myplr]._pLvlVisited[currlevel] || gbIsMultiplayer) {
				HoldThemeRooms();
				glMid1Seed[currlevel] = GetRndSeed();
				InitMonsters();
				glMid2Seed[currlevel] = GetRndSeed();
				IncProgress();
				InitObjects();
				InitItems();
				if (currlevel < 17)
					CreateThemeRooms();
				IncProgress();
				glMid3Seed[currlevel] = GetRndSeed();
				InitMissiles();
				InitDead();
				glEndSeed[currlevel] = GetRndSeed();

				if (gbIsMultiplayer)
					DeltaLoadLevel();

				IncProgress();
				SavePreLighting();
			} else {
				HoldThemeRooms();
				InitMonsters();
				InitMissiles();
				InitDead();
				IncProgress();
				LoadLevel();
				IncProgress();
			}
		} else {
			for (i = 0; i < MAXDUNX; i++) {
				for (j = 0; j < MAXDUNY; j++)
					dFlags[i][j] |= BFLAG_LIT;
			}

			InitTowners();
			InitItems();
			InitMissiles();
			IncProgress();

			if (!firstflag && lvldir != ENTRY_LOAD && plr[myplr]._pLvlVisited[currlevel] && !gbIsMultiplayer)
				LoadLevel();
			if (gbIsMultiplayer)
				DeltaLoadLevel();

			IncProgress();
		}
		if (!gbIsMultiplayer)
			ResyncQuests();
		else
			ResyncMPQuests();
	} else {
		LoadSetMap();
		IncProgress();
		GetLevelMTypes();
		IncProgress();
		InitMonsters();
		IncProgress();
		InitMissileGFX();
		IncProgress();
		InitDead();
		IncProgress();
		FillSolidBlockTbls();
		IncProgress();

		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();
		IncProgress();

		for (i = 0; i < MAX_PLRS; i++) {
			if (plr[i].plractive && currlevel == plr[i].plrlevel) {
				InitPlayerGFX(i);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(i, firstflag);
			}
		}
		IncProgress();

		InitMultiView();
		IncProgress();

		if (firstflag || lvldir == ENTRY_LOAD || !plr[myplr]._pSLvlVisited[setlvlnum]) {
			InitItems();
			SavePreLighting();
		} else {
			LoadLevel();
		}

		InitMissiles();
		IncProgress();
	}

	if (sgOptions.Graphics.bInitLightmapping) //Fluffy: Load subtile data
		Lightmap_LoadSubtileData();

	SyncPortals();

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && plr[i].plrlevel == currlevel && (!plr[i]._pLvlChanging || i == myplr)) {
			if (plr[i]._pHitPoints > 0) {
				if (!gbIsMultiplayer)
					dPlayer[plr[i]._px][plr[i]._py] = i + 1;
				else
					SyncInitPlrPos(i);
			} else {
				dFlags[plr[i]._px][plr[i]._py] |= BFLAG_DEAD_PLAYER;
			}
		}
	}

	SetDungeonMicros();

	InitLightMax();
	IncProgress();
	IncProgress();

	if (firstflag) {
		InitControlPan();
	}
	IncProgress();
	UpdateMonsterLights();
	if (leveltype != DTYPE_TOWN) {
		ProcessLightList();
		ProcessVisionList();
	}

	if (currlevel >= 21) {
		if (currlevel == 21) {
			items_427ABA(CornerStone.x, CornerStone.y);
		}
		if (quests[Q_NAKRUL]._qactive == QUEST_DONE && currlevel == 24) {
			objects_454BA8();
		}
	}

	if (currlevel >= 17)
		music_start(currlevel > 20 ? TMUSIC_L5 : TMUSIC_L6);
	else
		music_start(leveltype);

	while (!IncProgress())
		;

	if (!gbIsSpawn && setlevel && setlvlnum == SL_SKELKING && quests[Q_SKELKING]._qactive == QUEST_ACTIVE)
		PlaySFX(USFX_SKING1);

	//Fluffy: Load various CELs as SDL textures here
	if (sgOptions.Graphics.bInitHwUIRendering) { 
		if (firstflag) {
			Texture_ConvertCEL_SingleFrame(pInvCels, TEXTURE_INVENTORY, SPANEL_WIDTH); //Inventory texture
			LoadQuestDialogueTextures();
			Texture_ConvertCEL_SingleFrame(pSTextBoxCels, TEXTURE_TEXTBOX2, 271); //Narrow version of text box 2
			Texture_ConvertCEL_MultipleFrames(pSPentSpn2Cels, TEXTURE_SPINNINGPENTAGRAM2, 12); //Tiny spinning pentagram
			Texture_ConvertCEL_MultipleFrames(pSTextSlidCels, TEXTURE_DYNAMICWINDOW, 12); //Textures for dynamic window creation

			//Item textures
			int itemTypes = gbIsHellfire ? ITEMTYPES : 35;
			for (int i = 0; i < itemTypes; i++) {
				Texture_ConvertCEL_MultipleFrames(itemanims[i], TEXTURE_ITEMS + i, 96, -1, true);
			}
		}

		if (sgOptions.Graphics.bInitHwIngameRendering) {
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES, TEXTURE_DUNGEONTILES_DUNGEONPIECES, pLevelPieces);
			//Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES);
			Texture_ConvertCEL_MultipleFrames(pSpecialCels, TEXTURE_DUNGEONTILES_SPECIAL, 64, -1, currlevel == 0 ? false : true); //The town special CEL doens't have frame header (TODO: Can we replace 64 with a reference?)

			//Fluffy debug: Testing optimization
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES_LEFTFOLIAGE);
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES_RIGHTFOLIAGE);
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES_LEFTMASK);
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES_RIGHTMASK);
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES_LEFTMASKINVERTED);
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES_RIGHTMASKINVERTED);
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES_LEFTMASKOPAQUE);
			Texture_ConvertCEL_DungeonTiles(pDungeonCels, TEXTURE_DUNGEONTILES_RIGHTMASKOPAQUE);
		}
	}
}

static void game_logic()
{
	//Fluffy: Calculate delta between current and previous gameplay tick
	unsigned long long curTime = SDL_GetPerformanceCounter();
	if (frame_timeOfPreviousGamePlayTick != 0)
		frame_gameplayTickDelta = (double)((curTime - frame_timeOfPreviousGamePlayTick) * 1000) / SDL_GetPerformanceFrequency();
	frame_timeOfPreviousGamePlayTick = curTime;

	if (!ProcessInput()) {
		return;
	}
	if (gbProcessPlayers) { //gbProcessPlayers is almost always true. Set to false if Diablo dies
		ProcessPlayers();
	}
	if (leveltype != DTYPE_TOWN) {
		ProcessMonsters();
		ProcessObjects();
		ProcessMissiles();
		ProcessItems();
		ProcessLightList();
		ProcessVisionList();
	} else {
		ProcessTowners();
		ProcessItems();
		ProcessMissiles();
	}

#ifdef _DEBUG
	if (debug_mode_key_inverted_v && GetAsyncKeyState(DVL_VK_SHIFT) & 0x8000) {
		ScrollView();
	}
#endif

	sound_update();
	ClearPlrMsg();
	CheckTriggers();
	CheckQuests();
	force_redraw |= 1;
	pfile_update(false);

	plrctrls_after_game_logic();

	//Fluffy: Update gameplayTickCount and its progress value
	gameplayTickCount_progress++;
	if (gameplayTickCount_progress >= gSpeedMod) {
		gameplayTickCount++;
		gameplayTickCount_progress = 0;
	}
}

static void timeout_cursor(BOOL bTimeout)
{
	if (bTimeout) {
		if (sgnTimeoutCurs == CURSOR_NONE && sgbMouseDown == CLICK_NONE) {
			sgnTimeoutCurs = pcurs;
			multi_net_ping();
			ClearPanel();
			AddPanelString("-- Network timeout --", TRUE);
			AddPanelString("-- Waiting for players --", TRUE);
			NewCursor(CURSOR_HOURGLASS);
			force_redraw = 255;
		}
		scrollrt_draw_game_screen(TRUE);
	} else if (sgnTimeoutCurs != CURSOR_NONE) {
		NewCursor(sgnTimeoutCurs);
		sgnTimeoutCurs = CURSOR_NONE;
		ClearPanel();
		force_redraw = 255;
	}
}

/**
 * @param bStartup Process additional ticks before returning
 */
void game_loop(BOOL bStartup)
{
	/*
	- bStartup is only true during the very first gameplay tick in the session
	- nthread_has_500ms_passed() is ignored in this function while playing singleplayer (because of the gbMaxPlayers == 1 check)
	- multi_handle_delta() always returns true in singleplayer
	*/

	int i;

	i = bStartup ? gnTickRate * 3 : 3;

	while (i--) {
		if (!multi_handle_delta()) {
			timeout_cursor(TRUE);
			break;
		} else {
			timeout_cursor(FALSE);
			game_logic();
		}
		if (!gbRunGame || !gbIsMultiplayer || !nthread_has_500ms_passed())
			break;
	}
}

void diablo_color_cyc_logic()
{
	if (!sgOptions.Graphics.bColorCycling)
		return;

	if (leveltype == DTYPE_HELL) {
		lighting_color_cycling();
	} else if (currlevel >= 21) {
		palette_update_crypt();
	} else if (currlevel >= 17) {
		palette_update_hive();
	} else if (leveltype == DTYPE_CAVES) {
		palette_update_caves();
	}
}

DEVILUTION_END_NAMESPACE
