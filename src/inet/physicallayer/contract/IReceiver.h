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

#include "inet/physicallayer/contract/IListening.h"
#include "inet/physicallayer/contract/INoise.h"
#include "inet/physicallayer/contract/IReception.h"
#include "inet/physicallayer/contract/IInterference.h"
#include "inet/physicallayer/contract/IListeningDecision.h"
#include "inet/physicallayer/contract/IReceptionDecision.h"
#include "inet/physicallayer/contract/RadioControlInfo_m.h"

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
 * the actual result is needed. This process is controlled by the radio medium.
 */
// TODO: this is rather an interface for receivers that support "what if" questions for the future (parallel computation)
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
     * Returns a description of how the receiver is listening on the radio medium.
     */
    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const = 0;

    /**
     * Returns the result of the listening process specifying the reception
     * state of the receiver. This function must be purely functional and
     * support optimistic parallel computation.
     */
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const = 0;

    /**
     * Returns whether the transmission can be received successfully or not.
     * This function need not be purely functional and need not support
     * optimistic parallel computation.
     */
    virtual bool computeIsReceptionPossible(const ITransmission *transmission) const = 0;

    /**
     * Returns whether the transmission represented by the reception can be
     * received successfully or not. This function must be purely functional
     * and support optimistic parallel computation.
     */
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception) const = 0;

    /**
     * Returns whether the reception is actually attempted or ignored by the
     * receiver. This function must be purely functional and support optimistic
     * parallel computation.
     */
    virtual bool computeIsReceptionAttempted(const IListening *listening, const IReception *reception, const IInterference *interference) const = 0;

    /**
     * Returns the result of the reception process specifying whether it was
     * successful or not and any other physical properties. This function must
     * be purely functional and support optimistic parallel computation.
     */
    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IRECEIVER_H

