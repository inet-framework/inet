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

#include "inet/common/ModuleAccess.h"
#include "inet/common/NetworkCommunicationFingerprintCalculator.h"
#include "inet/common/packet/Packet.h"

namespace inet {

Register_Class(NetworkCommunicationFingerprintCalculator);

NetworkCommunicationFingerprintCalculator::FingerprintIngredient NetworkCommunicationFingerprintCalculator::validateIngredient(char ch)
{
    if (strchr("NID", ch) != nullptr)
        return (FingerprintIngredient)ch;
    else
        return cSingleFingerprintCalculator::validateIngredient(ch);
}

bool NetworkCommunicationFingerprintCalculator::addEventIngredient(cEvent *event, FingerprintIngredient ingredient)
{
    if (strchr("NID", ingredient) == nullptr)
        return false;
    else {
        auto cpacket = check_and_cast<cPacket *>(event);
        auto senderNode = findContainingNode(cpacket->getSenderModule());
        auto arrivalNode = findContainingNode(cpacket->getArrivalModule());
        switch ((NetworkCommunicationFingerprintIngredient)ingredient) {
            case NETWORK_NODE_PATH:
                hasher->add(senderNode->getFullPath().c_str());
                hasher->add(arrivalNode->getFullPath().c_str());
                break;
            case NETWORK_INTERFACE_PATH:
                if (auto senderInterface = findContainingNicModule(cpacket->getSenderModule()))
                    hasher->add(senderInterface->getInterfaceFullPath().c_str());
                if (auto arrivalInterface = findContainingNicModule(cpacket->getArrivalModule()))
                    hasher->add(arrivalInterface->getInterfaceFullPath().c_str());
                break;
            case PACKET_DATA: {
                auto packet = dynamic_cast<Packet *>(cpacket);
                if (packet == nullptr)
                    packet = check_and_cast<Packet *>(cpacket->getEncapsulatedPacket());
                if (packet->getTotalLength().get() % 8 == 0) {
                    const auto& content = packet->peekAllAsBytes();
                    for (auto byte : content->getBytes())
                        hasher->add(byte);
                }
                else {
                    const auto& content = packet->peekAllAsBits();
                    for (auto bit : content->getBits())
                        hasher->add(bit);
                }
                break;
            }
            default:
                throw cRuntimeError("Unknown fingerprint ingredient '%c' (%d)", ingredient, ingredient);
        }
        return true;
    }
}

void NetworkCommunicationFingerprintCalculator::addEvent(cEvent *event)
{
    if (event->isMessage() && static_cast<cMessage *>(event)->isPacket()) {
        auto cpacket = static_cast<cPacket *>(event);
        auto packet = dynamic_cast<Packet *>(cpacket);
        if (packet == nullptr)
            packet = dynamic_cast<Packet *>(cpacket->getEncapsulatedPacket());
        if (packet != nullptr) {
            auto senderNode = findContainingNode(cpacket->getSenderModule());
            auto arrivalNode = findContainingNode(cpacket->getArrivalModule());
            if (senderNode != arrivalNode)
                cSingleFingerprintCalculator::addEvent(event);
        }
    }

}

} // namespace

