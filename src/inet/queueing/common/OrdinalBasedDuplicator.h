//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORDINALBASEDDUPLICATOR_H
#define __INET_ORDINALBASEDDUPLICATOR_H

#include <vector>

#include "inet/queueing/base/PacketDuplicatorBase.h"

namespace inet {
namespace queueing {

/**
 * Ordinal Based Duplicator module.
 */
class INET_API OrdinalBasedDuplicator : public PacketDuplicatorBase
{
  protected:
    unsigned int numPackets;
    unsigned int numDuplicated;
    bool generateFurtherDuplicates;

    std::vector<unsigned int> duplicatesVector;

  protected:
    virtual void initialize(int stage) override;
    virtual void parseVector(const char *vector);
    int getNumPacketDuplicates(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

