//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/physicallayer/MediumCanvasVisualizer.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/figures/LabeledIconFigure.h"
#include "inet/common/figures/SignalFigure.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmission.h"
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

namespace inet {
namespace visualizer {

Define_Module(MediumCanvasVisualizer);

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON

using namespace inet::physicallayer;

void MediumCanvasVisualizer::initialize(int stage)
{
    MediumVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        const char *signalShapeString = par("signalShape");
        if (!strcmp(signalShapeString, "ring"))
            signalShape = SIGNAL_SHAPE_RING;
        else if (!strcmp(signalShapeString, "sphere"))
            signalShape = SIGNAL_SHAPE_SPHERE;
        else
            throw cRuntimeError("Unknown signalShape parameter value: '%s'", signalShapeString);
        signalOpacity = par("signalOpacity");
        signalRingCount = par("signalRingCount");
        signalRingSize = par("signalRingSize");
        signalFadingDistance = par("signalFadingDistance");
        signalFadingFactor = par("signalFadingFactor");
        signalWaveCount = par("signalWaveCount");
        signalWaveLength = par("signalWaveLength");
        signalWaveWidth = par("signalWaveWidth");
        signalWaveFadingAnimationSpeedFactor = par("signalWaveFadingAnimationSpeedFactor");
        cCanvas *canvas = visualizationTargetModule->getCanvas();
        if (displaySignals) {
            signalLayer = new cGroupFigure("communication");
            signalLayer->setZIndex(zIndex);
            signalLayer->insertBelow(canvas->getSubmodulesLayer());
        }
        displayCommunicationHeat = par("displayCommunicationHeat");
        if (displayCommunicationHeat) {
            communicationHeat = new HeatMapFigure(communicationHeatMapSize, "communication heat");
            communicationHeat->setZIndex(zIndex);
            communicationHeat->setTags((std::string("successful_reception heat ") + tags).c_str());
            canvas->addFigure(communicationHeat, 0);
        }
        animationSpeedInterpolator.setCurrentAnimationSpeed(0);
        animationSpeedInterpolator.setTargetAnimationSpeed(signalAnimationSpeedChangeTimeMode, 0, 0);
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
    else if (stage == INITSTAGE_LAST) {
        canvasProjection = CanvasProjection::getCanvasProjection(visualizationTargetModule->getCanvas());
        if (communicationHeat != nullptr) {
            const IMediumLimitCache *mediumLimitCache = radioMedium->getMediumLimitCache();
            Coord min = mediumLimitCache->getMinConstraintArea();
            Coord max = mediumLimitCache->getMaxConstraintArea();
            cFigure::Point o = canvasProjection->computeCanvasPoint(Coord::ZERO);
            cFigure::Point x = canvasProjection->computeCanvasPoint(Coord(1, 0, 0));
            cFigure::Point y = canvasProjection->computeCanvasPoint(Coord(0, 1, 0));
            double t1 = o.x;
            double t2 = o.y;
            double a = x.x - t1;
            double b = x.y - t2;
            double c = y.x - t1;
            double d = y.y - t2;
            communicationHeat->setTransform(cFigure::Transform(a, b, c, d, t1, t2));
            communicationHeat->setPosition(cFigure::Point((min.x + max.x) / 2, (min.y + max.y) / 2));
            communicationHeat->setWidth(max.x - min.x);
            communicationHeat->setHeight(max.y - min.y);
        }
        if (displayMainPowerDensityMap) {
            mainPowerDensityMapFigure = new HeatMapPlotFigure();
            mainPowerDensityMapFigure->setTags("signal_main_power_density_map");
            mainPowerDensityMapFigure->setTooltip("This plot represents the signal power density over space");
            mainPowerDensityMapFigure->setZIndex(zIndex - 2);
            mainPowerDensityMapFigure->setXAxisLabel("[m]");
            mainPowerDensityMapFigure->setYAxisLabel("[m]");
            mainPowerDensityMapFigure->setXValueFormat("%.3g");
            mainPowerDensityMapFigure->setYValueFormat("%.3g");
            mainPowerDensityMapFigure->invertYAxis();
            auto width = mainPowerDensityMapMaxX - mainPowerDensityMapMinX;
            auto height = mainPowerDensityMapMaxY - mainPowerDensityMapMinY;
            auto minPosition = canvasProjection->computeCanvasPointInverse(cFigure::Point(mainPowerDensityMapMinX, mainPowerDensityMapMinY), mainPowerDensityMapZ);
            auto maxPosition = canvasProjection->computeCanvasPointInverse(cFigure::Point(mainPowerDensityMapMaxX, mainPowerDensityMapMaxY), mainPowerDensityMapZ);
            mainPowerDensityMapFigure->setMinX(minPosition.x);
            mainPowerDensityMapFigure->setMinY(minPosition.y);
            mainPowerDensityMapFigure->setMaxX(maxPosition.x);
            mainPowerDensityMapFigure->setMaxY(maxPosition.y);
            mainPowerDensityMapFigure->setBounds(cFigure::Rectangle(mainPowerDensityMapMinX, mainPowerDensityMapMinY, width, height));
            mainPowerDensityMapFigure->setPlotSize(cFigure::Point(width, height), cFigure::Point(width * mainPowerDensityMapPixmapDensity, height * mainPowerDensityMapPixmapDensity));
            cCanvas *canvas = visualizationTargetModule->getCanvas();
            canvas->addFigure(mainPowerDensityMapFigure, 0);
        }
        for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
            auto networkNode = *it;
            if (isNetworkNode(networkNode) && networkNodeFilter.matches(networkNode)) {
                auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
                if (displayPowerDensityMaps) {
                    auto powerDensityMapFigure = new HeatMapPlotFigure();
                    powerDensityMapFigure->setTags("signal_power_density_map");
                    powerDensityMapFigure->setTooltip("This plot represents the signal power density over space");
                    powerDensityMapFigure->setZIndex(zIndex - 1);
                    powerDensityMapFigure->setXAxisLabel("[m]");
                    powerDensityMapFigure->setYAxisLabel("[m]");
                    powerDensityMapFigure->setXValueFormat("%.3g");
                    powerDensityMapFigure->setYValueFormat("%.3g");
                    powerDensityMapFigure->invertYAxis();
                    powerDensityMapFigure->setPlotSize(cFigure::Point(powerDensityMapFigureWidth, powerDensityMapFigureHeight), cFigure::Point(powerDensityMapPixmapWidth, powerDensityMapPixmapHeight));
                    // TODO center on node to align in space coordinates
                    powerDensityMapFigure->refreshDisplay();
                    networkNodeVisualization->addAnnotation(powerDensityMapFigure, powerDensityMapFigure->getPlotSize(), PLACEMENT_CENTER_CENTER, -1);
                    powerDensityMapFigures[networkNode->getId()] = powerDensityMapFigure;
                }
                if (displaySpectrums) {
                    auto spectrumFigure = new PlotFigure();
                    spectrumFigure->setTags("signal_spectrum");
                    spectrumFigure->setTooltip("This plot represents the signal spectral power density");
                    spectrumFigure->setZIndex(zIndex);
                    spectrumFigure->setNumSeries(3);
                    spectrumFigure->setLineColor(0, cFigure::parseColor("darkred"));
                    spectrumFigure->setLineColor(1, cFigure::parseColor("darkgreen"));
                    spectrumFigure->setLineColor(2, cFigure::parseColor("darkblue"));
                    spectrumFigure->setXAxisLabel("[GHz]");
                    spectrumFigure->setYAxisLabel("[dBmW/MHz]");
                    spectrumFigure->setXValueFormat("%.3g");
                    spectrumFigure->setYValueFormat("%.3g");
                    spectrumFigure->setPlotSize(cFigure::Point(spectrumFigureWidth, spectrumFigureHeight));
                    spectrumFigure->refreshDisplay();
                    networkNodeVisualization->addAnnotation(spectrumFigure, spectrumFigure->getPlotSize(), spectrumPlacementHint, spectrumPlacementPriority);
                    spectrumFigures[networkNode->getId()] = spectrumFigure;
                }
                if (displaySpectrograms) {
                    auto spectrogramFigure = new HeatMapPlotFigure();
                    spectrogramFigure->setTags("signal_spectrogram");
                    spectrogramFigure->setTooltip("This plot represents the signal power density over time and frequency");
                    spectrogramFigure->setZIndex(zIndex);
                    spectrogramFigure->setXAxisLabel("[GHz]");
                    spectrogramFigure->setYAxisLabel("[s]");
                    spectrogramFigure->setXValueFormat("%.3g");
                    spectrogramFigure->setYValueFormat("%.3g");
                    spectrogramFigure->invertYAxis();
                    spectrogramFigure->setPlotSize(cFigure::Point(spectrogramFigureWidth, spectrogramFigureHeight), cFigure::Point(spectrogramPixmapWidth, spectrogramPixmapHeight));
                    spectrogramFigure->refreshDisplay();
                    networkNodeVisualization->addAnnotation(spectrogramFigure, spectrogramFigure->getPlotSize(), spectrogramPlacementHint, spectrogramPlacementPriority);
                    spectrogramFigures[networkNode->getId()] = spectrogramFigure;
                }
                if (displayPowerDensityMaps || displaySpectrums || displaySpectrograms)
                    if (!networkNode->isSubscribed(IMobility::mobilityStateChangedSignal, this))
                        networkNode->subscribe(IMobility::mobilityStateChangedSignal, this);
            }
        }
    }
}

void MediumCanvasVisualizer::refreshDisplay() const
{
    if (invalidDisplay || lastRefreshDisplay != simTime()) {
        if (displaySignals || displayMainPowerDensityMap || displayPowerDensityMaps)
            const_cast<MediumCanvasVisualizer *>(this)->setAnimationSpeed();
        if (displaySignals) {
            for (auto transmission : transmissions)
                if (matchesTransmission(transmission))
                    refreshSignalFigure(transmission);
        }
        if (!getEnvir()->isExpressMode()) {
            if (displayMainPowerDensityMap)
                refreshMainPowerDensityMapFigure();
            if (displayPowerDensityMaps) {
                for (auto it : powerDensityMapFigures)
                    refreshPowerDensityMapFigure(getSimulation()->getModule(it.first), it.second);
            }
            if (displaySpectrums)
                for (auto it : spectrumFigures)
                    refreshSpectrumFigure(getSimulation()->getModule(it.first), it.second);
            if (displaySpectrograms)
                for (auto it : spectrogramFigures)
                    refreshSpectrogramFigure(getSimulation()->getModule(it.first), it.second);
            if (displayCommunicationHeat)
                communicationHeat->coolDown();
            lastRefreshDisplay = simTime();
            invalidDisplay = false;
        }
    }
}

void MediumCanvasVisualizer::refreshMainPowerDensityMapFigure() const
{
    if ((!std::isnan(powerDensityMapCenterFrequency.get()))) {
        mainPowerDensityMapFigure->setXTickCount(mainPowerDensityMapFigureXTickCount);
        mainPowerDensityMapFigure->setYTickCount(mainPowerDensityMapFigureYTickCount);
        mainPowerDensityMapFigure->clearValues();
        refreshPowerDensityMapFigurePowerFunction(mediumPowerDensityFunction, mainPowerDensityMapFigure, 2);
        mainPowerDensityMapFigure->bakeValues();
        mainPowerDensityMapFigure->refreshDisplay();
    }
}

void MediumCanvasVisualizer::refreshPowerDensityMapFigure(const cModule *networkNode, HeatMapPlotFigure *figure) const
{
    if (!std::isnan(powerDensityMapCenterFrequency.get())) {
        const ITransmission *transmissionInProgress;
        const ITransmission *receptionInProgress;
        const IAntenna *antenna;
        IMobility *mobility;
        std::tie(transmissionInProgress, receptionInProgress, antenna, mobility) = extractSpectrumFigureParameters(networkNode);
        auto position = antenna->getMobility()->getCurrentPosition();
        auto point = canvasProjection->computeCanvasPoint(position);
        auto zoom = getEnvir()->getZoomLevel(visualizationTargetModule);
        auto minPosition = canvasProjection->computeCanvasPointInverse(cFigure::Point(point.x - powerDensityMapFigureWidth / 2 / zoom, point.y - powerDensityMapFigureHeight / 2 / zoom), powerDensityMapZ);
        auto maxPosition = canvasProjection->computeCanvasPointInverse(cFigure::Point(point.x + powerDensityMapFigureWidth / 2 / zoom, point.y + powerDensityMapFigureHeight / 2 / zoom), powerDensityMapZ);
        figure->setMinX(minPosition.x);
        figure->setMaxX(maxPosition.x);
        figure->setMinY(minPosition.y);
        figure->setMaxY(maxPosition.y);
        figure->setXTickCount(powerDensityMapFigureXTickCount);
        figure->setYTickCount(powerDensityMapFigureYTickCount);
        figure->clearValues();
        if (!strcmp(powerDensityMapMode, "total"))
            refreshPowerDensityMapFigurePowerFunction(mediumPowerDensityFunction, figure, 2);
        else if (!strcmp(powerDensityMapMode, "signal")) {
            if (transmissionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(transmissionInProgress->getId())->second;
                refreshPowerDensityMapFigurePowerFunction(signalPowerDensityFunction, figure, 1);
            }
            else if (receptionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(receptionInProgress->getId())->second;
                refreshPowerDensityMapFigurePowerFunction(signalPowerDensityFunction, figure, 1);
            }
        }
        else if (!strcmp(powerDensityMapMode, "auto")) {
            if (transmissionInProgress == nullptr && receptionInProgress == nullptr)
                refreshPowerDensityMapFigurePowerFunction(mediumPowerDensityFunction, figure, 2);
            else if (transmissionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(transmissionInProgress->getId())->second;
                const auto& noisePowerDensityFunction = noisePowerDensityFunctions.find(transmissionInProgress->getId())->second;
                refreshPowerDensityMapFigurePowerFunction(noisePowerDensityFunction, figure, 0);
                refreshPowerDensityMapFigurePowerFunction(signalPowerDensityFunction, figure, 1);
            }
            else if (receptionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(receptionInProgress->getId())->second;
                const auto& noisePowerDensityFunction = noisePowerDensityFunctions.find(receptionInProgress->getId())->second;
                refreshPowerDensityMapFigurePowerFunction(noisePowerDensityFunction, figure, 0);
                refreshPowerDensityMapFigurePowerFunction(signalPowerDensityFunction, figure, 1);
            }
        }
        else
            throw cRuntimeError("Unknown signalFigureMode");
        figure->bakeValues();
        figure->refreshDisplay();
    }
}

void MediumCanvasVisualizer::refreshPowerDensityMapFigurePowerFunction(const Ptr<const math::IFunction<WpHz, math::Domain<m, m, m, simsec, Hz>>>& powerDensityFunction, HeatMapPlotFigure *figure, int channel) const
{
    Coord minPosition(figure->getMinX(), figure->getMinY(), 0);
    Coord maxPosition(figure->getMaxX(), figure->getMaxY(), 0);
    Ptr<const math::IFunction<W, math::Domain<m, m, m, simsec>>> powerFunction;
    if (powerDensityMapBandwidth == Hz(0)) {
        powerFunction = nullptr;
        figure->setMinValue(wpHz2dBmWpMHz(WpHz(signalMinPowerDensity).get()));
        figure->setMaxValue(wpHz2dBmWpMHz(WpHz(signalMaxPowerDensity).get()));
    }
    else if (std::isinf(powerDensityMapBandwidth.get())) {
        powerFunction = integrate<WpHz, Domain<m, m, m, simsec, Hz>, 0b11110, W, Domain<m, m, m, simsec>>(powerDensityFunction);
        figure->setMinValue(mW2dBmW(mW(signalMinPower).get()));
        figure->setMaxValue(mW2dBmW(mW(signalMaxPower).get()));
        throw cRuntimeError("TODO");
    }
    else {
        // TODO
//        auto bandpassFilterFunction = makeShared<Boxcar1DFunction<double, Hz>>(powerDensityMapCenterFrequency - powerDensityMapBandwidth / 2, powerDensityMapCenterFrequency + powerDensityMapBandwidth / 2, 1);
//        powerFunction = integrate<WpHz, Domain<m, m, m, simsec, Hz>, 0b11110, W, Domain<m, m, m, simsec>>(powerDensityFunction->multiply(bandpassFilter));
        figure->setMinValue(mW2dBmW(mW(signalMinPower).get()));
        figure->setMaxValue(mW2dBmW(mW(signalMaxPower).get()));
        throw cRuntimeError("TODO");
    }
    auto size = maxPosition - minPosition;
    auto pixmapSize = figure->getPixmapSize();
    if (!strcmp(powerDensityMapPixelMode, "sample")) {
        const int xsize = pixmapSize.x;
        // NOTE: this method is fast, but it only get samples of the function at certain places, so it may result in incorrect colors
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (int x = 0; x < xsize; x++) {
            for (int y = 0; y < pixmapSize.y; y++) {
                if (powerFunction == nullptr) {
                    auto value = powerDensityFunction->getValue(Point<m, m, m, simsec, Hz>(m(minPosition.x + x * size.x / pixmapSize.x), m(minPosition.y + y * size.y / pixmapSize.y), m(0), simsec(simTime()), powerDensityMapCenterFrequency));
                    figure->setValue(x, y, wpHz2dBmWpMHz(WpHz(value).get()), channel);
                }
                else {
                    auto value = powerFunction->getValue(Point<m, m, m, simsec>(m(minPosition.x + x * size.x / pixmapSize.x), m(minPosition.y + y * size.y / pixmapSize.y), m(0), simsec(simTime())));
                    figure->setValue(x, y, mW2dBmW(mW(value).get()), channel);
                }
            }
        }
    }
    else if (!strcmp(powerDensityMapPixelMode, "partition")) {
        Point<m, m, m, simsec, Hz> l(m(minPosition.x), m(minPosition.y), m(minPosition.z), simsec(simTime()), powerDensityMapCenterFrequency);
        Point<m, m, m, simsec, Hz> u(m(maxPosition.x), m(maxPosition.y), m(maxPosition.z), simsec(simTime()), powerDensityMapCenterFrequency);
        Interval<m, m, m, simsec, Hz> i(l, u, 0b00111, 0b00111, 0b00111);
        auto approximatedMediumPowerFunction1 = makeShared<ApproximatedFunction<WpHz, Domain<m, m, m, simsec, Hz>, 0, m>>(m(minPosition.x), m(maxPosition.x), m(size.x) / pixmapSize.x * powerDensityMapApproximationSize, &LinearInterpolator<m, WpHz>::singleton, powerDensityFunction);
        auto approximatedMediumPowerFunction2 = makeShared<ApproximatedFunction<WpHz, Domain<m, m, m, simsec, Hz>, 1, m>>(m(minPosition.y), m(maxPosition.y), m(size.y) / pixmapSize.y * powerDensityMapApproximationSize, &LinearInterpolator<m, WpHz>::singleton, approximatedMediumPowerFunction1);
        // NOTE: this method is fast, but it may paint the same pixel several times, so the last will be the effective color
        approximatedMediumPowerFunction2->partition(i, [&] (const Interval<m, m, m, simsec, Hz>& j, const IFunction<WpHz, Domain<m, m, m, simsec, Hz>> *partitonPowerFunction) {
            auto lower = j.getLower();
            auto upper = j.getUpper();
            auto x1 = m(std::get<0>(lower)).get();
            auto x2 = m(std::get<0>(upper)).get();
            auto y1 = m(std::get<1>(lower)).get();
            auto y2 = m(std::get<1>(upper)).get();
            if (auto cf = dynamic_cast<const ConstantFunction<WpHz, Domain<m, m, m, simsec, Hz>> *>(partitonPowerFunction)) {
                auto v = cf->getConstantValue() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(cf->getConstantValue()).get());
                figure->setConstantValue(x1, x2, y1, y2, v, channel);
            }
            else if (auto lf = dynamic_cast<const UnilinearFunction<WpHz, Domain<m, m, m, simsec, Hz>> *>(partitonPowerFunction)) {
                auto vl = lf->getRLower() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(lf->getRLower()).get());
                auto vu = lf->getRUpper() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(lf->getRUpper()).get());
                figure->setLinearValue(x1, x2, y1, y2, vl, vu, lf->getDimension(), channel);
            }
            else if (auto bf = dynamic_cast<const BilinearFunction<WpHz, Domain<m, m, m, simsec, Hz>> *>(partitonPowerFunction)) {
                auto v11 = bf->getRLowerLower() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(bf->getRLowerLower()).get());
                auto v21 = bf->getRLowerUpper() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(bf->getRLowerUpper()).get());
                auto v12 = bf->getRUpperLower() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(bf->getRUpperLower()).get());
                auto v22 = bf->getRUpperUpper() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(bf->getRUpperUpper()).get());
                figure->setBilinearValue(x1, x2, y1, y2, v11, v21, v12, v22, channel);
            }
            else
                throw cRuntimeError("TODO");
        });
    }
    else if (!strcmp(powerDensityMapPixelMode, "mean")) {
        auto approximatedMediumPowerFunction1 = makeShared<ApproximatedFunction<WpHz, Domain<m, m, m, simsec, Hz>, 0, m>>(m(minPosition.x), m(maxPosition.x), m(size.x) / pixmapSize.x * powerDensityMapApproximationSize, &LinearInterpolator<m, WpHz>::singleton, powerDensityFunction);
        auto approximatedMediumPowerFunction2 = makeShared<ApproximatedFunction<WpHz, Domain<m, m, m, simsec, Hz>, 1, m>>(m(minPosition.y), m(maxPosition.y), m(size.y) / pixmapSize.y * powerDensityMapApproximationSize, &LinearInterpolator<m, WpHz>::singleton, approximatedMediumPowerFunction1);
        Point<m, simsec, Hz> point(m(minPosition.z), simsec(simTime()), powerDensityMapCenterFrequency);
        auto fixedPowerFunction = makeShared<RightCurryingFunction<WpHz, Domain<m, m>, 0b11000, Domain<m, simsec, Hz>, 0b111, Domain<m, m, m, simsec, Hz>>>(point, approximatedMediumPowerFunction2);
        auto rasterizedFunction = makeShared<Rasterized2DFunction<WpHz, m, m>>(m(minPosition.x), m(maxPosition.x), pixmapSize.x + 1, m(minPosition.y), m(maxPosition.y), pixmapSize.y + 1, fixedPowerFunction);
        Point<m, m> lower(m(minPosition.x), m(minPosition.y));
        Point<m, m> upper(m(maxPosition.x), m(maxPosition.y));
        Interval<m, m> interval(lower, upper, 0b11, 0b00, 0b00);
        rasterizedFunction->partition(interval, [&] (const Interval<m, m>& i, const IFunction<WpHz, Domain<m, m>> *f) {
            if (auto cf = dynamic_cast<const ConstantFunction<WpHz, Domain<m, m>> *>(f)) {
                auto center = (i.getLower() + i.getUpper()) / 2;
                auto x = m(std::get<0>(center)).get();
                auto y = m(std::get<1>(center)).get();
                figure->setValue(x, y, wpHz2dBmWpMHz(WpHz(cf->getConstantValue()).get()), channel);
            }
            else
                throw cRuntimeError("Invalid partition function");
        });
    }
    else
        throw cRuntimeError("Unknown powerDensityMapPixelMode");
}

