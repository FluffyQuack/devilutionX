
#include "control.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/selok.h"

namespace devilution {
namespace {
int mainmenu_attract_time_out; //seconds
DWORD dwAttractTicks;

std::vector<UiItemBase *> vecMainMenuDialog;
std::vector<UiListItem *> vecMenuItems;

_mainmenu_selections MainMenuResult;

void UiMainMenuSelect(int value)
{
	MainMenuResult = (_mainmenu_selections)vecMenuItems[value]->m_value;
}

void mainmenu_Esc()
{
	std::size_t last = vecMenuItems.size() - 1;
	if (SelectedItem == last) {
		UiMainMenuSelect(last);
	} else {
		SelectedItem = last;
	}
}

void mainmenu_Load(const char *name, void (*fnSound)(const char *file))
{
	gfnSoundFunction = fnSound;

	vecMenuItems.push_back(new UiListItem("Single Player", MAINMENU_SINGLE_PLAYER));
	vecMenuItems.push_back(new UiListItem("Multi Player", MAINMENU_MULTIPLAYER));
	vecMenuItems.push_back(new UiListItem("Replay Intro", MAINMENU_REPLAY_INTRO));
	vecMenuItems.push_back(new UiListItem("Support", MAINMENU_SHOW_SUPPORT));
	if (gbIsHellfire) {
		vecMenuItems.push_back(new UiListItem("Credits", MAINMENU_SHOW_CREDITS));
		vecMenuItems.push_back(new UiListItem("Exit Hellfire", MAINMENU_EXIT_DIABLO));
	} else {
		vecMenuItems.push_back(new UiListItem("Show Credits", MAINMENU_SHOW_CREDITS));
		vecMenuItems.push_back(new UiListItem("Exit Diablo", MAINMENU_EXIT_DIABLO));
	}

	if (!gbSpawned || gbIsHellfire) {
		if (gbIsHellfire)
			LoadArt("ui_art\\mainmenuw.pcx", &ArtBackgroundWidescreen);
		LoadBackgroundArt("ui_art\\mainmenu.pcx");
	} else {
		LoadBackgroundArt("ui_art\\swmmenu.pcx");
	}

	UiAddBackground(&vecMainMenuDialog);
	UiAddLogo(&vecMainMenuDialog);

	vecMainMenuDialog.push_back(new UiList(vecMenuItems, PANEL_LEFT + 64, (UI_OFFSET_Y + 192), 510, 43, UIS_HUGE | UIS_GOLD | UIS_CENTER));

	SDL_Rect rect = { 17, (Sint16)(gnScreenHeight - 36), 605, 21 };
	vecMainMenuDialog.push_back(new UiArtText(name, rect, UIS_SMALL));

	UiInitList(vecMenuItems.size(), nullptr, UiMainMenuSelect, mainmenu_Esc, vecMainMenuDialog, true);
}

void mainmenu_Free()
{
	ArtBackgroundWidescreen.Unload();
	ArtBackground.Unload();

	for (auto pUIItem : vecMainMenuDialog) {
		delete pUIItem;
	}
	vecMainMenuDialog.clear();

	for (auto pUIMenuItem : vecMenuItems) {
		delete pUIMenuItem;
	}
	vecMenuItems.clear();
}

} // namespace

void mainmenu_restart_repintro()
{
	dwAttractTicks = SDL_GetTicks() + mainmenu_attract_time_out * 1000;
}

bool UiMainMenuDialog(const char *name, _mainmenu_selections *pdwResult, void (*fnSound)(const char *file), int attractTimeOut)
{
	MainMenuResult = MAINMENU_NONE;
	while (MainMenuResult == MAINMENU_NONE) {
		mainmenu_attract_time_out = attractTimeOut;
		mainmenu_Load(name, fnSound);

		mainmenu_restart_repintro(); // for automatic starts

		while (MainMenuResult == MAINMENU_NONE) {
			UiClearScreen();
			UiPollAndRender();
			if (!gbSpawned && SDL_GetTicks() >= dwAttractTicks) {
				MainMenuResult = MAINMENU_ATTRACT_MODE;
			}
		}

		mainmenu_Free();

		if (gbSpawned && !gbIsHellfire && MainMenuResult == MAINMENU_REPLAY_INTRO) {
			UiSelOkDialog(nullptr, "The Diablo introduction cinematic is only available in the full retail version of Diablo. Visit https://www.gog.com/game/diablo to purchase.", true);
			MainMenuResult = MAINMENU_NONE;
		}
	}

	*pdwResult = MainMenuResult;
	return true;
}

} // namespace devilution
