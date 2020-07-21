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

#ifndef __INET_PCAPFILEPACKETPRODUCER_H
#define __INET_PCAPFILEPACKETPRODUCER_H

#include "inet/common/packet/recorder/PcapReader.h"
#include "inet/queueing/base/ActivePacketSourceBase.h"

namespace inet {
namespace queueing {

class INET_API PcapFilePacketProducer : public ActivePacketSourceBase
{
  protected:
    PcapReader pcapReader;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *message) override;

    virtual void schedulePacket();

  public:
    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override { }
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PCAPFILEPACKETPRODUCER_H

