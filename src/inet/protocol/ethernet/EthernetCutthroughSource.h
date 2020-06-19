//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_ETHERNETCUTTHROUGHSOURCE_H
#define __INET_ETHERNETCUTTHROUGHSOURCE_H

#include "inet/linklayer/ethernet/switch/IMacAddressTable.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/protocol/common/PacketDestreamer.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetCutthroughSource : public PacketDestreamer
{
  protected:
    cGate *cutthroughOutputGate = nullptr;
    IPassivePacketSink *cutthroughConsumer = nullptr;

    InterfaceEntry *interfaceEntry = nullptr;
    IMacAddressTable *macTable = nullptr;

    bool cutthroughInProgress = false;
    cMessage *cutthroughTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual ~EthernetCutthroughSource() { cancelAndDelete(cutthroughTimer); }

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate, bps datarate) override;
};

} // namespace inet

#endif // ifndef __INET_ETHERNETCUTTHROUGHSOURCE_H

