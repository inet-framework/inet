// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTSESSION_H
#define __INET_MQTTSESSION_H
#include <set>
#include "inet/applications/mqtt/MqttPacket_m.h"
namespace inet::mqtt {
struct INET_API MqttSession
{
    std::map<std::string, int> subscriptions;
    std::set<int> allocatedIdentifiers;
    int nextIdentifier = 1;
    int allocateIdentifier();
    void releaseIdentifier(int identifier);
};
}
#endif
