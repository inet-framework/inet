//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DCTCPFAMILY_H
#define __INET_DCTCPFAMILY_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

/**
 * State variables for DcTcp.
 */
class INET_API DcTcpFamilyStateVariables : public TcpTahoeRenoFamilyStateVariables
{
  public:
    DcTcpFamilyStateVariables();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;

    // DCTCP
    bool dctcp_ce;
    uint32_t dctcp_windEnd;
    uint32_t dctcp_bytesAcked;
    uint32_t dctcp_bytesMarked; // amount of bytes marked
    double dctcp_alpha;
    double dctcp_gamma;
};

/**
 * Provides utility functions to implement DcTcp.
 */
class INET_API DcTcpFamily : public TcpTahoeRenoFamily
{
  protected:
    DcTcpFamilyStateVariables *& state; // alias to TcpAlgorithm's 'state'

  public:
    /** Ctor */
    DcTcpFamily();
};

} // namespace tcp
} // namespace inet

#endif

