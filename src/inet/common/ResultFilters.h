//
// Copyright (C) 2011 Zoltan Bojthe
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

#ifndef __INET_RESULTFILTERS_H
#define __INET_RESULTFILTERS_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace utils {

namespace filters {

class VoidPtrWrapper : public cObject
{
  private:
    void *object;

  public:
    VoidPtrWrapper(void *object) : object(object) { }

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
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) override { }
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, long l, cObject *details) override { }
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l, cObject *details) override { }
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) override { }
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) override { }
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) override { }
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) override { }
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
 * Filter that expects a cMessage and outputs its source address as string
 */
class INET_API MessageSourceAddrFilter : public cObjectResultFilter
{
  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that expects a cPacket and outputs the throughput as double.
 */
class INET_API ThroughputFilter : public cObjectResultFilter
{
  protected:
    simtime_t interval = 0.1;
    int packetLimit = 100;
    bool emitIntermediateZeros = true;

    simtime_t lastSignal = 0;
    double bytes = 0;
    int packets = 0;

  public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

/**
 * Filter that outputs the elapsed time since the creation of this filter object.
 */
class INET_API ElapsedTimeFilter : public cResultFilter
{
  protected:
    long startTime;
  public:
    ElapsedTimeFilter();
  protected:
    double getElapsedTime();
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) override {fire(this, t, getElapsedTime(), details);}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, long l, cObject *details) override {fire(this, t, getElapsedTime(), details);}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l, cObject *details) override {fire(this, t, getElapsedTime(), details);}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) override {fire(this, t, getElapsedTime(), details);}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) override {fire(this, t, getElapsedTime(), details);}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) override {fire(this, t, getElapsedTime(), details);}
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) override {fire(this, t, getElapsedTime(), details);}
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



} // namespace filters

} // namespace utils

} // namespace inet

#endif // ifndef __INET_RESULTFILTERS_H

