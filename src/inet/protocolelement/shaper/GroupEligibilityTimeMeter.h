//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMEMETER_H_
#define INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMEMETER_H_

#include "inet/protocolelement/shaper/EligibilityTimeMeter.h"
#include "inet/common/ModuleRefByPar.h"
#include "GroupEligibilityTimeTable.h"

namespace inet {

class GroupEligibilityTimeMeter : public EligibilityTimeMeter
{
    protected:
        ModuleRefByPar<GroupEligibilityTimeTable> groupEligibilityTimeTable;

    protected:
        virtual void initialize(int stage) override;
        virtual void meterPacket(Packet *packet) override;

};

} // namespace inet

#endif /* INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMEMETER_H_ */
