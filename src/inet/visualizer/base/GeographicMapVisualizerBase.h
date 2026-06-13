//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOGRAPHICMAPVISUALIZERBASE_H
#define __INET_GEOGRAPHICMAPVISUALIZERBASE_H

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/ModuleFilter.h"

namespace inet {

namespace visualizer {

/**
 * Base class for visualizers that project geographically located network nodes
 * (satellites, ground stations, anything carrying an ~IMobility) onto a 2D world
 * map. It discovers and tracks the relevant mobility modules using the same
 * mechanism as ~MobilityVisualizerBase: it subscribes to the mobility state
 * change signal on the subject module subtree, lazily creates a per-node
 * visualization on the first state change (so dynamically inserted satellites are
 * picked up), and removes it when the module is deleted.
 *
 * Each node's scene position is converted to a geographic (latitude/longitude/
 * altitude) coordinate through an ~IGeographicCoordinateSystem, which is then
 * projected to map pixels by the concrete (canvas) subclass. The actual drawing
 * is done in derived modules.
 *
 * @see ~GeographicMapCanvasVisualizer, ~IGeographicMapVisualizer
 */
class INET_API GeographicMapVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API MapNodeVisualization {
      public:
        IMobility *mobility = nullptr;

      public:
        MapNodeVisualization(IMobility *mobility) : mobility(mobility) {}
        virtual ~MapNodeVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayMap = false;
    double animationSpeed = NaN;
    double zIndex = NaN;
    ModuleFilter moduleFilter;
    const IGeographicCoordinateSystem *coordinateSystem = nullptr;

    // map background and projection
    bool mercator = false; // false: equirectangular (plate carree)
    std::string mapImage;
    double mapWidth = NaN;
    double mapHeight = NaN;
    double longitudeOffset = NaN; // degrees added to every longitude before projecting (horizontal calibration)
    bool adjustBackgroundBox = false;

    // graticule (latitude/longitude grid)
    bool displayGraticule = false;
    double graticuleLatSpacing = NaN;
    double graticuleLonSpacing = NaN;
    cFigure::Color graticuleLineColor;
    double graticuleLineWidth = NaN;

    // node markers and labels
    std::string markerImage;
    double markerRadius = NaN;
    ColorSet markerColorSet;
    bool displayLabels = false;
    cFigure::Color labelColor;

    // orbital ground tracks
    bool displayGroundTracks = false;
    int groundTrackLength = -1;
    ColorSet groundTrackLineColorSet;
    cFigure::LineStyle groundTrackLineStyle;
    double groundTrackLineWidth = NaN;

    // coverage footprints
    bool displayFootprints = false;
    double minElevationAngle = NaN; // degrees
    cFigure::Color footprintLineColor;
    cFigure::Color footprintFillColor;
    double footprintOpacity = NaN;
    //@}

    std::map<int, MapNodeVisualization *> mapNodeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual MapNodeVisualization *createMapNodeVisualization(IMobility *mobility) = 0;
    virtual MapNodeVisualization *getMapNodeVisualization(const IMobility *mobility) const;
    virtual void addMapNodeVisualization(const IMobility *mobility, MapNodeVisualization *visualization);
    virtual void removeMapNodeVisualization(const MapNodeVisualization *visualization);
    virtual void removeAllMapNodeVisualizations();

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif
