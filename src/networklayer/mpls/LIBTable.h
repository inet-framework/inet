//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_LIBTABLE_H
#define __INET_LIBTABLE_H

#include <vector>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/mpls/ConstType.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

namespace inet {

// label operations
#define PUSH_OPER    0
#define SWAP_OPER    1
#define POP_OPER     2

/**
 * TODO documentation
 */
struct LabelOp
{
    int label;
    int optcode;
};

typedef std::vector<LabelOp> LabelOpVector;

/**
 * TODO documentation
 */
class INET_API LIBTable : public cSimpleModule
{
  public:
    struct LIBEntry
    {
        int inLabel;
        std::string inInterface;

        LabelOpVector outLabel;
        std::string outInterface;

        // FIXME colors in nam, temporary solution
        int color;
    };

  protected:
    int maxLabel;
    std::vector<LIBEntry> lib;

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);

    // static configuration
    virtual void readTableFromXML(const cXMLElement *libtable);

  public:
    // label management
    virtual bool resolveLabel(std::string inInterface, int inLabel,
            LabelOpVector& outLabel, std::string& outInterface, int& color);

    virtual int installLibEntry(int inLabel, std::string inInterface, const LabelOpVector& outLabel,
            std::string outInterface, int color);

    virtual void removeLibEntry(int inLabel);

    // utility
    static LabelOpVector pushLabel(int label);
    static LabelOpVector swapLabel(int label);
    static LabelOpVector popLabel();
};

std::ostream& operator<<(std::ostream& os, const LIBTable::LIBEntry& lib);
std::ostream& operator<<(std::ostream& os, const LabelOpVector& label);

} // namespace inet

#endif // ifndef __INET_LIBTABLE_H

