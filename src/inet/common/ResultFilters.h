//
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RESULTFILTERS_H
#define __INET_RESULTFILTERS_H

#include "inet/common/INETMath.h"

namespace inet {

namespace utils {

namespace filters {

class INET_API VoidPtrWrapper : public cObject
{
  private:
    void *object;

  public:
    VoidPtrWrapper(void *object) : object(object) {}

    void *getObject() const { return object; }
};

/**
 * Filter that expects a Packet and outputs the age of packet data in seconds.
 */
class INET_API DataAgeFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a cMessage and outputs its age in seconds
 * (t - msg->getCreationTime()).
 */
class INET_API MessageAgeFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a cMessage and outputs its age from the timestamp field
 * in seconds (t - msg->getTimestamp()).
 */
class INET_API MessageTsAgeFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a cMessage and outputs its age in seconds
 * (t - msg->getArrivalTime()).
 */
class INET_API QueueingTimeFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects an IReception and outputs its minimum signal power.
 */
class INET_API ReceptionMinSignalPowerFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects an ApplicationPacket, and outputs its sequence number.
 */
class INET_API ApplicationPacketSequenceNumberFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects an IMobility and outputs its current coordinate
 */
class INET_API MobilityPosFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API cPointerResultFilter : public cResultFilter
{
  protected:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) override {}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t l, cObject *details) override {}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, uintval_t l, cObject *details) override {}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) override {}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) override {}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) override {}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) override {}
};

/**
 * Filter that expects a Coord and outputs its X coordinate
 */
class INET_API XCoordFilter : public cPointerResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a Coord and outputs its Y coordinate
 */
class INET_API YCoordFilter : public cPointerResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a Coord and outputs its Z coordinate
 */
class INET_API ZCoordFilter : public cPointerResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a numeric value and lets it path through if an atomic operation is not in progress.
 */
class INET_API AtomicFilter : public cNumericResultFilter
{
  protected:
    bool inAtomicOperation = false;

  public:
    virtual bool process(simtime_t& t, double& value, cObject *details) override;
};

/**
 * Filter that expects a packet and outputs its duration
 */
class INET_API PacketDurationFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a packet and outputs its length
 */
class INET_API PacketLengthFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a packet and a flow and outputs the flow specific data length
 */
class INET_API FlowPacketLengthFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a cMessage and outputs its source address as string
 */
class INET_API MessageSourceAddrFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API GroupRegionsPerPacketFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API LengthWeightedValuePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API MaxPerGroupFilter : public cObjectResultFilter
{
  protected:
    double max = NaN;
    std::string lastIdentifier;
    simtime_t lastTime = -1;

  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    virtual void finish(cComponent *component, simsignal_t signalID) override;
};

class INET_API WeighedMeanPerGroupFilter : public cObjectResultFilter
{
  protected:
    double weight = NaN;
    double sum = NaN;
    std::string lastIdentifier;
    simtime_t lastTime = -1;

  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    virtual void finish(cComponent *component, simsignal_t signalID) override;
};

class INET_API WeighedSumPerGroupFilter : public cObjectResultFilter
{
  protected:
    double sum = NaN;
    std::string lastIdentifier;
    simtime_t lastTime = -1;

  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    virtual void finish(cComponent *component, simsignal_t signalID) override;
};

class INET_API DropWeightFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API WeightTimesFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * The filter expects a packet and forwards it along with the flow names as details
 * that were found. The filter has a flowName parameter that is read from the INI
 * file configuration.
 */
class INET_API DemuxFlowFilter : public DemuxFilter
{
  protected:
    cMatchExpression flowNameMatcher;

  protected:
    virtual void init(Context *ctx) override;

  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API ResidenceTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API LifeTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API ElapsedTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API DelayingTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API ProcessingTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API QueueingTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API PropagationTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API TransmissionTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

class INET_API PacketTransmissionTimePerRegionFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects numbers and outputs their variance
 */
class INET_API VarianceFilter : public cNumericResultFilter
{
  protected:
    int64_t numValues = 0;
    double sumValues = 0;
    double sumSquaredValues = 0;

  protected:
    virtual bool process(simtime_t& t, double& value, cObject *details) override;
    virtual double getVariance() const;
};

/**
 * Filter that expects numbers and outputs their standard deviation
 */
class INET_API StddevFilter : public VarianceFilter
{
  protected:
    virtual bool process(simtime_t& t, double& value, cObject *details) override;
};

/**
 * Filter that expects numbers and outputs their jitter (difference between subsequent values)
 */
class INET_API JitterFilter : public cNumericResultFilter
{
  protected:
    double last = 0;

  protected:
    virtual bool process(simtime_t& t, double& value, cObject *details) override;
};

/**
 * Filter that expects numbers and outputs their difference to their mean
 */
class INET_API DifferenceToMeanFilter : public cNumericResultFilter
{
  protected:
    intval_t count = 0;
    double sum = 0;

  protected:
    virtual bool process(simtime_t& t, double& value, cObject *details) override;
};

/**
 * Filter that expects 0 or 1 as input and outputs the utilization as double.
 * By default, the value is computed for the *past* interval every 0.1s or 200
 * values, whichever comes first.
 *
 * Recommended interpolation mode: backward sample-hold.
 */
class INET_API UtilizationFilter : public cNumericResultFilter
{
  protected:
    // parameters
    simtime_t interval = 0.1;
    int numValueLimit = 200; // two values (1/0) per packet to match the result of the throughput filter
    bool emitIntermediateValues = true;

