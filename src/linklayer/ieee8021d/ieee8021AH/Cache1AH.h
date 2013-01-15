 /**
******************************************************
* @file Cache1AH.h
* @brief Cached data base
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2010
******************************************************/

#ifndef __A_CACHE1AH_H
#define __A_CACHE1AH_H

#include "MACAddress.h"
#include "8021Q.h"
#include "Cache1Q.h"


class Cache1AH: public Cache1Q
{
	protected:
    	typedef struct BVidISidMACGate {
    		vid BVid;
    		vid ISid;
    		MACAddress MAC;
    		int Gate;
    		simtime_t inserted;
    	}AhRelayEntry;		/// RelayTable entry
    	cModule* admac;
    public:
    	Cache1AH();
    	virtual ~Cache1AH();
    protected:
    	std::vector <AhRelayEntry> RelayTable;		/// BVid/ISid/BMAC/Gate/insertedTime
    	typedef std::vector <vid> ISIDregister;
    	std::vector <ISIDregister> ISIDregisterTable; /// ISID registration table

    	virtual void initialize();

        // Table management

    public:
    	/**
    	 * @brief For a known arriving port, V-TAG and destination MAC. It finds out the port where relay component should deliver the message
    	 * @param inputPort input gate index
    	 * @param MACDest MAC destination
    	 * @param outputPort Returns the output gate index
    	 * @return False=NotFound or output=input
    	 */
    	virtual bool resolveMAC(vid BVid, vid ISid, MACAddress MACDest, std::vector <int> * outputPorts);


        /**
         * @brief Register a new MAC at RelayTable.
         * @return True if refreshed. False if it is new.
         */
        virtual bool registerMAC(vid BVid, vid ISid, MACAddress MAC, int Gate);


        /**
         *  @brief Prints cached data
         */
        virtual void printState();

        /**
         * @brief Copy cache from port a to port b.
         */
        virtual void cpCache(int a, int b);
        /**
         * @brief Flushing Gate cache. It's entries will be removed.
         */
        virtual void flush(int Gate);

        /**
         * @brief Reads ISID configuration info from ISIDConfig xml.
         */
    	virtual void readconfigfromXML(const cXMLElement* isidConfig);

    	/**
    	 * @brief Checks if the ISid is registered at port.
    	 * @return True if the ISid is registered.
    	 */
    	virtual bool isRegistered(vid ISid, int port);

};
#endif
