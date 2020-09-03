//
// Copyright (C) 2004 OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

TcpTahoeRenoFamilyStateVariables::TcpTahoeRenoFamilyStateVariables()
{

}

void TcpTahoeRenoFamilyStateVariables::setSendQueueLimit(uint32 newLimit){
    // The initial value of ssthresh SHOULD be set arbitrarily high (e.g.,
    // to the size of the largest possible advertised window) -> defined by sendQueueLimit
    sendQueueLimit = newLimit;
    ssthresh = sendQueueLimit;
}

std::string TcpTahoeRenoFamilyStateVariables::str() const
{
    std::stringstream out;
    out << TcpBaseAlgStateVariables::str();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TcpTahoeRenoFamilyStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpBaseAlgStateVariables::detailedInfo();
    out << "ssthresh=" << ssthresh << "\n";
    return out.str();
}

//---

TcpTahoeRenoFamily::TcpTahoeRenoFamily() : TcpBaseAlg(),
    state((TcpTahoeRenoFamilyStateVariables *&)TcpAlgorithm::state)
{

}

void TcpTahoeRenoFamily::initialize()
{
    TcpBaseAlg::initialize();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");

}

} // namespace tcp
} // namespace inet

