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

#include "bbn_transmitter.h"
#include "ByteArrayMessage.h"
#include "GrTransmitter.h"

using namespace std;

Define_Module(GrTransmitter);

GrTransmitter::GrTransmitter()
{
    transmitter = bbn_make_transmitter(8, 0.4, 1.0, false);
    transmitter->start();
}

GrTransmitter::~GrTransmitter()
{
    transmitter->stop();
}

/*
 * TODO reuse bbn_transmitter
 */
const IRadioSignalTransmission *GrTransmitter::createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const
{
    const ByteArrayMessage *msg = check_and_cast<const ByteArrayMessage*>(packet);
    int payloadLength = msg->getByteArray().getDataArraySize();
    char *payload = new char[payloadLength];
    msg->copyDataToBuffer(payload, payloadLength);

    int length = payloadLength;
    gr_complex *samples = transmitter->transmit(payload, length);

    // In the physical signal there is 8 sample per symbol
    // and in 802.11b the symbol rate is 1Mbps
    const Hz sampleRate = Hz(8e6);
    const Hz carrierFrequency = Hz(2.4e9); // TODO
    simtime_t duration = length / sampleRate.get(); // FIXME there are 56 extra bits added by BBN transmitter
    simtime_t endTime = startTime + duration;
    GrSignal physicalSignal(startTime, endTime, samples, length, sampleRate, carrierFrequency);

    IMobility *mobility = radio->getAntenna()->getMobility();
    Coord startPosition = mobility->getPosition(startTime);
    Coord endPosition = mobility->getPosition(endTime);
    return new GrSignalTransmission(radio, packet, startTime, endTime, startPosition, endPosition, physicalSignal); // FIXME do not copy samples
}

void GrTransmitter::printToStream(std::ostream &out) const
{
    out << "GrTransmitter";
}
