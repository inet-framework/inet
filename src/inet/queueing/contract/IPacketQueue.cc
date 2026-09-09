// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace queueing {

simsignal_t IPacketQueue::packetQueueDepartureSignal = cComponent::registerSignal("packetQueueDeparture");

} // namespace queueing
} // namespace inet
