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

#include "PortFiltRSTP.h"

Define_Module(PortFiltRSTP);

void PortFiltRSTP::initialize()
{
    //Getting and validating parameters
    verbose=par("verbose");
    tagged=par("tagged");
    cost=par("cost");
    priority=par("priority");
    if (ev.isGUI())
        updateDisplayString();
}

void PortFiltRSTP::handleMessage(cMessage *msg)
{
    int arrival=msg->getArrivalGate()->getIndex();
    switch(arrival)
    {
       case 0:
           send(msg,"GatesOut",1);
           break;
       case 1:
           send(msg,"GatesOut",0);
           break;
       default:
           error("Unknown arrival gate");
           delete msg;
           break;
    }
}

void PortFiltRSTP::updateDisplayString()
{  //Tkenv shows port type.
    char buf[80];
    if(tagged==true)
        sprintf(buf, "Tagged");
    else
        sprintf(buf, "Untagged");
    getDisplayString().setTagArg("t",0,buf);
}

int PortFiltRSTP::getCost()
{// Returns RSTP port cost.
    return cost;
}

int PortFiltRSTP::getPriority()
{// Returns RSTP port priority.
    return priority;
}

bool PortFiltRSTP::isEdge()
{// Returns RSTP port type.
    return !tagged;
}
