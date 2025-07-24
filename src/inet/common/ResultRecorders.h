//
// Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RESULTRECORDERS_H
#define __INET_RESULTRECORDERS_H

#include <string>

#include "inet/common/INETMath.h"

namespace inet {

/**
 * Listener for counting the occurrences of signals with the same attribute
 */
class INET_API GroupCountRecorder : public cResultRecorder
{
  protected:
    std::map<std::string, long> groupcounts;

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
    virtual void finish(cResultFilter *prev) override;
};

class INET_API WeightedHistogramRecorder : public cNumericResultRecorder
{
  public:
    class INET_API cWeight : public cObject {
      protected:
        const double weight;

      public:
        cWeight(double weight) : weight(weight) {}
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
    virtual void init(Context *ctx) override;
    virtual void setStatistic(cStatistic *stat);
    virtual cStatistic *getStatistic() const { return statistic; }
    virtual std::string str() const override;
};

class INET_API ChannelOwnerRecorder : public VectorRecorder
{
  protected:
    class SetChannelOwnerEvent : public cEvent
    {
      protected:
        ChannelOwnerRecorder *recorder = nullptr;

      public:
        SetChannelOwnerEvent(const char *name, ChannelOwnerRecorder *recorder) : cEvent(name), recorder(recorder) {}
        ~SetChannelOwnerEvent() { }
        virtual cEvent *dup() const override { copyNotSupported(); return nullptr; }
        virtual cObject *getTargetObject() const override { return nullptr; }
        virtual void execute() override { recorder->collect(simTime(), 1, nullptr); }
    };

  protected:
    std::set<std::string> networkNodeNames;
    cEnum *_enum = nullptr;
    cEvent *event = nullptr;

  protected:
    virtual void init(Context *ctx) override;
    virtual opp_string_map getStatisticAttributes() override;

    void collectNetworkNodes(cModule *module, std::set<cModule *>& visitedModules);

  public:
    ChannelOwnerRecorder() {}
    virtual ~ChannelOwnerRecorder();

    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref time, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

