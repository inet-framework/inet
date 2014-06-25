/****************************************************************************/
/// @file    TraCIConstants.h
/// @author  Axel Wegener
/// @author  Friedemann Wesner
/// @author  Bjoern Hendriks
/// @author  Daniel Krajzewicz
/// @author  Thimor Bohn
/// @author  Tino Morenz
/// @author  Michael Behrisch
/// @author  Christoph Sommer
/// @date    2007/10/24
/// @version $Id$
///
/// holds codes used for TraCI
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo-sim.org/
// Copyright (C) 2001-2013 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef TRACICONSTANTS_H
#define TRACICONSTANTS_H

// ****************************************
// VERSION
// ****************************************
#define TRACI_VERSION 7

// ****************************************
// COMMANDS
// ****************************************
// command: get version
#define CMD_GETVERSION 0x00

// command: simulation step
#define CMD_SIMSTEP2 0x02

// command: stop node
#define CMD_STOP 0x12

// command: Resume from parking
#define CMD_RESUME 0x19

// command: set lane
#define CMD_CHANGELANE 0x13

// command: slow down
#define CMD_SLOWDOWN 0x14

// command: change target
#define CMD_CHANGETARGET 0x31

// command: close sumo
#define CMD_CLOSE 0x7F

// command: subscribe induction loop (e1) context
#define CMD_SUBSCRIBE_INDUCTIONLOOP_CONTEXT 0x80
// response: subscribe induction loop (e1) context
#define RESPONSE_SUBSCRIBE_INDUCTIONLOOP_CONTEXT 0x90
// command: get induction loop (e1) variable
#define CMD_GET_INDUCTIONLOOP_VARIABLE 0xa0
// response: get induction loop (e1) variable
#define RESPONSE_GET_INDUCTIONLOOP_VARIABLE 0xb0
// command: subscribe induction loop (e1) variable
#define CMD_SUBSCRIBE_INDUCTIONLOOP_VARIABLE 0xd0
// response: subscribe induction loop (e1) variable
#define RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE 0xe0

// command: subscribe areal detector (e2) context
#define CMD_SUBSCRIBE_AREAL_DETECTOR_CONTEXT 0x8D
// response: subscribe areal detector (e2) context
#define RESPONSE_SUBSCRIBE_AREAL_DETECTOR_CONTEXT 0x9D
// command: get areal detector (e2) variable
#define CMD_GET_AREAL_DETECTOR_VARIABLE 0x8E
// response: get areal detector (e3) variable
#define RESPONSE_GET_AREAL_DETECTOR_VARIABLE 0x9E
// command: subscribe areal detector (e2) variable
#define CMD_SUBSCRIBE_AREAL_DETECTOR_VARIABLE 0x8F
// response: subscribe areal detector (e2) variable
#define RESPONSE_SUBSCRIBE_AREAL_DETECTOR_VARIABLE 0x9F

// command: subscribe areal detector (e3) context
#define CMD_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_CONTEXT 0x81
// response: subscribe areal detector (e3) context
#define RESPONSE_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_CONTEXT 0x91
// command: get multi-entry/multi-exit detector (e3) variable
#define CMD_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE 0xa1
// response: get areal detector (e3) variable
#define RESPONSE_GET_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE 0xb1
// command: subscribe multi-entry/multi-exit detector (e3) variable
#define CMD_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE 0xd1
// response: subscribe areal detector (e3) variable
#define RESPONSE_SUBSCRIBE_MULTI_ENTRY_EXIT_DETECTOR_VARIABLE 0xe1

// command: subscribe traffic lights context
#define CMD_SUBSCRIBE_TL_CONTEXT 0x82
// response: subscribe traffic lights context
#define RESPONSE_SUBSCRIBE_TL_CONTEXT 0x92
// command: get traffic lights variable
#define CMD_GET_TL_VARIABLE 0xa2
// response: get traffic lights variable
#define RESPONSE_GET_TL_VARIABLE 0xb2
// command: set traffic lights variable
#define CMD_SET_TL_VARIABLE 0xc2
// command: subscribe traffic lights variable
#define CMD_SUBSCRIBE_TL_VARIABLE 0xd2
// response: subscribe traffic lights variable
#define RESPONSE_SUBSCRIBE_TL_VARIABLE 0xe2

