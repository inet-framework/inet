//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RATEVISUALIZERBASE_H
#define __INET_IEEE80211RATEVISUALIZERBASE_H

#include <map>
#include <string>
#include <vector>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ColorSet.h"
#include "inet/visualizer/util/InterfaceFilter.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

class INET_API Ieee80211RateVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    // Per-station rate entry maintained for an access point interface.
    class INET_API RateEntry {
      public:
        double bitrate = NaN; // most recent PHY data rate towards the station, in bps
        int staNodeId = -1; // network node id of the station, or -1 if not yet resolved
        std::string staName; // display label for the station (node name or MAC address)
        simtime_t lastUpdate = -1; // time of the last observed frame, negative for configured-only entries
        bool observed = false; // true if the rate came from an observed frame (vs. configured fallback)
    };

    // A rate visualization keeps the per-station rate entries for one access point interface.
    class INET_API Ieee80211RateVisualization {
      public:
        const int networkNodeId = -1;
        const int interfaceId = -1;
        std::map<MacAddress, RateEntry> rates;

      public:
        Ieee80211RateVisualization(int networkNodeId, int interfaceId);
        virtual ~Ieee80211RateVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayRates = false;
    NetworkNodeFilter nodeFilter;
    InterfaceFilter interfaceFilter;
    double maxRate = NaN; // bps
    double barWidth = NaN;
    double barSpacing = NaN;
    double maxBarHeight = NaN;
    std::vector<cFigure::Color> rateColors;
    std::string rateFormat;
    cFigure::Font rateLabelFont;
    cFigure::Color rateLabelColor;
    cFigure::Font stationLabelFont;
    cFigure::Color stationLabelColor;
    double nameRotation = NaN; // radians
    bool displayTitle = true;
    cFigure::Font titleFont;
    cFigure::Color titleColor;
    simtime_t holdTime;
    Placement placementHint;
    double placementPriority = 0;
    //@}

    std::map<std::pair<int, int>, Ieee80211RateVisualization *> ieee80211RateVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual Ieee80211RateVisualization *createRateVisualization(cModule *networkNode, NetworkInterface *networkInterface) = 0;
    virtual Ieee80211RateVisualization *getRateVisualization(int networkNodeId, int interfaceId);
    virtual void addRateVisualization(Ieee80211RateVisualization *rateVisualization);
    virtual void removeRateVisualization(Ieee80211RateVisualization *rateVisualization);
    virtual void removeAllRateVisualizations();

    // Records an observed rate for the given (access point node, interface, station).
    virtual void addOrUpdateObservedRate(cModule *networkNode, NetworkInterface *networkInterface, const MacAddress& stationAddress, double bitrate);

    // Merges configured per-receiver rates for the access point interface into the visualization,
    // and prunes stale observed entries according to holdTime.
    virtual void refreshRateEntries(cModule *networkNode, NetworkInterface *networkInterface, Ieee80211RateVisualization *rateVisualization) const;

    // Creates a visualization for each interface (matching the filters) that has configured
    // per-receiver rates (dataFrameBitratePerReceiver) but no visualization yet, so the chart
    // appears immediately from the configured rates, before any traffic.
    virtual void ensureConfiguredVisualizations();

    // Resolves a station MAC address to a network node (for labeling); returns nullptr if unknown.
    virtual cModule *findNetworkNodeByMacAddress(const MacAddress& address) const;

    // Formats a bitrate (bps) into a rate label using rateFormat.
    virtual std::string formatRate(double bitrate) const;

    // Interpolates the bar color for a bitrate on the rateColors gradient.
    virtual cFigure::Color getRateColor(double bitrate) const;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif
