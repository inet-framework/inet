//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IRECEPTIONDECISION_H
#define __INET_IRECEPTIONDECISION_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the decisions of a receiver's reception process.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IReceptionDecision : public IPrintableObject
{
  public:
    /**
     * Returns the corresponding reception that also specifies the receiver
     * and the received transmission. This function never returns nullptr.
     */
    virtual const IReception *getReception() const = 0;

    /**
     * Returns the signal part of this decision.
     */
    virtual IRadioSignal::SignalPart getSignalPart() const = 0;

    /**
     * Returns whether reception was possible according to the physical
     * properties of the received radio signal.
     */
    virtual bool isReceptionPossible() const = 0;

    /**
     * Returns whether the receiver decided to attempt the reception or
     * it decided to ignore it.
     */
    virtual bool isReceptionAttempted() const = 0;

    /**
     * Returns whether the reception was completely successful or not.
     */
    virtual bool isReceptionSuccessful() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

