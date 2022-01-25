//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#ifndef __INET_PACKETMETERBASE_H
#define __INET_PACKETMETERBASE_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/queueing/contract/IPacketMeter.h"

namespace inet {
namespace queueing {

class INET_API PacketMeterBase : public PacketFlowBase, public TransparentProtocolRegistrationListener, public virtual IPacketMeter
{
  protected:
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

