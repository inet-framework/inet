//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/DcTcpFamily.h"

#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {
namespace tcp {

DcTcpFamilyStateVariables::DcTcpFamilyStateVariables()
{
    dctcp_ce = false;
    dctcp_alpha = 0;
    dctcp_windEnd = snd_una;
    dctcp_bytesAcked = 0;
    dctcp_bytesMarked = 0;
    dctcp_gamma = 0.0625; // 1/16 (backup 0.16) TODO make it NED parameter;
}

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

