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
//            case NETWORK_INTERFACE_PATH:
//                break;
//            case PACKET_DATA:
//                break;
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
        if (cpacket != nullptr) {
            auto senderNode = findContainingNode(cpacket->getSenderModule());
            auto arrivalNode = findContainingNode(cpacket->getArrivalModule());
            if (senderNode != arrivalNode)
                cSingleFingerprintCalculator::addEvent(event);
        }
    }

}

} // namespace

