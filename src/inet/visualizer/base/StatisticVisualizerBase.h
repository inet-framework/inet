//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATISTICVISUALIZERBASE_H
#define __INET_STATISTICVISUALIZERBASE_H

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "inet/common/StringFormat.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/ModuleFilter.h"
#include "inet/visualizer/util/Placement.h"

namespace inet {

namespace visualizer {

class INET_API StatisticVisualizerBase : public VisualizerBase, public cListener
{
  public:
    class INET_API LastValueRecorder : public cNumericResultRecorder {
      protected:
        double lastValue = NaN;

      protected:
        virtual void collect(simtime_t_cref t, double value, cObject *details) override { lastValue = value; }

      public:
        void setLastValue(double lastValue) { this->lastValue = lastValue; }
        double getLastValue() const { return lastValue; }
    };

    class INET_API StatisticVisualization {
      public:
        LastValueRecorder *recorder = nullptr;
        const int moduleId = -1;
        const simsignal_t signal = -1;
        const char *unit = nullptr;
        mutable double printValue = NaN;
        mutable const char *printUnit = nullptr;

      public:
        StatisticVisualization(int moduleId, simsignal_t signal, const char *unit);
        virtual ~StatisticVisualization() {}
    };

    // A bar-chart visualization for one signal source: the last value of the signal for each distinct
    // demux label (the full name of the details object emitted with the value) becomes one bar. This is
    // the live counterpart of the demux() result filter used for recording.
    class INET_API BarSetVisualization {
      public:
        const int networkNodeId = -1;
        const int moduleId = -1;
        std::string title;
        std::map<std::string, double> values; // series label -> last value
        std::map<std::string, LastValueRecorder *> recorders; // series label -> recorder (barSeries=="sources" only)

      public:
        BarSetVisualization(int networkNodeId, int moduleId);
        virtual ~BarSetVisualization() {}
    };

    class INET_API DirectiveResolver : public StringFormat::IResolver {
      protected:
        const StatisticVisualizerBase *visualizer = nullptr;
        const StatisticVisualization *visualization = nullptr;

      public:
        DirectiveResolver(const StatisticVisualizerBase *visualizer, const StatisticVisualization *visualization) : visualizer(visualizer), visualization(visualization) {}

        virtual std::string resolveDirective(char directive) const override;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayStatistics = false;
    ModuleFilter sourceFilter;
    const char *signalName = nullptr;
    const char *statisticName = nullptr;
    const char *statisticUnit = nullptr;
    const char *statisticExpression = nullptr;
    StringFormat format;
    std::vector<std::string> units;
    cFigure::Font font;
    cFigure::Color textColor;
    cFigure::Color backgroundColor;
    double opacity = NaN;
    Placement placementHint;
    double placementPriority;
    //@}

    /** @name Bar chart mode parameters (displayMode == "bars") */
    //@{
    bool barChartMode = false;
    bool barSeriesBySources = false; // barSeries=="sources": one bar per matching source module (grouped by node), value from a recorder; else demux one source's details
    double maxValue = NaN;
    double minValue = NaN;
    double barWidth = NaN;
    double barSpacing = NaN;
    double maxBarHeight = NaN;
    std::vector<cFigure::Color> barColors;
    std::string valueFormat;
    cFigure::Font valueLabelFont;
    cFigure::Color valueLabelColor;
    cFigure::Font seriesLabelFont;
    cFigure::Color seriesLabelColor;
    double nameRotation = NaN; // radians
    bool displayTitle = true;
    cFigure::Font titleFont;
    cFigure::Color titleColor;
    //@}

    std::map<std::pair<int, simsignal_t>, const StatisticVisualization *> statisticVisualizations;
    std::map<int, BarSetVisualization *> barSetVisualizations; // keyed by source module id (details mode) or network node id (sources mode)
    std::set<int> groupedBarSourceIds; // source modules already registered in sources mode

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual void addResultRecorder(cComponent *source, simsignal_t signal);
    virtual LastValueRecorder *getResultRecorder(cComponent *source, simsignal_t signal);
    virtual LastValueRecorder *findResultRecorder(cResultListener *resultListener);
    virtual std::string getText(const StatisticVisualization *statisticVisualization);
    virtual const char *getUnit(cComponent *source);
    virtual std::string getRecordingMode();

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) = 0;
    virtual const StatisticVisualization *getStatisticVisualization(cComponent *source, simsignal_t signal);
    virtual void addStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void removeStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void removeAllStatisticVisualizations();

    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void processSignal(cComponent *source, simsignal_t signal, std::function<void(cIListener *)> receiveSignal);

    /** @name Bar chart mode */
    //@{
    // Creates the bar-chart visualization for one source (the concrete subclass builds the figure);
    // returns nullptr if bar charts are not supported (e.g. the osg visualizer).
    virtual BarSetVisualization *createBarSetVisualization(cComponent *source) { return nullptr; }
    virtual BarSetVisualization *getBarSetVisualization(int moduleId);
    virtual void addBarSetVisualization(BarSetVisualization *barSetVisualization);
    virtual void removeBarSetVisualization(BarSetVisualization *barSetVisualization);
    virtual void removeAllBarSetVisualizations();
    // Records the last value of a signal for a (source, demux label) pair into its bar set.
    virtual void processBarValue(cComponent *source, double value, cObject *details);
    // Registers a source module as one bar in its network node's bar set (barSeries=="sources"): attaches a
    // result recorder (using statisticExpression, e.g. count or throughput) whose value the bar shows.
    virtual void processGroupedBarSource(cComponent *source, simsignal_t signal);
    // Refreshes each sources-mode bar set's values from its per-source recorders (called before rendering).
    virtual void refreshGroupedBarValues();
    // Formats a bar value into its label using valueFormat.
    virtual std::string formatBarValue(double value) const;
    // Interpolates the bar color for a value on the barColors gradient (minValue..maxValue).
    virtual cFigure::Color getBarColor(double value) const;
    //@}

  public:
#define PROCESS_SIGNAL(value) { processSignal(source, signal, [=] (cIListener *listener) { listener->receiveSignal(source, signal, value, details); }); }
#define DISPATCH_NUMERIC(value) { if (barSeriesBySources) processGroupedBarSource(source, signal); else if (barChartMode) processBarValue(source, (double)(value), details); else PROCESS_SIGNAL(value); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, bool b, cObject *details) override { DISPATCH_NUMERIC(b); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t l, cObject *details) override { DISPATCH_NUMERIC(l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, uintval_t l, cObject *details) override { DISPATCH_NUMERIC(l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double d, cObject *details) override { DISPATCH_NUMERIC(d); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const SimTime& t, cObject *details) override { if (barSeriesBySources) processGroupedBarSource(source, signal); else if (barChartMode) processBarValue(source, t.dbl(), details); else PROCESS_SIGNAL(t); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const char *s, cObject *details) override { if (barSeriesBySources) processGroupedBarSource(source, signal); else if (!barChartMode) PROCESS_SIGNAL(s); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override { if (barSeriesBySources) processGroupedBarSource(source, signal); else if (!barChartMode) PROCESS_SIGNAL(obj); }
};

} // namespace visualizer

} // namespace inet

#endif

