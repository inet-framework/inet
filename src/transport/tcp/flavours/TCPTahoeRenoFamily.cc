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

#include "TCPTahoeRenoFamily.h"
#include "TCP.h"



TCPTahoeRenoFamilyStateVariables::TCPTahoeRenoFamilyStateVariables()
{
    ssthresh = 65535;
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
    out << "ssthresh = " << ssthresh << "\n";
    return out.str();
}

//---

TCPTahoeRenoFamily::TCPTahoeRenoFamily() : TCPBaseAlg(),
  state((TCPTahoeRenoFamilyStateVariables *&)TCPAlgorithm::state)
{
}


