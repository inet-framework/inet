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

#ifndef __INET_VECTORCOMMUNICATIONCACHE_H
#define __INET_VECTORCOMMUNICATIONCACHE_H

#include "inet/physicallayer/base/packetlevel/CommunicationCacheBase.h"

namespace inet {

namespace physicallayer {

class INET_API VectorCommunicationCache : public CommunicationCacheBase
{
  protected:
    /** @name Cache */
    //@{
    /**
     * The smallest radio id of all radios on the medium.
     */
    int baseRadioId;
    /**
     * The smallest transmission id of all ongoing transmissions on the medium.
     */
    int baseTransmissionId;
    /**
     * Caches intermediate computation results for transmissions. The outer
     * vector is indexed by transmission id (offset with base transmission id)
     * and the inner vector is indexed by radio id. Values that are no longer
     * needed are removed from the beginning only. May contain nullptr values
     * for not yet computed information.
     */
    std::vector<TransmissionCacheEntry> transmissionCache;
    /**
     * Caches intermediate computation results for radios. The vector is indexed
     * by radio id (offset with base transmission id).
     */
    std::vector<RadioCacheEntry> radioCache;
    //@}

  protected:
    /** @name Cache data structures */
    //@{
    virtual RadioCacheEntry *getRadioCacheEntry(const IRadio *radio);
    virtual TransmissionCacheEntry *getTransmissionCacheEntry(const ITransmission *transmission);
    virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission);
    //@}

  public:
    VectorCommunicationCache();
    virtual ~VectorCommunicationCache();

    virtual std::ostream& printToStream(std::ostream &stream, int level) const { return stream << "VectorCommunicationCache"; }

    /** @name Medium state change notifications */
    //@{
    virtual void addRadio(const IRadio *radio);
    virtual void removeRadio(const IRadio *radio);

    virtual void addTransmission(const ITransmission *transmission);
    virtual void removeTransmission(const ITransmission *transmission);
    //@}

    /** @name Interference cache */
    //@{
    virtual void removeNonInterferingTransmissions();
    //@}
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_VECTORCOMMUNICATIONCACHE_H

