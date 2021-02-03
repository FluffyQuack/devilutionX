/**
 * @file diablo.h
 *
 * Interface of the main game initialization functions.
 */
#ifndef __DIABLO_H__
#define __DIABLO_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern SDL_Window *ghMainWnd;
extern DWORD glSeedTbl[NUMLEVELS];
extern int gnLevelTypeTbl[NUMLEVELS];
extern int MouseX;
extern int MouseY;
extern BOOL gbRunGame;
extern BOOL gbRunGameResult;
extern BOOL zoomflag;
extern BOOL gbProcessPlayers;
extern BOOL gbLoadGame;
extern HINSTANCE ghInst;
extern int DebugMonsters[10];
extern BOOLEAN cineflag;
extern int force_redraw;
extern BOOL visiondebug;
/* These are defined in fonts.h */
extern BOOL was_fonts_init;
extern void FontsCleanup();
/** unused */
extern BOOL light4flag;
extern BOOL leveldebug;
extern BOOL monstdebug;
/** unused */
extern int debugmonsttypes;
extern int PauseMode;
extern BOOLEAN UseTheoQuest;
extern BOOLEAN UseCowFarmer;
extern BOOLEAN UseNestArt;
extern BOOLEAN UseBardTest;
extern BOOLEAN UseBarbarianTest;
extern BOOLEAN UseMultiTest;
extern char sgbMouseDown;
extern int ticks_per_sec;
extern unsigned long long tick_delay_highResolution; //Fluffy
extern int gSpeedMod;
extern int gMonsterSpeedMod;

//Fluffy: New global variables which are updated when loading config file (gameplay-changing ones are updated via network if we joined a network game)
extern BOOL gameSetup_fastWalkInTown;
extern BOOL gameSetup_allowAttacksInTown;
extern BOOL options_transparency;
extern BOOL options_opaqueWallsUnlessObscuring;
extern BOOL options_opaqueWallsWithBlobs;
extern BOOL options_opaqueWallsWithSilhouette;

void FreeGameMem();
BOOL StartGame(BOOL bNewGame, BOOL bSinglePlayer);
void diablo_quit(int exitStatus);
int DiabloMain(int argc, char **argv);
BOOL TryIconCurs();
void diablo_pause_game();
BOOL PressEscKey();
void DisableInputWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void GM_Game(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void LoadGameLevel(BOOL firstflag, int lvldir);
void game_loop(BOOL bStartup);
void diablo_color_cyc_logic();

/* rdata */

extern BOOL fullscreen;
extern int showintrodebug;
#ifdef _DEBUG
extern int questdebug;
extern int debug_mode_key_w;
extern int debug_mode_key_inverted_v;
extern int debug_mode_dollar_sign;
extern int debug_mode_key_d;
extern int debug_mode_key_i;
extern int dbgplr;
extern int dbgqst;
extern int dbgmon;
#endif
extern int frameflag;
extern int frameend;
extern int framerate;
extern unsigned long long framestart; //Fluffy: Gave this higher precision
extern unsigned long long frame_timeOfPreviousGamePlayTick; //Fluffy
extern unsigned long long frame_timeOfPreviousFrameRender; //Fluffy
extern double frame_gameplayTickDelta; //Fluffy
extern double frame_renderDelta; //Fluffy
extern BOOL FriendlyMode;

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DIABLO_H__ */
