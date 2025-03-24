#pragma once
#pragma pack(push, 1)
//#include <iostream>

enum {
	TIMERA = 104,
	TIMERB = 105,
	MAX_NUM_MISSION_SCRIPTS = 150,
};

/* 476 */
enum ePED_STATE
{
	PED_IDLE = 0x1,
	PED_SEEK_ENTITY = 0x7,
	PED_FLEE_ENTITY = 0x9,
	PED_ATTACK = 0x10,
	PED_TODO___17 = 0x11,
	PED_JUMP = 0x29,
	PED_ARREST_PLAYER = 0x36,
	PED_DRIVING = 0x37,
	PED_DIE = 0x39,
	PED_DEAD = 0x3A,
	PED_CARJACK = 0x3B,
	PED_DRAG_FROM_CAR = 0x3C,
	PED_ENTER_CAR = 0x3D,
	PED_ARRESTED = 0x41,
};

/* 479 */
enum AssocGroupId
{
	ASSOCGRP_WOMAN = 0x2C,
	ASSOCGRP_SEXYWOMAN = 0x2F,
};

/* 414 */
struct __declspec(align(4)) CRunningScript
{
	CRunningScript* m_pNext;
	CRunningScript* m_pPrev;
	int m_nId;
	int pad4;
	int m_nIp;
	int m_anStack[16];
	int m_anLocalVariables[106];
	int m_nLocalsPointer;
	int m_nWakeTime;
	__int16 m_nStackPointer;
	__int16 m_nAndOrState;
	char m_bIsActive;
	char m_bCondResult;
	char m_bIsMissionScript;
	char m_bSkipWakeTime;
	char m_bNotFlag;
	char m_bDeatharrestEnabled;
	char m_bDeatharrestExecuted;
	char m_abScriptName[8];
	char m_bMissionFlag;
};

/* 484 */
struct vag_header
{
	int header;
	int version;
	int unk;
	int dataSize;
	int freq;
	char pad[12];
	char name[16];
};

/* 303 */
struct CRGBA
{
	unsigned __int8 red;
	unsigned __int8 green;
	unsigned __int8 blue;
	unsigned __int8 alpha;
};

/* 182 */
struct __declspec(align(4)) CVector
{
	float x;
	float y;
	float z;
};

/* 514 */
struct CVectorVU_align16
{
	float x;
	float y;
	float z;
	int pad;
};

/* 471 */
struct CMatrix
{
	CVectorVU_align16 right;
	CVectorVU_align16 at_forward;
	CVectorVU_align16 up;
	CVectorVU_align16 pos;
	void* m_pRwMat;
};
void inline SetCVector4VU(CVectorVU_align16* p1, CVector* p2) { memcpy(p1, p2, 3 * 4); }

/* 450 */
struct CPedStats
{
	char a[25];
	char m_sexiness;
	char b[77777];
};

/* 489 */
struct CWeapon
{
	int m_eWeaponType;
	char field_4[20];
	int field_18;
};

/* 513 */
struct CStoredCollPoly
{
	CVectorVU_align16 verts[3];
	char valid;
	char pad[3];
};

/* 501 */
struct CPlaceable
{
	CMatrix m_pMat;
};

/* 478 */
struct CEntity
{
	CPlaceable CPlaceable;
	char CE_flags_A;
	char CE_flags_B;
	char CE_flags_C;
	char CE_flags_D;
	char _CE_flags_E;
	char CE_flags_F;
	char CE_flags_G;
	char CE_flags_H;
	char CE_flags_I;
	char CE_flags_J;
	char CE_flags_K;
	char CE_flags_L;
	int m_rwObject;
	__int16 m_scanCode;
	__int16 m_modelIndex;
	char _field_58[2];
	char flags_field_5A;
	char m_lastWepDam;
	void* vftable;
};


