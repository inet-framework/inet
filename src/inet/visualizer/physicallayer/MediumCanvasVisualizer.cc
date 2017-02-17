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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/physicallayer/MediumCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(MediumCanvasVisualizer);

MediumCanvasVisualizer::~MediumCanvasVisualizer()
{
    cancelAndDelete(signalPropagationUpdateTimer);
}

void MediumCanvasVisualizer::initialize(int stage)
{
    MediumVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        const char *signalShapeString = par("signalShape");
        if (!strcmp(signalShapeString, "ring"))
            signalShape = SIGNAL_SHAPE_RING;
        else if (!strcmp(signalShapeString, "sphere"))
            signalShape = SIGNAL_SHAPE_SPHERE;
        else
            throw cRuntimeError("Unknown signalShape parameter value: '%s'", signalShapeString);
        cCanvas *canvas = visualizerTargetModule->getCanvas();
        if (displaySignals) {
            communicationLayer = new cGroupFigure("communication");
            communicationLayer->insertBelow(canvas->getSubmodulesLayer());
        }
        if (displayRadioFrames) {
            radioFrameLayer = new cGroupFigure("radioFrameLayer");
            radioFrameLayer->insertAbove(canvas->getSubmodulesLayer());
        }
        displayCommunicationHeat = par("displayCommunicationHeat");
        if (displayCommunicationHeat) {
            communicationHeat = new HeatMapFigure(communicationHeatMapSize, "communication heat");
            communicationHeat->setTags("successful_reception heat");
            canvas->addFigure(communicationHeat, 0);
        }
        signalPropagationUpdateTimer = new cMessage("signalPropagation");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
    }
    else if (stage == INITSTAGE_LAST) {
        canvasProjection = CanvasProjection::getCanvasProjection(visualizerTargetModule->getCanvas());
        if (communicationHeat != nullptr) {
            const IMediumLimitCache *mediumLimitCache = radioMedium->getMediumLimitCache();
            Coord min = mediumLimitCache->getMinConstraintArea();
            Coord max = mediumLimitCache->getMaxConstraintArea();
            cFigure::Point o = canvasProjection->computeCanvasPoint(Coord::ZERO);
            cFigure::Point x = canvasProjection->computeCanvasPoint(Coord(1, 0, 0));
            cFigure::Point y = canvasProjection->computeCanvasPoint(Coord(0, 1, 0));
            double t1 = o.x;
            double t2 = o.y;
            double a = x.x - t1;
            double b = x.y - t2;
            double c = y.x - t1;
            double d = y.y - t2;
            communicationHeat->setTransform(cFigure::Transform(a, b, c, d, t1, t2));
            communicationHeat->setPosition(cFigure::Point((min.x + max.x) / 2, (min.y + max.y) / 2));
            communicationHeat->setWidth(max.x - min.x);
            communicationHeat->setHeight(max.y - min.y);
        }
    }
}

void MediumCanvasVisualizer::handleMessage(cMessage *message)
{
    if (message == signalPropagationUpdateTimer)
        scheduleSignalPropagationUpdateTimer();
    else
        throw cRuntimeError("Unknown message");
}

cFigure *MediumCanvasVisualizer::getCachedFigure(const ITransmission *transmission) const
{
    auto it = transmissionFigures.find(transmission);
    if (it == transmissionFigures.end())
        return nullptr;
    else
        return it->second;
}

void MediumCanvasVisualizer::setCachedFigure(const ITransmission *transmission, cFigure *figure)
{
    transmissionFigures[transmission] = figure;
}

void MediumCanvasVisualizer::removeCachedFigure(const ITransmission *transmission)
{
    transmissionFigures.erase(transmission);
}

