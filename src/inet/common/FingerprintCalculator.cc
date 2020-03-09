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
#include "inet/common/FingerprintCalculator.h"
#include "inet/common/packet/Packet.h"

namespace inet {

Register_Class(FingerprintCalculator);

void FingerprintCalculator::parseIngredients(const char *s)
{
    cSingleFingerprintCalculator::parseIngredients(s);
    filterEvents = strchr(s, NETWORK_COMMUNICATION_FILTER) != nullptr;
    filterProgress = strchr(s, PROGRESS_CONVERTER_FILTER) != nullptr;
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
            case NETWORK_COMMUNICATION_FILTER:
            case PROGRESS_CONVERTER_FILTER:
                break;
            case NETWORK_NODE_PATH:
                if (auto msg = dynamic_cast<cMessage *>(event)) {
                    if (auto senderNode = findContainingNode(msg->getSenderModule()))
                        hasher->add(senderNode->getFullPath().c_str());
                    if (auto arrivalNode = findContainingNode(msg->getArrivalModule()))
                        hasher->add(arrivalNode->getFullPath().c_str());
                }
                break;
            case NETWORK_INTERFACE_PATH:
                if (auto msg = dynamic_cast<cMessage *>(event)) {
                    if (auto senderInterface = findContainingNicModule(msg->getSenderModule()))
                        hasher->add(senderInterface->getInterfaceFullPath().c_str());
                    if (auto arrivalInterface = findContainingNicModule(msg->getArrivalModule()))
                        hasher->add(arrivalInterface->getInterfaceFullPath().c_str());
                }
                break;
            case PACKET_DATA: {
                if (auto cpacket = dynamic_cast<cPacket *>(event)) {
                    auto packet = dynamic_cast<Packet *>(cpacket);
                    if (packet == nullptr)
                        packet = dynamic_cast<Packet *>(cpacket->getEncapsulatedPacket());
                    if (packet != nullptr) {
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
                    }
                }
                break;
            }
            default:
                throw cRuntimeError("Unknown fingerprint ingredient '%c' (%d)", ingredient, ingredient);
        }
        return true;
    }
}

void FingerprintCalculator::addEvent(cEvent *event)
{
    if (filterProgress) {
        if (auto progress = dynamic_cast<cProgress*>(event)) {
            if (progress->getKind() != cProgress::PACKET_END)
                return;
            auto packet = progress->getPacket();
            packet->setArrival(progress->getArrivalModuleId(), progress->getArrivalGateId(), progress->getArrivalTime());
            packet->setSentFrom(progress->getSenderModule(), progress->getSenderGateId(), progress->getSendingTime());
            event = packet;
        }
    }

    if (!filterEvents) {
        cSingleFingerprintCalculator::addEvent(event);
    }
    else {
        if (event->isMessage()) {
            auto msg = static_cast<cMessage *>(event);
            if (! msg->isSelfMessage()) {
                auto senderNode = findContainingNode(msg->getSenderModule());
                auto arrivalNode = findContainingNode(msg->getArrivalModule());
                if (senderNode != arrivalNode)
                    cSingleFingerprintCalculator::addEvent(event);
            }
        }
    }
}

} // namespace

