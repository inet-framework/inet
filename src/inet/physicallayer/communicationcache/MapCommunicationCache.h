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

#ifndef __INET_MAPCOMMUNICATIONCACHE_H
#define __INET_MAPCOMMUNICATIONCACHE_H

#include "inet/physicallayer/base/packetlevel/CommunicationCacheBase.h"

namespace inet {
namespace physicallayer {

class INET_API MapCommunicationCache : public CommunicationCacheBase
{
  protected:
    class MapTransmissionCacheEntry : public TransmissionCacheEntry
    {
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

    virtual std::ostream& printToStream(std::ostream &stream, int level) const override { return stream << "MapCommunicationCache"; }

    /** @name Radio cache */
    //@{
    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;
    virtual const IRadio *getRadio(int id) const override;
    virtual void mapRadios(std::function<void (const IRadio *)> f) const override;
    //@}

    /** @name Transmission cache */
    //@{
    virtual void addTransmission(const ITransmission *transmission) override;
    virtual void removeTransmission(const ITransmission *transmission) override;
    virtual const ITransmission *getTransmission(int id) const override;
    virtual void mapTransmissions(std::function<void (const ITransmission *)> f) const override;
    //@}

    /** @name Interference cache */
    //@{
    virtual void removeNonInterferingTransmissions(std::function<void (const ITransmission *transmission)> f) override;
    //@}
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_MAPCOMMUNICATIONCACHE_H

