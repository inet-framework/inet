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

#ifndef __MPLS2IP_MODULE_H__
#define __MPLS2IP_MODULE_H__

#include <omnetpp.h>

#include "QueueBase.h"


/**
 * Adapter between MPLS and IP layers.
 */
class MPLS2IP: public QueueBase
{
  public:
    Module_Class_Members(MPLS2IP, QueueBase, 0);

  protected:
    virtual void endService(cMessage *msg);
};

#endif