#pragma once
#pragma pack(push, 1)
//#include <iostream>
#include "Windows.h"

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
struct VAGheader
{
	int format;
	int ver;
	int ssa;
	int size;
	int fs;
	__int16 volL;
	__int16 volR;
	__int16 pitch;
	__int16 ADSR1;
	__int16 ADSR2;
	__int16 reserved;
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

/* 680 */
struct INSTANCE
{
	int ptr_field_0;
	int ptr_field_4;
	void* vftable;
};

/* 181 */
struct CVector2D
{
	float x;
	float y;
};

/* 288 */
struct __declspec(align(4)) CRect
{
	float left;
	float bottom;
	float right;
	float top;
};

/* 182 */
struct __declspec(align(4)) CVector
{
	float x;
	float y;
	float z;
};
//class CVector
//{
//public:
//	float x;
//	float y;
//	float z;
//	CVector(float x, float y, float z) :x(x), y(y), z(z) {}
//};

/* 514 */
struct CVuVector
{
	float x;
	float y;
	float z;
	float w;
};

/* 200 */
struct __declspec(align(4)) RwV3d
{
	float x;
	float y;
	float z;
};


/* 470 */
struct RwMatrix
{
	RwV3d right;
	int field_C;
	RwV3d up;
	int field_1C;
	RwV3d forvard_at;
	int field_2C;
	RwV3d pos;
	int field_3C;
};

/* 471 */
struct CMatrix
{
	CVuVector right;
	CVuVector at_forward;
	CVuVector up;
	CVuVector pos;
	RwMatrix* m_pRwMat;
};
void inline SetCVector4VU(CVuVector* p1, CVector* p2) { if (p1 && p2) { memcpy(p1, p2, 3 * 4); } }
void inline SetRWV3D(RwV3d* p1, CVector* p2) { memcpy(p1, p2, 3 * 4); }

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
	int field_0;
	int m_eWeaponType;
	int m_eWeaponState;
	int m_nAmmoInClip;
	int m_nAmmoTotal;
	int possible_m_nTimer_field_14;
	int field_18;
};

/* 513 */
struct CStoredCollPoly
{
	CVuVector verts[3];
	char valid;
	char pad[15];
};

/* 581 */
struct LimbOrientation
{
	float yaw;
	float pitch;
};

/* 580 */
struct CPed;
struct CPedIK
{
	CPed* m_ped;
	LimbOrientation m_headOrient;
	LimbOrientation m_torsoOrient;
	LimbOrientation m_upperArmOrient;
	LimbOrientation m_lowerArmOrient;
	LimbOrientation unkLO;
	int m_flags;
};


/* 501 */
struct CPlaceable
{
	CMatrix m_pMat;
};

struct RpAtomic;
struct RwObject;
struct RpClump;
/* 954 */
union uRslObjects
{
	RwObject* m_rwObject;
	RpClump* m_rpClump;
	RpAtomic* m_rpAtomic;
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
	//RpAtomic* m_rwObject;
	uRslObjects m_urwObject;
	__int16 m_scanCode;
	__int16 m_modelIndex;
	__int16 m_modelIndex2;
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
	CVuVector m_vecTurnSpeed;
	CVuVector m_vecMoveFriction;
	CVuVector m_vecTurnFriction;
	char field_A0[32];
	CVuVector m_vecCOM__m_vecCentreOfMass;
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
	CVuVector m_vecDamageNormal;
	int flags12;
	char field_134[2];
	char m_nCollisionRecords;
	char field_137;
	char m_nZoneLevel;
	char field_139[7];
	CVuVector m_vecMoveSpeed;
	int m_pDamageEntity;
	char field_154;
	char field_155;
	char field_156[2];
	float m_fBuoyancy;
	int unk_field_15C;
};

