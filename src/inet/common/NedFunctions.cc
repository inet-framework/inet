//
// Copyright (C) 1992-2004 Andras Varga
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/common/INETDefs.h"

namespace inet {

namespace utils {

#if OMNETPP_VERSION < 0x0600 || OMNETPP_BUILDNUM < 1512
#define doubleValueRaw() doubleValue()
#endif

cNEDValue nedf_hasVisualizer(cComponent *context, cNEDValue argv[], int argc)
{
#ifdef WITH_VISUALIZERS
    return true;
#else
    return false;
#endif
}

Define_NED_Function2(nedf_hasVisualizer,
        "bool hasVisualizer()",
        "",
        "Returns true if the visualizer feature is available"
        );

cNEDValue nedf_hasModule(cComponent *context, cNEDValue argv[], int argc)
{
    cRegistrationList *types = OMNETPP6_CODE(omnetpp::internal::)componentTypes.getInstance();
    if (argv[0].getType() != cNEDValue::STRING)
        throw cRuntimeError("hasModule(): string arguments expected");
    const char *name = argv[0].stringValue();
    cComponentType *c;
    c = dynamic_cast<cComponentType *>(types->lookup(name)); // by qualified name
    if (c && c->isAvailable())
        return true;
    c = dynamic_cast<cComponentType *>(types->find(name)); // by simple name
    if (c && c->isAvailable())
        return true;
    return false;
}

Define_NED_Function2(nedf_hasModule,
        "bool hasModule(string nedTypeName)",
        "string",
        "Returns true if the given NED type exists"
        );

cNEDValue nedf_haveClass(cComponent *context, cNEDValue argv[], int argc)
{
    return OMNETPP6_CODE(omnetpp::internal::)classes.getInstance()->lookup(argv[0].stringValue()) != nullptr;
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

    for (int i = 0; i < topo.getNumNodes(); i++) {
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

    for (int i = 0; i < topo.getNumNodes(); i++) {
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
    if (index >= argc - 1)
        throw cRuntimeError("select(): index=%ld is too large, max value is %d", index, argc - 1);
    return argv[index + 1];
}

Define_NED_Function2(nedf_select,
        "any select(int index, ...)",
        "misc",
        "Returns the <index>th item from the rest of the argument list; numbering starts from 0."
        );

cNEDValue nedf_absPath(cComponent *context, cNEDValue argv[], int argc)
{
    if (argc != 1)
        throw cRuntimeError("absPath(): must be one argument instead of %d argument(s)", argc);
    const char *path = argv[0].stringValue();
    switch (*path) {
        case '.':
            return context->getFullPath() + path;

        case '^':
            return context->getFullPath() + '.' + path;

        default:
            return argv[0];
    }
}

Define_NED_Function2(nedf_absPath,
        "string absPath(string modulePath)",
        "string",
        "Returns absolute path of given module"
        );

cNEDValue nedf_firstAvailableOrEmpty(cComponent *context, cNEDValue argv[], int argc)
{
    cRegistrationList *types = OMNETPP6_CODE(omnetpp::internal::)componentTypes.getInstance();
    for (int i=0; i<argc; i++)
    {
        if (argv[i].getType() != cNEDValue::STRING)
            throw cRuntimeError("firstAvailable(): string arguments expected");
        const char *name = argv[i].stringValue();
        cComponentType *c;
        c = dynamic_cast<cComponentType *>(types->lookup(name)); // by qualified name
        if (c && c->isAvailable())
            return argv[i];
        c = dynamic_cast<cComponentType *>(types->find(name)); // by simple name
        if (c && c->isAvailable())
            return argv[i];
    }
    return "";
}

Define_NED_Function2(nedf_firstAvailableOrEmpty,
    "string firstAvailableOrEmpty(...)",
    "misc",
    "Accepts any number of strings, interprets them as NED type names "
    "(qualified or unqualified), and returns the first one that exists and "
    "its C++ implementation class is also available. Returns empty string if "
    "none of the types are available.");

cNEDValue nedf_nanToZero(cComponent *context, cNEDValue argv[], int argc)
{
    double x = argv[0].doubleValueRaw();
    const char *unit = argv[0].getUnit();
    return std::isnan(x) ? cNEDValue(0.0, unit) : argv[0];
}

Define_NED_Function2(nedf_nanToZero,
        "quantity nanToZero(quantity x)",
        "math",
        "Returns the argument if it is not NaN, otherwise returns 0."
        );

static cNedValue nedf_intWithUnit(cComponent *contextComponent, cNedValue argv[], int argc)
{
    switch (argv[0].getType()) {
        case cNedValue::BOOL:
            return (OMNETPP5_CODE(intpar_t) OMNETPP6_CODE(intval_t))( argv[0].boolValue() ? 1 : 0 );
        case cNedValue::INT:
            return argv[0];
        case cNedValue::DOUBLE:
            return cNedValue(checked_int_cast<OMNETPP5_CODE(intpar_t) OMNETPP6_CODE(intval_t)>(floor(argv[0].doubleValueRaw())), argv[0].getUnit());
        case cNedValue::STRING:
            throw cRuntimeError("intWithUnit(): Cannot convert string to int");
        case OMNETPP5_CODE(cNedValue::XML) OMNETPP6_CODE(cNedValue::OBJECT) :
            throw cRuntimeError("intWithUnit(): Cannot convert cObject to int");
        default:
            throw cRuntimeError("Internal error: Invalid cNedValue type");
    }
}

Define_NED_Function2(nedf_intWithUnit,
    "intquantity intWithUnit(any x)",
    "conversion",
    "Converts x to an integer (C++ long), and returns the result. A boolean argument becomes 0 or 1; a double is converted using floor(); a string or an XML argument causes an error.");

cNedValue nedf_xmlattr(cComponent *contextComponent, cNedValue argv[], int argc)
{
    if (argv[0].getType() != OMNETPP5_CODE(cNedValue::XML) OMNETPP6_CODE(cNedValue::OBJECT))
        throw cRuntimeError("xmlattr(): xmlNode argument must be an xml node");
    if (argv[1].getType() != cNEDValue::STRING)
        throw cRuntimeError("xmlattr(): attributeName argument must be a string");

    cXMLElement *node = argv[0].xmlValue();
    const char *attr = node->getAttribute(argv[1].stdstringValue().c_str());
    if (attr != nullptr)
        return cNedValue(attr);
    if (argc < 3)
        throw cRuntimeError("Attribute '%s' not found in xml '%s'", argv[1].stdstringValue().c_str(), argv[0].stdstringValue().c_str());
    return argv[2];
}

Define_NED_Function2(nedf_xmlattr,
        "string xmlattr(xml xmlNode, string attributeName, string defaultValue?)",
        "xml",
        "Returns the value of the specified XML attribute of xmlNode. "
        "It returns the defaultValue (or throws an error) if the attribute does not exists."
        )

} // namespace utils

} // namespace inet