// command: subscribe lane context
#define CMD_SUBSCRIBE_LANE_CONTEXT 0x83
// response: subscribe lane context
#define RESPONSE_SUBSCRIBE_LANE_CONTEXT 0x93
// command: get lane variable
#define CMD_GET_LANE_VARIABLE 0xa3
// response: get lane variable
#define RESPONSE_GET_LANE_VARIABLE 0xb3
// command: set lane variable
#define CMD_SET_LANE_VARIABLE 0xc3
// command: subscribe lane variable
#define CMD_SUBSCRIBE_LANE_VARIABLE 0xd3
// response: subscribe lane variable
#define RESPONSE_SUBSCRIBE_LANE_VARIABLE 0xe3

// command: subscribe vehicle context
#define CMD_SUBSCRIBE_VEHICLE_CONTEXT 0x84
// response: subscribe vehicle context
#define RESPONSE_SUBSCRIBE_VEHICLE_CONTEXT 0x94
// command: get vehicle variable
#define CMD_GET_VEHICLE_VARIABLE 0xa4
// response: get vehicle variable
#define RESPONSE_GET_VEHICLE_VARIABLE 0xb4
// command: set vehicle variable
#define CMD_SET_VEHICLE_VARIABLE 0xc4
// command: subscribe vehicle variable
#define CMD_SUBSCRIBE_VEHICLE_VARIABLE 0xd4
// response: subscribe vehicle variable
#define RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE 0xe4

// command: subscribe vehicle type context
#define CMD_SUBSCRIBE_VEHICLETYPE_CONTEXT 0x85
// response: subscribe vehicle type context
#define RESPONSE_SUBSCRIBE_VEHICLETYPE_CONTEXT 0x95
// command: get vehicle type variable
#define CMD_GET_VEHICLETYPE_VARIABLE 0xa5
// response: get vehicle type variable
#define RESPONSE_GET_VEHICLETYPE_VARIABLE 0xb5
// command: set vehicle type variable
#define CMD_SET_VEHICLETYPE_VARIABLE 0xc5
// command: subscribe vehicle type variable
#define CMD_SUBSCRIBE_VEHICLETYPE_VARIABLE 0xd5
// response: subscribe vehicle type variable
#define RESPONSE_SUBSCRIBE_VEHICLETYPE_VARIABLE 0xe5

// command: subscribe route context
#define CMD_SUBSCRIBE_ROUTE_CONTEXT 0x86
// response: subscribe route context
#define RESPONSE_SUBSCRIBE_ROUTE_CONTEXT 0x96
// command: get route variable
#define CMD_GET_ROUTE_VARIABLE 0xa6
// response: get route variable
#define RESPONSE_GET_ROUTE_VARIABLE 0xb6
// command: set route variable
#define CMD_SET_ROUTE_VARIABLE 0xc6
// command: subscribe route variable
#define CMD_SUBSCRIBE_ROUTE_VARIABLE 0xd6
// response: subscribe route variable
#define RESPONSE_SUBSCRIBE_ROUTE_VARIABLE 0xe6

// command: subscribe poi context
#define CMD_SUBSCRIBE_POI_CONTEXT 0x87
// response: subscribe poi context
#define RESPONSE_SUBSCRIBE_POI_CONTEXT 0x97
// command: get poi variable
#define CMD_GET_POI_VARIABLE 0xa7
// response: get poi variable
#define RESPONSE_GET_POI_VARIABLE 0xb7
// command: set poi variable
#define CMD_SET_POI_VARIABLE 0xc7
// command: subscribe poi variable
#define CMD_SUBSCRIBE_POI_VARIABLE 0xd7
// response: subscribe poi variable
#define RESPONSE_SUBSCRIBE_POI_VARIABLE 0xe7

// command: subscribe polygon context
#define CMD_SUBSCRIBE_POLYGON_CONTEXT 0x88
// response: subscribe polygon context
#define RESPONSE_SUBSCRIBE_POLYGON_CONTEXT 0x98
// command: get polygon variable
#define CMD_GET_POLYGON_VARIABLE 0xa8
// response: get polygon variable
#define RESPONSE_GET_POLYGON_VARIABLE 0xb8
// command: set polygon variable
#define CMD_SET_POLYGON_VARIABLE 0xc8
// command: subscribe polygon variable
#define CMD_SUBSCRIBE_POLYGON_VARIABLE 0xd8
// response: subscribe polygon variable
#define RESPONSE_SUBSCRIBE_POLYGON_VARIABLE 0xe8

