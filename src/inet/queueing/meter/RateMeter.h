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

#ifndef __INET_RATEMETER_H
#define __INET_RATEMETER_H

#include "inet/queueing/base/PacketMeterBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API RateMeter : public PacketMeterBase
{
  protected:
    double alpha = NaN;
    simtime_t lastUpdate = 0;
    int currentNumPackets = 0;
    b currentTotalPacketLength = b(0);
    bps datarate = bps(0);
    double packetrate = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void meterPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_RATEMETER_H

