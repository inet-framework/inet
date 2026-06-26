//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOHORIZONCANVASVISUALIZER_H
#define __INET_GEOHORIZONCANVASVISUALIZER_H

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/common/geometry/common/EquirectangularProjection.h"
#include "inet/visualizer/util/ModuleFilter.h"

namespace inet {

namespace visualizer {

/**
 * Draws the satellite visibility footprint of the tracked nodes on top of an
 * equirectangular map (e.g. a ~GeoMapCanvasVisualizer): the area on the Earth
 * from which the node is seen above a configurable elevation mask. It discovers and
 * tracks mobility modules like ~MobilityVisualizerBase (subscribing to the mobility
 * state change signal and lazily creating a per-node visualization), and rebuilds
 * the footprint polygons every refresh from the node's current position.
 *
 * The footprint is computed in the geocentric ECEF frame, so the coordinate system
 * referenced by the map visualizer should be a ~Wgs84EcefGeographicCoordinateSystem
 * (as used by ~SatelliteMobility). The map window and the coordinate system are taken
 * from the referenced map visualizer (the mapVisualizer parameter).
 */
class INET_API GeoHorizonCanvasVisualizer : public VisualizerBase, public cListener
{
  protected:
    class INET_API NodeVisualization {
      public:
        IMobility *mobility = nullptr;
        cGroupFigure *visibilityFigure = nullptr; // holds the visibility footprint polygon pieces
      public:
        NodeVisualization(IMobility *mobility) : mobility(mobility) {}
    };

  protected:
    /** @name Parameters */
    //@{
    double zIndex = 0;
    double animationSpeed = 1;
    ModuleFilter moduleFilter;
    const IGeographicCoordinateSystem *coordinateSystem = nullptr;
    EquirectangularProjection projection;

    bool displayVisibility = true;
    double elevationMaskRad = 0;
    int footprintSampleCount = 72;
    cFigure::Color visibilityFillColor, visibilityLineColor;
    double visibilityOpacity = 0.1;
    //@}

    std::map<int, NodeVisualization *> nodeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual NodeVisualization *getNodeVisualization(const IMobility *mobility) const;
    virtual NodeVisualization *createNodeVisualization(IMobility *mobility);
    virtual void removeNodeVisualization(const NodeVisualization *visualization);
    virtual void removeAllNodeVisualizations();

    virtual void refreshNode(NodeVisualization *visualization) const;
    // Rebuilds the polygon pieces of a group from a closed ring of geographic points, unwrapping the
    // longitude continuously around referenceLongitude and clipping/tiling to the map window.
    virtual void rebuildPolygons(cGroupFigure *group, const std::vector<GeoCoord>& ring, double referenceLongitudeDeg,
            const cFigure::Color& fillColor, const cFigure::Color& lineColor, double opacity) const;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;

    // Pure (GUI-independent) geometry: samples the satellite visibility footprint - the small circle on
    // the Earth (modeled as a sphere of WGS84 mean radius) within which the satellite, at the given
    // altitude above its sub-satellite point, is seen at or above the elevation mask. Returns sampleCount
    // points around the circle (altitude 0), or an empty vector when the altitude is too low to produce a
    // footprint at this elevation.
    static std::vector<GeoCoord> visibilityRing(const GeoCoord& subSatellitePoint, double altitude, double elevationMaskRad, int sampleCount);
};

} // namespace visualizer

} // namespace inet

#endif
