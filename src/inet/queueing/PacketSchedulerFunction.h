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

#ifndef __INET_PACKETSCHEDULERFUNCTION_H
#define __INET_PACKETSCHEDULERFUNCTION_H

#include "inet/queueing/contract/IPacketSchedulerFunction.h"

namespace inet {
namespace queueing {

typedef int (*PacketSchedulerFunction)(const std::vector<IPassivePacketSource *>& providers);

class INET_API CPacketSchedulerFunction : public cObject, public IPacketSchedulerFunction
{
  protected:
    PacketSchedulerFunction packetSchedulerFunction;

  public:
    CPacketSchedulerFunction(PacketSchedulerFunction packetSchedulerFunction) : packetSchedulerFunction(packetSchedulerFunction) { }

    virtual int schedulePacket(const std::vector<IPassivePacketSource *>& queues) const override { return packetSchedulerFunction(queues); }
};

#define Register_Packet_Scheduler_Function(name, function) \
    class INET_API name : public ::inet::queueing::CPacketSchedulerFunction { public: name() : CPacketSchedulerFunction(function) { } }; \
    Register_Class(name)

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_PACKETSCHEDULERFUNCTION_H