struct CPed;
/* 638 */
struct CAutoPilot
{
	void* m_aPathFindNodesInfo[8];
	__int16 m_nPrevRouteNode;
	__int16 field_22;
	char A_field_24;
	char A_field_25;
	__int16 m_nCurrentRouteNode;
	char A_field_28[3];
	char A_field_2B;
	__int16 m_nNextRouteNode;
	char A_field_2E[3];
	char A_field_31;
	char A_field_32[5];
	char A_field_37;
	CEntity* m_pTargetCar;
	__int16 NumPathNodes;
	char m_nSwitchDistance;
	char autopilot_field_3F;
	CVuVector TargetCoors___m_vecDestinationCoors;
	int possible_m_aPathFindNodesInfo[8];
	char m_nTimeEnteredCurve___TimeToGetToNextLink[4];
	int m_nTimeToSpendOnCurrentCurve___TimeToLeaveLink;
	int m_nAntiReverseTimer___LastTimeNotStuck;
	int m_nTimeToStartMission___LastTimeMoving;
	int TempActionFinish;
	float A_field_84;
	float m_fMaxTrafficSpeed;
	int field_8C;
	float field_90;
	char m_nDrivingStyle;
	char m_nCarMission;
	char m_nTempAction;
	char m_nCruiseSpeed;
	char m_nCruiseSpeedMultiplierType;
	char autopilot_flags_field_99;
	char autopilot_flags_field_9A;
	char autopilot_flags_field_9B;
};
/* 475 */
struct CVehicle
{
	CPhysical CPhysical;
	void* pHandling;
	void* pFlyingHandling;
	int m_pHandlingBoat;
	int m_pHandling6atv;
	void* m_pLeftWakesInfo;
	void* m_pRightWakesInfo;
	char field_178[8];
	CAutoPilot AutoPilot;
	char field_21C[4];
	int field_220;
	CRGBA m_currentColour1;
	CRGBA m_currentColour2;
	char m_aExtras[2];
	__int16 m_nAlarmState;
	__int16 m_nRouteSeed;
	char m_nNumGettingIn;
	char m_nGettingInFlags;
	char possible_m_nGettingOutFlags_field_234;
	char m_nNumMaxPassengers;
	char field_236[26];
	int m_pCurGroundEntity;
	int m_pCarFire;
	float m_fSteerAngle;
	float m_fGasPedal;
	float m_fBrakePedal___maskstart;
	char VehicleCreatedBy;
	char flags_field_265;
	char flags_field_266;
	char flags_field_267;
	char _flags_field_268;
	char flags_field_269;
	char flags_field_26A;
	char field_26B;
	char possible_animtype_field_26C;
	char field_26D;
	__int16 ExtendedRemovalRange;
	char unk_nUsedForCover;
	char m_nAmmoInClip;
	char field_272;
	char m_bGarageTurnedLightsOff;
	int field_274;
	float m_fMaxHealth;
	float m_fHealth;
	float m_fEngineEnergy;
	int field_284;
	int m_nSetPieceExtendedRangeTime;
	int TimeOfLastShotFired__m_nGunFiringTime;
	int field_290;
	int m_pBlowUpEntity_pDelayedExplosionInflictor;
	float field_298;
	float field_29C;
	int m_nDoorLock;
	int m_LastTimeGunFired;
	int m_pLastDamageEntity;
	__int16 field_2AC;
	__int16 field_2AE;
	__int16 m_nBombTimer_DelayedExplosion;
	char field_2B2[2];
	int m_cHorn__m_nCarHornTimer;
	char field_2B8;
	char transmission_nNumberOfGears;
	char m_nRadioStation;
	char field_2BB;
	char field_2BC;
	char field_2BD;
	char m_vehLCS_2A3_field_2BE;
	char field_2BF;
	char m_nCarHornPattern;
	char field_2C1[2];
	char m_comedyControlState;
	int field_2C4;
	int field_2C8;
	int field_2CC;
	char possible_m_aCollPolys_field_2D0;
	char field_2D1[47];
	int field_300;
	char field_304[60];
	int field_340;
	char field_344[12];
	float fSteer;
	int m_vehicleType;
	int m_baseVehicleType;
	int m_bSuperBrake;
	char field_360[16];
	int field_370;
	int field_374;
	int field_378;
	char field_37C[4];
	char m_currentColourIndex;
	char m_currentScriptColourIndex;
	char field_382[2];
	CPed* pDriver;
	void* pPassengers[8];
	char field_3A8[8];
};
/* 473 */
struct CPed
{
	CPhysical CPhysical;
	CStoredCollPoly m_collPoly;
	float m_fCollisionSpeed;
	char field_1A4[12];
	int field_1B0;
	char field_1B4[12];
	int field_1C0;
	float m_fFallHeight;
	char _CP_flags_E;
	char CP_flags_F;
	char CP_flags_G;
	char CP_flags_H;
	char CP_flags_I;
	char CP_flags_J;
	char CP_flags_K;
	char CP_flags_L;
	char _CP_flags_field_1D0;
	char CP_flags_field_1D1;
	char CP_flags_field_1D2;
	char CP_flags_field_1D3;
	char CP_flags_field_1D4;
	char CP_flags_field_1D5;
	char CP_flags_field_1D6;
	char CP_flags_field_1D7;
	char _flags_field_1D8;
	char field_1D9;
	char field_1DA;
	char field_1DB;
	char flags_field_1DC;
	char field_1DD;
	__int16 m_gangFlags;
	char field_1E0[160];
	int m_prevObjective;
	char field_284[12];
	CVuVector m_nextRoutePointPos;
	char field_2A0[4];
	CPed* m_leader;
	int m_pedFormation;
	char m_fearFlags1;
	char m_fearFlags2;
	char m_fearFlags3;
	char m_fearFlags4;
	int m_eventType;
	int m_pEventEntity;
	char field_2B8[2];
	char field_2BA;
	char field_2BB;
	int m_pFrames[18];
	char field_304[4];
	AssocGroupId m_animGroup;
	char blendstuff_field_30C[4];
	int field_310;
	int field_314;
	char field_318[24];
	CPedIK m_ik__m_pedIK;
	char field_360[36];
	int field_384;
	int field_388;
	char field_38C[116];
	int field_400;
	char field_404[4];
	int m_pNextPathNode;
	char field_40C[52];
	CVuVector m_vecSeekPos;
	int m_pSeekTarget;
	CVector2D m_fleeFromPos;
	int m_fleeFrom;
	CVector2D m_eventOrThreat;
	int m_threatEntity;
	int m_pathNodeTimer;
	int m_followPathTargetEnt;
	__int16 m_routeLastPoint;
	__int16 m_routeStartPoint;
	__int16 m_routePointsPassed;
	__int16 m_routeType;
	__int16 m_routePointsBeingPassed;
	__int16 m_walkAroundType;
	CVehicle* m_pMyVehicle;
	CVehicle* m_carInObjective;
	char field_488[88];
	int m_followPathMoveState;
	float m_fHealth;
	float m_fArmour;
	char field_4EC[4];
	int m_nScriptShootTimer;
	int m_nScriptAttackTimer;
	float m_fSprintControlCounter;
	char field_4FC[24];
	int m_pCurrentPhysSurface;
	char field_518[36];
	int m_attractor;
	char field_540[8];
	int m_phoneTalkTimer;
	char field_54C[4];
	int m_nPedType;
	CPedStats* m_pedStats;
	int m_fleeTimer;
	CWeapon** possiblewrongoldm_weapons;
	int m_collidingEntityWhileFleeing;
	int m_collidingThingTimer;
	char field_568[12];
	CWeapon m_weapons[10];
	int possible_m_storedWeapon_field_68C;
	char field_690[32];
	int m_pFire;
	int m_pLookTarget;
	int m_fLookDirection;
	int m_lastComment;
	int m_leaveCarTimer;
	int m_getUpTimer;
	int m_lookTimer;
	int m_chatTimer;
	int m_attackTimer;
	int m_shootTimer;
	int m_carJackTimer;
	int m_objectiveTimer;
	int m_duckTimer;
	char field_6E4[4];
	int m_bloodyFootprintCountOrDeathTime;
	char field_6EC[4];
	int m_ceaseAttackTimer;
	char field_6F4[4];
	void* m_nearPeds[10];
	char field_720[40];
	int m_lastDamEntity;
	char field_74C[20];
	float f_angleThreshold_field_760;
	char field_764[8];
	int m_threatFlags;
	int m_threatCheckTimer;
	int m_threatCheckInterval;
	__int16 num_field_778;
	__int16 m_numNearPeds;
	__int16 TeamId;
	__int16 m_nPedMoney;
	__int16 vectormatrixtypestuff_field_780;
	char field_782[2];
	__int16 m_phoneId;
	char field_786[3];
	char m_currentWeapon;
	char m_nPathDir;
	char _m_holdPositionState;
	__int16 field_78C;
	char m_wepAccuracy;
	char field_78F[5];
	CVector2D m_moved;
	char field_79C[4];
	int m_delayedSoundID;
	int m_delayedSoundTimer;
	char field_7A8[4];
	int m_soundStart;
	__int16 m_lastQueuedSound;
	__int16 m_queuedSound;
	char m_canTalk;
	char field_7B5[67];
	int field_7F8;
	char field_7FC[8];
	int field_804;
	char field_808[8];
	float m_distanceToCountSeekDone;
	char field_814[8];
	int m_nPedStateTimer;
	int m_pPointGunAt;
	int _z_pointer_stuff_field_824;
	int m_fMaxHealth;
	char field_82C[4];
	float m_fBreath;
	char field_834[12];
	CVuVector m_vecSpotToGuard;
	char field_850[8];
	CEntity* m_attachedTo;
	char field_85C[68];
	CPed* m_pedInObjective;
	int m_objective;
	char field_8A8[8];
	float field_8B0;
	char field_8B4[16];
	int m_nPedState;
	int m_nLastPedState;
	int m_nWaitState;
	int m_nWaitTimer;
	int m_nMoveState;
	int field_8D8;
	int m_nPrevMoveState;
	int m_fRotationCur;
	int m_fRotationDest;
	float m_headingRate;
	char field_8EC[4];
	char scr1199_1200_stuff_field_8F0;
	char colour_op1201_1199_field_8F1;
	__int16 m_vehDoor;
	int CharCreatedBy;
	int audiomanager_setbyscript_1287_field_8F8;
	char field_8FC[4];
};
/* 557 */
struct CDamageManager
{
	float fWheelDamageEffect;
	char field_4[2];
	char m_Wheel[4];
	char field_A[10];
	int m_Panels;
	int field_18;
	int field_1C;
};

/* 532 */
struct CDoor
{
	float m_fAngle;
	float m_fPrevAngle;
	float m_fAngVel;
	int _field_C;
	CVuVector m_vecSpeed;
	float m_fOpenAngle_m_fMaxAngle;
	float m_fClosedAngle_m_fMinAngle;
	char m_nDirn;
	char m_nAxis;
	char m_nDoorState;
	char _field_2B;
	int _field_2C;
};
/* 534 */
struct CColPoint
{
	CVuVector point;
	char field_10[16];
	CVuVector normal;
	char surfaceA;
	char pieceA;
	char surfaceB;
	char pieceB;
	char field_34[12];
};