void MediumCanvasVisualizer::refreshSpectrumFigure(const cModule *networkNode, PlotFigure *figure) const
{
    if (signalMinFrequency < signalMaxFrequency) {
        const ITransmission *transmissionInProgress;
        const ITransmission *receptionInProgress;
        const IAntenna *antenna;
        IMobility *mobility;
        std::tie(transmissionInProgress, receptionInProgress, antenna, mobility) = extractSpectrumFigureParameters(networkNode);
        figure->clearValues(0);
        figure->clearValues(1);
        figure->clearValues(2);
        auto position = mobility->getCurrentPosition();
        if (!strcmp(spectrumMode, "total"))
            refreshSpectrumFigurePowerFunction(mediumPowerDensityFunction, antenna, position, figure, 2);
        else if (!strcmp(spectrumMode, "signal")) {
            if (transmissionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(transmissionInProgress->getId())->second;
                refreshSpectrumFigurePowerFunction(signalPowerDensityFunction, antenna, position, figure, 1);
            }
            else if (receptionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(receptionInProgress->getId())->second;
                refreshSpectrumFigurePowerFunction(signalPowerDensityFunction, antenna, position, figure, 1);
            }
        }
        else if (!strcmp(spectrumMode, "auto")) {
            if (transmissionInProgress == nullptr && receptionInProgress == nullptr)
                refreshSpectrumFigurePowerFunction(mediumPowerDensityFunction, antenna, position, figure, 2);
            else if (transmissionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(transmissionInProgress->getId())->second;
                const auto& noisePowerDensityFunction = noisePowerDensityFunctions.find(transmissionInProgress->getId())->second;
                refreshSpectrumFigurePowerFunction(noisePowerDensityFunction, antenna, position, figure, 0);
                refreshSpectrumFigurePowerFunction(signalPowerDensityFunction, antenna, position, figure, 1);
            }
            else if (receptionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(receptionInProgress->getId())->second;
                const auto& noisePowerDensityFunction = noisePowerDensityFunctions.find(receptionInProgress->getId())->second;
                refreshSpectrumFigurePowerFunction(noisePowerDensityFunction, antenna, position, figure, 0);
                refreshSpectrumFigurePowerFunction(signalPowerDensityFunction, antenna, position, figure, 1);
            }
        }
        else
            throw cRuntimeError("Unknown signalFigureMode");
        double minValue = wpHz2dBmWpMHz(WpHz(signalMinPowerDensity).get());
        double maxValue = wpHz2dBmWpMHz(WpHz(signalMaxPowerDensity).get());
        if (minValue < maxValue) {
            double margin = 0.05 * (maxValue - minValue);
            figure->setMinY(minValue - margin);
            figure->setMaxY(maxValue + margin);
            figure->setXTickCount(spectrumFigureXTickCount);
            figure->setYTickCount(spectrumFigureYTickCount);
        }
        figure->refreshDisplay();
    }
}

