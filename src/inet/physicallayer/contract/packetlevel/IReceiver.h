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

#ifndef __INET_IRECEIVER_H
#define __INET_IRECEIVER_H

#include "inet/physicallayer/contract/packetlevel/IInterference.h"
#include "inet/physicallayer/contract/packetlevel/IListening.h"
#include "inet/physicallayer/contract/packetlevel/IListeningDecision.h"
#include "inet/physicallayer/contract/packetlevel/INoise.h"
#include "inet/physicallayer/contract/packetlevel/IRadioSignal.h"
#include "inet/physicallayer/contract/packetlevel/IReception.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionResult.h"
#include "inet/physicallayer/contract/packetlevel/ISNIR.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents a physical device (a part of the radio) which converts
 * electric signals into packets.
 *
 * The receiver interface supports optimistic parallel computation of reception
 * results. For this reason some functions are marked to be purely functional.
 * Unfortunately this cannot be enforced by the compiler in C++. We really need
 * to be very careful regarding this, because it's pretty easy to break the purely
 * functional API just be reading a non-const member variable. As a rule of thumb
 * these functions must be qualified with const and all of their parameters must
 * also be qualified with const. Moreover they are forbidden to access any state
 * that can change over time to avoid non-deterministic behavior. These functions
 * may be called from background threads running parallel with the main simulation
 * thread. They may also be called several times due to cache invalidation before
 * the actual result is needed. This process is controlled by the medium.
 */
class INET_API IReceiver : public IPrintableObject
{
  public:
    /**
     * Returns the minimum interference power below which receptions are to
     * be ignored while computing the interference. Returns a value in the
     * range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMinInterferencePower() const = 0;

    /**
     * Returns the minimum reception power below which successful reception
     * is definitely not possible. Returns a value in the range [0, +infinity)
     * or NaN if unspecified.
     */
    virtual W getMinReceptionPower() const = 0;

    /**
     * Returns a description of how the receiver is listening on the medium.
     */
    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const = 0;

    /**
     * Returns the result of the listening process specifying the reception
     * state of the receiver. This function must be purely functional and
     * support optimistic parallel computation.
     */
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const = 0;

    /**
     * Returns whether the reception of the provided transmission is possible or
     * not independently of the reception conditions. For example, it might check
     * if the carrier frequency and the modulation of the transmission matches
     * how the receiver is listening on the medium.
     *
     * This function may be called before the reception actually starts at the
     * receiver, thus it must be purely functional and support optimistic
     * parallel computation.
     */
    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const = 0;

    /**
     * Returns whether the reception of the provided part is possible or not.
     * For example, it might check if the reception power is above sensitivity.
     *
     * This function may be called before the reception actually starts at the
     * receiver, thus it must be purely functional and support optimistic
     * parallel computation.
     */
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const = 0;

    /**
     * Returns whether the reception of the provided part is actually attempted
     * or ignored by the receiver. For example, it might check that the radio is
     * not already receiving another signal.
     *
     * This function may be called before the reception actually starts at the
     * receiver, thus it must be purely functional and support optimistic
     * parallel computation.
     */
    virtual bool computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const = 0;

    /**
     * Returns whether the reception of the provided part is actually successful
     * or failed by the receiver. For example, it might compute the error rate
     * and draw a random number to make the decision.
     *
     * This function may be called before the reception actually starts at the
     * receiver, thus it must be purely functional and support optimistic
     * parallel computation.
     */
    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISNIR *snir) const = 0;

    /**
     * Returns the reception decision for the transmission part that specifies
     * whether the reception is possible, attempted, and successful.
     *
     * This function may be called before the reception actually starts at the
     * receiver, thus it must be purely functional and support optimistic
     * parallel computation.
     */
    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISNIR *snir) const = 0;

    /**
     * Returns the complete result of the reception process for the provided reception.
     *
     * This function may be called before the reception actually starts at the
     * receiver, thus it must be purely functional and support optimistic
     * parallel computation.
     */
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISNIR *snir) const = 0;

    /**
     * Returns the reception indication (control info) that is sent up to the MAC.
     *
     * This function may be called before the reception actually starts at the
     * receiver, thus it must be purely functional and support optimistic
     * parallel computation.
     */
    virtual const ReceptionIndication *computeReceptionIndication(const ISNIR *snir) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IRECEIVER_H

