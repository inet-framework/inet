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

#ifndef __INET_ICOMMUNICATIONCACHE_H
#define __INET_ICOMMUNICATIONCACHE_H

#include "inet/common/IntervalTree.h"
#include "inet/physicallayer/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/contract/packetlevel/IBackgroundNoise.h"
#include "inet/physicallayer/contract/packetlevel/IInterference.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionResult.h"
#include "inet/physicallayer/contract/packetlevel/ISignal.h"
#include "inet/physicallayer/contract/packetlevel/ISnir.h"

namespace inet {
namespace physicallayer {

/**
 * This interface is used to store the radios and transmissions of the medium.
 * It also provides caching for various intermediate computation results related
 * to the communication on the medium.
 */
class INET_API ICommunicationCache : public IPrintableObject
{
  public:
    virtual ~ICommunicationCache() { }

    /** @name Radio cache */
    //@{
    virtual void addRadio(const IRadio *radio) = 0;
    virtual void removeRadio(const IRadio *radio) = 0;
    virtual const IRadio *getRadio(int id) const = 0;
    virtual void mapRadios(std::function<void (const IRadio *)> f) const = 0;
    //@}

    /** @name Transmission cache */
    //@{
    virtual void addTransmission(const ITransmission *transmission) = 0;
    virtual void removeTransmission(const ITransmission *transmission) = 0;
    virtual const ITransmission *getTransmission(int id) const = 0;
    virtual void mapTransmissions(std::function<void (const ITransmission *)> f) const = 0;
    //@}

    /** @name Interference cache */
    //@{
    virtual std::vector<const ITransmission *> *computeInterferingTransmissions(const IRadio *radio, const simtime_t startTime, const simtime_t endTime) = 0;
    virtual void removeNonInterferingTransmissions(std::function<void (const ITransmission *transmission)> f) = 0;

    virtual const simtime_t getCachedInterferenceEndTime(const ITransmission *transmission) = 0;
    virtual void setCachedInterferenceEndTime(const ITransmission *transmission, const simtime_t interferenceEndTime) = 0;
    virtual void removeCachedInterferenceEndTime(const ITransmission *transmission) = 0;

    virtual const ISignal *getCachedSignal(const ITransmission *transmission) = 0;
    virtual void setCachedSignal(const ITransmission *transmission, const ISignal *signal) = 0;
    virtual void removeCachedSignal(const ITransmission *transmission) = 0;
    //@}

    /** @name Reception Cache */
    //@{
    virtual const IArrival *getCachedArrival(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedArrival(const IRadio *receiver, const ITransmission *transmission, const IArrival *arrival) = 0;
    virtual void removeCachedArrival(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const IntervalTree::Interval *getCachedInterval(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedInterval(const IRadio *receiver, const ITransmission *transmission, const IntervalTree::Interval *interval) = 0;
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

    virtual const ISnir *getCachedSNIR(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedSNIR(const IRadio *receiver, const ITransmission *transmission, const ISnir *snir) = 0;
    virtual void removeCachedSNIR(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const IReceptionDecision *getCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) = 0;
    virtual void setCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part, const IReceptionDecision *receptionDecision) = 0;
    virtual void removeCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) = 0;

    virtual const IReceptionResult *getCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission, const IReceptionResult *receptionResult) = 0;
    virtual void removeCachedReceptionResult(const IRadio *receiver, const ITransmission *transmission) = 0;

    virtual const ISignal *getCachedSignal(const IRadio *receiver, const ITransmission *transmission) = 0;
    virtual void setCachedSignal(const IRadio *receiver, const ITransmission *transmission, const ISignal *signal) = 0;
    virtual void removeCachedSignal(const IRadio *receiver, const ITransmission *transmission) = 0;
    //@}
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_ICOMMUNICATIONCACHE_H