struct RwFrame;
/* 510 */
struct CAutomobile
{
	CVehicle CVehicle;
	CDamageManager Damage;
	CDoor Doors[6];
	RwFrame* m_aCarNodes[27];
	char field_55C[20];
	CColPoint m_aWheelColPoints[4];
	float m_aSuspensionSpringRatio[4];
	float m_aSuspensionSpringRatioPrev[4];
	float m_aWheelCounts___m_aWheelTimer[4];
	float lcs_m_auto_unused1;
	int field_6A4;
	int field_6A8;
	char field_6AC[32];
	float ffield_6CC;
	char field_6D0[12];
	float m_aWheelRotation[4];
	float m_aWheelPosition[4];
	float m_aWheelSpeed____andflagsstuff[4];
	char CA_flags_E;
	char CA_flags_F;
	char CA_flags_G;
	char CA_flags_H;
	int m_pBombRigger;
	__int16 nBrakesOn;
	__int16 forkliftYval_m_nSuspensionHydraulics;
	char field_718[8];
	float m_aSuspensionSpringLength[4];
	char field_730[16];
	float m_fHeightAboveRoad;
	float m_fTraction;
	float m_fTireTemperature;
	float m_fOrientation;
	float m_fPlaneSteer;
	float field_754;
	int pBouncingPanels[6];
	float m_fFireBlowUpTimer;
	char field_774[92];
	int field_7D0;
	char field_7D4[8];
	int field_7DC;
	char field_7E0[8];
	int m_ForkliftObjectHandle;
	float forklift_posfloat__field_7EC;
	char m_bIsMovingFrokliftForks;
	char field_7F1[11];
	char m_nWheelsOnGround;
	char m_nDriveWheelsOnGround;
	char m_nDriveWheelsOnGroundPrev;
	char field_7FF;
	float m_fGasPedalAudio;
	float m_aWheelState[4];
	char field_814[12];
};
/* 550 */
struct __declspec(align(1)) CPlane
{
	CAutomobile CAutomobile;
	int m_fYawControl;
	float m_fPitchControl;
	float m_fRollControl;
	float m_fThrottleControl;
	float m_fScriptThrottleControl;
	float m_fPreviousRoll;
	int m_nStallCounter;
	float m_TakeOffDirection;
	float m_LowestFlightHeight;
	float m_DesiredHeight;
	float m_MinHeightAboveTerrain;
	float m_FlightDirection;
	float m_FlightDirectionAvoidingTerrain;
	int m_OldTilt;
	int m_OnGroundTimer;
	float m_fEngineSpeed;
	float m_fPropellerAngle;
	int m_fLGearAngle;
	int m_nDamageControlWaveCounter;
	char m_FiringRateMultiplier;
	char field_86D[3];
	int field_870;
	int field_874;
	char field_878[8];
	char field_880;
	char field_881[15];
	int field_890;
	int field_894;
	int field_898;
	int field_89C;
	float field_8A0;
	char ______field_8A4[7777];
};
/* 549 */
struct __declspec(align(1)) CHeli
{
	CAutomobile CAutomobile;
	char m_nHeliFlagsA;
	char m_nHeliFlagsB;
	char m_nHeliFlagsC;
	char m_nHeliFlagsD;
	float m_fYawControl;
	float m_fPitchControl;
	float m_fRollControl;
	float m_fThrottleControl;
	float m_fEngineSpeed;
	float m_fMainRotorAngle;
	float m_fRearRotorAngle;
	float m_LowestFlightHeight;
	float m_DesiredHeight;
	float m_MinHeightAboveTerrain;
	float m_FlightDirection;
	char m_bStopFlyingForAWhile;
	char m_nSwatOnBoard__m_numSwat;
	char m_SwatRopeActive;
	char _field_853[5];
	float m_OldSearchLightX[6];
	float m_OldSearchLightY[6];
	int m_LastSearchLightSample;
	float m_SearchLightX;
	float m_SearchLightY;
	float m_SearchLightZ;
	float m_LightBrightness;
	int m_LastTimeSearchLightWasTooFarAwayToShoot__m_nShootTimer;
	int m_nNextTalkTimer;
	char m_FiringRateMultiplier;
	char m_bSearchLightOn;
	char _pad_field_8A6[2];
	float m_crashAndBurnTurnSpeed;
	char _field_8AC[8];
	char ____field_8B4[7777];
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
	void* m_Objects; // m_entries
	unsigned __int8* m_ByteMap; // m_flags
	int m_nSize; // m_size
	int m_nFirstFree; // m_allocPtr
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
	CVuVector min;
	CVuVector max;
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


/* 583 */
struct tHudElementInfo
{
	int type;
	int pointer_textkey_field_4;
	int field_8;
	int some_x_field_C;
	int some_y_field_10;
	int field_14;
	int font_style_field_18;
	float font_size_field_1C;
	CRGBA font_crgba_col_field_20;
	char bfield_24;
	char bfield_25;
	char _field_26[2];
	char field_28;
	char bfield_29;
	char _field_2A[2];
	int bfield_2C;
	int field_30;
	int field_34;
	int field_38;
	int field_3C;
	int field_40;
	int field_44;
	int field_48;
};

/* 524 */
struct tHudElement
{
	int isVisible;
	int xstart_field_4;
	int ystart_field_8;
	int xsize_field_C;
	int ysize_field_10;
	int __z_field_14;
	CRGBA BackgroundColour_crect;
	CRGBA BorderColour_crect;
	tHudElementInfo* pHudElementInfoList;
	tHudElementInfo* pHudElementInfoListEnd;
};

/* 525 */
struct CEmpireHud
{
	int pointer_some_alloc_next_field_0;
	int pointer_field_4;
	int vftable;
	tHudElement** hudElementList;
	tHudElement** hudElementListEnd;
	int __field_14;
	int xmlConfig;
	int updateTimer;
	float scaleX;
	float scaleY;
	int texListRefCount;
};

/* 500 */
struct CEmpireBuildingInfo
{
	int m_nOwnerIndex;
	int m_nBusinessType;
	int m_nScaleLevel;
	int m_nCondition;
	int m_nBuildingState;
	int m_nCurrentModelIndex;
	int m_bUseLODModel;
	int m_bIsDamaged;
	float m_fHeading;
	char _pad_field_24[12];
	CVuVector m_vecPos;
	int m_prevUseLODModel;
	void* m_pActualEmpireBuilding;
	int using_todo_null___pEmpireEntityLOD;
	int todo_null___m_nNameHash;
};

/* 499 */
struct CEmpireMgr
{
	INSTANCE INSTANCE;
	int m_nEmpiresCount;
	CEmpireBuildingInfo* m_pEmpiresInfosStart;
	CEmpireBuildingInfo* m_pEmpiresInfosEnd;
	int _pad_field_18;
	int m_pPtr__possibleassets;
	int field_20[42];
	int field_C8[21];
	int m_pendingRequestModelIndex;
	int m_aColors[9];
	char m_ownerGxtKeyIndices[9];
};

struct RwObjectNameIdAssocation
{
	const char* name;
	int32_t hierId;
	uint32_t flags;
};

/* 504 */
struct tSample
{
	int nOffset;
	int nSize;
	int nFrequency;
};

/* 322 */
struct __declspec(align(1)) cSampleManager
{
	char field_0[6];
	char field_6;
	char field_7;
	char field_8;
	char field_9[11];
	tSample* m_aSamples;
	char field_18[77777];
};

/* 596 */
struct sMissionAudioManager
{
	int bSlotDurationSet[5];
	int _pad14;
	int bSlotBusy[5];
	int _pad2C;
	int pedsHandles[5];
	int _pad44;
	int pGxtTextKeys[5];
	int _pad5C;
	int bLockSlotsRelease[5];
	int totalDuration;
	int bCanUseSlots[5];
	int _pad8C;
	int slotAudioDuration[5];
	int _padA4;
	int bSlotsResetFlags[5];
	int _padBC;
};

struct CPtrNode
{
	void* item;
	CPtrNode* prev;
	CPtrNode* next;
};

/* 603 */
struct CPtrList
{
	CPtrNode* first;
};

/* 602 */
struct CSector // (14) todo m_empireList  m_empireOverlapList
{
	CPtrList m_buildingList;
	CPtrList m_buildingOverlapList;
	CPtrList m_multiplayerList;
	CPtrList m_objectList;
	CPtrList m_objectOverlapList;
	CPtrList empire;
	CPtrList empireover;
	CPtrList m_vehicleList;
	CPtrList m_vehicleOverlapList;
	CPtrList m_pedList;
	CPtrList m_pedOverlapList;
	CPtrList unk3;
	CPtrList m_dummyList;
	CPtrList m_dummyOverlapList;
};

///* 258 */ // old
//struct __declspec(align(1)) CCutsceneMgr
//{
//	char a[12];
//	char ms_cutsceneName[8];
//	int field_14;
//	int ms_animLoaded;
//	int ms_running;
//	int ms_cutsceneProcessing;
//	char field_24[8];
//	int ms_wasCutsceneSkipped;
//	int ms_cutscenePlayStatus;
//	int ms_cutsceneLoadStatus;
//	int ms_cutsceneTimer;
//	char ms_cutsceneAssociations[20];
//	int ms_pCutsceneDir;
//	int ms_numLoadObjectNames;
//	char field_58[24];
//	int field_70;
//	char field_74[16];
//	int ms_numAttachObjectToBones;
//	char field_88[16];
//	int mCutsceneSkipFadeTime;
//	int mCutsceneSkipFading;
//	char field_A0[16];
//	int field_B0;
//	int bCamLoaded;
//	int field_B8;
//	void** ms_pCutsceneObjects;
//	char field_C0[777];
//};


struct tMusicNameIdAssoc
{
	/*const*/ char* szTrackName;
	int iTrackId;
};


struct tEngineSounds
{
	int val1;
	int val2;
};

/* 628 */
enum ModelInfoType
{
	MITYPE_NA = 0,
	MITYPE_SIMPLE = 1,
	MITYPE_MLO = 2,
	MITYPE_TIME = 3,
	MITYPE_WEAPON = 4,
	MITYPE_CLUMP = 5,
	MITYPE_VEHICLE = 6,
	MITYPE_PED = 7,
	MITYPE_XTRACOMPS = 8,
	MITYPE_HAND = 9,
};

/* 526 */
struct CBaseModelInfo
{
	int m_unkTimers[2];
	int m_nameHashKey;
	int m_chunkMdlFile;
	char m_type;
	char m_num2dEffects;
	char ownsColModel;
	char field_13;
	CColModel* m_colModel;
	__int16 m_2dEffectsID;
	__int16 m_objectId;
	__int16 m_refCount;
	__int16 m_txdSlot;
	__int16 m_interiorGroupIndex;
	__int16 field_22;
	void* vftable_MLO;
};

/* 623 */
struct RwObject;
struct CSimpleModelInfo
{
	CBaseModelInfo CBaseModelInfo;
	RwObject** m_atomics_objects;
	float m_draw_lodDistances[3];
	char m_numAtomics;
	char m_alpha;
	char flags_A_field_3A;
	char flags_B_field_3B;
	CSimpleModelInfo* m_relatedModel;
};

/* 620 */
struct CElementGroupModelInfo
{
	CBaseModelInfo CBaseModelInfo;
	int m_clump;
	int m_animFileIndexOrName;
};

struct __declspec(align(1)) tHandlingData
{
	float fDragCoeff;
	CVuVector CentreOfMass;
	char field_14[84];
	float fSteeringLock;
	char field_6C[8];
	float fSuspensionForceLevel;
	char field_78[4];
	float fSuspensionUpperLimit;
	float fSuspensionLowerLimit;
	char field_84[16];
	char FrontLights;
	char RearLights;
	char field_96[2];
	float _unk_fDragCoeff;
	char field_9C;
	int field_9D;
	char field_A1[3];
	float ffield_A4;
	char field_A8[8];
	char field_B0;
	char field_B1[11];
	float fMass;
	float fTurnMass;
	char field_C4[4];
	float fCollisionDamageMultiplier;
	int Flags;
	int mFlags;
	char field_D4[7777];
};

/* 634 */
struct __declspec(align(1)) tVehicleSampleData
{
	int m_nAccelerationSampleIndex;
	uint8_t m_nBank;
	char _field_5[3];
	int m_nHornSample;
	int32_t m_nHornFrequency;
	int32_t m_nSirenOrAlarmSample; // ?
	int32_t m_nSirenOrAlarmFrequency;
	uint8_t m_bDoorType;
};

/* 629 */
struct CVehicleModelInfo
{
	CElementGroupModelInfo CElementGroupModelInfo;
	CRGBA m_aPrevSpawnedColors[2];
	tHandlingData* m_pHandlingData;
	int m_pHandlingBike;
	void* m_pHandlingFlying;
	void* m_pHandlingBoat;
	int m_pHandlingJetski;
	int m_pHandling6atv;
	float m_normalSplay;
	int m_vehicleType;
	float m_wheelScale;
	float m_wheelScaleRear;
	CVuVector m_positions[15];
	int m_compRules;
	int m_steerAngle___m_bikeSteerAngle;
	char m_gameName[8];
	char m_nUnk160;
	char m_nNumColorVariations;
	char m_nNumScriptColorVariations;
	char m_anColorVariationIndices[16];
	char m_anScriptColorVariationIndices[16];
	char field_183;
	CRGBA m_aCurrentColor1;
	CRGBA m_aCurrentColor2;
	int possible_m_materials1_field_18C[30];
	int possible_m_materials2_field_204[25];
	RpAtomic** m_comps__m_extras;
	int m_animFileIndexOrName;
	__int16 m_wheelId_Or_m_planeLodId_union;
	__int16 m_frequency;
	char m_numDoors;
	char m_vehicleClass;
	char m_level;
	char m_numComps;
	char m_anLastChosenColorVariations[2];
	char field_27A[2];
	int flags_field_27C;
	char field_280[4];
	tVehicleSampleData m_SampleData;
	char radiostation_3f3790_field_29D;
	char field_29E;
	char field_29F;
};
/* 622 */
struct RpMaterial;
struct tPedColMat
{
	RpMaterial* material;
	char colindex_field_4;
	char _field_5[3];
};
/* 573 */
struct cRGB
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};
struct CPedModelInfo
{
	CElementGroupModelInfo CElementGroupModelInfo;
	int animGroup;
	int pedType;
	int pedStatType;
	int carsDriveMask___m_carsCanDrive;
	void* pHitColModel;
	char radio1;
	char radio2;
	char m_nLastChosenColorVariation;
	char m_nNumColorVariations;
	char m_nNumScriptColorVariations;
	uint8_t m_anColorVariationIndices[64]; // 16*4
	uint8_t m_anScriptColorVariationIndices[16]; // 4*4
	cRGB field_99[9];
	tPedColMat renderMaterials[6];
	char gameName[8];
	int field_EC;
	int field_F0;
	int field_F4;
	int field_F8;
};
struct CRope
{
	int m_nRopeType;
	int m_bActive;
	int m_bWasRegistered;
	int unk_field_C;
	int m_ID;
	int m_nTimeToBeKeptAliveTill;
	char field_18[8];
	CVuVector m_pos[32];
	CVuVector m_speed[32];
	CVehicle* m_pOwnerVehicle;
	CEntity* m_pWinchHookObject; // CObject*
	CPhysical* m_pWinchCarriedObject;
	int m_WinchDisabled;
	float m_fWinchHeight;
	char field_434[11];
	char field_43F;
};