void MediumCanvasVisualizer::radioAdded(const IRadio *radio)
{
    Enter_Method_Silent();
    auto module = check_and_cast<const cModule *>(radio);
    if (displayInterferenceRanges || (module->hasPar("displayInterferenceRange") && module->par("displayInterferenceRange"))) {
        auto module = check_and_cast<const cModule *>(radio);
        auto node = getContainingNode(module);
        auto networkNodeVisualization = networkNodeVisualizer->getNeworkNodeVisualization(node);
        auto interferenceRangeFigure = new cOvalFigure("interferenceRange");
        m maxInterferenceRange = check_and_cast<const IRadioMedium *>(radio->getMedium())->getMediumLimitCache()->getMaxInterferenceRange(radio);
        interferenceRangeFigure->setBounds(cFigure::Rectangle(-maxInterferenceRange.get(), -maxInterferenceRange.get(), 2 * maxInterferenceRange.get(), 2 * maxInterferenceRange.get()));
        interferenceRangeFigure->setLineColor(interferenceRangeColor);
        networkNodeVisualization->addFigure(interferenceRangeFigure);
    }
    if (displayCommunicationRanges || (module->hasPar("displayCommunicationRange") && module->par("displayCommunicationRange"))) {
        auto module = check_and_cast<const cModule *>(radio);
        auto node = getContainingNode(module);
        auto networkNodeVisualization = networkNodeVisualizer->getNeworkNodeVisualization(node);
        auto communicationRangeFigure = new cOvalFigure("communicationRange");
        m maxCommunicationRange = check_and_cast<const IRadioMedium *>(radio->getMedium())->getMediumLimitCache()->getMaxCommunicationRange(radio);
        communicationRangeFigure->setBounds(cFigure::Rectangle(-maxCommunicationRange.get(), -maxCommunicationRange.get(), 2 * maxCommunicationRange.get(), 2 * maxCommunicationRange.get()));
        communicationRangeFigure->setLineColor(communicationRangeColor);
        networkNodeVisualization->addFigure(communicationRangeFigure);
    }
}

void MediumCanvasVisualizer::radioRemoved(const IRadio *radio)
{
    Enter_Method_Silent();
}

