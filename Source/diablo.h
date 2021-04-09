/**
 * @file diablo.h
 *
 * Interface of the main game initialization functions.
 */
#ifndef __DIABLO_H__
#define __DIABLO_H__

#include "pack.h"
#ifdef _DEBUG
#include "monstdat.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEFAULT_WIDTH
#define DEFAULT_WIDTH 640
#endif
#ifndef DEFAULT_HEIGHT
#define DEFAULT_HEIGHT 480
#endif

extern SDL_Window *ghMainWnd;
extern DWORD glSeedTbl[NUMLEVELS];
extern dungeon_type gnLevelTypeTbl[NUMLEVELS];
extern int MouseX;
extern int MouseY;
extern BOOL gbRunGame;
extern BOOL gbRunGameResult;
extern BOOL zoomflag;
extern BOOL gbProcessPlayers;
extern BOOL gbLoadGame;
extern BOOLEAN cineflag;
extern int force_redraw;
/* These are defined in fonts.h */
extern BOOL was_fonts_init;
extern void FontsCleanup();
extern BOOL light4flag;
extern int PauseMode;
extern bool gbTheoQuest;
extern bool gbCowQuest;
extern bool gbNestArt;
extern bool gbBard;
extern bool gbBarbarian;
extern char sgbMouseDown;
extern int gnTickRate;
extern WORD gnTickDelay;
extern unsigned long long tick_delay_highResolution; //Fluffy
extern unsigned int gameplayTickCount;
extern unsigned int gameplayTickCount_progress;
extern int gSpeedMod;
extern int gMonsterSpeedMod;

//Fluffy: New global variables which are updated when loading config file (gameplay-changing ones are updated via network if we joined a network game)
extern BOOL gameSetup_allowAttacksInTown;
extern BOOL gameSetup_safetyJog;
extern BOOL options_hwIngameRendering;
extern BOOL options_hwUIRendering;
extern BOOL options_lightmapping;

extern int lastLeftMouseButtonAction;
extern int lastRightMouseButtonAction;
extern unsigned long long lastLeftMouseButtonTime;
extern unsigned long long lastRightMouseButtonTime;

void FreeGameMem();
BOOL StartGame(BOOL bNewGame, BOOL bSinglePlayer);
[[noreturn]] void diablo_quit(int exitStatus);
int DiabloMain(int argc, char **argv);
BOOL TryIconCurs();
void diablo_pause_game();
bool PressEscKey();
void DisableInputWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
void GM_Game(UINT uMsg, WPARAM wParam, LPARAM lParam);
void LoadGameLevel(BOOL firstflag, int lvldir);
void game_loop(BOOL bStartup);
void diablo_color_cyc_logic();

/* rdata */

extern bool gbForceWindowed;
extern bool leveldebug;
#ifdef _DEBUG
extern bool monstdebug;
extern _monster_id DebugMonsters[10];
extern int debugmonsttypes;
extern bool visiondebug;
extern int questdebug;
extern bool debug_mode_key_w;
extern bool debug_mode_key_inverted_v;
extern bool debug_mode_dollar_sign;
extern bool debug_mode_key_d;
extern bool debug_mode_key_i;
extern int debug_mode_key_j;
#endif
extern unsigned long long frame_timeOfPreviousGamePlayTick; //Fluffy
extern unsigned long long frame_timeOfPreviousFrameRender; //Fluffy
extern double frame_gameplayTickDelta; //Fluffy
extern double frame_renderDelta; //Fluffy
extern bool gbFriendlyMode;
extern bool gbFriendlyFire;

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DIABLO_H__ */
