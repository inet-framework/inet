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

// TODO: add runtime configurable array and/or map based decision cache based on radio and transmission ids (use base offset)
class INET_API CachedRadioChannel : public RadioChannel
{
    protected:
        mutable long cacheGetCount;
        mutable long cacheHitCount;

        int baseTransmissionId;
// TODO: std::vector<std::vector<const IRadioDecision *> > cachedReceptions;
        std::vector<std::vector<const IRadioSignalReceptionDecision *> > cachedDecisions;

    protected:
        virtual void finish();

// TODO: virtual const IRadioSignalReception *getCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
// TODO: virtual void setCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReception *reception);
// TODO: virtual void removeCachedReception(const IRadio *radio, const IRadioSignalTransmission *transmission);

        virtual const IRadioSignalReceptionDecision *getCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission) const;
        virtual void setCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission, const IRadioSignalReceptionDecision *decision);
        virtual void removeCachedDecision(const IRadio *radio, const IRadioSignalTransmission *transmission);
        virtual void invalidateCachedDecisions(const IRadioSignalTransmission *transmission);
        virtual void invalidateCachedDecision(const IRadioSignalReceptionDecision *decision);

    public:
        CachedRadioChannel() :
            RadioChannel(),
            cacheGetCount(0),
            cacheHitCount(0),
            baseTransmissionId(0)
        {}

        CachedRadioChannel(const IRadioSignalPropagation *propagation, const IRadioSignalAttenuation *attenuation, const IRadioBackgroundNoise *backgroundNoise) :
            RadioChannel(propagation, attenuation, backgroundNoise),
            cacheGetCount(0),
            cacheHitCount(0),
            baseTransmissionId(0)
        {}

        virtual const IRadioSignalReceptionDecision *receiveFromChannel(const IRadio *radio, const IRadioSignalListening *listening, const IRadioSignalTransmission *transmission) const;
};

#endif
