//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "inet/routing/eigrp/EigrpSplitter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/Protocol.h"

namespace inet {
namespace eigrp {

Define_Module(EigrpSplitter);

EigrpSplitter::EigrpSplitter() {
}

EigrpSplitter::~EigrpSplitter() {
}

void EigrpSplitter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerService(Protocol::eigrp, gate("splitterIn"), gate("splitterOut"));
        registerService(Protocol::eigrp, gate("splitter6In"), gate("splitter6Out"));
        registerProtocol(Protocol::eigrp, gate("ipOut"), gate("ipIn"));
    }
}

void EigrpSplitter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        EV_DEBUG << "Self message received by EigrpSplitter" << endl;
        delete msg;
    }
    else {
        if (strcmp(msg->getArrivalGate()->getBaseName(), "splitterIn") == 0 || strcmp(msg->getArrivalGate()->getBaseName(), "splitter6In") == 0) {
            this->send(msg, "ipOut"); // A message from ipv6pdm or ipv4pdm to outside
        }
        else if (strcmp(msg->getArrivalGate()->getBaseName(), "ipIn") == 0) {

            Packet *packet = check_and_cast<Packet *>(msg); // A message to ipv6pdm or ipv4pdm

            auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
            if (protocol != &Protocol::eigrp) { // non eigrp message
                delete msg;
                return;
            }

            auto ipversion = packet->getTag<L3AddressInd>()->getSrcAddress().getType();
            if (ipversion == inet::L3Address::IPv4) {
                this->send(msg, "splitterOut");
            }
            else if (ipversion == inet::L3Address::IPv6) {
                this->send(msg, "splitter6Out");
            }
            else {
                delete msg;
            }
        }
    }
}

} // eigrp
} // inet

