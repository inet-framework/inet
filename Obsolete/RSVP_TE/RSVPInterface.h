/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/


/*
*    File Name RSVPInterface.h
*    RSVP-TE library
*    This file defines RSVPInterface class
**/

#ifndef __RSVP_INTERFACE_H__
#define __RSVP_INTERFACE_H__

#include <omnetpp.h>
#include "basic_consts.h"

const char *MY_ERROR_IP_ADDRESS = "10.0.0.255";


class RSVPInterface : public cSimpleModule   // was ProcessorAccess
{
private:


    void processMsgFromIp(cMessage *);
    void processMsgFromApp(cMessage *);
public:
    Module_Class_Members(RSVPInterface, cSimpleModule,
                ACTIVITY_STACK_SIZE);

    virtual void initialize();
    virtual void activity();

};

#endif

