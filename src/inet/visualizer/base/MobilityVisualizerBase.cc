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
    if (displayMovements)
        unsubscribe();
}

void MobilityVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayMovements = par("displayMovements");
        animationSpeed = par("animationSpeed");
        moduleFilter.setPattern(par("moduleFilter"));
        // orientation
        displayOrientations = par("displayOrientations");
        orientationArcSize = par("orientationArcSize");
        orientationLineColor = cFigure::parseColor(par("orientationLineColor"));
        orientationLineStyle = cFigure::parseLineStyle(par("orientationLineStyle"));
        orientationLineWidth = par("orientationLineWidth");
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
        if (displayMovements)
            subscribe();
    }
}

void MobilityVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "moduleFilter"))
            moduleFilter.setPattern(par("moduleFilter"));
        // TODO:
    }
}

void MobilityVisualizerBase::subscribe()
{
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
    subscriptionModule->subscribe(IMobility::mobilityStateChangedSignal, this);
}

void MobilityVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this, false);
    if (subscriptionModule != nullptr)
        subscriptionModule->unsubscribe(IMobility::mobilityStateChangedSignal, this);
}

} // namespace visualizer

} // namespace inet