void MediumCanvasVisualizer::refreshSpectrumFigurePowerFunction(const Ptr<const IFunction<WpHz, Domain<m, m, m, simsec, Hz>>>& powerFunction, const IAntenna *antenna, const Coord& position, PlotFigure *figure, int series) const
{
    auto marginFrequency = 0.05 * (signalMaxFrequency - signalMinFrequency);
    auto minFrequency = signalMinFrequency - marginFrequency;
    auto maxFrequency = signalMaxFrequency + marginFrequency;
    figure->setMinX(GHz(minFrequency).get());
    figure->setMaxX(GHz(maxFrequency).get());
    Point<m, m, m, simsec, Hz> l(m(position.x), m(position.y), m(position.z), simsec(simTime()), minFrequency);
    Point<m, m, m, simsec, Hz> u(m(position.x), m(position.y), m(position.z), simsec(simTime()), maxFrequency);
    Interval<m, m, m, simsec, Hz> i(l, u, 0b11110, 0b11110, 0b11110);
    auto dx = GHz(maxFrequency - minFrequency).get() * spectrumFigureInterpolationSize / spectrumFigureWidth;
    powerFunction->partition(i, [&] (const Interval<m, m, m, simsec, Hz>& j, const IFunction<WpHz, Domain<m, m, m, simsec, Hz>> *partitonPowerFunction) {
        ASSERT((dynamic_cast<const ConstantFunction<WpHz, Domain<m, m, m, simsec, Hz>> *>(partitonPowerFunction) != nullptr || dynamic_cast<const UnilinearFunction<WpHz, Domain<m, m, m, simsec, Hz>> *>(partitonPowerFunction) != nullptr));
        auto lower = j.getLower();
        auto upper = j.getUpper();
        // NOTE: the interval is closed at the lower boundary and open at the upper boundary
        // we want to have the limit of the function's value at the upper boundary from the left
        if (std::get<4>(upper) != std::get<4>(lower))
            std::get<4>(upper) = Hz(std::nextafter(std::get<4>(upper).get(), std::get<4>(lower).get()));
        WpHz power1;
        WpHz power2;
        std::tie(power1, power2) = computePowerForPartitionBounds(powerFunction, lower, upper, partitonPowerFunction, antenna, position);
        // TODO the function f is assumed to be constant or linear between l1 and u1 but on a logarithmic axis the plot is non-linear
        // TODO the following interpolation should be part of the PlotFigure along with logarithmic axis support
        auto x1 = GHz(std::get<4>(lower)).get();
        auto x2 = GHz(std::get<4>(upper)).get();
        for (double x = x1; x < x2; x += dx) {
            double xi = x;
            double xj = std::min(x2, x + dx);
            double ai = (xi - x1) / (x2 - x1);
            double aj = (xj - x1) / (x2 - x1);
            auto yi = power1 * (1 - ai) + power2 * ai;
            auto yj = power1 * (1 - aj) + power2 * aj;
            figure->setValue(series, xi, wpHz2dBmWpMHz(WpHz(yi).get()));
            figure->setValue(series, xj, wpHz2dBmWpMHz(WpHz(yj).get()));
        }
    });
}

std::pair<WpHz, WpHz> MediumCanvasVisualizer::computePowerForPartitionBounds(const Ptr<const IFunction<WpHz, Domain<m, m, m, simsec, Hz>>>& powerFunction, const math::Point<m, m, m, simsec, Hz>& lower, const math::Point<m, m, m, simsec, Hz>& upper, const IFunction<WpHz, Domain<m, m, m, simsec, Hz>> *partitonPowerFunction, const IAntenna *antenna, const Coord& position) const
{
    WpHz totalPower1;
    WpHz totalPower2;
    if (antenna != nullptr && antenna->isDirectional()) {
        if (auto summedFunction = dynamicPtrCast<const SummedFunction<WpHz, Domain<m, m, m, simsec, Hz>>>(powerFunction)) {
            totalPower1 = WpHz(0);
            totalPower2 = WpHz(0);
            for (auto elementFunction : summedFunction->getElements()) {
                WpHz p1;
                WpHz p2;
                std::tie(p1, p2) = computePowerForDirectionalAntenna(elementFunction, lower, upper, antenna, position);
                totalPower1 += p1;
                totalPower2 += p2;
            }
        }
        else
            std::tie(totalPower1, totalPower2) = computePowerForDirectionalAntenna(powerFunction, lower, upper, antenna, position);
    }
    else {
        totalPower1 = partitonPowerFunction->getValue(lower);
        totalPower2 = partitonPowerFunction->getValue(upper);
    }
    ASSERT(!std::isnan(totalPower1.get()));
    ASSERT(!std::isnan(totalPower2.get()));
    return { totalPower1, totalPower2 };
}

std::pair<WpHz, WpHz> MediumCanvasVisualizer::computePowerForDirectionalAntenna(const Ptr<const IFunction<WpHz, Domain<m, m, m, simsec, Hz>>>& powerFunction, const math::Point<m, m, m, simsec, Hz>& lower, const math::Point<m, m, m, simsec, Hz>& upper, const IAntenna *antenna, const Coord& position) const
{
    if (auto rf = dynamicPtrCast<const MultipliedFunction<WpHz, Domain<m, m, m, simsec, Hz>>>(powerFunction)) {
        if (auto tf = dynamicPtrCast<const PropagatedTransmissionPowerFunction>(rf->getF1())) {
            const Point<m, m, m>& startPosition = tf->getStartPosition();
            double dx = std::get<0>(startPosition).get() - position.x;
            double dy = std::get<1>(startPosition).get() - position.y;
            double dz = std::get<2>(startPosition).get() - position.z;
            double gain = 1;
            if (dx != 0 || dy != 0 || dz != 0) {
                const Quaternion& startOrientation = antenna->getMobility()->getCurrentAngularPosition();
                auto direction = Quaternion::rotationFromTo(Coord::X_AXIS, Coord(dx, dy, dz));
                auto antennaLocalDirection = startOrientation.inverse() * direction;
                gain = antenna->getGain()->computeGain(antennaLocalDirection);
            }
            WpHz power1 = gain * powerFunction->getValue(lower);
            WpHz power2 = gain * powerFunction->getValue(upper);
            ASSERT(!std::isnan(power1.get()));
            ASSERT(!std::isnan(power2.get()));
            return { power1, power2 };
        }
    }
    WpHz power1 = powerFunction->getValue(lower);
    WpHz power2 = powerFunction->getValue(upper);
    ASSERT(!std::isnan(power1.get()));
    ASSERT(!std::isnan(power2.get()));
    return { power1, power2 };
}

