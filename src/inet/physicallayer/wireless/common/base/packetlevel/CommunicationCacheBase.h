//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COMMUNICATIONCACHEBASE_H
#define __INET_COMMUNICATIONCACHEBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/ICommunicationCache.h"

namespace inet {

namespace physicallayer {

class INET_API CommunicationCacheBase : public cModule, public ICommunicationCache
{
  protected:
    /**
     * Caches the intermediate computation results related to a radio.
     */
    class INET_API RadioCacheEntry {
      public:
        /**
         * The corresponding radio.
         */
        const IRadio *radio = nullptr;
        /**
         * Caches reception intervals for efficient interference queries.
         */
        IntervalTree *receptionIntervals = nullptr;

      public:
        RadioCacheEntry() {}
        RadioCacheEntry(const RadioCacheEntry& other);
        RadioCacheEntry(RadioCacheEntry&& other) noexcept;
        RadioCacheEntry& operator=(const RadioCacheEntry& other);
        RadioCacheEntry& operator=(RadioCacheEntry&& other) noexcept;
        virtual ~RadioCacheEntry();
    };

    /**
     * Caches the intermediate computation results related to a reception.
     */
    class INET_API ReceptionCacheEntry {
      public:
        /**
         * The corresponding transmission.
         */
        const ITransmission *transmission = nullptr;
        /**
         * The corresponding receiver radio.
         */
        const IRadio *receiver = nullptr;
        /**
         * The signal that was sent to the receiver or nullptr if not yet sent.
         */
        const IWirelessSignal *signal = nullptr;
        const IArrival *arrival = nullptr;
        const IntervalTree::Interval *interval = nullptr;
        const IListening *listening = nullptr;
        const IReception *reception = nullptr;
        const IInterference *interference = nullptr;
        const INoise *noise = nullptr;
        const ISnir *snir = nullptr;
        std::vector<const IReceptionDecision *> receptionDecisions;
        const IReceptionResult *receptionResult = nullptr;

      public:
        ReceptionCacheEntry();
        ReceptionCacheEntry(const ReceptionCacheEntry& other);
        ReceptionCacheEntry(ReceptionCacheEntry&& other) noexcept;
        ReceptionCacheEntry& operator=(const ReceptionCacheEntry& other);
        ReceptionCacheEntry& operator=(ReceptionCacheEntry&& other) noexcept;
        virtual ~ReceptionCacheEntry();
    };

    /**
     * Caches the intermediate computation results related to a transmission.
     */
    class INET_API TransmissionCacheEntry {
      public:
        /**
         * The corresponding transmission.
         */
        const ITransmission *transmission = nullptr;
        /**
         * The last moment when this transmission may have any effect on
         * other transmissions by interfering with them.
         */
        simtime_t interferenceEndTime;
        /**
         * The signal that was created by the transmitter is never nullptr.
         */
        const IWirelessSignal *signal = nullptr;

      public:
        TransmissionCacheEntry() {}
        TransmissionCacheEntry(const TransmissionCacheEntry& other);
        TransmissionCacheEntry(TransmissionCacheEntry&& other) noexcept;
        TransmissionCacheEntry& operator=(const TransmissionCacheEntry& other);
        TransmissionCacheEntry& operator=(TransmissionCacheEntry&& other) noexcept;
        virtual ~TransmissionCacheEntry();

      protected:
        virtual void deleteSignal();
    };

  protected:
    /** @name Cache data structures */
    //@{
    virtual RadioCacheEntry *getRadioCacheEntry(const IRadio *radio) = 0;
    virtual TransmissionCacheEntry *getTransmissionCacheEntry(const ITransmission *transmission) = 0;
    virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission) = 0;
    //@}

  public:
    /** @name Radio cache */
    //@{
    virtual int getNumRadios() const override;
    //@}

    /** @name Transmission cache */
    //@{
    virtual int getNumTransmissions() const override;
    //@}

    /** @name Interference cache */
    //@{
    virtual std::vector<const ITransmission *> *computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime) override;

    virtual const simtime_t getCachedInterferenceEndTime(const ITransmission *transmission) override;
    virtual void setCachedInterferenceEndTime(const ITransmission *transmission, const simtime_t interferenceEndTime) override;
    virtual void removeCachedInterferenceEndTime(const ITransmission *transmission) override;

    virtual const IWirelessSignal *getCachedSignal(const ITransmission *transmission) override;
    virtual void setCachedSignal(const ITransmission *transmission, const IWirelessSignal *signal) override;
    virtual void removeCachedSignal(const ITransmission *transmission) override;
    //@}

    /** @name Reception cache */
    //@{
    virtual const IArrival *getCachedArrival(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedArrival(const IRadio *receiver, const ITransmission *transmission, const IArrival *arrival) override;
    virtual void removeCachedArrival(const IRadio *receiver, const ITransmission *transmission) override;

    virtual const IntervalTree::Interval *getCachedInterval(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedInterval(const IRadio *receiver, const ITransmission *transmission, const IntervalTree::Interval *interval) override;
    virtual void removeCachedInterval(const IRadio *receiver, const ITransmission *transmission) override;

    virtual const IListening *getCachedListening(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedListening(const IRadio *receiver, const ITransmission *transmission, const IListening *listening) override;
    virtual void removeCachedListening(const IRadio *receiver, const ITransmission *transmission) override;

    virtual const IReception *getCachedReception(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedReception(const IRadio *receiver, const ITransmission *transmission, const IReception *reception) override;
    virtual void removeCachedReception(const IRadio *receiver, const ITransmission *transmission) override;

    virtual const IInterference *getCachedInterference(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedInterference(const IRadio *receiver, const ITransmission *transmission, const IInterference *interference) override;
    virtual void removeCachedInterference(const IRadio *receiver, const ITransmission *transmission) override;

    virtual const INoise *getCachedNoise(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedNoise(const IRadio *receiver, const ITransmission *transmission, const INoise *noise) override;
    virtual void removeCachedNoise(const IRadio *receiver, const ITransmission *transmission) override;

    virtual const ISnir *getCachedSNIR(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedSNIR(const IRadio *receiver, const ITransmission *transmission, const ISnir *snir) override;
    virtual void removeCachedSNIR(const IRadio *receiver, const ITransmission *transmission) override;

    virtual const IReceptionDecision *getCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) override;
    virtual void setCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part, const IReceptionDecision *receptionDecision) override;
    virtual void removeCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) override;

    virtual const IReceptionResult *getCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission, const IReceptionResult *receptionResult) override;
    virtual void removeCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission) override;

    virtual const IWirelessSignal *getCachedSignal(const IRadio *receiver, const ITransmission *transmission) override;
    virtual void setCachedSignal(const IRadio *receiver, const ITransmission *transmission, const IWirelessSignal *signal) override;
    virtual void removeCachedSignal(const IRadio *receiver, const ITransmission *transmission) override;
    //@}
};

} // namespace physicallayer

} // namespace inet

#endif

