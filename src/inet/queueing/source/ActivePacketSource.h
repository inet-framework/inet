//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_ACTIVEPACKETSOURCE_H
#define __INET_ACTIVEPACKETSOURCE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/ActivePacketSourceBase.h"

namespace inet {
namespace queueing {

class INET_API ActivePacketSource : public ClockUserModuleMixin<ActivePacketSourceBase>
{
  protected:
    cPar *productionIntervalParameter = nullptr;
    ClockEvent *productionTimer = nullptr;
    bool scheduleForAbsoluteTime = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleProductionTimer(double delay);
    virtual void producePacket();

  public:
    virtual ~ActivePacketSource() { cancelAndDeleteClockEvent(productionTimer); }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

