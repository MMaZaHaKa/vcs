#pragma once
#include "hdr.h"
#include "Windows.h"
#include <iostream>
//#define FIX_BUGS
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define nil NULL

//------------------flags
enum
{
    CLUMP_FLAG_NO_HIERID = 0x1,
    VEHICLE_FLAG_COLLAPSE = 0x2,
    VEHICLE_FLAG_ADD_WHEEL = 0x4,
    VEHICLE_FLAG_POS = 0x8,
    VEHICLE_FLAG_DOOR = 0x10,
    VEHICLE_FLAG_LEFT = 0x20,
    VEHICLE_FLAG_RIGHT = 0x40,
    VEHICLE_FLAG_FRONT = 0x80,
    VEHICLE_FLAG_REAR = 0x100,
    VEHICLE_FLAG_COMP = 0x200,
    VEHICLE_FLAG_DRAWLAST = 0x400,
    VEHICLE_FLAG_WINDSCREEN = 0x800,
    VEHICLE_FLAG_ANGLECULL = 0x1000,
    VEHICLE_FLAG_REARDOOR = 0x2000,
    VEHICLE_FLAG_FRONTDOOR = 0x4000,
    VEHICLE_FLAG_UNK1 = 0x8000, // unused
    VEHICLE_FLAG_UNK2 = 0x10000, // wheel_rf_dummy (heli, plane)
    VEHICLE_FLAG_UNK3 = 0x20000, // unused
    VEHICLE_FLAG_UNK4 = 0x40000, // moving_rotor, moving_rotor2, static_rotor, static_rotor2 (boat, heli, plane)
    VEHICLE_FLAG_UNK5 = 0x80000, // unused
    VEHICLE_FLAG_UNK6 = 0x100000, // unused
    VEHICLE_FLAG_UNK7 = 0x200000, // unused
    VEHICLE_FLAG_UNK8 = 0x400000, // door_rf_dummy, door_rr_dummy, door_lf_dummy, door_lr_dummy (heli, plane)
};

//----------------------positions
enum eCarPositions
{
    CAR_POS_HEADLIGHTS,
    CAR_POS_TAILLIGHTS,
    CAR_POS_HEADLIGHTS_2,
    CAR_POS_TAILLIGHTS_2,
    CAR_POS_FRONTSEAT,
    CAR_POS_BACKSEAT,
    CAR_POS_EXHAUST,
    NUM_CAR_POS
};

enum eBoatPositions
{
    BOAT_POS_FRONTSEAT,
    NUM_BOAT_POS
};

enum eJetskiPositions
{
    JETSKI_POS_FRONTSEAT,
    NUM_JETSKI_POS
};

enum eTrainPositions
{
    TRAIN_POS_LIGHT_FRONT,
    TRAIN_POS_LIGHT_REAR,
    TRAIN_POS_LEFT_ENTRY,
    TRAIN_POS_MID_ENTRY,
    TRAIN_POS_RIGHT_ENTRY,
    NUM_TRAIN_POS
};

enum eHeliPositions
{
    HELI_POS_HEADLIGHTS,
    HELI_POS_TAILLIGHTS,
    HELI_POS_HEADLIGHTS_2,
    HELI_POS_TAILLIGHTS_2,
    HELI_POS_FRONTSEAT,
    HELI_POS_BACKSEAT,
    HELI_POS_EXHAUST,
    HELI_POS_ENGINE,
    HELI_POS_PETROL_CAP,
    HELI_POS_HOOKUP,
    HELI_POS_BARGRIP,
    HELI_POS_MISCPOS_C, // :/
    HELI_POS_MISCPOS_D, // :/
    HELI_POS_MISCPOS_A,
    HELI_POS_MISCPOS_B,
    NUM_HELI_POS
};

enum ePlanePositions
{
    PLANE_POS_HEADLIGHTS,
    PLANE_POS_TAILLIGHTS,
    PLANE_POS_HEADLIGHTS_2,
    PLANE_POS_TAILLIGHTS_2,
    PLANE_POS_FRONTSEAT,
    PLANE_POS_BACKSEAT,
    PLANE_POS_EXHAUST,
    PLANE_POS_ENGINE,
    PLANE_POS_PETROL_CAP,
    PLANE_POS_AILERON,
    PLANE_POS_ELEVATOR,
    PLANE_POS_RUDDER,
    PLANE_POS_WINGTIP,
    PLANE_POS_MISCPOS_A,
    PLANE_POS_MISCPOS_B,
    NUM_PLANE_POS
};

enum eBikePositions
{
    BIKE_POS_HEADLIGHTS,
    BIKE_POS_TAILLIGHTS,
    BIKE_POS_HEADLIGHTS_2,
    BIKE_POS_TAILLIGHTS_2,
    BIKE_POS_FRONTSEAT,
    BIKE_POS_BACKSEAT,
    BIKE_POS_EXHAUST,
    NUM_BIKE_POS
};

enum eFerryPositions
{
    FERRY_POS_LIGHT_FRONT,
    FERRY_POS_LIGHT_REAR,
    FERRY_POS_CHIM_LEFT,
    FERRY_POS_PED_POINT,
    FERRY_POS_CAR1,
    FERRY_POS_CAR2,
    FERRY_POS_CAR3,
    FERRY_POS_CAR4,
    NUM_FERRY_POS
};

enum eBmxPositions
{
    BMX_POS_HEADLIGHTS,
    BMX_POS_TAILLIGHTS,
    BMX_POS_HEADLIGHTS_2,
    BMX_POS_TAILLIGHTS_2,
    BMX_POS_FRONTSEAT,
    BMX_POS_BACKSEAT,
    BMX_POS_EXHAUST,
    BMX_POS_ENGINE,
    BMX_POS_PETROL_CAP,
    BMX_POS_HOOKUP,
    BMX_POS_BARGRIP,
    BMX_POS_MISCPOS_A,
    BMX_POS_MISCPOS_B,
    NUM_BMX_POS
};

enum eQuadBikePositions
{
    QUADBIKE_POS_HEADLIGHTS,
    QUADBIKE_POS_TAILLIGHTS,
    QUADBIKE_POS_HEADLIGHTS_2,
    QUADBIKE_POS_TAILLIGHTS_2,
    QUADBIKE_POS_FRONTSEAT,
    QUADBIKE_POS_BACKSEAT,
    QUADBIKE_POS_EXHAUST,
    QUADBIKE_POS_ENGINE,
    QUADBIKE_POS_PETROL_CAP,
    QUADBIKE_POS_HOOKUP,
    QUADBIKE_POS_BARGRIP,
    QUADBIKE_POS_MISCPOS_C, // :/
    QUADBIKE_POS_MISCPOS_D, // :/
    QUADBIKE_POS_MISCPOS_A,
    QUADBIKE_POS_MISCPOS_B,
    NUM_QUADBIKE_POS
};

enum
{
    //NUM_VEHICLE_POSITIONS = 5
    NUM_VEHICLE_POSITIONS = Max(NUM_CAR_POS,
                            Max(NUM_BOAT_POS,
                            Max(NUM_JETSKI_POS,
                            Max(NUM_TRAIN_POS,
                            Max(NUM_HELI_POS,
                            Max(NUM_PLANE_POS,
                            Max(NUM_BIKE_POS,
                            Max(NUM_FERRY_POS,
                            Max(NUM_BMX_POS, NUM_QUADBIKE_POS)))))))))
};

//-------------nodes

enum eCarNodes
{
    CAR_NODE_NONE = 0,
    CAR_CHASSIS,
    CAR_WHEEL_RF,
    CAR_WHEEL_RM,
    CAR_WHEEL_RB,
    CAR_WHEEL_LF,
    CAR_WHEEL_LM,
    CAR_WHEEL_LB,
    CAR_DOOR_RF,
    CAR_DOOR_RR,
    CAR_DOOR_LF,
    CAR_DOOR_LR,
    CAR_BUMP_FRONT,
    CAR_BUMP_REAR,
    CAR_WING_RF,
    CAR_WING_LF,
    CAR_WING_RR,
    CAR_WING_LR,
    CAR_BONNET,
    CAR_BOOT,
    CAR_WINDSCREEN,
    CAR_UNK_21, // todo
    CAR_FORKS,
    NUM_CAR_NODES,
};

enum eBoatNodes
{
    BOAT_NODE_NONE = 0,
    BOAT_MOVING,
    BOAT_WINDSCREEN,
    BOAT_RUDDER,
    BOAT_FLAP_LEFT,
    BOAT_FLAP_RIGHT,
    BOAT_REARFLAP_LEFT,
    BOAT_REARFLAP_RIGHT,
    BOAT_WATEROUT,
    BOAT_STATIC_ROTOR,
    BOAT_MOVING_ROTOR,
    NUM_BOAT_NODES
};

enum eJetskiNodes
{
    JETSKI_NODE_NONE = 0,
    JETSKI_CHASSIS,
    JETSKI_FORKS_FRONT,
    JETSKI_HANDLEBARS,
    NUM_JETSKI_NODES
};

enum eTrainNodes
{
    TRAIN_DOOR_LHS = 1,
    TRAIN_DOOR_RHS,
    NUM_TRAIN_NODES
};

