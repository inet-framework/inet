//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_CONNECTIONMANAGER_H
#define __INET_CONNECTIONMANAGER_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/physicallayer/common/packetlevel/Signal.h"

namespace inet {

class INET_API ConnectionManager : public cSimpleModule, public cListener
{
  protected:
    InterfaceEntry *interfaceEntry;
    cGate *physInGate = nullptr;    // pointer to the phys input gate
    cChannel *rxTransmissionChannel = nullptr;    // tx transmission channel
    cGate *physOutGate = nullptr;    // pointer to the output gate
    cChannel *txTransmissionChannel = nullptr;    // tx transmission channel
    double bitrate = NAN;
    bool connected = false;
    bool disabled = true;
    simtime_t txStartTime = -1;
    physicallayer::Signal *txSignal = nullptr;

  public:
    ~ConnectionManager();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void propagateDatarate();
    virtual void propagateStatus();
    virtual void propagatePreChannelOff();
    void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif // __INET_TXCONNECTIONMANAGER_H

