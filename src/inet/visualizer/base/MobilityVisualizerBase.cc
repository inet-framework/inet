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

#include "inet/visualizer/base/MobilityVisualizerBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

MobilityVisualizerBase::MobilityVisualization::MobilityVisualization(IMobility *mobility) :
    mobility(mobility)
{
}

void MobilityVisualizerBase::preDelete(cComponent *root)
{
    if (displayMobility) {
        unsubscribe();
        removeAllMobilityVisualizations();
    }
}

void MobilityVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayMobility = par("displayMobility");
        animationSpeed = par("animationSpeed");
        moduleFilter.setPattern(par("moduleFilter"));
        // position
        displayPositions = par("displayPositions");
        positionCircleRadius = par("positionCircleRadius");
        positionCircleLineWidth = par("positionCircleLineWidth");
        positionCircleLineColorSet.parseColors(par("positionCircleLineColor"));
        positionCircleFillColorSet.parseColors(par("positionCircleFillColor"));
        // orientation
        displayOrientations = par("displayOrientations");
        orientationPieRadius = par("orientationPieRadius");
        orientationPieSize = par("orientationPieSize");
        orientationPieOpacity = par("orientationPieOpacity");
        orientationLineColor = cFigure::parseColor(par("orientationLineColor"));
        orientationLineStyle = cFigure::parseLineStyle(par("orientationLineStyle"));
        orientationLineWidth = par("orientationLineWidth");
        orientationFillColor = cFigure::parseColor(par("orientationFillColor"));
        // velocity
        displayVelocities = par("displayVelocities");
        velocityArrowScale = par("velocityArrowScale");
        velocityLineColor = cFigure::parseColor(par("velocityLineColor"));
        velocityLineStyle = cFigure::parseLineStyle(par("velocityLineStyle"));
        velocityLineWidth = par("velocityLineWidth");
        // movement trail
        displayMovementTrails = par("displayMovementTrails");
        movementTrailLineColorSet.parseColors(par("movementTrailLineColor"));
        movementTrailLineStyle = cFigure::parseLineStyle(par("movementTrailLineStyle"));
        movementTrailLineWidth = par("movementTrailLineWidth");
        trailLength = par("trailLength");
        if (displayMobility)
            subscribe();
    }
}

void MobilityVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr) {
        if (!strcmp(name, "moduleFilter"))
            moduleFilter.setPattern(par("moduleFilter"));
        // TODO
    }
}

void MobilityVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(IMobility::mobilityStateChangedSignal, this);
    visualizationSubjectModule->subscribe(PRE_MODEL_CHANGE, this);
}

void MobilityVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(IMobility::mobilityStateChangedSignal, this);
        visualizationSubjectModule->unsubscribe(PRE_MODEL_CHANGE, this);
    }
}

MobilityVisualizerBase::MobilityVisualization *MobilityVisualizerBase::getMobilityVisualization(const IMobility *mobility) const
{
    auto it = mobilityVisualizations.find(mobility);
    if (it == mobilityVisualizations.end())
        return nullptr;
    else
        return it->second;
}

void MobilityVisualizerBase::addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization)
{
    mobilityVisualizations[mobility] = mobilityVisualization;
}

void MobilityVisualizerBase::removeMobilityVisualization(const MobilityVisualization *mobilityVisualization)
{
    mobilityVisualizations.erase(mobilityVisualization->mobility);
}

void MobilityVisualizerBase::removeAllMobilityVisualizations()
{
    std::vector<const MobilityVisualization *> removedMobilityVisualizations;
    for (auto it : mobilityVisualizations)
        removedMobilityVisualizations.push_back(it.second);
    for (auto mobilityVisualization : removedMobilityVisualizations) {
        removeMobilityVisualization(mobilityVisualization);
        delete mobilityVisualization;
    }
}

void MobilityVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("receiveSignal");
    if (signal == IMobility::mobilityStateChangedSignal) {
        if (moduleFilter.matches(check_and_cast<cModule *>(source))) {
            auto mobility = dynamic_cast<IMobility *>(source);
            auto mobilityVisualization = getMobilityVisualization(mobility);
            if (mobilityVisualization == nullptr) {
                mobilityVisualization = createMobilityVisualization(mobility);
                addMobilityVisualization(mobility, mobilityVisualization);
            }
        }
    }
    else if (signal == PRE_MODEL_CHANGE) {
        if (dynamic_cast<cPreModuleDeleteNotification *>(object)) {
            if (auto mobility = dynamic_cast<IMobility *>(source)) {
                auto mobilityVisualization = getMobilityVisualization(mobility);
                removeMobilityVisualization(mobilityVisualization);
                delete mobilityVisualization;
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

