//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/common/packetlevel/MediumVisualizer.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"

namespace inet {

namespace physicallayer {

Define_Module(MediumVisualizer);

MediumVisualizer::MediumVisualizer() :
    radioMedium(nullptr),
    displayCommunication(false),
    drawCommunication2D(false),
    leaveCommunicationTrail(false),
#if OMNETPP_VERSION >= 0x500
    leaveCommunicationHeat(false),
#endif
    communicationHeatMapSize(100),
    updateCanvasInterval(NaN),
    updateCanvasTimer(nullptr),
    communicationLayer(nullptr),
    communicationTrail(nullptr)
#if OMNETPP_VERSION >= 0x500
    ,communicationHeat(nullptr)
#endif
{
}

MediumVisualizer::~MediumVisualizer()
{
    cancelAndDelete(updateCanvasTimer);
}

void MediumVisualizer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium = check_and_cast<RadioMedium *>(getParentModule());
        displayCommunication = par("displayCommunication");
        drawCommunication2D = par("drawCommunication2D");
        cCanvas *canvas = radioMedium->getParentModule()->getCanvas();
        if (displayCommunication) {
            communicationLayer = new cGroupFigure("communication");
            canvas->addFigureBelow(communicationLayer, canvas->getSubmodulesLayer());
        }
        leaveCommunicationTrail = par("leaveCommunicationTrail");
        if (leaveCommunicationTrail) {
            communicationTrail = new TrailFigure(100, true, "communication trail");
            canvas->addFigureBelow(communicationTrail, canvas->getSubmodulesLayer());
        }
#if OMNETPP_VERSION >= 0x500
        leaveCommunicationHeat = par("leaveCommunicationHeat");
        if (leaveCommunicationHeat) {
            communicationHeat = new HeatMapFigure(communicationHeatMapSize, "communication heat");
            communicationHeat->setTags("successful_reception heat");
            canvas->addFigure(communicationHeat, 0);
        }
#endif
        updateCanvasInterval = par("updateCanvasInterval");
        updateCanvasTimer = new cMessage("updateCanvas");
    }
    else if (stage == INITSTAGE_LAST) {
#if OMNETPP_VERSION >= 0x500
        if (communicationHeat != nullptr) {
            const IMediumLimitCache *mediumLimitCache = radioMedium->getMediumLimitCache();
            const IPhysicalEnvironment *physicalEnvironment = radioMedium->getPhysicalEnvironment();
            Coord min = mediumLimitCache->getMinConstraintArea();
            Coord max = mediumLimitCache->getMaxConstraintArea();
            cFigure::Point o = physicalEnvironment->computeCanvasPoint(Coord::ZERO);
            cFigure::Point x = physicalEnvironment->computeCanvasPoint(Coord(1, 0, 0));
            cFigure::Point y = physicalEnvironment->computeCanvasPoint(Coord(0, 1, 0));
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
#endif
    }
}


void MediumVisualizer::handleMessage(cMessage *message)
{
    if (message == updateCanvasTimer) {
        updateCanvas();
        scheduleUpdateCanvasTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void MediumVisualizer::addTransmission(const ITransmission *transmission)
{
    if (displayCommunication) {
        transmissions.push_back(transmission);
        const IPhysicalEnvironment *physicalEnvironment = radioMedium->getPhysicalEnvironment();
        ICommunicationCache *communicationCache = const_cast<ICommunicationCache *>(radioMedium->getCommunicationCache());
        cFigure::Point position = physicalEnvironment->computeCanvasPoint(transmission->getStartPosition());
        cGroupFigure *groupFigure = new cGroupFigure();
#if OMNETPP_VERSION >= 0x500
        cFigure::Color color = cFigure::GOOD_DARK_COLORS[transmission->getId() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
        cRingFigure *communicationFigure = new cRingFigure();
#else
        cFigure::Color color(64 + rand() % 64, 64 + rand() % 64, 64 + rand() % 64);
        cOvalFigure *communicationFigure = new cOvalFigure();
#endif
        communicationFigure->setTags("ongoing_transmission");
        communicationFigure->setBounds(cFigure::Rectangle(position.x, position.y, 0, 0));
        communicationFigure->setFillColor(color);
        communicationFigure->setLineWidth(1);
        communicationFigure->setLineColor(cFigure::BLACK);
        groupFigure->addFigure(communicationFigure);
#if OMNETPP_VERSION >= 0x500
        communicationFigure->setFilled(true);
        communicationFigure->setFillOpacity(0.5);
        communicationFigure->setLineOpacity(0.5);
        communicationFigure->setZoomLineWidth(false);
        cLabelFigure *nameFigure = new cLabelFigure();
        nameFigure->setPosition(position);
#else
        cTextFigure *nameFigure = new cTextFigure();
        nameFigure->setLocation(position);
#endif
        nameFigure->setTags("ongoing_transmission packet_name label");
        nameFigure->setText(transmission->getMacFrame()->getName());
        nameFigure->setColor(color);
        groupFigure->addFigure(nameFigure);
        communicationLayer->addFigure(groupFigure);
        communicationCache->setCachedFigure(transmission, groupFigure);
        updateCanvas();
        if (updateCanvasInterval > 0 && !updateCanvasTimer->isScheduled())
            scheduleUpdateCanvasTimer();
    }
}

void MediumVisualizer::removeTransmission(const ITransmission *transmission)
{
    if (displayCommunication) {
        transmissions.erase(std::remove(transmissions.begin(), transmissions.end(), transmission));
        ICommunicationCache *communicationCache = const_cast<ICommunicationCache *>(radioMedium->getCommunicationCache());
        cFigure *figure = communicationCache->getCachedFigure(transmission);
        if (figure != nullptr)
            delete communicationLayer->removeFigure(figure);
    }
}

void MediumVisualizer::receivePacket(const IReceptionResult *result)
{
    bool isReceptionSuccessful = true;
    for (auto decision : *result->getDecisions())
        isReceptionSuccessful &= decision->isReceptionSuccessful();
    if (isReceptionSuccessful) {
        const ITransmission *transmission = result->getReception()->getTransmission();
        const IReception *reception = result->getReception();
        if (leaveCommunicationTrail) {
            const IPhysicalEnvironment *physicalEnvironment = radioMedium->getPhysicalEnvironment();
            cLineFigure *communicationFigure = new cLineFigure();
            communicationFigure->setTags("successful_reception recent_history");
            cFigure::Point start = physicalEnvironment->computeCanvasPoint(transmission->getStartPosition());
            cFigure::Point end = physicalEnvironment->computeCanvasPoint(reception->getStartPosition());
            communicationFigure->setStart(start);
            communicationFigure->setEnd(end);
            communicationFigure->setLineColor(cFigure::BLUE);
#if OMNETPP_VERSION >= 0x500 && OMNETPP_BUILDNUM >= 1006
            communicationFigure->setEndArrowhead(cFigure::ARROW_BARBED);
#else
            communicationFigure->setEndArrowHead(cFigure::ARROW_BARBED);
#endif
            communicationFigure->setLineWidth(1);
#if OMNETPP_VERSION >= 0x500
            communicationFigure->setZoomLineWidth(false);
#endif
            communicationTrail->addFigure(communicationFigure);
        }
#if OMNETPP_VERSION >= 0x500
        if (leaveCommunicationHeat) {
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
#endif
    }
    if (displayCommunication)
        updateCanvas();
}

void MediumVisualizer::mediumChanged() const
{
    if (displayCommunication)
        updateCanvas();
}

void MediumVisualizer::updateCanvas() const
{
    const IPropagation *propagation = radioMedium->getPropagation();
    const IPhysicalEnvironment *physicalEnvironment = radioMedium->getPhysicalEnvironment();
    ICommunicationCache *communicationCache = const_cast<ICommunicationCache *>(radioMedium->getCommunicationCache());
#if OMNETPP_VERSION >= 0x500
    if (communicationHeat != nullptr)
        communicationHeat->coolDown();
#endif
    for (const auto transmission : transmissions) {
        cFigure *groupFigure = communicationCache->getCachedFigure(transmission);
        if (groupFigure) {
#if OMNETPP_VERSION >= 0x500
            cRingFigure *communicationFigure = (cRingFigure *)groupFigure->getFigure(0);
#else
            cOvalFigure *communicationFigure = (cOvalFigure *)groupFigure->getFigure(0);
#endif
            const Coord transmissionStart = transmission->getStartPosition();
            double startRadius = propagation->getPropagationSpeed().get() * (simTime() - transmission->getStartTime()).dbl();
            double endRadius = propagation->getPropagationSpeed().get() * (simTime() - transmission->getEndTime()).dbl();
            if (endRadius < 0) endRadius = 0;
            // KLUDGE: to workaround overflow bugs in drawing
            if (startRadius > 10000)
                startRadius = 10000;
            if (endRadius > 10000)
                endRadius = 10000;
#if OMNETPP_VERSION >= 0x500
            if (drawCommunication2D) {
                // determine the rotated 2D canvas points by computing the 2D affine trasnformation from the 3D transformation of the environment
                cFigure::Point o = physicalEnvironment->computeCanvasPoint(transmissionStart);
                cFigure::Point x = physicalEnvironment->computeCanvasPoint(transmissionStart + Coord(1, 0, 0));
                cFigure::Point y = physicalEnvironment->computeCanvasPoint(transmissionStart + Coord(0, 1, 0));
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
            }
            else {
#else
            {
#endif
                // a sphere looks like a circle from any view angle
                cFigure::Point center = physicalEnvironment->computeCanvasPoint(transmissionStart);
                communicationFigure->setBounds(cFigure::Rectangle(center.x - startRadius, center.y - startRadius, 2 * startRadius, 2 * startRadius));
#if OMNETPP_VERSION >= 0x500
                communicationFigure->setInnerRx(endRadius);
                communicationFigure->setInnerRy(endRadius);
#endif
            }
        }
    }
}

void MediumVisualizer::scheduleUpdateCanvasTimer()
{
    simtime_t now = simTime();
    ICommunicationCache *communicationCache = const_cast<ICommunicationCache *>(radioMedium->getCommunicationCache());
    const IMediumLimitCache *mediumLimitCache = radioMedium->getMediumLimitCache();
    for (const auto transmission : transmissions) {
        const simtime_t startTime = transmission->getStartTime();
        const simtime_t endTime = transmission->getEndTime();
        simtime_t maxPropagationTime = communicationCache->getCachedInterferenceEndTime(transmission) - endTime - mediumLimitCache->getMaxTransmissionDuration();
        if ((startTime <= now && now <= startTime + maxPropagationTime) ||
            (endTime <= now && now <= endTime + maxPropagationTime))
        {
            scheduleAt(simTime() + updateCanvasInterval, updateCanvasTimer);
            return;
        }
    }
    simtime_t nextEndTime = -1;
    for (const auto transmission : transmissions) {
        const simtime_t endTime = transmission->getEndTime();
        if (endTime > now && (nextEndTime == -1 || endTime < nextEndTime))
            nextEndTime = endTime;
    }
    if (nextEndTime > now)
        scheduleAt(nextEndTime, updateCanvasTimer);
}

} // namespace physicallayer

} // namespace inet