void MediumCanvasVisualizer::refreshSpectrogramFigure(const cModule *networkNode, HeatMapPlotFigure *figure) const
{
    if (signalMinFrequency < signalMaxFrequency) {
        const ITransmission *transmissionInProgress;
        const ITransmission *receptionInProgress;
        const IAntenna *antenna;
        IMobility *mobility;
        std::tie(transmissionInProgress, receptionInProgress, antenna, mobility) = extractSpectrumFigureParameters(networkNode);
        double minValue = wpHz2dBmWpMHz(WpHz(signalMinPowerDensity).get());
        double maxValue = wpHz2dBmWpMHz(WpHz(signalMaxPowerDensity).get());
        auto marginFrequency = 0.05 * (signalMaxFrequency - signalMinFrequency);
        auto minFrequency = signalMinFrequency - marginFrequency;
        auto maxFrequency = signalMaxFrequency + marginFrequency;
        figure->setMinX(GHz(minFrequency).get());
        figure->setMaxX(GHz(maxFrequency).get());
        SimTimeUnit signalTimeUnit;
        if (signalMaxTime < 1E-9) {
            signalTimeUnit = SIMTIME_PS;
            figure->setYAxisLabel("[ps]");
        }
        else if (signalMaxTime < 1E-6) {
            signalTimeUnit = SIMTIME_NS;
            figure->setYAxisLabel("[ns]");
        }
        else if (signalMaxTime < 1E-3) {
            signalTimeUnit = SIMTIME_US;
            figure->setYAxisLabel("[us]");
        }
        else if (signalMaxTime < 1) {
            signalTimeUnit = SIMTIME_MS;
            figure->setYAxisLabel("[ms]");
        }
        else {
            signalTimeUnit = SIMTIME_S;
            figure->setYAxisLabel("[s]");
        }
        figure->setMinY(-signalMaxTime.dbl() / std::pow(10.0, signalTimeUnit));
        figure->setMaxY(signalMaxTime.dbl() / std::pow(10.0, signalTimeUnit));
        figure->setMinValue(minValue);
        figure->setMaxValue(maxValue);
        figure->clearValues();
        auto position = mobility->getCurrentPosition();
        if (!strcmp(spectrogramMode, "total"))
            refreshSpectrogramFigurePowerFunction(mediumPowerDensityFunction, position, signalTimeUnit, figure, 2);
        else if (!strcmp(spectrogramMode, "signal")) {
            if (transmissionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(transmissionInProgress->getId())->second;
                refreshSpectrogramFigurePowerFunction(signalPowerDensityFunction, position, signalTimeUnit, figure, 1);
            }
            else if (receptionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(receptionInProgress->getId())->second;
                refreshSpectrogramFigurePowerFunction(signalPowerDensityFunction, position, signalTimeUnit, figure, 1);
            }
        }
        else if (!strcmp(spectrogramMode, "auto")) {
            if (transmissionInProgress == nullptr && receptionInProgress == nullptr)
                refreshSpectrogramFigurePowerFunction(mediumPowerDensityFunction, position, signalTimeUnit, figure, 2);
            else if (transmissionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(transmissionInProgress->getId())->second;
                const auto& noisePowerDensityFunction = noisePowerDensityFunctions.find(transmissionInProgress->getId())->second;
                refreshSpectrogramFigurePowerFunction(noisePowerDensityFunction, position, signalTimeUnit, figure, 0);
                refreshSpectrogramFigurePowerFunction(signalPowerDensityFunction, position, signalTimeUnit, figure, 1);
            }
            else if (receptionInProgress != nullptr) {
                const auto& signalPowerDensityFunction = signalPowerDensityFunctions.find(receptionInProgress->getId())->second;
                const auto& noisePowerDensityFunction = noisePowerDensityFunctions.find(receptionInProgress->getId())->second;
                refreshSpectrogramFigurePowerFunction(noisePowerDensityFunction, position, signalTimeUnit, figure, 0);
                refreshSpectrogramFigurePowerFunction(signalPowerDensityFunction, position, signalTimeUnit, figure, 1);
            }
        }
        else
            throw cRuntimeError("Unknown signalFigureMode");
        figure->setXTickCount(spectrogramFigureXTickCount);
        figure->setYTickCount(spectrogramFigureYTickCount);
        figure->bakeValues();
        figure->refreshDisplay();
    }
}

void MediumCanvasVisualizer::refreshSpectrogramFigurePowerFunction(const Ptr<const IFunction<WpHz, Domain<m, m, m, simsec, Hz>>>& powerFunction, const Coord& position, SimTimeUnit signalTimeUnit, HeatMapPlotFigure *figure, int channel) const
{
    auto minTime = simTime() - signalMaxTime;
    auto maxTime = simTime() + signalMaxTime;
    auto marginFrequency = 0.05 * (signalMaxFrequency - signalMinFrequency);
    auto minFrequency = signalMinFrequency - marginFrequency;
    auto maxFrequency = signalMaxFrequency + marginFrequency;
    auto pixmapSize = figure->getPixmapSize();
    if (!strcmp(spectrogramPixelMode, "sample")) {
        const int xsize = pixmapSize.x;
        // NOTE: this method is fast, but it only get samples of the function at certain places, so it may result in incorrect colors
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (int x = 0; x < xsize; x++) {
            auto frequency = minFrequency + (maxFrequency - minFrequency) * x / pixmapSize.x;
            for (int y = 0; y < pixmapSize.y; y++) {
                auto time = minTime + (maxTime - minTime) * y / pixmapSize.y;
                Point<m, m, m, simsec, Hz> p(m(position.x), m(position.y), m(position.z), simsec(time), frequency);
                auto value = powerFunction->getValue(p);
                figure->setValue(x, y, wpHz2dBmWpMHz(WpHz(value).get()), channel);
            }
        }
    }
    else if (!strcmp(spectrogramPixelMode, "partition")) {
        Point<m, m, m, simsec, Hz> l(m(position.x), m(position.y), m(position.z), simsec(minTime), minFrequency);
        Point<m, m, m, simsec, Hz> u(m(position.x), m(position.y), m(position.z), simsec(maxTime), maxFrequency);
        Interval<m, m, m, simsec, Hz> i(l, u, 0b11100, 0b11100, 0b11100);
        // NOTE: this method is fast, but it may paint the same pixel several times, so the last will be the effective color
        powerFunction->partition(i, [&] (const Interval<m, m, m, simsec, Hz>& j, const IFunction<WpHz, Domain<m, m, m, simsec, Hz>> *partitonPowerFunction) {
            auto lower = j.getLower();
            auto upper = j.getUpper();
            auto x1 = GHz(std::get<4>(lower)).get();
            auto x2 = GHz(std::get<4>(upper)).get();
            auto y1 = (simsec(std::get<3>(lower)).get() - simTime()).dbl() / std::pow(10.0, signalTimeUnit);
            auto y2 = (simsec(std::get<3>(upper)).get() - simTime()).dbl() / std::pow(10.0, signalTimeUnit);
            if (auto cf = dynamic_cast<const ConstantFunction<WpHz, Domain<m, m, m, simsec, Hz>> *>(partitonPowerFunction)) {
                auto v = cf->getConstantValue() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(cf->getConstantValue()).get());
                figure->setConstantValue(x1, x2, y1, y2, v, channel);
            }
            else if (auto lf = dynamic_cast<const UnilinearFunction<WpHz, Domain<m, m, m, simsec, Hz>> *>(partitonPowerFunction)) {
                auto vl = lf->getRLower() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(lf->getRLower()).get());
                auto vu = lf->getRUpper() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(lf->getRUpper()).get());
                figure->setLinearValue(x1, x2, y1, y2, vl, vu, 4 - lf->getDimension(), channel);
            }
            else if (auto bf = dynamic_cast<const BilinearFunction<WpHz, Domain<m, m, m, simsec, Hz>> *>(partitonPowerFunction)) {
                auto v11 = bf->getRLowerLower() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(bf->getRLowerLower()).get());
                auto v21 = bf->getRLowerUpper() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(bf->getRLowerUpper()).get());
                auto v12 = bf->getRUpperLower() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(bf->getRUpperLower()).get());
                auto v22 = bf->getRUpperUpper() == WpHz(0) ? figure->getMinValue() : wpHz2dBmWpMHz(WpHz(bf->getRUpperUpper()).get());
                figure->setBilinearValue(x1, x2, y1, y2, v11, v21, v12, v22, 2);
            }
            else
                throw cRuntimeError("TODO");
        });
    }
    else if (!strcmp(spectrogramPixelMode, "mean")) {
        Point<m, m, m> point(m(position.x), m(position.y), m(position.z));
        auto fixedPowerFunction = makeShared<LeftCurryingFunction<WpHz, Domain<m, m, m>, 0b11100, Domain<simsec, Hz>, 0b11, Domain<m, m, m, simsec, Hz>>>(point, powerFunction);
        auto rasterizedFunction = makeShared<Rasterized2DFunction<WpHz, simsec, Hz>>(simsec(minTime), simsec(maxTime), pixmapSize.y + 1, minFrequency, maxFrequency, pixmapSize.x + 1, fixedPowerFunction);
        Point<simsec, Hz> lower(simsec(minTime), minFrequency);
        Point<simsec, Hz> upper(simsec(maxTime), maxFrequency);
        Interval<simsec, Hz> interval(lower, upper, 0b11, 0b00, 0b00);
        rasterizedFunction->partition(interval, [&] (const Interval<simsec, Hz>& i, const IFunction<WpHz, Domain<simsec, Hz>> *f) {
            if (auto cf = dynamic_cast<const ConstantFunction<WpHz, Domain<simsec, Hz>> *>(f)) {
                auto center = (i.getLower() + i.getUpper()) / 2;
                auto x = GHz(std::get<1>(center)).get();
                auto y = (simsec(std::get<0>(center)).get() - simTime()).dbl() / std::pow(10.0, signalTimeUnit);
                figure->setValue(x, y, wpHz2dBmWpMHz(WpHz(cf->getConstantValue()).get()), channel);
            }
            else
                throw cRuntimeError("Invalid partition function");
        });
    }
    else
        throw cRuntimeError("Unknown spectrogramPixelMode");
}

