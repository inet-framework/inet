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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "QueueAppSocket.h"

namespace inet {
namespace quic {

QueueAppSocket::QueueAppSocket(Quic *quicSimpleMod) : AppSocket(quicSimpleMod, -1) { }

QueueAppSocket::~QueueAppSocket() { }

void QueueAppSocket::sendIndication(Indication *indication)
{
    indications.push_back(indication);
}

void QueueAppSocket::sendPacket(Packet *pkt)
{
    throw cRuntimeError("QueueAppSocket::sendPacket: cannot send packets with a QueueAppSocket.");
}

void QueueAppSocket::processAppCommand(cMessage *msg)
{
    throw cRuntimeError("QueueAppSocket::processAppCommand: cannot process app commands in QueueAppSocket.");
}

std::list<Indication *> QueueAppSocket::getIndications()
{
    return indications;
}

} /* namespace quic */
} /* namespace inet */
