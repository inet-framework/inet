//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/FingerprintCalculator.h"

#include "inet/common/ModuleAccess.h"
//#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Register_Class(FingerprintCalculator);

void FingerprintCalculator::parseIngredients(const char *s)
{
    cSingleFingerprintCalculator::parseIngredients(s);
    networkCommunicationFilter = strchr(s, NETWORK_COMMUNICATION_FILTER) != nullptr;
    packetUpdateFilter = strchr(s, PACKET_UPDATE_FILTER) != nullptr;
}

cSingleFingerprintCalculator::FingerprintIngredient FingerprintCalculator::validateIngredient(char ch)
{
    if (strchr(INET_FINGERPRINT_INGREDIENTS, ch) != nullptr)
        return (cSingleFingerprintCalculator::FingerprintIngredient)ch;
    else
        return cSingleFingerprintCalculator::validateIngredient(ch);
}

bool FingerprintCalculator::addEventIngredient(cEvent *event, cSingleFingerprintCalculator::FingerprintIngredient ingredient)
{
    if (strchr(INET_FINGERPRINT_INGREDIENTS, ingredient) == nullptr)
        return cSingleFingerprintCalculator::addEventIngredient(event, ingredient);
    else {
        switch ((FingerprintIngredient)ingredient) {
            case PACKET_UPDATE_FILTER:
            case NETWORK_COMMUNICATION_FILTER:
                break;
            case NETWORK_NODE_PATH:
                if (auto cpacket = dynamic_cast<cPacket *>(event)) {
                    if (auto senderNode = findContainingNode(cpacket->getSenderModule()))
                        OMNETPP6_CODE(hasher_.) OMNETPP5_CODE(hasher->)add(senderNode->getFullPath().c_str());
                    if (auto arrivalNode = findContainingNode(cpacket->getArrivalModule()))
                        OMNETPP6_CODE(hasher_.) OMNETPP5_CODE(hasher->)add(arrivalNode->getFullPath().c_str());
                }
                break;
            case NETWORK_INTERFACE_PATH:
                if (auto cpacket = dynamic_cast<cPacket *>(event)) {
                    if (auto senderInterface = findContainingNicModule(cpacket->getSenderModule()))
                        OMNETPP6_CODE(hasher_.) OMNETPP5_CODE(hasher->)add(senderInterface->getFullPath().c_str());
                    if (auto arrivalInterface = findContainingNicModule(cpacket->getArrivalModule()))
                        OMNETPP6_CODE(hasher_.) OMNETPP5_CODE(hasher->)add(arrivalInterface->getFullPath().c_str());
                }
                break;
            default:
                throw cRuntimeError("Unknown fingerprint ingredient '%c' (%d)", ingredient, ingredient);
        }
        return true;
    }
}

void FingerprintCalculator::addEvent(cEvent *event)
{
    if (networkCommunicationFilter || packetUpdateFilter) {
        cPacket *cpacket =
            (event->isMessage() && static_cast<cMessage *>(event)->isPacket()) ?
            static_cast<cPacket *>(event) : nullptr;

        if (packetUpdateFilter) {
            if (cpacket != nullptr && OMNETPP6_CODE(cpacket->isUpdate()) OMNETPP5_CODE(false))
                return;
        }

        if (networkCommunicationFilter) {
            if (cpacket == nullptr)
                return;

            auto senderNode = findContainingNode(cpacket->getSenderModule());
            auto arrivalNode = findContainingNode(cpacket->getArrivalModule());
            if (senderNode == arrivalNode)
                return;
        }
    }
    cSingleFingerprintCalculator::addEvent(event);
}

} // namespace

