//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
//               2009 Zoltan Bojthe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPCHECKSUM_H
#define __INET_SCTPCHECKSUM_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace sctp {

/**
 * Calculates checksum.
 */
class INET_API SctpChecksum
{
  public:
    SctpChecksum() {}

    static uint32_t checksum(const void *addr, unsigned int count);
};

} // namespace sctp
} // namespace inet

#endif

