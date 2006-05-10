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


#include "ModuleAccess.h"

static cModule *findSubmodRecursive(cModule *curmod, const char *name, const char *classname)
{
    for (cSubModIterator i(*curmod); !i.end(); i++)
    {
        cModule *submod = i();
        if (!strcmp(submod->fullName(), name))
            return submod;
        cModule *foundmod = findSubmodRecursive(submod, name, classname);
        if (foundmod)
            return foundmod;
    }
    return NULL;
}

cModule *findModuleWherever(const char *name, const char *classname, cModule *from)
{
    cModule *mod = NULL;
    for (cModule *curmod=from; !mod && curmod; curmod=curmod->parentModule())
        mod = findSubmodRecursive(curmod, name, classname);
    return mod;
}

cModule *findModuleSomewhereUp(const char *name, cModule *from)
{
    cModule *mod = NULL;
    for (cModule *curmod=from; !mod && curmod; curmod=curmod->parentModule())
        mod = curmod->submodule(name);
    return mod;
}