//========================================== rw
// my + https://github.com/aap/librwgta/blob/master/tools/storiesconv/rsl.h

/* 669 */
enum RwObjectType
{
	rpFRAME = 0,
	rpATOMIC = 1,
	rpCLUMP = 2,
	rpLIGHT = 3,
	rpCAMERA = 4,
	rp5 = 5,
	rpTEXDICTIONARY = 6,
	rpWORLD = 7,
	rpGEOMETRY = 8,
};


struct RwLinkList;
/* 235 */
struct __declspec(align(4)) RwLLLink
{
	RwLLLink* next;
	RwLLLink* prev;
};

/* 150 */
struct RwLinkList
{
	RwLLLink link;
};

/* 540 */
struct RwObject
{
	char type;
	char subType;
	char flags;
	char privateFlags;
	RwObject* parent;
};

/* 645 */
struct RwObjectHasFrame
{
	RwObject object;
	RwLLLink lFrame;
	int sync;
};

/* 676 */
struct ClumpExt
{
	int visibilityCallBack;
	int alpha;
};

enum eAnimAssocFlags // Original names from sa indicate ABA_FLAG_XXX
{
	ASSOC_RUNNING = 0x1,
	ASSOC_REPEAT = 0x2,
	ASSOC_DELETEFADEDOUT = 0x4,
	ASSOC_FADEOUTWHENDONE = 0x8,
	ASSOC_PARTIAL = 0x10,
	ASSOC_MOVEMENT = 0x20, // ???
	ASSOC_HAS_TRANSLATION = 0x40,
	ASSOC_HAS_X_TRANSLATION = 0x80, // for 2d velocity extraction
	ASSOC_WALK = 0x100,             // for CPed::PlayFootSteps(void)
	ASSOC_IDLE = 0x200,             // only xpress scratch has it by default, but game adds it to player's idle animations later
	ASSOC_NOWALK = 0x400,           // see CPed::PlayFootSteps(void)
	ASSOC_BLOCK = 0x800,            // unused in assoc description, blocks other anims from being played
	ASSOC_FRONTAL = 0x1000,         // anims that we fall to front
	ASSOC_DRIVING = 0x2000,         // new in VC
	ASSOC_4000 = 0x4000,
	ASSOC_MIRROR = 0x8000, // Playback the anim mirrored horizontally
	ASSOC_10000 = 0x10000,
	ASSOC_20000 = 0x20000,
	ASSOC_SCRIPTED = 0x40000,
	ASSOC_80000 = 0x80000,
	ASSOC_JAW = 0x100000
};


