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

#include "Radio.h"
#include "ImplementationBase.h"
#include "PhyControlInfo_m.h"

unsigned int Radio::nextId = 0;

Radio::~Radio()
{
    delete antenna;
    delete transmitter;
    delete receiver;
}

void Radio::printToStream(std::ostream &stream) const
{
    stream << (cSimpleModule *)this;
}

IRadioFrame *Radio::transmitPacket(cPacket *packet, const simtime_t startTime)
{
    const IRadioSignalTransmission *transmission = transmitter->createTransmission(this, packet, startTime);
    channel->transmitToChannel(this, transmission);
    RadioFrame *radioFrame = new RadioFrame(transmission);
    radioFrame->setName(packet->getName());
    radioFrame->setDuration(transmission->getDuration());
    radioFrame->encapsulate(packet);
    return radioFrame;
}

cPacket *Radio::receivePacket(IRadioFrame *frame)
{
    const IRadioSignalTransmission *transmission = frame->getTransmission();
    const IRadioSignalListening *listening = receiver->createListening(this, transmission->getStartTime(), transmission->getEndTime(), transmission->getStartPosition(), transmission->getEndPosition());
    const IRadioSignalReceptionDecision *radioDecision = channel->receiveFromChannel(this, listening, transmission);
    cPacket *packet = check_and_cast<cPacket *>(frame)->decapsulate();
    if (!radioDecision->isReceptionSuccessful())
        packet->setKind(radioDecision->getBitErrorCount() > 0 ? BITERROR : COLLISION);
    packet->setControlInfo(const_cast<cObject *>(check_and_cast<const cObject *>(radioDecision)));
    return packet;
}
