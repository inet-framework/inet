/**
******************************************************
* @file IComponent.h
* @brief Part of the I-Component module. 802.1ah-802.1ad conversion
* Basic conversion from IEEE 802.1ad frames to 802.1ah frames.
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2010
******************************************************/
#ifndef __A_ICOMPONENT_H
#define __A_ICOMPONENT_H

#include "MACAddress.h"
#include "ITagTableAccess.h"
#include "EtherFrame_m.h"
#include "8021Q.h"


/**
 * Basic conversion from IEEE 802.1ad frames to IEEE 802.1ah
 */
class IComponent : public cSimpleModule
{
protected:
	ITagTable * it;		//ITagTable pointer
	vid VID;			//defaultVID. Default BVLAN for this IComponent frames.
	vid defaultSVID;	// Default SVid. If outputFrame<2
	vid defaultCVID;	// Default CVid. If outputFrame<1
	vid requestVID;		// Requested CVid if outputFrame=1
	simtime_t interFrameTime;	// number of seconds it waits between MVRP messages.
	int outputFrame; // 0=Ethernet 1=802.1q  2=802.1ad
	bool verbose; // Module verbosity


	typedef std::vector <vid> Vidvector;
	Vidvector activeISid;   //This I-Component ISid.  (I-Component 1:N ISid)
	MACAddress address; ///BEB MAC address.

public:
	IComponent();
	~IComponent();

protected:

	virtual void initialize(int stage);
    virtual int numInitStages() const {return 3;}

    /**
     * @brief Reads itagtable info from configIS xml.
     */
	virtual void readconfigfromXML(const cXMLElement* isidtab);



	/**
	 * @brief Checks msg type and uses it to determine the message direction.
	 * 802.1ad messages are supposed to go towards the BackBone.
	 * 802.1ah messages are supposed to go towards the Client Network.
	 */
	virtual void handleMessage(cMessage *msg);

	/**
	 * @brief Encapsulates 802.1Q frame into a 802.1ad frame and calls handle1adFrame.
	 * It also calls the corresponding learning/solving functions from @see itagtable
	 */
	virtual void handle1QFrame(EthernetIIFrame *frame);

	/**
	 * @brief Encapsulates 802.1ad frame into a 802.1ah
	 * It also calls the corresponding learning/solving functions from @see itagtable
	 */
	virtual void handle1adFrame(EthernetIIFrame *frame);

	/**
	 * @brief Encapsulates 802.1ad frame into a 802.1ah frame.
	 * It also calls the corresponding learning/solving functions from @see itagtable
	 */
	virtual void handleEtherIIFrame(EthernetIIFrame *frame);

	/**
	 * @brief Decapsulates 802.1ah frame into a 802.1ad frame.
	 */
	virtual void handle1AHFrame(EthernetIIFrame *frame);

	/**
	 * @brief sends corresponding MVRPDUs through the client gates.
	 */
	virtual void sendMVRPDUs(cMessage *);

	/**
	 * @brief Checks mac and shows if it is a Backbone Service Instance Group address.
	 */
	virtual bool isISidBroadcast(MACAddress mac, int ISid);

	virtual void finish();

};

#endif

