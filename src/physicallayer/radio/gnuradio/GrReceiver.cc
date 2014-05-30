//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "bbn_receiver.h"
#include "bbn_tap.h"
#include "ByteArrayMessage.h"
#include "GenericImplementation.h"
#include "GrReceiver.h"

using namespace std;

Define_Module(GrReceiver);

GrReceiver::GrReceiver()
{
    receiver = bbn_make_receiver(8, 0.4, false);
    receiver->start();
}

GrReceiver::~GrReceiver()
{
    receiver->stop();
}

void GrReceiver::printToStream(std::ostream &out) const
{
    out << "GrReceiver";
}

const IRadioSignalListening *GrReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new RadioSignalListeningBase(radio, startTime, endTime, startPosition, endPosition);
}

const IRadioSignalListeningDecision *GrReceiver::computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    bool isListeningPossible = true; // TODO
    return new RadioSignalListeningDecision(listening, isListeningPossible);
}

bool GrReceiver::computeIsReceptionPossible(const IRadioSignalTransmission *transmission) const
{
    // TODO check carrierFrequency and bandwidth
    return true;
}

bool GrReceiver::computeIsReceptionAttempted(const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions) const
{
    return true;
}

const IRadioSignalReceptionDecision *GrReceiver::computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    const GrSignalReception *currentReception = check_and_cast<const GrSignalReception*>(reception);

    // Compute startTime and endTime.
    //   Signals in the [startTime, endTime] interval are summed and passed to the gnuradio receiver.
    //   To prevent feeding the gnuradio with the same signal twice, we remember the endTime of the
    //   last reception.
    //   We compute the startTime as the minimum of the start time of this reception and the interfering
    //   receptions. (We do not want to pass a long period of noise to the gnuradio.)
    simtime_t startTime = reception->getStartTime();
    if (interferingReceptions)
        for (size_t i = 0; i < interferingReceptions->size(); i++)
            startTime = std::min(startTime, (*interferingReceptions)[i]->getStartTime());
    if (startTime > lastReceptionTime)
        startTime = lastReceptionTime;

    simtime_t endTime = reception->getEndTime();

    // Compute the sum of reception/interfering/noise signals in the [startTime, endTime] interval
    const GrSignal &receivedSignal = currentReception->getPhysicalSignal();
    GrSignal signal(startTime, endTime, receivedSignal.getSampleRate(), receivedSignal.getCarrierFrequency());
    signal.addSignal(receivedSignal);
    if (interferingReceptions)
    {
        for (size_t i = 0; i < interferingReceptions->size(); i++)
        {
            const GrSignalReception *interferingReception = check_and_cast<const GrSignalReception*>((*interferingReceptions)[i]);
            signal.addSignal(interferingReception->getPhysicalSignal());
        }
    }
    if (backgroundNoise)
    {
        const GrSignalNoise *noise  = check_and_cast<const GrSignalNoise*>(backgroundNoise);
        signal.addSignal(noise->getPhysicalSignal());
    }

    // feed combined signal into bbn_receiver
    int length = signal.getNumSamples();
    const char *data = receiver->receive(signal.getSamples(), length);
    lastReceptionTime = endTime;

    // data is empty (length ==0) if the reception failed,
    // otherwise it contains a header (oob_header_t) and a payload
    ByteArrayMessage *macFrame = NULL;
    if (length >= (int)sizeof(oob_hdr_t))
    {
        macFrame = new ByteArrayMessage();
        macFrame->setDataFromBuffer(data+sizeof(oob_hdr_t), length-sizeof(oob_hdr_t));
    }

    RadioReceptionIndication *indication = new RadioReceptionIndication();
    bool isReceptionPossible = true; // TODO
    bool isReceptionAttempted = true; // TODO
    bool isReceptionSuccessful = macFrame != NULL;
    return new GrSignalReceptionDecision(reception, macFrame, indication, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

GrSignalReceptionDecision::GrSignalReceptionDecision(const IRadioSignalReception *reception, cPacket *macFrame, const RadioReceptionIndication *indication,
                                                     bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful)
    : RadioSignalReceptionDecision(reception, indication, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful), macFrame(macFrame)
{
}
