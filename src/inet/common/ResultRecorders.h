//
// Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
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

#ifndef __INET_RESULTRECORDERS_H
#define __INET_RESULTRECORDERS_H

#include <string>

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"

namespace inet {

/**
 * Listener for counting the occurrences of signals with the same attribute
 */
class INET_API GroupCountRecorder : public cResultRecorder
{
    protected:
        std::map<std::string,long> groupcounts;
    protected:
        virtual void collect(std::string val);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t l, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, uintval_t l, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) override;

    public:
        GroupCountRecorder() {}
        virtual void finish(cResultFilter *prev) override;
};

class INET_API ElapsedTimeRecorder : public cResultRecorder
{
    protected:
        clock_t startTime;
    protected:
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) override {}
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t l, cObject *details) override {}
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, uintval_t l, cObject *details) override {}
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) override {}
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) override {}
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) override {}
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) override {}

    public:
        ElapsedTimeRecorder();
        virtual void finish(cResultFilter* prev) override;
};

class INET_API WeightedHistogramRecorder : public cNumericResultRecorder
{
    public:
        class cWeight : public cObject {
            protected:
                const double weight;

            public:
                cWeight(double weight) : weight(weight) { }
                double getWeight() const { return weight; }
        };

    protected:
        cStatistic *statistic = nullptr;
    protected:
        virtual void collect(simtime_t_cref t, double value, cObject *details) override;
        virtual void finish(cResultFilter *prev) override;
        virtual void forEachChild(cVisitor *v) override;
    public:
        WeightedHistogramRecorder();
        ~WeightedHistogramRecorder();
        virtual void init(cComponent *component, const char *statisticName, const char *recordingMode, cProperty *attrsProperty, opp_string_map *manualAttrs) override;
        virtual void setStatistic(cStatistic* stat);
        virtual cStatistic *getStatistic() const {return statistic;}
        virtual std::string str() const override;
};


} // namespace inet

#endif
