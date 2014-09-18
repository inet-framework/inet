//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Author: Zoltan Bojthe
//

#ifndef __INET_LOOPBACK_H
#define __INET_LOOPBACK_H

#include "inet/common/INETDefs.h"

#include "inet/linklayer/common/MACBase.h"
#include "inet/linklayer/common/TxNotifDetails.h"

namespace inet {

class InterfaceEntry;
class IPassiveQueue;

/**
 * Loopback interface implementation.
 */
class INET_API Loopback : public MACBase
{
  protected:
    // statistics
    long numSent;
    long numRcvdOK;

    static simsignal_t packetSentToUpperSignal;
    static simsignal_t packetReceivedFromUpperSignal;

  protected:
    virtual InterfaceEntry *createInterfaceEntry();
    virtual void flushQueue();
    virtual void clearQueue();
    virtual bool isUpperMsg(cMessage *msg);

  public:
    Loopback();
    virtual ~Loopback();

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void updateDisplayString();
};

} // namespace inet

#endif // ifndef __INET_LOOPBACK_H

