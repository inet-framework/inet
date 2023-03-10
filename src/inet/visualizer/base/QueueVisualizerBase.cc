//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/QueueVisualizerBase.h"

#include <algorithm>

namespace inet {

namespace visualizer {

bool QueueVisualizerBase::QueueVisitor::visit(cObject *object)
{
    if (auto queue = dynamic_cast<queueing::IPacketQueue *>(object))
        queues.push_back(queue);
    else
        object->forEachChild(this);
    return true;
}

QueueVisualizerBase::QueueVisualization::QueueVisualization(queueing::IPacketQueue *queue) :
    queue(queue)
{
}

void QueueVisualizerBase::preDelete(cComponent *root)
{
    if (displayQueues)
        removeAllQueueVisualizations();
}

void QueueVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayQueues = par("displayQueues");
        queueFilter.setPattern(par("queueFilter"));
        color = cFigure::parseColor(par("color"));
        width = par("width");
        height = par("height");
        spacing = par("spacing");
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
    }
    else if (stage == INITSTAGE_LAST) {
        if (displayQueues)
            addQueueVisualizations();
    }
}

void QueueVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    removeAllQueueVisualizations();
    addQueueVisualizations();
}

void QueueVisualizerBase::refreshDisplay() const
{
    for (auto queueVisualization : queueVisualizations)
        refreshQueueVisualization(queueVisualization);
}

void QueueVisualizerBase::addQueueVisualization(const QueueVisualization *queueVisualization)
{
    queueVisualizations.push_back(queueVisualization);
}

void QueueVisualizerBase::removeQueueVisualization(const QueueVisualization *queueVisualization)
{
    queueVisualizations.erase(std::remove(queueVisualizations.begin(), queueVisualizations.end(), queueVisualization), queueVisualizations.end());
}

void QueueVisualizerBase::addQueueVisualizations()
{
    QueueVisitor queueVisitor;
    visualizationSubjectModule->forEachChild(&queueVisitor);
    for (auto queue : queueVisitor.queues) {
        if (queueFilter.matches(queue))
            addQueueVisualization(createQueueVisualization(queue));
    }
}

void QueueVisualizerBase::removeAllQueueVisualizations()
{
    for (auto queueVisualization : std::vector<const QueueVisualization *>(queueVisualizations)) {
        removeQueueVisualization(queueVisualization);
        delete queueVisualization;
    }
}

} // namespace visualizer

} // namespace inet

