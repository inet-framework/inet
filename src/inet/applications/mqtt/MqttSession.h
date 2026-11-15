// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTSESSION_H
#define __INET_MQTTSESSION_H
#include <set>
#include <deque>
#include "inet/applications/mqtt/MqttPacket_m.h"
namespace inet::mqtt {
struct INET_API MqttSession
{
    std::map<std::string, int> subscriptions;
    std::set<int> allocatedIdentifiers;
    int nextIdentifier = 1;
    std::deque<Ptr<const MqttPacket>> pendingPublications;
    std::map<int, Ptr<const MqttPacket>> outgoing;
    std::deque<int> outgoingOrder;
    int allocateIdentifier();
    void releaseIdentifier(int identifier);
    void enqueue(const Ptr<const MqttPacket>& publication);
    std::vector<Ptr<MqttPacket>> flush();
    std::vector<Ptr<MqttPacket>> retry() const;
    void acknowledge(const Ptr<const MqttPacket>& packet);
    void clearExchanges();
};
}
#endif
