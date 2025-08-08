//
// Copyright (C) 2008-2009 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPHEADER_H
#define __INET_SCTPHEADER_H

#include <list>

#include "inet/common/INETDefs.h"
//#include "inet/transportlayer/contract/ITransportPacket.h"
#include "inet/transportlayer/sctp/SctpHeader_m.h"

namespace inet {
namespace sctp {

class INET_API SctpIncomingSsnResetRequestParameter : public SctpIncomingSsnResetRequestParameter_Base
{
  private:
    void copy(const SctpIncomingSsnResetRequestParameter& other);

  public:
    SctpIncomingSsnResetRequestParameter(const char *name = nullptr, int kind = 0) : SctpIncomingSsnResetRequestParameter_Base() {}
    SctpIncomingSsnResetRequestParameter(const SctpIncomingSsnResetRequestParameter& other) : SctpIncomingSsnResetRequestParameter_Base(other) { copy(other); }
    SctpIncomingSsnResetRequestParameter& operator=(const SctpIncomingSsnResetRequestParameter& other) { if (this == &other) return *this; SctpIncomingSsnResetRequestParameter_Base::operator=(other); copy(other); return *this; }
    virtual SctpIncomingSsnResetRequestParameter *dup() const override { return new SctpIncomingSsnResetRequestParameter(*this); }
};

} // namespace sctp
} // namespace inet

#endif

