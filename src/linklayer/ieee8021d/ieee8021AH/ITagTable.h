 /**
******************************************************
* @file ITagTable.h
* @brief IComponent module cached data base
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
*
*
******************************************************/
#ifndef __A_ITAGTABLE_H
#define __A_ITAGTABLE_H

#include "MACAddress.h"
#include "8021Q.h"

class ITagTable: public cSimpleModule
{


    protected:
    	simtime_t agingTime;   // max idle time for address table entries
								// (when it expires, entry is removed from the table)

    	struct cmac_time{
    		MACAddress CMAC;
    		simtime_t inserted;
    	};    /// Local CMAC and inserted time


    	struct Local {
    		vid SVid;	/// S-Tag VID
    		std::vector <cmac_time>  cache;
    	};    /// Represents local port associated info. SVid and known clients MAC


    	struct cmac_bmac_time {
    		MACAddress CMAC; /// Other BEBs clients MAC
    		MACAddress BMAC; /// BEBs MAC where those clients where connected.
    		simtime_t inserted;
    	};		///Represents learned info from remote backbone switches. Remote client MAC and backbone MAC associations.

    	struct isidinfo {
    		vid ISid;	/// I-Tag VID
    		std::vector <Local> local;
    		std::vector <cmac_bmac_time>  remote;
    	};	/// Groups remote and local info for a unique ISid

    	typedef std::vector <isidinfo> Isidtable; ///ITagTable information structure.

    public:
    	ITagTable();
    	~ITagTable();

    protected:
    	Isidtable isidtable;
    	virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);


    public:

    	/**
    	 * @brief Saves a new CMAC to an ISid local info or refreshes the insertion time.
    	 * @note Towards backbone.
    	 * @param ISid generating that info.
    	 * @param Gate New entry gate index.
    	 * @param SVid New entry SVid.
    	 * @param CMACs Client MAC coded at source field.
    	 * @return True for a new entry. False if refreshing.
    	 */
        virtual bool registerCMAC (vid ISid, int Gate, vid SVid, MACAddress CMACs);


    	/**
    	 * @brief Is ISID associated to a known SVid/Gate
    	 * @note Towards backbone
    	 * @param Gate Gate index
    	 * @param ISid Return
    	 * @return True if found association
    	 */
        virtual bool checkISid(vid ISid, int Gate, vid SVid);

        /**
         * @brief Gets SVids associated to a Gate.
         */
        virtual std::vector<vid> getSVids(std::vector<vid> ISids, int Gate);

    	/**
    	 * @brief It gets the BMAC where a CMAC/ISid could be reached.
    	 * @note Towards backbone
    	 * @param CMACd Client MAC destination
    	 * @param BMAC Returns the backbone MAC
    	 * @return True if a BMAC has been found.
    	 */
        virtual bool resolveBMAC(vid ISid, MACAddress CMACd, MACAddress * BMACd);



    	/**
    	 * @brief Registers/Refreshes a new remote entry with data from the other side of the BB
    	 * @note Towards client network
    	 * @param CMACs Client MAC source
    	 * @param BMACs Backbone MAC source
    	 * @return False if refreshing. True for a new entry.
    	 */
    	virtual bool registerBMAC (vid ISid, MACAddress CMACs, MACAddress BMACs);

    	/**
    	 * @brief Look up for the Gate assigned to the ISid and CMAC
    	 * @note Towards client network
    	 * @param Gate Returns a vector with the gates index where the CMAC-ISid combination can be found.
    	 * @param SVid Returns a vector of SVids associated to the returned vector of Gates
    	 * @return True if a match found, false if not found.
    	 */
    	virtual bool resolveGate (vid ISid, MACAddress CMAC, std::vector <int> * Gate, std::vector <int> * SVid);


    	/**
    	 * @brief Prints all cached information and some module identification data.
    	 */
        virtual void printState();


        /**
         * @brief Creates a new ISid at isidtable
         * Used during initialization
         */
        virtual bool createISid(vid ISid,unsigned int GateSize);

        /**
         * @brief Adds the asociation ISid/SVid
         * Used during initialization
         */
        virtual bool asociateSVid(vid ISid, unsigned int Gate, vid SVid);
};
#endif
