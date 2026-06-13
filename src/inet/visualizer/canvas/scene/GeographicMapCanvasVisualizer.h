//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOGRAPHICMAPCANVASVISUALIZER_H
#define __INET_GEOGRAPHICMAPCANVASVISUALIZER_H

#include "inet/common/figures/TrailFigure.h"
#include "inet/visualizer/base/GeographicMapVisualizerBase.h"

namespace inet {

namespace visualizer {

/**
 * Projects geographically located network nodes onto a 2D world map drawn on the
 * OMNeT++ canvas. The map background is the target module's background image
 * (sized by the map area), optionally overlaid with a latitude/longitude
 * graticule. Each tracked node is drawn as a marker (image or circle) at its
 * sub-satellite point, optionally with a name label, a trailing ground track,
 * and a coverage footprint.
 *
 * @see ~GeographicMapVisualizerBase
 */
class INET_API GeographicMapCanvasVisualizer : public GeographicMapVisualizerBase
{
  protected:
    class INET_API MapNodeCanvasVisualization : public MapNodeVisualization {
      public:
        cFigure *markerFigure = nullptr; // cImageFigure or cOvalFigure
        cLabelFigure *labelFigure = nullptr;
        TrailFigure *groundTrackFigure = nullptr;
        cGroupFigure *footprintFigure = nullptr; // holds one or more polygon pieces (the footprint can be split across the seam)
        bool hasLastPoint = false;
        cFigure::Point lastPoint;

      public:
        MapNodeCanvasVisualization(IMobility *mobility) : MapNodeVisualization(mobility) {}
        virtual ~MapNodeCanvasVisualization();
    };

  protected:
    cGroupFigure *mapFigure = nullptr;
    cLabelFigure *clockFigure = nullptr; // wall-clock in the lower-left corner, null if no epoch is configured
    double clockBaseSeconds = 0; // absolute UTC seconds (since 1970-01-01) corresponding to simulation time 0

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void createBackgroundFigure();
    // Creates the lower-left wall-clock figure from the 'epoch' parameter of the SatelliteController
    // named by the 'satelliteController' parameter; does nothing when none is configured.
    virtual void createClockFigure();
    // Projects a geographic point to map pixels. When normalizeLongitude is false the
    // longitude is used as given (possibly outside [-180, 180]) so that a sequence of
    // points stays continuous across the antimeridian instead of wrapping.
    virtual cFigure::Point geoToCanvas(double latitudeDeg, double longitudeDeg, bool normalizeLongitude = true) const;
    // Rebuilds the coverage footprint (a small circle on the Earth around the sub-satellite
    // point) as one or more polygon pieces, clipped to the map and tiled across the seam so
    // that the part running off one side reappears on the other; closes across the top or
    // bottom edge when the footprint encloses a pole.
    virtual void refreshFootprint(MapNodeCanvasVisualization *visualization, double latitudeDeg, double longitudeDeg, double altitude) const;
    virtual void addFootprintPolygon(cGroupFigure *group, const std::vector<cFigure::Point>& points) const;
    // Adds one styled ground-track segment between two map points to the node's trail.
    virtual void addGroundTrackSegment(MapNodeCanvasVisualization *visualization, const cFigure::Point& from, const cFigure::Point& to, cModule *module) const;

    virtual MapNodeVisualization *createMapNodeVisualization(IMobility *mobility) override;
    virtual void addMapNodeVisualization(const IMobility *mobility, MapNodeVisualization *visualization) override;
    virtual void removeMapNodeVisualization(const MapNodeVisualization *visualization) override;

    virtual const char *getNodeName(const IMobility *mobility) const;
};

} // namespace visualizer

} // namespace inet

#endif
