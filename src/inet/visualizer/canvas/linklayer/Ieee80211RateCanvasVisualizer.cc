//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/linklayer/Ieee80211RateCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

namespace visualizer {

Define_Module(Ieee80211RateCanvasVisualizer);

Ieee80211RateCanvasVisualizer::Ieee80211RateCanvasVisualization::Ieee80211RateCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, cGroupFigure *figure, int networkNodeId, int interfaceId) :
    Ieee80211RateVisualization(networkNodeId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

Ieee80211RateCanvasVisualizer::Ieee80211RateCanvasVisualization::~Ieee80211RateCanvasVisualization()
{
    delete figure;
}

void Ieee80211RateCanvasVisualizer::initialize(int stage)
{
    Ieee80211RateVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

Ieee80211RateVisualizerBase::Ieee80211RateVisualization *Ieee80211RateCanvasVisualizer::createRateVisualization(cModule *networkNode, NetworkInterface *networkInterface)
{
    auto figure = new cGroupFigure("ieee80211Rates");
    figure->setTags((std::string("ieee80211_rate ") + tags).c_str());
    figure->setZIndex(zIndex);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    auto canvasVisualization = new Ieee80211RateCanvasVisualization(networkNodeVisualization, figure, networkNode->getId(), networkInterface->getInterfaceId());
    canvasVisualization->title = networkInterface->getInterfaceName();
    return canvasVisualization;
}

void Ieee80211RateCanvasVisualizer::addRateVisualization(Ieee80211RateVisualization *rateVisualization)
{
    Ieee80211RateVisualizerBase::addRateVisualization(rateVisualization);
    auto canvasVisualization = static_cast<Ieee80211RateCanvasVisualization *>(rateVisualization);
    if (canvasVisualization->networkNodeVisualization != nullptr)
        canvasVisualization->networkNodeVisualization->addAnnotation(canvasVisualization->figure, cFigure::Rectangle(0, 0, 0, 0), placementHint, placementPriority);
}

void Ieee80211RateCanvasVisualizer::removeRateVisualization(Ieee80211RateVisualization *rateVisualization)
{
    Ieee80211RateVisualizerBase::removeRateVisualization(rateVisualization);
    auto canvasVisualization = static_cast<Ieee80211RateCanvasVisualization *>(rateVisualization);
    if (networkNodeVisualizer != nullptr && canvasVisualization->networkNodeVisualization != nullptr)
        canvasVisualization->networkNodeVisualization->removeAnnotation(canvasVisualization->figure);
}

void Ieee80211RateCanvasVisualizer::refreshChart(Ieee80211RateCanvasVisualization *canvasVisualization) const
{
    auto figure = canvasVisualization->figure;
    // clear previous content
    while (figure->getNumFigures() > 0)
        delete figure->removeFigure(0);

    // order stations by name for a stable layout
    std::vector<const RateEntry *> entries;
    for (auto& kv : canvasVisualization->rates)
        entries.push_back(&kv.second);
    std::sort(entries.begin(), entries.end(), [](const RateEntry *a, const RateEntry *b) {
        return a->staName < b->staName;
    });

    double titleHeight = displayTitle ? 12 : 0;
    double rateLabelHeight = 10; // space reserved above the bars for the rate labels
    double baselineY = titleHeight + rateLabelHeight + maxBarHeight;
    double chartWidth = entries.empty() ? 0 : (entries.size() * barWidth + (entries.size() - 1) * barSpacing);

    if (displayTitle) {
        auto titleFigure = new cLabelFigure("title");
        titleFigure->setText(canvasVisualization->title.c_str());
        titleFigure->setFont(titleFont);
        titleFigure->setColor(titleColor);
        titleFigure->setAnchor(cFigure::ANCHOR_N);
        titleFigure->setPosition(cFigure::Point(chartWidth / 2, 0));
        figure->addFigure(titleFigure);
    }

    if (!entries.empty()) {
        // baseline
        auto baseline = new cLineFigure("baseline");
        baseline->setStart(cFigure::Point(-1, baselineY));
        baseline->setEnd(cFigure::Point(chartWidth + 1, baselineY));
        baseline->setLineColor(cFigure::Color("grey50"));
        figure->addFigure(baseline);
    }

    for (size_t i = 0; i < entries.size(); i++) {
        const RateEntry *entry = entries[i];
        double x = i * (barWidth + barSpacing);
        double fraction = maxRate > 0 ? entry->bitrate / maxRate : 0;
        if (fraction < 0) fraction = 0;
        if (fraction > 1) fraction = 1;
        double h = maxBarHeight * fraction;

        auto bar = new cRectangleFigure("bar");
        bar->setBounds(cFigure::Rectangle(x, baselineY - h, barWidth, h));
        bar->setFilled(true);
        bar->setFillColor(getRateColor(entry->bitrate));
        bar->setLineColor(cFigure::Color("grey30"));
        bar->setTooltip((std::string("Data rate to ") + entry->staName + ": " + formatRate(entry->bitrate) + (entry->observed ? " (observed)" : " (configured)")).c_str());
        figure->addFigure(bar);

        // rate label centered above the bar
        auto rateLabel = new cLabelFigure("rate");
        rateLabel->setText(formatRate(entry->bitrate).c_str());
        rateLabel->setFont(rateLabelFont);
        rateLabel->setColor(rateLabelColor);
        rateLabel->setAnchor(cFigure::ANCHOR_S);
        rateLabel->setPosition(cFigure::Point(x + barWidth / 2, baselineY - h - 1));
        figure->addFigure(rateLabel);

        // station name below the baseline, rotated
        auto nameLabel = new cLabelFigure("station");
        nameLabel->setText(entry->staName.c_str());
        nameLabel->setFont(stationLabelFont);
        nameLabel->setColor(stationLabelColor);
        nameLabel->setAnchor(cFigure::ANCHOR_NE);
        nameLabel->setAngle(nameRotation);
        nameLabel->setPosition(cFigure::Point(x + barWidth / 2, baselineY + 2));
        figure->addFigure(nameLabel);
    }

    // Estimate the bounding box of the chart (in the group's local coordinates). The rotated
    // station name labels hang below and to the left of the baseline; reserve space for them.
    double longestName = 0;
    for (auto entry : entries)
        longestName = std::max(longestName, (double)entry->staName.length());
    double nameExtent = longestName * 4 + 4; // rough per-character estimate for the small font
    double bottom = baselineY + 4 + std::max(6.0, nameExtent * std::abs(std::sin(nameRotation)));
    double left = -std::max(2.0, nameExtent * std::abs(std::cos(nameRotation)));
    canvasVisualization->bounds = cFigure::Rectangle(left, 0, (chartWidth - left) + 2, bottom);
}

void Ieee80211RateCanvasVisualizer::refreshDisplay() const
{
    VisualizerBase::refreshDisplay();
    auto simulation = getSimulation();
    L3AddressResolver addressResolver;
    for (auto& entry : ieee80211RateVisualizations) {
        auto canvasVisualization = static_cast<Ieee80211RateCanvasVisualization *>(entry.second);
        auto networkNode = simulation->getModule(canvasVisualization->networkNodeId);
        if (networkNode == nullptr)
            continue;
        auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
        if (interfaceTable == nullptr)
            continue;
        auto networkInterface = interfaceTable->getInterfaceById(canvasVisualization->interfaceId);
        if (networkInterface == nullptr)
            continue;
        refreshRateEntries(networkNode, networkInterface, canvasVisualization);
        refreshChart(canvasVisualization);
        if (canvasVisualization->networkNodeVisualization != nullptr)
            canvasVisualization->networkNodeVisualization->setAnnotationSize(canvasVisualization->figure, canvasVisualization->bounds.getSize());
    }
}

} // namespace visualizer

} // namespace inet
