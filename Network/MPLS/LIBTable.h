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

    struct PRTEntry
    {
        int pos;
        int fecValue;
    };

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
    int readLibTableFromFile(const char* filename);

    /**
     * Load the Partial Routing Information from files
     *
     * @param filename The prt table file input
     * @return The successfule or unsuccesful code
     */
    int readPrtTableFromFile(const char* filename);


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
     * @param outLabel     The label returned from peers
     * @param inInterface  The name of the incoming interface of the label mapping
     * @param outInterface The name of the outgoing interface that new label mapping
     *                     will be forwarded
     * @param fec          The Forward Equivalent Class of the label
     * @param optcode      The optcode used in this Label Switching Router
     * @return             The value of the new label installed
     */
    int installNewLabel(int outLabel, std::string inInterface,
                        std::string outInterface, int fec, int optcode);

    /**
     * Find outgoing label for a packet based on the packet's Forward Equivalent Class
     *
     * @param fec   The packet's FEC
     * @return      The outgoing label, or -2 if there is no label found
     */
    int findLabelforFec(int fec) const;

    /**
     * Find the FEC based on corresponding incoming label and incoming interface
     *
     * @param label        The incoming label
     * @param inInterface  The incoming interface
     * @return             The FEC value, or 0 if the FEC cannot be found
     */
    //FIXME unused
    int findFec(int label, std::string inInterface) const;

    /**
     * Find the outgoing interface based on the incoming interface and the outgoing label
     *
     * @param senderInterface  The incoming interface name
     * @param newLabel         The outgoing label
     * @return                 The outgoing interface name or "X" if the outgoing interface
     *                         cannot be found
     */
    // FIXME Why "X" ??? (Andras)
    std::string findOutgoingInterface(std::string senderInterface,int newLabel) const;

    /**
     * Find the outgoing interface name based on the FEC.
     *
     * @param fec   The FEC value
     * @return      The outgoing interface name
     */
    std::string findOutgoingInterface(int fec) const;

    /**
     * Find the outgoing interface name based on incoming interface,
     * outgoing label and incoming label.
     * If the new label is not -1 (native IP), this function is the same
     * as findOutgoingInterface(string senderInterface, int newLabel).
     *
     * @param senderInterface The incoming interface
     * @param newLabel        The outgoing label
     * @param oldLabel        The incoming label
     * @return                The outgoing interface name
     */
    std::string findOutgoingInterface(std::string senderInterface, int newLabel, int oldLabel) const;

    /**
     * Returns new label based on incoming interface and incoming label
     *
     * @param senderInterface The incoming interface
     * @param oldLabel        The incoming label
     * @return                The value of the new label, or -2 if not found
     */
    int findNewLabel(std::string senderInterface,int oldLabel) const;

    /**
     * Returns the optcode based on incoming interface and incoming label.
     *
     * @param senderInterface The incoming interface name
     * @param oldLabel        The incoming label
     * @return                The operation code
     */
    int getOptCode(std::string senderInterface, int oldLabel) const;
};

std::ostream& operator<<(std::ostream& os, const LIBTable::PRTEntry& prt);
std::ostream& operator<<(std::ostream& os, const LIBTable::LIBEntry& lib);

#endif

