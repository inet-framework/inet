//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/routing/leach/LeachBS.h"
#include "inet/routing/leach/Leach.h"

using namespace std;

namespace inet {
namespace leach {

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
        host = getContainingNode(this);
        interfaceTable.reference(this, "interfaceTableModule", true);
        interfaces = par("interfaces");

        bsPktReceived = 0;

    } else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerService(Protocol::manet, nullptr, gate("ipIn"));
        registerProtocol(Protocol::manet, gate("ipOut"), nullptr);
    }
}

void LeachBS::start() {
    /* Search the 802154 interface */
    configureInterfaces();

}

void LeachBS::stop() {

}

void LeachBS::configureInterfaces() {
    cPatternMatcher interfaceMatcher(interfaces, false, true, false);
    int numInterfaces = interfaceTable->getNumInterfaces();
    for (int i = 0; i < numInterfaces; i++) {
        NetworkInterface *networkInterface = interfaceTable->getInterface(i);
        if (networkInterface->isMulticast()
                && interfaceMatcher.matches(
                        networkInterface->getInterfaceName())) {
            wirelessInterface = networkInterface;
        }
    }
}

void LeachBS::handleMessageWhenUp(cMessage *msg) {
    Ipv4Address nodeAddr =
            (wirelessInterface->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
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
            if (packetType == BS) {
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

} /* namespace leach */
} /* namespace inet */
