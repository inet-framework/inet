//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/linklayer/ieee80211/mac/framesequence/GenericFrameSequences.h"

namespace inet {
namespace ieee80211 {

SequentialFs::SequentialFs(std::vector<IFrameSequence*> elements) :
    elements(elements)
{
}

void SequentialFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
    elementIndex = 0;
    if (elementIndex < (int) (elements.size()))
        elements[elementIndex]->startSequence(context, firstStep);
}

IFrameSequenceStep *SequentialFs::prepareStep(FrameSequenceContext *context)
{
    while (elementIndex < (int) elements.size()) {
        auto elementStep = elements[elementIndex]->prepareStep(context);
        if (elementStep != nullptr)
            return elementStep;
        else {
            elementIndex++;
            if (elementIndex < (int) elements.size())
                elements[elementIndex]->startSequence(context, firstStep + step);
        }
    }
    return nullptr;
}

bool SequentialFs::completeStep(FrameSequenceContext *context)
{
    step++;
    return elements[elementIndex]->completeStep(context);
}

std::string SequentialFs::getHistory() const
{
    ASSERT(step != -1);
    std::string history;
    for (int i = 0; i < std::min(elementIndex + 1, (int)elements.size()); i++) {
        auto elementHistory = elements.at(i)->getHistory();
        if (!elementHistory.empty()) {
            if (!history.empty())
                history += " ";
            history += elementHistory;
        }
    }
    history = "(" + history + ")";
    return history;
}


SequentialFs::~SequentialFs()
{
    for (auto element : elements)
        delete element;
}


OptionalFs::OptionalFs(IFrameSequence *element, std::function<bool(OptionalFs*, FrameSequenceContext*)> predicate) :
    element(element),
    predicate(predicate)
{
}

void OptionalFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
    apply = isSequenceApply(context);
    if (apply)
        element->startSequence(context, firstStep);
}

IFrameSequenceStep *OptionalFs::prepareStep(FrameSequenceContext *context)
{
    return apply ? element->prepareStep(context) : nullptr;
}

bool OptionalFs::completeStep(FrameSequenceContext *context)
{
    ASSERT(apply);
    step++;
    return element->completeStep(context);
}

std::string OptionalFs::getHistory() const
{
    ASSERT(step != -1);
    return apply ? "["+ element->getHistory() + "]" : "";
}

RepeatingFs::RepeatingFs(IFrameSequence *element, std::function<bool(RepeatingFs*, FrameSequenceContext*)> predicate) :
    element(element),
    predicate(predicate)
{
}

void RepeatingFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
    apply = isSequenceApply(context);
    if (apply) {
        count = 1;
        element->startSequence(context, firstStep);
        histories.push_back(element->getHistory());
    }
}

void RepeatingFs::repeatSequence(FrameSequenceContext *context)
{
    apply = isSequenceApply(context);
    if (apply) {
        count++;
        element->startSequence(context, firstStep + step);
        histories.push_back(element->getHistory());
    }
}

IFrameSequenceStep *RepeatingFs::prepareStep(FrameSequenceContext *context)
{
    if (apply) {
        auto elementStep = element->prepareStep(context);
        if (elementStep != nullptr)
            return elementStep;
        else {
            repeatSequence(context);
            return prepareStep(context);
        }
    }
    else
        return nullptr;
}

bool RepeatingFs::completeStep(FrameSequenceContext *context)
{
    ASSERT(apply);
    step++;
    bool complete = element->completeStep(context);
    histories.at(count - 1) = element->getHistory();
    return complete;
}

std::string RepeatingFs::getHistory() const
{
    ASSERT(step != -1);
    std::string history;
    for (int i = 0; i < (int) histories.size(); i++) {
        auto elementHistory = histories.at(i);
        if (!elementHistory.empty()) {
            if (!history.empty())
                history += " ";
            history += elementHistory;
        }
    }
    return "{" + history + "}";
}

AlternativesFs::AlternativesFs(std::vector<IFrameSequence*> elements, std::function<int(AlternativesFs*, FrameSequenceContext*)> selector) :
    elements(elements),
    selector(selector)
{
}

void AlternativesFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
    elementIndex = selectSequence(context);
    elements[elementIndex]->startSequence(context, firstStep);
}

IFrameSequenceStep *AlternativesFs::prepareStep(FrameSequenceContext *context)
{
    return elements[elementIndex]->prepareStep(context);
}

bool AlternativesFs::completeStep(FrameSequenceContext *context)
{
    step++;
    return elements[elementIndex]->completeStep(context);
}

std::string AlternativesFs::getHistory() const
{
    ASSERT(step != -1);
    ASSERT(0 <= elementIndex && (size_t)elementIndex < elements.size());
    return elements[elementIndex]->getHistory();
}

AlternativesFs::~AlternativesFs()
{
    for (auto element : elements)
        delete element;
}

} // namespace ieee80211
} // namespace inet
