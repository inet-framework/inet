//
// Copyright (C) 2020 OpenSimLtd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNAMESPACECONTEXT_H
#define __INET_NETWORKNAMESPACECONTEXT_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API NetworkNamespaceContext
{
  protected:
    int oldNs = -1;
    int newNs = -1;

  public:
    NetworkNamespaceContext(const char *networkNamespace);
    ~NetworkNamespaceContext();
};

} // namespace inet

#endif

