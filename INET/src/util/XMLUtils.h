#ifndef __INET_XMLUTILS_H
#define __INET_XMLUTILS_H

#include <omnetpp.h>
#include "IPAddress.h"

const cXMLElement* getUniqueChild(const cXMLElement *node, const char *name);
const cXMLElement* getUniqueChildIfExists(const cXMLElement *node, const char *name);

void checkTags(const cXMLElement *node, const char *allowed);

bool getParameterBoolValue(const cXMLElement *ptr, const char *name, bool def);
bool getParameterBoolValue(const cXMLElement *ptr, const char *name);
int getParameterIntValue(const cXMLElement *ptr, const char *name);
int getParameterIntValue(const cXMLElement *ptr, const char *name, int def);
const char* getParameterStrValue(const cXMLElement *ptr, const char *name);
const char* getParameterStrValue(const cXMLElement *ptr, const char *name, const char *def);
IPAddress getParameterIPAddressValue(const cXMLElement *ptr, const char *name);
IPAddress getParameterIPAddressValue(const cXMLElement *ptr, const char *name, IPAddress def);
double getParameterDoubleValue(const cXMLElement *ptr, const char *name);
double getParameterDoubleValue(const cXMLElement *ptr, const char *name, double def);

bool parseBool(const char *text);

#endif
