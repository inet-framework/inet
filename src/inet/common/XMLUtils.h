#ifndef __INET_XMLUTILS_H
#define __INET_XMLUTILS_H

#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

namespace xmlutils {

INET_API const cXMLElement *getUniqueChild(const cXMLElement *node, const char *name);
INET_API const cXMLElement *getUniqueChildIfExists(const cXMLElement *node, const char *name);

INET_API void checkTags(const cXMLElement *node, const char *allowed);

INET_API bool getParameterBoolValue(const cXMLElement *ptr, const char *name, bool def);
INET_API bool getParameterBoolValue(const cXMLElement *ptr, const char *name);
INET_API int getParameterIntValue(const cXMLElement *ptr, const char *name);
INET_API int getParameterIntValue(const cXMLElement *ptr, const char *name, int def);
INET_API const char *getParameterStrValue(const cXMLElement *ptr, const char *name);
INET_API const char *getParameterStrValue(const cXMLElement *ptr, const char *name, const char *def);
INET_API Ipv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name);
INET_API Ipv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name, Ipv4Address def);
INET_API double getParameterDoubleValue(const cXMLElement *ptr, const char *name);
INET_API double getParameterDoubleValue(const cXMLElement *ptr, const char *name, double def);

INET_API const char *getMandatoryAttribute(const cXMLElement& node, const char *attr);
INET_API const char *getMandatoryFilledAttribute(const cXMLElement& node, const char *attr);
INET_API bool getAttributeBoolValue(const cXMLElement *node, const char *attrName, bool defVal);
INET_API bool getAttributeBoolValue(const cXMLElement *node, const char *attrName);
INET_API double getAttributeDoubleValue(const cXMLElement *node, const char *attrName, double defVal);
INET_API double getAttributeDoubleValue(const cXMLElement *node, const char *attrName);

INET_API bool parseBool(const char *text);

} // namespace xmlutils

} // namespace inet

#endif