// command: subscribe junction context
#define CMD_SUBSCRIBE_JUNCTION_CONTEXT 0x89
// response: subscribe junction context
#define RESPONSE_SUBSCRIBE_JUNCTION_CONTEXT 0x99
// command: get junction variable
#define CMD_GET_JUNCTION_VARIABLE 0xa9
// response: get junction variable
#define RESPONSE_GET_JUNCTION_VARIABLE 0xb9
// command: set junction variable
#define CMD_SET_JUNCTION_VARIABLE 0xc9
// command: subscribe junction variable
#define CMD_SUBSCRIBE_JUNCTION_VARIABLE 0xd9
// response: subscribe junction variable
#define RESPONSE_SUBSCRIBE_JUNCTION_VARIABLE 0xe9

// command: subscribe edge context
#define CMD_SUBSCRIBE_EDGE_CONTEXT 0x8a
// response: subscribe edge context
#define RESPONSE_SUBSCRIBE_EDGE_CONTEXT 0x9a
// command: get edge variable
#define CMD_GET_EDGE_VARIABLE 0xaa
// response: get edge variable
#define RESPONSE_GET_EDGE_VARIABLE 0xba
// command: set edge variable
#define CMD_SET_EDGE_VARIABLE 0xca
// command: subscribe edge variable
#define CMD_SUBSCRIBE_EDGE_VARIABLE 0xda
// response: subscribe edge variable
#define RESPONSE_SUBSCRIBE_EDGE_VARIABLE 0xea

// command: subscribe simulation context
#define CMD_SUBSCRIBE_SIM_CONTEXT 0x8b
// response: subscribe simulation context
#define RESPONSE_SUBSCRIBE_SIM_CONTEXT 0x9b
// command: get simulation variable
#define CMD_GET_SIM_VARIABLE 0xab
// response: get simulation variable
#define RESPONSE_GET_SIM_VARIABLE 0xbb
// command: set simulation variable
#define CMD_SET_SIM_VARIABLE 0xcb
// command: subscribe simulation variable
#define CMD_SUBSCRIBE_SIM_VARIABLE 0xdb
// response: subscribe simulation variable
#define RESPONSE_SUBSCRIBE_SIM_VARIABLE 0xeb

// command: subscribe GUI context
#define CMD_SUBSCRIBE_GUI_CONTEXT 0x8c
// response: subscribe GUI context
#define RESPONSE_SUBSCRIBE_GUI_CONTEXT 0x9c
// command: get GUI variable
#define CMD_GET_GUI_VARIABLE 0xac
// response: get GUI variable
#define RESPONSE_GET_GUI_VARIABLE 0xbc
// command: set GUI variable
#define CMD_SET_GUI_VARIABLE 0xcc
// command: subscribe GUI variable
#define CMD_SUBSCRIBE_GUI_VARIABLE 0xdc
// response: subscribe GUI variable
#define RESPONSE_SUBSCRIBE_GUI_VARIABLE 0xec

// ****************************************
// POSITION REPRESENTATIONS
// ****************************************
// Position in geo-coordinates
#define POSITION_LON_LAT 0x00
// 2D cartesian coordinates
#define POSITION_2D 0x01
// Position in geo-coordinates with altitude
#define POSITION_LON_LAT_ALT 0x02
// 3D cartesian coordinates
#define POSITION_3D 0x03
// Position on road map
#define POSITION_ROADMAP 0x04

// ****************************************
// DATA TYPES
// ****************************************
// Boundary Box (4 doubles)
#define TYPE_BOUNDINGBOX 0x05
// Polygon (2*n doubles)
#define TYPE_POLYGON 0x06
// unsigned byte
#define TYPE_UBYTE 0x07
// signed byte
#define TYPE_BYTE 0x08
// 32 bit signed integer
#define TYPE_INTEGER 0x09
// float
#define TYPE_FLOAT 0x0A
// double
#define TYPE_DOUBLE 0x0B
// 8 bit ASCII string
#define TYPE_STRING 0x0C
// list of traffic light phases
#define TYPE_TLPHASELIST 0x0D
// list of strings
#define TYPE_STRINGLIST 0x0E
// compound object
#define TYPE_COMPOUND 0x0F
// color (four ubytes)
#define TYPE_COLOR 0x11

// ****************************************
// RESULT TYPES
// ****************************************
// result type: Ok
#define RTYPE_OK 0x00
// result type: not implemented
#define RTYPE_NOTIMPLEMENTED 0x01
// result type: error
#define RTYPE_ERR 0xFF

// return value for invalid queries (especially vehicle is not on the road)
#define INVALID_DOUBLE_VALUE -1001.
// return value for invalid queries (especially vehicle is not on the road)
#define INVALID_INT_VALUE -1

