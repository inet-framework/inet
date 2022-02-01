//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "Ieee80211TesterMac.h"
//#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"

namespace inet {

Define_Module(Ieee80211TesterMac);

void Ieee80211TesterMac::handleLowerPacket(Packet *packet)
{
    actions = par("actions");
    int len = strlen(actions);
    if (msgCounter >= len)
        throw cRuntimeError("No action is defined for this msg %s", packet->getName());
    if (actions[msgCounter] == 'A') {
        if (rx->lowerFrameReceived(packet)) {
            auto header = packet->peekAtFront<Ieee80211MacHeader>();
            processLowerFrame(packet, header);
        }
        else { // corrupted frame received
            if (mib->qos)
                hcf->corruptedFrameReceived();
            else
                dcf->corruptedFrameReceived();
        }
    }
    else if (actions[msgCounter] == 'B')
        delete packet; // block
    else
        throw cRuntimeError("Unknown action = %c", actions[msgCounter]);
    msgCounter++;
}

} // namespace inet
