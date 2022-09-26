//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4NETWORKLAYER_H
#define __INET_IPV4NETWORKLAYER_H

#include "inet/common/INETDefs.h"
#include "inet/common/StringFormat.h"

namespace inet {

class INET_API Ipv4NetworkLayer : public cModule, public StringFormat::IDirectiveResolver
{
  protected:
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;
    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif

