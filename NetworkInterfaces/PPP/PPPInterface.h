//
// Copyright (C) 2004 Andras Varga
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
//

#ifndef __PPPINTERFACE_H
#define __PPPINTERFACE_H


#include <omnetpp.h>
#include "PPPFrame_m.h"


/**
 * PPP implementation. Derived from the p-to-p OMNeT++ sample simulation.
 */
class PPPInterface : public cSimpleModule
{
  protected:
    bool connected;

    long frameCapacity;
    long bitCapacity;
    cGate *gateToWatch;

    cQueue queue;
    cMessage *endTransmissionEvent;

    InterfaceEntry *interfaceEntry;  // points into RoutingTable
    double datarate;  // only cached for the display string

    // statistics
    long numSent;
    long numRcvdOK;
    long numBitErr;
    long numDropped;

    InterfaceEntry *registerInterface(double datarate);
    void startTransmitting(cMessage *msg);
    PPPFrame *encapsulate(cMessage *msg);
    cMessage *decapsulate(PPPFrame *pppFrame);
    void displayBusy();
    void displayIdle();
    void updateDisplayString();

  public:
    Module_Class_Members(PPPInterface, cSimpleModule, 0);

    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif


