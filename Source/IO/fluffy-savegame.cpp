//Fluffy: For extended information in the savegame

#include "..\all.h"

DEVILUTION_BEGIN_NAMESPACE

/* Version numbers
* 1 = First version
* 2 = Save camera offset and direction
*/

#define SAVEGAME_VERSION 2
#define SAVEGAME_MAGIC 'GVSF'

static int version = 0;

static void SaveInt(BYTE *buffer, DWORD *pos, int val)
{
	*((int *)&buffer[*pos]) = val;
	*pos += 4;
}

static void SaveBoolean(BYTE *buffer, DWORD *pos, BOOLEAN val)
{
	*((BOOL *)&buffer[*pos]) = val;
	*pos += 1;
}

void SaveGame_ExtendedData()
{
	//Calculate size of buffer
	unsigned int bufferSize = 0;
	bufferSize += 4; //Magic
	bufferSize += 4;  //Version number
	bufferSize += 12; //Three ints we use to figure out if this savegame is valid
	bufferSize += 4; //gSpeedMod
	bufferSize += 4; //gMonsterSpeedMod
	bufferSize += nummonsters * (4); //tickCount for every monster
	bufferSize += 1 * (1 + 4 + 4 + 4 + 4); //walkedLastTick, _pNFrames_c, _pNWidth_c, _pWFrames_c, and _pWWidth_c
	bufferSize += nummissiles * (4); //tickCount for every missile
	bufferSize += numitems * (4); //iAnimCnt for every item
	bufferSize += 4 + 4 + 4 + 4 + 4; //ScrollInfo.pxoffDiff, ScrollInfo.pyoffDiff, ScrollInfo._sdir, ScrollInfo._sxoff, ScrollInfo._syoff

	//Allocate memory
	BYTE *buffer;
	int initialEncodedSize = codec_get_encoded_len(bufferSize);
	buffer = DiabloAllocPtr(initialEncodedSize);

	//Start adding data
	DWORD pos = 0;
	SaveInt(buffer, &pos, SAVEGAME_MAGIC);
	SaveInt(buffer, &pos, SAVEGAME_VERSION);
	SaveInt(buffer, &pos, plr[myplr]._px);
	SaveInt(buffer, &pos, plr[myplr]._py);
	SaveInt(buffer, &pos, plr[myplr]._pExperience);
	SaveInt(buffer, &pos, gSpeedMod);
	SaveInt(buffer, &pos, gMonsterSpeedMod);
	for (unsigned int i = 0; i < nummonsters; i++) {
		MonsterStruct *mon = &monster[monstactive[i]];
		SaveInt(buffer, &pos, mon->tickCount);
	}
	{
		PlayerStruct *plPtr = &plr[myplr];
		SaveBoolean(buffer, &pos, plPtr->walkedLastTick);
		SaveInt(buffer, &pos, plPtr->_pNFrames_c);
		SaveInt(buffer, &pos, plPtr->_pNWidth_c);
		SaveInt(buffer, &pos, plPtr->_pWFrames_c);
		SaveInt(buffer, &pos, plPtr->_pWWidth_c);
	}
	for (unsigned int i = 0; i < nummissiles; i++) {
		MissileStruct *mis = &missile[missileactive[i]];
		SaveInt(buffer, &pos, mis->tickCount);
	}
	for (unsigned int i = 0; i < numitems; i++) {
		ItemStruct *itm = &item[itemactive[i]];
		SaveInt(buffer, &pos, itm->iAnimCnt);
	}
	SaveInt(buffer, &pos, ScrollInfo.pxoffDiff);
	SaveInt(buffer, &pos, ScrollInfo.pyoffDiff);
	SaveInt(buffer, &pos, ScrollInfo._sdir);
	SaveInt(buffer, &pos, ScrollInfo._sxoff);
	SaveInt(buffer, &pos, ScrollInfo._syoff);
	assert(pos == bufferSize);

	//Save to file
	int finalEncodedSize = codec_get_encoded_len(pos);
	char szName[MAX_PATH];
	sprintf(szName, "FluffyModSaveGame");
	pfile_write_save_file(szName, buffer, pos, finalEncodedSize);
	mem_free_dbg(buffer);
}

static void ReadInt(BYTE *buffer, unsigned int *pos, int *val)
{
	*val = *((int *)&buffer[*pos]);
	*pos += 4;
}

static void ReadBoolean(BYTE *buffer, unsigned int *pos, BOOLEAN *val)
{
	*val = *((BOOL *)&buffer[*pos]);
	*pos += 1;
}

static void AdjustSpeedModValues(int from)
{
	double modifier = (double)from / gSpeedMod;
	{
		PlayerStruct *plPtr = &plr[myplr];
		plPtr->_pVar6 /= modifier;
		plPtr->_pVar7 /= modifier;
		plPtr->_pVar8 /= modifier;
		plPtr->_pAnimCnt /= modifier;
	}
	for (unsigned int i = 0; i < nummissiles; i++) {
		MissileStruct *mis = &missile[missileactive[i]];
		mis->_miAnimCnt /= modifier;
		mis->_mitxoff /= modifier;
		mis->_mityoff /= modifier;
		mis->tickCount /= modifier;
	}
	for (unsigned int i = 0; i < nobjects; i++) {
		ObjectStruct *obj = &object[objectactive[i]];
		obj->_oAnimCnt /= modifier;
	}
	for (unsigned int i = 0; i < numitems; i++) {
		ItemStruct *itm = &item[itemactive[i]];
		itm->iAnimCnt /= modifier;
	}

	ScrollInfo.pxoffDiff /= modifier;
	ScrollInfo.pyoffDiff /= modifier;
	ScrollInfo._sxoff /= modifier;
	ScrollInfo._syoff /= modifier;
}

