// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttSession.h"
namespace inet::mqtt {
int MqttSession::allocateIdentifier()
{
    // Section 2.3.1: do not reuse a sender identifier before its terminal ACK.
    for (int i = 0; i < 65535; i++) {
        int identifier = nextIdentifier;
        nextIdentifier = nextIdentifier == 65535 ? 1 : nextIdentifier + 1;
        if (allocatedIdentifiers.insert(identifier).second) return identifier;
    }
    return 0;
}
void MqttSession::releaseIdentifier(int identifier)
{
    allocatedIdentifiers.erase(identifier);
}
void MqttSession::enqueue(const Ptr<const MqttPacket>& publication)
{
    pendingPublications.push_back(publication);
}
std::vector<Ptr<MqttPacket>> MqttSession::flush()
{
    std::vector<Ptr<MqttPacket>> result;
    while (!pendingPublications.empty()) {
        auto packet = staticPtrCast<MqttPacket>(pendingPublications.front()->dupShared());
        int identifier = packet->getQos() ? allocateIdentifier() : 0;
        if (packet->getQos() && !identifier) break;
        pendingPublications.pop_front();
        packet->setPacketIdentifier(identifier); packet->setDuplicate(false);
        if (identifier) { outgoing[identifier] = packet; outgoingOrder.push_back(identifier); }
        result.push_back(packet);
    }
    return result;
}
std::vector<Ptr<MqttPacket>> MqttSession::retry() const
{
    std::vector<Ptr<MqttPacket>> result;
    for (int identifier : outgoingOrder) {
        auto packet = staticPtrCast<MqttPacket>(outgoing.at(identifier)->dupShared());
        packet->setDuplicate(packet->getType() == PUBLISH);
        result.push_back(packet);
    }
    return result;
}
Ptr<MqttPacket> MqttSession::acknowledge(const Ptr<const MqttPacket>& packet)
{
    auto found = outgoing.find(packet->getPacketIdentifier());
    if (found == outgoing.end()) return nullptr;
    if (packet->getType() == PUBREC && (found->second->getQos() == 2 || found->second->getType() == PUBREL)) {
        // Section 4.3.3: after PUBREC retain PUBREL, never resend the old PUBLISH.
        auto release = makeShared<MqttPacket>(); release->setType(PUBREL); release->setPacketIdentifier(found->first);
        found->second = release;
        return release;
    }
    if (!((packet->getType() == PUBACK && found->second->getType() == PUBLISH && found->second->getQos() == 1) ||
            (packet->getType() == PUBCOMP && found->second->getType() == PUBREL)))
        throw cRuntimeError("MQTT acknowledgement does not match the pending exchange");
    int identifier = found->first;
    releaseIdentifier(identifier); outgoing.erase(found);
    outgoingOrder.erase(std::find(outgoingOrder.begin(), outgoingOrder.end(), identifier));
    return nullptr;
}
bool MqttSession::acceptPublication(const Ptr<const MqttPacket>& packet)
{
    // Section 4.3.3 Method B: keep the identifier until PUBREL, hand off only once.
    return packet->getQos() != 2 || incomingQos2.insert(packet->getPacketIdentifier()).second;
}
void MqttSession::clearExchanges()
{
    allocatedIdentifiers.clear(); outgoing.clear(); outgoingOrder.clear(); incomingQos2.clear(); pendingPublications.clear(); nextIdentifier = 1;
}
}
