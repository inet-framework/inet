//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IMACADDRESSTABLE_H
#define __INET_IMACADDRESSTABLE_H

#include "inet/linklayer/common/MacAddress.h"

namespace inet {

/*
 * A C++ interface to abstract the functionality of IMacAddressTable.
 */
class INET_API IMacAddressTable
{
  public:
    /**
     *  @brief Clears portno cache
     */
    virtual void flush(int portno) = 0;

    /**
     * @brief Copy cache from portA to portB port
     */
    virtual void copyTable(int portA, int portB) = 0;

    /*
     * Some (eg.: STP, RSTP) protocols may need to change agingTime
     */
    virtual void setAgingTime(simtime_t agingTime) = 0;
    virtual void resetDefaultAging() = 0;
};

} // namespace inet

#endif

