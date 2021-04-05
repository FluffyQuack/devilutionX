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

static bool RepeatLeftMouseAttackAction() //Fluffy
{
	if (!(lastLeftMouseButtonAction == MOUSEACTION_ATTACK || lastLeftMouseButtonAction == MOUSEACTION_ATTACK_MONSTERTARGET || lastLeftMouseButtonAction == MOUSEACTION_ATTACK_PLAYERTARGET) || pcurs != CURSOR_HAND || sgbMouseDown != CLICK_LEFT)
		return false;

	//Repeat action if it's been X duration since the attack or spell cast
	unsigned long long currentTime = SDL_GetPerformanceCounter();
	if (currentTime - lastLeftMouseButtonTime > SDL_GetPerformanceFrequency() / 5) { //Check if it's been at least 200ms
		if (lastLeftMouseButtonAction == MOUSEACTION_ATTACK) {
			if (plr[myplr]._pwtype == WT_RANGED)
				NetSendCmdLoc(TRUE, CMD_RATTACKXY, cursmx, cursmy);
			else
				NetSendCmdLoc(TRUE, CMD_SATTACKXY, cursmx, cursmy);
		} else if (lastLeftMouseButtonAction == MOUSEACTION_ATTACK_MONSTERTARGET && pcursmonst != -1) {
			if (plr[myplr]._pwtype == WT_RANGED)
				NetSendCmdParam1(TRUE, CMD_RATTACKID, pcursmonst);
			else
				NetSendCmdParam1(TRUE, CMD_ATTACKID, pcursmonst);
		} else if (lastLeftMouseButtonAction == MOUSEACTION_ATTACK_PLAYERTARGET && pcursplr != -1 && !FriendlyMode) {
			if (plr[myplr]._pwtype == WT_RANGED)
				NetSendCmdParam1(TRUE, CMD_RATTACKPID, pcursplr);
			else
				NetSendCmdParam1(TRUE, CMD_ATTACKPID, pcursplr);
		}
	}
	return true;
}

static bool RepeatRightMouseAction() //Fluffy
{
	if (!(lastRightMouseButtonAction == MOUSEACTION_SPELL || lastRightMouseButtonAction == MOUSEACTION_ATTACK) || pcurs != CURSOR_HAND || sgbMouseDown != CLICK_RIGHT)
		return false;

	//Repeat action if it's been X duration since the attack or spell cast
	unsigned long long currentTime = SDL_GetPerformanceCounter();
	if (currentTime - lastRightMouseButtonTime > SDL_GetPerformanceFrequency() / 5) //Check if it's been at least 200ms
		CheckPlrSpell(true);
	return true;
}

void track_process()
{
	if (options_holdToAttack) { //Fluffy
		if (RepeatLeftMouseAttackAction())
			return;

		if (RepeatRightMouseAction())
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
