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
#include <iostream>
#include "LIBtable.h"
#include "StringTokenizer.h"
#include "stlwatch.h"

using namespace std;


Define_Module(LIBTable);


std::ostream & operator<<(std::ostream & os, const LIBTable::PRTEntry & prt)
{
    return os << "FEC:" << prt.fec << "LIBIndex:" << prt.libIndex;
}

std::ostream & operator<<(std::ostream & os, const LIBTable::LIBEntry & lib)
{
    os << "InL:" << lib.inLabel;
    os << "  InIf:" << lib.inInterface.c_str();
    os << "  OutL:" << lib.outLabel;
    os << "  OutIf:" << lib.outInterface.c_str();
    os << "  Optcode:" << lib.optcode;
    return os;
}

void LIBTable::initialize()
{
/*
//FIXME why is it commented out? --Andras
   // Read routing table file
   const char *libFilename = par("LibTableFileName").stringValue();
   const char *prtFilename = par("PrtTableFileName").stringValue();

   readLibTableFromFile(libFilename);
   readPrtTableFromFile(prtFilename);

   printTables();
*/
    WATCH_VECTOR(lib);
    WATCH_VECTOR(prt);

    updateDisplayString();
}

void LIBTable::handleMessage(cMessage *)
{
    error("Message arrived -- LIBTable doesn't process messages, it is used via direct method calls");
}


int LIBTable::readLibTableFromFile(const char *filename)
{
    //FIXME revise file format!
    ifstream fin(filename);
    if (!fin)
        error("Cannot open file %s", filename);

    char line[100];

    // Move to the first record
    while (fin.getline(line, 100) && !fin.eof())
    {
        string s(line);
        int loc = s.find(ConstType::libDataMarker, 0);

        if (loc != (int)string::npos)
        {
            fin.getline(line, 100);  // Get into the blank line
            break;
        }
    }

    while (fin.getline(line, 100) && !fin.eof())
    {
        LIBEntry record;

        // Reach the end of table
        if (!line[0])
            break;

        StringTokenizer tokenizer(line, ", ");
        const char *aField;

        // Get the first field - Incoming Label
        if ((aField = tokenizer.nextToken()) != NULL)
            record.inLabel = atoi(aField);
        else
            record.inLabel = -1;

        // Get the second field - Incoming Interface
        if ((aField = tokenizer.nextToken()) != NULL)
            record.inInterface = aField;
        else
            record.inInterface = ConstType::UnknownData;

        // Get the third field - Outgoing Label
        if ((aField = tokenizer.nextToken()) != NULL)
            record.outLabel = atoi(aField);
        else
            record.outLabel = -1;

        // Get the fourth field - Outgoing Interface
        if ((aField = tokenizer.nextToken()) != NULL)
            record.outInterface = aField;
        else
            record.outInterface = ConstType::UnknownData;

        // Get the fifth field - Optcode
        if ((aField = tokenizer.nextToken()) != NULL)
            record.optcode = atoi(aField);
        else
            record.optcode = -1;

        // Insert into the vector
        lib.push_back(record);
    }
    return 0;
}

int LIBTable::readPrtTableFromFile(const char *filename)
{
    //FIXME revise file format!
    ifstream fin(filename);
    if (!fin)
        error("Cannot open file %s", filename);

    char line[100];

    // Move to the first record
    while (fin.getline(line, 100) && !fin.eof())
    {
        string s(line);
        int loc = s.find(ConstType::prtDataMarker, 0);

        if (loc != (int)string::npos)
        {
            fin.getline(line, 100);  // Get into the blank line
            break;
        }
    }

    while (fin.getline(line, 100) && !fin.eof())
    {
        PRTEntry record;

        if (!line[0])
            break;  // Reach the end of table

        StringTokenizer tokenizer(line, ", ");
        const char *aField;
        if ((aField = tokenizer.nextToken()) != NULL)
            record.fec = IPAddress(aField).getInt(); // FIXME probably whole PRT table is broken (Andras)
        else
            record.fec = 0;

        if ((aField = tokenizer.nextToken()) != NULL)
            record.libIndex = atoi(aField);
        else
            record.libIndex = -1;

        // Add the record
        prt.push_back(record);
    }
    return 0;
}


void LIBTable::printTables() const
{
    unsigned int i;

    // print out the LIB table
    ev << "*** LIB table contents:\n";
    ev << "  Index  InL     InInf     OutL     Outf    Optcode\n";
    for (i = 0; i < lib.size(); i++)
        ev << "  " << i << ":  "
           << "   " << lib[i].inLabel  << "   " << lib[i].inInterface.c_str()
           << "   " << lib[i].outLabel << "   " << lib[i].outInterface.c_str()
           << "   " << lib[i].optcode << "\n";

    // print out the PRT table
    ev << "*** PRT table contents:\n";
    ev << "FEC    LIBIndex\n";
    for (i = 0; i < prt.size(); i++)
        ev << "  " << prt[i].fec << "   " << prt[i].libIndex << "\n";
}

void LIBTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    sprintf(buf, "%d LIB entries\nfor %d FECs", lib.size(), prt.size());
    displayString().setTagArg("t",0,buf);
}

int LIBTable::installNewLabel(int outLabel, string inInterface,
                              string outInterface, int fec, int optcode)
{
    // Consistence check: we want to use unified label space for all input interfaces.
    // This means that any given FEC may only map to a single lib entry. So we must
    // check FEC does not already occur in prt.
    for (unsigned int i = 0; i < prt.size(); i++)
        if (prt[i].fec==fec)
            error("installNewLabel(): FEC %s already in LIB", IPAddress(fec).str().c_str());

    LIBEntry newLabelEntry;
    newLabelEntry.inInterface = inInterface;
    newLabelEntry.outInterface = outInterface;
    newLabelEntry.optcode = optcode;
    newLabelEntry.inLabel = lib.size();  // Auto generate inLabel
    newLabelEntry.outLabel = outLabel;
    lib.push_back(newLabelEntry);

    PRTEntry aPrt;
    aPrt.libIndex = lib.size()-1;
    aPrt.fec = fec;
    prt.push_back(aPrt);

    printTables();
    updateDisplayString();

    return newLabelEntry.inLabel;
}

int LIBTable::findInLabel(int fec)
{
    // search in the PRT for exact match of the FEC value
    for (unsigned int i = 0; i < prt.size(); i++)
    {
        if (prt[i].fec==fec)
        {
            int p = prt[i].libIndex;
            return lib[p].inLabel;
        }
    }

    // not found
    return -1;
}

bool LIBTable::resolveFec(int fec, int& outLabel, std::string& outInterface) const
{
    // search in the PRT for exact match of the FEC value
    for (unsigned int i = 0; i < prt.size(); i++)
    {
        if (prt[i].fec == fec)
        {
            // found
            int p = prt[i].libIndex;

            outLabel = lib[p].outLabel;
            outInterface = lib[p].outInterface;
            return true;
        }
    }

    // not found
    outLabel = -1;
    outInterface = "";
    return false;
}

bool LIBTable::resolveLabel(int inLabel, std::string inInterface,
                            int& outOptCode, int& outLabel,
                            std::string& outInterface) const
{
    for (unsigned int i = 0; i < lib.size(); i++)
    {
        if (lib[i].inInterface == inInterface && lib[i].inLabel == inLabel)
        {
            // found
            outOptCode = lib[i].optcode;
            outLabel = lib[i].outLabel;
            outInterface = lib[i].outInterface;
            return true;
        }
    }

    // not found
    outOptCode = -1;
    outLabel = -1;
    outInterface = "";
    return false;
}