/* 951 */
struct CAnimBlendLink
{
	CAnimBlendLink* next;
	CAnimBlendLink* prev;
};

/* 745 */
struct AnimBlendFrameData;
struct CAnimBlendClumpData
{
	CAnimBlendLink link;
	int numFrames;
	CVuVector* velocity2d;
	AnimBlendFrameData* frames;
};

/* 467 */
struct CAnimBlendElementGroupData // same CAnimBlendClumpData
{
	CAnimBlendLink link;
	int numFrames;
	CVector* velocity3d;
	AnimBlendFrameData* frames;
};

/* 541 */
struct RpClump
{
	RwObject object;
	RwLinkList atomicList;
	ClumpExt ClumpExt;
	CAnimBlendClumpData* pClumpAnimDataPlugin;
};

/* 970 */
struct CQuaternion
{
	//float x;
	//float y;
	//float z;
	//float w;

	RwV3d imag;
	float real;
};

/* 971 */
struct RpHAnimStdInterpFrame
{
	int field_0;
	int field_4;
	CQuaternion quad;
	int field_18;
	int field_1C;
	int field_20;
};
union uFrameData
{
	RwFrame* frame;
	RpHAnimStdInterpFrame* hanimFrame;
};

/* 747 */
struct AnimBlendFrameData
{
	char flag;
	char _field_1[3];
	RwV3d resetPos;
	uFrameData uFrameData;
	int nodeID;
};

/* 539 */
struct RpHAnimHierarchy;
struct RwFrame
{
	RwObject object;
	RwLinkList objectList;
	RwMatrix modelling_matrix;
	RwMatrix ltm;
	RwFrame* child;
	RwFrame* next;
	RwFrame* root;
	int nodeId;
	int unk;
	RpHAnimHierarchy* hierarchy;
	int name;
	int hierId;
};

/* 646 */
struct RwRaster
{
	int unk1;
	int unk2;
	char* data;
	int flags;
};

/* 564 */
struct RwTexDictionary;
struct RwTexture
{
	RwRaster* raster;
	RwTexDictionary* dict;
	RwLLLink lInDictionary;
	char name[32];
	char mask[32];
	int refCount; // ?
	int field_54; // ?
};

/* 631 */
struct RpMaterial
{
	RwTexture* texture;
	CRGBA color;
	__int16 refCount;
	__int16 pad;
	int unk2;
};

/* 648 */
struct RpMaterialList
{
	RpMaterial** materials;
	int numMaterials;
	int space;
};

/* 644 */
struct RpGeometry
{
	RwObject object;
	__int16 refCount;
	__int16 field_A;
	RpMaterialList matList;
	int field_18;
};

/* 798 */
struct Model // .mdl
{
	int lastFrame;
	int resId;
	int next;
};


///* 643 */
//struct __declspec(align(1)) RpAtomic
//{
//	RwObjectHasFrame object;
//	RpGeometry* geometry;
//	char field_18[18];
//	__int16 flags_field_2A;
//	char field_2C[4];
//	int field_30;
//};

/* 674 */
struct AtomicExt
{
	__int16 modelId;
	__int16 flags;
};

/* 969 */
struct HAnimNodeInfo
{
	char field_0;
	char field_1;
	char flags;
	char field_3;
	int field_4;
};

/* 952 */
struct RpHAnimHierarchy
{
	int flags;
	int numNodes;
	int pCurrentAnim;
	int currentTime;
	int pNextFrame;
	int pAnimCallBack;
	int pAnimCallBackData;
	int animCallBackTime;
	int pAnimLoopCallBack;
	int pAnimLoopCallBackData;
	int pMatrixArray;
	int pMatrixArrayUnaligned;
	HAnimNodeInfo* pNodeInfo;
	int field_34;
};

/* 643 */
struct RpAtomic
{
	RwObjectHasFrame object;
	RpGeometry* geometry;
	RpClump* clump;
	RwLLLink inElementGroupLink;
	void* renderCallBack;
	AtomicExt AtomicExt;
	RpHAnimHierarchy* pAtomicHAnimHierarchyPlugin;
	int field_30;
};

/* 660 */
struct RwTexDictionary
{
	RwObject object;
	RwLinkList textures__texturesInDict;
	RwLLLink lInInstance;
};

/* 659 */
struct TxdDef
{
	RwTexDictionary* texDict;
	__int16 refCount;
	__int16 refCountGu;
	char name[20];
};

/* 799 */
struct ColDef
{
	int unk;
	int loaded;
	CRect rect;
	CRect rect2;
	char name[20];
	int firstIndex;
	int lastIndex;
	int chunkData;
};







/* 561 */
struct tStreamedSample
{
	int m_nLength;
	int m_nPosition;
	int m_nLastPosCheckTimer;
};

/* 716 */
struct st32
{
	CVuVector vecpos_field_0;
	int possible_maxdist_field_10;
	int possible_trackid_field_14;
	int field_18;
	int field_1C;
};

/* 250 */
struct cMusicManager
{
	char m_bIsInitialised;
	char field_1;
	char m_bDisabled;
	char m_bSetNextStation;
	char field_4;
	char possible_nLastMaxVol_field_5;
	char possible_nLastComputedVol_field_6;
	char field_7;
	char m_nAnnouncement;
	char m_bAnnouncementInProgress;
	char field_A;
	char field_B;
	int field_C;
	char field_10;
	char field_11[3];
	int field_14;
	tStreamedSample m_aTracks[113];
	char field_564;
	char field_565[3];
	int field_568;
	char field_56C;
	char field_56D[7];
	char m_nRadioInCar;
	char m_nFrontendTrack;
	char m_nPlayingTrack;
	char m_nUpcomingMusicMode;
	char m_nMusicMode;
	char field_579;
	char m_bTrackChangeStarted;
	char m_nNextTrack;
	char m_nNextLoopFlag;
	char m_bVerifyNextTrackStartedToPlay;
	char field_57E[2];
	char field_580;
	char m_bUserResumedGame;
	char m_bMusicModeChangeStarted;
	char field_583;
	char m_bEarlyFrontendTrack;
	char field_585[3];
	int aListenTimeArray[10];
	int possible_bUseCustom3DAmbient_field_5B0;
	char field_5B4[12];
	CVuVector possible_vEmitterPos_vec_field_5C0;
	int possible_fMaxDistance_field_5D0;
	int possible_nTargetTrackId_field_5D4;
	char field_5D8[8];
	int possible_nFallbackCountdown_field_5E0;
	int field_5E4;
	int field_5E8;
	char field_5EC;
	char field_5ED[3];
	int field_5F0;
	char field_5F4;
	char field_5F5[3];
	st32* pliststart_field_5F8;
	st32* plistend_field_5FC;
	int field_600;
	int field_604;
	int musicstreamer_filepos_sub189150_field_608;
	char _189198_field_60C;
	char field_60D[3];
	int field_610;
	char field_614;
	char field_615[3];
	int _resetin18a240_field_618;
	char field_61C[3];
	char field_61F;
};

