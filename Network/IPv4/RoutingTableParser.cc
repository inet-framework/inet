//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//  Cleanup and rewrite: Andras Varga, 2004

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "RoutingTableParser.h"


//
// Tokens that mark the route file.
//
const char  *IFCONFIG_START_TOKEN = "ifconfig:",
            *IFCONFIG_END_TOKEN = "ifconfigend.",
            *ROUTE_START_TOKEN = "route:",
            *ROUTE_END_TOKEN = "routeend.";

RoutingTableParser::RoutingTableParser(RoutingTable *r)
{
    rt = r;
}


int RoutingTableParser::streq(const char *str1, const char *str2)
{
    return (strncmp(str1, str2, strlen(str2)) == 0);
}


int RoutingTableParser::strcpyword(char *dest, const char *src)
{
    int i;
    for(i = 0; !isspace(dest[i] = src[i]); i++) ;
    dest[i] = '\0';
    return i;
}


void RoutingTableParser::skipBlanks (char *str, int &charptr)
{
    for(;isspace(str[charptr]); charptr++) ;
}


int RoutingTableParser::readRoutingTableFromFile (const char *filename)
{
    FILE *fp;
    int charpointer;
    char *file = new char[MAX_FILESIZE];
    char *ifconfigFile = NULL;
    char *routeFile = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL)
        opp_error("Error on opening routing table file.");

    // read the whole into the file[] char-array
    for (charpointer = 0;
         (file[charpointer] = getc(fp)) != EOF;
         charpointer++) ;

    charpointer++;
    for (; charpointer < MAX_FILESIZE; charpointer++)
        file[charpointer] = '\0';
    //    file[++charpointer] = '\0';

    fclose(fp);


    // copy file into specialized, filtered char arrays
    for (charpointer = 0;
         (charpointer < MAX_FILESIZE) && (file[charpointer] != EOF);
         charpointer++) {
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

    delete file;

    // parse filtered files
    if (ifconfigFile)
        parseInterfaces(ifconfigFile);
    if (routeFile)
        parseRouting(routeFile);

    delete ifconfigFile;
    delete routeFile;

    return 0;

}

char *RoutingTableParser::createFilteredFile (char *file,
                                              int &charpointer,
                                              const char *endtoken)
{
    int i = 0;
    char *filterFile = new char[MAX_FILESIZE];
    filterFile[0] = '\0';

    while(true) {
        // skip blank lines and comments
        // FIXME TBD consider this: while ( !isalnum(file[charpointer]) && !isspace(file[charpointer]) ) {
        while ( !isalnum(file[charpointer]) ) {
            while (file[charpointer++] != '\n') ;
        }

        // check for endtoken:
        if (streq(file + charpointer, endtoken)) {
            filterFile[i] = '\0';
            break;
        }

        // copy whole line to filterFile
        while ((filterFile[i++] = file[charpointer++]) != '\n') ;
    }

    return filterFile;
}


void RoutingTableParser::parseInterfaces(char *ifconfigFile)
{
    int charpointer = 0;
    InterfaceEntry *e;

    // parsing of entries in interface definition
    while(ifconfigFile[charpointer] != '\0')
    {
        // name entry
        if (streq(ifconfigFile + charpointer, "name:")) {
            // find existing interface with this name
            char *name = parseInterfaceEntry(ifconfigFile, "name:", charpointer,
                                             new char[MAX_ENTRY_STRING_SIZE]);
            e = rt->interfaceByName(name);
            if (!e)
                opp_error("Error in routing file: interface name `%s' not registered by any L2 module", name);
            delete [] name;
            continue;
        }

        // encap entry
        if (streq(ifconfigFile + charpointer, "encap:")) {
            // ignore encap
            parseInterfaceEntry(ifconfigFile, "encap:", charpointer,
                                new char[MAX_ENTRY_STRING_SIZE]);
            continue;
        }

        // HWaddr entry
        if (streq(ifconfigFile + charpointer, "HWaddr:")) {
            // ignore hwAddr
            parseInterfaceEntry(ifconfigFile, "HWaddr:", charpointer,
                                new char[MAX_ENTRY_STRING_SIZE]);
            continue;
        }

        // inet_addr entry
        if (streq(ifconfigFile + charpointer, "inet_addr:")) {
            e->inetAddr = IPAddress(parseInterfaceEntry(ifconfigFile, "inet_addr:", charpointer,
                                    new char[MAX_ENTRY_STRING_SIZE]));  // FIXME mem leak
            continue;
        }

        // Broadcast address entry
        if (streq(ifconfigFile + charpointer, "Bcast:")) {
            // ignore Bcast
            parseInterfaceEntry(ifconfigFile, "Bcast:", charpointer,
                                new char[MAX_ENTRY_STRING_SIZE]);  // FIXME mem leak
            continue;
        }

        // Mask entry
        if (streq(ifconfigFile + charpointer, "Mask:")) {
            e->mask = IPAddress(parseInterfaceEntry(ifconfigFile, "Mask:", charpointer,
                                new char[MAX_ENTRY_STRING_SIZE]));  // FIXME mem leak
            continue;
        }

        // Multicast groups entry
        if (streq(ifconfigFile + charpointer, "Groups:")) {
            char *grStr = parseInterfaceEntry(ifconfigFile, "Groups:",
                                              charpointer,
                                              new char[MAX_GROUP_STRING_SIZE]);
            //PRINTF("\nMulticast gr str: %s\n", grStr);
            parseMulticastGroups(grStr, e);
            continue;
        }

        // MTU entry
        if (streq(ifconfigFile + charpointer, "MTU:")) {
            e->mtu = atoi(
                parseInterfaceEntry(ifconfigFile, "MTU:", charpointer,
                                    new char[MAX_ENTRY_STRING_SIZE])); // FIXME mem leak
            continue;
        }

        // Metric entry
        if (streq(ifconfigFile + charpointer, "Metric:")) {
            e->metric = atoi(
                parseInterfaceEntry(ifconfigFile, "Metric:", charpointer,
                                    new char[MAX_ENTRY_STRING_SIZE]));
            continue;
        }

        // BROADCAST Flag
        if (streq(ifconfigFile + charpointer, "BROADCAST")) {
            e->broadcast = true;
            charpointer += strlen("BROADCAST");
            skipBlanks(ifconfigFile, charpointer);
            continue;
        }

        // MULTICAST Flag
        if (streq(ifconfigFile + charpointer, "MULTICAST")) {
            e->multicast = true;
            charpointer += strlen("MULTICAST");
            skipBlanks(ifconfigFile, charpointer);
            continue;
        }

        // POINTTOPOINT Flag
        if (streq(ifconfigFile + charpointer, "POINTTOPOINT")) {
            e->pointToPoint= true;
            charpointer += strlen("POINTTOPOINT");
            skipBlanks(ifconfigFile, charpointer);
            continue;
        }

        // no entry discovered: move charpointer on
        charpointer++;
    }
}


char *RoutingTableParser::parseInterfaceEntry (char *ifconfigFile,
                                               const char *tokenStr,
                                               int &charpointer,
                                               char* destStr)
{
    int temp = 0;

    charpointer += strlen(tokenStr);
    skipBlanks(ifconfigFile, charpointer);
    temp = strcpyword(destStr, ifconfigFile + charpointer);
    charpointer += temp;

    skipBlanks(ifconfigFile, charpointer);

    return destStr;
}


