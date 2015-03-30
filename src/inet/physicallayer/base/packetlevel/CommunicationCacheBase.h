//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_COMMUNICATIONCACHEBASE_H
#define __INET_COMMUNICATIONCACHEBASE_H

#include "inet/common/IntervalTree.h"
#include "inet/physicallayer/contract/packetlevel/ICommunicationCache.h"

namespace inet {

namespace physicallayer {

class INET_API CommunicationCacheBase : public cModule, public ICommunicationCache
{
  protected:
    /**
     * Caches the intermediate computation results related to a radio.
     */
    class RadioCacheEntry {
      public:
        /**
         * Caches reception intervals for efficient interference queries.
         */
        IntervalTree *receptionIntervals;

      private:
        RadioCacheEntry(const RadioCacheEntry &other);
        RadioCacheEntry &operator=(const RadioCacheEntry &other);

      public:
        RadioCacheEntry();
        RadioCacheEntry(RadioCacheEntry &&other);
        RadioCacheEntry &operator=(RadioCacheEntry &&other);
        virtual ~RadioCacheEntry();
    };

    /**
     * Caches the intermediate computation results related to a reception.
     */
    class ReceptionCacheEntry
    {
      public:
        /**
         * The radio frame that was sent to the receiver or nullptr if not yet sent.
         */
        const IRadioFrame *frame;
        const IArrival *arrival;
        const Interval *interval;
        const IListening *listening;
        const IReception *reception;
        const IInterference *interference;
        const INoise *noise;
        const ISNIR *snir;
        const IReceptionDecision *decision;

      private:
        ReceptionCacheEntry(const ReceptionCacheEntry &other);
        ReceptionCacheEntry &operator=(const ReceptionCacheEntry &other);

      public:
        ReceptionCacheEntry();
        ReceptionCacheEntry(ReceptionCacheEntry &&other);
        ReceptionCacheEntry &operator=(ReceptionCacheEntry &&other);
        virtual ~ReceptionCacheEntry();
    };

    /**
     * Caches the intermediate computation results related to a transmission.
     */
    class TransmissionCacheEntry
    {
      public:
        /**
         * The last moment when this transmission may have any effect on
         * other transmissions by interfering with them.
         */
        simtime_t interferenceEndTime;
        /**
         * The radio frame that was created by the transmitter is never nullptr.
         */
        const IRadioFrame *frame;
        /**
         * The figure representing this transmission.
         */
        cFigure *figure;
        /**
         * The list of intermediate reception computation results.
         */
        void *receptionCacheEntries;

      public:
        TransmissionCacheEntry();
    };

  protected:
    /** @name Cache data structures */
    //@{
    virtual RadioCacheEntry *getRadioCacheEntry(const IRadio *radio) = 0;
    virtual TransmissionCacheEntry *getTransmissionCacheEntry(const ITransmission *transmission) = 0;
    virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission) = 0;
    //@}

  public:
    CommunicationCacheBase();
    virtual ~CommunicationCacheBase();

    /** @name Interference cache */
    //@{
    virtual std::vector<const ITransmission *> *computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime);
    //@}

    /** @name Transmission cache */
    //@{
    virtual const simtime_t getCachedInterferenceEndTime(const ITransmission *transmission);
    virtual void setCachedInterferenceEndTime(const ITransmission *transmission, const simtime_t interferenceEndTime);
    virtual void removeCachedInterferenceEndTime(const ITransmission *transmission);

    virtual const IRadioFrame *getCachedFrame(const ITransmission *transmission);
    virtual void setCachedFrame(const ITransmission *transmission, const IRadioFrame *frame);
    virtual void removeCachedFrame(const ITransmission *transmission);

    virtual cFigure *getCachedFigure(const ITransmission *transmission);
    virtual void setCachedFigure(const ITransmission *transmission, cFigure *figure);
    virtual void removeCachedFigure(const ITransmission *transmission);
    //@}

    /** @name Reception cache */
    //@{
    virtual const IArrival *getCachedArrival(const IRadio *radio, const ITransmission *transmission);
    virtual void setCachedArrival(const IRadio *radio, const ITransmission *transmission, const IArrival *arrival);
    virtual void removeCachedArrival(const IRadio *radio, const ITransmission *transmission);

    virtual const Interval *getCachedInterval(const IRadio *radio, const ITransmission *transmission);
    virtual void setCachedInterval(const IRadio *radio, const ITransmission *transmission, const Interval *interval);
    virtual void removeCachedInterval(const IRadio *radio, const ITransmission *transmission);

    virtual const IListening *getCachedListening(const IRadio *radio, const ITransmission *transmission);
    virtual void setCachedListening(const IRadio *radio, const ITransmission *transmission, const IListening *listening);
    virtual void removeCachedListening(const IRadio *radio, const ITransmission *transmission);

    virtual const IReception *getCachedReception(const IRadio *radio, const ITransmission *transmission);
    virtual void setCachedReception(const IRadio *radio, const ITransmission *transmission, const IReception *reception);
    virtual void removeCachedReception(const IRadio *radio, const ITransmission *transmission);

    virtual const IInterference *getCachedInterference(const IRadio *receiver, const ITransmission *transmission);
    virtual void setCachedInterference(const IRadio *receiver, const ITransmission *transmission, const IInterference *interference);
    virtual void removeCachedInterference(const IRadio *receiver, const ITransmission *transmission);

    virtual const INoise *getCachedNoise(const IRadio *receiver, const ITransmission *transmission);
    virtual void setCachedNoise(const IRadio *receiver, const ITransmission *transmission, const INoise *noise);
    virtual void removeCachedNoise(const IRadio *receiver, const ITransmission *transmission);

    virtual const ISNIR *getCachedSNIR(const IRadio *receiver, const ITransmission *transmission);
    virtual void setCachedSNIR(const IRadio *receiver, const ITransmission *transmission, const ISNIR *snir);
    virtual void removeCachedSNIR(const IRadio *receiver, const ITransmission *transmission);

    virtual const IReceptionDecision *getCachedDecision(const IRadio *radio, const ITransmission *transmission);
    virtual void setCachedDecision(const IRadio *radio, const ITransmission *transmission, const IReceptionDecision *decision);
    virtual void removeCachedDecision(const IRadio *radio, const ITransmission *transmission);

    virtual const IRadioFrame *getCachedFrame(const IRadio *radio, const ITransmission *transmission);
    virtual void setCachedFrame(const IRadio *radio, const ITransmission *transmission, const IRadioFrame *frame);
    virtual void removeCachedFrame(const IRadio *radio, const ITransmission *transmission);
    //@}
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_COMMUNICATIONCACHEBASE_H

