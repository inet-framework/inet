//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_SIMPLEGATESCHEDULINGCONFIGURATOR_H
#define __INET_SIMPLEGATESCHEDULINGCONFIGURATOR_H

#include <algorithm>
#include <vector>

#include "inet/common/PatternMatcher.h"
#include "inet/common/Topology.h"
#include "inet/linklayer/configurator/gatescheduling/base/GateSchedulingConfiguratorBase.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API SimpleGateSchedulingConfigurator : public GateSchedulingConfiguratorBase
{
  protected:
    class Slot {
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