std::tuple<const ITransmission *, const ITransmission *, const IAntenna *, IMobility *> MediumCanvasVisualizer::extractSpectrumFigureParameters(const cModule *networkNode) const
{
    // TODO what about multiple radios? what if it's not called wlan, this is quite accidental, etc.?
    const IRadio *radio = dynamic_cast<const IRadio *>(networkNode);
    if (radio == nullptr) {
        auto wlan0 = networkNode->getSubmodule("wlan", 0);
        if (wlan0 != nullptr)
            radio = dynamic_cast<IRadio *>(wlan0->getSubmodule("radio"));
    }
    auto transmissionInProgress = radio != nullptr ? radio->getTransmissionInProgress() : nullptr;
    auto receptionInProgress = radio != nullptr ? radio->getReceptionInProgress() : nullptr;
    const IAntenna *antenna = nullptr;
    if (radio != nullptr)
        antenna = radio->getAntenna();
    if (antenna == nullptr)
        antenna = dynamic_cast<IAntenna *>(networkNode->getSubmodule("antenna"));
    IMobility *mobility = nullptr;
    if (antenna != nullptr)
        mobility = antenna->getMobility();
    else
        mobility = check_and_cast<IMobility *>(networkNode->getSubmodule("mobility"));
    return std::tuple<const ITransmission *, const ITransmission *, const IAntenna *, IMobility *> { transmissionInProgress, receptionInProgress, antenna, mobility };
}

void MediumCanvasVisualizer::setAnimationSpeed()
{
    SignalInProgress newSignalInProgress = SIP_NONE;
    double newSignalTransmissionAnimationSpeed = DBL_MAX;
    for (auto transmission : transmissions) {
        if (matchesTransmission(transmission)) {
            if (isSignalPropagationInProgress(transmission))
                newSignalInProgress = SIP_PROPAGATION;
            if (isSignalTransmissionInProgress(transmission)) {
                if (newSignalInProgress == SIP_NONE)
                    newSignalInProgress = SIP_TRANSMISSION;
                // TODO overwrite only...
                if (std::isnan(signalTransmissionAnimationSpeed))
                    newSignalTransmissionAnimationSpeed = std::min(newSignalTransmissionAnimationSpeed, transmission->getDuration().dbl() / signalTransmissionAnimationTime);
            }
        }
    }
    if (newSignalTransmissionAnimationSpeed != DBL_MAX)
        defaultSignalTransmissionAnimationSpeed = newSignalTransmissionAnimationSpeed;
    double currentSignalPropagationAnimationSpeed = std::isnan(signalPropagationAnimationSpeed) ? defaultSignalPropagationAnimationSpeed : signalPropagationAnimationSpeed;
    double currentSignalTransmissionAnimationSpeed = std::isnan(signalTransmissionAnimationSpeed) ? defaultSignalTransmissionAnimationSpeed : signalTransmissionAnimationSpeed;
    AnimationPosition currentPosition;
    if (lastSignalInProgress == SIP_NONE) {
        if (newSignalInProgress == SIP_NONE) {
            if (animationSpeedInterpolator.getCurrentAnimationSpeed() == animationSpeedInterpolator.getTargetAnimationSpeed())
                animationSpeedInterpolator.setAnimationSpeed(0);
        }
        else if (newSignalInProgress == SIP_PROPAGATION)
            animationSpeedInterpolator.setAnimationSpeed(currentSignalPropagationAnimationSpeed);
        else if (newSignalInProgress == SIP_TRANSMISSION)
            animationSpeedInterpolator.setAnimationSpeed(currentSignalTransmissionAnimationSpeed);
    }
    else if (lastSignalInProgress == SIP_PROPAGATION) {
        if (newSignalInProgress == SIP_NONE) {
            animationSpeedInterpolator.setCurrentAnimationSpeed(currentSignalPropagationAnimationSpeed);
            animationSpeedInterpolator.setTargetAnimationSpeed(signalAnimationSpeedChangeTimeMode, currentPosition.getTime(signalAnimationSpeedChangeTimeMode) + signalAnimationSpeedChangeTime, currentSignalTransmissionAnimationSpeed);
        }
        else if (newSignalInProgress == SIP_PROPAGATION)
            ; // void
        else if (newSignalInProgress == SIP_TRANSMISSION) {
            animationSpeedInterpolator.setCurrentAnimationSpeed(currentSignalPropagationAnimationSpeed);
            animationSpeedInterpolator.setTargetAnimationSpeed(signalAnimationSpeedChangeTimeMode, currentPosition.getTime(signalAnimationSpeedChangeTimeMode) + signalAnimationSpeedChangeTime, currentSignalTransmissionAnimationSpeed);
        }
    }
    else if (lastSignalInProgress == SIP_TRANSMISSION) {
        if (newSignalInProgress == SIP_NONE)
            animationSpeedInterpolator.setAnimationSpeed(0);
        else if (newSignalInProgress == SIP_PROPAGATION)
            animationSpeedInterpolator.setAnimationSpeed(currentSignalPropagationAnimationSpeed);
        else if (newSignalInProgress == SIP_TRANSMISSION)
            ; // void
    }
    lastSignalInProgress = newSignalInProgress;
    double animationSpeed = animationSpeedInterpolator.getCurrentAnimationSpeed();
    ASSERT(!std::isnan(animationSpeed));
    visualizationTargetModule->getCanvas()->setAnimationSpeed(animationSpeed, this);
}

cFigure *MediumCanvasVisualizer::getSignalDepartureFigure(const IRadio *radio) const
{
    auto it = signalDepartureFigures.find(radio->getId());
    return (it == signalDepartureFigures.end()) ? nullptr : it->second;
}

void MediumCanvasVisualizer::setSignalDepartureFigure(const IRadio *radio, cFigure *figure)
{
    signalDepartureFigures[radio->getId()] = figure;
}

cFigure *MediumCanvasVisualizer::removeSignalDepartureFigure(const IRadio *radio)
{
    auto it = signalDepartureFigures.find(radio->getId());
    if (it == signalDepartureFigures.end())
        return nullptr;
    else {
        signalDepartureFigures.erase(it);
        return it->second;
    }
}

cFigure *MediumCanvasVisualizer::getSignalArrivalFigure(const IRadio *radio) const
{
    auto it = signalArrivalFigures.find(radio->getId());
    return (it == signalArrivalFigures.end()) ? nullptr : it->second;
}

void MediumCanvasVisualizer::setSignalArrivalFigure(const IRadio *radio, cFigure *figure)
{
    signalArrivalFigures[radio->getId()] = figure;
}

cFigure *MediumCanvasVisualizer::removeSignalArrivalFigure(const IRadio *radio)
{
    auto it = signalArrivalFigures.find(radio->getId());
    if (it == signalArrivalFigures.end())
        return nullptr;
    else {
        signalArrivalFigures.erase(it);
        return it->second;
    }
}

cFigure *MediumCanvasVisualizer::getSignalFigure(const ITransmission *transmission) const
{
    auto it = signalFigures.find(transmission->getId());
    return (it == signalFigures.end()) ? nullptr : it->second;
}

void MediumCanvasVisualizer::setSignalFigure(const ITransmission *transmission, cFigure *figure)
{
    signalFigures[transmission->getId()] = figure;
}

cFigure *MediumCanvasVisualizer::removeSignalFigure(const ITransmission *transmission)
{
    auto it = signalFigures.find(transmission->getId());
    if (it == signalFigures.end())
        return nullptr;
    else {
        signalFigures.erase(it);
        return it->second;
    }
}