/* 502 */
struct CPhysical
{
	CEntity CEntity;
	int m_audioEntityId;
	int possible_m_LastCollisionTime__m_phys_unused1;
	int m_nLastTimeCollided;
	char field_6C[4];
	CVectorVU_align16 m_vecTurnSpeed;
	CVectorVU_align16 m_vecMoveFriction;
	CVectorVU_align16 m_vecTurnFriction;
	char field_A0[32];
	CVectorVU_align16 m_vecCOM__m_vecCentreOfMass;
	float m_fMass;
	float m_fTurnMass;
	int field_D8;
	float m_fAirResistance;
	float m_fElasticity;
	int IsPedPointerValidstuff__field_E4;
	char _flags_field_E8;
	char field_E9;
	char field_EA;
	char field_EB;
	char field_EC;
	char field_ED;
	char field_EE;
	char field_EF;
	int m_aCollisionRecords[6];
	int posible_m_fDistanceTravelled;
	float m_fDamageImpulse;
	int field_110;
	int field_114;
	int field_118;
	int field_11C;
	CVectorVU_align16 m_vecDamageNormal;
	int flags12;
	char field_134[2];
	char m_nCollisionRecords;
	char field_137;
	char m_nZoneLevel;
	char field_139[7];
	CVectorVU_align16 m_vecMoveSpeed;
	int m_pDamageEntity;
	char field_154;
	char field_155;
	char field_156[2];
	float m_fBuoyancy;
	int unk_field_15C;
};

struct CPed;
/* 475 */
struct CVehicle
{
	CPhysical CPhysical;
	void* pHandling;
	void* pFlyingHandling;
	int field_168;
	int field_16C;
	void* m_pLeftWakesInfo;
	void* m_pRightWakesInfo;
	char field_178[8];
	char AutoPilot[32];
	int autopilot_m_nTimeTempAction_field_1A0;
	char A_field_1A4;
	char A_field_1A5;
	char A_field_1A6[5];
	char A_field_1AB;
	char A_field_1AC[5];
	char A_field_1B1;
	char A_field_1B2[5];
	char A_field_1B7;
	char A_field_1B8[6];
	char AutoPilot_m_nSwitchDistance;
	char autopilot_field_1BF;
	CVector AutoPilot_m_vecDestinationCoors;
	char autopilot_field_1CC[44];
	int AutoPilot_m_nAntiReverseTimer;
	char autopilot_field_1FC[12];
	float autopilot_m_fMaxTrafficSpeed;
	char A_field_20C[8];
	char autopilot_m_nDrivingStyle;
	char AutoPilot_m_nCarMission;
	char autopilot_m_nTempAction;
	char autopilot_m_nCruiseSpeed;
	char autopilot_flags_field_218;
	char field_219[11];
	CRGBA m_currentColour1;
	CRGBA m_currentColour2;
	char field_22C[2];
	__int16 m_nAlarmState;
	__int16 m_nRouteSeed;
	char m_nNumGettingIn;
	char m_nGettingInFlags;
	char possible_m_nGettingOutFlags_field_234;
	char m_nNumMaxPassengers;
	char field_236[26];
	int field_250;
	int m_pCarFire;
	float m_fSteerAngle;
	float m_fGasPedal;
	float m_fBrakePedal___pstartflagsmask;
	char VehicleCreatedBy;
	char flags_field_265;
	char flags_field_266;
	char flags_field_267;
	char flags_field_268;
	char field_269;
	char field_26A;
	char field_26B;
	char field_26C[5];
	char m_nAmmoInClip;
	char field_272;
	char m_bGarageTurnedLightsOff;
	char field_274[4];
	float m_fMaxHealth;
	float m_fHealth;
	float m_fEngineEnergy;
	char field_284[8];
	int m_nGunFiringTime;
	char field_290[4];
	int m_pBlowUpEntity_pDelayedExplosionInflictor;
	char field_298[8];
	int m_nDoorLock;
	char field_2A4[4];
	int m_pLastDamageEntity;
	char field_2AC[4];
	__int16 m_nBombTimer_DelayedExplosion;
	char field_2B2[2];
	int m_cHorn;
	char field_2B8;
	char transmission_nNumberOfGears;
	char field_2BA[4];
	char field_2BE;
	char field_2BF[4];
	char m_comedyControlState;
	char field_2C4[8];
	int field_2CC;
	char possible_m_aCollPolys_field_2D0;
	char field_2D1[127];
	float fSteer;
	int m_vehicleType;
	int m_baseVehicleType;
	int m_bSuperBrake;
	char field_360[36];
	CPed* pDriver;
	void* pPassengers[8];
	char field_3A8[8];
};
/* 473 */
struct CPed
{
	CPhysical CPhysical;
	CStoredCollPoly m_collPoly;
	char field_194[12];
	int m_fCollisionSpeed;
	char field_1A4[36];
	char flags11[8];
	int field_1D0;
	int flags1;
	char aa___flags2[168];
	int m_prevObjective;
	char field_284[32];
	CPed* m_leader;
	int m_pedFormation;
	char field_2AC[8];
	int m_pEventEntity;
	char field_2B8[80];
	AssocGroupId m_animGroup;
	char field_30C[36];
	int m_pedIK;
	char m_pedIK_field_334[40];
	int m_pedIK_m_flags;
	char field_360[36];
	int field_384;
	char field_388[128];
	int m_pNextPathNode;
	char field_40C[52];
	CVectorVU_align16 m_vecSeekPos;
	int m_pSeekTarget;
	char field_454[20];
	int m_threatEntity;
	int field_46C;
	int m_followPathTargetEnt;
	char field_474[12];
	CVehicle* m_pMyVehicle;
	CVehicle* m_carInObjective;
	char field_488[92];
	float m_fHealth;
	float m_fArmour;
	char field_4EC[4];
	int field_4F0;
	int todo_setpedtimersswitch_field_4F4;
	char field_4F8[28];
	int m_pCurrentPhysSurface;
	char field_518[56];
	int m_nPedType;
	CPedStats* m_pedStats;
	int m_fleeTimer;
	CWeapon m_weapons[10];
	char field_674[64];
	int m_pLookTarget;
	int m_fLookDirection;
	char field_6BC[4];
	int m_leaveCarTimer;
	char field_6C4[4];
	int m_lookTimer;
	char field_6CC[4];
	int m_attackTimer;
	int m_shootTimer;
	int m_carJackTimer;
	int m_objectiveTimer;
	char field_6E0[8];
	int m_bloodyFootprintCountOrDeathTime;
	char field_6EC[12];
	void* m_nearPeds[10];
	char field_720[76];
	int m_threatFlags;
	int m_threatCheckTimer;
	int m_threatCheckInterval;
	__int16 field_778;
	__int16 m_numNearPeds;
	char field_77C[2];
	__int16 m_nPedMoney;
	char field_780[9];
	char m_currentWeapon;
	char m_nPathDir;
	char field_78B;
	__int16 field_78C;
	char field_78E;
	char field_78F[21];
	int time_field_7A4;
	char field_7A8[116];
	int m_nPedStateTimer;
	int m_pPointGunAt;
	char field_824[12];
	float m_fBreath;
	char field_834[36];
	int attachedTo;
	char field_85C[68];
	int m_pedInObjective;
	int m_objective;
	char field_8A8[8];
	float field_8B0;
	char field_8B4[16];
	ePED_STATE m_nPedState;
	int m_nLastPedState;
	int m_nWaitState;
	int m_nWaitTimer;
	int m_nMoveState;
	int field_8D8;
	int m_nPrevMoveState;
	int m_fRotationCur;
	int m_fRotationDest;
	float m_headingRate;
	char field_8EC[5];
	char colour_1201_field_8F1;
	__int16 m_vehDoor;
	int CharCreatedBy;
	char field_8F8[4];
	char field_8FC[4];
};

