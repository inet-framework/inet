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

#ifndef __INET_ICOMMUNICATIONCACHE_H
#define __INET_ICOMMUNICATIONCACHE_H

#include "inet/physicallayer/contract/packetlevel/ISNIR.h"
#include "inet/physicallayer/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/contract/packetlevel/IInterference.h"
#include "inet/physicallayer/contract/packetlevel/IBackgroundNoise.h"
#include "inet/physicallayer/contract/packetlevel/IRadioFrame.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionResult.h"
#include "inet/physicallayer/contract/packetlevel/IRadioFrame.h"

namespace inet {

namespace physicallayer {

/**
 * This interface is used to cache various intermediate computation results
 * related to the communication on the medium.
 */
class INET_API ICommunicationCache : public IPrintableObject
{
  public:
    /** @name Medium state change notifications */
    //@{
    virtual void addRadio(const IRadio *radio) = 0;
    virtual void removeRadio(const IRadio *radio) = 0;

    virtual void addTransmission(const ITransmission *transmission) = 0;
    virtual void removeTransmission(const ITransmission *transmission) = 0;
    //@}

    /** @name Interference cache */
    //@{
    virtual std::vector<const ITransmission *> *computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime) = 0;
    virtual void removeNonInterferingTransmissions() = 0;
    //@}

    /** @name Transmission cache */
    //@{
    virtual const simtime_t getCachedInterferenceEndTime(const ITransmission *transmission) = 0;
    virtual void setCachedInterferenceEndTime(const ITransmission *transmission, const simtime_t interferenceEndTime) = 0;
    virtual void removeCachedInterferenceEndTime(const ITransmission *transmission) = 0;

    virtual const IRadioFrame *getCachedFrame(const ITransmission *transmission) = 0;
    virtual void setCachedFrame(const ITransmission *transmission, const IRadioFrame *radioFrame) = 0;
    virtual void removeCachedFrame(const ITransmission *transmission) = 0;

    virtual cFigure *getCachedFigure(const ITransmission *transmission) = 0;
    virtual void setCachedFigure(const ITransmission *transmission, cFigure *figure) = 0;
    virtual void removeCachedFigure(const ITransmission *transmission) = 0;
    //@}

    /** @name Reception Cache */
    //@{
    virtual const IArrival *getCachedArrival(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedArrival(const IRadio *receiver, const ITransmission *transmission, const IArrival *arrival) = 0;
    virtual void removeCachedArrival(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const Interval *getCachedInterval(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedInterval(const IRadio *receiver, const ITransmission *transmission, const Interval *interval) = 0;
    virtual void removeCachedInterval(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const IListening *getCachedListening(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedListening(const IRadio *receiver, const ITransmission *transmission, const IListening *listening) = 0;
    virtual void removeCachedListening(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const IReception *getCachedReception(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedReception(const IRadio *receiver, const ITransmission *transmission, const IReception *reception) = 0;
    virtual void removeCachedReception(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const IInterference *getCachedInterference(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedInterference(const IRadio *receiver, const ITransmission *transmission, const IInterference *interference) = 0;
    virtual void removeCachedInterference(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const INoise *getCachedNoise(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedNoise(const IRadio *receiver, const ITransmission *transmission, const INoise *noise) = 0;
    virtual void removeCachedNoise(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const ISNIR *getCachedSNIR(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedSNIR(const IRadio *receiver, const ITransmission *transmission, const ISNIR *snir) = 0;
    virtual void removeCachedSNIR(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const IReceptionDecision *getCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) = 0;
    virtual void setCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part, const IReceptionDecision *receptionDecision) = 0;
    virtual void removeCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) = 0;

    virtual const IReceptionResult *getCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission, const IReceptionResult *receptionResult) = 0;
    virtual void removeCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const IRadioFrame *getCachedFrame(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedFrame(const IRadio *receiver, const ITransmission *transmission, const IRadioFrame *radioFrame) = 0;
    virtual void removeCachedFrame(const IRadio *receiver, const ITransmission *transmission) = 0;
    //@}
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ICOMMUNICATIONCACHE_H

