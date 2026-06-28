//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPSITERECORD_H
#define __INET_LISPSITERECORD_H

#include <list>

#include "inet/routing/lisp/LispMapStorageBase.h"
#include "inet/routing/lisp/LispServerEntry.h"

namespace inet {
namespace lisp {

/**
 * One registered ETR within a LISP site (Map-Server side): the ETR's server
 * entry plus the EID-to-RLOC mappings it registered (inherited storage).
 */
class INET_API LispSiteRecord : public LispMapStorageBase
{
  protected:
    LispServerEntry ServerEntry;

  public:
    LispSiteRecord() {}
    virtual ~LispSiteRecord() {}

    bool operator==(const LispSiteRecord& other) const;
    bool operator<(const LispSiteRecord& other) const;

    LispServerEntry& getServerEntry() { return ServerEntry; }
    void setServerEntry(const LispServerEntry& serverEntry) { ServerEntry = serverEntry; }

    std::string str() const;
};

typedef std::list<LispSiteRecord> Etrs;
typedef Etrs::iterator EtrItem;
typedef Etrs::const_iterator EtrCItem;

std::ostream& operator<<(std::ostream& os, const LispSiteRecord& sr);

} // namespace lisp
} // namespace inet

#endif
