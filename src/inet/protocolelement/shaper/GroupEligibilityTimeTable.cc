//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "GroupEligibilityTimeTable.h"

namespace inet {

Define_Module(GroupEligibilityTimeTable);


GroupEligibilityTimeTable::~GroupEligibilityTimeTable()
{
    groupEligibilityTimeTable = {};
}


void GroupEligibilityTimeTable::updateGroupEligibilityTime(std::string group, clocktime_t newTime)
{
    if (groupEligibilityTimeTable.find(group) == groupEligibilityTimeTable.end())
    {
        groupEligibilityTimeTable[group] = 0;
    }

    clocktime_t currentTime = groupEligibilityTimeTable[group];

    if (newTime > currentTime)
    {
        groupEligibilityTimeTable[group] = newTime;
    }

}


clocktime_t GroupEligibilityTimeTable::getGroupEligibilityTime(std::string group)
{
    if (groupEligibilityTimeTable.find(group) == groupEligibilityTimeTable.end())
    {
        groupEligibilityTimeTable[group] = 0;
    }

    return groupEligibilityTimeTable[group];
}


void GroupEligibilityTimeTable::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        WATCH_MAP(groupEligibilityTimeTable);
    }
}

} //namespace inet
