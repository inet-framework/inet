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

#ifndef __INET_REFERENCECOMMUNICATIONCACHE_H
#define __INET_REFERENCECOMMUNICATIONCACHE_H

#include "inet/physicallayer/base/packetlevel/CommunicationCacheBase.h"

namespace inet {

namespace physicallayer {

class INET_API ReferenceCommunicationCache : public CommunicationCacheBase
{
  protected:
    std::vector<TransmissionCacheEntry> transmissionCache;
    std::vector<RadioCacheEntry> radioCache;

  protected:
    /** @name Cache data structures */
    //@{
    virtual RadioCacheEntry *getRadioCacheEntry(const IRadio *radio);
    virtual TransmissionCacheEntry *getTransmissionCacheEntry(const ITransmission *transmission);
    virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *radio, const ITransmission *transmission);
    //@}

  public:
    ReferenceCommunicationCache();
    virtual ~ReferenceCommunicationCache();

    virtual std::ostream& printToStream(std::ostream &stream, int level) const { return stream << "ReferenceCommunicationCache"; }

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
    virtual std::vector<const ITransmission *> *computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime);
    //@}

    /** @name Reception cache */
    //@{
    virtual const IReception *getCachedReception(const IRadio *radio, const ITransmission *transmission) { return nullptr; }
    virtual const IInterference *getCachedInterference(const IRadio *receiver, const ITransmission *transmission) { return nullptr; }
    virtual const INoise *getCachedNoise(const IRadio *receiver, const ITransmission *transmission) { return nullptr; }
    virtual const ISNIR *getCachedSNIR(const IRadio *receiver, const ITransmission *transmission) { return nullptr; }
    virtual const IReceptionDecision *getCachedDecision(const IRadio *radio, const ITransmission *transmission) { return nullptr; }
    //@}
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_REFERENCECOMMUNICATIONCACHE_H

