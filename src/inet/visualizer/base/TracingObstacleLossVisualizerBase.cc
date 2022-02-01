//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/TracingObstacleLossVisualizerBase.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

using namespace inet::physicalenvironment;
using namespace inet::physicallayer;

void TracingObstacleLossVisualizerBase::preDelete(cComponent *root)
{
    if (displayIntersections) {
        unsubscribe();
        removeAllObstacleLossVisualizations();
    }
}

void TracingObstacleLossVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayIntersections = par("displayIntersections");
        intersectionLineColor = cFigure::parseColor(par("intersectionLineColor"));
        intersectionLineStyle = cFigure::parseLineStyle(par("intersectionLineStyle"));
        intersectionLineWidth = par("intersectionLineWidth");
        displayFaceNormalVectors = par("displayFaceNormalVectors");
        faceNormalLineColor = cFigure::parseColor(par("faceNormalLineColor"));
        faceNormalLineStyle = cFigure::parseLineStyle(par("faceNormalLineStyle"));
        faceNormalLineWidth = par("faceNormalLineWidth");
        fadeOutMode = par("fadeOutMode");
        fadeOutTime = par("fadeOutTime");
        fadeOutAnimationSpeed = par("fadeOutAnimationSpeed");
        if (displayIntersections)
            subscribe();
    }
}

void TracingObstacleLossVisualizerBase::refreshDisplay() const
{
    AnimationPosition currentAnimationPosition;
    std::vector<const ObstacleLossVisualization *> removedObstacleLossVisualizations;
    for (auto obstacleLossVisualization : obstacleLossVisualizations) {
        double delta;
        if (!strcmp(fadeOutMode, "simulationTime"))
            delta = (currentAnimationPosition.getSimulationTime() - obstacleLossVisualization->obstacleLossAnimationPosition.getSimulationTime()).dbl();
        else if (!strcmp(fadeOutMode, "animationTime"))
            delta = currentAnimationPosition.getAnimationTime() - obstacleLossVisualization->obstacleLossAnimationPosition.getAnimationTime();
        else if (!strcmp(fadeOutMode, "realTime"))
            delta = currentAnimationPosition.getRealTime() - obstacleLossVisualization->obstacleLossAnimationPosition.getRealTime();
        else
            throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
        if (delta > fadeOutTime)
            removedObstacleLossVisualizations.push_back(obstacleLossVisualization);
        else
            setAlpha(obstacleLossVisualization, 1 - delta / fadeOutTime);
    }
    for (auto obstacleLossVisualization : removedObstacleLossVisualizations) {
        const_cast<TracingObstacleLossVisualizerBase *>(this)->removeObstacleLossVisualization(obstacleLossVisualization);
        delete obstacleLossVisualization;
    }
}

void TracingObstacleLossVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(ITracingObstacleLoss::obstaclePenetratedSignal, this);
}

void TracingObstacleLossVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr)
        visualizationSubjectModule->unsubscribe(ITracingObstacleLoss::obstaclePenetratedSignal, this);
}

void TracingObstacleLossVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == ITracingObstacleLoss::obstaclePenetratedSignal) {
        if (displayIntersections || displayFaceNormalVectors) {
            auto event = static_cast<ITracingObstacleLoss::ObstaclePenetratedEvent *>(object);
            auto obstacleLossVisualization = createObstacleLossVisualization(event);
            addObstacleLossVisualization(obstacleLossVisualization);
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

void TracingObstacleLossVisualizerBase::addObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization)
{
    obstacleLossVisualizations.push_back(obstacleLossVisualization);
}

void TracingObstacleLossVisualizerBase::removeObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization)
{
    obstacleLossVisualizations.erase(std::remove(obstacleLossVisualizations.begin(), obstacleLossVisualizations.end(), obstacleLossVisualization), obstacleLossVisualizations.end());
}

void TracingObstacleLossVisualizerBase::removeAllObstacleLossVisualizations()
{
    std::vector<const ObstacleLossVisualization *> removedObstacleLossVisualizations;
    for (auto it : obstacleLossVisualizations)
        removedObstacleLossVisualizations.push_back(it);
    for (auto it : removedObstacleLossVisualizations) {
        removeObstacleLossVisualization(it);
        delete it;
    }
}

} // namespace visualizer

} // namespace inet

