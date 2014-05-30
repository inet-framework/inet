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

#include "GrReceiver.h"
#include "Radio.h"
#include "RadioControlInfo_m.h"
#include "GrRadioChannel.h"

using namespace std;

Define_Module(GrRadioChannel);

cPacket *GrRadioChannel::receivePacket(const IRadio *radio, IRadioFrame *radioFrame)
{
    const IRadioSignalTransmission *transmission = radioFrame->getTransmission();
    const IRadioSignalListening *listening = radio->getReceiver()->createListening(radio, transmission->getStartTime(), transmission->getEndTime(), transmission->getStartPosition(), transmission->getEndPosition());
    const IRadioSignalReceptionDecision *decision = receiveFromChannel(radio, listening, transmission);
    if (recordCommunication) {
        const IRadioSignalReception *reception = decision->getReception();
        communicationLog << "R " << check_and_cast<const Radio *>(radio)->getFullPath() << " " << reception->getReceiver()->getId() << " "
                         << "M " << check_and_cast<const RadioFrame *>(radioFrame)->getName() << " " << transmission->getId() << " "
                         << "S " << reception->getStartTime() << " " <<  reception->getStartPosition() << " -> "
                         << "E " << reception->getEndTime() << " " <<  reception->getEndPosition() << " "
                         << "D " << decision->isReceptionPossible() << " " << decision->isReceptionAttempted() << " " << decision->isReceptionSuccessful() << endl;
    }
    cPacket *macFrame = check_and_cast<const GrSignalReceptionDecision *>(decision)->getMacFrame();
    macFrame->setBitError(!decision->isReceptionSuccessful());
    macFrame->setControlInfo(const_cast<RadioReceptionIndication *>(decision->getIndication()));
    delete listening;
    // delete decision; ?
    return macFrame;
}

const IRadioSignalLoss *GrSignalAttenuation::computeLoss(const IRadioSignalTransmission *transmission, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) const
{
    return NULL;
}

// basic lossless propagation (for testing)
const IRadioSignalReception *GrSignalAttenuation::computeReception(const IRadio *receiver, const IRadioSignalTransmission *transmission) const
{
    const IRadioChannel *channel = receiver->getChannel();
    const IRadioSignalArrival *arrival = channel->getArrival(receiver, transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const GrSignal &physicalSignal = check_and_cast<const GrSignalTransmission *>(transmission)->getPhysicalSignal();
    return new GrSignalReception(receiver, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, physicalSignal);
}
