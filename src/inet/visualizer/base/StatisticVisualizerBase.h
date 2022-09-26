//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATISTICVISUALIZERBASE_H
#define __INET_STATISTICVISUALIZERBASE_H

#include <functional>

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
    };

    class INET_API DirectiveResolver : public StringFormat::IDirectiveResolver {
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

    std::map<std::pair<int, simsignal_t>, const StatisticVisualization *> statisticVisualizations;

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

  public:
#define PROCESS_SIGNAL(value) { processSignal(source, signal, [=] (cIListener *listener) { listener->receiveSignal(source, signal, value, details); }); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, bool b, cObject *details) override { PROCESS_SIGNAL(b); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t l, cObject *details) override { PROCESS_SIGNAL(l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, uintval_t l, cObject *details) override { PROCESS_SIGNAL(l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double d, cObject *details) override { PROCESS_SIGNAL(d); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const SimTime& t, cObject *details) override { PROCESS_SIGNAL(t); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const char *s, cObject *details) override { PROCESS_SIGNAL(s); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override { PROCESS_SIGNAL(obj); }
};

} // namespace visualizer

} // namespace inet

#endif

