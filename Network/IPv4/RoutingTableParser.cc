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
    char *ifconfigFile;
    char *routeFile;

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
    parseInterfaces(ifconfigFile);
    addLocalLoopback();
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
            e = new InterfaceEntry();
            rt->intrface[rt->numIntrfaces] = e;
            rt->numIntrfaces++; // ready for the next one
            e->name = parseInterfaceEntry(ifconfigFile, "name:", charpointer,
                                          new char[MAX_ENTRY_STRING_SIZE]);
            continue;
        }

        // encap entry
        if (streq(ifconfigFile + charpointer, "encap:")) {
            e->encap = parseInterfaceEntry(ifconfigFile, "encap:", charpointer,
                                           new char[MAX_ENTRY_STRING_SIZE]);
            continue;
        }

        // HWaddr entry
        if (streq(ifconfigFile + charpointer, "HWaddr:")) {
            e->hwAddrStr = parseInterfaceEntry(ifconfigFile, "HWaddr:", charpointer,
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
            e->bcastAddr = IPAddress(parseInterfaceEntry(ifconfigFile, "Bcast:", charpointer,
                                     new char[MAX_ENTRY_STRING_SIZE]));  // FIXME mem leak
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
                                    new char[MAX_ENTRY_STRING_SIZE]));
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

    // add default multicast groups to all interfaces without
    // set multicast group field
    for (int i = 0; i < rt->numIntrfaces; i++) {
        InterfaceEntry *interf = rt->intrface[i];
        if (interf->multicastGroupCtr == 0) {
            char emptyGroupStr[MAX_GROUP_STRING_SIZE];
            strcpy(emptyGroupStr, "");
            parseMulticastGroups(emptyGroupStr, interf);
        }
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

    // add 224.0.0.2" only if Router (IPForward == true)
    if (rt->IPForward) {
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


void RoutingTableParser::addLocalLoopback()
{
    rt->loopbackInterface = new InterfaceEntry();

    rt->loopbackInterface->name = "lo";
    rt->loopbackInterface->encap = "Local Loopback";

    //loopbackInterface->inetAddr = IPAddress("127.0.0.1");
    //loopbackInterface->mask = IPAddress("255.0.0.0");
// BCH Andras -- code from UTS MPLS model
    cModule *curmod = rt;
    IPAddress loopbackIP = IPAddress("127.0.0.1");

    for (curmod = rt->parentModule(); curmod != NULL;curmod = curmod->parentModule())
    {
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        // the following line is a terrible hack. For some unknown reason,
        // the MPLS models use the host's "local_addr" parameter (string)
        // as loopback address (and also change its netmask to 255.255.255.255).
        // But this conflicts with the IPSuite which also has "local_addr" parameters,
        // numeric and not intended for use as loopback address. So until we
        // figure out why exactly the MPLS models do this, we just patch up
        // the thing and only regard "local_addr" parameters that are strings....
        // Horrible hacking.  --Andras
        if (curmod->hasPar("local_addr") && curmod->par("local_addr").type()=='S')
        {
            loopbackIP = IPAddress(curmod->par("local_addr").stringValue());
            break;
        }

    }
    ev << "My loopback Address is : " << loopbackIP << "\n";
    rt->loopbackInterface->inetAddr = loopbackIP;
    rt->loopbackInterface->mask = IPAddress("255.255.255.255");  // ????? -- Andras
// ECH

    rt->loopbackInterface->mtu = 3924;
    rt->loopbackInterface->metric = 1;
    rt->loopbackInterface->loopback = true;

    // add default multicast groups
    char emptyGroupStr[MAX_GROUP_STRING_SIZE];
    strcpy(emptyGroupStr, "");
    parseMulticastGroups(emptyGroupStr, rt->loopbackInterface);
}


void RoutingTableParser::parseRouting(char *routeFile)
{
    int i, charpointer = 0;
    RoutingEntry *e;
    char *str = new char[MAX_ENTRY_STRING_SIZE];

    charpointer += strlen(ROUTE_START_TOKEN);
    skipBlanks(routeFile, charpointer);
    while (routeFile[charpointer] != '\0')
    {
        // 1st entry: Host
        charpointer += strcpyword(str, routeFile + charpointer);
        skipBlanks(routeFile, charpointer);
        e = new RoutingEntry();
        if (strcmp(str, "default:"))
        {
            // if entry is not the default entry
            if (!IPAddress::isWellFormed(str))
                opp_error("Syntax error in routing file: `%s' on 1st column should be `default:' or a valid IP address", str);

            e->host = IPAddress(str);

            // check if entry is for multicast address
            if (!e->host.isMulticast()) {
                rt->route->add(e);
            } else {
                rt->mcRoute->add(e);
            }
        }
        else
        {
            // default entry
            rt->defaultRoute = e;
        }

        // 2nd entry: Gateway
        charpointer += strcpyword(str, routeFile + charpointer);
        skipBlanks(routeFile, charpointer);
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
        charpointer += strcpyword(str, routeFile + charpointer);
        skipBlanks(routeFile, charpointer);
        if (!IPAddress::isWellFormed(str))
            opp_error("Syntax error in routing file: `%s' on 3rd column should be a valid IP address", str);
        e->netmask = IPAddress(str);

        // 4th entry: flags
        charpointer += strcpyword(str, routeFile + charpointer);
        skipBlanks(routeFile, charpointer);
        // parse flag-String to set flags
        for(i = 0; str[i]; i++)
        {
            if (str[i] == 'H') {
                e->type = DIRECT;
            } else if (str[i] == 'G') {
                e->type = REMOTE;
            } else {
                opp_error("Syntax error in routing file: 4th column should be `G' or `H' not `%s'", str);
            }
        }

        // 5th entry: references (unsupported by Linux)
        charpointer += strcpyword(str, routeFile + charpointer);
        skipBlanks(routeFile, charpointer);
        e->ref = atoi(str);
        if (e->ref==0 && str[0]!='0')
            opp_error("Syntax error in routing file: 5th column should be numeric not `%s'", str);

        // 6th entry: interfaceName
        e->interfaceName.reserve(MAX_ENTRY_STRING_SIZE);
        charpointer += strcpyword(e->interfaceName.buffer(), routeFile + charpointer);
        skipBlanks(routeFile, charpointer);
        e->interfaceNo = rt->findInterfaceByName(e->interfaceName.c_str());
        if (e->interfaceNo==-1)
            opp_error("Syntax error in routing file: 6th column should be an existing "
                      "interface name not `%s'", e->interfaceName.c_str());
    }
}

