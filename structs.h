/**
 * @file structs.h
 *
 * Various global structures.
 */

DEVILUTION_BEGIN_NAMESPACE

//////////////////////////////////////////////////
// control
//////////////////////////////////////////////////

typedef struct RECT32 {
	int x;
	int y;
	int w;
	int h;
} RECT32;

//////////////////////////////////////////////////
// items
//////////////////////////////////////////////////

typedef struct PLStruct {
	const char *PLName;
	int PLPower;
	int PLParam1;
	int PLParam2;
	char PLMinLvl;
	int PLIType;
	BYTE PLGOE;
	BOOL PLDouble;
	BOOL PLOk;
	int PLMinVal;
	int PLMaxVal;
	int PLMultVal;
} PLStruct;

typedef struct UItemStruct {
	char *UIName;
	char UIItemId;
	char UIMinLvl;
	char UINumPL;
	int UIValue;
	char UIPower1;
	int UIParam1;
	int UIParam2;
	char UIPower2;
	int UIParam3;
	int UIParam4;
	char UIPower3;
	int UIParam5;
	int UIParam6;
	char UIPower4;
	int UIParam7;
	int UIParam8;
	char UIPower5;
	int UIParam9;
	int UIParam10;
	char UIPower6;
	int UIParam11;
	int UIParam12;
} UItemStruct;

typedef struct ItemDataStruct {
	int iRnd;
	char iClass;
	char iLoc;
	int iCurs;
	char itype;
	char iItemId;
	char *iName;
	char *iSName;
	char iMinMLvl;
	int iDurability;
	int iMinDam;
	int iMaxDam;
	int iMinAC;
	int iMaxAC;
	char iMinStr;
	char iMinMag;
	char iMinDex;
	// item_special_effect
	int iFlags;
	// item_misc_id
	int iMiscId;
	// spell_id
	int iSpell;
	BOOL iUsable;
	int iValue;
	int iMaxValue;
} ItemDataStruct;

typedef struct ItemGetRecordStruct {
	int nSeed;
	unsigned short wCI;
	int nIndex;
	unsigned int dwTimestamp;
} ItemGetRecordStruct;

typedef struct ItemStruct {
	int _iSeed;
	WORD _iCreateInfo;
	int _itype;
	int _ix;
	int _iy;
	BOOL _iAnimFlag;
	unsigned char *_iAnimData; // PSX name -> ItemFrame
	int iAnimCnt; //Fluffy: Added this so we can slow down the animation using gSpeedMod
	int _iAnimLen;
	int _iAnimFrame;
	int _iAnimWidth;
	int _iAnimWidth2; // width 2?
	BOOL _iDelFlag;   // set when item is flagged for deletion, deprecated in 1.02
	char _iSelFlag;
	BOOL _iPostDraw;
	BOOL _iIdentified;
	char _iMagical;
	char _iName[64];
	char _iIName[64];
	char _iLoc;
	// item_class enum
	char _iClass;
	int _iCurs;
	int _ivalue;
	int _iIvalue;
	int _iMinDam;
	int _iMaxDam;
	int _iAC;
	// item_special_effect
	int _iFlags;
	// item_misc_id
	int _iMiscId;
	// spell_id
	int _iSpell;
	int _iCharges;
	int _iMaxCharges;
	int _iDurability;
	int _iMaxDur;
	int _iPLDam;
	int _iPLToHit;
	int _iPLAC;
	int _iPLStr;
	int _iPLMag;
	int _iPLDex;
	int _iPLVit;
	int _iPLFR;
	int _iPLLR;
	int _iPLMR;
	int _iPLMana;
	int _iPLHP;
	int _iPLDamMod;
	int _iPLGetHit;
	int _iPLLight;
	char _iSplLvlAdd;
	char _iRequest;
	int _iUid;
	int _iFMinDam;
	int _iFMaxDam;
	int _iLMinDam;
	int _iLMaxDam;
	int _iPLEnAc;
	char _iPrePower;
	char _iSufPower;
	int _iVAdd1;
	int _iVMult1;
	int _iVAdd2;
	int _iVMult2;
	char _iMinStr;
	unsigned char _iMinMag;
	char _iMinDex;
	BOOL _iStatFlag;
	int IDidx;
	int offs016C; // _oldlight or _iInvalid
} ItemStruct;

//////////////////////////////////////////////////
// player
//////////////////////////////////////////////////

