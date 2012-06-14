//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#include "cnedvalue.h"

#include "InterfaceTableAccess.h"
#include "InterfaceEntry.h"
#include "opp_utils.h"

#ifdef WITH_IPv4
#include "IPv4Datagram.h"
#endif

#ifdef WITH_IPv6
#include "IPv6Datagram.h"
#endif


#include "DiffservUtil.h"
#include "DSCP_m.h"

using namespace OPP_Global;

namespace DiffservUtil
{

// cached enums
cEnum *dscpEnum = NULL;
cEnum *protocolEnum = NULL;

const char *getRequiredAttribute(cXMLElement *element, const char *attrName)
{
    const char *attrValue = element->getAttribute(attrName);
    if (!attrValue)
        throw cRuntimeError("missing attribute '%s' from <%s> element", attrName, element->getTagName());
    return attrValue;
}

double parseInformationRate(const char *attrValue, const char *attrName, cSimpleModule &owner, int defaultValue)
{
    if (isEmpty(attrValue))
        return defaultValue;

    const char *percentPtr = strchr(attrValue, '%');
    if (percentPtr)
    {
        char *e;
        double percent = strtod(attrValue, &e);
        if (e != percentPtr)
            throw cRuntimeError("malformed %s attribute: %s", attrName, attrValue);
        if (percent < 0.0 || percent > 100.0)
            throw cRuntimeError("%s must be between 0\% and 100\%, found: %s", attrName, attrValue);

        double datarate = getInterfaceDatarate(&owner);
        if (datarate < 0.0)
            throw cRuntimeError("cannot determine datarate for module %s, (no interface table in the node?)", owner.getFullPath().c_str());

        return (percent / 100.0) * datarate;
    }
    else
    {
        char *unit;
        double datarate = strtod(attrValue, &unit);
        return cNEDValue::convertUnit(datarate, unit, "bps");
    }
    return 0;
}

int parseIntAttribute(const char *attrValue, const char *attrName, bool isOptional)
{
    if (isEmpty(attrValue))
    {
        if (isOptional)
            return -1;
        else
            throw cRuntimeError("missing %s attribute", attrName);
    }

    unsigned long num;
    char *endp;
    if (*attrValue == '0' && *(attrValue+1) == 'b') // 0b prefix for binary
        num = strtoul(attrValue+2, &endp, 2);
    else
        num = strtoul(attrValue, &endp, 0); // will handle hex/octal/decimal

    if (*endp != '\0')
        throw cRuntimeError("malformed %s attribute: %s", attrName, attrValue);

    if (num > INT_MAX)
        throw cRuntimeError("attribute %s is too large: %s", attrName, attrValue);

    return (int)num;
}

int parseProtocol(const char *attrValue, const char *attrName)
{
    if (isEmpty(attrValue))
        return -1;
    if (isdigit(*attrValue))
        return parseIntAttribute(attrValue, attrName);
    if (!protocolEnum)
        protocolEnum = cEnum::get("IPProtocolId");
    char name[20];
    strcpy(name, "IP_PROT_");
    char *dest;
    for (dest = name+8; *attrValue; ++dest, ++attrValue)
        *dest = toupper(*attrValue);
    *dest = '\0';

    return protocolEnum->lookup(name);
}

int parseDSCP(const char *attrValue, const char *attrName)
{
    if (isEmpty(attrValue))
        throw cRuntimeError("missing %s attribute", attrName);
    if (isdigit(*attrValue))
    {
        int dscp = parseIntAttribute(attrValue, attrName);
        if (dscp < 0 || dscp >= DSCP_MAX)
            throw cRuntimeError("value of %s attribute is out of range [0,%d)", DSCP_MAX);
        return dscp;
    }
    if (!dscpEnum)
        dscpEnum = cEnum::get("DSCP");
    char name[20];
    strcpy(name, "DSCP_");
    const char *src;
    char *dest;
    for (src = attrValue, dest = name+5; *src; ++src, ++dest)
        *dest = toupper(*src);
    *dest = '\0';

    int dscp = dscpEnum->lookup(name);
    if (dscp < 0)
        throw cRuntimeError("malformed %s attribute", attrName);
    return dscp;
}

void parseDSCPs(const char *attrValue, const char *attrName, std::vector<int> &result)
{
    if (isEmpty(attrValue))
        return;
    if (*attrValue == '*' && *(attrValue+1) == '\0')
    {
        for (int dscp = 0; dscp < DSCP_MAX; ++dscp)
            result.push_back(dscp);
    }
    else
    {
        cStringTokenizer tokens(attrValue);
        while(tokens.hasMoreTokens())
            result.push_back(parseDSCP(tokens.nextToken(), attrName));
    }
}

std::string dscpToString(int dscp)
{
    if (!dscpEnum)
        dscpEnum = cEnum::get("DSCP");
    const char *name = dscpEnum->getStringFor(dscp);
    if (name)
    {
        if (!strncmp(name, "DSCP_", 5))
            name += 5;
        return name;
    }
    else
        return ltostr(dscp);
}

std::string colorToString(int color)
{
    switch(color)
    {
        case GREEN: return "green";
        case YELLOW: return "yellow";
        case RED: return "red";
        default: return ltostr(color);
    }
}

double getInterfaceDatarate(cSimpleModule *interfaceModule)
{
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists(interfaceModule);
    InterfaceEntry *ie = ift ? ift->getInterfaceByInterfaceModule(interfaceModule) : NULL;
    return ie ? ie->getDatarate() : -1;
}

cPacket *findIPDatagramInPacket(cPacket *packet)
{
    for (; packet; packet = packet->getEncapsulatedPacket())
    {
#ifdef WITH_IPv4
        if (dynamic_cast<IPv4Datagram*>(packet))
            return packet;
#endif
#ifdef WITH_IPv6
        if (dynamic_cast<IPv6Datagram *>(packet))
            return packet;
#endif
    }

    return NULL;
}

class ColorAttribute : public cObject
{
    public:
        int color;
    public:
        ColorAttribute(int color) : color(color) {}
        virtual const char *getName() const  {return "dscolor";}
        virtual std::string info() const { return colorToString(color); }
};

int getColor(cPacket *packet)
{
    ColorAttribute *attr = dynamic_cast<ColorAttribute*>(packet->getParList().get("dscolor"));
    return attr ? attr->color : -1;
}

void setColor(cPacket *packet, int color)
{
    ColorAttribute *attr = dynamic_cast<ColorAttribute*>(packet->getParList().get("dscolor"));
    if (attr)
        attr->color = color;
    else
        packet->addObject(new ColorAttribute(color));
}

}
