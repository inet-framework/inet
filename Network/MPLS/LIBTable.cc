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


std::ostream & operator<<(std::ostream & os, const prt_type & prt)
{
    return os << "Pos:" << prt.pos << "  Fec:" << prt.fecValue;
}

std::ostream & operator<<(std::ostream & os, const lib_type & lib)
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
   // Read routing table file
   const char *libFilename = par("LibTableFileName").stringValue();
   const char *prtFilename = par("PrtTableFileName").stringValue();

   readLibTableFromFile(libFilename);
   readPrtTableFromFile(prtFilename);

   printTables();
*/
    WATCH_VECTOR(lib);
    WATCH_VECTOR(prt);

}

void LIBTable::handleMessage(cMessage *)
{
    error
        ("Message arrived -- LIBTable doesn't process messages, it is used via direct method calls");
}


int LIBTable::readLibTableFromFile(const char *filename)
{
    ifstream fin(filename);
    if (!fin)
        error("Cannot open file %s", filename);

    char line[100];

    // Move to the first record
    while (fin.getline(line, 100) && !fin.eof())
    {
        string s(line);
        int loc = s.find(ConstType::libDataMarker, 0);

        if (loc != string::npos)
        {
            fin.getline(line, 100);  // Get into the blank line
            break;
        }
    }

    while (fin.getline(line, 100) && !fin.eof())
    {
        lib_type record;

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
    ifstream fin(filename);
    if (!fin)
        error("Cannot open file %s", filename);

    char line[100];

    // Move to the first record
    while (fin.getline(line, 100) && !fin.eof())
    {
        string s(line);
        int loc = s.find(ConstType::prtDataMarker, 0);

        if (loc != string::npos)
        {
            fin.getline(line, 100);  // Get into the blank line
            break;
        }
    }

    while (fin.getline(line, 100) && !fin.eof())
    {
        prt_type record;

        if (!line[0])
            break;  // Reach the end of table

        StringTokenizer tokenizer(line, ", ");
        const char *aField;
        if ((aField = tokenizer.nextToken()) != NULL)
            record.fecValue = IPAddressPrefix(aField, ConstType::prefixLength);
        else
            record.fecValue = IPAddressPrefix("0.0.0.0", ConstType::prefixLength);

        if ((aField = tokenizer.nextToken()) != NULL)
            record.pos = atoi(aField);
        else
            record.pos = -1;

        // Add the record
        prt.push_back(record);
    }
    return 0;
}


void LIBTable::printTables()
{
    int i;
    // Print out the LIB table
    ev << "************LIB TABLE CONTENTS***************** \n";
    ev << " InL       InInf        OutL     Outf    Optcode  \n";
    for (i = 0; i < lib.size(); i++)
        ev << lib[i].inLabel << "    " << lib[i].inInterface.c_str() << " " << lib[i].
            outLabel << "     " << lib[i].outInterface.c_str() << "   " << lib[i].optcode << "\n";

    // Print out the PRT table
    ev << "*****************PRT TABLE CONTENT**************\n";
    ev << "Pos  Fec \n";
    for (i = 0; i < prt.size(); i++)
        ev << prt[i].pos << "    " << prt[i].fecValue << "\n";
}

int LIBTable::installNewLabel(int outLabel, string inInterface,
                              string outInterface, int fec, int optcode)
{
    lib_type newLabelEntry;
    newLabelEntry.inInterface = inInterface;
    newLabelEntry.outInterface = outInterface;
    newLabelEntry.optcode = optcode;

    // Auto generate inLabel
    newLabelEntry.inLabel = lib.size();
    newLabelEntry.outLabel = outLabel;
    lib.push_back(newLabelEntry);

    prt_type aPrt;
    aPrt.pos = lib.size() - 1;
    aPrt.fecValue = fec;
    prt.push_back(aPrt);
    printTables();

    return newLabelEntry.inLabel;
}

int LIBTable::requestLabelforFec(int fec)
{
    // search in the PRT for exact match of the FEC value
    for (int i = 0; i < prt.size(); i++)
    {
        if (prt[i].fecValue.getInt() == fec)
        {
            int p = prt[i].pos;

            // Return the outgoing label
            return lib[p].outLabel;
        }
    }
    return -2;
}

int LIBTable::findFec(int label, string inInterface)
{
    int pos;

    // Search LIB for matching of the incoming interface and label
    for (int i = 0; i < lib.size(); i++)
    {
        if ((lib[i].inInterface == inInterface) && (lib[i].inLabel == label))
        {
            // Get the position of this match
            pos = i;
            break;
        }
    }

    // Get the FEC value in PRT tabel
    for (int k = 0; k < prt.size(); k++)
    {
        if (prt[k].pos == pos)
            return prt[k].fecValue.getInt();
    }
    return 0;

}

string LIBTable::requestOutgoingInterface(int fec)
{
    int pos = -1;

    // Search in PRT for matching of FEC
    for (int k = 0; k < prt.size(); k++)
    {
        if (prt[k].fecValue.getInt() == fec)
        {
            pos = prt[k].pos;
            break;
        }
    }

    if (pos != -1)
        return lib[pos].outInterface;
    else
        return string("X");
}

string LIBTable::requestOutgoingInterface(string senderInterface, int newLabel)
{
    for (int i = 0; i < lib.size(); i++)
    {
        if (senderInterface.compare("X") != 0)
        {
            if ((lib[i].inInterface == senderInterface) && (lib[i].outLabel == newLabel))
                return lib[i].outInterface;
        }
        else if (lib[i].outLabel == newLabel)  // LIB of the IR
            return lib[i].outInterface;
    }
    return string("X");
}

string LIBTable::requestOutgoingInterface(string senderInterface, int newLabel, int oldLabel)
{
    if (newLabel != -1)
        return requestOutgoingInterface(senderInterface, newLabel);

    for (int i = 0; i < lib.size(); i++)
    {
        if (senderInterface.compare("X") != 0)
        {
            if ((lib[i].inInterface == senderInterface) && (lib[i].inLabel == oldLabel))
                return lib[i].outInterface;
        }
        else if (lib[i].outLabel == newLabel)
            return lib[i].outInterface;
    }
    return string("X");
}

int LIBTable::requestNewLabel(string senderInterface, int oldLabel)
{
    for (int i = 0; i < lib.size(); i++)
        if ((lib[i].inInterface == senderInterface) && (lib[i].inLabel == oldLabel))
            return lib[i].outLabel;

    return -2;  // Fail to allocate new label
}

int LIBTable::getOptCode(string senderInterface, int oldLabel)
{
    for (int i = 0; i < lib.size(); i++)
        if ((lib[i].inInterface == senderInterface) && (lib[i].inLabel == oldLabel))
            return lib[i].optcode;
    return -1;
}

/*
string LIBTable::ini_requestLabelforDest(IPAddressPrefix *dest)
{
    string label(ConstType::UnknownData);
    for(int i=0;i<prt.size();i++)
    {
        if(prt[i].fecValue.compareTo(dest, ConstType::prefixLength)) // Mean that the dest find its prefix entry
        {
            int p = prt[i].pos;
            label =lib[p].outLabel;
            break;
        }
    }
    return label;
}

void LIBTable::updateTable(label_mapping_type *newMapping)
{
    prt_type *aPrt;
    IPAddressPrefix aFec=newMapping->fec;
    string label =newMapping->label;

    // Update the cross-entry
    for(int i=0;i<lib.size();i++)
    {
        if(lib[i].outLabel.compare(label)==0)
        {
            aPrt->pos=i;
            (aPrt->fecValue)= aFec;
            prt.push_back(*aPrt);
            break;
        }
    }
    return;
}

void LIBTable::updateTable( string inInterface, string outLabel,
                           string outInterface, IPAddressPrefix fec)
{
    string inLabel = string(label).append(string(lib.size()));

    lib_type newLabelEntry;
    newLabelEntry.inInterface =inInterface;
    newLabelEntry.outInterface=outInterface;
    newLabelEntry.inLabel =inLabel;
    newLabelEntry.outLabel =outLabel;
    lib.push_back(newLabelEntry);

    prt_type aPrt;
    aPrt.pos= lib.size()-1;
    aPrt.fecValue = fec;
    prt.push_back(aPrt);
}

string LIBTable::requestNewLabel(string senderInterface, string oldLabel)
{
    for(int i=0;i< lib.size();i++)
        if((lib[i].inInterface==senderInterface) &&  (lib[i].inLabel==oldLabel))
            return lib[i].outLabel;
    return string(ConstType::UnknownData);
}

string LIBTable::requestOutgoingInterface(string senderInterface, string newLabel)
{
    for(int i=0;i< lib.size();i++)
    {
        if(senderInterface.compare("OUTSIDE") !=0)
        {
            if((lib[i].inInterface==senderInterface) &&  (lib[i].outLabel==newLabel))
                return lib[i].outInterface;
        }
        else if(lib[i].outLabel==newLabel)
            return lib[i].outInterface;
    }
    return string(ConstType::UnknownData);
}

string LIBTable::requestIncomingInterface(IPAddressPrefix fec)
{
    string incomingInterface(ConstType::UnknownData);
    for(int i=0;i<prt.size();i++)
    {
        if(prt[i].fecValue.equals(fec))
        {
            int p = prt[i].pos;
            incomingInterface =lib[p].inInterface;
            break;
        }
    }
    return incomingInterface;
}

*/
