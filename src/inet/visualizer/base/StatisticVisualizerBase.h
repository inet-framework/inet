//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_STATISTICVISUALIZERBASE_H
#define __INET_STATISTICVISUALIZERBASE_H

#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/Placement.h"
#include "inet/visualizer/util/ModuleFilter.h"
#include "inet/visualizer/util/StringFormat.h"

namespace inet {

namespace visualizer {

class INET_API StatisticVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API LastValueRecorder : public cNumericResultRecorder
    {
      protected:
        double lastValue = NaN;

      protected:
        virtual void collect(simtime_t_cref t, double value, cObject *details) override { lastValue = value; }

      public:
        void setLastValue(double lastValue) { this->lastValue = lastValue; }
        double getLastValue() const { return lastValue; }
    };

    class StatisticVisualization {
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

    class DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const StatisticVisualizerBase *visualizer = nullptr;
        const StatisticVisualization *visualization = nullptr;
        std::string result;

      public:
        DirectiveResolver(const StatisticVisualizerBase *visualizer, const StatisticVisualization *visualization) : visualizer(visualizer), visualization(visualization) { }

        virtual const char *resolveDirective(char directive) override;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayStatistics = false;
    ModuleFilter sourceFilter;
    const char *signalName = nullptr;
    const char *statisticName = nullptr;
    const char *statisticUnit = nullptr;
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

    virtual void subscribe();
    virtual void unsubscribe();

    virtual cResultFilter *findResultFilter(cComponent *source, simsignal_t signal);
    virtual cResultFilter *findResultFilter(cResultFilter *parentResultFilter, cResultListener *resultListener);
    virtual std::string getText(const StatisticVisualization *statisticVisualization);
    virtual const char *getUnit(cComponent *source);

    virtual StatisticVisualization *createStatisticVisualization(cComponent *source, simsignal_t signal) = 0;
    virtual const StatisticVisualization *getStatisticVisualization(cComponent *source, simsignal_t signal);
    virtual void addStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void removeStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void removeAllStatisticVisualizations();

    virtual void refreshStatisticVisualization(const StatisticVisualization *statisticVisualization);
    virtual void processSignal(cComponent *source, simsignal_t signal, double value);

  public:
    virtual ~StatisticVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, bool b, cObject *details) override { processSignal(source, signal, NaN); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, long l, cObject *details) override { processSignal(source, signal, l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, unsigned long l, cObject *details) override { processSignal(source, signal, l); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double d, cObject *details) override { processSignal(source, signal, d); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const SimTime& t, cObject *details) override { processSignal(source, signal, t.dbl()); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const char *s, cObject *details) override { processSignal(source, signal, NaN); }
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override { processSignal(source, signal, NaN); }
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_STATISTICVISUALIZERBASE_H

