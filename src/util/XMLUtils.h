#ifndef __INET_XMLUTILS_H
#define __INET_XMLUTILS_H

#include "INETDefs.h"

#include "IPv4Address.h"
#include "IPv6Address.h"

const cXMLElement* getUniqueChild(const cXMLElement *node, const char *name);
const cXMLElement* getUniqueChildIfExists(const cXMLElement *node, const char *name);

void checkTags(const cXMLElement *node, const char *allowed);

bool getElementBoolValue(const cXMLElement *ptr);
int getElementIntValue(const cXMLElement *ptr);
IPv4Address getElementIPv4AddressValue(const cXMLElement *ptr);
// if prefix length pointer is given, address will be parsed directly and not sent to IPvXResolver!
IPv6Address getElementIPv6AddressValue(const cXMLElement *ptr, int* prefixLen = 0);
double getElementDoubleValue(const cXMLElement *ptr);

bool getParameterBoolValue(const cXMLElement *ptr, const char *name, bool def);
bool getParameterBoolValue(const cXMLElement *ptr, const char *name);
int getParameterIntValue(const cXMLElement *ptr, const char *name);
int getParameterIntValue(const cXMLElement *ptr, const char *name, int def);
const char* getParameterStrValue(const cXMLElement *ptr, const char *name);
const char* getParameterStrValue(const cXMLElement *ptr, const char *name, const char *def);
IPv4Address getParameterIPv4AddressValue(const cXMLElement *ptr, const char *name);
IPv4Address getParameterIPv4AddressValue(const cXMLElement *ptr, const char *name, IPv4Address def);
// if prefix length pointer is given, address will be parsed directly and not sent to IPvXResolver!
IPv6Address getParameterIPv6AddressValue(const cXMLElement *ptr, const char *name, int* prefixLen = 0);
IPv6Address getParameterIPv6AddressValue(const cXMLElement *ptr, const char *name, IPv6Address def);
double getParameterDoubleValue(const cXMLElement *ptr, const char *name);
double getParameterDoubleValue(const cXMLElement *ptr, const char *name, double def);

bool getAttributeBoolValue(const cXMLElement *ptr, const char *name, bool def);
bool getAttributeBoolValue(const cXMLElement *ptr, const char *name);
int getAttributeIntValue(const cXMLElement *ptr, const char *name);
int getAttributeIntValue(const cXMLElement *ptr, const char *name, int def);
const char* getAttributeStrValue(const cXMLElement *ptr, const char *name, const char *def);
IPv4Address getAttributeIPv4AddressValue(const cXMLElement *ptr, const char *name);
IPv4Address getAttributeIPv4AddressValue(const cXMLElement *ptr, const char *name, IPv4Address def);
// if prefix length pointer is given, address will be parsed directly and not sent to IPvXResolver!
IPv6Address getAttributeIPv6AddressValue(const cXMLElement *ptr, const char *name, int* prefixLen = 0);
IPv6Address getAttributeIPv6AddressValue(const cXMLElement *ptr, const char *name, IPv6Address def);
double getAttributeDoubleValue(const cXMLElement *ptr, const char *name);
double getAttributeDoubleValue(const cXMLElement *ptr, const char *name, double def);

const char *getRequiredAttribute(const cXMLElement& node, const char *attr);
bool getAttributeBoolValue(const cXMLElement *node, const char *attrName, bool defVal);
bool getAttributeBoolValue(const cXMLElement *node, const char *attrName);

bool parseBool(const char *text);

#endif
