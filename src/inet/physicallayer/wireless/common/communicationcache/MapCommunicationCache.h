//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MAPCOMMUNICATIONCACHE_H
#define __INET_MAPCOMMUNICATIONCACHE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/CommunicationCacheBase.h"

namespace inet {
namespace physicallayer {

class INET_API MapCommunicationCache : public CommunicationCacheBase
{
  protected:
    class INET_API MapTransmissionCacheEntry : public TransmissionCacheEntry {
      public:
        /**
         * The map of intermediate reception computation results.
         */
        std::map<int, ReceptionCacheEntry> *receptionCacheEntries = nullptr;
    };

  protected:
    /** @name Cache */
    //@{
    /**
     * Caches intermediate computation results for radios.
     */
    std::map<int, RadioCacheEntry> radioCache;
    /**
     * Caches intermediate computation results for transmissions.
     */
    std::map<int, MapTransmissionCacheEntry> transmissionCache;
    //@}

  protected:
    /** @name Cache data structures */
    //@{
    virtual RadioCacheEntry *getRadioCacheEntry(const IRadio *radio) override;
    virtual MapTransmissionCacheEntry *getTransmissionCacheEntry(const ITransmission *transmission) override;
    virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission) override;
    //@}

  public:
    virtual ~MapCommunicationCache();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "MapCommunicationCache"; }

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
    //@}
};

} // namespace physicallayer
} // namespace inet

#endif

