//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIFFSERVUTIL_H
#define __INET_DIFFSERVUTIL_H

#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {
namespace DiffservUtil {

// colors for naming the output of meters
enum Color { GREEN, YELLOW, RED };

/**
 * Parses the information rate parameter (bits/sec).
 * Supported formats:
 *  - absolute (e.g. 10Mbps)
 *  - relative to the datarate of the interface (e.g. 10%)
 */
double parseInformationRate(const char *attrValue, const char *attrName, IInterfaceTable *ift, cSimpleModule& owner, int defaultValue);

/**
 * Parses an integer attribute.
 * Supports decimal, octal ("0" prefix), hexadecimal ("0x" prefix), and binary ("0b" prefix) bases.
 */
int parseIntAttribute(const char *attrValue, const char *attrName, bool isOptional = true);

/**
 * Parses an IP protocol number.
 * Recognizes the names defined in IpProtocolId.msg (e.g. "Udp", "udp", "Tcp"),
 * and accepts decimal/octal/hex/binary numbers.
 */
int parseProtocol(const char *attrValue, const char *attrName);

/**
 * Parses a Diffserv code point.
 * Recognizes the names defined in Dscp.msg (e.g. "BE", "AF11"),
 * and accepts decimal/octal/hex/binary numbers.
 */
int parseDSCP(const char *attrValue, const char *attrName);

/**
 * Parses a space separated list of DSCP values and puts them into the result vector.
 * "*" is interpreted as all possible DSCP values (i.e. the 0..63 range).
 */
void parseDSCPs(const char *attrValue, const char *attrName, std::vector<int>& result);

/**
 * Returns the string representation of the given DSCP value.
 * Values defined in DSCP.msg are returned as "BE", "AF11", etc.,
 * others are returned as a decimal number.
 */
std::string dscpToString(int dscp);

/**
 * Returns the string representation of the given color.
 * For values defined in IMeter.h it returns their name,
 * other values are returned as decimal constants.
 */
std::string colorToString(int color);

/**
 * Returns the datarate of the interface containing the given module.
 * Returns -1, if the interface entry not found.
 */
double getInterfaceDatarate(IInterfaceTable *ift, cSimpleModule *interfaceModule);

/**
 * Returns the color of the packet.
 * The color was set by a previous meter component.
 * Returns -1, if the color was not set.
 */
int getColor(cPacket *packet);

/**
 * Sets the color of the packet.
 * The color is stored in the parlist of the cPacket object.
 */
void setColor(cPacket *packet, int color);

} // namespace DiffservUtil

} // namespace inet

#endif

