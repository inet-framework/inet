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

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/mobility/MobilityCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(MobilityCanvasVisualizer);

MobilityCanvasVisualizer::MobilityCanvasVisualization::MobilityCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cModule *visualRepresentation, cArcFigure *orientationFigure, cLineFigure *veloctiyFigure, TrailFigure *trailFigure) :
    networkNodeVisualization(networkNodeVisualization),
    visualRepresentation(visualRepresentation),
    orientationFigure(orientationFigure),
    veloctiyFigure(veloctiyFigure),
    trailFigure(trailFigure)
{
}

MobilityCanvasVisualizer::~MobilityCanvasVisualizer()
{
    for (auto mobilityVisualization : mobilityVisualizations)
        delete mobilityVisualization.second;
}

void MobilityCanvasVisualizer::initialize(int stage)
{
    MobilityVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        zIndex = par("zIndex");
        canvasProjection = CanvasProjection::getCanvasProjection(visualizerTargetModule->getCanvas());
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

cModule *MobilityCanvasVisualizer::findVisualRepresentation(cModule *module) const
{
    while (module != nullptr && module->getParentModule() != visualizerTargetModule)
        module = module->getParentModule();
    return module;
}

MobilityCanvasVisualizer::MobilityCanvasVisualization *MobilityCanvasVisualizer::getMobilityVisualization(const IMobility *mobility) const
{
    auto it = mobilityVisualizations.find(mobility);
    if (it == mobilityVisualizations.end())
        return nullptr;
    else
        return it->second;
}

void MobilityCanvasVisualizer::setMobilityVisualization(const IMobility *mobility, MobilityCanvasVisualization *entry)
{
    mobilityVisualizations[mobility] = entry;
}

void MobilityCanvasVisualizer::removeMobilityVisualization(const IMobility *mobility)
{
    mobilityVisualizations.erase(mobility);
}

MobilityCanvasVisualizer::MobilityCanvasVisualization* MobilityCanvasVisualizer::ensureMobilityVisualization(const IMobility *mobility)
{
    auto mobilityVisualization = getMobilityVisualization(mobility);
    if (mobilityVisualization == nullptr) {
        auto canvas = visualizerTargetModule->getCanvas();
        auto module = const_cast<cModule *>(dynamic_cast<const cModule *>(mobility));
        auto visualRepresentation = findVisualRepresentation(module);
        auto visualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(module));
        cArcFigure *orientationFigure = nullptr;
        if (displayOrientation) {
            int size = 32;
            orientationFigure = new cArcFigure();
            orientationFigure->setBounds(cFigure::Rectangle(-size, -size, 2 * size, 2 * size));
            orientationFigure->setLineColor(orientationLineColor);
            orientationFigure->setLineWidth(orientationLineWidth);
            visualization->addFigure(orientationFigure);
        }
        cLineFigure *velocityFigure = nullptr;
        if (displayVelocity) {
            velocityFigure = new cLineFigure();
            velocityFigure->setZIndex(zIndex);
            velocityFigure->setVisible(false);
            velocityFigure->setEndArrowhead(cFigure::ARROW_BARBED);
            velocityFigure->setLineColor(velocityLineColor);
            velocityFigure->setLineWidth(velocityLineWidth);
            velocityFigure->setLineStyle(velocityLineStyle);
            visualization->addFigure(velocityFigure);
        }
        TrailFigure *trailFigure = nullptr;
        if (displayMovementTrail) {
            trailFigure = new TrailFigure(trailLength, true, "movement trail");
            trailFigure->setZIndex(zIndex);
            canvas->addFigure(trailFigure);
        }
        mobilityVisualization = new MobilityCanvasVisualization(visualization, visualRepresentation, orientationFigure, velocityFigure, trailFigure);
        setMobilityVisualization(mobility, mobilityVisualization);
    }
    return mobilityVisualization;
}

void MobilityCanvasVisualizer::extendMovementTrail(const IMobility *mobility, TrailFigure *trailFigure, cFigure::Point position)
{
    cFigure::Point startPosition;
    cFigure::Point endPosition = position;
    if (trailFigure->getNumFigures() == 0)
        startPosition = position;
    else
        startPosition = static_cast<cLineFigure*>(trailFigure->getFigure(trailFigure->getNumFigures() - 1))->getEnd();
    double dx = startPosition.x - endPosition.x;
    double dy = startPosition.y - endPosition.y;
    // TODO: 1?
    if (trailFigure->getNumFigures() == 0 || dx * dx + dy * dy > 1) {
        cLineFigure *movementLine = new cLineFigure("movementTrail");
        movementLine->setTags("movement_trail recent_history");
        movementLine->setStart(startPosition);
        movementLine->setEnd(endPosition);
        movementLine->setLineWidth(1);
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
        cFigure::Color color = autoMovementTrailLineColor ? cFigure::GOOD_DARK_COLORS[module->getId() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))] : movementTrailLineColor;
        movementLine->setLineColor(color);
        movementLine->setZoomLineWidth(false);
        trailFigure->addFigure(movementLine);
    }
}

void MobilityCanvasVisualizer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if (!hasGUI()) return;
    if (signal == IMobility::mobilityStateChangedSignal) {
        auto mobility = dynamic_cast<IMobility *>(object);
        auto position = canvasProjection->computeCanvasPoint(mobility->getCurrentPosition());
        auto orientation = mobility->getCurrentAngularPosition();
        auto speed = canvasProjection->computeCanvasPoint(mobility->getCurrentSpeed());
        auto entry = ensureMobilityVisualization(mobility);
        entry->networkNodeVisualization->setTransform(cFigure::Transform().translate(position.x, position.y));
        if (entry->visualRepresentation != nullptr)
            setPosition(entry->visualRepresentation, position);
        if (displayOrientation) {
            // TODO: this doesn't correctly take canvas projetion into account
            double angle = orientation.alpha;
            entry->orientationFigure->setStartAngle(angle - M_PI / 4);
            entry->orientationFigure->setEndAngle(angle + M_PI / 4);
        }
        if (displayVelocity) {
            auto end = speed;
            auto length = end.getLength();
            entry->veloctiyFigure->setEnd(end / length * 100);
            entry->veloctiyFigure->setVisible(length != 0);
        }
        if (displayMovementTrail)
            extendMovementTrail(mobility, entry->trailFigure, position);
    }
}

void MobilityCanvasVisualizer::setPosition(cModule* visualRepresentation, cFigure::Point position)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%lf", position.x);
    buf[sizeof(buf) - 1] = 0;
    visualRepresentation->getDisplayString().setTagArg("p", 0, buf);
    snprintf(buf, sizeof(buf), "%lf", position.y);
    buf[sizeof(buf) - 1] = 0;
    visualRepresentation->getDisplayString().setTagArg("p", 1, buf);
}

} // namespace visualizer

} // namespace inet

