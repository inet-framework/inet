//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __PROTOCOLMAP_H__
#define __PROTOCOLMAP_H__

#include <vector>
#include "INETDefs.h"

/**
 * Maps protocol numbers to output gates
 */
class INET_API ProtocolMapping
{
  private:
    struct Entry
    {
        int protocolNumber;
        int outGateIndex;
    };
    // we use vector because it's probably faster: we'll hit 1st or 2nd entry
    // (TCP or UDP) in 90% of cases
    typedef std::vector<Entry> Entries;
    Entries entries;

  public:
    ProtocolMapping() {}
    ~ProtocolMapping() {}
    void parseProtocolMapping(const char *s);
    int outputGateForProtocol(int protocol);
};

#endif