/* 615 */
struct cAudioScriptObjectManager
{
	int m_anScriptObjectEntityIndices[40];
	int m_nScriptObjectEntityTotal;
};

/* 249 */
struct cPedComments
{
	char m_aPedCommentQueue[1920];
	char m_aPedCommentOrderList[40];
	char m_nPedCommentCount[2];
	char m_nActiveQueue;
};

/* 715 */
struct cAMCrime
{
	char field_0[16];
	char field_10[16];
	char field_20[15];
	char field_2F;
};

/* 591 */
struct tAudioEntity
{
	int m_nType;
	CEntity* m_pEntity;
	char m_bIsUsed;
	char m_bStatus;
	__int16 m_awAudioEvent[4];
	char _field_12[2];
	int field_14[4];
	int field_24[4];
	int m_AudioEvents;
};

/* 588 */
struct tSound
{
	int m_nEntityIndex;
	int m_nCounter;
	/*eSfxSample*/int m_nSampleIndex;
	char m_nBankIndex;
	char m_bIs2D;
	char field_E[2];
	int m_nPriority;
	int m_nFrequency;
	char m_nVolume;
	char field_19[3];
	float m_fDistance;
	int m_nLoopCount;
	float m_fSpeedMultiplier;
	float m_MaxDistance;
	char m_bStatic;
	char field_2D[3];
	CVuVector m_vecPos;
	char m_nReflectionDelay;
	char m_bReflections;
	char m_nPan;
	char m_nFrontRearPan;
	int m_nFramesToPlay;
	char m_bIsBeingPlayed;
	char m_bIsPlayingFinished;
	char _field_4A[2];
	int m_nFinalPriority;
	char m_nVolumeChange;
	char field_51_vcs;
	char _field_52[13];
	char _field_5F;
};


/* 248 */
struct cAudioManager
{
	char m_bIsInitialised;
	char m_bIsSurround;
	char field_2;
	char m_bReduceReleasingPriority;
	char m_nActiveSamples;
	char m_bDoubleVolume;
	char m_bDynamicAcousticModelingStatus;
	char m_nChannelOffset;
	char field_8[12];
	int timerstuff_field_14;
	char field_18[24];
	tSound m_sQueueSample;
	char m_nActiveQueue;
	char _field_91[15];
	tSound m_aRequestedQueue[74];
	char m_aRequestedOrderList[74];
	char m_nRequestedCount[2];
	char field_1CAC[4];
	tSound m_asActiveSamples[37];
	tAudioEntity m_asAudioEntities[250];
	int m_aAudioEntityOrderList[250];
	int m_nAudioEntitiesCount;
	char field_652C[104];
	cAudioScriptObjectManager m_sAudioScriptObjectManager;
	char m_bIsPlayerShutUp;
	char m_nPlayerMood;
	char field_663A[2];
	int m_nPlayerMoodTimer;
	char field_6640[4];
	char m_bGenericSfx;
	char field_6645[11];
	cPedComments m_sPedComments;
	char field_6DFB[5];
	int m_nFireAudioEntity;
	int m_nWaterCannonEntity;
	int m_nPoliceChannelEntity;
	char field_6E0C[240];
	char field_6EFC;
	char field_6EFD;
	char field_6EFE;
	char field_6EFF;
	cAMCrime m_aCrimes[10];
	int m_nFrontEndEntity;
	char m_nCollisionEntity[1884];
	int m_nProjectileEntity;
	int m_nEscalatorEntity;
	int m_nExtraSoundsEntity;
	char field_784C[84];
	char m_bIsMissionAudio2D[5];
	char _field_78A5[3];
	int m_nMissionAudioSampleIndex[5];
	char m_nMissionAudioLoadingStatus[5];
	char m_nMissionAudioPlayStatus[5];
	char m_bIsMissionAudioPlaying[5];
	/*__unaligned __declspec(align(1))*/ int m_nMissionAudioFramesToPlay[5]; // ?
	char _field_78DF;
	char m_bIsMissionAudioAllowedToPlay[5];
	char m_bIsMissionAudioPhoneCall[5];
	char unk_m_nMissio[5];
	char m_nGlobalSfxVolumeMultiplier;
	int m_anRandomTable[5];
	char m_nTimeSpent;
	char m_bIsPaused;
	char m_bWasPaused;
	char field_7907;
	int m_FrameCounter;
	char field_790C[4];
	char cooldown;
	char field_7911[495];
	int field_7B00;
	char field_7B04[4];
	int field_7B08;
	char field_7B0C[179];
	char field_7BBF;
	char pedsfxstuff_field_7BC0[16];
};



/* 733 */
enum eRadarSprite
{
	RADAR_SPRITE_NONE = 0x0,
	RADAR_SPRITE_CENTRE = 0x1,
	RADAR_SPRITE_ARROW = 0x2,
	RADAR_SPRITE_NORTH = 0x3,
	RADAR_SPRITE_GUN = 0x4,
	RADAR_SPRITE_HARDWARE = 0x5,
	RADAR_SPRITE_SAVE = 0x6,
	RADAR_SPRITE_PROPERTY = 0x7,
	RADAR_SPRITE_SPRAY = 0x8,
	RADAR_SPRITE_PHONE = 0x9,
	RADAR_SPRITE_BOMB = 0xA,
	RADAR_SPRITE_CSHOP = 0xB,
	RADAR_SPRITE_POWERUP = 0xC,
	RADAR_SPRITE_MP_BASE = 0xD,
	RADAR_SPRITE_MP_CHECKPOINT = 0xE,
	RADAR_SPRITE_MP_PLAYER = 0xF,
	RADAR_SPRITE_MP_OBJECTIVE = 0x10,
	RADAR_SPRITE_MAPWAYPOINT = 0x10,
	RADAR_SPRITE_MP_CAR = 0x11,
	RADAR_SPRITE_MP_TANK = 0x12,
	RADAR_SPRITE_MP_CARLOCKUP = 0x13,
	RADAR_SPRITE_MP_TARGETPLAYER = 0x14,
	RADAR_SPRITE_ARROW_00 = 0x15,
	RADAR_SPRITE_ARROW_01 = 0x16,
	RADAR_SPRITE_ARROW_02 = 0x17,
	RADAR_SPRITE_ARROW_03 = 0x18,
	RADAR_SPRITE_ARROW_04 = 0x19,
	RADAR_SPRITE_ARROW_05 = 0x1A,
	RADAR_SPRITE_ARROW_06 = 0x1B,
	RADAR_SPRITE_ARROW_07 = 0x1C,
	RADAR_SPRITE_EMPIRE = 0x1D,
	RADAR_SPRITE_EMPIRE_DAM = 0x1E,
	RADAR_SPRITE_EMPIRE_COM = 0x1F,
	RADAR_SPRITE_EMIPREATTACK01 = 0x20,
	RADAR_SPRITE_EMIPREATTACK02 = 0x21,
	RADAR_SPRITE_EMIPREATTACK03 = 0x22,
	RADAR_SPRITE_BRYAN = 0x23,
	RADAR_SPRITE_GONZ = 0x24,
	RADAR_SPRITE_LANCE = 0x25,
	RADAR_SPRITE_LOUISE = 0x26,
	RADAR_SPRITE_MARTY = 0x27,
	RADAR_SPRITE_PHIL = 0x28,
	RADAR_SPRITE_RENI = 0x29,
	RADAR_SPRITE_DIAZ = 0x2A,
	RADAR_SPRITE_JERRY = 0x2B,
	RADAR_SPRITE_MENDEZ = 0x2C,
	RADAR_SPRITE_UMBERTO = 0x2D,
	RADAR_SPRITE_PLANE = 0x2E,
	RADAR_SPRITE_GUNSHOP = 0x2F,
	RADAR_SPRITE_USJ = 0x30,
	RADAR_SPRITE_RAMPAGE = 0x31,
	RADAR_SPRITE_BALLOON = 0x32,
	RADAR_SPRITE_BCASE = 0x33,
	RADAR_SPRITE_MPBOMB = 0x34,
	RADAR_SPRITE_BOAT = 0x35,
	RADAR_SPRITE_HELI = 0x36,
	RADAR_SPRITE_COUNT = 0x37,
	RADAR_SPRITE_ENTITY_BLIP = 0xFFFFFFFE,
	RADAR_SPRITE_COORD_BLIP = 0xFFFFFFFF,
};



