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

#ifndef __INET_IRADIOMEDIUM_H
#define __INET_IRADIOMEDIUM_H

#include "inet/environment/Material.h"
#include "inet/physicallayer/contract/IRadio.h"
#include "inet/physicallayer/contract/IRadioFrame.h"
#include "inet/physicallayer/contract/IArrival.h"
#include "inet/physicallayer/contract/IPropagation.h"
#include "inet/physicallayer/contract/IPathLoss.h"
#include "inet/physicallayer/contract/IObstacleLoss.h"
#include "inet/physicallayer/contract/IAnalogModel.h"
#include "inet/physicallayer/contract/IBackgroundNoise.h"
#include "inet/physicallayer/contract/ISNIR.h"
#include "inet/physicallayer/contract/IReceptionDecision.h"
#include "inet/physicallayer/contract/IListeningDecision.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the whole radio medium. It keeps track of all radio
 * signal sources, all radio signal transmissions, and all radio signal receptions.
 *
 * This interface supports optimistic parallel computation of reception decisions
 * and related information.
 */
class INET_API IRadioMedium : public IPrintableObject
{
  public:
    /**
     * Returns the minimum interference power among the radio receivers is
     * in the range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMinInterferencePower() const = 0;

    /**
     * Returns the minimum reception power among the radio receivers is in
     * the range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMinReceptionPower() const = 0;

    /**
     * Returns the maximum antenna gain among the radio antennas. The value
     * is in the range [1, +infinity) or NaN if unspecified.
     */
    virtual double getMaxAntennaGain() const = 0;

    /**
     * Returns the material of the radio medium. This function never returns NULL.
     */
    virtual const Material *getMaterial() const = 0;

    /**
     * Returns the radio signal propagation model of this radio medium. This
     * function never returns NULL.
     */
    virtual const IPropagation *getPropagation() const = 0;

    /**
     * Returns the radio signal path loss model of this radio medium. This
     * function never returns NULL.
     */
    virtual const IPathLoss *getPathLoss() const = 0;

    /**
     * Returns the radio signal path loss model of this radio medium. This
     * function may return NULL.
     */
    virtual const IObstacleLoss *getObstacleLoss() const = 0;

    /**
     * Returns the radio signal analog model of this radio medium. This
     * function never returns NULL.
     */
    virtual const IAnalogModel *getAnalogModel() const = 0;

    /**
     * Returns the background noise model of this radio medium. This function
     * may return NULL.
     */
    virtual const IBackgroundNoise *getBackgroundNoise() const = 0;

    /**
     * Adds a new radio to the radio medium. An exception is thrown if the
     * radio is already added. The radio may immediately start new transmissions
     * and will potentially receive all ongoing and further transmissions.
     */
    virtual void addRadio(const IRadio *radio) = 0;

    /**
     * Removes a radio from the radio medium. An exception is thrown if the
     * radio is not yet added. The radio cannot start new transmissions and
     * will not receive any further transmission including the ongoing ones.
     */
    virtual void removeRadio(const IRadio *radio) = 0;

    /**
     * Returns a new radio frame containing the radio signal transmission that
     * represents the provided MAC frame. A copy of this radio frame is sent
     * to all affected radios. The MAC frame control info must be an instance
     * of the RadioTransmissionRequest class.
     */
    virtual IRadioFrame *transmitPacket(const IRadio *transmitter, cPacket *macFrame) = 0;

    /**
     * Returns the MAC frame that was transmitted in the provided radio frame.
     * The MAC frame control info will be an instance of the RadioReceptionIndication
     * class.
     */
    virtual cPacket *receivePacket(const IRadio *receiver, IRadioFrame *radioFrame) = 0;

    /**
     * Returns a listening decision that describes what the receiver detects
     * on the radio medium.
     */
    virtual const IListeningDecision *listenOnMedium(const IRadio *receiver, const IListening *listening) const = 0;

    /**
     * Returns true when the radio attempts the reception of the provided
     * transmission.
     */
    virtual bool isReceptionAttempted(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the space and time coordinates of the transmission arriving at
     * the provided receiver. This function never returns NULL as long as the
     * transmission is live on the radio medium.
     */
    virtual const IArrival *getArrival(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the reception of the transmission arriving at the provided receiver.
     * This function never returns NULL as long as the transmission is live on
     * the radio medium.
     */
    virtual const IReception *getReception(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the interference of the transmission arriving at the provided receiver.
     * This function never returns NULL as long as the transmission is live on
     * the radio medium.
     */
    virtual const IInterference *getInterference(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the total noise computed from the interference of the transmission
     * arriving at the provided receiver. This function never returns NULL as
     * long as the transmission is live on the radio medium.
     */
    virtual const INoise *getNoise(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the signal to noise and interference ratio of the transmission
     * arriving at the provided receiver. This function never returns NULL as
     * long as the transmission is live on the radio medium.
     */
    virtual const ISNIR *getSNIR(const IRadio *receiver, const ITransmission *transmission) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IRADIOMEDIUM_H

