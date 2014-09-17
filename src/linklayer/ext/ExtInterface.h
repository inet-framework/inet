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

#ifndef __INET_EXTINTERFACE_H
#define __INET_EXTINTERFACE_H

#ifndef MAX_MTU_SIZE
#define MAX_MTU_SIZE    4000
#endif // ifndef MAX_MTU_SIZE

#include "inet/common/INETDefs.h"

#include "inet/linklayer/common/MACBase.h"
#include "inet/linklayer/ext/ExtFrame_m.h"
#include "inet/linklayer/ext/cSocketRTScheduler.h"

namespace inet {

// Forward declarations:
class InterfaceEntry;

/**
 * Implements an interface that corresponds to a real interface
 * on the host running the simulation. Suitable for hardware-in-the-loop
 * simulations.
 *
 * Requires cSocketRTScheduler to be configured as scheduler in omnetpp.ini.
 *
 * See NED file for more details.
 */
class ExtInterface : public MACBase
{
  protected:
    bool connected;
    uint8 buffer[1 << 16];
    const char *device;

    // statistics
    int numSent;
    int numRcvd;
    int numDropped;

    // access to real network interface via Scheduler class:
    cSocketRTScheduler *rtScheduler;

  protected:
    void displayBusy();
    void displayIdle();
    void updateDisplayString();

    // MACBase functions
    InterfaceEntry *createInterfaceEntry();
    virtual void flushQueue();
    virtual void clearQueue();
    virtual bool isUpperMsg(cMessage *msg) { return msg->arrivedOn("upperLayerIn"); }

  private:
    const char *tag_color;
    const char *tag_width;

  public:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void finish();
};

} // namespace inet

#endif // ifndef __INET_EXTINTERFACE_H