enum eHeliNodes // real heli new !!!
{
    HELI_NODE_NONE = 0,
    HELI_CHASSIS,
    HELI_WHEEL_RF,
    HELI_WHEEL_RM,
    HELI_WHEEL_RB,
    HELI_WHEEL_LF,
    HELI_WHEEL_LM,
    HELI_WHEEL_LB,
    HELI_DOOR_RF,
    HELI_DOOR_RR,
    HELI_DOOR_LF,
    HELI_DOOR_LR,
    HELI_STATIC_ROTOR,
    HELI_MOVING_ROTOR,
    HELI_STATIC_ROTOR_2,
    HELI_MOVING_ROTOR_2,
    HELI_RUDDER,
    HELI_ELEVATORS,
    HELI_MISC_A,
    HELI_MISC_B,
    HELI_MISC_C,
    HELI_MISC_D,
    NUM_HELI_NODES
};

enum ePlaneNodes // real plane new !!!
{
    PLANE_NODE_NONE = 0,
    PLANE_CHASSIS,
    PLANE_WHEEL_RF,
    PLANE_WHEEL_RM,
    PLANE_WHEEL_RB,
    PLANE_WHEEL_LF,
    PLANE_WHEEL_LM,
    PLANE_WHEEL_LB,
    PLANE_DOOR_RF,
    PLANE_DOOR_RR,
    PLANE_DOOR_LF,
    PLANE_DOOR_LR,
    PLANE_STATIC_PROP,
    PLANE_MOVING_PROP,
    PLANE_STATIC_PROP_2,
    PLANE_MOVING_PROP_2,
    PLANE_RUDDER,
    PLANE_ELEVATOR_RL,
    PLANE_ELEVATOR_RR,
    PLANE_AILERON_L,
    PLANE_AILERON_R,
    PLANE_GEAR_L,
    PLANE_GEAR_R,
    PLANE_MISC_A,
    PLANE_MISC_B,
    PLANE_ELEVATOR_FL,
    PLANE_ELEVATOR_FR,
    NUM_PLANE_NODES
};

enum eBikeNodes
{
    BIKE_NODE_NONE = 0,
    BIKE_CHASSIS,
    BIKE_FORKS_FRONT,
    BIKE_FORKS_REAR,
    BIKE_WHEEL_FRONT,
    BIKE_WHEEL_REAR,
    BIKE_MUDGUARD,
    BIKE_HANDLEBARS,
    BIKE_NUM_NODES
};

enum eFerryNodes
{
    FERRY_DOOR_FRONT = 1,
    FERRY_RAMP_FRONT,
    FERRY_DOOR_BACK,
    FERRY_RAMP_BACK,
    NUM_FERRY_NODES
};

enum eBmxNodes
{
    BMX_NODE_NONE = 0,
    BMX_CHASSIS,
    BMX_FORKS_FRONT,
    BMX_FORKS_REAR,
    BMX_WHEEL_FRONT,
    BMX_WHEEL_REAR,
    BMX_HANDLEBARS,
    BMX_CHAINSET,
    BMX_PEDAL_R,
    BMX_PEDAL_L,
    NUM_BMX_NODES
};

enum eQuadBikeNodes
{
    QUAD_NODE_NONE = 0,
    quad_unk_possible_chassis_dummy, // todo!!!
    QUAD_WHEEL_RF,
    QUAD_WHEEL_RM,
    QUAD_WHEEL_RB,
    QUAD_WHEEL_LF,
    QUAD_WHEEL_LM,
    QUAD_WHEEL_LB,
    QUAD_DOOR_RF,
    QUAD_DOOR_RR,
    QUAD_DOOR_LF,
    QUAD_DOOR_LR,
    QUAD_CHASSIS,
    QUAD_BODY_FRONT,
    QUAD_BODY_REAR,
    QUAD_SUSPENSION_RF,
    QUAD_SUSPENSION_LF,
    QUAD_REAR_AXLE,
    QUAD_HANDLEBARS,
    QUAD_MISC_A,
    QUAD_MISC_B,
    NUM_QUAD_NODES
};

