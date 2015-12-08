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

#include "inet/environment/contract/IMaterial.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/physicallayer/contract/packetlevel/IAnalogModel.h"
#include "inet/physicallayer/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/contract/packetlevel/IBackgroundNoise.h"
#include "inet/physicallayer/contract/packetlevel/IListeningDecision.h"
#include "inet/physicallayer/contract/packetlevel/IObstacleLoss.h"
#include "inet/physicallayer/contract/packetlevel/IPathLoss.h"
#include "inet/physicallayer/contract/packetlevel/IPropagation.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/IRadioFrame.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/ISNIR.h"

namespace inet {

namespace physicallayer {

using namespace inet::physicalenvironment;

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
     * Returns the material of the radio medium. This function never returns nullptr.
     */
    virtual const IMaterial *getMaterial() const = 0;

    /**
     * Returns the radio signal propagation model of this radio medium. This
     * function never returns nullptr.
     */
    virtual const IPropagation *getPropagation() const = 0;

    /**
     * Returns the radio signal path loss model of this radio medium. This
     * function never returns nullptr.
     */
    virtual const IPathLoss *getPathLoss() const = 0;

    /**
     * Returns the radio signal obstacle loss model of this radio medium. This
     * function may return nullptr if there's no obstacle loss model.
     */
    virtual const IObstacleLoss *getObstacleLoss() const = 0;

    /**
     * Returns the radio signal analog model of this radio medium. This
     * function never returns nullptr.
     */
    virtual const IAnalogModel *getAnalogModel() const = 0;

    /**
     * Returns the background noise model of this radio medium. This function
     * may return nullptr if there's no background noise model.
     */
    virtual const IBackgroundNoise *getBackgroundNoise() const = 0;

    /**
     * Returns the physical environment model of this radio medium. This function
     * may return nullptr if there's no physical environment model.
     */
    virtual const IPhysicalEnvironment *getPhysicalEnvironment() const = 0;

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
     * of the TransmissionRequest class.
     */
    virtual IRadioFrame *transmitPacket(const IRadio *transmitter, cPacket *macFrame) = 0;

    /**
     * Returns the MAC frame that was transmitted in the provided radio frame.
     * The MAC frame control info will be an instance of the ReceptionIndication
     * class.
     */
    virtual cPacket *receivePacket(const IRadio *receiver, IRadioFrame *radioFrame) = 0;

    /**
     * Returns the listening decision that describes what the receiver detects
     * on the radio medium.
     */
    virtual const IListeningDecision *listenOnMedium(const IRadio *receiver, const IListening *listening) const = 0;

    /**
     * Returns the space and time coordinates of the transmission arriving at
     * the provided receiver. This function never returns nullptr as long as the
     * transmission is live on the radio medium.
     */
    virtual const IArrival *getArrival(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns how the radio is listening on the medium when the transmission
     * arrives at the provided receiver. This function never returns nullptr as
     * long as the transmission is live on the radio medium.
     */
    virtual const IListening *getListening(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the reception of the transmission arriving at the provided receiver.
     * This function never returns nullptr as long as the transmission is live on
     * the radio medium.
     */
    virtual const IReception *getReception(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the interference of the transmission arriving at the provided receiver.
     * This function never returns nullptr as long as the transmission is live on
     * the radio medium.
     */
    virtual const IInterference *getInterference(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the total noise computed from the interference of the transmission
     * arriving at the provided receiver. This function never returns nullptr as
     * long as the transmission is live on the radio medium.
     */
    virtual const INoise *getNoise(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the signal to noise and interference ratio of the transmission
     * arriving at the provided receiver. This function never returns nullptr as
     * long as the transmission is live on the radio medium.
     */
    virtual const ISNIR *getSNIR(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns true when the reception is possible of the transmission part.
     */
    virtual bool isReceptionPossible(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const = 0;

    /**
     * Returns true when the reception is attempted of the transmission part.
     */
    virtual bool isReceptionAttempted(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const = 0;

    /**
     * Returns true when the reception is successful of the transmission part.
     */
    virtual bool isReceptionSuccessful(const IRadio *receiver, const ITransmission *transmission, IRadioSignal::SignalPart part) const = 0;

    /**
     * Returns the reception decision for the transmission part that specifies
     * whether the reception is possible, attempted, and successful.
     */
    virtual const IReceptionDecision *getReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part) const = 0;

    /**
     * Returns the reception result for the transmission that describes the end
     * result of the reception process.
     */
    virtual const IReceptionResult *getReceptionResult(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IRADIOMEDIUM_H

