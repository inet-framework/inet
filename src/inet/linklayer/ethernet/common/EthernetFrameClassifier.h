//
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETFRAMECLASSIFIER_H
#define __INET_ETHERNETFRAMECLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {

/**
 * Ethernet Frame classifier.
 *
 * Ethernet frames are classified as:
 * - PAUSE frames
 * - others
 */
class INET_API EthernetFrameClassifier : public queueing::PacketClassifierBase
{
  public:
    /**
     * Sends the incoming packet to either the first or the second gate.
     */
    virtual int classifyPacket(Packet *packet) override;
};

} // namespace inet

#endif

