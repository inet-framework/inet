//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    virtual const char *resolveDirective(char directive) const override;
};

} // namespace inet

#endif

