/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#ifndef __LIBTABLE_H
#define __LIBTABLE_H

#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "RoutingTableAccess.h"
#include "ConstType.h"



/**
 * Manages the label space and label mappings that belong to the current LSR.
 * For more info see the NED file.
 *
 * Table examples:
 *
 * LIB TABLE:
 * <pre>
 * In-lbl  In-intf  Out-lbl   Out-intf Optcode
 *    L1      ppp0     L2        ppp3     1
 *    L3      ppp1     L2        ppp4     1
 * </pre>
 *
 * PRT TABLE:
 * <pre>
 *  Fec   Pointer
 *    F1    3
 *    F3    5
 * </pre>
 *
 * Prefix TABLE:
 * <pre>
 *  Prefix        FEC
 *   124.3.0.0     F1
 *   135.6.1.0     F2
 * </pre>
 */
class LIBTable: public cSimpleModule
{
public:
    // PRT (Prefix Table) entry. The PRT table maps one or more FECs
    // to a single LIB entry.
    struct PRTEntry
    {
        int fec;       // *not* fecId! (this is destAddr)
        int libIndex;  // index into the LIB table
    };

    // LIB (Label Information Base) entry
    struct LIBEntry
    {
        int inLabel;
        int outLabel;
        std::string inInterface;
        std::string outInterface;
        int optcode;
    };

private:
    std::vector<LIBEntry> lib;
    std::vector<PRTEntry> prt;

    /**
     * Load the label information table from files
     *
     *  @param filename The lib table file input
     *  @return The successful or unsuccessful code
     */
    int readLibTableFromFile(const char *filename);

    /**
     * Load the Partial Routing Information from files
     *
     * @param filename The prt table file input
     * @return The successfule or unsuccesful code
     */
    int readPrtTableFromFile(const char *filename);

    // display LIB and PRT table sizes above icon
    void updateDisplayString();

public:

    Module_Class_Members(LIBTable, cSimpleModule, 0);

    /**
     * Initializes all the parameters.
     */
    void initialize();

    /**
     * Handles message from other modules
     */
    void handleMessage(cMessage *msg);

    /**
     * Print out the contents of Label Information Base and Partial Routing Table
     */
    void printTables() const;

    /**
     * Installs a new label on this Label Switching Router when receiving a label
     * mapping from peers.
     *
     * @param outLabel     The label returned from peers (-1 on Egress router)
     * @param inInterface  The name of the incoming interface of the label mapping
     * @param outInterface The name of the outgoing interface that new label mapping
     *                     will be forwarded
     * @param fec          The Forwarding Equivalence Class of the label
     * @param optcode      The optcode used in this Label Switching Router
     * @return             The value of the new label installed
     */
    int installNewLabel(int outLabel, std::string inInterface,
                        std::string outInterface, int fec, int optcode);

    /**
     * Returns inLabel (if exists) for a FEC (inInterface is irrelevant because
     * we want to use unified label space for all input interfaces).
     * Used by signalling protocols (e.g. LDP) to determine if a Label Request
     * already has a corresponding entry in LIB, or it should be created.
     * Returns -1 if there's no such entry in LIB.
     */
    int findInLabel(int fec);

    /**
     * Given a FEC, it returns (outLabel, outInterface) in the last two
     * arguments. Returns false if the FEC was not found in the table.
     */
    bool resolveFec(int fec, int& outLabel, std::string& outInterface) const;

    /**
     * Given (inLabel, inInterface), it returns (optCode, outLabel, outInterface)
     * in the last three arguments. Returns false if (inLabel, inInterface)
     * was not found in the table.
     */
    bool resolveLabel(int inLabel, std::string inInterface,
                      int& outOptCode, int& outNewLabel, std::string& outInterface) const;
};

std::ostream& operator<<(std::ostream& os, const LIBTable::PRTEntry& prt);
std::ostream& operator<<(std::ostream& os, const LIBTable::LIBEntry& lib);

#endif

