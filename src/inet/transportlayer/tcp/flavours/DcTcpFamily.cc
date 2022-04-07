//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/DcTcpFamily.h"

#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {
namespace tcp {

std::string DcTcpFamilyStateVariables::str() const
{
    std::stringstream out;
    out << TcpTahoeRenoFamilyStateVariables::str();
    out << " dctcp_alpha=" << dctcp_alpha;
    out << " dctcp_windEnd=" << dctcp_windEnd;
    out << " dctcp_bytesAcked=" << dctcp_bytesAcked;
    out << " dctcp_bytesMarked=" << dctcp_bytesMarked;
    out << " dctcp_gamma=" << dctcp_gamma;

    return out.str();
}

std::string DcTcpFamilyStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpTahoeRenoFamilyStateVariables::detailedInfo();
    out << " dctcp_alpha=" << dctcp_alpha;
    out << " dctcp_windEnd=" << dctcp_windEnd;
    out << " dctcp_bytesAcked=" << dctcp_bytesAcked;
    out << " dctcp_bytesMarked=" << dctcp_bytesMarked;
    out << " dctcp_gamma=" << dctcp_gamma;
    return out.str();
}

// ---

DcTcpFamily::DcTcpFamily() : TcpTahoeRenoFamily(),
    state((DcTcpFamilyStateVariables *&)TcpTahoeRenoFamily::state)
{
}

} // namespace tcp
} // namespace inet

