//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

