//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>

#include "inet/visualizer/base/QueueVisualizerBase.h"

namespace inet {

namespace visualizer {

VISIT_RETURNTYPE QueueVisualizerBase::QueueVisitor::visit(cObject *object)
{
    if (auto queue = dynamic_cast<queueing::IPacketQueue *>(object))
        queues.push_back(queue);
    else
        object->forEachChild(this);
    VISIT_RETURN(true);
}

QueueVisualizerBase::QueueVisualization::QueueVisualization(queueing::IPacketQueue *queue) :
    queue(queue)
{
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
    if (name != nullptr) {
        removeAllQueueVisualizations();
        addQueueVisualizations();
    }
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

