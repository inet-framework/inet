//
// Copyright (C) OpenSimLtd.
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

#ifndef __INET_INHERITABLECAPABILITYCONTEXT_H
#define __INET_INHERITABLECAPABILITYCONTEXT_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETUtils.h"
#include "inet/common/LinuxUtils.h"

namespace inet {

class INET_API InheritableCapabilityContext
{
  protected:
    std::vector<cap_value_t> capabilities;
  public:

    InheritableCapabilityContext(const std::vector<cap_value_t>& capabilities = required_capabilities);

    ~InheritableCapabilityContext();
};

} // namespace inet

#endif // ifndef __INET_INHERITABLECAPABILITYCONTEXT_H

