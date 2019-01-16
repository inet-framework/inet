//
// Copyright (C) 2007 Vojtech Janota
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_NETWORKINFO_H
#define __INET_NETWORKINFO_H

#include "inet/common/INETDefs.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

/**
 * TODO documentation
 */
class INET_API NetworkInfo : public cSimpleModule, public IScriptable
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  protected:
    virtual void dumpRoutingInfo(cModule *target, const char *filename, bool append, bool compat);
};

} // namespace inet

#endif // ifndef __INET_NETWORKINFO_H