/* 729 */
struct sRadarTrace
{
	int m_nColor;
	int m_eBlipType;
	int m_nEntityHandle;
	int _field_C;
	CVuVector m_vecPos;
	int possible_flags_field_20;
	float m_Radius;
	char m_wScale;
	char m_eRadarSprite;
	__int16 field_2A;
	int field_2C;
};

/* 340 */
struct CRadar
{
	INSTANCE INSTANCE;
	char field_C[72];
	int field_54;
	char field_58[528];
	__int16 field_268;
	char field_26A[22];
	sRadarTrace ms_RadarTrace[75];
	char field_1090[2400];
	void* RadarSprites[55];
	float m_radarRange;
	int field_1AD0;
	int m_FadeDownRadar;
	char field_1AD8;
	char field_1AD9;
	char field_1ADA[2];
	int field_1ADC;
	float field_1AE0;
	float field_1AE4;
	char field_1AE8[8];
	CVuVector vec_field_1AF0;
	char field_1B00[16];
	int field_1B10;
	char field_1B14[4];
	int radarTextures;
	int field_1B1C;
};


/* 752 */
struct CGroupedBuilding
{
	int m_pActualBuilding;
	int m_nGroupNameHash;
	int m_nSwapState;
	int m_nOrigModelIndex;
	int m_nReplacementModelIndex;
	int m_nCurrentID;
	int m_pParentArrayPtr;
};

/* 503 */
struct sChunkHeader
{
	int ident;
	int shrink;
	int fileEnd;
	int dataEnd;
	int relocTab;
	int numRelocs;
	int globalTab;
	__int16 numClasses;
	__int16 numFuncs;
};
struct cReloctableChunk
{
	uint32_t ident;
	uint32_t shrink;
};

/* 792 */
struct sLevelSectorDirectory
{
	sChunkHeader* header;
	int startOff;
};

/* 786 */
struct sBuildingGeometry
{
	__int16 numMeshes;
	__int16 size;
};

/* 787 */
struct sClippableBuildingMesh
{
	int packetSize;
	__int16 texID;
	__int16 uvScale[2];
	__int16 unk1;
	__int16 min[3];
	__int16 max[3];
};

///* 791 */
//struct Resource
//{
//	char* raster_or_geometry_or_raw;
//	int dmaChain;
//	int id;
//};
struct Resource
{
	union {
		RwRaster* raster;
		sBuildingGeometry* geometry;
		uint8_t* raw;
	};
	void* dmaChain;	// at runtime
	int id;
};


/* 752 */
struct SwapInfo
{
	int m_pActualBuilding;
	int m_nGroupNameHash;
	int m_nSwapState;
	int m_nOrigModelIndex;
	int m_nReplacementModelIndex;
	int m_nCurrentID;
	int m_pParentArrayPtr;
};

/* 784 */
struct sLevelSwap
{
	char timeOff;
	char timeOn;
	__int16 id;
};

/* 783 */
struct sInteriorSwap
{
	char secx;
	char secy;
	char swapSlot;
	char swapState;
	__int16 sectorId;
};

/* 785 */
struct sDynamic
{
	int scaleX;
	int scaleY;
	int scaleZ;
	__int16 modelId;
	__int16 resId;
	int posX;
	int posY;
	int posZ;
	__int16 flags;
	__int16 resId2;
	float bounds[4];
};

/* 788 */
struct AreaInfo
{
	__int16 a;
	__int16 b;
	int fileOffset;
	int fileSize;
	int numResources;
};

/* 753 */
struct sLevelChunk
{
	Resource* resourceTable;
	sLevelSectorDirectory sectorRows[36];
	sLevelSectorDirectory sectorEnd;
	int numResources;
	CVector positions[32];
	int numSwapInfos;
	//SwapInfo* swapInfos;
	CGroupedBuilding* swapInfos; // silent name
	int numLevelSwaps;
	sLevelSwap* levelSwaps;
	int numInteriors;
	sInteriorSwap* interiors;
	int numDynamics;
	sDynamic* dynamics;
	int numAreas;
	AreaInfo* areas;
};


/* 751 */
struct cWorldStream
{
	INSTANCE INSTANCE;
	int m_nDecompressedLvzSize;
	int z_field_10;
	int m_nBufferSize;
	int m_nLevelid;
	sLevelChunk* m_pWorldDataLevelChunk;
	char* m_pUncompressedLvzBuffer;
	char field_24[548];
	int field_248;
	char field_24C[20];
	int m_pCurrentInterior;
	char field_264[8];
	int field_26C;
	int _4_field_270;
	char field_274[4];
	int m_IMGUmdHandle;
	int umdcancel_umdread_field_27C;
	char field_280[408];
	int swaps_field_418[8];
	char buildingState[368];
	int field_5A8;
	char field_5AC[28];
	float ffield_5C8;
	int field_5CC;
	float m_fTextureAnimCurrentU;
	float m_fTextureAnimCurrentV;
	char field_5D8[8];
	char field_5E0[6560];
	char field_1F80[96];
	char field_1FE0[4804];
	int field_32A4;
	char field_32A8[23];
	char field_32BF;
};









/* 700 */
struct CInteriorInfo
{
	char m_nType;
	char m_nSubType;
	char m_chFilter;
	char _pad_field_3[13];
	CVuVector m_vecOffset;
	float m_fHeading;
	int _pad_AA_field_24[3];
};

/* 704 */
struct CInteriorGroup
{
	int m_count;
	CInteriorInfo* m_pEntries;
};

/* 701 */
struct CInteriorPool
{
	int m_numGroups;
	CInteriorGroup m_groups[63]; // 70?
};

/* 566 */
struct CInteriorManager
{
	INSTANCE INSTANCE;
	CInteriorPool* m_interiorPool;
};

/* 1000 */
struct CMuzzleFlash
{
	CVuVector m_vecPosition;
	CVuVector m_vecDirection;
	int8_t m_nType;
	char pad_field_21[14];
	char pad_field_2F;
};




/* 803 */
struct CPathNode // 10bytes
{
	__int16 posX; //  worldX = ((real)DTZX / 8)  (x*0.125)      ipl = [((real)DTZX / 8) * 16 ??]
	__int16 posY;
	char posZ;
	char width;
	__int16 firstLink; // 0
	char numLinks_AndFlags8; // 0123num ??, 4_bDeadEnd?, 5_bDisabled?, 6_bBetweenLevels?, 7_bUseInRoadBlock
	char flags9; // 0_bWaterPath?, 1_bOnlySmallBoats?, 2_bSelected?, 3_speedLimit[b0]?, 4_speedLimit[b1]?, 5_spawnRate[b0]?, 6_spawnRate[b1], 7_spawnRate[b2]

};
struct CPathConnection
{
	uint16_t nNodeIndex : 14;
	uint16_t bTrafficLight : 1;
	uint16_t bCrossesRoad : 1;
};
struct CCarPathLink
{
	////uint16_t idx : 14;
	//uint16_t bTrafficLight : 1; // ?
	//uint16_t bCrossesRoad : 1; // ?

	uint16_t pathNodeIndex : 14;
	uint16_t trafficLightType : 2;

	int8_t numLeftLanes : 3;
	int8_t numRightLanes : 3;
	int8_t trafficLightDirection : 1; // ?
	int8_t unk4 : 1; // ?

