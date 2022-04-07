//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DCTCPFAMILY_H
#define __INET_DCTCPFAMILY_H

#include "inet/transportlayer/tcp/flavours/DcTcpFamilyState_m.h"
#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

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