// ****************************************
// TRAFFIC LIGHT PHASES
// ****************************************
// red phase
#define TLPHASE_RED 0x01
// yellow phase
#define TLPHASE_YELLOW 0x02
// green phase
#define TLPHASE_GREEN 0x03
// tl is blinking
#define TLPHASE_BLINKING 0x04
// tl is off and not blinking
#define TLPHASE_NOSIGNAL 0x05

// ****************************************
// DIFFERENT DISTANCE REQUESTS
// ****************************************
// air distance
#define REQUEST_AIRDIST 0x00
// driving distance
#define REQUEST_DRIVINGDIST 0x01

// ****************************************
// VEHICLE REMOVAL REASONS
// ****************************************
// vehicle started teleport
#define REMOVE_TELEPORT 0x00
// vehicle removed while parking
#define REMOVE_PARKING 0x01
// vehicle arrived
#define REMOVE_ARRIVED 0x02
// vehicle was vaporized
#define REMOVE_VAPORIZED 0x03
// vehicle finished route during teleport
#define REMOVE_TELEPORT_ARRIVED 0x04

// ****************************************
// VARIABLE TYPES (for CMD_GET_*_VARIABLE)
// ****************************************
// list of instances' ids (get: all)
#define ID_LIST 0x00

// count of instances (get: all)
#define ID_COUNT 0x01

// subscribe object variables (get: all)
#define OBJECT_VARIABLES_SUBSCRIPTION 0x02

// subscribe context variables (get: all)
#define SURROUNDING_VARIABLES_SUBSCRIPTION 0x03

// last step vehicle number (get: induction loops, multi-entry/multi-exit detector, lanes, edges)
#define LAST_STEP_VEHICLE_NUMBER 0x10

// last step vehicle number (get: induction loops, multi-entry/multi-exit detector, lanes, edges)
#define LAST_STEP_MEAN_SPEED 0x11

// last step vehicle number (get: induction loops, multi-entry/multi-exit detector, lanes, edges)
#define LAST_STEP_VEHICLE_ID_LIST 0x12

// last step occupancy (get: induction loops, lanes, edges)
#define LAST_STEP_OCCUPANCY 0x13

// last step vehicle halting number (get: multi-entry/multi-exit detector, lanes, edges)
#define LAST_STEP_VEHICLE_HALTING_NUMBER 0x14

// last step mean vehicle length (get: induction loops, lanes, edges)
#define LAST_STEP_LENGTH 0x15

// last step time since last detection (get: induction loops)
#define LAST_STEP_TIME_SINCE_DETECTION 0x16

// entry times
#define LAST_STEP_VEHICLE_DATA 0x17

// last step jam length in vehicles
#define JAM_LENGTH_VEHICLE 0x18

// last step jam length in meters
#define JAM_LENGTH_METERS 0x19

// traffic light states, encoded as rRgGyYoO tuple (get: traffic lights)
#define TL_RED_YELLOW_GREEN_STATE 0x20

// index of the phase (set: traffic lights)
#define TL_PHASE_INDEX 0x22

// traffic light program (set: traffic lights)
#define TL_PROGRAM 0x23

// phase duration (set: traffic lights)
#define TL_PHASE_DURATION 0x24

// controlled lanes (get: traffic lights)
#define TL_CONTROLLED_LANES 0x26

// controlled links (get: traffic lights)
#define TL_CONTROLLED_LINKS 0x27

// index of the current phase (get: traffic lights)
#define TL_CURRENT_PHASE 0x28

// name of the current program (get: traffic lights)
#define TL_CURRENT_PROGRAM 0x29

// controlled junctions (get: traffic lights)
#define TL_CONTROLLED_JUNCTIONS 0x2a

// complete definition (get: traffic lights)
#define TL_COMPLETE_DEFINITION_RYG 0x2b

// complete program (set: traffic lights)
#define TL_COMPLETE_PROGRAM_RYG 0x2c

// assumed time to next switch (get: traffic lights)
#define TL_NEXT_SWITCH 0x2d

// outgoing link number (get: lanes)
#define LANE_LINK_NUMBER 0x30

// id of parent edge (get: lanes)
#define LANE_EDGE_ID 0x31

// outgoing link definitions (get: lanes)
#define LANE_LINKS 0x33

// list of allowed vehicle classes (get&set: lanes)
#define LANE_ALLOWED 0x34

// list of not allowed vehicle classes (get&set: lanes)
#define LANE_DISALLOWED 0x35