RwObjectNameIdAssocation carIds[] = {
    { "chassis", CAR_CHASSIS, 0 },
    { "wheel_rf_dummy", CAR_WHEEL_RF, VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_rm_dummy", CAR_WHEEL_RM, VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_rb_dummy", CAR_WHEEL_RB, VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lf_dummy", CAR_WHEEL_LF, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lm_dummy", CAR_WHEEL_LM, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lb_dummy", CAR_WHEEL_LB, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "bump_front_dummy", CAR_BUMP_FRONT, VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "bonnet_dummy", CAR_BONNET, VEHICLE_FLAG_COLLAPSE },
    { "wing_rf_dummy", CAR_WING_RF, VEHICLE_FLAG_COLLAPSE },
    { "wing_rr_dummy", CAR_WING_RR, VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_COLLAPSE },
    { "door_rf_dummy", CAR_DOOR_RF, VEHICLE_FLAG_FRONTDOOR | VEHICLE_FLAG_ANGLECULL | VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_rr_dummy", CAR_DOOR_RR, VEHICLE_FLAG_REARDOOR | VEHICLE_FLAG_ANGLECULL | VEHICLE_FLAG_REAR | VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "wing_lf_dummy", CAR_WING_LF, VEHICLE_FLAG_COLLAPSE },
    { "wing_lr_dummy", CAR_WING_LR, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_COLLAPSE },
    { "door_lf_dummy", CAR_DOOR_LF, VEHICLE_FLAG_FRONTDOOR | VEHICLE_FLAG_ANGLECULL | VEHICLE_FLAG_LEFT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_lr_dummy", CAR_DOOR_LR, VEHICLE_FLAG_REARDOOR | VEHICLE_FLAG_ANGLECULL | VEHICLE_FLAG_REAR | VEHICLE_FLAG_LEFT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "boot_dummy", CAR_BOOT, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "bump_rear_dummy", CAR_BUMP_REAR, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "windscreen_dummy", CAR_WINDSCREEN, VEHICLE_FLAG_WINDSCREEN | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "ped_frontseat", CAR_POS_FRONTSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_backseat", CAR_POS_BACKSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights", CAR_POS_HEADLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights", CAR_POS_TAILLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "exhaust", CAR_POS_EXHAUST, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "extra1", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra2", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra3", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra4", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra5", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra6", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "forks1", CAR_FORKS, 0 },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation boatIds[] = {
    { "boat_moving_hi", BOAT_MOVING, 0 },
    { "boat_rudder_hi", BOAT_RUDDER, 0 },
    { "boat_flap_left", BOAT_FLAP_LEFT, 0 },
    { "boat_flap_right", BOAT_FLAP_RIGHT, 0 },
    { "boat_rearflap_left", BOAT_REARFLAP_LEFT, 0 },
    { "boat_rearflap_right", BOAT_REARFLAP_RIGHT, 0 },
    // let's just accept both
    { "windscreen", BOAT_WINDSCREEN, VEHICLE_FLAG_WINDSCREEN }, // VCS: VEHICLE_FLAG_DRAWLAST missed!!
    //{ "windscreen_hi_ok", BOAT_WINDSCREEN, VEHICLE_FLAG_WINDSCREEN | VEHICLE_FLAG_DRAWLAST },
    { "ped_frontseat", BOAT_POS_FRONTSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "waterout", BOAT_WATEROUT, VEHICLE_FLAG_COMP },
    { "static_rotor", BOAT_STATIC_ROTOR, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "moving_rotor", BOAT_MOVING_ROTOR, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation jetskiIds[] = {
    { "chassis_dummy", JETSKI_CHASSIS, 0 },
    { "forks_front", JETSKI_FORKS_FRONT, 0 },
    { "handlebars", JETSKI_HANDLEBARS, 0 },
    { "ped_frontseat", JETSKI_POS_FRONTSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation trainIds[] = {
    { "door_lhs_dummy", TRAIN_DOOR_LHS, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_COLLAPSE },
    { "door_rhs_dummy", TRAIN_DOOR_RHS, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_COLLAPSE },
    { "light_front", TRAIN_POS_LIGHT_FRONT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "light_rear", TRAIN_POS_LIGHT_REAR, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_left_entry", TRAIN_POS_LEFT_ENTRY, VEHICLE_FLAG_DOOR | VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_mid_entry", TRAIN_POS_MID_ENTRY, VEHICLE_FLAG_DOOR | VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_right_entry", TRAIN_POS_RIGHT_ENTRY, VEHICLE_FLAG_DOOR | VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation heliIds[] = {
    { "chassis", HELI_CHASSIS, 0 },
    { "wheel_rf_dummy", HELI_WHEEL_RF, VEHICLE_FLAG_UNK2 | VEHICLE_FLAG_RIGHT },
    { "wheel_rm_dummy", HELI_WHEEL_RM, VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_rb_dummy", HELI_WHEEL_RB, VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lf_dummy", HELI_WHEEL_LF, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lm_dummy", HELI_WHEEL_LM, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lb_dummy", HELI_WHEEL_LB, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "door_rf_dummy", HELI_DOOR_RF, VEHICLE_FLAG_UNK8 | VEHICLE_FLAG_FRONTDOOR | VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_rr_dummy", HELI_DOOR_RR, VEHICLE_FLAG_UNK8 | VEHICLE_FLAG_REARDOOR | VEHICLE_FLAG_REAR | VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_lf_dummy", HELI_DOOR_LF, VEHICLE_FLAG_UNK8 | VEHICLE_FLAG_FRONTDOOR | VEHICLE_FLAG_LEFT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_lr_dummy", HELI_DOOR_LR, VEHICLE_FLAG_UNK8 | VEHICLE_FLAG_REARDOOR | VEHICLE_FLAG_REAR | VEHICLE_FLAG_LEFT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "static_rotor", HELI_STATIC_ROTOR, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COLLAPSE },
    { "moving_rotor", HELI_MOVING_ROTOR, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COLLAPSE },
    { "static_rotor2", HELI_STATIC_ROTOR_2, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_COLLAPSE },
    { "moving_rotor2", HELI_MOVING_ROTOR_2, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_COLLAPSE },
    { "rudder", HELI_RUDDER, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "elevators", HELI_ELEVATORS, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "misc_a", HELI_MISC_A, VEHICLE_FLAG_COLLAPSE },
    { "misc_b", HELI_MISC_B, VEHICLE_FLAG_COLLAPSE },
    { "misc_c", HELI_MISC_C, VEHICLE_FLAG_COLLAPSE },
    { "misc_d", HELI_MISC_D, VEHICLE_FLAG_COLLAPSE },
    { "ped_frontseat", HELI_POS_FRONTSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_backseat", HELI_POS_BACKSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights", HELI_POS_HEADLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights", HELI_POS_TAILLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights2", HELI_POS_HEADLIGHTS_2, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights2", HELI_POS_TAILLIGHTS_2, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "exhaust", HELI_POS_EXHAUST, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "engine", HELI_POS_ENGINE, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "petrolcap", HELI_POS_PETROL_CAP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "hookup", HELI_POS_HOOKUP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_arm", HELI_POS_BARGRIP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_c", HELI_POS_MISCPOS_C, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_d", HELI_POS_MISCPOS_D, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_a", HELI_POS_MISCPOS_A, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_b", HELI_POS_MISCPOS_B, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "extra1", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra2", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra3", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra4", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra5", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra6", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation planeIds[] = {
    { "chassis", PLANE_CHASSIS, 0 },
    { "wheel_rf_dummy", PLANE_WHEEL_RF, VEHICLE_FLAG_UNK2 | VEHICLE_FLAG_RIGHT },
    { "wheel_rm_dummy", PLANE_WHEEL_RM, VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_rb_dummy", PLANE_WHEEL_RB, VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lf_dummy", PLANE_WHEEL_LF, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lm_dummy", PLANE_WHEEL_LM, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "wheel_lb_dummy", PLANE_WHEEL_LB, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_ADD_WHEEL },
    { "door_rf_dummy", PLANE_DOOR_RF, VEHICLE_FLAG_UNK8 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_rr_dummy", PLANE_DOOR_RR, VEHICLE_FLAG_UNK8 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_lf_dummy", PLANE_DOOR_LF, VEHICLE_FLAG_UNK8 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_lr_dummy", PLANE_DOOR_LR, VEHICLE_FLAG_UNK8 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "static_prop", PLANE_STATIC_PROP, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "moving_prop", PLANE_MOVING_PROP, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "static_prop2", PLANE_STATIC_PROP_2, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "moving_prop2", PLANE_MOVING_PROP_2, VEHICLE_FLAG_UNK4 | VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "rudder", PLANE_RUDDER, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "elevator_l", PLANE_ELEVATOR_RL, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "elevator_r", PLANE_ELEVATOR_RR, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "aileron_l", PLANE_AILERON_L, VEHICLE_FLAG_COLLAPSE },
    { "aileron_r", PLANE_AILERON_R, VEHICLE_FLAG_COLLAPSE },
    { "gear_l", PLANE_GEAR_L, 0 },
    { "gear_r", PLANE_GEAR_R, 0 },
    { "misc_a", PLANE_MISC_A, 0 },
    { "misc_b", PLANE_MISC_B, 0 },
    { "f_elevator_l", PLANE_ELEVATOR_FL, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "f_elevator_r", PLANE_ELEVATOR_FR, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "ped_frontseat", PLANE_POS_FRONTSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_backseat", PLANE_POS_BACKSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights", PLANE_POS_HEADLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights", PLANE_POS_TAILLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights2", PLANE_POS_HEADLIGHTS_2, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights2", PLANE_POS_TAILLIGHTS_2, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "exhaust", PLANE_POS_EXHAUST, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "engine", PLANE_POS_ENGINE, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "petrolcap", PLANE_POS_PETROL_CAP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "aileron_pos", PLANE_POS_AILERON, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "elevator_pos", PLANE_POS_ELEVATOR, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "rudder_pos", PLANE_POS_RUDDER, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "wingtip_pos", PLANE_POS_WINGTIP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_a", PLANE_POS_MISCPOS_A, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_b", PLANE_POS_MISCPOS_B, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "extra1", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra2", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra3", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra4", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra5", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra6", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation bikeIds[] = {
    { "chassis_dummy", BIKE_CHASSIS, 0 },
    { "forks_front", BIKE_FORKS_FRONT, 0 },
    { "forks_rear", BIKE_FORKS_REAR, 0 },
    { "wheel_front", BIKE_WHEEL_FRONT, 0 },
    { "wheel_rear", BIKE_WHEEL_REAR, 0 },
    { "mudguard", BIKE_MUDGUARD, 0 },
    { "handlebars", BIKE_HANDLEBARS, 0 },
    { "ped_frontseat", BIKE_POS_FRONTSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_backseat", BIKE_POS_BACKSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights", BIKE_POS_HEADLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights", BIKE_POS_TAILLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "exhaust", BIKE_POS_EXHAUST, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "extra1", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra2", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra3", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra4", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra5", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra6", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation ferryIds[] = {
    { "door_front_dummy", FERRY_DOOR_FRONT, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_COLLAPSE },
    { "door_back_dummy", FERRY_DOOR_BACK, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_COLLAPSE },
    { "ramp_front_dummy", FERRY_RAMP_FRONT, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_COLLAPSE },
    { "ramp_back_dummy", FERRY_RAMP_BACK, VEHICLE_FLAG_LEFT | VEHICLE_FLAG_COLLAPSE },
    { "light_front", FERRY_POS_LIGHT_FRONT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "light_rear", FERRY_POS_LIGHT_REAR, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "chim_left", FERRY_POS_CHIM_LEFT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_point", FERRY_POS_PED_POINT, VEHICLE_FLAG_DOOR | VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "car1_dummy", FERRY_POS_CAR1, VEHICLE_FLAG_DOOR | VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "car2_dummy", FERRY_POS_CAR2, VEHICLE_FLAG_DOOR | VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "car3_dummy", FERRY_POS_CAR3, VEHICLE_FLAG_DOOR | VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "car4_dummy", FERRY_POS_CAR4, VEHICLE_FLAG_DOOR | VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation bmxIds[] = {
    { "chassis_dummy", BMX_CHASSIS, 0 },
    { "forks_front", BMX_FORKS_FRONT, 0 },
    { "forks_rear", BMX_FORKS_REAR, 0 },
    { "wheel_front", BMX_WHEEL_FRONT, 0 },
    { "wheel_rear", BMX_WHEEL_REAR, 0 },
    { "handlebars", BMX_HANDLEBARS, 0 },
    { "crank", BMX_CHAINSET, 0 },
    { "rightpedal", BMX_PEDAL_R, 0 },
    { "leftpedal", BMX_PEDAL_L, 0 },
    { "ped_frontseat", BMX_POS_FRONTSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_backseat", BMX_POS_BACKSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights", BMX_POS_HEADLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights", BMX_POS_TAILLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights2", BMX_POS_HEADLIGHTS_2, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights2", BMX_POS_TAILLIGHTS_2, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "exhaust", BMX_POS_EXHAUST, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "engine", BMX_POS_ENGINE, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "petrolcap", BMX_POS_PETROL_CAP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "hookup", BMX_POS_HOOKUP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "bargrip", BMX_POS_BARGRIP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_a", BMX_POS_MISCPOS_A, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_b", BMX_POS_MISCPOS_B, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "extra1", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra2", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra3", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra4", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra5", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra6", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation quadIds[] = {
    { "wheel_rf", QUAD_WHEEL_RF, VEHICLE_FLAG_RIGHT },
    { "wheel_rm", QUAD_WHEEL_RM, VEHICLE_FLAG_RIGHT },
    { "wheel_rb", QUAD_WHEEL_RB, VEHICLE_FLAG_RIGHT },
    { "wheel_lf", QUAD_WHEEL_LF, VEHICLE_FLAG_LEFT },
    { "wheel_lm", QUAD_WHEEL_LM, VEHICLE_FLAG_LEFT },
    { "wheel_lb", QUAD_WHEEL_LB, VEHICLE_FLAG_LEFT },
    { "door_rf_dummy", QUAD_DOOR_RF, VEHICLE_FLAG_FRONTDOOR | VEHICLE_FLAG_ANGLECULL | VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_rr_dummy", QUAD_DOOR_RR, VEHICLE_FLAG_REARDOOR | VEHICLE_FLAG_ANGLECULL | VEHICLE_FLAG_REAR | VEHICLE_FLAG_RIGHT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_lf_dummy", QUAD_DOOR_LF, VEHICLE_FLAG_FRONTDOOR | VEHICLE_FLAG_ANGLECULL | VEHICLE_FLAG_LEFT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "door_lr_dummy", QUAD_DOOR_LR, VEHICLE_FLAG_REARDOOR | VEHICLE_FLAG_ANGLECULL | VEHICLE_FLAG_REAR | VEHICLE_FLAG_LEFT | VEHICLE_FLAG_DOOR | VEHICLE_FLAG_COLLAPSE },
    { "chassis", QUAD_CHASSIS, 0 },
    { "body_front_dummy", QUAD_BODY_FRONT, VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "body_rear_dummy", QUAD_BODY_REAR, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "suspension_rf", QUAD_SUSPENSION_RF, VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "suspension_lf", QUAD_SUSPENSION_LF, VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "rear_axle", QUAD_REAR_AXLE, VEHICLE_FLAG_REAR | VEHICLE_FLAG_COLLAPSE },
    { "handlebars", QUAD_HANDLEBARS, VEHICLE_FLAG_FRONT | VEHICLE_FLAG_COLLAPSE },
    { "misc_a", QUAD_MISC_A, VEHICLE_FLAG_COLLAPSE },
    { "misc_b", QUAD_MISC_B, VEHICLE_FLAG_COLLAPSE },
    { "ped_frontseat", QUADBIKE_POS_FRONTSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_backseat", QUADBIKE_POS_BACKSEAT, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights", QUADBIKE_POS_HEADLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights", QUADBIKE_POS_TAILLIGHTS, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "headlights2", QUADBIKE_POS_HEADLIGHTS_2, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "taillights2", QUADBIKE_POS_TAILLIGHTS_2, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "exhaust", QUADBIKE_POS_EXHAUST, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "engine", QUADBIKE_POS_ENGINE, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "petrolcap", QUADBIKE_POS_PETROL_CAP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "hookup", QUADBIKE_POS_HOOKUP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "ped_arm", QUADBIKE_POS_BARGRIP, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_c", QUADBIKE_POS_MISCPOS_C, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_d", QUADBIKE_POS_MISCPOS_D, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_a", QUADBIKE_POS_MISCPOS_A, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "miscpos_b", QUADBIKE_POS_MISCPOS_B, VEHICLE_FLAG_POS | CLUMP_FLAG_NO_HIERID },
    { "extra1", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra2", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra3", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra4", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra5", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { "extra6", 0, VEHICLE_FLAG_DRAWLAST | VEHICLE_FLAG_COMP | CLUMP_FLAG_NO_HIERID },
    { nil, 0, 0 }
};

RwObjectNameIdAssocation* CVehicleModelInfo__ms_vehicleDescs[] = {
    carIds,
    boatIds,
    jetskiIds,
    trainIds,
    heliIds,
    planeIds,
    bikeIds,
    ferryIds,
    bmxIds,
    quadIds,
};




//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//------------------------------------MAIN---------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#define _PCSX2POINTER(p) (((uintptr_t)p) + 0x20000000)
#define _ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
template<typename T> T inline _EMUPOINTER(void* p) { return (T)(p ? _PCSX2POINTER(p) : NULL); }
template<typename T> T inline _EMUPOINTER(uintptr_t p) { return _EMUPOINTER<T>((void*)p); }

size_t countElements(const RwObjectNameIdAssocation* array)
{
    size_t count = 0;
    while (array && array[count].name != nullptr)
        count++;
    return count;
}

// Функция сравнения одного дескриптора с ожидаемым
bool checkNode(const char* descName, const RwObjectNameIdAssocation& expected, const RwObjectNameIdAssocation& actual)
{
    bool res = true;
    //// Сравнение имени
    if (
        (expected.name && !actual.name) ||
        (actual.name && !expected.name) ||
        ((expected.name && actual.name) &&
        (strcmp(expected.name, _EMUPOINTER<char*>((char*)actual.name)) != 0)
        ))
    {
        printf("[%s] Node \"%s\": name mismatch (in code here \"%s\", in memory \"%s\")\n",
            descName, _EMUPOINTER<char*>((char*)actual.name), expected.name, _EMUPOINTER<char*>((char*)actual.name));
        res = false;
    }
    // Сравнение hierId
    if (expected.hierId != actual.hierId)
    {
        printf("[%s] Node \"%s\": hierId mismatch (in code here %d, in memory %d)\n",
            descName, _EMUPOINTER<char*>((char*)actual.name), expected.hierId, actual.hierId);
        res = false;
    }
    // Сравнение флагов
    if (expected.flags != actual.flags)
    {
        printf("[%s] Node \"%s\": flags mismatch (in code here 0x%x, in memory 0x%x)\n",
            descName, _EMUPOINTER<char*>((char*)actual.name), expected.flags, actual.flags);
        res = false;
    }
    return res;
}

//// Основная функция проверки массивов из памяти.
//void checkVehicleDescs()
//{
//    // Массив строк для удобного вывода имени дескриптора
//    const char* descNames[] = { "carIds", "boatIds", "jetskiIds", "trainIds", "heliIds", "planeIds", "bikeIds", "ferryIds", "bmxIds", "quadIds" };
//
//    // Предполагается, что в памяти содержится массив указателей по адресу 0x489E38 (пример)
//    RwObjectNameIdAssocation** pVehicleDescs = (RwObjectNameIdAssocation**)_PCSX2POINTER(0x489E38);
//
//    for (size_t i = 0; i < _ARRAY_SIZE(descNames); i++)
//    {
//        // Получаем ожидаемый массив из статически определённых данных
//        RwObjectNameIdAssocation* expectedArray = CVehicleModelInfo__ms_vehicleDescs[i];
//        // Читаем соответствующий массив из памяти
//        RwObjectNameIdAssocation* actualArray = _EMUPOINTER<RwObjectNameIdAssocation*>(pVehicleDescs[i]);
//
//        // Подсчитываем количество элементов в ожидаемом и фактическом массиве
//        size_t expectedSize = countElements(expectedArray);
//        size_t actualSize = countElements(actualArray);
//
//        if (expectedSize != actualSize)
//        {
//            printf("[%s] Size mismatch: expected %zu elements, got %zu elements\n", descNames[i], expectedSize, actualSize);
//        }
//
//        // Индекс элемента
//        size_t index = 0;
//        while (index < expectedSize && index < actualSize)
//        {
//            checkNode(descNames[i], expectedArray[index], actualArray[index]);
//            index++;
//        }
//
//        // Если фактический массив длиннее, выводим предупреждение о лишних элементах
//        if (actualSize > expectedSize)
//        {
//            for (size_t j = expectedSize; j < actualSize; j++)
//            {
//                printf("[%s] Extra node \"%s\" found at index %zu (hierId: %d, flags: 0x%x)\n",
//                    descNames[i], actualArray[j].name, j, actualArray[j].hierId, actualArray[j].flags);
//            }
//        }
//        // Если ожидаемый массив длиннее, сообщаем о недостающих элементах
//        else if (expectedSize > actualSize)
//        {
//            for (size_t j = actualSize; j < expectedSize; j++)
//            {
//                printf("[%s] Missing node \"%s\" at index %zu (expected hierId: %d, expected flags: 0x%x)\n",
//                    descNames[i], expectedArray[j].name, j, expectedArray[j].hierId, expectedArray[j].flags);
//            }
//        }
//    }
//}

void checkVehicleDescs()
{
    // Массив строк для удобного вывода имени дескриптора
    const char* descNames[] = { "carIds", "boatIds", "jetskiIds", "trainIds", "heliIds", "planeIds", "bikeIds", "ferryIds", "bmxIds", "quadIds" };

    // Предполагается, что в памяти содержится массив указателей по адресу 0x489E38 (пример)
    RwObjectNameIdAssocation** pVehicleDescs = (RwObjectNameIdAssocation**)_PCSX2POINTER(0x489E38); // size 10
    //printf("0x%p \n", pVehicleDescs); return;
    bool res = true;

    for (int i = 0; i < _ARRAY_SIZE(descNames); i++)
    {
        // Получаем ожидаемый массив из статически определённых данных
        RwObjectNameIdAssocation* expectedArray = CVehicleModelInfo__ms_vehicleDescs[i];
        // Читаем соответствующий массив из памяти
        RwObjectNameIdAssocation* actualArray = _EMUPOINTER<RwObjectNameIdAssocation*>(pVehicleDescs[i]);

        // Индекс элемента
        size_t index = 0;
        while (actualArray[index].name != nullptr)
        {
            // Если actualArray пуст или достигнут конец – сообщаем об ошибке
            if (!actualArray || actualArray[index].name == nullptr)
            {
                printf("[%s] Node at index %zu is missing in memory data.\n", descNames[i], index);
                break;
            }
            // Сравниваем поля текущего элемента
            if (!checkNode(descNames[i], expectedArray[index], /**_EMUPOINTER<RwObjectNameIdAssocation*>*/(/*&*/actualArray[index]))) { res = false; }
            index++;
        }
       //printf("0x%p \n", actualArray);
    }
    if (res) {
        printf("OK CHECK!!!!\n");
    }
    else {
        printf("ERROR CHECK!!!!\n");
    }
}

void testhpp()
{
	printf("testhpp()\n");
    //-----------------------

    checkVehicleDescs();

    //RwObjectNameIdAssocation** CVehicleModelInfo_ms_vehicleDescs = (RwObjectNameIdAssocation**)_PCSX2POINTER(0x489E38); // size 10
    //const char* desc[] = { "carIds", "boatIds", "jetskiIds", "trainIds", "heliIds", "planeIds", "bikeIds", "ferryIds", "bmxIds", "quadIds" };
    //for (int i = 0; i < _ARRAY_SIZE(desc); i++)
    //{
    //    RwObjectNameIdAssocation* pIdAssoc = (_EMUPOINTER<RwObjectNameIdAssocation*>(CVehicleModelInfo_ms_vehicleDescs[i]));
    //    while (pIdAssoc && (*(uint32_t*)pIdAssoc))
    //    {
    //        printf("    { \"%s\", %d, 0x%x },\n", _EMUPOINTER<char*>((char*)pIdAssoc->name), pIdAssoc->hierId, pIdAssoc->flags);
    //        ++pIdAssoc;
    //    }
    //    ++pIdAssoc; // we null, go next
    //}
}