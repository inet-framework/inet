//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOSKYVIEWCANVASVISUALIZER_H
#define __INET_GEOSKYVIEWCANVASVISUALIZER_H

#include <map>

#include "inet/common/figures/TrailFigure.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/ModuleFilter.h"

namespace inet {

namespace visualizer {

/**
 * Draws an azimuth/elevation "sky view" polar plot next to each observer node
 * (typically a ground station using StationaryMobility), showing where
 * the target nodes (typically satellites) currently are in the observer's sky.
 *
 * The plot is a circle: zenith (90 deg elevation) at the center, the horizon
 * (0 deg) at the outer rim; North up, East to the right (azimuth clockwise).
 * Concentric rings mark elevations, N/E/S/W cardinal lines and labels are drawn,
 * and the band below `elevationMask` is shown with a translucent mask. Each
 * visible target is a colored dot (optionally named, with a fading sky trail);
 * targets below the horizon are hidden.
 *
 * Look angles are computed with wgs84::computeLookAngles() from the observer's and
 * targets' geographic positions (via the geographic coordinate system), so the
 * visualizer has no satellite-specific dependency.
 */
class INET_API GeoSkyViewCanvasVisualizer : public VisualizerBase, public cListener
{
  protected:
    class INET_API TargetMark {
      public:
        IMobility *target = nullptr;
        cOvalFigure *dotFigure = nullptr; // child of the observer plot group (local coords)
        cLabelFigure *labelFigure = nullptr;
        TrailFigure *trailFigure = nullptr;
        bool hadPoint = false;
        cFigure::Point lastPoint;

      public:
        TargetMark(IMobility *target) : target(target) {}
        // figures are owned by (and deleted with) the observer plot group, not here
    };

    class INET_API ObserverPlot {
      public:
        IMobility *observer = nullptr;
        cGroupFigure *groupFigure = nullptr; // the whole plot; translated to the observer icon each refresh
        std::map<int, TargetMark *> targetMarks; // keyed by target module id

      public:
        ObserverPlot(IMobility *observer) : observer(observer) {}
        ~ObserverPlot();
    };

  protected:
    /** @name Parameters */
    //@{
    bool displaySkyView = false;
    double animationSpeed = NaN;
    double zIndex = NaN;
    const IGeographicCoordinateSystem *coordinateSystem = nullptr;
    ModuleFilter observerFilter;
    ModuleFilter targetFilter;

    double plotRadius = NaN;
    double plotOffsetX = NaN;
    double plotOffsetY = NaN;
    double elevationMask = NaN; // degrees

    cFigure::Color backgroundColor;
    double backgroundOpacity = NaN;
    cFigure::Color ringColor;
    double ringLineWidth = NaN;
    cFigure::Color maskColor;
    double maskOpacity = NaN;
    cFigure::Color cardinalColor;
    cFigure::Color labelColor;

    ColorSet targetColorSet;
    double targetRadius = NaN;
    double lowElevationOpacity = NaN;
    bool displayLabels = false;
    bool displayTrails = false;
    int trailLength = -1;
    cFigure::LineStyle trailLineStyle;
    double trailLineWidth = NaN;
    //@}

    std::map<int, ObserverPlot *> observerPlots; // keyed by observer module id
    std::map<int, IMobility *> targets; // keyed by target module id

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual ObserverPlot *addObserver(IMobility *observer);
    virtual void addTarget(IMobility *target);
    virtual void removeObserver(int id);
    virtual void removeTarget(int id);

    virtual void buildPlotDecorations(ObserverPlot *plot);
    virtual TargetMark *createTargetMark(ObserverPlot *plot, IMobility *target);
    virtual void removeTargetMark(ObserverPlot *plot, TargetMark *mark);

    virtual cFigure::Point elevationAzimuthToCanvas(double elevationDeg, double azimuthDeg) const;
    virtual std::string getTargetLabel(const IMobility *target) const;
    virtual cFigure::Rectangle getViewportBounds() const; // the target module's drawing area, used to tile detached plots below it

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif
