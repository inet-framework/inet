//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

//  Cleanup and rewrite: Andras Varga, 2004

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/RoutingTableParser.h"

namespace inet {

//
// Constants
//
const int MAX_FILESIZE = 10000;    // TBD lift hardcoded limit
const int MAX_ENTRY_STRING_SIZE = 500;

//
// Tokens that mark the route file.
//
const char *IFCONFIG_START_TOKEN = "ifconfig:",
*IFCONFIG_END_TOKEN = "ifconfigend.",
*ROUTE_START_TOKEN = "route:",
*ROUTE_END_TOKEN = "routeend.";

int RoutingTableParser::streq(const char *str1, const char *str2)
{
    return strncmp(str1, str2, strlen(str2)) == 0;
}

int RoutingTableParser::strcpyword(char *dest, const char *src)
{
    int i;
    for (i = 0; !isspace(dest[i] = src[i]); i++)
        ;
    dest[i] = '\0';
    return i;
}

void RoutingTableParser::skipBlanks(char *str, int& charptr)
{
    for ( ; isspace(str[charptr]); charptr++)
        ;
}

int RoutingTableParser::readRoutingTableFromFile(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == nullptr)
        throw cRuntimeError("Error opening routing table file `%s'", filename);

    int charpointer;
    char *file = new char[MAX_FILESIZE];
    char *ifconfigFile = nullptr;
    char *routeFile = nullptr;
    int c;

    // read the whole into the file[] char-array
    for (charpointer = 0; (c = getc(fp)) != EOF; charpointer++)
        file[charpointer] = c;

    for ( ; charpointer < MAX_FILESIZE; charpointer++)
        file[charpointer] = '\0';

    fclose(fp);

    // copy file into specialized, filtered char arrays
    for (charpointer = 0;
         (charpointer < MAX_FILESIZE) && (file[charpointer] != '\0');
         charpointer++)
    {
        // check for tokens at beginning of file or line
        if (charpointer == 0 || file[charpointer - 1] == '\n') {
            // copy into ifconfig filtered chararray
            if (streq(file + charpointer, IFCONFIG_START_TOKEN)) {
                ifconfigFile = createFilteredFile(file,
                            charpointer,
                            IFCONFIG_END_TOKEN);
                //PRINTF("Filtered File 1 created:\n%s\n", ifconfigFile);
            }

            // copy into route filtered chararray
            if (streq(file + charpointer, ROUTE_START_TOKEN)) {
                routeFile = createFilteredFile(file,
                            charpointer,
                            ROUTE_END_TOKEN);
                //PRINTF("Filtered File 2 created:\n%s\n", routeFile);
            }
        }
    }

    delete[] file;

    // parse filtered files
    if (ifconfigFile)
        parseInterfaces(ifconfigFile);
    if (routeFile)
        parseRouting(routeFile);

    delete[] ifconfigFile;
    delete[] routeFile;

    return 0;
}

char *RoutingTableParser::createFilteredFile(char *file, int& charpointer, const char *endtoken)
{
    int i = 0;
    char *filterFile = new char[MAX_FILESIZE];
    filterFile[0] = '\0';

    while (true) {
        // skip blank lines and comments
        while (!isalnum(file[charpointer]) && !isspace(file[charpointer])) {
            while (file[charpointer++] != '\n')
                ;
        }

        // check for endtoken:
        if (streq(file + charpointer, endtoken)) {
            filterFile[i] = '\0';
            break;
        }

        // copy whole line to filterFile
        while ((filterFile[i++] = file[charpointer++]) != '\n')
            ;
    }

    return filterFile;
}

void RoutingTableParser::parseInterfaces(char *ifconfigFile)
{
    char buf[MAX_ENTRY_STRING_SIZE];
    int charpointer = 0;
    InterfaceEntry *ie = nullptr;

    // parsing of entries in interface definition
    while (ifconfigFile[charpointer] != '\0') {
        // name entry
        if (streq(ifconfigFile + charpointer, "name:")) {
            // find existing interface with this name
            char *name = parseEntry(ifconfigFile, "name:", charpointer, buf);
            ie = ift->findInterfaceByName(name);
            if (!ie)
                throw cRuntimeError("Error in routing file: interface name `%s' not registered by any L2 module", name);
            if (!ie->findProtocolData<Ipv4InterfaceData>())
                throw cRuntimeError("Error in routing file: interface name `%s' doesn't have Ipv4 data fields", name);

            continue;
        }

        // encap entry
        if (streq(ifconfigFile + charpointer, "encap:")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            // ignore encap
            parseEntry(ifconfigFile, "encap:", charpointer, buf);
            continue;
        }

        // HWaddr entry
        if (streq(ifconfigFile + charpointer, "HWaddr:")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            // ignore hwAddr
            parseEntry(ifconfigFile, "HWaddr:", charpointer, buf);
            continue;
        }

        // inet_addr entry
        if (streq(ifconfigFile + charpointer, "inet_addr:")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            ie->getProtocolDataForUpdate<Ipv4InterfaceData>()->setIPAddress(Ipv4Address(parseEntry(ifconfigFile, "inet_addr:", charpointer, buf)));
            continue;
        }

        // Broadcast address entry
        if (streq(ifconfigFile + charpointer, "Bcast:")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            // ignore Bcast
            parseEntry(ifconfigFile, "Bcast:", charpointer, buf);
            continue;
        }

        // Mask entry
        if (streq(ifconfigFile + charpointer, "Mask:")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            ie->getProtocolDataForUpdate<Ipv4InterfaceData>()->setNetmask(Ipv4Address(parseEntry(ifconfigFile, "Mask:", charpointer, buf)));
            continue;
        }

        // Multicast groups entry
        if (streq(ifconfigFile + charpointer, "Groups:")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            char *grStr = parseEntry(ifconfigFile, "Groups:", charpointer, buf);
            parseMulticastGroups(grStr, ie);
            continue;
        }

        // MTU entry
        if (streq(ifconfigFile + charpointer, "MTU:")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            ie->setMtu(atoi(parseEntry(ifconfigFile, "MTU:", charpointer, buf)));
            continue;
        }

        // Metric entry
        if (streq(ifconfigFile + charpointer, "Metric:")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            ie->getProtocolDataForUpdate<Ipv4InterfaceData>()->setMetric(atoi(parseEntry(ifconfigFile, "Metric:", charpointer, buf)));
            continue;
        }

        // BROADCAST Flag
        if (streq(ifconfigFile + charpointer, "BROADCAST")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            ie->setBroadcast(true);
            charpointer += strlen("BROADCAST");
            skipBlanks(ifconfigFile, charpointer);
            continue;
        }

        // MULTICAST Flag
        if (streq(ifconfigFile + charpointer, "MULTICAST")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            ie->setMulticast(true);
            charpointer += strlen("MULTICAST");
            skipBlanks(ifconfigFile, charpointer);
            continue;
        }

        // POINTTOPOINT Flag
        if (streq(ifconfigFile + charpointer, "POINTTOPOINT")) {
            if (!ie)
                throw cRuntimeError("Error in routing file: missing the `name:' entry");
            ie->setPointToPoint(true);
            charpointer += strlen("POINTTOPOINT");
            skipBlanks(ifconfigFile, charpointer);
            continue;
        }

        // no entry discovered: move charpointer on
        charpointer++;
    }
}

char *RoutingTableParser::parseEntry(char *ifconfigFile, const char *tokenStr,
        int& charpointer, char *destStr)
{
    int temp = 0;

    charpointer += strlen(tokenStr);
    skipBlanks(ifconfigFile, charpointer);
    temp = strcpyword(destStr, ifconfigFile + charpointer);
    charpointer += temp;

    skipBlanks(ifconfigFile, charpointer);

    return destStr;
}

void RoutingTableParser::parseMulticastGroups(char *groupStr, InterfaceEntry *itf)
{
    // Parse string (Ipv4 addresses separated by colons)
    cStringTokenizer tokenizer(groupStr, ":");
    const char *token;
    while ((token = tokenizer.nextToken()) != nullptr)
        itf->getProtocolDataForUpdate<Ipv4InterfaceData>()->joinMulticastGroup(Ipv4Address(token));
}

void RoutingTableParser::parseRouting(char *routeFile)
{
    char *str = new char[MAX_ENTRY_STRING_SIZE];

    int pos = strlen(ROUTE_START_TOKEN);
    skipBlanks(routeFile, pos);
    while (routeFile[pos] != '\0') {
        // 1st entry: Host
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        Ipv4Route *e = new Ipv4Route();
        if (strcmp(str, "default:")) {
            // if entry is not the default entry
            if (!Ipv4Address::isWellFormed(str))
                throw cRuntimeError("Syntax error in routing file: `%s' on 1st column should be `default:' or a valid Ipv4 address", str);

            e->setDestination(Ipv4Address(str));
        }

        // 2nd entry: Gateway
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        if (!strcmp(str, "*") || !strcmp(str, "0.0.0.0")) {
            e->setGateway(Ipv4Address::UNSPECIFIED_ADDRESS);
        }
        else {
            if (!Ipv4Address::isWellFormed(str))
                throw cRuntimeError("Syntax error in routing file: `%s' on 2nd column should be `*' or a valid Ipv4 address", str);

            e->setGateway(Ipv4Address(str));
        }

        // 3rd entry: Netmask
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        if (!Ipv4Address::isWellFormed(str))
            throw cRuntimeError("Syntax error in routing file: `%s' on 3rd column should be a valid Ipv4 address", str);

        e->setNetmask(Ipv4Address(str));

        // 4th entry: flags
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        // parse flag-String to set flags
        for (int i = 0; str[i]; i++) {
            if (str[i] == 'H') {
                // e->setType(Ipv4Route::DIRECT);
            }
            else if (str[i] == 'G') {
                // e->setType(Ipv4Route::REMOTE);
            }
            else {
                throw cRuntimeError("Syntax error in routing file: 4th column should be `G' or `H' not `%s'", str);
            }
        }

        // 5th entry: metric
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        int metric = atoi(str);
        if (metric == 0 && str[0] != '0')
            throw cRuntimeError("Syntax error in routing file: 5th column should be numeric not `%s'", str);

        e->setMetric(metric);

        // 6th entry: interface
        opp_string interfaceName;
        interfaceName.reserve(MAX_ENTRY_STRING_SIZE);
        pos += strcpyword(interfaceName.buffer(), routeFile + pos);
        skipBlanks(routeFile, pos);
        InterfaceEntry *ie = ift->findInterfaceByName(interfaceName.c_str());
        if (!ie)
            throw cRuntimeError("Syntax error in routing file: 6th column: `%s' is not an existing interface", interfaceName.c_str());

        e->setInterface(ie);
        e->setSourceType(IRoute::MANUAL);

        // add entry
        rt->addRoute(e);
    }
    delete [] str;
}

} // namespace inet

