//
// Copyright (C) 2015 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/INETUtils.h"
#include "inet/linklayer/tun/TunInterface.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

Define_Module(TunInterface);

simsignal_t TunInterface::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t TunInterface::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t TunInterface::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t TunInterface::packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");

void TunInterface::initialize(int stage)
{
    MACBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        registerInterface();
}

InterfaceEntry *TunInterface::createInterfaceEntry()
{
    return new InterfaceEntry(this);
}

void TunInterface::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGate()->isName("appIn")) {
        emit(packetSentToUpperSignal, msg);
        send(msg, "upperLayerOut");
    }
    else if (msg->getArrivalGate()->isName("upperLayerIn")) {
        emit(packetReceivedFromUpperSignal, msg);
        send(msg, "appOut");
    }
    if (hasGUI())
        updateDisplayString();
}

void TunInterface::updateDisplayString()
{
    if (!hasGUI())
        return;
}

void TunInterface::finish()
{
}

void TunInterface::flushQueue()
{
    // does not have a queue, do nothing
}

void TunInterface::clearQueue()
{
    // does not have a queue, do nothing
}

} // namespace inet

