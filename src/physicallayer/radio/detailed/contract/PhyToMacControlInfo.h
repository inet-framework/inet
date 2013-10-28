#ifndef PHYTOMACCONTROLINFO_H_
#define PHYTOMACCONTROLINFO_H_

#include <omnetpp.h>

#include "INETDefs.h"
#include "Decider.h"

/**
 * @brief Controlinfo for packets which are send from Physical
 * layer to the MAC layer.
 *
 * The ControlInfo contains the the DeciderResult of the Decider.
 * @ingroup phyLayer
 * @ingroup macLayer
 */
class INET_API PhyToMacControlInfo: public cObject {
protected:
	/** The result of the decider evaluation.*/
	DeciderResult * result;

private:
	/** @brief Copy constructor is not allowed.
	 */
	PhyToMacControlInfo(const PhyToMacControlInfo&);
	/** @brief Assignment operator is not allowed.
	 */
	PhyToMacControlInfo& operator=(const PhyToMacControlInfo&);

public:
	/**
	 * @brief Initializes the PhyToMacControlInfo with the passed DeciderResult.
	 *
	 * NOTE: PhyToMacControlInfo takes ownership of the passed DeciderResult!
	 */
	PhyToMacControlInfo(DeciderResult* result):
		result(result) {}

	/**
	 * @brief Clean up the DeciderResult.
	 */
	virtual ~PhyToMacControlInfo() {
		if(result)
			delete result;
	}

	/**
	 * @brief Returns the result of the evaluation of the Decider.
	 */
	DeciderResult* getDeciderResult() const {
		return result;
	}

    /**
     * @brief Attaches a "control info" structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the decider result.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg				The message where the "control info" shall be attached.
     * @param pDeciderResult	The decider results.
     */
    static cObject* setControlInfo(cMessage *const pMsg, DeciderResult *const pDeciderResult) {
    	PhyToMacControlInfo *const cCtrlInfo = new PhyToMacControlInfo(pDeciderResult);
    	pMsg->setControlInfo(cCtrlInfo);

    	return cCtrlInfo;
    }
    /**
     * @brief extracts the decider result from message "control info".
     */
    static DeciderResult* getDeciderResult(cMessage *const pMsg) {
    	return getDeciderResultFromControlInfo(pMsg->getControlInfo());
    }
    /**
     * @brief extracts the decider result from message "control info".
     */
    static DeciderResult* getDeciderResultFromControlInfo(cObject *const pCtrlInfo) {
    	PhyToMacControlInfo *const cCtrlInfo = dynamic_cast<PhyToMacControlInfo *const>(pCtrlInfo);

    	if (cCtrlInfo)
    		return cCtrlInfo->getDeciderResult();
    	return NULL;
    }
};

#endif /*PHYTOMACCONTROLINFO_H_*/
