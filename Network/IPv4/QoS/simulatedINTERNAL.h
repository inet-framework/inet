// -*- C++ -*-
// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

/***************************************************************************
                          simulatedINTERNAL.h  -  description
                             -------------------
    begin                : Fri Sep 1 2000
    author               : Dirk Holzhausen
    email                : dholzh@gmx.de


		Header file for all modules and components in Simulated INTERNAL

 ***************************************************************************/


#ifndef __SIMULATED_INTERNAL_H__
#define __SIMULATED_INTERNAL_H__

#include "basic_consts.h"
#include "any_queue.h"


// switches...

#define EVAL
#define IP_SIM


// global functions...

bool isINTERNALDataPacket ( cMessage *msg );
ANY_Queue *findINTERNALQueue ( cModule *startMod, const char *searchStr );
cModule *findINTERNALModule ( cModule *startMod, const char *modName );
cMessage *createINTERNALMessage ( int kind );
void deleteINTERNALMessage ( cMessage *msg );



// constants...

// Router stuff etc.


const char PAR_ROUTER_RATE []   = "rate";


// Queues/Schedulers

const char PAR_QUEUE_NAME []    = "queue_name";
const char PAR_QUEUE_POINTER [] = "queue_pointer";
const char PAR_PARENTS_GOUP	[]  = "queue_parents_up";

const char PAR_QUEUE_MAX_LEN [] = "max_len";

const char  PAR_GATE_SIZE [] 		= "gate_size";



// NED parameters for RIO

//const char PAR_RIO_ENQUEUE []   = "rio";


// Weighted Round Robin
/*
const char PAR_WRR_WORK_CONSERVING [] = "work_conserving";
const char PAR_WRR_WEIGHT_BY_BYTE  [] = "weight_by_byte";
const char PAR_WRR_WEIGHT []          = "weight";


const char PAR_WFQ_GEF_WEIHGT_EF []	  = "weight_ef_gef";
const char PAR_WFQ_GEF_WEIHGT_AF []	  = "weight_af";

const char PAR_WFQ_PQ_GEF_GATE []     = "ef_gate_no";
*/

// Classifier/Profiles
/*
const char PAR_CLASSIFIER_FILE []     = "classifier_file";
const char PAR_CLASSIFIER_ID []       = "classifier_id";
*/



// Marker
/*
const char PAR_MARKER_CP [] = "codepoint";
*/
// Traffic Shaping
/*
const char PAR_TS_RATE []					 = "rate";
*/

// Leaky Bucket
/*
const char PAR_LB_RATE []          = "rate";
const char PAR_LB_BUCKET_SIZE []   = "bucket_size";
const char PAR_LB_DEQ_HOOK_NAME [] = "deq_hook_name";

const int  LB_PARENTS_UP = 2;			 							// dequeue hook search for send direct gate
const char LB_DIRECT_GATE []  = "inDirekt";			// name of send direct gate for wakeup messages
*/

// Token Bucket
/*
const char PAR_TB_RATE [] 		   = "rate";
const char PAR_TB_BUCKET_SIZE [] = "bucket_size";

const char PAR_TB_WQ []					 = "wq";
*/
// RED
/*
const char PAR_RED_WQ [] 			= "wq";
const char PAR_RED_MAX_P [] 	= "max_p";
const char PAR_RED_MIN_TH [] 	= "min_th";
const char PAR_RED_MAX_TH [] 	= "max_th";
*/

// RIO
/*
const int  RIO_IN_PCK  = 512;										// used in RIO Queue
const int  RIO_OUT_PCK = 514;
*/

// EFMD
/*
const char PAR_EFMD_QUEUE_MAX_LEN []  = "max_len";
const char PAR_EFMD_CREDIT []         = "credit";
const char PAR_EFMD_SENDTIME []       = "send_time";
const char PAR_EFMD_ARR_TIME []       = "arr_time";
const char PAR_EFMD_DEFAULT_CREDIT [] = "default_credit";
const char PAR_EFMD_WEIGHT []         = "weight";
*/


// IP Fields...

const char PAR_IP_PROTOCOL []  = "protocol";
const char PAR_IP_CODEPOINT [] = "codepoint";
const char PAR_IP_SRC_ADDR []  = "src_addr";
const char PAR_IP_DEST_ADDR [] = "dest_addr";
const char PAR_IP_SRC_PORT []  = "src_port";
const char PAR_IP_DEST_PORT [] = "dest_port";

const char PAR_IP_IN_ADAPTER [] = "in_adapter";

// general



const int   MIN_FRAG_SIZE_BITS = 576 * 8;				// minimum fragmentation size

#endif
