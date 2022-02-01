//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE80211LLC_H
#define __INET_IIEEE80211LLC_H

#include "inet/common/Protocol.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211Llc
{
  public:
    virtual const Protocol *getProtocol() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

