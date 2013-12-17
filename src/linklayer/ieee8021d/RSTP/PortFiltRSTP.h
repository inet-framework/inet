//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_PORTFILTRSTP_H_
#define __INET_PORTFILTRSTP_H_

#include <omnetpp.h>

class PortFiltRSTP : public cSimpleModule
{
  protected:
    bool tagged;        ///Port type.
    bool verbose;
    int cost;           ///RSTP port cost.
    int priority;       ///RSTP port priority

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
// utility functions
    /**
    * Shows port type at Tkenv.
    */
    virtual void updateDisplayString();

  public:

    /**
     * @return RSTP port cost.
     */
    virtual int getCost();

    /**
     * @return RSTP port priority.
     */
    virtual int getPriority();

    /**
     * @return true if it is an edge port.
     */
    virtual bool isEdge();
};

#endif