static void AdjustMonsterSpeedModValues(int from)
{
	double modifier = (double)from / gMonsterSpeedMod;

	for (unsigned int i = 0; i < nummonsters; i++) {
		MonsterStruct *mon = &monster[monstactive[i]];
		mon->_mVar6 /= modifier;
		mon->_mVar7 /= modifier;
		mon->_mVar8 /= modifier;
		mon->_mAnimCnt /= modifier;
		mon->tickCount /= modifier;
	}
}

static void ExtendedDataMissing() //This is called if extended data is missing
{
	//Set a bunch of values to defaults
	{
		PlayerStruct *plPtr = &plr[myplr];
		plPtr->walkedLastTick = 0;
	}
	for (unsigned int i = 0; i < nummissiles; i++) {
		MissileStruct *mis = &missile[missileactive[i]];
		mis->tickCount = 0;
	}
	for (unsigned int i = 0; i < numitems; i++) {
		ItemStruct *itm = &item[itemactive[i]];
		itm->iAnimCnt = 0;
	}
	for (unsigned int i = 0; i < nummonsters; i++) {
		MonsterStruct *mon = &monster[monstactive[i]];
		mon->tickCount = 0;
	}

	ScrollInfo.pxoffDiff = 0;
	ScrollInfo.pyoffDiff = 0;

	if (gSpeedMod > 1)
		AdjustSpeedModValues(1);
	if (gMonsterSpeedMod > 1)
		AdjustMonsterSpeedModValues(1);
}

void LoadGame_ExtendedData()
{
	//Load savegame
	char szName[MAX_PATH];
	sprintf(szName, "FluffyModSaveGame");

	if (!pfile_CheckIfFileExists(szName)) {
		ExtendedDataMissing();
		return;
	}
	BYTE *buffer;
	DWORD dwLen;
	buffer = pfile_read(szName, &dwLen);

	//Verify magic
	DWORD pos = 0;
	int magic;
	ReadInt(buffer, &pos, &magic);
	if (magic != SAVEGAME_MAGIC) {
		mem_free_dbg(buffer);
		ExtendedDataMissing();
		return;
	}

	//Read version
	int version;
	ReadInt(buffer, &pos, &version);
	if (version > SAVEGAME_VERSION) {
		mem_free_dbg(buffer);
		ExtendedDataMissing();
		return;
	}

	//Figure out if this savegame matches the normal savegame data (this is trying to avoid a very obscure bug where a user saves with FluffyMod, then loads and saves in DevilutionX, and then tries to load the same savegame in FluffyMod which would then have invalid extended data)
	int px, py, pExperience;
	ReadInt(buffer, &pos, &px);
	ReadInt(buffer, &pos, &py);
	ReadInt(buffer, &pos, &pExperience);
	if (px != plr[myplr]._px || py != plr[myplr]._py || pExperience != plr[myplr]._pExperience) {
		mem_free_dbg(buffer);
		ExtendedDataMissing();
		return;
	}

	//Read data (we might read more or less depending on the version)
	int saveSpeedMod;
	int saveMonsterSpeedMod;
	ReadInt(buffer, &pos, &saveSpeedMod);
	ReadInt(buffer, &pos, &saveMonsterSpeedMod);
	for (unsigned int i = 0; i < nummonsters; i++) {
		MonsterStruct *mon = &monster[monstactive[i]];
		ReadInt(buffer, &pos, &mon->tickCount);
	}
	{
		PlayerStruct *plPtr = &plr[myplr];
		ReadBoolean(buffer, &pos, &plPtr->walkedLastTick);
		ReadInt(buffer, &pos, &plPtr->_pNFrames_c);
		ReadInt(buffer, &pos, &plPtr->_pNWidth_c);
		ReadInt(buffer, &pos, &plPtr->_pWFrames_c);
		ReadInt(buffer, &pos, &plPtr->_pWWidth_c);
	}
	for (unsigned int i = 0; i < nummissiles; i++) {
		MissileStruct *mis = &missile[missileactive[i]];
		ReadInt(buffer, &pos, &mis->tickCount);
	}
	for (unsigned int i = 0; i < numitems; i++) {
		ItemStruct *itm = &item[itemactive[i]];
		ReadInt(buffer, &pos, &itm->iAnimCnt);
	}
	ReadInt(buffer, &pos, &ScrollInfo.pxoffDiff);
	ReadInt(buffer, &pos, &ScrollInfo.pyoffDiff);
	if (version >= 2) {
		ReadInt(buffer, &pos, &ScrollInfo._sdir);
		ReadInt(buffer, &pos, &ScrollInfo._sxoff);
		ReadInt(buffer, &pos, &ScrollInfo._syoff);
	}
	mem_free_dbg(buffer);

	//If the savegame has different speed modifiers, then we'll need to adjust some values
	if (gSpeedMod != saveSpeedMod)
		AdjustSpeedModValues(saveSpeedMod);
	if (gMonsterSpeedMod != saveMonsterSpeedMod)
		AdjustMonsterSpeedModValues(saveMonsterSpeedMod);
}

DEVILUTION_END_NAMESPACE
