//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211TESTERMAC_H
#define __INET_IEEE80211TESTERMAC_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"

namespace inet {

using namespace ieee80211;
using namespace physicallayer;

class INET_API Ieee80211TesterMac : public Ieee80211Mac
{
    protected:
        int msgCounter = 0;
        const char *actions = nullptr;

    protected:
        virtual void handleLowerPacket(Packet *packet) override;
};

} // namespace inet

#endif

