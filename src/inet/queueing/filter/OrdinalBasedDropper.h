//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORDINALBASEDDROPPER_H
#define __INET_ORDINALBASEDDROPPER_H

#include <vector>

#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {
namespace queueing {

/**
 * Ordinal Based Dropper module.
 */
class INET_API OrdinalBasedDropper : public PacketFilterBase
{
  protected:
    unsigned int numPackets;
    unsigned int numDropped;

    bool generateFurtherDrops;
    std::vector<unsigned int> dropsVector;

  protected:
    virtual void initialize(int stage) override;
    virtual void parseVector(const char *vector);

    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace queueing
} // namespace inet

#endif

