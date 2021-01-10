/**
 * @file track.cpp
 *
 * Implementation of functionality tracking what the mouse cursor is pointing at.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static BYTE sgbIsScrolling;
static DWORD sgdwLastWalk;
static BOOL sgbIsWalking;

void track_process()
{
	if (!sgbIsWalking)
		return;

	if (cursmx < 0 || cursmx >= MAXDUNX - 1 || cursmy < 0 || cursmy >= MAXDUNY - 1)
		return;

	//Fluffy: If fastwalk is allowed, then we should allow the player to repeat walking more often when holding down left click
	int moveProgress = plr[myplr]._pVar8;
	int minWaitForRepeatWalk = 300;
	if (gameSetup_fastWalkInTown && currlevel == 0) {
		moveProgress *= 2;
		minWaitForRepeatWalk /= 2;
	}

	if (moveProgress <= 6 && plr[myplr]._pmode != PM_STAND)
		return;

	if (cursmx != plr[myplr]._ptargx || cursmy != plr[myplr]._ptargy) {
		DWORD tick = SDL_GetTicks();
		if ((int)(tick - sgdwLastWalk) >= minWaitForRepeatWalk) {
			sgdwLastWalk = tick;
			NetSendCmdLoc(TRUE, CMD_WALKXY, cursmx, cursmy);
			if (!sgbIsScrolling)
				sgbIsScrolling = TRUE;
		}
	}
}

void track_repeat_walk(BOOL rep)
{
	if (sgbIsWalking == rep)
		return;

	sgbIsWalking = rep;
	if (rep) {
		sgbIsScrolling = FALSE;
		sgdwLastWalk = SDL_GetTicks() - 50;
		NetSendCmdLoc(TRUE, CMD_WALKXY, cursmx, cursmy);
	} else if (sgbIsScrolling) {
		sgbIsScrolling = FALSE;
	}
}

BOOL track_isscrolling()
{
	return sgbIsScrolling;
}

DEVILUTION_END_NAMESPACE
