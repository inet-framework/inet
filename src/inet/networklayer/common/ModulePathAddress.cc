//
// Copyright (C) 2012 Opensim Ltd.
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

#include "inet/networklayer/common/ModulePathAddress.h"

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

namespace {

// copied the cModule::getModuleByRelativePath(), but returns nullptr instead throw cRuntimeError
cModule *getModuleByRelativePath(cModule *modp, const char *path)
{
    // match components of the path
    opp_string pathbuf(path);
    char *token = strtok(pathbuf.buffer(), ".");
    while (token && modp) {
        char *lbracket;
        if ((lbracket = strchr(token, '[')) == nullptr)
            modp = modp->getSubmodule(token); // no index given
        else {
            if (token[strlen(token) - 1] != ']')
                return nullptr;
            int index = atoi(lbracket + 1);
            *lbracket = '\0';    // cut off [index]
            modp = modp->getSubmodule(token, index);
        }
        token = strtok(nullptr, ".");
    }
    return modp;    // nullptr if not found
}

} // namespace

bool ModulePathAddress::tryParse(const char *addr)
{
    cModule *module = getModuleByRelativePath(getSimulation()->getSystemModule(), addr);
    if (module) {
        // accepts network interface modules only:
        if (isNetworkNode(module))
            return false;
        IInterfaceTable *ift = L3AddressResolver().findInterfaceTableOf(findContainingNode(module));
        if (ift == nullptr)
            return false;
        if (ift->getInterfaceByInterfaceModule(module) == nullptr)
            return false;
        id = module->getId();
        return true;
    }
    return false;
}

std::string ModulePathAddress::str() const
{
    if (id == 0) {
        return "<unspec>";
    }
    else if (id == -1) {
        return "<BROADCAST>";
    }
    else if (id < -1) {
        std::ostringstream s;
        s << "<MULTICAST ID=" << -id << ">";
        return s.str();
    }
    else /* if (id > 0) */ {
        cModule *module = getSimulation()->getModule(id);
        if (module) {
            std::string fullPath = module->getFullPath();
            return strchr(fullPath.c_str(), '.') + 1;
        }
        std::ostringstream s;
        s << "<module ID=" << id << ">";
        return s.str();
    }
}

} // namespace inet

