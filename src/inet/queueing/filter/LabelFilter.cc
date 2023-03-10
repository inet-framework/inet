//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/filter/LabelFilter.h"

#include "inet/queueing/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

Define_Module(LabelFilter);

void LabelFilter::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        labelFilter.setPattern(par("labelFilter"), false, true, true);
}

void LabelFilter::handleParameterChange(const char *name)
{
    if (!strcmp(name, "labelFilter"))
        labelFilter.setPattern(par("labelFilter"), false, true, true);
}

cGate *LabelFilter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

bool LabelFilter::matchesPacket(const Packet *packet) const
{
    auto labelsTag = packet->findTag<LabelsTag>();
    if (labelsTag != nullptr) {
        for (int i = 0; i < labelsTag->getLabelsArraySize(); i++) {
            auto label = labelsTag->getLabels(i);
            cMatchableString matchableString(label);
            if (const_cast<cMatchExpression *>(&labelFilter)->matches(&matchableString))
                return true;
        }
    }
    return false;
}

} // namespace queueing
} // namespace inet

