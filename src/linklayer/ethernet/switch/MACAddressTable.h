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

#ifndef __INET_MACADDRESSTABLE_H_
#define __INET_MACADDRESSTABLE_H_

#include "MACAddress.h"
class MACAddressTable : public cSimpleModule
{
    protected:
        struct RelayEntry {
            unsigned int Vid;
            MACAddress MAC;
            int Gate;
            simtime_t inserted;
        };
        simtime_t agingTime;
        bool verbose;
    public:
        std::vector <RelayEntry> RelayTable;        /// Vid/MAC/Gate/insertedTime

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

    public:
        // Table management

        /**
         * @brief For a known arriving port, V-TAG and destination MAC. It finds out the port where relay component should deliver the message
         * @param MACDest MAC destination
         * @param outputPort Returns the output gate index
         * @return False=NotFound or output=input
         */

        virtual bool resolveMAC(std::vector <int> * outputPorts, MACAddress MACDest, unsigned int Vid=0);


        /**
         * @brief Register a new MAC at RelayTable.
         * @return True if refreshed. False if it is new.
         */
        virtual bool registerMAC(int Gate, MACAddress MAC, unsigned int Vid=0);

        /**
         *  @brief Clears Gate cache
         */
        virtual void flush(int Gate);

        /**
         *  @brief Prints cached data
         */
        virtual void printState();

        /**
         * @brief Copy cache from a to b port
         */
        virtual void cpCache(int a, int b);

        /**
         * @brief Clean aged entries
         */
        virtual void cleanAgedEntries();
};

#endif
