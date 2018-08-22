//
// Copyright (C) 2004 Andras Varga
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

#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

namespace tcp {

Register_Class(Sack);

bool Sack::empty() const
{
    return start == 0 && end == 0;
}

bool Sack::contains(const Sack& other) const
{
    return seqLE(start, other.start) && seqLE(other.end, end);
}

void Sack::clear()
{
    start = end = 0;
}

void Sack::setSegment(unsigned int start_par, unsigned int end_par)
{
    setStart(start_par);
    setEnd(end_par);
}

std::string Sack::str() const
{
    std::stringstream out;

    out << "[" << start << ".." << end << ")";
    return out.str();
}

B TcpHeader::getHeaderOptionArrayLength()
{
    unsigned short usedLength = 0;

    for (uint i = 0; i < getHeaderOptionArraySize(); i++)
        usedLength += getHeaderOption(i)->getLength();

    return B(usedLength);
}

void TcpHeader::dropHeaderOptions()
{
    setHeaderOptionArraySize(0);
    setHeaderLength(TCP_MIN_HEADER_LENGTH);
    setChunkLength(TCP_MIN_HEADER_LENGTH);
}

std::string TcpHeader::str() const
{
    std::ostringstream stream;
    stream << getClassName() << ", ";
    static const char *flagStart = " [";
    static const char *flagSepar = " ";
    static const char *flagEnd = "]";
    stream << getSrcPort() << "->" << getDestPort();
    const char *separ = flagStart;
    if (getCwrBit()) {
        stream << separ << "Cwr";
        separ = flagSepar;
    }
    if (getEceBit()) {
        stream << separ << "Ece";
        separ = flagSepar;
    }
    if (getUrgBit()) {
        stream << separ << "Urg=" << getUrgentPointer();
        separ = flagSepar;
    }
    if (getSynBit()) {
        stream << separ << "Syn";
        separ = flagSepar;
    }
    if (getAckBit()) {
        stream << separ << "Ack=" << getAckNo();
        separ = flagSepar;
    }
    if (getPshBit()) {
        stream << separ << "Psh";
        separ = flagSepar;
    }
    if (getRstBit()) {
        stream << separ << "Rst";
        separ = flagSepar;
    }
    if (getFinBit()) {
        stream << separ << "Fin";
        separ = flagSepar;
    }
    if (separ == flagSepar)
        stream << flagEnd;

    stream << " Seq=" << getSequenceNo()
           << " Win=" << getWindow()
           << ", length = " << getChunkLength();

    //TODO show TCP Options

    return stream.str();
}


} // namespace tcp

} // namespace inet

