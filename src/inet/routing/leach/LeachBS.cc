//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <iostream>
#include <fstream>
#include <unordered_set>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/routing/leach/Leach.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "LeachBS.h"

using namespace std;

namespace inet {

Define_Module(LeachBS);

LeachBS::LeachBS() {

}

LeachBS::~LeachBS() {
    // TODO Auto-generated destructor stub
}

void LeachBS::initialize(int stage) {
    RoutingProtocolBase::initialize(stage);

    //reads from omnetpp.ini
    if (stage == INITSTAGE_LOCAL) {
        sequencenumber = 0;
        host = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"),
                this);

        bsPktReceived = 0;

    } else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerService(Protocol::manet, nullptr, gate("ipIn"));
        registerProtocol(Protocol::manet, gate("ipOut"), nullptr);
    }
}

void LeachBS::start() {
    /* Search the 80211 interface */
    int num_80211 = 0;
    NetworkInterface *ie;
    NetworkInterface *i_face;
    const char *name;

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        ie = ift->getInterface(i);
        name = ie->getInterfaceName();
        if (strstr(name, "wlan") != nullptr) {
            i_face = ie;
            num_80211++;
            interfaceId = i;
        }
    }

    // One enabled network interface (in total)
    if (num_80211 == 1)
        interface80211ptr = i_face;
    else
        throw cRuntimeError("DSDV has found %i 80211 interfaces", num_80211);

    CHK(interface80211ptr->getProtocolDataForUpdate<Ipv4InterfaceData>())->joinMulticastGroup(
            Ipv4Address::LL_MANET_ROUTERS);
}

void LeachBS::stop() {

}

void LeachBS::handleMessageWhenUp(cMessage *msg) {
    Ipv4Address nodeAddr =
            (interface80211ptr->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
    // if node is sending message
    if (msg->isSelfMessage()) {
        delete msg;
    }
    // if node is receiving message
    else if (check_and_cast<Packet*>(msg)->getTag<PacketProtocolTag>()->getProtocol()
            == &Protocol::manet) {
        auto receivedCtrlPkt =
                staticPtrCast<LeachControlPkt>(
                        check_and_cast<Packet*>(msg)->peekData<LeachControlPkt>()->dupShared());
        Packet *receivedPkt = check_and_cast<Packet*>(msg);
        auto &leachControlPkt = receivedPkt->popAtFront<LeachControlPkt>();

        auto packetType = leachControlPkt->getPacketType();

        // filter packet based on type and run specific functions
        if (msg->arrivedOn("ipIn")) {
            if (packetType == 5) {
                bsPktReceived += 1;
            } else {
                delete msg;
            }
        } else {
            throw cRuntimeError("Message arrived on unknown gate %s",
                    msg->getArrivalGate()->getName());
        }
    } else {
        throw cRuntimeError("Message not supported %s", msg->getName());
    }
}

} /* namespace inet */
