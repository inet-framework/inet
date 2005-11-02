
#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <omnetpp.h>
#include "IntServ.h"

/**
 * FIXME missing documentation
 */
void removeDuplicates(std::vector<int>& vec);

/**
 * FIXME missing documentation
 */
bool find(std::vector<int>& vec, int value);

/**
 * FIXME missing documentation
 */
bool find(const IPAddressVector& vec, IPAddress addr); // use TEMPLATE

/**
 * FIXME missing documentation
 */
void append(std::vector<int>& dest, const std::vector<int>& src);

/**
 * FIXME missing documentation
 */
int nodepos(const EroVector& ERO, IPAddress node);

/**
 * FIXME missing documentation
 */
cModule *payloadOwner(cMessage *msg);

//void prepend(EroVector& dest, const EroVector& src, bool reverse);


#endif
