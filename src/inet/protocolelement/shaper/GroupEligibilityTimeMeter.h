//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMEMETER_H_
#define INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMEMETER_H_

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/common/VlanTagType.h"
#include "inet/protocolelement/shaper/EligibilityTimeMeter.h"
#include "GroupEligibilityTimeTable.h"

namespace inet {

class INET_API GroupEligibilityTimeMeter : public EligibilityTimeMeter
{
    protected:
        VlanTagType vlanTagType = VLAN_TAG_TYPE_C;
        ModuleRefByPar<GroupEligibilityTimeTable> groupEligibilityTimeTable;

    protected:
        virtual void initialize(int stage) override;
        virtual void meterPacket(Packet *packet) override;

};

} // namespace inet

#endif /* INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMEMETER_H_ */