cGroupFigure *MediumCanvasVisualizer::createSignalFigure(const ITransmission *transmission) const
{
    cFigure::Point position = canvasProjection->computeCanvasPoint(transmission->getStartPosition());
    cGroupFigure *groupFigure = new cGroupFigure("signal");
    cFigure::Color color = signalColorSet.getColor(transmission->getId());
    SignalFigure *signalFigure = new SignalFigure("bubble");
    signalFigure->setTags((std::string("propagating_signal ") + tags).c_str());
    signalFigure->setTooltip("These rings represents a signal propagating through the medium");
    signalFigure->setAssociatedObject(const_cast<cObject *>(check_and_cast<const cObject *>(transmission)));
    signalFigure->setRingCount(signalRingCount);
    signalFigure->setRingSize(signalRingSize);
    signalFigure->setFadingDistance(signalFadingDistance);
    signalFigure->setFadingFactor(signalFadingFactor);
    signalFigure->setWaveCount(signalWaveCount);
    signalFigure->setWaveLength(signalWaveLength);
    signalFigure->setWaveWidth(signalWaveWidth);
    signalFigure->setOpacity(signalOpacity);
    signalFigure->setColor(color);
    signalFigure->setBounds(cFigure::Rectangle(position.x, position.y, 0, 0));
    signalFigure->refresh();
    groupFigure->addFigure(signalFigure);
    cLabelFigure *nameFigure = new cLabelFigure("packet name");
    nameFigure->setPosition(position);
    nameFigure->setTags((std::string("propagating_signal packet_name label ") + tags).c_str());
    auto packet = transmission->getPacket();
    if (packet != nullptr)
        nameFigure->setText(packet->getName());
    nameFigure->setColor(color);
    groupFigure->addFigure(nameFigure);
    return groupFigure;
}

void MediumCanvasVisualizer::refreshSignalFigure(const ITransmission *transmission) const
{
    const IPropagation *propagation = radioMedium->getPropagation();
    cFigure *groupFigure = getSignalFigure(transmission);
    double startRadius = propagation->getPropagationSpeed().get() * (simTime() - transmission->getStartTime()).dbl();
    double endRadius = std::max(0.0, propagation->getPropagationSpeed().get() * (simTime() - transmission->getEndTime()).dbl());
    if (groupFigure) {
        SignalFigure *signalFigure = static_cast<SignalFigure *>(groupFigure->getFigure(0));
        cLabelFigure *labelFigure = static_cast<cLabelFigure *>(groupFigure->getFigure(1));
        double phi = transmission->getId();
        labelFigure->setTransform(cFigure::Transform().translate(endRadius * sin(phi), endRadius * cos(phi)));
        const Coord& transmissionStart = transmission->getStartPosition();
        // KLUDGE to workaround overflow bugs in drawing, Tkenv?
        double offset = std::fmod(startRadius, signalFigure->getWaveLength());
//        if (startRadius > 10000)
//            startRadius = 10000;
//        if (endRadius > 10000)
//            endRadius = 10000;
        switch (signalShape) {
            case SIGNAL_SHAPE_RING: {
                // determine the rotated 2D canvas points by computing the 2D affine transformation from the 3D transformation of the environment
                cFigure::Point o = canvasProjection->computeCanvasPoint(transmissionStart);
                cFigure::Point x = canvasProjection->computeCanvasPoint(transmissionStart + Coord(1, 0, 0));
                cFigure::Point y = canvasProjection->computeCanvasPoint(transmissionStart + Coord(0, 1, 0));
                double t1 = o.x;
                double t2 = o.y;
                double a = x.x - t1;
                double b = x.y - t2;
                double c = y.x - t1;
                double d = y.y - t2;
                signalFigure->setTransform(cFigure::Transform(a, b, c, d, t1, t2));
                signalFigure->setBounds(cFigure::Rectangle(-startRadius, -startRadius, startRadius * 2, startRadius * 2));
                signalFigure->setInnerRx(endRadius);
                signalFigure->setInnerRy(endRadius);
                signalFigure->setWaveOffset(offset);
                double currentSignalPropagationAnimationSpeed = std::isnan(signalPropagationAnimationSpeed) ? defaultSignalPropagationAnimationSpeed : signalPropagationAnimationSpeed;
                signalFigure->setWaveOpacityFactor(std::min(1.0, currentSignalPropagationAnimationSpeed / getSimulation()->getEnvir()->getAnimationSpeed() / signalWaveFadingAnimationSpeedFactor));
                signalFigure->refresh();
                break;
            }
            case SIGNAL_SHAPE_SPHERE: {
                // a sphere looks like a circle from any view angle
                cFigure::Point center = canvasProjection->computeCanvasPoint(transmissionStart);
                signalFigure->setBounds(cFigure::Rectangle(center.x - startRadius, center.y - startRadius, 2 * startRadius, 2 * startRadius));
                signalFigure->setInnerRx(endRadius);
                signalFigure->setInnerRy(endRadius);
                break;
            }
            default:
                throw cRuntimeError("Unimplemented signal shape");
        }
    }
}

void MediumCanvasVisualizer::handleRadioAdded(const IRadio *radio)
{
    Enter_Method("handleRadioAdded");
    auto module = check_and_cast<const cModule *>(radio);
    auto networkNode = getContainingNode(module);
    if (networkNodeFilter.matches(networkNode)) {
        invalidDisplay = true;
        auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
        if (displayInterferenceRanges || (module->hasPar("displayInterferenceRange") && module->par("displayInterferenceRange"))) {
            auto interferenceRangeFigure = new cOvalFigure("interferenceRange");
            m maxInterferenceRange = check_and_cast<const IRadioMedium *>(radio->getMedium())->getMediumLimitCache()->getMaxInterferenceRange(radio);
            interferenceRangeFigure->setTags((std::string("interference_range ") + tags).c_str());
            interferenceRangeFigure->setTooltip("This circle represents the interference range of a wireless interface");
            interferenceRangeFigure->setBounds(cFigure::Rectangle(-maxInterferenceRange.get(), -maxInterferenceRange.get(), 2 * maxInterferenceRange.get(), 2 * maxInterferenceRange.get()));
            interferenceRangeFigure->setLineColor(interferenceRangeLineColor);
            interferenceRangeFigure->setLineStyle(interferenceRangeLineStyle);
            interferenceRangeFigure->setLineWidth(interferenceRangeLineWidth);
            networkNodeVisualization->addFigure(interferenceRangeFigure);
        }
        if (displayCommunicationRanges || (module->hasPar("displayCommunicationRange") && module->par("displayCommunicationRange"))) {
            auto communicationRangeFigure = new cOvalFigure("communicationRange");
            m maxCommunicationRange = check_and_cast<const IRadioMedium *>(radio->getMedium())->getMediumLimitCache()->getMaxCommunicationRange(radio);
            communicationRangeFigure->setTags((std::string("communication_range ") + tags).c_str());
            communicationRangeFigure->setTooltip("This circle represents the communication range of a wireless interface");
            communicationRangeFigure->setBounds(cFigure::Rectangle(-maxCommunicationRange.get(), -maxCommunicationRange.get(), 2 * maxCommunicationRange.get(), 2 * maxCommunicationRange.get()));
            communicationRangeFigure->setLineColor(communicationRangeLineColor);
            communicationRangeFigure->setLineStyle(communicationRangeLineStyle);
            communicationRangeFigure->setLineWidth(communicationRangeLineWidth);
            networkNodeVisualization->addFigure(communicationRangeFigure);
        }
        if (displaySignalDepartures || displaySignalArrivals) {
            if (displaySignalDepartures) {
                std::string imageName = par("signalDepartureImage");
                auto signalDepartureFigure = new LabeledIconFigure("signalDeparture");
                signalDepartureFigure->setTags((std::string("signal_departure ") + tags).c_str());
                signalDepartureFigure->setTooltip("This icon represents an ongoing signal departure");
                signalDepartureFigure->setVisible(false);
                auto iconFigure = signalDepartureFigure->getIconFigure();
                iconFigure->setImageName(imageName.substr(0, imageName.find_first_of(".")).c_str());
                iconFigure->setAnchor(cFigure::ANCHOR_NW);
                auto labelFigure = signalDepartureFigure->getLabelFigure();
                labelFigure->setPosition(iconFigure->getBounds().getSize() / 2);
                networkNodeVisualization->addAnnotation(signalDepartureFigure, signalDepartureFigure->getBounds().getSize(), signalDeparturePlacementHint, signalDeparturePlacementPriority);
                setSignalDepartureFigure(radio, signalDepartureFigure);
            }
            if (displaySignalArrivals) {
                std::string imageName = par("signalArrivalImage");
                auto signalArrivalFigure = new LabeledIconFigure("signalArrival");
                signalArrivalFigure->setTags((std::string("signal_arrival ") + tags).c_str());
                signalArrivalFigure->setTooltip("This icon represents an ongoing signal arrival");
                signalArrivalFigure->setVisible(false);
                auto iconFigure = signalArrivalFigure->getIconFigure();
                iconFigure->setImageName(imageName.substr(0, imageName.find_first_of(".")).c_str());
                iconFigure->setAnchor(cFigure::ANCHOR_NW);
                auto labelFigure = signalArrivalFigure->getLabelFigure();
                labelFigure->setPosition(iconFigure->getBounds().getSize() / 2);
                networkNodeVisualization->addAnnotation(signalArrivalFigure, signalArrivalFigure->getBounds().getSize(), signalArrivalPlacementHint, signalArrivalPlacementPriority);
                setSignalArrivalFigure(radio, signalArrivalFigure);
            }
        }
        if (displayPowerDensityMaps || displaySpectrums || displaySpectrograms)
            networkNode->subscribe(IMobility::mobilityStateChangedSignal, this);
    }
}

