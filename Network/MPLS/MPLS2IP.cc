/*******************************************************************
*
*   This library is free software, you can redistribute it
*   and/or modify
*   it under  the terms of the GNU Lesser General Public License
*   as published by the Free Software Foundation;
*   either version 2 of the License, or any later version.
*   The library is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*   See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#include <omnetpp.h>
#include <string.h>

#include "MPLS2IP.h"
#include "hook_types.h"  // for NWI_IDLE

Define_Module_Like(MPLS2IP, NetworkInterface);


void MPLS2IP::endService(cMessage *msg)
{
    if (!strcmp(msg->arrivalGate()->name(), "ipOutputQueueIn"))
    {
        cMessage *nwiIdleMsg = new cMessage();
        nwiIdleMsg->setKind(NWI_IDLE);

        send(msg, "physicalOut");
        send(nwiIdleMsg, "ipOutputQueueOut");  // FIXME is this OK?
    }
    else
    {
        send(msg, "ipInputQueueOut");
    }

}
