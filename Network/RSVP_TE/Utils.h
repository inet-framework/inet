
#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <omnetpp.h>
#include "IntServ.h"

void removeDuplicates(std::vector<int>& vec);
bool find(std::vector<int>& vec, int value);
bool find(const IPAddressVector& vec, IPAddress addr); // use TEMPLATE
void append(std::vector<int>& dest, const std::vector<int>& src);
int nodepos(const EroVector& ERO, IPAddress node);

cModule *payloadOwner(cMessage *msg);

//void prepend(EroVector& dest, const EroVector& src, bool reverse);


#endif