    // state
    simtime_t lastSignalTime;
    simtime_t totalValueTime;
    double lastValue = 0;
    double totalValue = 0;
    int numValues = 0;

  protected:
    virtual void init(Context *ctx) override;
    virtual UtilizationFilter *clone() const override;
    virtual bool process(simtime_t& t, double& value, cObject *details) override;
    virtual void emitUtilization(simtime_t time, cObject *details);
    virtual void updateTotalValue(simtime_t time);

  public:
    virtual void finish(cComponent *component, simsignal_t signalID) override;
};

/**
 * Filter that expects a Packet and outputs the packet rate as double.
 * Packet rate is computed for the *past* interval every 1.0s.
 * The filter reads the interval parameter from the INI file configuration.
 *
 * Note that this filter is unsuitable for interactive use (with instrument figures,
 * for example), because zeroes for long silent periods are only emitted retroactively,
 * when the silent period (or the simulation) is over.
 *
 * Recommended interpolation mode: backward sample-hold.
 */
class INET_API PacketRateFilter : public cObjectResultFilter
{
  protected:
    simtime_t interval = -1;
    bool emitIntermediateZeros = true;

    simtime_t lastSignalTime;
    long numPackets = 0;

  protected:
    virtual void init(Context *ctx) override;
    virtual PacketRateFilter *clone() const override;
    virtual void emitPacketRate(simtime_t endInterval, cObject *details);

  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    virtual void finish(cComponent *component, simsignal_t signalID) override;
};

/**
 * Filter that expects a Packet or a packet length and outputs the throughput as double.
 * Throughput is computed for the *past* interval every 0.1s or 100 packets,
 * whichever comes first. The filter reads the interval and numLengthLimit
 * parameters from the INI file configuration.
 *
 * Note that this filter is unsuitable for interactive use (with instrument figures,
 * for example), because zeroes for long silent periods are only emitted retroactively,
 * when the silent period (or the simulation) is over.
 *
 * Recommended interpolation mode: backward sample-hold.
 */
class INET_API ThroughputFilter : public cObjectResultFilter
{
  protected:
    simtime_t interval = -1;
    int numLengthLimit = -1;
    bool emitIntermediateZeros = true;

    simtime_t lastSignalTime;
    double totalLength = 0;
    int numLengths = 0;

  protected:
    virtual void init(Context *ctx) override;
    virtual ThroughputFilter *clone() const override;
    virtual void emitThroughput(simtime_t endInterval, cObject *details);

  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t value, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    virtual void finish(cComponent *component, simsignal_t signalID) override;
};

/**
 * Filter that expects a Packet or a packet length and outputs the throughput as double.
 * Throughput is computed and emitted for the *past* interval strictly
 * every 0.1s. (To achieve that, this filter employs a timer event.)
 *
 * For interactive use (with instrument figures, for example), this filter
 * should be preferred to ThroughputFilter.
 *
 * Recommended interpolation mode: backward sample-hold.
 */
class INET_API LiveThroughputFilter : public cObjectResultFilter
{
  protected:
    simtime_t interval = 0.1;
    simtime_t lastSignal = 0;
    double totalLength = 0;
    cEvent *event = nullptr;

  public:
    ~LiveThroughputFilter();
    virtual void init(Context *ctx) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t value, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
    virtual void finish(cComponent *component, simsignal_t signalID) override;
    virtual void timerExpired();
    virtual void timerDeleted();
};

/**
 * Filter that outputs the elapsed time since the creation of this filter object.
 */
class INET_API ElapsedTimeFilter : public cResultFilter
{
  protected:
    time_t startTime;

  public:
    ElapsedTimeFilter();

  protected:
    double getElapsedTime();
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) override { fire(this, t, getElapsedTime(), details); }
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t l, cObject *details) override { fire(this, t, getElapsedTime(), details); }
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, uintval_t l, cObject *details) override { fire(this, t, getElapsedTime(), details); }
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) override { fire(this, t, getElapsedTime(), details); }
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) override { fire(this, t, getElapsedTime(), details); }
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) override { fire(this, t, getElapsedTime(), details); }
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) override { fire(this, t, getElapsedTime(), details); }
};

class INET_API PacketDropReasonFilter : public cObjectResultFilter
{
  protected:
    int reason = -1;

  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a Packet and outputs its minsnir from snirIndication tag if exists .
 */
class INET_API MinimumSnirFromSnirIndFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a Packet and outputs its packetErrorRate from ErrorRateInd tag if exists .
 */
class INET_API PacketErrorRateFromErrorRateIndFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a Packet and outputs its bitErrorRate from ErrorRateInd tag if exists .
 */
class INET_API BitErrorRateFromErrorRateIndFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a Packet and outputs its symbolErrorRate from ErrorRateInd tag if exists .
 */
class INET_API SymbolErrorRateFromErrorRateIndFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a Packet and outputs its it if the source is the same component as where the filter is used.
 */
class INET_API LocalSignalFilter : public cObjectResultFilter
{
  protected:
    cComponent *component = nullptr;

  public:
    virtual void subscribedTo(cComponent *component, simsignal_t signal) override { this->component = component; }

    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, intval_t l, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, uintval_t l, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) override;
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, bool b, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, intval_t l, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, uintval_t l, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, double d, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const SimTime& v, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, const char *s, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace filters

} // namespace utils

} // namespace inet

#endif