	int8_t unk5;
};
/* 636 */
struct __declspec(align(2)) CPathFind
{
	CPathNode* m_pathNodes; // 0x21A4E0B0  end 0x21A6284E    0x1479E 83870 / 10 = 8387   m_numPathNodes
	CCarPathLink* m_carPathLinks; // 0x21A3A770     end 0x21A3D914   0x31A4  12708 / 4 = 3177   m_numCarPathLinks
	CPathConnection* m_carPathConnections;

	int m_numPathNodes; // 8387
	int m_numCarPathNodes;
	int m_numPedPathNodes;
	__int16 m_numMapObjects;
	__int16 m_numConnections;
	__int16 m_numCarPathConnections;
	__int16 pad_AAAA_field_1E;
	int m_numCarPathLinks; // 3177

	__int16 pad_AAAA_field_24;
	__int16 field_26;
	char AAAAAA_field_28[1608];
	__int16 field_670;
	__int16 field_672;
	int field_674;
	char field_678[29988];
	__int16 field_7B9C;
	__int16 field_7B9E;

	CPathConnection* m_connections;
	__int8* m_distances;

	int field_7BA8; // -----\/ ???
	int field_7BAC;
	int field_7BB0;
	char field_7BB4[496];
	int field_7DA4;
	int field_7DA8;
	int field_7DAC;
	char __field_7DB0[5520];
	CPathNode m_aPathNodes[8387];
	int field_1DADE;
};


struct script_sphere_struct
{
	CVuVector m_vecCenter;
	int m_Id;
	float m_fRadius;
	__int16 m_Index;
	char m_bInUse;
	char field_1B;
	char m_Type;
	char field_1D;
	char field_1E;
	char field_1F;
};

/* 670 */
struct C3dMarker
{
	CMatrix m_Matrix;
	int field_44;
	int field_48;
	char field_4C[4];
	RpAtomic* m_pAtomic;
	int m_pMaterial;
	int m_nIdentifier;
	CRGBA m_Color;
	int m_nStartTime;
	float m_fPulseFraction;
	float m_fSize;
	float m_fStdSize;
	float m_fBrightness;
	float m_fCameraRange;
	int field_78;
	__int16 m_nType;
	__int16 m_nPulsePeriod;
	__int16 m_nRotateRate;
	__int16 _field_82;
	char field_84[11];
	char field_8F;
};


/* 699 */
struct CAnimAssocGroup
{
	char groupname[24];
	char blockname[20];
	int animBase;
	int numAnims;
};
/* 743 */
struct CAnimBlendKeyFrame
{
	char field_0[7777];
};

/* 938 */
struct CAnimBlock
{
	char m_name[20];
	int m_loaded;
	int m_numRefs;
	int m_animIndex;
	int m_numAnims;
	void* chunkData;
	int field_28;
	int field_2C;
};


/* 748 */
struct __declspec(align(1)) tCCutscenemgrGlobals
{
	char todo_ms_aUncompressedCutsceneAnims_field_0[256];
	char field_100[2028];
	char field_8EC[1316];
	char ms_cTextOutput[512];
	char field_1010[256];
	int ms_iTextDuration[64];
	char field_1210[3296];
	void* ms_pCutsceneObjects[50];
	char ms_aUncompressedCutsceneAnims[256];
	char field_20B8[600];
	int field_2310;
	char field_2314[7777];
};

/* 936 */
struct KeyFrame
{
	__int16 x;
	__int16 y;
	__int16 z;
	__int16 w;
	__int16 dt;
};

/* 742 */
struct CAnimBlendSequence // анимка кости (12 0xC)
{
	__int16 flag;
	__int16 numFrames;
	KeyFrame* keyFrames;
	__int16 boneTag;
	__int16 unk;
};

/* 741 */
struct CAnimBlendAssociation;
struct CAnimBlendNode
{
	float m_fTheta;
	float m_fOOSinTheta;
	int m_iCurrentFrame;
	int m_iPreviousFrame;
	float m_fTimeRemaining;
	CAnimBlendSequence* m_pSequence;
	CAnimBlendAssociation* m_pAssociation;
	CAnimBlendSequence* m_pMirroredSequence;
};

/* 744 */
struct CAnimBlendAssocGroup
{
	CAnimBlock* m_pAnimBlock;
	CAnimBlendAssociation* m_aAssociationArray;
	int numAssociations;
	int firstAnimId;
	int groupId;
};

/* 937 */
struct CAnimBlendTree
{
	CAnimBlendSequence* blendSequences; // кости с кадрами
	char name[24];
	__int16 numSequences;
	char loadSpecial;
	char compressed;
	int totalLength;
	int unk;
};

/* 536 */
struct CAnimBlendAssociation // шуруем анимку
{
	CAnimBlendLink link;
	int m_bitsFlags;
	float timeStep;
	float m_fBlendAmount;
	int m_syncId;
	int m_iNumAnimBlendNodes;
	CAnimBlendNode* m_pAnimBlendNodes; // плеер пачки фоток, перелистывает
	CAnimBlendTree* m_pAnimBlendHierarchy; // сами анимации
	float m_fBlendDelta;
	float m_fCurrentTime;
	float m_fSpeed;
	__int16 animId;
	__int16 groupId;
	int callbackType;
	int callback;
	int callbackArg;
	int m_lastAnimState;
};// 0x44  68


/* 258 */
struct __declspec(align(1)) CCutsceneMgr
{
	INSTANCE INSTANCE;
	char ms_cutsceneName[8];
	int field_14;
	int ms_animLoaded;
	int ms_running;
	int ms_cutsceneProcessing;
	int field_24;
	int field_28;
	int ms_wasCutsceneSkipped;
	int ms_cutscenePlayStatus;
	int ms_cutsceneLoadStatus;
	int ms_cutsceneTimer;
	CAnimBlendAssocGroup ms_cutsceneAssociations;
	int ms_pCutsceneDir;
	int ms_numLoadObjectNames;
	int field_58;
	int ms_currTextOutput;
	int field_60;
	int field_64;
	int ms_numCutsceneObjs;
	char field_6C[4];
	int field_70;
	char field_74[12];
	int ms_numUncompressedCutsceneAnims;
	int ms_numAttachObjectToBones;
	int field_88;
	int field_8C;
	int field_90;
	int field_94;
	int mCutsceneSkipFadeTime;
	int mCutsceneSkipFading;
	int NumberOfSavedWeapons;
	char field_A4[8];
	int bModelsRemovedForCutscene;
	int field_B0;
	int bCamLoaded;
	int bCamFading;
	tCCutscenemgrGlobals* m_pCutsceneGlobals;
	char field_C0[777];
};

/* 939 */
struct AnimAssocDefinition
{
	char pName[24];
	char pBlockName[20];
	int firstAnim;
	int numAnims;
};

/* 940 */
struct AnimDescriptor
{
	int id;
	char name[20];
	char field_18[24];
	char field_30;
	char field_31[3];
	int defaultFlags;
};

///* 937 */
//struct CAnimBlendTree
//{
//	CAnimBlendSequence* blendSequences;
//	char name[24];
//	__int16 numSequences;
//	char loadSpecial;
//	char compressed;
//	int totalLength;
//	int unk;
//};

/* 361 */
struct CAnimManagerInst
{
	AnimAssocDefinition m_aAnimAssocDefinitions[200];
	AnimDescriptor m_aAnimDescriptors[990];
	int m_numAnimAssocDefinitions;
	int m_numAnimDescriptors;
	int m_numAnimationIds;
	CAnimBlendTree* m_aAnimations;
	CAnimBlock* m_aAnimBlocks;
	int m_numAnimations;
	int m_numAnimBlocks;
	CAnimBlendAssocGroup* m_aAnimAssocGroups;
};

/* 941 */
struct CAnimManager
{
	INSTANCE INSTANCE;
	CAnimManagerInst* mspInst;
};

#pragma pack(pop)