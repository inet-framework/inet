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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_LINKLAYERNODEBASE_H_
#define __INET_LINKLAYERNODEBASE_H_

#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class LinkLayerNodeBase : public cModule
{
  friend class LinkLayerNodeBaseDescriptor;

  protected:
    int getNumInterfaces() const { return check_and_cast<IInterfaceTable *>(getSubmodule("interfaceTable"))->getNumInterfaces(); } // only for class descriptor
    const InterfaceEntry *getInterface(int i) const { return check_and_cast<IInterfaceTable *>(getSubmodule("interfaceTable"))->getInterface(i); } // only for class descriptor
};

} // namespace inet

#endif // __INET_LINKLAYERNODEBASE_H_
