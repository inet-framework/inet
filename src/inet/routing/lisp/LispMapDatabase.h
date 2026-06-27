//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_LISPMAPDATABASE_H
#define __INET_LISPMAPDATABASE_H

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/lisp/LispMapStorageBase.h"

namespace inet {
namespace lisp {

/**
 * The map-database: this ETR's own (static) EID-to-RLOC mappings, loaded from
 * configuration and advertised to Map-Servers. Owned by the Lisp module.
 */
class INET_API LispMapDatabase : public LispMapStorageBase
{
  public:
    /**
     * Loads the local mappings from the <EtrMapping> config element, optionally
     * keeping only the EIDs that match this node's interface addresses, and marks
     * locators that are local interface addresses.
     */
    void load(cXMLElement *etrMappingConfig, IInterfaceTable *ift, bool advertOnlyOwnEids);

    bool isOneOfMyEids(L3Address addr) { return lookupMapEntry(addr) != nullptr; }
};

} // namespace lisp
} // namespace inet

#endif
