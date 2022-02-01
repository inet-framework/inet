//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETSCHEDULERFUNCTION_H
#define __INET_PACKETSCHEDULERFUNCTION_H

#include "inet/queueing/contract/IPacketSchedulerFunction.h"

namespace inet {
namespace queueing {

typedef int (*PacketSchedulerFunction)(const std::vector<IPassivePacketSource *>& sources);

class INET_API CPacketSchedulerFunction : public cObject, public virtual IPacketSchedulerFunction
{
  protected:
    PacketSchedulerFunction packetSchedulerFunction;

  public:
    CPacketSchedulerFunction(PacketSchedulerFunction packetSchedulerFunction) : packetSchedulerFunction(packetSchedulerFunction) {}

    virtual int schedulePacket(const std::vector<IPassivePacketSource *>& sources) const override { return packetSchedulerFunction(sources); }
};

#define Register_Packet_Scheduler_Function(name, function) \
    class INET_API name : public ::inet::queueing::CPacketSchedulerFunction { \
      public: name() : CPacketSchedulerFunction(function) {} \
    }; \
    Register_Class(name)

} // namespace queueing
} // namespace inet

#endif