typedef struct PlayerStruct {
	int _pmode;
	char walkpath[MAX_PATH_LENGTH];
	BOOLEAN walkedLastTick; //Fluffy: If true, this player moved last tick (this is used for keeping animation frame when resuming walk animation)
	BOOLEAN plractive;
	int destAction;
	int destParam1;
	int destParam2;
	int destParam3;
	int destParam4;
	int plrlevel;
	int _px; //Tile (X) position of player
	int _py; //Tile (Y) position of player
	int _pfutx; //Future (X) position of player. Defined at start of walking animation
	int _pfuty; //Future (Y) position of player. Defined at start of walking animation
	int _ptargx; //Target (X) position for player during pathfinding. This indicates the final tile player should stand on at the end of the entire path. Defined at the start of pathfinding.
	int _ptargy; //Target (Y) position for player during pathfinding. This indicates the final tile player should stand on at the end of the entire path. Defined at the start of pathfinding.
	int _pownerx; //X position which is equal to the player position whenever they do input (or maybe only when starting pathfinding). This is only referenced by enemy AI for an enemy called "SNEAK"
	int _pownery; //Y position which is equal to the player position whenever they do input (or maybe only when starting pathfinding). This is only referenced by enemy AI for an enemy called "SNEAK"
	int _poldx; //Player's old X position. Set by a lot of code when player moves. Only referenced during FixPlrWalkTags() which is called by a lot of movement-related code
	int _poldy; //Player's old Y position. Set by a lot of code when player moves. Only referenced during FixPlrWalkTags() which is called by a lot of movement-related code
	int _pxoff; //X offset render (basically, the player's sub-tile position)
	int _pyoff; //Y offset render (basically, the player's sub-tile position)
	int _pxvel; //Player's X velocity while walking
	int _pyvel; //Player's Y velocity while walking
	int _pdir; //Facing of the player (0..7 starting from DIR_S)
	int _nextdir; //Unused
	int _pgfxnum; //Defines what variant of the sprite the player is using. Lower values define weapon (starting with ANIM_ID_UNARMED) and higher values define armour (starting with ANIM_ID_LIGHT_ARMOR)
	unsigned char *_pAnimData;
	int _pAnimDelay; //By default, all animations advance by one frame for each tick. This value lets you set by how many ticks each frame should get delayed (apparently the last frame of an animation is only ever one tick long)
	int _pAnimCnt; //Increases by one each gameplay tick, counting how close we are to _pAnimDelay
	int _pAnimLen; //Quantity of frames in animation
	int _pAnimFrame; //Current frame we're at in the animation. Always starts at 1?
	int _pAnimWidth;
	int _pAnimWidth2;
	int _peflag; //Unused
	int _plid; //Related to light rendering?
	int _pvid; //Related to line of sight?
	int _pSpell;
	char _pSplType;
	char _pSplFrom;
	int _pTSpell;
	char _pTSplType;
	int _pRSpell;
	// enum spell_type
	char _pRSplType;
	int _pSBkSpell;
	char _pSBkSplType;
	char _pSplLvl[64];
	uint64_t _pMemSpells; //Ownership of spells (1 bit is one spell). This references a database which includes all spells and abilities
	uint64_t _pAblSpells; //Ownership of abilities (1 bit is one spell)
	uint64_t _pScrlSpells; //Ownership of spells/skills via scrolls (gets updated as you use scrolls or do anything with inventory?)
	UCHAR _pSpellFlags;
	int _pSplHotKey[4];
	char _pSplTHotKey[4];
	int _pwtype;
	BOOLEAN _pBlockFlag;
	BOOLEAN _pInvincible;
	char _pLightRad;
	BOOLEAN _pLvlChanging; //Set to true when the player starts loading another level, and set to false as the player finishes loading
	char _pName[PLR_NAME_LEN];
	// plr_class enum value.
	// TODO: this could very well be `enum plr_class _pClass`
	// since there are 3 bytes of alingment after this field.
	// it could just be that the compiler optimized away all accesses to
	// the higher bytes by using byte instructions, since all possible values
	// of plr_class fit into one byte.
	char _pClass;
	int _pStrength;
	int _pBaseStr;
	int _pMagic;
	int _pBaseMag;
	int _pDexterity;
	int _pBaseDex;
	int _pVitality;
	int _pBaseVit;
	int _pStatPts;
	int _pDamageMod;
	int _pBaseToBlk;
	int _pHPBase;
	int _pMaxHPBase;
	int _pHitPoints;
	int _pMaxHP;
	int _pHPPer;
	int _pManaBase;
	int _pMaxManaBase;
	int _pMana;
	int _pMaxMana;
	int _pManaPer;
	char _pLevel;
	char _pMaxLvl;
	int _pExperience;
	int _pMaxExp;
	int _pNextExper;
	char _pArmorClass;
	char _pMagResist;
	char _pFireResist;
	char _pLghtResist;
	int _pGold;
	BOOL _pInfraFlag;
	int _pVar1; //Used for referring to X position of player when finishing moving one tile (also used to define target coordinates for spells and ranged attacks)
	int _pVar2; //Used for referring to Y position of player when finishing moving one tile (also used to define target coordinates for spells and ranged attacks)
	int _pVar3; //Player's direction when ending movement. Also used for defining direction of SPL_FIREWALL spell when casting it.
	int _pVar4; //Used for storing X position of a tile which should have its BFLAG_PLAYERLR flag removed after walking. When starting to walk the game places the player in the dPlayer array -1 in the Y coordinate, and uses BFLAG_PLAYERLR to check if it should be using -1 to the Y coordinate when rendering the player (also used for storing the level of a spell when the player casts it)
	int _pVar5; //Used for storing Y position of a tile which should have its BFLAG_PLAYERLR flag removed after walking. When starting to walk the game places the player in the dPlayer array -1 in the Y coordinate, and uses BFLAG_PLAYERLR to check if it should be using -1 to the Y coordinate when rendering the player (also used for storing the level of a spell when the player casts it)
	int _pVar6; //Same as _pxoff but contains the value in a higher range
	int _pVar7; //Same as _pyoff but contains the value in a higher range
	int _pVar8; //Used for counting how close we are to reaching the next tile when walking (usually counts up to the 8, which is equal to animation length of walk animation). Also used for stalling the appearance of the options screen after dying in singleplayer
	BOOLEAN _pLvlVisited[NUMLEVELS];
	BOOLEAN _pSLvlVisited[NUMLEVELS]; // only 10 used
	int _pGFXLoad;
	unsigned char *_pNAnim[8]; //Stand animation
	int _pNFrames;
	int _pNWidth;
	unsigned char *_pWAnim[8]; //Walk animation
	int _pWFrames;
	int _pWWidth;
	unsigned char *_pAAnim[8]; //Attack animation
	int _pAFrames;
	int _pAWidth;
	int _pAFNum;
	unsigned char *_pLAnim[8]; //Lightning spell cast animation
	unsigned char *_pFAnim[8]; //Fire spell cast animation
	unsigned char *_pTAnim[8]; //Generic spell cast animation
	int _pSFrames;
	int _pSWidth;
	int _pSFNum;
	unsigned char *_pHAnim[8]; //Getting hit animation
	int _pHFrames;
	int _pHWidth;
	unsigned char *_pDAnim[8]; //Death animation
	int _pDFrames;
	int _pDWidth;
	unsigned char *_pBAnim[8]; //Block animation
	int _pBFrames;
	int _pBWidth;

	//Fluffy
	unsigned char *_pNAnim_c[8]; //Casual standing animation
	int _pNFrames_c;
	int _pNWidth_c;
	unsigned char *_pWAnim_c[8]; //Casual walking animation
	int _pWFrames_c;
	int _pWWidth_c;

	ItemStruct InvBody[NUM_INVLOC];
	ItemStruct InvList[NUM_INV_GRID_ELEM];
	int _pNumInv;
	char InvGrid[NUM_INV_GRID_ELEM];
	ItemStruct SpdList[MAXBELTITEMS];
	ItemStruct HoldItem;
	int _pIMinDam;
	int _pIMaxDam;
	int _pIAC;
	int _pIBonusDam;
	int _pIBonusToHit;
	int _pIBonusAC;
	int _pIBonusDamMod;
	uint64_t _pISpells; //Ownership of spells/skills via charged staff
	int _pIFlags;
	int _pIGetHit;
	char _pISplLvlAdd;
	char _pISplCost;
	int _pISplDur;
	int _pIEnAc;
	int _pIFMinDam;
	int _pIFMaxDam;
	int _pILMinDam;
	int _pILMaxDam;
	int _pOilType;
	unsigned char pTownWarps;
	unsigned char pDungMsgs;
	unsigned char pLvlLoad;
	unsigned char pBattleNet;
	BOOLEAN pManaShield;
	char bReserved[3]; //Unused
	short wReserved[8]; //Unused
	DWORD pDiabloKillLevel;
	int pDifficulty;
	int dwReserved[7]; //Unused
	unsigned char *_pNData;
	unsigned char *_pWData;
	unsigned char *_pAData;
	unsigned char *_pLData;
	unsigned char *_pFData;
	unsigned char *_pTData;
	unsigned char *_pHData;
	unsigned char *_pDData;
	unsigned char *_pBData;
	unsigned char *_pNData_c; //Fluffy: Standing casually
	unsigned char *_pWData_c; //Fluffy: Walking casually
	void *pReserved; //Unused
} PlayerStruct;