void MediumCanvasVisualizer::transmissionAdded(const ITransmission *transmission)
{
    Enter_Method_Silent();
    if (displaySignals) {
        transmissions.push_back(transmission);
        cFigure::Point position = canvasProjection->computeCanvasPoint(transmission->getStartPosition());
        cGroupFigure *groupFigure = new cGroupFigure("signal");
        cFigure::Color color = cFigure::GOOD_DARK_COLORS[transmission->getId() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
        cRingFigure *communicationFigure = new cRingFigure("bubble");
        communicationFigure->setTags("ongoing_transmission");
        communicationFigure->setBounds(cFigure::Rectangle(position.x, position.y, 0, 0));
        communicationFigure->setFillColor(color);
        communicationFigure->setLineWidth(1);
        communicationFigure->setLineColor(cFigure::BLACK);
        groupFigure->addFigure(communicationFigure);
        communicationFigure->setFilled(true);
        communicationFigure->setFillOpacity(0.5);
        communicationFigure->setLineOpacity(0.5);
        communicationFigure->setZoomLineWidth(false);
        cLabelFigure *nameFigure = new cLabelFigure("name");
        nameFigure->setPosition(position);
        nameFigure->setTags("ongoing_transmission packet_name label");
        nameFigure->setText(transmission->getMacFrame()->getName());
        nameFigure->setColor(color);
        groupFigure->addFigure(nameFigure);
        communicationLayer->addFigure(groupFigure);
        setCachedFigure(transmission, groupFigure);
        if (signalPropagationUpdateInterval > 0)
            scheduleSignalPropagationUpdateTimer();
    }
}

void MediumCanvasVisualizer::transmissionRemoved(const ITransmission *transmission)
{
    Enter_Method_Silent();
    if (displaySignals) {
        transmissions.erase(std::remove(transmissions.begin(), transmissions.end(), transmission));
        cFigure *figure = getCachedFigure(transmission);
        removeCachedFigure(transmission);
        if (figure != nullptr)
            delete communicationLayer->removeFigure(figure);
    }
}

void MediumCanvasVisualizer::transmissionStarted(const ITransmission *transmission)
{
    Enter_Method_Silent();
}

void MediumCanvasVisualizer::transmissionEnded(const ITransmission *transmission)
{
    Enter_Method_Silent();
}

void MediumCanvasVisualizer::receptionStarted(const IReception *reception)
{
    Enter_Method_Silent();
    const ITransmission *transmission = reception->getTransmission();
    if (displayRadioFrames) {
        cLineFigure *communicationFigure = new cLineFigure("signal");
        communicationFigure->setTags("radio_frame recent_history");
        cFigure::Point start = canvasProjection->computeCanvasPoint(transmission->getStartPosition());
        cFigure::Point end = canvasProjection->computeCanvasPoint(reception->getStartPosition());
        communicationFigure->setStart(start);
        communicationFigure->setEnd(end);
        communicationFigure->setLineColor(radioFrameLineColor);
        communicationFigure->setEndArrowhead(cFigure::ARROW_BARBED);
        communicationFigure->setLineWidth(1);
        communicationFigure->setZoomLineWidth(false);
        radioFrameLayer->addFigure(communicationFigure);
    }
    if (displayCommunicationHeat) {
        const IMediumLimitCache *mediumLimitCache = radioMedium->getMediumLimitCache();
        Coord min = mediumLimitCache->getMinConstraintArea();
        Coord max = mediumLimitCache->getMaxConstraintArea();
        Coord delta = max - min;
        int x1 = std::round((communicationHeatMapSize - 1) * ((transmission->getStartPosition().x - min.x) / delta.x));
        int y1 = std::round((communicationHeatMapSize - 1) * ((transmission->getStartPosition().y - min.x) / delta.y));
        int x2 = std::round((communicationHeatMapSize - 1) * ((reception->getStartPosition().x - min.x) / delta.x));
        int y2 = std::round((communicationHeatMapSize - 1) * ((reception->getStartPosition().y - min.y) / delta.y));
        communicationHeat->heatLine(x1, y1, x2, y2);
    }
}

void MediumCanvasVisualizer::receptionEnded(const IReception *reception)
{
    Enter_Method_Silent();
}

void MediumCanvasVisualizer::refreshDisplay() const
{
    if (displaySignals) {
        const IPropagation *propagation = radioMedium->getPropagation();
        if (communicationHeat != nullptr)
            communicationHeat->coolDown();
        for (const auto transmission : transmissions) {
            cFigure *groupFigure = getCachedFigure(transmission);
            double startRadius = propagation->getPropagationSpeed().get() * (simTime() - transmission->getStartTime()).dbl();
            double endRadius = std::max(0.0, propagation->getPropagationSpeed().get() * (simTime() - transmission->getEndTime()).dbl());
            if (groupFigure) {
                cRingFigure *communicationFigure = static_cast<cRingFigure *>(groupFigure->getFigure(0));
                cLabelFigure *labelFigure = static_cast<cLabelFigure *>(groupFigure->getFigure(1));
                double phi = transmission->getId();
                labelFigure->setTransform(cFigure::Transform().translate(endRadius * sin(phi), endRadius * cos(phi)));
                const Coord transmissionStart = transmission->getStartPosition();
                // KLUDGE: to workaround overflow bugs in drawing
                if (startRadius > 10000)
                    startRadius = 10000;
                if (endRadius > 10000)
                    endRadius = 10000;
                switch (signalShape) {
                    case SIGNAL_SHAPE_RING: {
                        // determine the rotated 2D canvas points by computing the 2D affine trasnformation from the 3D transformation of the environment
                        cFigure::Point o = canvasProjection->computeCanvasPoint(transmissionStart);
                        cFigure::Point x = canvasProjection->computeCanvasPoint(transmissionStart + Coord(1, 0, 0));
                        cFigure::Point y = canvasProjection->computeCanvasPoint(transmissionStart + Coord(0, 1, 0));
                        double t1 = o.x;
                        double t2 = o.y;
                        double a = x.x - t1;
                        double b = x.y - t2;
                        double c = y.x - t1;
                        double d = y.y - t2;
                        communicationFigure->setTransform(cFigure::Transform(a, b, c, d, t1, t2));
                        communicationFigure->setBounds(cFigure::Rectangle(-startRadius, -startRadius, startRadius * 2, startRadius * 2));
                        communicationFigure->setInnerRx(endRadius);
                        communicationFigure->setInnerRy(endRadius);
                        break;
                    }
                    case SIGNAL_SHAPE_SPHERE: {
                        // a sphere looks like a circle from any view angle
                        cFigure::Point center = canvasProjection->computeCanvasPoint(transmissionStart);
                        communicationFigure->setBounds(cFigure::Rectangle(center.x - startRadius, center.y - startRadius, 2 * startRadius, 2 * startRadius));
                        communicationFigure->setInnerRx(endRadius);
                        communicationFigure->setInnerRy(endRadius);
                        break;
                    }
                    default:
                        throw cRuntimeError("Unimplemented signal shape");
                }
            }
        }
    }
}

void MediumCanvasVisualizer::scheduleSignalPropagationUpdateTimer()
{
    if (signalPropagationUpdateTimer->isScheduled())
        cancelEvent(signalPropagationUpdateTimer);
    simtime_t earliestUpdateTime = SimTime::getMaxTime();
    for (auto transmission : transmissions) {
        simtime_t nextSignalPropagationUpdateTime = getNextSignalPropagationUpdateTime(transmission);
        if (nextSignalPropagationUpdateTime < earliestUpdateTime)
            earliestUpdateTime = nextSignalPropagationUpdateTime;
    }
    if (earliestUpdateTime != SimTime::getMaxTime()) {
        scheduleAt(earliestUpdateTime, signalPropagationUpdateTimer);
    }
}

} // namespace visualizer

} // namespace inet