void MediumCanvasVisualizer::handleRadioRemoved(const IRadio *radio)
{
    Enter_Method("handleRadioRemoved");
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
    auto networkNode = getContainingNode(module);
    if (networkNodeFilter.matches(networkNode)) {
        invalidDisplay = true;
        auto departureFigure = removeSignalDepartureFigure(radio);
        if (departureFigure != nullptr) {
            auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
            networkNodeVisualization->removeAnnotation(departureFigure);
        }
        auto arrivalFigure = removeSignalArrivalFigure(radio);
        if (arrivalFigure != nullptr) {
            auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
            networkNodeVisualization->removeAnnotation(arrivalFigure);
        }
        if (displayPowerDensityMaps || displaySpectrums || displaySpectrograms)
            networkNode->unsubscribe(IMobility::mobilityStateChangedSignal, this);
    }
}

void MediumCanvasVisualizer::handleSignalAdded(const ITransmission *transmission)
{
    Enter_Method("handleSignalAdded");
    MediumVisualizerBase::handleSignalAdded(transmission);
    if (matchesTransmission(transmission)) {
        invalidDisplay = true;
        transmissions.push_back(transmission);
        if (displaySignals) {
            cGroupFigure *signalFigure = createSignalFigure(transmission);
            signalLayer->addFigure(signalFigure);
            setSignalFigure(transmission, signalFigure);
        }
        if (displaySignals || displayMainPowerDensityMap || displayPowerDensityMaps)
            setAnimationSpeed();
    }
}

void MediumCanvasVisualizer::handleSignalRemoved(const ITransmission *transmission)
{
    Enter_Method("handleSignalRemoved");
    MediumVisualizerBase::handleSignalRemoved(transmission);
    if (matchesTransmission(transmission)) {
        invalidDisplay = true;
        transmissions.erase(std::remove(transmissions.begin(), transmissions.end(), transmission));
        if (displaySignals) {
            cFigure *signalFigure = getSignalFigure(transmission);
            removeSignalFigure(transmission);
            if (signalFigure != nullptr)
                delete signalLayer->removeFigure(signalFigure);
        }
        if (displaySignals || displayMainPowerDensityMap || displayPowerDensityMaps)
            setAnimationSpeed();
    }
}

void MediumCanvasVisualizer::handleSignalDepartureStarted(const ITransmission *transmission)
{
    Enter_Method("handleSignalDepartureStarted");
    if (matchesTransmission(transmission)) {
        invalidDisplay = true;
        if (displaySignals || displayMainPowerDensityMap || displayPowerDensityMaps)
            setAnimationSpeed();
        if (displaySignalDepartures) {
            auto transmitter = transmission->getTransmitter();
            if (!transmitter) return;
            auto figure = getSignalDepartureFigure(transmitter);
            auto networkNode = getContainingNode(check_and_cast<const cModule *>(transmitter));
            auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
            networkNodeVisualization->setAnnotationVisible(figure, true);
            auto labelFigure = check_and_cast<LabeledIconFigure *>(figure)->getLabelFigure();
            if (auto scalarTransmission = dynamic_cast<const ScalarTransmission *>(transmission)) {
                char tmp[32];
                sprintf(tmp, "%.4g dBW", fraction2dB(W(scalarTransmission->getPower()).get()));
                labelFigure->setText(tmp);
            }
            else
                labelFigure->setText("");
        }
    }
}

void MediumCanvasVisualizer::handleSignalDepartureEnded(const ITransmission *transmission)
{
    Enter_Method("handleSignalDepartureEnded");
    if (matchesTransmission(transmission)) {
        invalidDisplay = true;
        if (displaySignals || displayMainPowerDensityMap || displayPowerDensityMaps)
            setAnimationSpeed();
        if (displaySignalDepartures) {
            auto transmitter = transmission->getTransmitter();
            if (!transmitter) return;
            auto figure = getSignalDepartureFigure(transmitter);
            auto networkNode = getContainingNode(check_and_cast<const cModule *>(transmitter));
            auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
            networkNodeVisualization->setAnnotationVisible(figure, false);
        }
    }
}

void MediumCanvasVisualizer::handleSignalArrivalStarted(const IReception *reception)
{
    Enter_Method("handleSignalArrivalStarted");
    MediumVisualizerBase::handleSignalArrivalStarted(reception);
    if (matchesTransmission(reception->getTransmission())) {
        invalidDisplay = true;
        if (displaySignals || displayMainPowerDensityMap || displayPowerDensityMaps)
            setAnimationSpeed();
        if (displaySignalArrivals) {
            auto receiver = reception->getReceiver();
            if (networkNodeFilter.matches(check_and_cast<const cModule *>(receiver))) {
                auto figure = getSignalArrivalFigure(receiver);
                auto networkNode = getContainingNode(check_and_cast<const cModule *>(receiver));
                auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
                networkNodeVisualization->setAnnotationVisible(figure, true);
                auto labelFigure = check_and_cast<LabeledIconFigure *>(figure)->getLabelFigure();
                if (auto scalarReception = dynamic_cast<const ScalarReception *>(reception)) {
                    char tmp[32];
                    sprintf(tmp, "%.4g dBW", fraction2dB(W(scalarReception->getPower()).get()));
                    labelFigure->setText(tmp);
                }
                else
                    labelFigure->setText("");
            }
        }
        if (displayCommunicationHeat) {
            const ITransmission *transmission = reception->getTransmission();
            const IMediumLimitCache *mediumLimitCache = radioMedium->getMediumLimitCache();
            Coord min = mediumLimitCache->getMinConstraintArea();
            Coord max = mediumLimitCache->getMaxConstraintArea();
            Coord delta = max - min;
            int x1 = std::round((communicationHeatMapSize - 1) * ((transmission->getStartPosition().x - min.x) / delta.x));
            int y1 = std::round((communicationHeatMapSize - 1) * ((transmission->getStartPosition().y - min.x) / delta.y));
            int x2 = std::round((communicationHeatMapSize - 1) * ((reception->getStartPosition().x - min.x) / delta.x));
            int y2 = std::round((communicationHeatMapSize - 1) * ((reception->getStartPosition().y - min.y) / delta.y));
            communicationHeat->heatLine(x1, y1, x2, y2);
        }
    }
}

void MediumCanvasVisualizer::handleSignalArrivalEnded(const IReception *reception)
{
    Enter_Method("handleSignalArrivalEnded");
    if (matchesTransmission(reception->getTransmission())) {
        invalidDisplay = true;
        if (displaySignals || displayMainPowerDensityMap || displayPowerDensityMaps)
            setAnimationSpeed();
        if (displaySignalArrivals) {
            auto receiver = reception->getReceiver();
            if (networkNodeFilter.matches(check_and_cast<const cModule *>(receiver))) {
                auto figure = getSignalArrivalFigure(receiver);
                auto networkNode = getContainingNode(check_and_cast<const cModule *>(receiver));
                auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
                networkNodeVisualization->setAnnotationVisible(figure, false);
            }
        }
    }
}

void MediumCanvasVisualizer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == IMobility::mobilityStateChangedSignal) {
        invalidDisplay = true;
    }
    else
        MediumVisualizerBase::receiveSignal(source, signal, object, details);
}

#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

} // namespace visualizer
} // namespace inet

