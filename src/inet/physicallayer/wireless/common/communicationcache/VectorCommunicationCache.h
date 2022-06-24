//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VECTORCOMMUNICATIONCACHE_H
#define __INET_VECTORCOMMUNICATIONCACHE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/CommunicationCacheBase.h"

namespace inet {
namespace physicallayer {

class INET_API VectorCommunicationCache : public CommunicationCacheBase
{
  protected:
    class INET_API VectorTransmissionCacheEntry : public TransmissionCacheEntry {
      public:
        /**
         * The list of intermediate reception computation results.
         */
        std::vector<ReceptionCacheEntry> *receptionCacheEntries = nullptr;
    };

  protected:
    /** @name Cache */
    //@{
    /**
     * The smallest radio id of all radios on the medium.
     */
    int baseRadioId = -1;
    /**
     * The smallest transmission id of all ongoing transmissions on the medium.
     */
    int baseTransmissionId = -1;
    /**
     * Caches intermediate computation results for transmissions. The outer
     * vector is indexed by transmission id (offset with base transmission id)
     * and the inner vector is indexed by radio id. Values that are no longer
     * needed are removed from the beginning only. May contain nullptr values
     * for not yet computed information.
     */
    std::vector<VectorTransmissionCacheEntry> transmissionCache;
    /**
     * Caches intermediate computation results for radios. The vector is indexed
     * by radio id (offset with base transmission id).
     */
    std::vector<RadioCacheEntry> radioCache;
    //@}

  protected:
    /** @name Cache data structures */
    //@{
    virtual RadioCacheEntry *getRadioCacheEntry(const IRadio *radio) override;
    virtual VectorTransmissionCacheEntry *getTransmissionCacheEntry(const ITransmission *transmission) override;
    virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission) override;
    //@}

  public:
    virtual ~VectorCommunicationCache();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "VectorCommunicationCache"; }

    /** @name Radio cache */
    //@{
    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;
    virtual const IRadio *getRadio(int id) const override;
    virtual void mapRadios(std::function<void(const IRadio *)> f) const override;
    //@}

    /** @name Transmission cache */
    //@{
    virtual void addTransmission(const ITransmission *transmission) override;
    virtual void removeTransmission(const ITransmission *transmission) override;
    virtual const ITransmission *getTransmission(int id) const override;
    virtual void mapTransmissions(std::function<void(const ITransmission *)> f) const override;
    //@}

    /** @name Interference cache */
    //@{
    virtual void removeNonInterferingTransmissions(std::function<void(const ITransmission *transmission)> f) override;
    //@}
};

} // namespace physicallayer
} // namespace inet

#endif