/* 486 */
struct CPlayerInfo
{
	CPed* m_pPed;
	CVehicle* m_pRemoteVehicle;
	char field_8[180];
	int m_nMoney;
	char pppaaaddd[28];
	int m_pHooker;
	char field_E0[24];
	int field_F8;
	char field_FC[4];
	int m_nTimeTankShotGun;
	char field_104[140];
}; // size done


/* 244 */
struct CWanted
{
	int m_nChaos;
	int m_nMinChaos;
	int m_nLastUpdateTime;
	int m_nLastWantedLevelChange;
	int m_nLastTimeSuspended;
	float m_fCrimeSensitivity;
	char m_CurrentCops;
	char m_MaxCops;
	char m_MaximumLawEnforcerVehicles;
	char m_CopsBeatingSuspect;
	__int16 m_RoadblockDensity;
	char flags_field_1E;
	char field_1F;
	int m_nWantedLevel;
	int m_nMinWantedLevel;
	char field_28[776];
	int m_pCops[10];
};

/* 474 */
struct __declspec(align(1)) CPlayerPed
{
	CPed CPed;
	CWanted m_pWanted;
	char field_C58[12];
	float m_fMoveSpeed;
	float m_fMaxStamina;
	char field_C6C[68];
	int field_CB0;
	int field_CB4;
	char field_CB8[16];
	char _flags_field_CC8;
	char flags_field_CC9;
	char flags_field_CCA;
	char flags_field_CCB;
	char flags_field_CCC;
	char flags_field_CCD;
	char field_CCE[2];
	float m_fCurrentStamina;
	char field_CD4[12];
	int m_nLastBusFareCollected;
	char field_CE4[60];
	char ____field_D20[7777];
};

