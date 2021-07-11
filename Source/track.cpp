/**
 * @file track.cpp
 *
 * Implementation of functionality tracking what the mouse cursor is pointing at.
 */
#include "all.h"
#include "options.h" //Fluffy

DEVILUTION_BEGIN_NAMESPACE

namespace {

BYTE sgbIsScrolling;
Uint32 sgdwLastWalk;
bool sgbIsWalking;

}

static bool RepeatMouseAttack(bool leftButton) //Fluffy
{
	if (pcurs != CURSOR_HAND)
		return false;

	unsigned long long *timePressed;
	int lastAction;
	if (leftButton) {
		if (sgbMouseDown != CLICK_LEFT)
			return false;
		timePressed = &lastLeftMouseButtonTime;
		lastAction = lastLeftMouseButtonAction;
	} else {
		if (sgbMouseDown != CLICK_RIGHT)
			return false;
		timePressed = &lastRightMouseButtonTime;
		lastAction = lastRightMouseButtonAction;
	}

	if (lastAction != MOUSEACTION_ATTACK && lastAction != MOUSEACTION_ATTACK_MONSTERTARGET && lastAction != MOUSEACTION_ATTACK_PLAYERTARGET && lastAction != MOUSEACTION_SPELL && lastAction != MOUSEACTION_SPELL_COMPLAINEDABOUTMANA)
		return false;

	if (plr[myplr]._pmode != PM_DEATH && plr[myplr]._pmode != PM_QUIT && plr[myplr].destAction == ACTION_NONE && SDL_GetPerformanceCounter() - *timePressed >= SDL_GetPerformanceFrequency() / 5) { //Check if it's been at least 200ms
		*timePressed = SDL_GetPerformanceCounter();
		if (!leftButton) { 
			CheckPlrSpell(true);
		} else {
			bool rangedAttack = plr[myplr]._pwtype == WT_RANGED;
			switch (lastAction) {
			case MOUSEACTION_ATTACK:
				if (cursmx >= 0 && cursmx < MAXDUNX && cursmy >= 0 && cursmy < MAXDUNY)
					NetSendCmdLoc(TRUE, rangedAttack ? CMD_RATTACKXY : CMD_SATTACKXY, cursmx, cursmy);
				break;
			case MOUSEACTION_ATTACK_MONSTERTARGET:
				if (pcursmonst != -1)
					NetSendCmdParam1(TRUE, rangedAttack ? CMD_RATTACKID : CMD_ATTACKID, pcursmonst);
				break;
			case MOUSEACTION_ATTACK_PLAYERTARGET:
				if (pcursplr != -1 && !gbFriendlyMode)
					NetSendCmdParam1(TRUE, rangedAttack ? CMD_RATTACKPID : CMD_ATTACKPID, pcursplr);
				break;
			/*case MOUSEACTION_SPELL:
			case MOUSEACTION_SPELL_COMPLAINEDABOUTMANA:
				CheckPlrSpell(true);
				break;*/
			}
		}
	}

	return true;
}

void track_process()
{
	if (sgOptions.Gameplay.bHoldToAttack) { //Fluffy
		if (RepeatMouseAttack(true) || RepeatMouseAttack(false)) //Fluffy
			return;
	} 

	if (!sgbIsWalking)
		return;

	if (cursmx < 0 || cursmx >= MAXDUNX - 1 || cursmy < 0 || cursmy >= MAXDUNY - 1)
		return;

	//Fluffy: If fastwalk is allowed, then we should allow the player to repeat walking more often when holding down left click
	//We also scale some of these values using gSpeedMod to further change how often we're allowed to repeatedly walk
	int moveProgress = plr[myplr]._pVar8;
	int minWaitForRepeatWalk = ((tick_delay_highResolution / SDL_GetPerformanceFrequency()) * 6) / gSpeedMod;
	if (gameSetup_safetyJog) {
		if (plr[myplr].walking) {
			moveProgress *= 2;
			minWaitForRepeatWalk /= 2;
		}
	} else if (gbRunInTown && currlevel == 0) {
		moveProgress *= 2;
		minWaitForRepeatWalk /= 2;
	}

	if (moveProgress <= 6 * gSpeedMod && plr[myplr]._pmode != PM_STAND) //Fluffy
		return;

	if (cursmx != plr[myplr]._ptargx || cursmy != plr[myplr]._ptargy) {
		Uint32 tick = SDL_GetTicks();
		if ((int)(tick - sgdwLastWalk) >= minWaitForRepeatWalk) {
			sgdwLastWalk = tick;
			NetSendCmdLoc(true, CMD_WALKXY, cursmx, cursmy);
			if (!sgbIsScrolling)
				sgbIsScrolling = true;
		}
	}
}

void track_repeat_walk(bool rep)
{
	if (sgbIsWalking == rep)
		return;

	sgbIsWalking = rep;
	if (rep) {
		sgbIsScrolling = false;
		sgdwLastWalk = SDL_GetTicks() - (tick_delay_highResolution / SDL_GetPerformanceFrequency()); //Fluffy: Use high resolution tick delay
		NetSendCmdLoc(true, CMD_WALKXY, cursmx, cursmy);
	} else if (sgbIsScrolling) {
		sgbIsScrolling = false;
	}
}

bool track_isscrolling()
{
	return sgbIsScrolling;
}

DEVILUTION_END_NAMESPACE