void RoutingTableParser::parseMulticastGroups (char *groupStr,
                                              InterfaceEntry *itf)
{
    int i, j, groupNo;

    itf->multicastGroupCtr = 1;

    // add "224.0.0.1" automatically,
    // use ":"-separator only if string not empty
    if (!strcmp(groupStr, "")) {
        strcat(groupStr, "224.0.0.1");
    } else { // string not empty, use seperator
        strcat(groupStr, ":224.0.0.1");
    }

    // add 224.0.0.2" only if Router (IP forwarding enabled)
    if (rt->ipForward()) {
        strcat(groupStr, ":224.0.0.2");
    }

    // count number of group entries
    for (i = 0; groupStr[i] != '\0'; i++) {
        if (groupStr[i] == ':')
            itf->multicastGroupCtr++;
    }

    char *str = new char[ADDRESS_STRING_SIZE];
    itf->multicastGroup = new IPAddress[itf->multicastGroupCtr];

    // Create the different IPAddress
    for (i = 0, j = 0, groupNo = 0; groupStr[i] != '\0'; i++, j++) {
        // Skip to next multicast group, if separator found
        // it's a bit a ugly...
        if (groupStr[i] == ':') {
            str[j] = '\0';
            itf->multicastGroup[groupNo] = IPAddress(str);
            j = -1;
            groupNo++;
            continue;
        }
        if (groupStr[i + 1] == '\0') {
            str[j] = groupStr[i];
            str[j + 1] = '\0';
            itf->multicastGroup[groupNo] = IPAddress(str);
            break;
        }
        str[j] = groupStr[i];
    }
}

void RoutingTableParser::parseRouting(char *routeFile)
{
    char *str = new char[MAX_ENTRY_STRING_SIZE];

    int pos = strlen(ROUTE_START_TOKEN);
    skipBlanks(routeFile, pos);
    while (routeFile[pos] != '\0')
    {
        // 1st entry: Host
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        RoutingEntry *e = new RoutingEntry();
        if (strcmp(str, "default:"))
        {
            // if entry is not the default entry
            if (!IPAddress::isWellFormed(str))
                opp_error("Syntax error in routing file: `%s' on 1st column should be `default:' or a valid IP address", str);
            e->host = IPAddress(str);
        }

        // 2nd entry: Gateway
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        if (!strcmp(str, "*") || !strcmp(str, "0.0.0.0"))
        {
            e->gateway = IPAddress("0.0.0.0");
        }
        else
        {
            if (!IPAddress::isWellFormed(str))
                opp_error("Syntax error in routing file: `%s' on 2nd column should be `*' or a valid IP address", str);
            e->gateway = IPAddress(str);
        }

        // 3rd entry: Netmask
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        if (!IPAddress::isWellFormed(str))
            opp_error("Syntax error in routing file: `%s' on 3rd column should be a valid IP address", str);
        e->netmask = IPAddress(str);

        // 4th entry: flags
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        // parse flag-String to set flags
        for (int i = 0; str[i]; i++)
        {
            if (str[i] == 'H') {
                e->type = RoutingEntry::DIRECT;
            } else if (str[i] == 'G') {
                e->type = RoutingEntry::REMOTE;
            } else {
                opp_error("Syntax error in routing file: 4th column should be `G' or `H' not `%s'", str);
            }
        }

        // 5th entry: references (ignored)
        pos += strcpyword(str, routeFile + pos);
        skipBlanks(routeFile, pos);
        int ref = atoi(str);
        if (ref==0 && str[0]!='0')
            opp_error("Syntax error in routing file: 5th column should be numeric not `%s'", str);

        // 6th entry: interfaceName
        e->interfaceName.reserve(MAX_ENTRY_STRING_SIZE);
        pos += strcpyword(e->interfaceName.buffer(), routeFile + pos);
        skipBlanks(routeFile, pos);
        e->interfacePtr = rt->interfaceByName(e->interfaceName.c_str());
        if (e->interfacePtr==NULL)
            opp_error("Syntax error in routing file: 6th column should be an existing "
                      "interface name not `%s'", e->interfaceName.c_str());

        // add entry
        rt->addRoutingEntry(e);
    }
}

