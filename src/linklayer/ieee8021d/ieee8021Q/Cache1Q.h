 /**
******************************************************
* @file Cache1Q.h
* @brief Cached data base
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Octubre 2010
******************************************************/

#ifndef __A_CACHE1Q_H
#define __A_CACHE1Q_H

#include "8021Q.h"
#include "MACAddress.h"


class Cache1Q: public cSimpleModule
{
	protected:
    	typedef struct VidMACGate {
    		vid Vid;
    		MACAddress MAC;
    		int Gate;
    		simtime_t inserted;
    	}RelayEntry;		/// RelayTable entry
    	simtime_t agingTime;
    	bool verbose;
    public:
    	std::vector <RelayEntry> RelayTable;		/// Vid/MAC/Gate/insertedTime

    public:
    	Cache1Q();
    	~Cache1Q();

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
    	virtual bool resolveMAC(vid Vid,MACAddress MACDest, std::vector <int> * outputPorts);


        /**
         * @brief Register a new MAC at RelayTable.
         * @return True if refreshed. False if it is new.
         */
        virtual bool registerMAC(vid BVid, MACAddress MAC, int Gate);

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
