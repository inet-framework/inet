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
#include "inet/visualizer/base/MobilityVisualizerBase.h"

namespace inet {

namespace visualizer {

MobilityVisualizerBase::MobilityVisualization::MobilityVisualization(IMobility *mobility) :
    mobility(mobility)
{
}

MobilityVisualizerBase::~MobilityVisualizerBase()
{
    if (displayMobility)
        unsubscribe();
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
        // TODO:
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
    auto visualizationSubjectModule = getModuleFromPar<cModule>(par("visualizationSubjectModule"), this, false);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(IMobility::mobilityStateChangedSignal, this);
        visualizationSubjectModule->unsubscribe(PRE_MODEL_CHANGE, this);
    }
}

} // namespace visualizer

} // namespace inet

