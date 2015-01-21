//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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

//  Cleanup and rewrite: Andras Varga, 2004

#ifndef __INET_ERRORHANDLING_H
#define __INET_ERRORHANDLING_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Error Handling: print out received error
 */
// FIXME is such thing needed at all???
class INET_API ErrorHandling : public cSimpleModule
{
  protected:
    long numReceived;
    long numHostUnreachable;
    long numTimeExceeded;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif // ifndef __INET_ERRORHANDLING_H

