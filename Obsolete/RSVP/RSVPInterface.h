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
#ifndef __RSVP_INTERFACE_H__
#define __RSVP_INTERFACE_H__

#include <omnetpp.h>

const char *MY_ERROR_IP_ADDRESS = "10.0.0.255";


class RSVPInterface : public cSimpleModule
{
  private:
    void processMsgFromIp(cMessage *);
    void processMsgFromApp(cMessage *);
  public:
    Module_Class_Members(RSVPInterface, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif

