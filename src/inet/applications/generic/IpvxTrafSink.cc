//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/generic/IpvxTrafGen.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace inet {

Define_Module(IpvxTrafSink);

void IpvxTrafSink::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numReceived = 0;
        WATCH(numReceived);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        int protocolId = par("protocol");
        if (protocolId < 143 || protocolId > 254)
            throw cRuntimeError("invalid protocol id %d, accepts only between 143 and 254", protocolId);
        auto protocol = ProtocolGroup::getIpProtocolGroup()->findProtocol(protocolId);
        if (!protocol) {
            char *buff = new char[40];
            sprintf(buff, "prot_%d", protocolId);
            protocol = new Protocol(buff, buff);
            ProtocolGroup::getIpProtocolGroup()->addProtocol(protocolId, protocol);
        }
        registerProtocol(*protocol, gate("ipOut"), gate("ipIn"));
    }
}

void IpvxTrafSink::handleMessageWhenUp(cMessage *msg)
{
    processPacket(check_and_cast<Packet *>(msg));
}

void IpvxTrafSink::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[32];
    sprintf(buf, "rcvd: %d pks", numReceived);
    getDisplayString().setTagArg("t", 0, buf);
}

void IpvxTrafSink::printPacket(Packet *msg)
{
    L3Address src, dest;
    int protocol = -1;
    auto ctrl = msg->getControlInfo();
    if (ctrl != nullptr) {
        protocol = ProtocolGroup::getIpProtocolGroup()->getProtocolNumber(msg->getTag<PacketProtocolTag>()->getProtocol());
    }
    Ptr<const L3AddressTagBase> addresses = msg->findTag<L3AddressReq>();
    if (addresses == nullptr)
        addresses = msg->findTag<L3AddressInd>();
    if (addresses != nullptr) {
        src = addresses->getSrcAddress();
        dest = addresses->getDestAddress();
    }

    EV_INFO << msg << endl;
    EV_INFO << "Payload length: " << msg->getByteLength() << " bytes" << endl;

    if (ctrl != nullptr)
        EV_INFO << "src: " << src << "  dest: " << dest << "  protocol=" << protocol << endl;
}

void IpvxTrafSink::processPacket(Packet *msg)
{
    emit(packetReceivedSignal, msg);
    EV_INFO << "Received packet: ";
    printPacket(msg);
    delete msg;
    numReceived++;
}

} // namespace inet

