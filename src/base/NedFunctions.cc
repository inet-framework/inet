//
// Copyright (C) 1992-2004 Andras Varga
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


#include "INETDefs.h"

// compatibility for pre-4.2b3 omnetpp
#ifndef Define_NED_Math_Function
#define cNEDValue  cDynamicExpression::Value
#define stringValue()  s.c_str()
#define stdstringValue()  s
#endif

cNEDValue nedf_haveClass(cComponent *context, cNEDValue argv[], int argc)
{
    return classes.getInstance()->lookup(argv[0].stringValue()) != NULL;
}

Define_NED_Function2(nedf_haveClass,
        "bool haveClass(string className)",
        "string",
        "Returns true if the given C++ class exists"
);

cNEDValue nedf_moduleListByPath(cComponent *context, cNEDValue argv[], int argc)
{
    std::string modulenames;
    cTopology topo;
    std::vector<std::string> paths;
    for (int i = 0; i < argc; i++)
        paths.push_back(argv[i].stdstringValue());

    topo.extractByModulePath(paths);

    for (int i = 0; i < topo.getNumNodes(); i++)
    {
        std::string path = topo.getNode(i)->getModule()->getFullPath();
        if (modulenames.length() > 0)
            modulenames = modulenames + " " + path;
        else
            modulenames = path;
    }
    return modulenames;
}

Define_NED_Function2(nedf_moduleListByPath,
        "string moduleListByPath(string modulePath,...)",
        "string",
        "Returns a space-separated list of the modules at the given path(s). "
        "See cTopology::extractByModulePath()."
);

cNEDValue nedf_moduleListByNedType(cComponent *context, cNEDValue argv[], int argc)
{
    std::string modulenames;
    cTopology topo;
    std::vector<std::string> paths;
    for (int i = 0; i < argc; i++)
        paths.push_back(argv[i].stdstringValue());

    topo.extractByNedTypeName(paths);

    for (int i = 0; i < topo.getNumNodes(); i++)
    {
        std::string path = topo.getNode(i)->getModule()->getFullPath();
        if (modulenames.length() > 0)
            modulenames = modulenames + " " + path;
        else
            modulenames = path;
    }
    return modulenames;
}

Define_NED_Function2(nedf_moduleListByNedType,
        "string moduleListByNedType(string nedTypeName,...)",
        "string",
        "Returns a space-separated list of the modules with the given NED type(s). "
        "See cTopology::extractByNedTypeName()."
);

cNEDValue nedf_select(cComponent *context, cNEDValue argv[], int argc)
{
    long index = argv[0];
    if (index < 0)
        throw cRuntimeError("select(): negative index %ld", index);
    if (index >= argc-1)
        throw cRuntimeError("select(): index=%ld is too large", index, argc-1);
    return argv[index+1];
}

Define_NED_Function2(nedf_select,
        "any select(int index, ...)",
        "misc",
        "Returns the <index>th item from the rest of the argument list; numbering starts from 0."
);

