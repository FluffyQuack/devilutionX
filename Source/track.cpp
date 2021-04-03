/**
 * @file track.cpp
 *
 * Implementation of functionality tracking what the mouse cursor is pointing at.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

namespace {

BYTE sgbIsScrolling;
Uint32 sgdwLastWalk;
bool sgbIsWalking;

}

static bool RepeatRightMouseAction() //Fluffy
{
	if (!(lastRightMouseButtonAction == MOUSEACTION_SPELL || lastRightMouseButtonAction == MOUSEACTION_ATTACK) || pcurs != CURSOR_HAND || sgbMouseDown != CLICK_RIGHT)
		return false;

	//Repeat action if it's been X duration since the attack or spell cast
	unsigned long long currentTime = SDL_GetPerformanceCounter();
	if (currentTime - lastRightMouseButtonTime > SDL_GetPerformanceFrequency() / 5) { //Check if it's been at least 200ms
		CheckPlrSpell(true);
		return true;
	}
	return false;
}

void track_process()
{
	if (RepeatRightMouseAction())
		return;

	if (!sgbIsWalking)
		return;

	if (cursmx < 0 || cursmx >= MAXDUNX - 1 || cursmy < 0 || cursmy >= MAXDUNY - 1)
		return;

	//Fluffy: If fastwalk is allowed, then we should allow the player to repeat walking more often when holding down left click
	//We also scale some of these values using gSpeedMod to further change how often we're allowed to repeatedly walk
	int moveProgress = plr[myplr]._pVar8;
	int minWaitForRepeatWalk = ((tick_delay_highResolution / SDL_GetPerformanceFrequency()) * 6) / gSpeedMod;
	if (gameSetup_fastWalkInTown && currlevel == 0) {
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