// speed (get: vehicle)
#define VAR_SPEED 0x40

// maximum allowed/possible speed (get: vehicle types, lanes, set: edges, lanes)
#define VAR_MAXSPEED 0x41

// position (2D) (get: vehicle, poi, set: poi)
#define VAR_POSITION 0x42

// angle (get: vehicle)
#define VAR_ANGLE 0x43

// angle (get: vehicle types, lanes, set: lanes)
#define VAR_LENGTH 0x44

// color (get: vehicles, vehicle types, polygons, pois)
#define VAR_COLOR 0x45

// max. acceleration (get: vehicle types)
#define VAR_ACCEL 0x46

// max. deceleration (get: vehicle types)
#define VAR_DECEL 0x47

// driver reaction time (get: vehicle types)
#define VAR_TAU 0x48

// vehicle class (get: vehicle types)
#define VAR_VEHICLECLASS 0x49

// emission class (get: vehicle types)
#define VAR_EMISSIONCLASS 0x4a

// shape class (get: vehicle types)
#define VAR_SHAPECLASS 0x4b

// minimum gap (get: vehicle types)
#define VAR_MINGAP 0x4c

// width (get: vehicle types, lanes)
#define VAR_WIDTH 0x4d

// shape (get: polygons)
#define VAR_SHAPE 0x4e

// type id (get: vehicles, polygons, pois)
#define VAR_TYPE 0x4f

// road id (get: vehicles)
#define VAR_ROAD_ID 0x50

// lane id (get: vehicles)
#define VAR_LANE_ID 0x51

// lane index (get: vehicles)
#define VAR_LANE_INDEX 0x52

// route id (get & set: vehicles)
#define VAR_ROUTE_ID 0x53

// edges (get: routes)
#define VAR_EDGES 0x54

// filled? (get: polygons)
#define VAR_FILL 0x55

// position (1D along lane) (get: vehicle)
#define VAR_LANEPOSITION 0x56

// route (set: vehicles)
#define VAR_ROUTE 0x57

// travel time information (get&set: vehicle)
#define VAR_EDGE_TRAVELTIME 0x58

// effort information (get&set: vehicle)
#define VAR_EDGE_EFFORT 0x59

// last step travel time (get: edge, lane)
#define VAR_CURRENT_TRAVELTIME 0x5a

// signals state (get/set: vehicle)
#define VAR_SIGNALS 0x5b

// new lane/position along (set: vehicle)
#define VAR_MOVE_TO 0x5c

// driver imperfection (set: vehicle)
#define VAR_IMPERFECTION 0x5d

// speed factor (set: vehicle)
#define VAR_SPEED_FACTOR 0x5e

// speed deviation (set: vehicle)
#define VAR_SPEED_DEVIATION 0x5f

// speed without TraCI influence (get: vehicle)
#define VAR_SPEED_WITHOUT_TRACI 0xb1

// best lanes (get: vehicle)
#define VAR_BEST_LANES 0xb2

// how speed is set (set: vehicle)
#define VAR_SPEEDSETMODE 0xb3

// move vehicle, VTD version (set: vehicle)
#define VAR_MOVE_TO_VTD 0xb4

// is the vehicle stopped, and if so parked and/or triggered?
// value = stopped + 2 * parking + 4 * triggered
#define VAR_STOPSTATE 0xb5

// how lane changing is performed (set: vehicle)
#define VAR_LANECHANGE_MODE 0xb6

// maximum speed regarding max speed on the current lane and speed factor (get: vehicle)
#define VAR_ALLOWED_SPEED 0xb7

// current CO2 emission of a node (get: vehicle, lane, edge)
#define VAR_CO2EMISSION 0x60

// current CO emission of a node (get: vehicle, lane, edge)
#define VAR_COEMISSION 0x61

// current HC emission of a node (get: vehicle, lane, edge)
#define VAR_HCEMISSION 0x62

// current PMx emission of a node (get: vehicle, lane, edge)
#define VAR_PMXEMISSION 0x63

// current NOx emission of a node (get: vehicle, lane, edge)
#define VAR_NOXEMISSION 0x64

// current fuel consumption of a node (get: vehicle, lane, edge)
#define VAR_FUELCONSUMPTION 0x65

// current noise emission of a node (get: vehicle, lane, edge)
#define VAR_NOISEEMISSION 0x66

// current person number (get: vehicle)
#define VAR_PERSON_NUMBER 0x67

#define VAR_BUS_STOP_WAITING 0x67

