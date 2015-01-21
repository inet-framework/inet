#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include <cstdlib>

namespace inet {

namespace xmlutils {

const cXMLElement *getUniqueChild(const cXMLElement *node, const char *name)
{
    const cXMLElement *child = getUniqueChildIfExists(node, name);
    if (!child)
        throw cRuntimeError("XML error: exactly one %s element expected", name);

    return child;
}

const cXMLElement *getUniqueChildIfExists(const cXMLElement *node, const char *name)
{
    cXMLElementList list = node->getChildrenByTagName(name);
    if (list.size() > 1)
        throw cRuntimeError("XML error: at most one %s element expected", name);
    else if (list.size() == 1)
        return *list.begin();
    else
        return nullptr;
}

bool parseBool(const char *text)
{
    if (!strcasecmp(text, "down"))
        return false;
    else if (!strcasecmp(text, "off"))
        return false;
    else if (!strcasecmp(text, "false"))
        return false;
    else if (!strcasecmp(text, "no"))
        return false;
    else if (!strcasecmp(text, "0"))
        return false;
    else if (!strcasecmp(text, "up"))
        return true;
    else if (!strcasecmp(text, "on"))
        return true;
    else if (!strcasecmp(text, "true"))
        return true;
    else if (!strcasecmp(text, "yes"))
        return true;
    else if (!strcasecmp(text, "1"))
        return true;
    else
        throw cRuntimeError("Unknown bool constant: %s", text);
}

void checkTags(const cXMLElement *node, const char *allowed)
{
    std::vector<const char *> tags;

    cStringTokenizer st(allowed, " ");
    const char *nt;
    while ((nt = st.nextToken()) != nullptr)
        tags.push_back(nt);

    for (cXMLElement *child = node->getFirstChild(); child; child = child->getNextSibling()) {
        unsigned int i;
        for (i = 0; i < tags.size(); i++)
            if (!strcmp(child->getTagName(), tags[i]))
                break;

        if (i == tags.size())
            throw cRuntimeError("Subtag <%s> not expected in <%s>",
                    child->getTagName(), node->getTagName());
    }
}

// Elements

bool getElementBoolValue(const cXMLElement *ptr)
{
    try
    {
        return parseBool(ptr->getNodeValue());
    }
    catch (cRuntimeError& e)
    {
        throw cRuntimeError("XML error: value '%s' of <%s> at %s cannot be interpreted as boolean (%s)",
                ptr->getNodeValue(), ptr->getTagName(), ptr->getSourceLocation(), e.what());
    }
}

int getElementIntValue(const cXMLElement *ptr)
{
    char *e;
    long v = strtol(ptr->getNodeValue(), &e, 10);
    if (*e)
        throw cRuntimeError("XML error: value '%s' of <%s> at %s cannot be interpreted as an integer",
                ptr->getNodeValue(), ptr->getTagName(), ptr->getSourceLocation());

    return v;
}

IPv4Address getElementIPv4AddressValue(const cXMLElement *ptr)
{
    try
    {
        return L3AddressResolver().resolve(ptr->getNodeValue()).toIPv4();
    }
    catch(cRuntimeError& e)
    {
        throw cRuntimeError("XML error: value '%s' of <%s> at %s cannot be interpreted as an IPv4 address (%s)",
                ptr->getNodeValue(), ptr->getTagName(), ptr->getSourceLocation(), e.what());
    }
}

IPv6Address getElementIPv6AddressValue(const cXMLElement *ptr, int *prefixLen)
{
    if (prefixLen)
    {
        IPv6Address addr;
        if (!addr.tryParseAddrWithPrefix(ptr->getNodeValue(), *prefixLen))
            throw cRuntimeError("XML error: value '%s' of <%s> at %s is not a valid IPv6Address/prefix value",
                    ptr->getNodeValue(), ptr->getTagName(), ptr->getSourceLocation());
        return addr;
    }
    else
    {
        try
        {
            return L3AddressResolver().resolve(ptr->getNodeValue()).toIPv6();
        }
        catch(cRuntimeError& e)
        {
            throw cRuntimeError("XML error: value '%s' of <%s> at %s cannot be interpreted as an IPv6 address (%s)",
                    ptr->getNodeValue(), ptr->getTagName(), ptr->getSourceLocation(), e.what());
        }
    }
}

double getElementDoubleValue(const cXMLElement *ptr)
{
    char *e;
    double v = strtod(ptr->getNodeValue(), &e);
    if (*e)
        throw cRuntimeError("XML error: value '%s' of <%s> at %s cannot be interpreted as double",
                ptr->getNodeValue(), ptr->getTagName(), ptr->getSourceLocation());

    return v;
}


// Parameters

const char* getParameterStrValue(const cXMLElement *ptr, const char *name, const char *def)
{
    const cXMLElement *xvalue = getUniqueChildIfExists(ptr, name);
    if (xvalue)
        return xvalue->getNodeValue();
    else
        return def;
}

bool getParameterBoolValue(const cXMLElement *ptr, const char *name, bool def)
{
    const cXMLElement *xvalue = getUniqueChildIfExists(ptr, name);
    if (xvalue)
        return getElementBoolValue(ptr);
    else
        return def;
}

bool getParameterBoolValue(const cXMLElement *ptr, const char *name)
{
    return getElementBoolValue(getUniqueChild(ptr, name));
}

const char* getParameterStrValue(const cXMLElement *ptr, const char *name)
{
    const cXMLElement *xvalue = getUniqueChild(ptr, name);
    return xvalue->getNodeValue();
}

int getParameterIntValue(const cXMLElement *ptr, const char *name, int def)
{
    const cXMLElement *xvalue = getUniqueChildIfExists(ptr, name);
    if (xvalue)
        return getElementIntValue(xvalue);
    else
        return def;
}

int getParameterIntValue(const cXMLElement *ptr, const char *name)
{
    return getElementIntValue(getUniqueChild(ptr, name));
}

IPv4Address getParameterIPv4AddressValue(const cXMLElement *ptr, const char *name, IPv4Address def)
{
    const cXMLElement *xvalue = getUniqueChildIfExists(ptr, name);
    if (xvalue)
        return getElementIPv4AddressValue(xvalue);
    else
        return def;
}

IPv4Address getParameterIPv4AddressValue(const cXMLElement *ptr, const char *name)
{
    return getElementIPv4AddressValue(getUniqueChild(ptr, name));
}

IPv6Address getParameterIPv6AddressValue(const cXMLElement *ptr, const char *name, IPv6Address def)
{
    const cXMLElement *xvalue = getUniqueChildIfExists(ptr, name);
    if (xvalue)
        return getElementIPv6AddressValue(xvalue);
    else
        return def;
}

IPv6Address getParameterIPv6AddressValue(const cXMLElement *ptr, const char *name, int *prefixLen)
{
    return getElementIPv6AddressValue(getUniqueChild(ptr, name), prefixLen);
}

double getParameterDoubleValue(const cXMLElement *ptr, const char *name, double def)
{
    const cXMLElement *xvalue = getUniqueChildIfExists(ptr, name);
    if (xvalue)
        return getElementDoubleValue(xvalue);
    else
        return def;
}

double getParameterDoubleValue(const cXMLElement *ptr, const char *name)
{
    return getElementDoubleValue(getUniqueChild(ptr, name));
}


// Attributes

const char *getRequiredAttribute(const cXMLElement& node, const char *attr)
{
    const char *s = node.getAttribute(attr);
    if (!(s && *s))
        throw cRuntimeError("XML error: required attribute %s of <%s> missing at %s",
                attr, node.getTagName(), node.getSourceLocation());
    return s;
}

const char* getAttributeStrValue(const cXMLElement *ptr, const char *name, const char *def)
{
    const char *value = ptr->getAttribute(name);
    if (value)
        return value;
    else
        return def;
}

bool getAttributeBoolValue(const cXMLElement *ptr, const char *name)
{
    const char *value = getRequiredAttribute(*ptr, name);
    try
    {
        return parseBool(value);
    }
    catch (cRuntimeError& e)
    {
        throw cRuntimeError("XML error: value '%s' of <%s> attribute %s at %s cannot be interpreted as boolean (%s)",
                value, ptr->getTagName(), name, ptr->getSourceLocation(), e.what());
    }
}

bool getAttributeBoolValue(const cXMLElement *ptr, const char *name, bool def)
{
    if (ptr->getAttribute(name))
        return getAttributeBoolValue(ptr, name);
    else
        return def;
}

int getAttributeIntValue(const cXMLElement *ptr, const char *name, int def)
{
    if (ptr->getAttribute(name))
        return getAttributeIntValue(ptr, name);
    else
        return def;
}

int getAttributeIntValue(const cXMLElement *ptr, const char *name)
{
    const char *value = getRequiredAttribute(*ptr, name);

    char *e;
    long v = strtol(value, &e, 10);
    if (*e)
        throw cRuntimeError("XML error: value '%s' of <%s> attribute %s at %s cannot be interpreted as an integer",
                value, ptr->getTagName(), name, ptr->getSourceLocation());

    return v;
}

IPv4Address getAttributeIPv4AddressValue(const cXMLElement *ptr, const char *name, IPv4Address def)
{
    if (ptr->getAttribute(name))
        return getAttributeIPv4AddressValue(ptr, name);
    else
        return def;
}

IPv4Address getAttributeIPv4AddressValue(const cXMLElement *ptr, const char *name)
{
    const char *value = getRequiredAttribute(*ptr, name);
    try
    {
        return L3AddressResolver().resolve(value).toIPv4();
    }
    catch(cRuntimeError& e)
    {
        throw cRuntimeError("XML error: value '%s' of <%s> attriubte %s at %s cannot be interpreted as an IPv4 address (%s)",
                value, ptr->getTagName(), name, ptr->getSourceLocation(), e.what());
    }
}

IPv6Address getAttributeIPv6AddressValue(const cXMLElement *ptr, const char *name, IPv6Address def)
{
    if (ptr->getAttribute(name))
        return getAttributeIPv6AddressValue(ptr, name, 0);
    else
        return def;
}

IPv6Address getAttributeIPv6AddressValue(const cXMLElement *ptr, const char *name, int *prefixLen)
{
    const char *value = getRequiredAttribute(*ptr, name);
    if (prefixLen)
    {
        IPv6Address addr;
        if (!addr.tryParseAddrWithPrefix(value, *prefixLen))
            throw cRuntimeError("XML error: value '%s' of <%s> attribute %s at %s is not a valid IPv6Address/prefix value",
                    value, ptr->getTagName(), name, ptr->getSourceLocation());
        return addr;
    }
    else
    {
        try
        {
            return L3AddressResolver().resolve(value).toIPv6();
        }
        catch(cRuntimeError& e)
        {
            throw cRuntimeError("XML error: value '%s' of <%s> attribute %s at %s cannot be interpreted as an IPv6 address (%s)",
                    value, ptr->getTagName(), name, ptr->getSourceLocation(), e.what());
        }
    }
}


double getAttributeDoubleValue(const cXMLElement *ptr, const char *name, double def)
{
    if (ptr->getAttribute(name))
        return getAttributeDoubleValue(ptr, name);
    else
        return def;
}

double getAttributeDoubleValue(const cXMLElement *ptr, const char *name)
{
    const char *value = getRequiredAttribute(*ptr, name);
    char *e;
    double v = strtod(value, &e);
    if (*e)
        throw cRuntimeError("XML error: value '%s' of <%s> attribute %s at %s cannot be interpreted as double",
                value, ptr->getTagName(), name, ptr->getSourceLocation());

    return v;
}

} // namespace xmlutils

} // namespace inet