//////////////////////////////////////////////////
// textdat
//////////////////////////////////////////////////

typedef struct TextDataStruct {
	char *txtstr;
	int scrlltxt;
	int txtspd;
	int sfxnr;
} TextDataStruct;

//////////////////////////////////////////////////
// missiles
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID MIADDPRC
// TPDEF PTR FCN VOID MIPROC

typedef struct MissileData {
	unsigned char mName;
	void (*mAddProc)(int, int, int, int, int, int, char, int, int);
	void (*mProc)(int);
	BOOL mDraw; //If false, the projectile is invisible
	unsigned char mType; //If true, this projectile is magical and we check Magic stat for damage calculation (if not, we do similar calculation as with physical weapons). Some missiles set this value to 2 which seems pointless
	unsigned char mResist;
	unsigned char mFileNum;
	int mlSFX;
	int miSFX;
} MissileData;

typedef struct MisFileData {
	unsigned char mAnimName;
	unsigned char mAnimFAmt;
	char *mName;
	int mFlags;
	unsigned char *mAnimData[16];
	unsigned char mAnimDelay[16];
	unsigned char mAnimLen[16];
	int mAnimWidth[16];
	int mAnimWidth2[16];
} MisFileData;

typedef struct ChainStruct {
	int idx;
	int _mitype;
	int _mirange;
} ChainStruct;

typedef struct MissileStruct {
	int _mitype; //Type of projectile (arrow, firebolt, town portal, etc)
	int _mix; //The current X tile position of the missile
	int _miy; //The current Y tile position of the missile
	int _mixoff; //Current X render offset for the missile (essentially sub-tile position)
	int _miyoff; //Current Y render offset for the missile (essentially sub-tile position)
	int _mixvel; //X velocity of missile. This gets added onto _mitxoff each gameplay tick
	int _miyvel; //Y velocity of missile. This gets added onto _mityoff each gameplay tick
	int _misx; //Initial X position for missile
	int _misy; //Initial Y position for missile
	int _mitxoff; //How far missile has travelled in its lifespan along the X axis. mix/miy/mxoff/myoff get updated every gameplay tick based on this
	int _mityoff; //How far missile has travelled in its lifespan along the Y axis. mix/miy/mxoff/myoff get updated every gameplay tick based on this
	int _mimfnum; //The direction of the missile (0..8 starting from DIR_S)
	int _mispllvl;
	BOOL _miDelFlag; //If true, this missile will get deleted. Missiles are deleted at the start and end of ProcessMissiles()
	BYTE _miAnimType;
	int _miAnimFlags;
	unsigned char *_miAnimData;
	int _miAnimDelay;
	int _miAnimLen;
	int _miAnimWidth;
	int _miAnimWidth2;
	int _miAnimCnt;
	int _miAnimAdd;
	int _miAnimFrame;
	BOOL _miDrawFlag;
	BOOL _miLightFlag;
	BOOL _miPreFlag;
	int _miUniqTrans;
	int _mirange; //Total range the missile will travel. This ticks down by 1 each gameplay tick, and if _mirange reaches 0 then _miDelFlag turns true
	int _misource; //Source of whoever created this missile?
	int _micaster;
	int _midam;
	BOOL _miHitFlag;
	int _midist; //Used for arrows to measure distance travelled (goes up 1 each gameplay tick). Higher value is a penalty for accuracy calculation when hitting enemy
	int _mlid; //Points to an entry in the LightList array
	int _mirnd;
	int _miVar1;
	int _miVar2;
	int _miVar3;
	int _miVar4;
	int _miVar5;
	int _miVar6;
	int _miVar7;
	int _miVar8;
	int tickCount; //Fluffy: Used for updating _mirange and _midist at the same rate as the base game when gSpeedMod is in play
} MissileStruct;

//////////////////////////////////////////////////
// effects/sound
//////////////////////////////////////////////////

typedef struct TSnd {
	char *sound_path;
	SoundSample *DSB;
	int start_tc;
} TSnd;

typedef struct TSFX {
	unsigned char bFlags;
	char *pszName;
	TSnd *pSnd;
} TSFX;

//////////////////////////////////////////////////
// monster
//////////////////////////////////////////////////

typedef struct AnimStruct {
	BYTE *CMem;
	BYTE *Data[8];
	int Frames;
	int Rate;
} AnimStruct;

