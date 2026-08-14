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
    // Determines whether the values a signal source emits are split into several
    // statistics, and what identifies each of them.
    enum SplitMode {
        SPLIT_NONE, // the source emits the values of a single statistic
        SPLIT_DETAILS, // one statistic per distinct details object emitted with the value
        SPLIT_FLOW, // one statistic per packet flow of the source, demultiplexed by the flow tag
    };

    // Determines which statistics are displayed together as the items of a single
    // figure, which then needs to be one that displays several items, e.g. a bar chart.
    enum GroupMode {
        GROUP_NONE, // each statistic is displayed on a figure of its own
        GROUP_SOURCE, // the statistics of one signal source are displayed together
        GROUP_NETWORK_NODE, // the statistics of the signal sources of one network node are displayed together
    };

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

    // A visualization that displays the statistics of one group at once, one per item
    // of its figure, keyed by the item label. The set of items is not known in advance,
    // they are created as their labels appear. This is the live counterpart of the
    // demux() result filter used for recording.
    class INET_API GroupVisualization {
      public:
        const int moduleId = -1; // the module the group belongs to: the signal source, or the network node when grouping by network node
        std::map<std::string, double> values; // item label -> last value
        std::map<std::string, LastValueRecorder *> recorders; // item label -> recorder (only when grouping by network node)

      public:
        GroupVisualization(int moduleId);
        virtual ~GroupVisualization() {}
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
    simsignal_t subscribedSignal = SIMSIGNAL_NULL; // the signal named by signalName, resolved once
    const char *statisticName = nullptr;
    const char *statisticUnit = nullptr;
    const char *statisticExpression = nullptr;
    SplitMode splitMode = SPLIT_NONE;
    GroupMode groupMode = GROUP_NONE;
    StringFormat format;
    std::vector<std::string> units;
    cFigure::Font font;
    cFigure::Color textColor;
    cFigure::Color backgroundColor;
    double opacity = NaN;
    Placement placementHint;
    double placementPriority;
    //@}

    std::map<std::pair<int, simsignal_t>, const StatisticVisualization *> statisticVisualizations;
    std::map<int, GroupVisualization *> groupVisualizations;
    std::set<int> registeredSourceIds; // signal sources whose result recorders are already attached

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
    virtual std::string getRecordingMode() const;
    // Converts a value from the statistic unit to the first display unit, if both are given.
    virtual double convertToDisplayUnit(double value) const;

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) = 0;
    virtual const StatisticVisualization *getStatisticVisualization(cComponent *source, simsignal_t signal);
    virtual void addStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void removeStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void removeAllStatisticVisualizations();

    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void processSignal(cComponent *source, simsignal_t signal, std::function<void(cIListener *)> receiveSignal);

    /** @name Several statistics displayed together on one figure */
    //@{
    // Creates the visualization displaying the statistics of one group (the concrete
    // subclass creates the figure); returns nullptr if grouping is not supported (e.g. by osg).
    virtual GroupVisualization *createGroupVisualization(cComponent *module) { return nullptr; }
    virtual GroupVisualization *getGroupVisualization(int moduleId);
    virtual GroupVisualization *getOrCreateGroupVisualization(cComponent *module);
    virtual void addGroupVisualization(GroupVisualization *groupVisualization);
    virtual void removeGroupVisualization(GroupVisualization *groupVisualization);
    virtual void removeAllGroupVisualizations();
    // Stores the value of a signal as the last value of the item identified by the
    // details object emitted with it (SPLIT_DETAILS).
    virtual void processSplitValue(cComponent *source, double value, cObject *details);
    // Registers a signal source as one item of its network node's group (GROUP_NETWORK_NODE),
    // or as the source of the per flow items (SPLIT_FLOW), by attaching the result recorders
    // that provide the values.
    virtual void registerSource(cComponent *source, simsignal_t signal);
    // Returns the label identifying the item of a signal source among the ones displayed
    // together in the visualization of its network node (GROUP_NETWORK_NODE).
    virtual std::string getSourceItemLabel(cModule *module, cModule *networkNode) const;
    // Updates the values of the items from their result recorders, before rendering.
    virtual void refreshSourceItemValues() const;
    virtual void refreshFlowItemValues() const;
    // Collects all result recorders in the result listener chain of a signal, including
    // the ones created per flow by a demuxFlow() result filter.
    virtual void collectResultRecorders(cResultListener *resultListener, std::vector<LastValueRecorder *>& recorders) const;
    //@}

  public:
#define PROCESS_SIGNAL(value) { processSignal(source, signal, [=] (cIListener *listener) { listener->receiveSignal(source, signal, value, details); }); }
#define PROCESS_NUMERIC_SIGNAL(value, doubleValue) { \
        if (splitMode == SPLIT_NONE && groupMode == GROUP_NONE) PROCESS_SIGNAL(value) \
        else if (splitMode == SPLIT_DETAILS) processSplitValue(source, doubleValue, details); \
        else registerSource(source, signal); }
#define PROCESS_NONNUMERIC_SIGNAL(value) { \
        if (splitMode == SPLIT_NONE && groupMode == GROUP_NONE) PROCESS_SIGNAL(value) \
        else if (splitMode != SPLIT_DETAILS) registerSource(source, signal); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, bool b, cObject *details) override { PROCESS_NUMERIC_SIGNAL(b, b); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t l, cObject *details) override { PROCESS_NUMERIC_SIGNAL(l, l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, uintval_t l, cObject *details) override { PROCESS_NUMERIC_SIGNAL(l, l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double d, cObject *details) override { PROCESS_NUMERIC_SIGNAL(d, d); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const SimTime& t, cObject *details) override { PROCESS_NUMERIC_SIGNAL(t, t.dbl()); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const char *s, cObject *details) override { PROCESS_NONNUMERIC_SIGNAL(s); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override { PROCESS_NONNUMERIC_SIGNAL(obj); }
};

} // namespace visualizer

} // namespace inet

#endif

