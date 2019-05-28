//
// Copyright (C) 2013 Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Author: Andras Varga (andras@omnetpp.org)
//

#ifndef __INET_MACBASE_H
#define __INET_MACBASE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

class InterfaceEntry;

/**
 * Base class for MAC modules.
 */
class INET_API MacBase : public MacProtocolBase
{
  public:
    MacBase() {}
    virtual ~MacBase();

  protected:
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif // ifndef __INET_MACBASE_H

