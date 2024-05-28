//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMETABLE_H_
#define INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMETABLE_H_

#include "inet/clock/contract/ClockTime.h"

namespace inet {

class INET_API GroupEligibilityTimeTable : public cSimpleModule
{
    protected:
        std::map<std::string, clocktime_t> groupEligibilityTimeTable;

    public:
        virtual ~GroupEligibilityTimeTable();

        /**
         * Updates groupEligibilityTime for the specified group, if newTime is more recent than the time in the table.
         * "Most recent value of the eligibilityTime variable from the previous frame" [Ieee802.1Qcr - 8.6.11.3.10]
         */
        virtual void updateGroupEligibilityTime(std::string group, clocktime_t newTime);

        /**
         * Returns the groupEligibilityTime for the corresponding group.
         */
        virtual clocktime_t getGroupEligibilityTime(std::string group);

    protected:
        virtual void initialize(int stage) override;


};

} //namespace inet


#endif /* INET_PROTOCOLELEMENT_SHAPER_GROUPELIGIBILITYTIMETABLE_H_ */
