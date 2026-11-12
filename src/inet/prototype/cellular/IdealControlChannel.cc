//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/IdealControlChannel.h"

namespace inet {

namespace cellular {

Define_Module(IdealControlChannel);

void IdealControlChannel::handleMessage(cMessage *message)
{
    throw cRuntimeError("IdealControlChannel takes no messages, only method calls");
}

int IdealControlChannel::registerGnbMac(GnbMac *gnbMac)
{
    Enter_Method("registerGnbMac");
    gnbMacs.push_back(gnbMac);
    return gnbMacs.size() - 1;
}

int IdealControlChannel::registerUeMac(UeMac *ueMac)
{
    Enter_Method("registerUeMac");
    ueMacs.push_back(ueMac);
    return ueMacs.size() - 1;
}

} // namespace cellular

} // namespace inet

