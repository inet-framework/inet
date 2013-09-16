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

#ifndef VLANINTERFACEDATA_H_
#define VLANINTERFACEDATA_H_

#include "NotificationBoard.h"
#include "InterfaceEntry.h"

class VLANInterfaceData : public InterfaceProtocolData
{
protected:
    bool tagged;        ///Port type.
    int cost;           ///RSTP port cost.
    int priority;       ///RSTP port priority

public:
    VLANInterfaceData();
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    /** @name Getters */
    //@{
    bool isTagged() const {return tagged;}
    int getCost() const {return cost;}
    int getPriority() const {return priority;}
    //@}

    /** @name Setters */
    //@{
    virtual void setTagged(bool b) {tagged = b;}
    virtual void setCost(int c) {cost = c;}
    virtual void setPriority(int p) {priority = p;}
    //@}
};

#endif /* VLANINTERFACEDATA_H_ */