//current waiting time (get: vehicle, lane)
#define VAR_WAITING_TIME 0x7a

// current time step (get: simulation)
#define VAR_TIME_STEP 0x70

// number of loaded vehicles (get: simulation)
#define VAR_LOADED_VEHICLES_NUMBER 0x71

// loaded vehicle ids (get: simulation)
#define VAR_LOADED_VEHICLES_IDS 0x72

// number of departed vehicle (get: simulation)
#define VAR_DEPARTED_VEHICLES_NUMBER 0x73

// departed vehicle ids (get: simulation)
#define VAR_DEPARTED_VEHICLES_IDS 0x74

// number of vehicles starting to teleport (get: simulation)
#define VAR_TELEPORT_STARTING_VEHICLES_NUMBER 0x75

// ids of vehicles starting to teleport (get: simulation)
#define VAR_TELEPORT_STARTING_VEHICLES_IDS 0x76

// number of vehicles ending to teleport (get: simulation)
#define VAR_TELEPORT_ENDING_VEHICLES_NUMBER 0x77

// ids of vehicles ending to teleport (get: simulation)
#define VAR_TELEPORT_ENDING_VEHICLES_IDS 0x78

// number of arrived vehicles (get: simulation)
#define VAR_ARRIVED_VEHICLES_NUMBER 0x79

// ids of arrived vehicles (get: simulation)
#define VAR_ARRIVED_VEHICLES_IDS 0x7a

// number of vehicles starting to park (get: simulation)
#define VAR_PARKING_STARTING_VEHICLES_NUMBER 0x6c

// ids of vehicles starting to park (get: simulation)
#define VAR_PARKING_STARTING_VEHICLES_IDS 0x6d

// number of vehicles ending to park (get: simulation)
#define VAR_PARKING_ENDING_VEHICLES_NUMBER 0x6e

// ids of vehicles ending to park (get: simulation)
#define VAR_PARKING_ENDING_VEHICLES_IDS 0x6f

// delta t (get: simulation)
#define VAR_DELTA_T 0x7b

// bounding box (get: simulation)
#define VAR_NET_BOUNDING_BOX 0x7c

// minimum number of expected vehicles (get: simulation)
#define VAR_MIN_EXPECTED_VEHICLES 0x7d

// number of vehicles starting to park (get: simulation)
#define VAR_STOP_STARTING_VEHICLES_NUMBER 0x68

// ids of vehicles starting to park (get: simulation)
#define VAR_STOP_STARTING_VEHICLES_IDS 0x69

// number of vehicles ending to park (get: simulation)
#define VAR_STOP_ENDING_VEHICLES_NUMBER 0x6a

// ids of vehicles ending to park (get: simulation)
#define VAR_STOP_ENDING_VEHICLES_IDS 0x6b

// number of vehicles starting to park (get: simulation)
#define VAR_PARKING_STARTING_VEHICLES_NUMBER 0x6c

// ids of vehicles starting to park (get: simulation)
#define VAR_PARKING_STARTING_VEHICLES_IDS 0x6d

// number of vehicles ending to park (get: simulation)
#define VAR_PARKING_ENDING_VEHICLES_NUMBER 0x6e

// ids of vehicles ending to park (get: simulation)
#define VAR_PARKING_ENDING_VEHICLES_IDS 0x6f

// clears the simulation of all not inserted vehicles (set: simulation)
#define CMD_CLEAR_PENDING_VEHICLES 0x94

// add an instance (poi, polygon, vehicle, route)
#define ADD 0x80

// remove an instance (poi, polygon)
#define REMOVE 0x81

// convert coordinates
#define POSITION_CONVERSION 0x82

// distance between points or vehicles
#define DISTANCE_REQUEST 0x83

//the current driving distance
#define VAR_DISTANCE 0x84

// force rerouting based on travel time (vehicles)
#define CMD_REROUTE_TRAVELTIME 0x90

// force rerouting based on effort (vehicles)
#define CMD_REROUTE_EFFORT 0x91

// validates current route (vehicles)
#define VAR_ROUTE_VALID 0x92

// zoom
#define VAR_VIEW_ZOOM 0xa0

// view position
#define VAR_VIEW_OFFSET 0xa1

// view schema
#define VAR_VIEW_SCHEMA 0xa2

// view by boundary
#define VAR_VIEW_BOUNDARY 0xa3

// screenshot
#define VAR_SCREENSHOT 0xa5

// track vehicle
#define VAR_TRACK_VEHICLE 0xa6

#endif
