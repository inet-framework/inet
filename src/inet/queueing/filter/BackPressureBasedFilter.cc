//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/queueing/filter/BackPressureBasedFilter.h"

namespace inet {
namespace queueing {

Define_Module(BackPressureBasedFilter);

bool BackPressureBasedFilter::matchesPacket(const Packet *packet) const
{
    return consumer != nullptr && consumer.canPushPacket(const_cast<Packet *>(packet));
}

} // namespace queueing
} // namespace inet