typedef struct MonsterData {
	int width;
	int mImage;
	char *GraphicType;
	BOOL has_special;
	char *sndfile;
	BOOL snd_special;
	BOOL has_trans;
	char *TransFile;
	int Frames[6];
	int Rate[6];
	char *mName;
	char mMinDLvl;
	char mMaxDLvl;
	char mLevel;
	int mMinHP;
	int mMaxHP;
	char mAi;
	int mFlags;
	unsigned char mInt;
	unsigned short mHit; // BUGFIX: Some monsters overflow this value on high difficultys (fixed)
	unsigned char mAFNum;
	unsigned char mMinDamage;
	unsigned char mMaxDamage;
	unsigned short mHit2; // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	unsigned char mAFNum2;
	unsigned char mMinDamage2;
	unsigned char mMaxDamage2;
	unsigned char mArmorClass;
	char mMonstClass;
	unsigned short mMagicRes;
	unsigned short mMagicRes2;
	unsigned short mTreasure;
	char mSelFlag;
	unsigned short mExp;
} MonsterData;

typedef struct CMonster {
	unsigned char mtype;
	// TODO: Add enum for place flags
	unsigned char mPlaceFlags;
	AnimStruct Anims[6];
	TSnd *Snds[4][2];
	int width;
	int width2;
	unsigned char mMinHP;
	unsigned char mMaxHP;
	BOOL has_special;
	unsigned char mAFNum;
	char mdeadval;
	MonsterData *MData;
	// A TRN file contains a sequence of colour transitions, represented
	// as indexes into a palette. (a 256 byte array of palette indices)
	BYTE *trans_file;
} CMonster;

typedef struct MonsterStruct { // note: missing field _mAFNum
	int _mMTidx;
	int _mmode; /* MON_MODE */
	unsigned char _mgoal;
	int _mgoalvar1;
	int _mgoalvar2;
	int _mgoalvar3;
	int field_18;
	unsigned char _pathcount;
	int _mx; //Tile X coordinate for monster
	int _my; //Tile Y coordinate for monster
	int _mfutx; //Future tile X position for monster. Set when starting movement
	int _mfuty; //Future tile Y position for monster. Set when starting movement
	int _moldx; //Monster's old X position
	int _moldy; //Monster's old Y position
	int _mxoff; //X render offset for monster. Used during movement
	int _myoff; //Y render offset for monster. Used during movement
	int _mxvel; //X velocity to apply to offset per gameplay tick during movement
	int _myvel; //Y velocity to apply to offset per gameplay tick during movement
	int _mdir; //Direction the monster is facing
	int _menemy; //The player the monster is targeting. The value corresponds to an entry in the plr array. Behaviour changes if MFLAG_TARGETS_MONSTER is true, then this corresponds to an entry in the monster array
	unsigned char _menemyx; //X coordinate of enemy (usually correspond's to the enemy's futx value)
	unsigned char _menemyy; //Y coordinate of enemy (usually correspond's to the enemy's futy value)
	short falign_52; // probably _mAFNum (unused)
	unsigned char *_mAnimData;
	int _mAnimDelay; //Duration of each frame
	int _mAnimCnt; //Value for measuring progress through current frame
	int _mAnimLen; //Length of animation
	int _mAnimFrame; //Current animation frame
	BOOL _meflag;
	BOOL _mDelFlag;
	int _mVar1; //This value is used differently for different states. Walking = Stores an X coordinate. Standing = Stores the previous state type the monster was in.
	int _mVar2; //This value is used differently for different states. Walking = Stores an Y coordinate. Standing = Ticks up counting the amount of time spent idle. Attacking = Stores the damage it will deal.
	int _mVar3;
	int _mVar4;
	int _mVar5;
	int _mVar6; //Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	int _mVar7; //Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	int _mVar8; //Value used to measure progress for moving from one tile to another
	int _mmaxhp;
	int _mhitpoints;
	unsigned char _mAi;
	unsigned char _mint; //I think this is some kind of difficulty modifier. Higher values mean enemy might be more deadly or aggressive.
	short falign_9A;
	int _mFlags;
	BYTE _msquelch; //Seems to be a counter to the last time the enemy was visible. While visible this is maxed out and the AI can do whatever it wants. While below max, the enemy will try to follow its target. If zero, the enemy does nothing.
	int falign_A4;
	int _lastx; //I think this corresponds to the last seen X coordinate of the monster's target
	int _lasty; //I think this corresponds to the last seen Y coordinate of the monster's target
	int _mRndSeed;
	int _mAISeed;
	int falign_B8;
	unsigned char _uniqtype;
	unsigned char _uniqtrans;
	char _udeadval;
	char mWhoHit;
	char mLevel;
	unsigned short mExp;
	unsigned short mHit;
	unsigned char mMinDamage;
	unsigned char mMaxDamage;
	unsigned short mHit2;
	unsigned char mMinDamage2;
	unsigned char mMaxDamage2;
	unsigned char mArmorClass;
	char falign_CB;
	unsigned short mMagicRes;
	int mtalkmsg;
	unsigned char leader;
	unsigned char leaderflag;
	unsigned char packsize;
	unsigned char mlid;
	char *mName;
	CMonster *MType;
	MonsterData *MData;
	int tickCount; //Fluffy: For making some variables always update at the same rate as the original game when gMonsterSpeedMod is in play
} MonsterStruct;

typedef struct UniqMonstStruct {
	char mtype;
	char *mName;
	char *mTrnName;
	unsigned char mlevel;
	unsigned short mmaxhp;
	unsigned char mAi;
	unsigned char mint;
	unsigned char mMinDamage;
	unsigned char mMaxDamage;
	unsigned short mMagicRes;
	unsigned short mUnqAttr;
	unsigned char mUnqVar1;
	unsigned char mUnqVar2;
	int mtalkmsg;
} UniqMonstStruct;

//////////////////////////////////////////////////
// objects
//////////////////////////////////////////////////

