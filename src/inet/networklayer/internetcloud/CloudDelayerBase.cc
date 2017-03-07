//
// Copyright (C) 2012 OpenSim Ltd
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
// @author Zoltan Bojthe
//

#include "inet/networklayer/internetcloud/CloudDelayerBase.h"

#include "inet/networklayer/ipv4/IPv4.h"

namespace inet {

Define_Module(CloudDelayerBase);

#define SRCPAR    "incomingInterfaceID"

CloudDelayerBase::CloudDelayerBase()
{
    networkProtocol = nullptr;
}

CloudDelayerBase::~CloudDelayerBase()
{
}

void CloudDelayerBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        networkProtocol->registerHook(0, this);
    }
}

void CloudDelayerBase::finish()
{
    if (isRegisteredHook())
        networkProtocol->unregisterHook(this);
}

void CloudDelayerBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        Packet *context = (Packet *)msg->getContextPointer();
        delete msg;
        networkProtocol->reinjectQueuedDatagram(context);
    }
}

void CloudDelayerBase::calculateDropAndDelay(const cMessage *msg, int srcID, int destID, bool& outDrop, simtime_t& outDelay)
{
    outDrop = false;
    outDelay = SIMTIME_ZERO;
}

INetfilter::IHook::Result CloudDelayerBase::datagramPreRoutingHook(Packet *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result CloudDelayerBase::datagramForwardHook(Packet *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress)
{
    Enter_Method_Silent();

    int srcID = inputInterfaceEntry ? inputInterfaceEntry->getInterfaceId() : -1;
    int destID = outputInterfaceEntry->getInterfaceId();

    simtime_t propDelay;
    bool isDrop;
    calculateDropAndDelay(datagram, srcID, destID, isDrop, propDelay);
    if (isDrop) {
        //TODO emit?
        EV_INFO << "Message " << datagram->STR_COMPAT() << " dropped in cloud.\n";
        return INetfilter::IHook::DROP;
    }

    if (propDelay > SIMTIME_ZERO) {
        //TODO emit?
        EV_INFO << "Message " << datagram->STR_COMPAT() << " delayed with " << propDelay * 1000.0 << "ms in cloud.\n";
        cMessage *selfmsg = new cMessage("Delay");
        selfmsg->setContextPointer(datagram);
        scheduleAt(simTime() + propDelay, selfmsg);
        return INetfilter::IHook::QUEUE;
    }
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result CloudDelayerBase::datagramPostRoutingHook(Packet *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result CloudDelayerBase::datagramLocalInHook(Packet *datagram, const InterfaceEntry *inputInterfaceEntry)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result CloudDelayerBase::datagramLocalOutHook(Packet *datagram, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHopAddress)
{
    return INetfilter::IHook::ACCEPT;
}

} // namespace inet

