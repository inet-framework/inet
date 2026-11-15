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
}
