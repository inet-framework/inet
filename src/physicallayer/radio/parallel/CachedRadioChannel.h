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

#ifndef __INET_CACHEDRADIOCHANNEL_H_
#define __INET_CACHEDRADIOCHANNEL_H_

#include "RadioChannel.h"

/**
 * This class provides an efficient cache for pre-computed radio signal receptions
 * and radio signal reception decisions.
 */
class INET_API CachedRadioChannel : public RadioChannel
{
    protected:
        /**
         * Total number of radio signal reception cache queries.
         */
        mutable long receptionCacheGetCount;

        /**
         * Total number of radio signal reception cache hits.
         */
        mutable long receptionCacheHitCount;

        /**
         * Total number of radio signal reception decision cache queries.
         */
        mutable long decisionCacheGetCount;

        /**
         * Total number of radio signal reception decision cache hits.
         */
        mutable long decisionCacheHitCount;

        /**
         * The smallest transmission id that is used in the cache.
         */
        int baseTransmissionId;

        /**
         * Caches pre-computed radio signal receptions for transmissions and
         * radios. The outer vector is indexed by transmission id (offset with
         * base transmission id) and the inner vector is indexed by radio id.
         * Values that are no longer needed are removed from the beginning only.
         * Contains NULL values for not yet pre-computed receptions.
         */
        std::vector<std::vector<const IRadioSignalReception *> > cachedReceptions;

        /**
         * Caches pre-computed radio signal reception decisions for transmissions
         * and radios. The outer vector is indexed by transmission id (offset
         * with base transmission id) and the inner vector is indexed by radio id.
         * Values that are no longer needed are removed from the beginning only.
         * Contains NULL values for not yet pre-computed reception decisions.
         */
        std::vector<std::vector<const IRadioSignalReceptionDecision *> > cachedDecisions;

    protected:
        virtual void finish();

        virtual const IRadioSignalReception *getCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual void setCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception);
        virtual void removeCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission);

        virtual const IRadioSignalReceptionDecision *getCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual void setCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReceptionDecision *decision);
        virtual void removeCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission);

        virtual void invalidateCachedDecisions(const IRadioSignalTransmission *transmission);
        virtual void invalidateCachedDecision(const IRadioSignalReceptionDecision *decision);

        virtual const IRadioSignalReception *computeReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;

    public:
        CachedRadioChannel() :
            RadioChannel(),
            receptionCacheGetCount(0),
            receptionCacheHitCount(0),
            decisionCacheGetCount(0),
            decisionCacheHitCount(0),
            baseTransmissionId(0)
        {}

        CachedRadioChannel(const IRadioSignalPropagation *propagation, const IRadioSignalAttenuation *attenuation, const IRadioBackgroundNoise *backgroundNoise) :
            RadioChannel(propagation, attenuation, backgroundNoise),
            receptionCacheGetCount(0),
            receptionCacheHitCount(0),
            decisionCacheGetCount(0),
            decisionCacheHitCount(0),
            baseTransmissionId(0)
        {}

        virtual const IRadioSignalReceptionDecision *receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const;
};

#endif
