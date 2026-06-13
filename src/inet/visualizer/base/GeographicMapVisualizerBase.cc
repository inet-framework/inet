//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/GeographicMapVisualizerBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

void GeographicMapVisualizerBase::preDelete(cComponent *root)
{
    if (displayMap) {
        unsubscribe();
        removeAllMapNodeVisualizations();
    }
}

void GeographicMapVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayMap = par("displayMap");
        animationSpeed = par("animationSpeed");
        zIndex = par("zIndex");
        moduleFilter.setPattern(par("moduleFilter"));
        coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
        if (coordinateSystem == nullptr)
            throw cRuntimeError("Geographic coordinate system module not found on path '%s' defined by par 'coordinateSystemModule'", par("coordinateSystemModule").stringValue());
        // map background and projection
        mercator = !strcmp(par("projection"), "mercator");
        mapImage = par("mapImage").stringValue();
        mapWidth = par("mapWidth");
        mapHeight = par("mapHeight");
        longitudeOffset = deg(par("longitudeOffset")).get<deg>();
        adjustBackgroundBox = par("adjustBackgroundBox");
        // graticule
        displayGraticule = par("displayGraticule");
        graticuleLatSpacing = deg(par("graticuleLatSpacing")).get<deg>();
        graticuleLonSpacing = deg(par("graticuleLonSpacing")).get<deg>();
        graticuleLineColor = cFigure::parseColor(par("graticuleLineColor"));
        graticuleLineWidth = par("graticuleLineWidth");
        // markers and labels
        markerImage = par("markerImage").stringValue();
        markerRadius = par("markerRadius");
        markerColorSet.parseColors(par("markerColor"));
        displayLabels = par("displayLabels");
        labelColor = cFigure::parseColor(par("labelColor"));
        // ground tracks
        displayGroundTracks = par("displayGroundTracks");
        groundTrackLength = par("groundTrackLength");
        groundTrackLineColorSet.parseColors(par("groundTrackLineColor"));
        groundTrackLineStyle = cFigure::parseLineStyle(par("groundTrackLineStyle"));
        groundTrackLineWidth = par("groundTrackLineWidth");
        // footprints
        displayFootprints = par("displayFootprints");
        minElevationAngle = deg(par("minElevationAngle")).get<deg>();
        footprintLineColor = cFigure::parseColor(par("footprintLineColor"));
        footprintFillColor = cFigure::parseColor(par("footprintFillColor"));
        footprintOpacity = par("footprintOpacity");
        if (displayMap)
            subscribe();
    }
}

void GeographicMapVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "moduleFilter"))
        moduleFilter.setPattern(par("moduleFilter"));
}

void GeographicMapVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(IMobility::mobilityStateChangedSignal, this);
    visualizationSubjectModule->subscribe(PRE_MODEL_CHANGE, this);
}

void GeographicMapVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(IMobility::mobilityStateChangedSignal, this);
        visualizationSubjectModule->unsubscribe(PRE_MODEL_CHANGE, this);
    }
}

GeographicMapVisualizerBase::MapNodeVisualization *GeographicMapVisualizerBase::getMapNodeVisualization(const IMobility *mobility) const
{
    auto it = mapNodeVisualizations.find(mobility->getId());
    return (it == mapNodeVisualizations.end()) ? nullptr : it->second;
}

void GeographicMapVisualizerBase::addMapNodeVisualization(const IMobility *mobility, MapNodeVisualization *visualization)
{
    mapNodeVisualizations[mobility->getId()] = visualization;
}

void GeographicMapVisualizerBase::removeMapNodeVisualization(const MapNodeVisualization *visualization)
{
    mapNodeVisualizations.erase(visualization->mobility->getId());
}

void GeographicMapVisualizerBase::removeAllMapNodeVisualizations()
{
    std::vector<const MapNodeVisualization *> removedVisualizations;
    for (auto it : mapNodeVisualizations)
        removedVisualizations.push_back(it.second);
    for (auto visualization : removedVisualizations) {
        removeMapNodeVisualization(visualization);
        delete visualization;
    }
}

void GeographicMapVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == IMobility::mobilityStateChangedSignal) {
        if (moduleFilter.matches(check_and_cast<cModule *>(source))) {
            auto mobility = dynamic_cast<IMobility *>(source);
            auto visualization = getMapNodeVisualization(mobility);
            if (visualization == nullptr) {
                visualization = createMapNodeVisualization(mobility);
                addMapNodeVisualization(mobility, visualization);
            }
        }
    }
    else if (signal == PRE_MODEL_CHANGE) {
        if (dynamic_cast<cPreModuleDeleteNotification *>(object)) {
            if (auto mobility = dynamic_cast<IMobility *>(source)) {
                auto visualization = getMapNodeVisualization(mobility);
                if (visualization != nullptr) {
                    removeMapNodeVisualization(visualization);
                    delete visualization;
                }
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet
