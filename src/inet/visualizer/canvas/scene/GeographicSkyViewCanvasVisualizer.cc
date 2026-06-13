//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/scene/GeographicSkyViewCanvasVisualizer.h"

#include <cmath>
#include <cstdio>
#include <vector>

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/Wgs84.h"

namespace inet {

namespace visualizer {

Define_Module(GeographicSkyViewCanvasVisualizer);

GeographicSkyViewCanvasVisualizer::ObserverPlot::~ObserverPlot()
{
    delete groupFigure; // deletes all child figures (plot decorations and target dots/labels/trails)
    for (auto it : targetMarks)
        delete it.second; // a TargetMark owns no figures (the group does)
}

void GeographicSkyViewCanvasVisualizer::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displaySkyView = par("displaySkyView");
        animationSpeed = par("animationSpeed");
        zIndex = par("zIndex");
        coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
        if (coordinateSystem == nullptr)
            throw cRuntimeError("Geographic coordinate system module not found on path '%s' defined by par 'coordinateSystemModule'", par("coordinateSystemModule").stringValue());
        observerFilter.setPattern(par("observerFilter"));
        targetFilter.setPattern(par("targetFilter"));
        plotRadius = par("plotRadius");
        plotOffsetX = par("plotOffsetX");
        plotOffsetY = par("plotOffsetY");
        minElevationAngle = deg(par("minElevationAngle")).get<deg>();
        backgroundColor = cFigure::parseColor(par("backgroundColor"));
        backgroundOpacity = par("backgroundOpacity");
        ringColor = cFigure::parseColor(par("ringColor"));
        ringLineWidth = par("ringLineWidth");
        maskColor = cFigure::parseColor(par("maskColor"));
        maskOpacity = par("maskOpacity");
        cardinalColor = cFigure::parseColor(par("cardinalColor"));
        labelColor = cFigure::parseColor(par("labelColor"));
        targetColorSet.parseColors(par("targetColor"));
        targetRadius = par("targetRadius");
        lowElevationOpacity = par("lowElevationOpacity");
        displayLabels = par("displayLabels");
        displayTrails = par("displayTrails");
        trailLength = par("trailLength");
        trailLineStyle = cFigure::parseLineStyle(par("trailLineStyle"));
        trailLineWidth = par("trailLineWidth");
        if (displaySkyView)
            subscribe();
    }
}

void GeographicSkyViewCanvasVisualizer::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr && !strcmp(name, "observerFilter"))
        observerFilter.setPattern(par("observerFilter"));
    else if (name != nullptr && !strcmp(name, "targetFilter"))
        targetFilter.setPattern(par("targetFilter"));
}

void GeographicSkyViewCanvasVisualizer::preDelete(cComponent *root)
{
    if (displaySkyView) {
        unsubscribe();
        std::vector<int> ids;
        for (auto it : observerPlots)
            ids.push_back(it.first);
        for (int id : ids)
            removeObserver(id);
        targets.clear();
    }
}

void GeographicSkyViewCanvasVisualizer::subscribe()
{
    visualizationSubjectModule->subscribe(IMobility::mobilityStateChangedSignal, this);
    visualizationSubjectModule->subscribe(PRE_MODEL_CHANGE, this);
}

void GeographicSkyViewCanvasVisualizer::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(IMobility::mobilityStateChangedSignal, this);
        visualizationSubjectModule->unsubscribe(PRE_MODEL_CHANGE, this);
    }
}

void GeographicSkyViewCanvasVisualizer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == IMobility::mobilityStateChangedSignal) {
        auto module = check_and_cast<cModule *>(source);
        auto mobility = dynamic_cast<IMobility *>(source);
        if (mobility == nullptr)
            return;
        int id = module->getId();
        if (observerFilter.matches(module) && observerPlots.find(id) == observerPlots.end())
            addObserver(mobility);
        if (targetFilter.matches(module) && targets.find(id) == targets.end())
            addTarget(mobility);
    }
    else if (signal == PRE_MODEL_CHANGE) {
        if (dynamic_cast<cPreModuleDeleteNotification *>(object)) {
            if (dynamic_cast<IMobility *>(source) != nullptr) {
                int id = check_and_cast<cModule *>(source)->getId();
                removeTarget(id);
                removeObserver(id);
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

GeographicSkyViewCanvasVisualizer::ObserverPlot *GeographicSkyViewCanvasVisualizer::addObserver(IMobility *observer)
{
    int id = check_and_cast<cModule *>(observer)->getId();
    auto plot = new ObserverPlot(observer);
    auto group = new cGroupFigure("skyView");
    group->setTags((std::string("sky_view ") + tags).c_str());
    group->setZIndex(zIndex);
    plot->groupFigure = group;
    buildPlotDecorations(plot);
    for (auto it : targets)
        if (it.first != id)
            plot->targetMarks[it.first] = createTargetMark(plot, it.second);
    observerPlots[id] = plot;
    visualizationTargetModule->getCanvas()->addFigure(group);
    return plot;
}

void GeographicSkyViewCanvasVisualizer::addTarget(IMobility *target)
{
    int id = check_and_cast<cModule *>(target)->getId();
    targets[id] = target;
    for (auto it : observerPlots)
        if (it.first != id && it.second->targetMarks.find(id) == it.second->targetMarks.end())
            it.second->targetMarks[id] = createTargetMark(it.second, target);
}

void GeographicSkyViewCanvasVisualizer::removeObserver(int id)
{
    auto it = observerPlots.find(id);
    if (it == observerPlots.end())
        return;
    visualizationTargetModule->getCanvas()->removeFigure(it->second->groupFigure);
    delete it->second; // deletes the group (and all its child figures) and the marks
    observerPlots.erase(it);
}

void GeographicSkyViewCanvasVisualizer::removeTarget(int id)
{
    auto it = targets.find(id);
    if (it == targets.end())
        return;
    for (auto pit : observerPlots) {
        auto plot = pit.second;
        auto mit = plot->targetMarks.find(id);
        if (mit != plot->targetMarks.end()) {
            removeTargetMark(plot, mit->second);
            plot->targetMarks.erase(mit);
        }
    }
    targets.erase(it);
}

GeographicSkyViewCanvasVisualizer::TargetMark *GeographicSkyViewCanvasVisualizer::createTargetMark(ObserverPlot *plot, IMobility *target)
{
    auto module = check_and_cast<cModule *>(target);
    cFigure::Color color = targetColorSet.getColor(module->getId());
    auto mark = new TargetMark(target);

    auto dot = new cOvalFigure("target");
    dot->setTags((std::string("sky_target ") + tags).c_str());
    dot->setFilled(true);
    dot->setLineColor(color);
    dot->setFillColor(color);
    dot->setVisible(false);
    plot->groupFigure->addFigure(dot);
    mark->dotFigure = dot;

    if (displayLabels) {
        auto label = new cLabelFigure("targetLabel");
        label->setTags((std::string("sky_target_label ") + tags).c_str());
        label->setText(getTargetLabel(target).c_str());
        label->setColor(color);
        label->setHalo(true);
        label->setAnchor(cFigure::ANCHOR_W);
        label->setVisible(false);
        plot->groupFigure->addFigure(label);
        mark->labelFigure = label;
    }
    if (displayTrails) {
        auto trail = new TrailFigure(trailLength, true, "skyTrail");
        trail->setTags((std::string("sky_trail recent_history ") + tags).c_str());
        plot->groupFigure->addFigure(trail);
        mark->trailFigure = trail;
    }
    return mark;
}

void GeographicSkyViewCanvasVisualizer::removeTargetMark(ObserverPlot *plot, TargetMark *mark)
{
    auto group = plot->groupFigure;
    if (mark->dotFigure != nullptr) { group->removeFigure(mark->dotFigure); delete mark->dotFigure; }
    if (mark->labelFigure != nullptr) { group->removeFigure(mark->labelFigure); delete mark->labelFigure; }
    if (mark->trailFigure != nullptr) { group->removeFigure(mark->trailFigure); delete mark->trailFigure; }
    delete mark;
}

void GeographicSkyViewCanvasVisualizer::buildPlotDecorations(ObserverPlot *plot)
{
    auto group = plot->groupFigure;
    double R = plotRadius;

    auto addLabel = [&](const char *text, cFigure::Point position, cFigure::Anchor anchor, const cFigure::Color& color, int pointSize) {
        auto label = new cLabelFigure();
        label->setText(text);
        label->setColor(color);
        label->setHalo(true);
        label->setAnchor(anchor);
        label->setPosition(position);
        if (pointSize > 0)
            label->setFont(cFigure::Font("", pointSize));
        group->addFigure(label);
    };

    // sky disc background
    auto disc = new cOvalFigure("sky");
    disc->setBounds(cFigure::Rectangle(-R, -R, 2 * R, 2 * R));
    disc->setFilled(true);
    disc->setFillColor(backgroundColor);
    disc->setFillOpacity(backgroundOpacity);
    disc->setLineColor(ringColor);
    disc->setLineWidth(ringLineWidth);
    group->addFigure(disc);

    // translucent mask over the band between minElevationAngle and the horizon
    if (minElevationAngle > 0) {
        double rMin = R * (90 - minElevationAngle) / 90;
        auto mask = new cRingFigure("mask");
        mask->setBounds(cFigure::Rectangle(-R, -R, 2 * R, 2 * R));
        mask->setInnerRadius(rMin);
        mask->setFilled(true);
        mask->setFillColor(maskColor);
        mask->setFillOpacity(maskOpacity);
        mask->setLineOpacity(0);
        group->addFigure(mask);
    }

    // elevation rings at 30 and 60 degrees (0 is the disc rim, 90 is the center)
    for (double el : { 30.0, 60.0 }) {
        double r = R * (90 - el) / 90;
        auto ring = new cOvalFigure("ring");
        ring->setBounds(cFigure::Rectangle(-r, -r, 2 * r, 2 * r));
        ring->setLineColor(ringColor);
        ring->setLineWidth(ringLineWidth);
        group->addFigure(ring);
    }
    // dashed ring at the minimum elevation
    if (minElevationAngle > 0 && minElevationAngle < 90) {
        double r = R * (90 - minElevationAngle) / 90;
        auto ring = new cOvalFigure("minElevationRing");
        ring->setBounds(cFigure::Rectangle(-r, -r, 2 * r, 2 * r));
        ring->setLineColor(ringColor);
        ring->setLineStyle(cFigure::LINE_DASHED);
        ring->setLineWidth(ringLineWidth);
        group->addFigure(ring);
    }

    // cardinal cross lines (N-S and W-E)
    auto northSouth = new cLineFigure("northSouth");
    northSouth->setStart(cFigure::Point(0, -R));
    northSouth->setEnd(cFigure::Point(0, R));
    northSouth->setLineColor(cardinalColor);
    northSouth->setLineWidth(ringLineWidth);
    group->addFigure(northSouth);
    auto westEast = new cLineFigure("westEast");
    westEast->setStart(cFigure::Point(-R, 0));
    westEast->setEnd(cFigure::Point(R, 0));
    westEast->setLineColor(cardinalColor);
    westEast->setLineWidth(ringLineWidth);
    group->addFigure(westEast);

    // cardinal labels (North up, East right)
    addLabel("N", cFigure::Point(0, -R - 1), cFigure::ANCHOR_S, cardinalColor, 0);
    addLabel("S", cFigure::Point(0, R + 1), cFigure::ANCHOR_N, cardinalColor, 0);
    addLabel("E", cFigure::Point(R + 1, 0), cFigure::ANCHOR_W, cardinalColor, 0);
    addLabel("W", cFigure::Point(-R - 1, 0), cFigure::ANCHOR_E, cardinalColor, 0);

    // elevation labels sitting just above their ring along the North spoke, in a small
    // (half-size) font; the 0 deg ring is the disc rim and is not labeled. No title is
    // drawn: the observer module icon is right next to the plot and already shows its name.
    for (double el : { 30.0, 60.0 }) {
        double r = R * (90 - el) / 90;
        char buf[8];
        snprintf(buf, sizeof(buf), "%g°", el);
        addLabel(buf, cFigure::Point(2, -r - 1), cFigure::ANCHOR_SW, labelColor, 6);
    }
}

cFigure::Point GeographicSkyViewCanvasVisualizer::elevationAzimuthToCanvas(double elevationDeg, double azimuthDeg) const
{
    double r = plotRadius * (90 - elevationDeg) / 90;
    double a = azimuthDeg * M_PI / 180;
    // North up, East right, azimuth clockwise from North
    return cFigure::Point(r * std::sin(a), -r * std::cos(a));
}

std::string GeographicSkyViewCanvasVisualizer::getTargetLabel(const IMobility *target) const
{
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(target));
    if (module->hasPar("satelliteName")) {
        const char *name = module->par("satelliteName").stringValue();
        if (*name)
            return name;
    }
    auto node = module->getParentModule();
    return node != nullptr ? node->getFullName() : module->getFullName();
}

void GeographicSkyViewCanvasVisualizer::refreshDisplay() const
{
    for (auto pit : observerPlots) {
        auto plot = pit.second;
        auto observerModule = check_and_cast<const cModule *>(plot->observer);
        auto observerNode = observerModule->getParentModule();
        // anchor the plot to the observer's icon position (not its Earth-scale ECEF position)
        auto bounds = getEnvir()->getSubmoduleBounds(observerNode != nullptr ? observerNode : observerModule);
        auto anchor = bounds.getCenter();
        plot->groupFigure->setTransform(cFigure::Transform().translate(anchor.x + plotOffsetX, anchor.y + plotOffsetY));

        GeoCoord observerGeo = coordinateSystem->computeGeographicCoordinate(plot->observer->getCurrentPosition());
        for (auto mit : plot->targetMarks) {
            auto mark = mit.second;
            GeoCoord targetGeo = coordinateSystem->computeGeographicCoordinate(mark->target->getCurrentPosition());
            auto lookAngles = wgs84::computeLookAngles(observerGeo, wgs84::geodeticToEcef(targetGeo));
            double elevation = lookAngles.elevation.get<deg>();
            double azimuth = lookAngles.azimuth.get<deg>();
            if (elevation < 0) {
                // below the horizon: hide the target and remove its sky trail immediately
                mark->dotFigure->setVisible(false);
                if (mark->labelFigure != nullptr)
                    mark->labelFigure->setVisible(false);
                if (mark->trailFigure != nullptr)
                    while (mark->trailFigure->getNumFigures() > 0)
                        delete mark->trailFigure->removeFigure(0);
                mark->hadPoint = false;
                continue;
            }
            cFigure::Point point = elevationAzimuthToCanvas(elevation, azimuth);
            double opacity = (elevation < minElevationAngle) ? lowElevationOpacity : 1.0;
            mark->dotFigure->setBounds(cFigure::Rectangle(point.x - targetRadius, point.y - targetRadius, 2 * targetRadius, 2 * targetRadius));
            mark->dotFigure->setVisible(true);
            mark->dotFigure->setFillOpacity(opacity);
            mark->dotFigure->setLineOpacity(opacity);
            char tooltip[96];
            snprintf(tooltip, sizeof(tooltip), "az %.1f° el %.1f° range %.0fkm", azimuth, elevation, lookAngles.range.get<m>() / 1000);
            mark->dotFigure->setTooltip(tooltip);
            if (mark->labelFigure != nullptr) {
                mark->labelFigure->setVisible(true);
                mark->labelFigure->setOpacity(opacity);
                mark->labelFigure->setPosition(cFigure::Point(point.x + targetRadius + 2, point.y));
            }
            if (mark->trailFigure != nullptr) {
                if (!mark->hadPoint) {
                    mark->lastPoint = point;
                    mark->hadPoint = true;
                }
                else {
                    double dx = point.x - mark->lastPoint.x;
                    double dy = point.y - mark->lastPoint.y;
                    if (dx * dx + dy * dy > 1) {
                        auto segment = new cLineFigure("skyTrailSegment");
                        segment->setTags((std::string("sky_trail recent_history ") + tags).c_str());
                        segment->setStart(mark->lastPoint);
                        segment->setEnd(point);
                        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mark->target));
                        segment->setLineColor(targetColorSet.getColor(module->getId()));
                        segment->setLineStyle(trailLineStyle);
                        segment->setLineWidth(trailLineWidth);
                        mark->trailFigure->addFigure(segment);
                        mark->lastPoint = point;
                    }
                }
            }
        }
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(observerPlots.empty() ? 0 : animationSpeed, this);
}

} // namespace visualizer

} // namespace inet
