//
// Copyright (C) 2004 Andras Varga
// Copyrigth (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// This file is based on the PPP.h of INET written by Andras Varga.

#ifndef __EXTINTERFACE_H
#define __EXTINTERFACE_H


#ifndef MAX_MTU_SIZE
#define MAX_MTU_SIZE 4000
#endif

#include <omnetpp.h>
#include "ExtFrame_m.h"
#include "cSocketRTScheduler.h"
#include "IPDatagram.h"

#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP 132
#endif


class ExtInterface : public cSimpleModule
{
  protected:
    bool connected;
    uint8 buffer[1<<16];
    const char *device;

    InterfaceEntry *interfaceEntry;  // points into RoutingTable

    // statistics
    int numSent;
    int numRcvd;
    int numDropped;

    // access to real network interface via Scheduler class:
    cSocketRTScheduler *rtScheduler;

    InterfaceEntry *registerInterface();
    void displayBusy();
    void displayIdle();
    void updateDisplayString();

  private:
    const char *tag_color;
    const char *tag_width;

  public:
   // Module_Class_Members(ExtInterface, cSimpleModule, 0);

    virtual int32 numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void finish();
};

#endif


