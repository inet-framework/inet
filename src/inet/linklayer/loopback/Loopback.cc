//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Author: Zoltan Bojthe
//

#include <stdio.h>
#include <string.h>

#include "inet/linklayer/loopback/Loopback.h"

#include "inet/common/INETUtils.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/NotifierConsts.h"

namespace inet {

Define_Module(Loopback);

simsignal_t Loopback::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t Loopback::packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");

Loopback::~Loopback()
{
}

void Loopback::initialize(int stage)
{
    MACBase::initialize(stage);

    // all initialization is done in the first stage
    if (stage == INITSTAGE_LOCAL) {
        numSent = numRcvdOK = 0;
        WATCH(numSent);
        WATCH(numRcvdOK);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        // register our interface entry in IInterfaceTable
        registerInterface();
    }
    // update display string when addresses have been autoconfigured etc.
    else if (stage == INITSTAGE_LAST) {
        updateDisplayString();
    }
}

InterfaceEntry *Loopback::createInterfaceEntry()
{
    InterfaceEntry *ie = new InterfaceEntry(this);

//    // generate a link-layer address to be used as interface token for IPv6
//    InterfaceToken token(0, getSimulation()->getUniqueNumber(), 64);
//    ie->setInterfaceToken(token);

    // capabilities
    ie->setMtu(par("mtu").longValue());
    ie->setLoopback(true);

    return ie;
}

void Loopback::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        handleMessageWhenDown(msg);
        return;
    }

    emit(packetReceivedFromUpperSignal, msg);
    EV << "Received " << msg << " for transmission\n";
    ASSERT(PK(msg)->hasBitError() == false);

    // pass up payload
    numRcvdOK++;
    emit(packetSentToUpperSignal, msg);
    numSent++;
    send(msg, "netwOut");

    if (hasGUI())
        updateDisplayString();
}

void Loopback::flushQueue()
{
    // do nothing, lo interface doesn't have any queue
}

void Loopback::clearQueue()
{
    // do nothing, lo interface doesn't have any queue
}

bool Loopback::isUpperMsg(cMessage *msg)
{
    return true;
}

void Loopback::updateDisplayString()
{
    /* TBD find solution for displaying IPv4 address without dependence on IPv4 or IPv6
            IPv4Address addr = interfaceEntry->ipv4Data()->getIPAddress();
            sprintf(buf, "%s / %s\nrcv:%ld snt:%ld", addr.isUnspecified()?"-":addr.str().c_str(), datarateText, numRcvdOK, numSent);
     */
    char buf[80];
    sprintf(buf, "rcv:%ld snt:%ld", numRcvdOK, numSent);

    getDisplayString().setTagArg("t", 0, buf);
}

} // namespace inet
