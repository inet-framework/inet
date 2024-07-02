//
// Copyright (C) 2020 OpenSimLtd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNAMESPACECONTEXT_H
#define __INET_NETWORKNAMESPACECONTEXT_H

#include "inet/common/INETDefs.h"

namespace inet {

// TODO: what should happen when using multiple threads?
extern std::map<std::string, int> localNetworkNamespaces;

void createNetworkNamespace(const char *name, bool global);
void deleteNetworkNamespace(const char *name);
bool existsNetworkNamespace(const char *name);

class INET_API NetworkNamespaceContext
{
  protected:
    std::string name;
    int oldFd = -1;
    int newFd = -1;
    bool global = false;

  public:
    NetworkNamespaceContext(const char *name);
    ~NetworkNamespaceContext();
};

} // namespace inet

#endif