/* 415 */
struct CCarGenerator
{
	int m_nModelIndex;
	CVector m_vecPos;
	float m_fAngle;
	int m_nColor1;
	int m_nColor2;
	char m_bForceSpawn;
	char m_nAlarm;
	char m_nDoorlock;
	char pad_1F;
	__int16 m_nMinDelay;
	__int16 m_nMaxDelay;
	int m_nTimer;
	int m_nVehicleHandle;
	__int16 m_nUsesRemaining;
	char m_bIsBlocking;
	char pad_2F;
};

/* 269 */
struct __declspec(align(4)) CPool
{
	int m_Objects;
	unsigned __int8* m_ByteMap;
	int m_nSize;
	int m_nFirstFree;
};

/* 507 */
struct CControllerState
{
	__int16 LeftStickX;
	__int16 LeftStickY;
	__int16 RightStickX;
	__int16 RightStickY;
	__int16 LeftShoulder1;
	__int16 LeftShoulder2;
	__int16 RightShoulder1;
	__int16 RightShoulder2;
	__int16 DPadUp;
	__int16 DPadDown;
	__int16 DPadLeft;
	__int16 DPadRight;
	__int16 unk_nipple_up_field_18;
	__int16 unk_nipple_down_field_1A;
	__int16 unk_nipple_left_field_1C;
	__int16 unk_nipple_right_field_1E;
	__int16 Start;
	__int16 Select;
	__int16 Square;
	__int16 Triangle;
	__int16 Cross;
	__int16 Circle;
	__int16 LeftShock;
	__int16 RightShock;
};

/* 518 */
struct CPad
{
	__int16 field_0;
	CControllerState NewState;
	CControllerState OldState;
	char field_62[48];
	__int16 Mode;
	__int16 field_94;
	__int16 DisablePlayerControls;
	char field_98[13];
	char KeyBoardCheatString[12];
	char field_B1[31];
	float field_D0;
};

/* 529 */
struct CSphere
{
	CVector center;
	float radius;
};

/* 528 */
struct CBox
{
	CVectorVU_align16 min;
	CVectorVU_align16 max;
};

/* 527 */
struct CColModel
{
	CSphere boundingSphere;
	CBox boundingBox;
	__int16 numBoxes;
	__int16 numTriangles;
	char numSpheres;
	char numLines;
	char numTriBBoxes;
	char level;
	int spheres;
	int lines;
	int boxes;
	int triBBoxes;
	int vertices;
	int triangles;
	int trianglePlanes;
};

/* 526 */
struct __declspec(align(1)) CBaseModelInfo
{
	int field_0;
	int field_4;
	int val_field_8;
	int pointer_possiblename_field_C;
	int bytes_field_10;
	CColModel* m_colModel;
	char field_18[777];
};


/* 524 */
struct __declspec(align(1)) tHudProp
{
	int isVisible;
	int xoffset_field_4;
	int yoffset_field_8;
	int some_x_size_field_C;
	int some_y_size_field_10;
	int field_14;
	int field_18;
	char field_1C[777];
};

/* 525 */
struct CEmpireHud
{
	int pointer_some_alloc_next_field_0;
	int pointer_field_4;
	int vftable;
	tHudProp** pHudNodeArrayList; // save 3 cnt10
	tHudProp** pHudNodeArrayListEnd; // ptr to afterarray->Lst[nodes10], ->LstEnd[nextxmldata]
	int field_14;
	int pointer_some_fe_xml_field_18;
	int some_timer;
	float some_X_size_val_field_20;
	float some_Y_size_val_field_24;
	int some_decr_timer_byedit_field_28;
};






































#pragma pack(pop)