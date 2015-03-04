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

#include "inet/transportlayer/tcp/flavours/TCPTahoeRenoFamily.h"
#include "inet/transportlayer/tcp/TCP.h"

namespace inet {

namespace tcp {

TCPTahoeRenoFamilyStateVariables::TCPTahoeRenoFamilyStateVariables()
{
    // The initial value of ssthresh SHOULD be set arbitrarily high (e.g.,
    // to the size of the largest possible advertised window)
    // Without user interaction there is no limit...
    ssthresh = 0xFFFFFFFF;
}

void TCPTahoeRenoFamilyStateVariables::setSendQueueLimit(uint32 newLimit){
    // The initial value of ssthresh SHOULD be set arbitrarily high (e.g.,
    // to the size of the largest possible advertised window) -> defined by sendQueueLimit
    sendQueueLimit = newLimit;
    ssthresh = sendQueueLimit;
}

std::string TCPTahoeRenoFamilyStateVariables::info() const
{
    std::stringstream out;
    out << TCPBaseAlgStateVariables::info();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TCPTahoeRenoFamilyStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TCPBaseAlgStateVariables::detailedInfo();
    out << "ssthresh=" << ssthresh << "\n";
    return out.str();
}

//---

TCPTahoeRenoFamily::TCPTahoeRenoFamily() : TCPBaseAlg(),
    state((TCPTahoeRenoFamilyStateVariables *&)TCPAlgorithm::state)
{
}

} // namespace tcp

} // namespace inet

