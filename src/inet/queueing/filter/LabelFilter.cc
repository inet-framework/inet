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
    if (stage == INITSTAGE_LOCAL) {
        cValueArray *includeLabels = check_and_cast<cValueArray *>(par("includeLabels").objectValue());
        for (int i = 0; i < includeLabels->size(); i++) {
            auto includeLabel = includeLabels->get(i).stringValue();
            PatternMatcher patternMatcher;
            patternMatcher.setPattern(includeLabel, false, true, true);
            includeLabelMatchers.push_back(patternMatcher);
        }
        cValueArray *excludeLabels = check_and_cast<cValueArray *>(par("excludeLabels").objectValue());
        for (int i = 0; i < excludeLabels->size(); i++) {
            auto excludeLabel = excludeLabels->get(i).stringValue();
            PatternMatcher patternMatcher;
            patternMatcher.setPattern(excludeLabel, false, true, true);
            excludeLabelMatchers.push_back(patternMatcher);
        }
    }
}

bool LabelFilter::matchesPacket(const Packet *packet) const
{
    auto labelsTag = packet->findTag<LabelsTag>();
    if (labelsTag != nullptr) {
        for (int i = 0; i < labelsTag->getLabelsArraySize(); i++) {
            auto label = labelsTag->getLabels(i);
            for (auto& excludeLabelMatcher : excludeLabelMatchers) {
                if (excludeLabelMatcher.matches(label))
                    return false;
            }
            for (auto& includeLabelMatcher : includeLabelMatchers) {
                if (includeLabelMatcher.matches(label))
                    return true;
            }
        }
    }
    return false;
}

} // namespace queueing
} // namespace inet

