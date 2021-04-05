//Fluffy: For loading various options from the game's config file

#include "../all.h"
#include "../../3rdParty/Storm/Source/storm.h"

//TODO: We could create a struct which contains all general options which can be defined in the config file to make them tidier

DEVILUTION_BEGIN_NAMESPACE

static void LoadBoolVariableFromConfig(char *name, BOOL *variable)
{
	int temp = *variable;
	if (SRegLoadValue("devilutionx", name, 0, &temp))
		*variable = (BOOL)temp;
	else
		SRegSaveValue("devilutionx", name, 0, temp);
}

void LoadOptionsFromConfig()
{
	if (!SRegLoadValue("devilutionx", "game speed", 0, &ticks_per_sec)) {
		SRegSaveValue("devilutionx", "game speed", 0, ticks_per_sec);
	}
	tick_delay_highResolution = SDL_GetPerformanceFrequency() / ticks_per_sec; //Fluffy

	//Fluffy: Load speed modifiers from config, but if they don't exist, then we calculate based on the tick rate
	if (!SRegLoadValue("devilutionx", "Game Simulation Speed Modifier", 0, &gSpeedMod)) {
		gSpeedMod = ticks_per_sec / 20;
	}
	if (!SRegLoadValue("devilutionx", "Monster Speed Modifier", 0, &gMonsterSpeedMod)) {
		gMonsterSpeedMod = ticks_per_sec / 20;
	}
	if (gSpeedMod < 1)
		gSpeedMod = 1;
	if (gMonsterSpeedMod < 1)
		gMonsterSpeedMod = 1;

	//Fluffy: Various new config toggles (gameplay-changing ones are updated via network)
	LoadBoolVariableFromConfig("Fast Walking In Town", &gameSetup_fastWalkInTown);
	LoadBoolVariableFromConfig("Allow Attacks In Town", &gameSetup_allowAttacksInTown);
	LoadBoolVariableFromConfig("Jog When Safe", &gameSetup_safetyJog);
	LoadBoolVariableFromConfig("Transparency", &options_transparency);
	LoadBoolVariableFromConfig("Nonobscuring Walls Are Opaque", &options_opaqueWallsUnlessObscuring);
	LoadBoolVariableFromConfig("Opaque Walls With Blobs", &options_opaqueWallsWithBlobs);
	LoadBoolVariableFromConfig("Opaque Walls With Silhouettes", &options_opaqueWallsWithSilhouette);
	LoadBoolVariableFromConfig("Hardware Rendering", &options_initHwRendering);
	options_hwRendering = options_initHwRendering;
	LoadBoolVariableFromConfig("Lightmapping", &options_initLightmapping);
	options_lightmapping = options_initLightmapping;
	if (!options_initHwRendering) {
		options_initLightmapping = false;
		options_lightmapping = false;
	}
	options_hwRendering = 0; //Fluffy TODO: Remove this once we've fully replaced all UI rendering with hw rendering
	LoadBoolVariableFromConfig("Animated UI Flasks", &options_animatedUIFlasks);

	LoadBoolVariableFromConfig("Durability Icon Gradual Change", &options_durabilityIconGradualChange);
	if (!SRegLoadValue("devilutionx", "Durability Icon Gold Value", 0, &options_durabilityIconGold)) {
		options_durabilityIconGold = 5;
	}
	if (!SRegLoadValue("devilutionx", "Durability Icon Red Value", 0, &options_durabilityIconRed)) {
		options_durabilityIconRed = 2;
	}
}

DEVILUTION_END_NAMESPACE
