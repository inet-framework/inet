//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_WATCHLIST_H
#define __INET_WATCHLIST_H

#include <vector>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Deletes the WATCHes of an object's members together with the object.
 *
 * A WATCH is owned by the module in whose context it is created, and it lives
 * as long as that module. When an object that is not a module (e.g. a protocol
 * state object that is deleted and recreated on node shutdown and restart)
 * watches its own members, its WATCHes would outlive it and refer to freed
 * memory, and the module's inspector would crash on them. Such an object
 * should add its WATCHes to a WatchList member:
 *
 * <pre>
 * watches.add(WATCH(routingTable));
 * </pre>
 *
 * Not copyable, as the WATCHes refer to the members of one object.
 */
class INET_API WatchList
{
  private:
    std::vector<cWatchBase *> watches;

  public:
    WatchList() {}
    WatchList(const WatchList&) = delete;
    WatchList& operator=(const WatchList&) = delete;
    ~WatchList() { for (auto watch : watches) delete watch; }

    void add(cWatchBase *watch) { watches.push_back(watch); }
};

} // namespace inet

#endif
