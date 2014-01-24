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

#include "ModulePathAddress.h"

#include "InterfaceTableAccess.h"


namespace {
    // copied the cModule::getModuleByRelativePath(), but returns NULL instead throw cRuntimeError
    cModule *getModuleByRelativePath(cModule *modp, const char *path)
    {
        // match components of the path
        opp_string pathbuf(path);
        char *token = strtok(pathbuf.buffer(), ".");
        while (token && modp)
        {
            char *lbracket;
            if ((lbracket=strchr(token,'[')) == NULL)
                modp = modp->getSubmodule(token);  // no index given
            else
            {
                if (token[strlen(token)-1] != ']')
                    return NULL;
                int index = atoi(lbracket+1);
                *lbracket = '\0'; // cut off [index]
                modp = modp->getSubmodule(token, index);
            }
            token = strtok(NULL,".");
        }
        return modp;  // NULL if not found
    }
}

bool ModulePathAddress::tryParse(const char *addr)
{
    cModule *module = getModuleByRelativePath(simulation.getSystemModule(), addr);
    if (module) {
        // accepts network interface modules only:
        if (isNetworkNode(module))
            return false;
        IInterfaceTable *ift = InterfaceTableAccess().get(module);
        if (ift == NULL)
            return false;
        if (ift->getInterfaceByInterfaceModule(module)==NULL)
            return false;
        id = module->getId();
        return true;
    }
    return false;
}

std::string ModulePathAddress::str() const
{
    cModule * module = simulation.getModule(id);
    std::string fullPath = module->getFullPath();
    return strchr(fullPath.c_str(), '.') + 1;
}

