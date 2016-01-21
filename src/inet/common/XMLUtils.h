#ifndef __INET_XMLUTILS_H
#define __INET_XMLUTILS_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv4/IPv4Address.h"

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
INET_API IPv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name);
INET_API IPv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name, IPv4Address def);
INET_API double getParameterDoubleValue(const cXMLElement *ptr, const char *name);
INET_API double getParameterDoubleValue(const cXMLElement *ptr, const char *name, double def);

INET_API const char *getRequiredAttribute(const cXMLElement& node, const char *attr);
INET_API bool getAttributeBoolValue(const cXMLElement *node, const char *attrName, bool defVal);
INET_API bool getAttributeBoolValue(const cXMLElement *node, const char *attrName);

INET_API bool parseBool(const char *text);

} // namespace xmlutils

} // namespace inet

#endif // ifndef __INET_XMLUTILS_H