typedef struct ObjDataStruct {
	char oload;
	char ofindex;
	char ominlvl;
	char omaxlvl;
	char olvltype;
	char otheme;
	char oquest;
	int oAnimFlag;
	int oAnimDelay;
	int oAnimLen;
	int oAnimWidth;
	BOOL oSolidFlag;
	BOOL oMissFlag;
	BOOL oLightFlag;
	char oBreak;
	char oSelFlag;
	BOOL oTrapFlag;
} ObjDataStruct;

typedef struct ObjectStruct {
	int _otype;
	int _ox;
	int _oy;
	int _oLight;
	int _oAnimFlag;
	unsigned char *_oAnimData;
	int _oAnimDelay;
	int _oAnimCnt;
	int _oAnimLen;
	int _oAnimFrame;
	int _oAnimWidth;
	int _oAnimWidth2;
	BOOL _oDelFlag;
	char _oBreak; // check
	BOOL _oSolidFlag;
	BOOL _oMissFlag;
	char _oSelFlag; // check
	BOOL _oPreFlag;
	BOOL _oTrapFlag;
	BOOL _oDoorFlag;
	int _olid;
	int _oRndSeed;
	int _oVar1;
	int _oVar2;
	int _oVar3;
	int _oVar4;
	int _oVar5;
	int _oVar6;
	int _oVar7;
	int _oVar8;
} ObjectStruct;

//////////////////////////////////////////////////
// portal
//////////////////////////////////////////////////

typedef struct PortalStruct {
	BOOL open;
	int x;
	int y;
	int level;
	int ltype;
	BOOL setlvl;
} PortalStruct;

//////////////////////////////////////////////////
// msg
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct TCmd {
	BYTE bCmd;
} TCmd;

typedef struct TCmdLoc {
	BYTE bCmd;
	BYTE x;
	BYTE y;
} TCmdLoc;

typedef struct TCmdLocParam1 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	WORD wParam1;
} TCmdLocParam1;

typedef struct TCmdLocParam2 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	WORD wParam1;
	WORD wParam2;
} TCmdLocParam2;

typedef struct TCmdLocParam3 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	WORD wParam1;
	WORD wParam2;
	WORD wParam3;
} TCmdLocParam3;

typedef struct TCmdParam1 {
	BYTE bCmd;
	WORD wParam1;
} TCmdParam1;

typedef struct TCmdParam2 {
	BYTE bCmd;
	WORD wParam1;
	WORD wParam2;
} TCmdParam2;

typedef struct TCmdParam3 {
	BYTE bCmd;
	WORD wParam1;
	WORD wParam2;
	WORD wParam3;
} TCmdParam3;

typedef struct TCmdGolem {
	BYTE bCmd;
	BYTE _mx;
	BYTE _my;
	BYTE _mdir;
	char _menemy;
	int _mhitpoints;
	BYTE _currlevel;
} TCmdGolem;

typedef struct TCmdQuest {
	BYTE bCmd;
	BYTE q;
	BYTE qstate;
	BYTE qlog;
	BYTE qvar1;
} TCmdQuest;

typedef struct TCmdGItem {
	BYTE bCmd;
	BYTE bMaster;
	BYTE bPnum;
	BYTE bCursitem;
	BYTE bLevel;
	BYTE x;
	BYTE y;
	WORD wIndx;
	WORD wCI;
	int dwSeed;
	BYTE bId;
	BYTE bDur;
	BYTE bMDur;
	BYTE bCh;
	BYTE bMCh;
	WORD wValue;
	DWORD dwBuff;
	int dwTime;
} TCmdGItem;

typedef struct TCmdPItem {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	WORD wIndx;
	WORD wCI;
	int dwSeed;
	BYTE bId;
	BYTE bDur;
	BYTE bMDur;
	BYTE bCh;
	BYTE bMCh;
	WORD wValue;
	DWORD dwBuff;
} TCmdPItem;

typedef struct TCmdChItem {
	BYTE bCmd;
	BYTE bLoc;
	WORD wIndx;
	WORD wCI;
	int dwSeed;
	BOOLEAN bId;
} TCmdChItem;

typedef struct TCmdDelItem {
	BYTE bCmd;
	BYTE bLoc;
} TCmdDelItem;

typedef struct TCmdDamage {
	BYTE bCmd;
	BYTE bPlr;
	DWORD dwDam;
} TCmdDamage;

typedef struct TCmdPlrInfoHdr {
	BYTE bCmd;
	WORD wOffset;
	WORD wBytes;
} TCmdPlrInfoHdr;

typedef struct TCmdString {
	BYTE bCmd;
	char str[MAX_SEND_STR_LEN];
} TCmdString;

typedef struct TFakeCmdPlr {
	BYTE bCmd;
	BYTE bPlr;
} TFakeCmdPlr;

typedef struct TFakeDropPlr {
	BYTE bCmd;
	BYTE bPlr;
	DWORD dwReason;
} TFakeDropPlr;

typedef struct TSyncHeader {
	BYTE bCmd;
	BYTE bLevel;
	WORD wLen;
	BYTE bObjId;
	BYTE bObjCmd;
	BYTE bItemI;
	BYTE bItemX;
	BYTE bItemY;
	WORD wItemIndx;
	WORD wItemCI;
	DWORD dwItemSeed;
	BYTE bItemId;
	BYTE bItemDur;
	BYTE bItemMDur;
	BYTE bItemCh;
	BYTE bItemMCh;
	WORD wItemVal;
	DWORD dwItemBuff;
	BYTE bPInvLoc;
	WORD wPInvIndx;
	WORD wPInvCI;
	DWORD dwPInvSeed;
	BYTE bPInvId;
} TSyncHeader;

typedef struct TSyncMonster {
	BYTE _mndx;
	BYTE _mx;
	BYTE _my;
	BYTE _menemy;
	BYTE _mdelta;
} TSyncMonster;

typedef struct TPktHdr {
	BYTE px;
	BYTE py;
	BYTE targx;
	BYTE targy;
	int php;
	int pmhp;
	BYTE bstr;
	BYTE bmag;
	BYTE bdex;
	WORD wCheck;
	WORD wLen;
} TPktHdr;

typedef struct TPkt {
	TPktHdr hdr;
	BYTE body[493];
} TPkt;

