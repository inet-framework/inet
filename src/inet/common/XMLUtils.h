#ifndef __INET_XMLUTILS_H
#define __INET_XMLUTILS_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {

namespace xmlutils {

const cXMLElement *getUniqueChild(const cXMLElement *node, const char *name);
const cXMLElement *getUniqueChildIfExists(const cXMLElement *node, const char *name);

void checkTags(const cXMLElement *node, const char *allowed);

bool getParameterBoolValue(const cXMLElement *ptr, const char *name, bool def);
bool getParameterBoolValue(const cXMLElement *ptr, const char *name);
int getParameterIntValue(const cXMLElement *ptr, const char *name);
int getParameterIntValue(const cXMLElement *ptr, const char *name, int def);
const char *getParameterStrValue(const cXMLElement *ptr, const char *name);
const char *getParameterStrValue(const cXMLElement *ptr, const char *name, const char *def);
IPv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name);
IPv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name, IPv4Address def);
double getParameterDoubleValue(const cXMLElement *ptr, const char *name);
double getParameterDoubleValue(const cXMLElement *ptr, const char *name, double def);

const char *getRequiredAttribute(const cXMLElement& node, const char *attr);
bool getAttributeBoolValue(const cXMLElement *node, const char *attrName, bool defVal);
bool getAttributeBoolValue(const cXMLElement *node, const char *attrName);

bool parseBool(const char *text);

} // namespace xmlutils

} // namespace inet

#endif // ifndef __INET_XMLUTILS_H

