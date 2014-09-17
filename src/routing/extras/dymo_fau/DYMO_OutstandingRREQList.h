/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __INET_DYMO_OUTSTANDINGRREQLIST_H
#define __INET_DYMO_OUTSTANDINGRREQLIST_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/routing/extras/dymo_fau/DYMO_Timer.h"

namespace inet {

namespace inetmanet {

class DYMO_OutstandingRREQ
{
  public:
    unsigned int tries;
    DYMO_Timer* wait_time;
    unsigned int destAddr;
    simtime_t creationTime;

  public:
    friend std::ostream& operator<<(std::ostream& os, const DYMO_OutstandingRREQ& o);
};


class DYMO_OutstandingRREQList : public cObject
{
  public:
    DYMO_OutstandingRREQList();
    ~DYMO_OutstandingRREQList();

    /** @brief inherited from cObject */
    virtual const char* getFullName() const;

    /** @brief inherited from cObject */
    virtual std::string info() const;

    /** @brief inherited from cObject */
    virtual std::string detailedInfo() const;

    /**
     * @returns DYMO_OutstandingRREQ with matching destAddr or 0 if none is found
     */
    DYMO_OutstandingRREQ* getByDestAddr(unsigned int destAddr, int prefix);

    /**
     * @returns a DYMO_OutstandingRREQ whose wait_time is expired or 0 if none is found
     */
    DYMO_OutstandingRREQ* getExpired();

    bool hasActive() const;

    void add(DYMO_OutstandingRREQ* outstandingRREQ);

    void del(DYMO_OutstandingRREQ* outstandingRREQ);

    void delAll();

  protected:
    cModule* host;
    std::vector<DYMO_OutstandingRREQ*> outstandingRREQs;

  public:
    friend std::ostream& operator<<(std::ostream& os, const DYMO_OutstandingRREQList& o);

};

} // namespace inetmanet

} // namespace inet

#endif

