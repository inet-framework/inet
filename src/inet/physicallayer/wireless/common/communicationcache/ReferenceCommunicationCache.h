//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_REFERENCECOMMUNICATIONCACHE_H
#define __INET_REFERENCECOMMUNICATIONCACHE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/CommunicationCacheBase.h"

namespace inet {
namespace physicallayer {

class INET_API ReferenceCommunicationCache : public CommunicationCacheBase
{
  protected:
    class INET_API ReferenceTransmissionCacheEntry : public TransmissionCacheEntry {
      public:
        /**
         * The list of intermediate reception computation results.
         */
        std::vector<ReceptionCacheEntry> *receptionCacheEntries = nullptr;
    };

  protected:
    std::vector<RadioCacheEntry> radioCache;
    std::vector<ReferenceTransmissionCacheEntry> transmissionCache;
    std::vector<const IReception *> receptions;
    std::vector<const IInterference *> interferences;
    std::vector<const INoise *> noises;
    std::vector<const ISnir *> snirs;

  protected:
    /** @name Cache data structures */
    //@{
    virtual RadioCacheEntry *getRadioCacheEntry(const IRadio *radio) override;
    virtual ReferenceTransmissionCacheEntry *getTransmissionCacheEntry(const ITransmission *transmission) override;
    virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission) override;
    //@}

  public:
    virtual ~ReferenceCommunicationCache();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "ReferenceCommunicationCache"; }

    /** @name Radio cache */
    //@{
    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;
    virtual const IRadio *getRadio(int id) const override;
    virtual void mapRadios(std::function<void(const IRadio *)> f) const override;
    virtual int getNumRadios() const override;
    //@}

    /** @name Transmission cache */
    //@{
    virtual void addTransmission(const ITransmission *transmission) override;
    virtual void removeTransmission(const ITransmission *transmission) override;
    virtual const ITransmission *getTransmission(int id) const override;
    virtual void mapTransmissions(std::function<void(const ITransmission *)> f) const override;
    virtual int getNumTransmissions() const override;
    //@}

    /** @name Interference cache */
    //@{
    virtual void removeNonInterferingTransmissions(std::function<void(const ITransmission *transmission)> f) override;
    virtual std::vector<const ITransmission *> *computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime) override;
    //@}

    /** @name Reception cache */
    //@{
    virtual const IReception *getCachedReception(const IRadio *radio, const ITransmission *transmission) override { return nullptr; }
    virtual void setCachedReception(const IRadio *receiver, const ITransmission *transmission, const IReception *reception) override { receptions.push_back(reception); }

    virtual const IInterference *getCachedInterference(const IRadio *receiver, const ITransmission *transmission) override { return nullptr; }
    virtual void setCachedInterference(const IRadio *receiver, const ITransmission *transmission, const IInterference *interference) override { interferences.push_back(interference); }

    virtual const INoise *getCachedNoise(const IRadio *receiver, const ITransmission *transmission) override { return nullptr; }
    virtual void setCachedNoise(const IRadio *receiver, const ITransmission *transmission, const INoise *noise) override { noises.push_back(noise); }

    virtual const ISnir *getCachedSNIR(const IRadio *receiver, const ITransmission *transmission) override { return nullptr; }
    virtual void setCachedSNIR(const IRadio *receiver, const ITransmission *transmission, const ISnir *snir) override { snirs.push_back(snir); }

    // TODO disabling this cache makes the fingerprint of the reference and other models different,
    //       because recomputing the reception decision involves drawing random numbers
    // virtual const IReceptionDecision *getCachedReceptionDecision(const IRadio *radio, const ITransmission *transmission, IRadioSignal::SignalPart part) override { return nullptr; }
    // virtual void setCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part, const IReceptionDecision *receptionDecision) override {}
    //@}
};

} // namespace physicallayer
} // namespace inet

#endif

