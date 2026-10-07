//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/GenericFrameSequences.h"

namespace inet {
namespace ieee80211 {

SequentialFs::SequentialFs(std::vector<IFrameSequence *> elements) :
    elements(elements)
{
}

void SequentialFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    executionPlan = nullptr;
    this->firstStep = firstStep;
    step = 0;
    elementIndex = 0;
    if (elementIndex < (int)(elements.size()))
        elements[elementIndex]->startSequence(context, firstStep);
}

IFrameSequenceStep *SequentialFs::prepareStep(FrameSequenceContext *context)
{
    while (elementIndex < (int)elements.size()) {
        auto elementStep = elements[elementIndex]->prepareStep(context);
        if (elementStep != nullptr)
            return elementStep;
        else {
            elementIndex++;
            if (elementIndex < (int)elements.size()) {
                if (executionPlan)
                    elements[elementIndex]->startPlannedSequence(context, firstStep + step, *executionPlan->children.at(elementIndex));
                else
                    elements[elementIndex]->startSequence(context, firstStep + step);
            }
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

OptionalFs::OptionalFs(IFrameSequence *element, std::function<bool(OptionalFs *, FrameSequenceContext *)> predicate) :
    element(element),
    predicate(predicate)
{
}

void OptionalFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    executionPlan = nullptr;
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
    return apply ? "[" + element->getHistory() + "]" : "";
}

RepeatingFs::RepeatingFs(IFrameSequence *element, std::function<bool(RepeatingFs *, FrameSequenceContext *)> predicate) :
    element(element),
    predicate(predicate)
{
}

void RepeatingFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    executionPlan = nullptr;
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
            if (executionPlan) {
                apply = false;
                return nullptr;
            }
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
    for (size_t i = 0; i < histories.size(); i++) {
        auto elementHistory = histories.at(i);
        if (!elementHistory.empty()) {
            if (!history.empty())
                history += " ";
            history += elementHistory;
        }
    }
    return "{" + history + "}";
}

AlternativesFs::AlternativesFs(std::vector<IFrameSequence *> elements, std::function<int(AlternativesFs *, FrameSequenceContext *)> selector) :
    elements(elements),
    selector(selector)
{
}

void AlternativesFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    executionPlan = nullptr;
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


FrameSequencePlanResult SequentialFs::planSequence(FrameSequencePlanningContext& context) const
{
    auto plan = context.makePlan(this);
    for (auto element : elements) {
        auto result = element->planSequence(context);
        if (result.status != FrameSequencePlanResult::Status::READY)
            return result.status;
        plan->append(std::move(result.plan));
    }
    return plan;
}

void SequentialFs::startPlannedSequence(FrameSequenceContext *context, int firstStep, FrameSequencePlan& plan)
{
    ASSERT(plan.sequence == this);
    executionPlan = &plan;
    this->firstStep = firstStep;
    step = 0;
    elementIndex = 0;
    if (!elements.empty())
        elements[0]->startPlannedSequence(context, firstStep, *plan.children.at(0));
}

FrameSequencePlanResult OptionalFs::planSequence(FrameSequencePlanningContext& context) const
{
    if (!predicate)
        return FrameSequencePlanResult::Status::UNSUPPORTED;
    auto plan = context.makePlan(this);
    if (predicate(const_cast<OptionalFs *>(this), &context)) {
        auto result = element->planSequence(context);
        if (result.status != FrameSequencePlanResult::Status::READY)
            return result.status;
        plan->append(std::move(result.plan));
    }
    return plan;
}

void OptionalFs::startPlannedSequence(FrameSequenceContext *context, int firstStep, FrameSequencePlan& plan)
{
    ASSERT(plan.sequence == this);
    executionPlan = &plan;
    this->firstStep = firstStep;
    step = 0;
    apply = !plan.children.empty();
    if (apply)
        element->startPlannedSequence(context, firstStep, *plan.children.at(0));
}

FrameSequencePlanResult AlternativesFs::planSequence(FrameSequencePlanningContext& context) const
{
    if (elements.empty() || !selector)
        return FrameSequencePlanResult::Status::UNSUPPORTED;
    if (!context.supportsPreparation())
        return FrameSequencePlanResult::Status::UNSUPPORTED;
    if (!context.hasFrameToTransmit())
        return FrameSequencePlanResult::Status::EMPTY;
    auto plan = context.makePlan(this);
    plan->selectedChild = selector(const_cast<AlternativesFs *>(this), &context);
    if (plan->selectedChild < 0 || plan->selectedChild >= (int)elements.size())
        return FrameSequencePlanResult::Status::UNSUPPORTED;
    auto result = elements[plan->selectedChild]->planSequence(context);
    if (result.status != FrameSequencePlanResult::Status::READY)
        return result.status;
    plan->append(std::move(result.plan));
    return plan;
}

void AlternativesFs::startPlannedSequence(FrameSequenceContext *context, int firstStep, FrameSequencePlan& plan)
{
    ASSERT(plan.sequence == this);
    executionPlan = &plan;
    this->firstStep = firstStep;
    step = 0;
    elementIndex = plan.selectedChild;
    elements.at(elementIndex)->startPlannedSequence(context, firstStep, *plan.children.at(0));
}

FrameSequencePlanResult RepeatingFs::planSequence(FrameSequencePlanningContext& context) const
{
    if (!predicate)
        return FrameSequencePlanResult::Status::UNSUPPORTED;
    auto plan = context.makePlan(this);
    if (predicate(const_cast<RepeatingFs *>(this), &context)) {
        if (!context.enterRepetition())
            return FrameSequencePlanResult::Status::UNSUPPORTED;
        // One finite use per admission point. An empty repeated child cannot make progress.
        auto result = element->planSequence(context);
        if (result.status != FrameSequencePlanResult::Status::READY)
            return result.status;
        if (result.plan->stepCount == 0)
            return FrameSequencePlanResult::Status::UNSUPPORTED;
        plan->append(std::move(result.plan));
    }
    return plan;
}

void RepeatingFs::startPlannedSequence(FrameSequenceContext *context, int firstStep, FrameSequencePlan& plan)
{
    ASSERT(plan.sequence == this);
    executionPlan = &plan;
    this->firstStep = firstStep;
    step = 0;
    histories.clear();
    apply = !plan.children.empty();
    count = apply ? 1 : 0;
    if (apply) {
        element->startPlannedSequence(context, firstStep, *plan.children.at(0));
        histories.push_back(element->getHistory());
    }
}

} // namespace ieee80211
} // namespace inet