typedef struct DMonsterStr {
	BYTE _mx;
	BYTE _my;
	BYTE _mdir;
	BYTE _menemy;
	BYTE _mactive;
	int _mhitpoints;
} DMonsterStr;

typedef struct DObjectStr {
	BYTE bCmd;
} DObjectStr;

typedef struct DLevel {
	TCmdPItem item[MAXITEMS];
	DObjectStr object[MAXOBJECTS];
	DMonsterStr monster[MAXMONSTERS];
} DLevel;

typedef struct LocalLevel {
	BYTE automapsv[DMAXX][DMAXY];
} LocalLevel;

typedef struct DPortal {
	BYTE x;
	BYTE y;
	BYTE level;
	BYTE ltype;
	BYTE setlvl;
} DPortal;

typedef struct MultiQuests {
	BYTE qstate;
	BYTE qlog;
	BYTE qvar1;
} MultiQuests;

typedef struct DJunk {
	DPortal portal[MAXPORTAL];
	MultiQuests quests[MAXMULTIQUESTS];
} DJunk;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct TMegaPkt {
	struct TMegaPkt *pNext;
	DWORD dwSpaceLeft;
	BYTE data[32000];
} TMegaPkt;
#pragma pack(pop)

typedef struct TBuffer {
	DWORD dwNextWriteOffset;
	BYTE bData[4096];
} TBuffer;

//////////////////////////////////////////////////
// quests
//////////////////////////////////////////////////

typedef struct QuestStruct {
	unsigned char _qlevel;
	unsigned char _qtype;
	unsigned char _qactive;
	unsigned char _qlvltype;
	int _qtx;
	int _qty;
	unsigned char _qslvl;
	unsigned char _qidx;
	unsigned char _qmsg;
	unsigned char _qvar1;
	unsigned char _qvar2;
	int _qlog;
} QuestStruct;

typedef struct QuestData {
	unsigned char _qdlvl;
	char _qdmultlvl;
	unsigned char _qlvlt;
	unsigned char _qdtype;
	unsigned char _qdrnd;
	unsigned char _qslvl;
	int _qflags; /* unsigned char */
	int _qdmsg;
	char *_qlstr;
} QuestData;

//////////////////////////////////////////////////
// gamemenu/gmenu
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID TMenuFcn

typedef struct TMenuItem {
	DWORD dwFlags;
	char *pszStr;
	void (*fnMenu)(BOOL); /* fix, should have one arg */
} TMenuItem;

// TPDEF PTR FCN VOID TMenuUpdateFcn

//////////////////////////////////////////////////
// spells
//////////////////////////////////////////////////

typedef struct SpellData {
	unsigned char sName;
	unsigned char sManaCost;
	unsigned char sType;
	char *sNameText;
	char *sSkillText;
	int sBookLvl;
	int sStaffLvl;
	BOOL sTargeted;
	BOOL sTownSpell;
	int sMinInt;
	unsigned char sSFX;
	unsigned char sMissiles[3];
	unsigned char sManaAdj;
	unsigned char sMinMana;
	int sStaffMin;
	int sStaffMax;
	int sBookCost;
	int sStaffCost;
} SpellData;

//////////////////////////////////////////////////
// towners
//////////////////////////////////////////////////

typedef struct TNQ {
	unsigned char _qsttype;
	unsigned char _qstmsg;
	BOOLEAN _qstmsgact;
} TNQ;

typedef struct TownerStruct {
	int _tmode;
	int _ttype;
	int _tx;    //X tile position of NPC
	int _ty;    //Y tile position of NPC
	int _txoff; //X render offset (not used)
	int _tyoff; //Y render offset (not used)
	int _txvel; //X velocity during movement (not used)
	int _tyvel; //Y velocity during movement (not used)
	int _tdir; //Facing of NPC (not used)
	unsigned char *_tAnimData;
	int _tAnimDelay;
	int _tAnimCnt;
	int _tAnimLen;
	int _tAnimFrame;
	int _tAnimFrameCnt;
	char _tAnimOrder;
	int _tAnimWidth;
	int _tAnimWidth2;
	int _tTenPer;
	int _teflag;
	int _tbtcnt;
	int _tSelFlag;
	BOOL _tMsgSaid;
	TNQ qsts[MAXQUESTS];
	int _tSeed;
	int _tVar1;
	int _tVar2;
	int _tVar3;
	int _tVar4;
	char _tName[PLR_NAME_LEN];
	unsigned char *_tNAnim[8];
	int _tNFrames;
	unsigned char *_tNData;
} TownerStruct;

typedef struct QuestTalkData {
	int _qinfra;
	int _qblkm;
	int _qgarb;
	int _qzhar;
	int _qveil;
	int _qmod;
	int _qbutch;
	int _qbol;
	int _qblind;
	int _qblood;
	int _qanvil;
	int _qwarlrd;
	int _qking;
	int _qpw;
	int _qbone;
	int _qvb;
} QuestTalkData;

//////////////////////////////////////////////////
// gendung
//////////////////////////////////////////////////

typedef struct ScrollStruct {
	int _sxoff; //X offset when rendering camera position. This usually corresponds to a negative version of plr[myplr]._pxoff
	int _syoff; //Y offset when rendering camera position. This usually corresponds to a negative version of plr[myplr]._pyoff
	int _sdx;
	int _sdy;
	int _sdir;
	int pxoffDiff; //Fluffy: How the offset coordinates should differ from player offset. We use this to ensure we can keep the camera in sync with player offset
	int pyoffDiff;
} ScrollStruct;

typedef struct THEME_LOC {
	int x;
	int y;
	int ttval;
	int width;
	int height;
} THEME_LOC;

typedef struct MICROS {
	WORD mt[16];
} MICROS;

//////////////////////////////////////////////////
// drlg
//////////////////////////////////////////////////

typedef struct ShadowStruct {
	unsigned char strig;
	unsigned char s1;
	unsigned char s2;
	unsigned char s3;
	unsigned char nv1;
	unsigned char nv2;
	unsigned char nv3;
} ShadowStruct;

