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

#include "inet/common/packet/Packet.h"
#include "inet/environment/contract/IMaterial.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/physicallayer/contract/packetlevel/IAnalogModel.h"
#include "inet/physicallayer/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/contract/packetlevel/IBackgroundNoise.h"
#include "inet/physicallayer/contract/packetlevel/ICommunicationCache.h"
#include "inet/physicallayer/contract/packetlevel/IListeningDecision.h"
#include "inet/physicallayer/contract/packetlevel/IMediumLimitCache.h"
#include "inet/physicallayer/contract/packetlevel/INeighborCache.h"
#include "inet/physicallayer/contract/packetlevel/IObstacleLoss.h"
#include "inet/physicallayer/contract/packetlevel/IPathLoss.h"
#include "inet/physicallayer/contract/packetlevel/IPropagation.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/ISignal.h"
#include "inet/physicallayer/contract/packetlevel/ISnir.h"

namespace inet {
namespace physicallayer {

/**
 * This interface represents the whole medium. It keeps track of radios,
 * signals departures, signal arrivals, and propagating signals.
 *
 * This interface supports optimistic parallel computation.
 */
class INET_API IRadioMedium : public IPrintableObject
{
  public:
    /**
     * This simsignal is emitted when a radio is added to the medium.
     * The simsignal source is the medium and the emitted value is the radio.
     */
    static simsignal_t radioAddedSignal;

    /**
     * This simsignal is emitted when a radio is removed from the medium.
     * The simsignal source is the medium and the emitted value is the radio.
     */
    static simsignal_t radioRemovedSignal;

    /**
     * This simsignal is emitted when a signal is added to the medium.
     * The simsignal source is the medium and the emitted value is the signal.
     */
    static simsignal_t signalAddedSignal;

    /**
     * This simsignal is emitted when a signal is removed from the medium.
     * The simsignal source is the medium and the emitted value is the signal.
     */
    static simsignal_t signalRemovedSignal;

    /**
     * This simsignal is emitted when a signal departure is started on the medium.
     * The simsignal source is the medium and the emitted value is the signal.
     */
    static simsignal_t signalDepartureStartedSignal;

    /**
     * This simsignal is emitted when a signal departure is ended on the medium.
     * The simsignal source is the medium and the emitted value is the signal.
     */
    static simsignal_t signalDepartureEndedSignal;

    /**
     * This simsignal is emitted when a signal arrival is started on the medium.
     * The simsignal source is the medium and the emitted value is the signal.
     */
    static simsignal_t signalArrivalStartedSignal;

    /**
     * This simsignal is emitted when a signal arrival is ended on the medium.
     * The simsignal source is the medium and the emitted value is the signal.
     */
    static simsignal_t signalArrivalEndedSignal;

  public:
    /**
     * Returns the material of this medium.
     * This function never returns nullptr.
     */
    virtual const physicalenvironment::IMaterial *getMaterial() const = 0;

    /**
     * Returns the signal propagation model of this medium.
     * This function never returns nullptr.
     */
    virtual const IPropagation *getPropagation() const = 0;

    /**
     * Returns the signal path loss model of this medium.
     * This function never returns nullptr.
     */
    virtual const IPathLoss *getPathLoss() const = 0;

    /**
     * Returns the signal obstacle loss model of this medium.
     * This function may return nullptr if there's no obstacle loss model.
     */
    virtual const IObstacleLoss *getObstacleLoss() const = 0;

    /**
     * Returns the signal analog model of this medium.
     * This function never returns nullptr.
     */
    virtual const IAnalogModel *getAnalogModel() const = 0;

    /**
     * Returns the background noise model of this medium.
     * This function may return nullptr if there's no background noise model.
     */
    virtual const IBackgroundNoise *getBackgroundNoise() const = 0;

    /**
     * Returns the physical environment model of this medium.
     * This function may return nullptr if there's no physical environment model.
     */
    virtual const physicalenvironment::IPhysicalEnvironment *getPhysicalEnvironment() const = 0;

    virtual const IMediumLimitCache *getMediumLimitCache() const = 0;
    virtual const INeighborCache *getNeighborCache() const = 0;
    virtual const ICommunicationCache *getCommunicationCache() const = 0;

    /**
     * Adds a new radio to the medium. An exception is thrown if the
     * radio is already added. The radio may immediately start new transmissions
     * and will potentially receive all ongoing and further transmissions.
     */
    virtual void addRadio(const IRadio *radio) = 0;

    /**
     * Removes a radio from the medium. An exception is thrown if the
     * radio is not yet added. The radio cannot start new transmissions and
     * will not receive any further transmission including the ongoing ones.
     */
    virtual void removeRadio(const IRadio *radio) = 0;

    /**
     * Returns radio instance from medium by id.
     * May return a nullptr if no matching radio is registered.
     */
    virtual const IRadio *getRadio(int id) const = 0;

    /**
     * Returns a new signal containing the signal transmission that
     * represents the provided packet. A copy of this signal is sent to
     * all affected radios.
     */
    virtual ISignal *transmitPacket(const IRadio *transmitter, Packet *packet) = 0;

    /**
     * Returns the packet that was transmitted in the provided signal.
     */
    virtual Packet *receivePacket(const IRadio *receiver, ISignal *signal) = 0;

    /**
     * Returns transmission instance from medium by id.
     * May return a nullptr if no matching transmission is registered.
     */
    virtual const ITransmission *getTransmission(int id) const = 0;

    /**
     * Returns the listening decision that describes what the receiver detects
     * on the medium.
     */
    virtual const IListeningDecision *listenOnMedium(const IRadio *receiver, const IListening *listening) const = 0;

    /**
     * Returns the space and time coordinates of the transmission arriving at
     * the provided receiver. This function never returns nullptr as long as the
     * transmission is live on the medium.
     */
    virtual const IArrival *getArrival(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns how the radio is listening on the medium when the transmission
     * arrives at the provided receiver. This function never returns nullptr as
     * long as the transmission is live on the medium.
     */
    virtual const IListening *getListening(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the reception of the transmission arriving at the provided receiver.
     * This function never returns nullptr as long as the transmission is live on
     * the medium.
     */
    virtual const IReception *getReception(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the interference of the transmission arriving at the provided receiver.
     * This function never returns nullptr as long as the transmission is live on
     * the medium.
     */
    virtual const IInterference *getInterference(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the total noise computed from the interference of the transmission
     * arriving at the provided receiver. This function never returns nullptr as
     * long as the transmission is live on the medium.
     */
    virtual const INoise *getNoise(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the signal to noise and interference ratio of the transmission
     * arriving at the provided receiver. This function never returns nullptr as
     * long as the transmission is live on the medium.
     */
    virtual const ISnir *getSNIR(const IRadio *receiver, const ITransmission *transmission) const = 0;

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

