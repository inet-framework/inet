/***************************************************************************
                          Router_Deq_Hook_Trigger.h  -  description
                             -------------------
    begin                : Mon Aug 28 2000
    copyright            : (C) 2000 by Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/


#ifndef ROUTER_DEQ_HOOK_TRIGGER_H
#define ROUTER_DEQ_HOOK_TRIGGER_H

#include "omnetpp.h"
#include "simulatedINTERNAL.h"

/**sends REQUEST_PACKETS at specified rate
  *@author Dirk Holzhausen
  */

class Router_Deq_Hook_Trigger : public cSimpleModule  {

	

	public:
		Module_Class_Members ( Router_Deq_Hook_Trigger, cSimpleModule, ACTIVITY_STACK_SIZE )

		virtual void activity();

		
};

#endif