typedef struct HALLNODE {
	int nHallx1;
	int nHally1;
	int nHallx2;
	int nHally2;
	int nHalldir;
	struct HALLNODE *pNext;
} HALLNODE;

typedef struct ROOMNODE {
	int nRoomx1;
	int nRoomy1;
	int nRoomx2;
	int nRoomy2;
	int nRoomDest;
} ROOMNODE;

//////////////////////////////////////////////////
// themes
//////////////////////////////////////////////////

typedef struct ThemeStruct {
	char ttype; /* aligned 4 */
	int ttval;
} ThemeStruct;

//////////////////////////////////////////////////
// inv
//////////////////////////////////////////////////

typedef struct InvXY {
	int X;
	int Y;
} InvXY;

//////////////////////////////////////////////////
// lighting
//////////////////////////////////////////////////

typedef struct LightListStruct {
	int _lx;
	int _ly;
	int _lradius;
	int _lid;
	int _ldel;
	int _lunflag;
	int field_18;
	int _lunx;
	int _luny;
	int _lunr;
	int _xoff;
	int _yoff;
	int _lflags;
} LightListStruct;

//////////////////////////////////////////////////
// dead
//////////////////////////////////////////////////

typedef struct DeadStruct {
	unsigned char *_deadData[8];
	int _deadFrame;
	int _deadWidth;
	int _deadWidth2;
	char _deadtrans;
} DeadStruct;

//////////////////////////////////////////////////
// diabloui
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID PLAYSND

typedef struct _gamedata {
	int dwSeed;
	BYTE bDiff;
	BYTE bRate;
	BOOL fastWalkInTown;
	BOOL allowAttacksInTown;
	int gSpeedMod;
	int gMonsterSpeedMod;
} _gamedata;

typedef struct _uidefaultstats {
	WORD strength;
	WORD magic;
	WORD dexterity;
	WORD vitality;
} _uidefaultstats;

typedef struct _uiheroinfo {
	struct _uiheroinfo *next;
	char name[16];
	WORD level;
	BYTE heroclass;
	BYTE herorank;
	WORD strength;
	WORD magic;
	WORD dexterity;
	WORD vitality;
	int gold;
	int hassaved;
	BOOL spawned;
} _uiheroinfo;

// TPDEF PTR FCN UCHAR ENUMHEROPROC
// TPDEF PTR FCN UCHAR ENUMHEROS
// TPDEF PTR FCN UCHAR CREATEHERO
// TPDEF PTR FCN UCHAR DELETEHERO
// TPDEF PTR FCN UCHAR GETDEFHERO

// TPDEF PTR FCN INT PROGRESSFCN

//////////////////////////////////////////////////
// storm
//////////////////////////////////////////////////

// TPDEF PTR FCN UCHAR SMSGIDLEPROC
// TPDEF PTR FCN VOID SMSGHANDLER

typedef struct _SNETCAPS {
	DWORD size;
	DWORD flags;
	DWORD maxmessagesize;
	DWORD maxqueuesize;
	DWORD maxplayers;
	DWORD bytessec;
	DWORD latencyms;
	DWORD defaultturnssec;
	DWORD defaultturnsintransit;
} _SNETCAPS;

typedef struct _SNETEVENT {
	DWORD eventid;
	DWORD playerid;
	void *data;
	DWORD databytes;
} _SNETEVENT;

// TPDEF PTR FCN UCHAR SNETABORTPROC
// TPDEF PTR FCN UCHAR SNETCATEGORYPROC
// TPDEF PTR FCN UCHAR SNETCHECKAUTHPROC
// TPDEF PTR FCN UCHAR SNETCREATEPROC
// TPDEF PTR FCN UCHAR SNETDRAWDESCPROC
// TPDEF PTR FCN UCHAR SNETENUMDEVICESPROC
// TPDEF PTR FCN UCHAR SNETENUMGAMESPROC
// TPDEF PTR FCN UCHAR SNETENUMPROVIDERSPROC
// TPDEF PTR FCN VOID SNETEVENTPROC
// TPDEF PTR FCN UCHAR SNETGETARTPROC
// TPDEF PTR FCN UCHAR SNETGETDATAPROC
// TPDEF PTR FCN INT SNETMESSAGEBOXPROC
// TPDEF PTR FCN UCHAR SNETPLAYSOUNDPROC
// TPDEF PTR FCN UCHAR SNETSELECTEDPROC
// TPDEF PTR FCN UCHAR SNETSTATUSPROC

typedef struct _SNETPLAYERDATA {
	int size;
	char *playername;
	char *playerdescription;
	int reserved;
} _SNETPLAYERDATA;

typedef struct _SNETPROGRAMDATA {
	int size;
	char *programname;
	char *programdescription;
	int programid;
	int versionid;
	int reserved1;
	int maxplayers;
	_gamedata *initdata;
	int initdatabytes;
	void *reserved2;
	int optcategorybits;
	char *cdkey;
	char *registereduser;
	int spawned;
	int lcid;
} _SNETPROGRAMDATA;

typedef struct _SNETVERSIONDATA {
	int size;
	char *versionstring;
	char *executablefile;
	char *originalarchivefile;
	char *patcharchivefile;
} _SNETVERSIONDATA;

typedef struct _SNETUIDATA {
	int size;
	int uiflags;
	HWND parentwindow;
	void (*artcallback)();
	void (*authcallback)();
	void (*createcallback)();
	void (*drawdesccallback)();
	void (*selectedcallback)();
	void (*messageboxcallback)();
	void (*soundcallback)();
	void (*statuscallback)();
	void (*getdatacallback)();
	void (*categorycallback)();
	void (*categorylistcallback)();
	void (*newaccountcallback)();
	void (*profilecallback)();
	const char **profilefields;
	void (*profilebitmapcallback)();
	int (*selectnamecallback)(
	    const struct _SNETPROGRAMDATA *,
	    const struct _SNETPLAYERDATA *,
	    const struct _SNETUIDATA *,
	    const struct _SNETVERSIONDATA *,
	    DWORD provider, /* e.g. 'BNET', 'IPXN', 'MODM', 'SCBL' */
	    char *, DWORD,  /* character name will be copied here */
	    char *, DWORD,  /* character "description" will be copied here (used to advertise games) */
	    BOOL *          /* new character? - unsure about this */
	);
	void (*changenamecallback)();
} _SNETUIDATA;

