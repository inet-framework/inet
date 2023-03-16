//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EAGERGATESCHEDULECONFIGURATOR_H
#define __INET_EAGERGATESCHEDULECONFIGURATOR_H

#include <algorithm>
#include <vector>

#include "inet/common/PatternMatcher.h"
#include "inet/common/Topology.h"
#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API EagerGateScheduleConfigurator : public GateScheduleConfiguratorBase
{
  protected:
    class INET_API Slot {
      public:
        int gateOpenIndex = -1;
        simtime_t gateOpenTime = -1;
        simtime_t gateCloseTime = -1;
    };

  protected:
    virtual Output *computeGateScheduling(const Input& input) const override;

    virtual simtime_t computeStreamStartOffset(Input::Flow& flow, std::map<NetworkInterface *, std::vector<Slot>>& interfaceSchedules) const;
    virtual simtime_t computeStartOffsetForPathFragments(Input::Flow& flow, std::string startNetworkNodeName, simtime_t startTime, std::map<NetworkInterface *, std::vector<Slot>>& interfaceSchedules) const;

    virtual void addGateScheduling(Input::Flow& flow, int startIndex, int endIndex, simtime_t startOffset, std::map<NetworkInterface *, std::vector<Slot>>& interfaceSchedules) const;
    virtual void addGateSchedulingForPathFragments(Input::Flow& flow, std::string startNetworkNodeName, simtime_t startTime, int index, std::map<NetworkInterface *, std::vector<Slot>>& interfaceSchedules) const;
};

} // namespace inet

#endif

