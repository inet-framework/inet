#ifndef __INET_XMLUTILS_H
#define __INET_XMLUTILS_H

#include "INETDefs.h"

#include "IPv4Address.h"

const cXMLElement* getUniqueChild(const cXMLElement *node, const char *name);
const cXMLElement* getUniqueChildIfExists(const cXMLElement *node, const char *name);

void checkTags(const cXMLElement *node, const char *allowed);

bool getParameterBoolValue(const cXMLElement *ptr, const char *name, bool def);
bool getParameterBoolValue(const cXMLElement *ptr, const char *name);
int getParameterIntValue(const cXMLElement *ptr, const char *name);
int getParameterIntValue(const cXMLElement *ptr, const char *name, int def);
const char* getParameterStrValue(const cXMLElement *ptr, const char *name);
const char* getParameterStrValue(const cXMLElement *ptr, const char *name, const char *def);
IPv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name);
IPv4Address getParameterIPAddressValue(const cXMLElement *ptr, const char *name, IPv4Address def);
double getParameterDoubleValue(const cXMLElement *ptr, const char *name);
double getParameterDoubleValue(const cXMLElement *ptr, const char *name, double def);

const char *getRequiredAttribute(const cXMLElement& node, const char *attr);

bool parseBool(const char *text);

#endif