// TPDEF PTR FCN UCHAR SNETSPIBIND
// TPDEF PTR FCN UCHAR SNETSPIQUERY

//////////////////////////////////////////////////
// pack
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct PkItemStruct {
	DWORD iSeed;
	WORD iCreateInfo;
	WORD idx;
	BYTE bId;
	BYTE bDur;
	BYTE bMDur;
	BYTE bCh;
	BYTE bMCh;
	WORD wValue;
	DWORD dwBuff;
} PkItemStruct;

typedef struct PkPlayerStruct {
	FILETIME archiveTime;
	char destAction;
	char destParam1;
	char destParam2;
	BYTE plrlevel;
	BYTE px;
	BYTE py;
	BYTE targx;
	BYTE targy;
	char pName[PLR_NAME_LEN];
	char pClass;
	BYTE pBaseStr;
	BYTE pBaseMag;
	BYTE pBaseDex;
	BYTE pBaseVit;
	char pLevel;
	BYTE pStatPts;
	int pExperience;
	int pGold;
	int pHPBase;
	int pMaxHPBase;
	int pManaBase;
	int pMaxManaBase;
	char pSplLvl[MAX_SPELLS];
	uint64_t pMemSpells;
	PkItemStruct InvBody[NUM_INVLOC];
	PkItemStruct InvList[NUM_INV_GRID_ELEM];
	char InvGrid[NUM_INV_GRID_ELEM];
	BYTE _pNumInv;
	PkItemStruct SpdList[MAXBELTITEMS];
	char pTownWarps;
	char pDungMsgs;
	char pLvlLoad;
	char pBattleNet;
	BOOLEAN pManaShield;
	char bReserved[3];
	short wReserved[8];
	int pDiabloKillLevel;
	int dwReserved[7];
} PkPlayerStruct;
#pragma pack(pop)

//////////////////////////////////////////////////
// path
//////////////////////////////////////////////////

typedef struct PATHNODE {
	char f;
	char h;
	char g;
	int x;
	int y;
	struct PATHNODE *Parent;
	struct PATHNODE *Child[8];
	struct PATHNODE *NextNode;
} PATHNODE;

// TPDEF PTR FCN UCHAR CHECKFUNC1

// TPDEF PTR FCN UCHAR CHECKFUNC

//////////////////////////////////////////////////
// sha
//////////////////////////////////////////////////

typedef struct SHA1Context {
	DWORD state[5];
	DWORD count[2];
	char buffer[64];
} SHA1Context;

//////////////////////////////////////////////////
// tmsg
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct TMsg TMsg;

typedef struct TMsgHdr {
	TMsg *pNext;
	int dwTime;
	BYTE bLen;
} TMsgHdr;

typedef struct TMsg {
	TMsgHdr hdr;
	// this is actually alignment padding, but the message body is appended to the struct
	// so it's convenient to use byte-alignment and name it "body"
	unsigned char body[3];
} TMsg;
#pragma pack(pop)

//////////////////////////////////////////////////
// mpqapi
//////////////////////////////////////////////////

typedef struct _FILEHEADER {
	int signature;
	int headersize;
	int filesize;
	WORD version;
	short sectorsizeid;
	int hashoffset;
	int blockoffset;
	int hashcount;
	int blockcount;
	char pad[72];
} _FILEHEADER;

typedef struct _HASHENTRY {
	uint32_t hashcheck[2];
	uint32_t lcid;
	uint32_t block;
} _HASHENTRY;

typedef struct _BLOCKENTRY {
	uint32_t offset;
	uint32_t sizealloc;
	uint32_t sizefile;
	uint32_t flags;
} _BLOCKENTRY;

// TPDEF PTR FCN UCHAR TGetNameFcn

// TPDEF PTR FCN VOID TCrypt

//////////////////////////////////////////////////
// trigs
//////////////////////////////////////////////////

typedef struct TriggerStruct {
	int _tx;
	int _ty;
	int _tmsg;
	int _tlvl;
} TriggerStruct;

//////////////////////////////////////////////////
// stores
//////////////////////////////////////////////////

typedef struct STextStruct {
	int _sx;
	int _syoff;
	char _sstr[128];
	BOOL _sjust;
	char _sclr;
	int _sline;
	BOOL _ssel;
	int _sval;
} STextStruct;

//////////////////////////////////////////////////
// wave
//////////////////////////////////////////////////

typedef struct MEMFILE {
	DWORD end;
	LONG offset;
	DWORD buf_len;
	DWORD dist;
	DWORD bytes_to_read;
	BYTE *buf;
	HANDLE file;
} MEMFILE;

//////////////////////////////////////////////////
// plrmsg
//////////////////////////////////////////////////

typedef struct _plrmsg {
	DWORD time;
	unsigned char player;
	char str[144];
} _plrmsg;

//////////////////////////////////////////////////
// capture
//////////////////////////////////////////////////

typedef struct _PcxHeader {
	BYTE Manufacturer;
	BYTE Version;
	BYTE Encoding;
	BYTE BitsPerPixel;
	WORD Xmin;
	WORD Ymin;
	WORD Xmax;
	WORD Ymax;
	WORD HDpi;
	WORD VDpi;
	BYTE Colormap[48];
	BYTE Reserved;
	BYTE NPlanes;
	WORD BytesPerLine;
	WORD PaletteInfo;
	WORD HscreenSize;
	WORD VscreenSize;
	BYTE Filler[54];
} PCXHEADER;

//////////////////////////////////////////////////
// encrypt
//////////////////////////////////////////////////

typedef struct TDataInfo {
	BYTE *srcData;
	DWORD srcOffset;
	BYTE *destData;
	DWORD destOffset;
	DWORD size;
} TDataInfo;

DEVILUTION_END_NAMESPACE
