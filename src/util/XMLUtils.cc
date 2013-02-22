
#include "XMLUtils.h"
#include "IPvXAddressResolver.h"

const cXMLElement* getUniqueChild(const cXMLElement *node, const char *name)
{
    const cXMLElement *child = getUniqueChildIfExists(node, name);
    if (!child)
        throw cRuntimeError("XML error: exactly one %s element expected", name);

    return child;
}

const cXMLElement* getUniqueChildIfExists(const cXMLElement *node, const char *name)
{
    cXMLElementList list = node->getChildrenByTagName(name);
    if (list.size() > 1)
        throw cRuntimeError("XML error: at most one %s element expected", name);
    else if (list.size() == 1)
        return (*list.begin());
    else
        return NULL;
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
    while ((nt = st.nextToken())!=NULL)
        tags.push_back(nt);

    for (cXMLElement *child=node->getFirstChild(); child; child=child->getNextSibling())
    {
        unsigned int i;
        for (i = 0; i < tags.size(); i++)
            if (!strcmp(child->getTagName(), tags[i]))
                break;
        if (i == tags.size())
            throw cRuntimeError("Subtag <%s> not expected in <%s>",
                    child->getTagName(), node->getTagName());
    }
}

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
        return parseBool(xvalue->getNodeValue());
    else
        return def;
}

bool getParameterBoolValue(const cXMLElement *ptr, const char *name)
{
    const cXMLElement *xvalue = getUniqueChild(ptr, name);
    return parseBool(xvalue->getNodeValue());
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
        return atoi(xvalue->getNodeValue());
    else
        return def;
}

int getParameterIntValue(const cXMLElement *ptr, const char *name)
{
    const cXMLElement *xvalue = getUniqueChild(ptr, name);
    return atoi(xvalue->getNodeValue());
}

IPv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name, IPv4Address def)
{
    const cXMLElement *xvalue = getUniqueChildIfExists(ptr, name);
    if (xvalue)
        return IPvXAddressResolver().resolve(xvalue->getNodeValue()).get4();
    else
        return def;
}

IPv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name)
{
    const cXMLElement *xvalue = getUniqueChild(ptr, name);
    return IPvXAddressResolver().resolve(xvalue->getNodeValue()).get4();
}

double getParameterDoubleValue(const cXMLElement *ptr, const char *name, double def)
{
    const cXMLElement *xvalue = getUniqueChildIfExists(ptr, name);
    if (xvalue)
        return strtod(xvalue->getNodeValue(), NULL);
    else
        return def;
}

double getParameterDoubleValue(const cXMLElement *ptr, const char *name)
{
    const cXMLElement *xvalue = getUniqueChild(ptr, name);
    return strtod(xvalue->getNodeValue(), NULL);
}

const char *getRequiredAttribute(const cXMLElement& node, const char *attr)
{
    const char *s = node.getAttribute(attr);
    if (!(s && *s))
        throw cRuntimeError("required attribute %s of <%s> missing at %s",
                attr, node.getTagName(), node.getSourceLocation());
    return s;
}

