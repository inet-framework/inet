//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketLabelerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

void PacketLabelerBase::initialize(int stage)
{
    PacketMarkerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        labels = cStringTokenizer(par("labels")).asVector();
    }
}

} // namespace queueing
} // namespace inet

