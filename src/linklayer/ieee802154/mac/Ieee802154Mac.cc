
#include "Ieee802154Mac.h"
#include "InterfaceTableAccess.h"
#include "MACAddress.h"

//#undef EV
//#define EV (ev.isDisabled() || !m_debug) ? std::cout : ev ==> EV is now part of <omnetpp.h>


IE3ADDR Ieee802154Mac::addrCount = 0;

/** Default MAC PIB Attributes */
MAC_PIB Ieee802154Mac::MPIB =
{
    def_macAckWaitDuration,     def_macAssociationPermit,
    def_macAutoRequest,     def_macBattLifeExt,
    def_macBattLifeExtPeriods,  def_macBeaconPayload,
    def_macBeaconPayloadLength, def_macBeaconOrder,
    def_macBeaconTxTime,        0/*def_macBSN*/,
    def_macCoordExtendedAddress,    def_macCoordShortAddress,
    0/*def_macDSN*/,        def_macGTSPermit,
    def_macMaxCSMABackoffs,     def_macMinBE,
    def_macPANId,           def_macPromiscuousMode,
    def_macRxOnWhenIdle,        def_macShortAddress,
    def_macSuperframeOrder,     def_macTransactionPersistenceTime
    /*def_macACLEntryDescriptorSet, def_macACLEntryDescriptorSetSize,
    def_macDefaultSecurity,     def_macACLDefaultSecurityMaterialLength,
    def_macDefaultSecurityMaterial, def_macDefaultSecuritySuite,
    def_macSecurityMode*/
};

Define_Module(Ieee802154Mac);

Ieee802154Mac::Ieee802154Mac()
{
    // aExtendedAddress = addrCount++;
    // buffer
    txPkt           = NULL;
    txBeacon            = NULL;
    txBcnCmdUpper   = NULL;
    txBcnCmd        = NULL;
    txData          = NULL;
    txGTS = NULL;
    txAck           = NULL;
    txCsmaca        = NULL;
    tmpCsmaca       = NULL;
    rxBeacon            = NULL;
    rxData          = NULL;
    rxCmd           = NULL;

    // timer
    backoffTimer        = NULL;
    deferCCATimer   = NULL;
    bcnRxTimer      = NULL;
    bcnTxTimer      = NULL;
    ackTimeoutTimer     = NULL;
    txAckBoundTimer     = NULL;
    txCmdDataBoundTimer = NULL;
    ifsTimer        = NULL;
    txSDTimer       = NULL;
    rxSDTimer       = NULL;
    finalCAPTimer       = NULL;
    gtsTimer            = NULL;

    // link
    hlistBLink1         = NULL;
    hlistBLink2         = NULL;
    hlistDLink1         = NULL;
    hlistDLink2         = NULL;
    //deviceLink1       = NULL;
    //deviceLink2       = NULL;
    //transacLink1      = NULL;
    //transacLink2      = NULL;
}

Ieee802154Mac::~Ieee802154Mac()
{
    cancelAndDelete(backoffTimer);
    cancelAndDelete(deferCCATimer);
    cancelAndDelete(bcnRxTimer);
    cancelAndDelete(bcnTxTimer);
    cancelAndDelete(ackTimeoutTimer);
    cancelAndDelete(txAckBoundTimer);
    cancelAndDelete(txCmdDataBoundTimer);
    cancelAndDelete(ifsTimer);
    cancelAndDelete(txSDTimer);
    cancelAndDelete(rxSDTimer);
    cancelAndDelete(finalCAPTimer);
    cancelAndDelete(gtsTimer);

    emptyHListLink(&hlistBLink1,&hlistBLink2);
    emptyHListLink(&hlistDLink1,&hlistDLink2);
}

void Ieee802154Mac::registerInterface()
{
    int size = sizeof(IE3ADDR);
    aExtendedAddress = 0;
    for (int i=0; i<size; i++)
    {
        if (i<6)
        {
            IE3ADDR aux = macaddress.getAddressByte(5-i);
            aExtendedAddress |=  aux<<(8*i);
        }
    }

    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return;

    InterfaceEntry *e = new InterfaceEntry();

    // interface name: NetworkInterface module's name without special characters ([])
    char *interfaceName = new char[strlen(getParentModule()->getFullName()) + 1];
    char *d = interfaceName;
    for (const char *s = getParentModule()->getFullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    e->setName(interfaceName);
    delete [] interfaceName;

    // address
    e->setMACAddress(macaddress);
    e->setInterfaceToken(macaddress.formInterfaceIdentifier());

    // FIXME: MTU on 802.11 = ?
    e->setMtu(aMaxMACFrameSize);

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    ift->addInterface(e, this);
}

/** Initialization */
void Ieee802154Mac::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV << getParentModule()->getParentModule()->getFullName() << ": initializing Ieee802154Mac, stage=" << stage << endl;

    if (0 == stage)
    {
        const char *addressString = par("address");
        if (!strcmp(addressString, "auto"))
        {
            // assign automatic address
            macaddress = MACAddress::generateAutoAddress();
            // change module parameter from "auto" to concrete address
            par("address").setStringValue(macaddress.str().c_str());
        }
        else
            macaddress.setAddress(addressString);

        registerInterface();

        // get gate ID
        mUppergateIn  = findGate("uppergateIn");
        mUppergateOut = findGate("uppergateOut");
        mLowergateIn  = findGate("lowergateIn");
        mLowergateOut = findGate("lowergateOut");

        // get a pointer to the NotificationBoard module
        mpNb = NotificationBoardAccess().get();
        // subscribe for the information of the carrier sense
        mpNb->subscribe(this, NF_RADIOSTATE_CHANGED);
        //mpNb->subscribe(this, NF_BITRATE_CHANGED);
        mpNb->subscribe(this, NF_RADIO_CHANNEL_CHANGED);

        // obtain pointer to external queue
        initializeQueueModule();

        // initialize MAC PIB attributes
        mpib = MPIB;
        mpib.macBSN = intrand(256);     //Random::random() % 0x100;
        mpib.macDSN = intrand(256);     //Random::random() % 0x100;

        // User-specified MAC parameters
        m_debug                 = par("debug");
        isPANCoor               = par("isPANCoor");
        mpib.macBeaconOrder     = par("BO");
        mpib.macSuperframeOrder = par("SO");
        dataTransMode           = par("dataTransMode");
        panCoorName             = par("panCoorName");
        isRecvGTS               = par("isRecvGTS");
        gtsPayload = par("gtsPayload");
        ack4Gts                 = par("ack4Gts");

        if (mpib.macBeaconOrder >15 || mpib.macSuperframeOrder > mpib.macBeaconOrder)
            error("[MAC]: wrong values for MAC parameter BO or SO");
        else if (mpib.macBeaconOrder == 15)
            error("[MAC]: non-beacon mode (BO = 15) has not been tested!");
        else if (mpib.macSuperframeOrder == mpib.macBeaconOrder)
            error("[MAC]: warning!! the case BO == SO has not been tested!");

        if (dataTransMode != 1 && dataTransMode!= 3)
            error("[MAC]: data transfer mode = %d is not supported in the model!", dataTransMode);

        panStartTime        = 0.0;
        ack4Data        = true;
        secuData        = false;
        secuBeacon      = false;

        taskP.init();

        // for beacon
        rxBO                = 15;
        rxSO                    = 15;
        beaconWaitingTx     = false;
        notAssociated           = true;
        bcnLossCounter = 0;

        // for transmission
        inTransmission          = false;
        waitBcnCmdAck       = false;
        waitBcnCmdUpperAck  = false;
        waitDataAck         = false;
        waitGTSAck = false;
        numBcnCmdRetry      = 0;
        numBcnCmdUpperRetry = 0;
        numDataRetry            = 0;
        numGTSRetry = 0;

        // for CSMA-CA
        csmacaWaitNextBeacon    = false;
        backoffStatus           = 0;

        // for timer
        lastTime_bcnRxTimer = 0;
        inTxSD_txSDTimer        = false;
        inRxSD_rxSDTimer        = false;
        index_gtsTimer = 0;

        // device capability
        capability.alterPANCoor         = false;
        capability.FFD              = true;
        //capability.mainsPower     = false;
        capability.recvOnWhenIdle   = mpib.macRxOnWhenIdle;
        capability.secuCapable      = false;
        capability.alloShortAddr        = true;
        capability.hostName = getParentModule()->getParentModule()->getFullName();

        // GTS variables for PAN coordinator
        gtsCount = 0;
        for (int i=0; i<7; i++)
        {
            gtsList[i].devShortAddr = def_macShortAddress;  // 0xffff
            gtsList[i].startSlot        = 0;
            gtsList[i].length       = 0;
            gtsList[i].isRecvGTS        = false;
            gtsList[i].isTxPending  = false;
        }
        tmp_finalCap = aNumSuperframeSlots - 1; // 15 if no GTS
        indexCurrGts = 99;

        // GTS variables for devices
        gtsLength = 0;
        gtsStartSlot = 0;
        gtsTransDuration = 0;

        /* for indirect transmission
        txPaFields.numShortAddr = 0;
        txPaFields.numExtendedAddr  = 0;
        rxPaFields.numShortAddr = 0;
        rxPaFields.numExtendedAddr = 0;*/

        // pkt counter

        numUpperPkt         = 0;
        numUpperPktLost     = 0;
        numCollision            = 0;
        numLostBcn = 0;
        numTxBcnPkt         = 0;
        numTxDataSucc       = 0;
        numTxDataFail           = 0;
        numTxGTSSucc = 0;
        numTxGTSFail = 0;
        numTxAckPkt         = 0;
        numRxBcnPkt         = 0;
        numRxDataPkt            = 0;
        numRxGTSPkt = 0;
        numRxAckPkt         = 0;

        numTxAckInactive        = 0;

        WATCH(inTxSD_txSDTimer);
        WATCH(inRxSD_rxSDTimer);
        WATCH(numUpperPkt);
        WATCH(numUpperPktLost);
        WATCH(numCollision);
        WATCH(numLostBcn);
        WATCH(numTxBcnPkt);
        WATCH(numTxDataSucc);
        WATCH(numTxDataFail);
        WATCH(numTxGTSSucc);
        WATCH(numTxGTSFail);
        WATCH(numTxAckPkt);
        WATCH(numRxBcnPkt);
        WATCH(numRxDataPkt);
        WATCH(numRxGTSPkt);
        WATCH(numRxAckPkt);

        WATCH(numTxAckInactive);
        radioModule = gate("lowergateOut")->getNextGate()->getOwnerModule()->getId();
    }
    else if (1 == stage)
    {
        // initialize timers
        backoffTimer                = new cMessage("backoffTimer",      MAC_BACKOFF_TIMER);
        deferCCATimer           = new cMessage("deferCCATimer",     MAC_DEFER_CCA_TIMER);
        bcnRxTimer              = new cMessage("bcnRxTimer",        MAC_BCN_RX_TIMER);
        bcnTxTimer              = new cMessage("bcnTxTimer",        MAC_BCN_TX_TIMER);
        ackTimeoutTimer         = new cMessage("ackTimeoutTimer",   MAC_ACK_TIMEOUT_TIMER);
        txAckBoundTimer         = new cMessage("txAckBoundTimer",   MAC_TX_ACK_BOUND_TIMER);
        txCmdDataBoundTimer     = new cMessage("txCmdDataBoundTimer",   MAC_TX_CMD_DATA_BOUND_TIMER);
        ifsTimer                    = new cMessage("ifsTimer",          MAC_IFS_TIMER);
        txSDTimer               = new cMessage("txSDTimer",         MAC_TX_SD_TIMER);
        rxSDTimer               = new cMessage("rxSDTimer",         MAC_RX_SD_TIMER);
        finalCAPTimer               = new cMessage("finalCAPTimer",     MAC_FINAL_CAP_TIMER);
        gtsTimer                    = new cMessage("gtsTimer",      MAC_GTS_TIMER);
        // get initial radio state, channel number, transmit power etc. from Phy layer in this stage
    }
    else if (2 == stage)
    {
        EV << "MAC extended address is: " << aExtendedAddress << endl;
        EV << "mpib.macBSN initialized with: " << (int)mpib.macBSN << endl;
        EV << "mpib.macDSN initialized with: " << (int)mpib.macDSN << endl;
        // start a pan coordinator or a device
        if (isPANCoor)
        {
            // check name
            if (strcmp(panCoorName, getParentModule()->getParentModule()->getFullName()) != 0)
                error("[MAC]: name of PAN coordinator does not match!");
            // chage icon desplayed in Tkenv
            cDisplayString* display_string = &getParentModule()->getParentModule()->getDisplayString();
            display_string->setTagArg("i", 0, "misc/house");

            ev << "**************************** PAN Parameters ****************************" << endl;
            ev << getParentModule()->getParentModule()->getFullName() << " is the PAN coordinator!" << endl;
            ev << "Channel Number: " << ppib.phyCurrentChannel << "; bit rate: " << phy_bitrate /1000 << " kb/s; symbol rate: " << phy_symbolrate/1000 << " ksymbol/s" << endl;
            ev << "BO = " << (int)mpib.macBeaconOrder << ", BI = " << (int)aBaseSuperframeDuration * (1 << mpib.macBeaconOrder)/phy_symbolrate << " s; SO = " << (int)mpib.macSuperframeOrder << ", SD = " << (int)aBaseSuperframeDuration * (1 << mpib.macSuperframeOrder)/phy_symbolrate << " s; duty cycle = " << pow(2, (mpib.macSuperframeOrder-mpib.macBeaconOrder))*100 << "%" << endl;
            ev << "There are " << (int)aBaseSuperframeDuration * (1 << mpib.macSuperframeOrder) << " symbols (" <<  (int)aBaseSuperframeDuration * (1 << mpib.macSuperframeOrder)/aUnitBackoffPeriod << " backoff periods) in CAP" << endl;
            ev << "Length of a unit of backoff period: " << bPeriod << " s" << endl;
            ev << "************************************************************************" << endl;
            cMessage* startPANCoorTimer = new cMessage("startPANCoorTimer", START_PAN_COOR_TIMER);
            scheduleAt(simTime() + panStartTime, startPANCoorTimer);    // timer will be handled after initialization is done, even when panStartTime = 0.
        }
        else    // start a device
            startDevice();
    }
}

void Ieee802154Mac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (par("queueModule").stringValue()[0])
    {
        cModule *module = getParentModule()->getSubmodule(par("queueModule").stringValue());
        queueModule = check_and_cast<IPassiveQueue *>(module);

        EV << "[IFQUEUE]: requesting first one frame from queue module\n";
        queueModule->requestPacket();
    }
}

void Ieee802154Mac::finish()
{
    double t = SIMTIME_DBL(simTime());
    if (t==0) return;
    recordScalar("Total simulation time",               t);
    recordScalar("total num of upper pkts received",        numUpperPkt);
    recordScalar("num of upper pkts dropped",           numUpperPktLost);
    recordScalar("num of BEACON pkts sent",             numTxBcnPkt);
    recordScalar("num of DATA pkts sent successfully",  numTxDataSucc);
    recordScalar("num of DATA pkts failed",             numTxDataFail);
    recordScalar("num of DATA pkts sent successfully in GTS",   numTxGTSSucc);
    recordScalar("num of DATA pkts failed in GTS",          numTxGTSFail);
    recordScalar("num of ACK pkts sent",                numTxAckPkt);
    recordScalar("num of BEACON pkts received",     numRxBcnPkt);
    recordScalar("num of BEACON pkts lost",     numLostBcn);
    recordScalar("num of DATA pkts received",           numRxDataPkt);
    recordScalar("num of DATA pkts received in GTS",            numRxGTSPkt);
    recordScalar("num of ACK pkts received",            numRxAckPkt);
    recordScalar("num of collisions"    ,               numCollision);
}

void Ieee802154Mac::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    switch (category)
    {

        if (check_and_cast<RadioState *>(details)->getRadioId()!=getRadioModuleId())
            return;

    case NF_RADIO_CHANNEL_CHANGED:
        ppib.phyCurrentChannel = check_and_cast<RadioState *>(details)->getChannelNumber();
        phy_bitrate = getRate('b');
        phy_symbolrate = getRate('s');
        bPeriod = aUnitBackoffPeriod / phy_symbolrate;
        break;

        /*case NF_CHANNELS_SUPPORTED_CHANGED:
            ppib.phyChannelsSupported = check_and_cast<Ieee802154RadioState *>(details)->getPhyChannelsSupported();
            break;

        case NF_TRANSMIT_POWER_CHANGED:
            ppib.phyTransmitPower = check_and_cast<Ieee802154RadioState *>(details)->getPhyTransmitPower();
            break;

        case NF_CCA_MODE_CHANGED:
            ppib.phyCCAMode = check_and_cast<Ieee802154RadioState *>(details)->getPhyCCAMode();
            break;*/

    default:
        break;
    }
}

//-------------------------------------------------------------------------------------------------------------/
/*************************** <Start star topology > *****************************/
//------------------------------------------------------------------------------------------------------------/
void Ieee802154Mac::startPANCoor()
{
    mpib.macShortAddress = aExtendedAddress;    // simpley use mac extended address
    mpib.macPANId = aExtendedAddress;       // simpley use mac extended address
    mpib.macAssociationPermit = true;
    txSfSlotDuration = aBaseSlotDuration * (1 << mpib.macSuperframeOrder);
    startBcnTxTimer(true);              // start to transmit my first beacon immediately
}

void Ieee802154Mac::startDevice()
{
    // now only star topology, only PAN coordinator transmits beacon
    mpib.macBeaconOrder     = 15;
    mpib.macSuperframeOrder     = 15;
    mpib.macAssociationPermit   = false;

    // open radio receiver, waiting for first beacon's arrival
    PLME_SET_TRX_STATE_request(phy_RX_ON);
}


//-------------------------------------------------------------------------------/
/*************************** <General Msg Handler> ******************************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::handleMessage(cMessage* msg)
{

    if (msg->getArrivalGateId() == mLowergateIn && dynamic_cast<cPacket*>(msg)==NULL)
    {
        if (msg->getKind()==0)
            error("[MAC]: message '%s' with length==0 is supposed to be a primitive, but msg kind is also zero", msg->getName());
        handleMacPhyPrimitive(msg->getKind(), msg);
        return;
    }

    if (msg->getArrivalGateId() == mLowergateIn)
    {
        handleLowerMsg(msg);
    }
    else if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
    else
    {
        handleUpperMsg(msg);
    }
}

void Ieee802154Mac::handleUpperMsg(cMessage* msg)
{
    UINT_8 index;
    IE3ADDR destAddr;
    bool gtsFound = false;
    numUpperPkt++;

    // MAC layer can process only one data request at a time
    // If no ifq exists, upper layer is not aware of the current state of MAC layer
    // Check if MAC is busy processing one data request, if true, drop it
    // if ifq exists, request another msg from the ifq whenever MAC is idle and ready for next data transmission

    if (taskP.taskStatus(TP_MCPS_DATA_REQUEST))
    {
        EV << "[MAC]: an " << msg->getName() <<" (#" << numUpperPkt << ") received from the upper layer, but drop it due to busy MAC" << endl;
        delete msg;
        numUpperPktLost++;
        reqtMsgFromIFq();
        return;
    }

    //check if parameters valid or not, only check msdu size here
    if (PK(msg)->getByteLength() > aMaxMACFrameSize)
    {
        EV << "[MAC]: an " << msg->getName() <<" (#" << numUpperPkt << ") received from the upper layer, but drop it due to oversize" << endl;
        MCPS_DATA_confirm(mac_INVALID_PARAMETER);
        delete msg;
        numUpperPktLost++;
        reqtMsgFromIFq();
        return;
    }

    Ieee802154NetworkCtrlInfo* control_info = check_and_cast<Ieee802154NetworkCtrlInfo *>(msg->removeControlInfo());

    // translate to MAC address
    if (control_info->getToParent())    // pkt destined for my coordinator
    {
        if (notAssociated)
        {
            EV << "[MAC]: an " << msg->getName() <<" destined for the coordinator received from the upper layer, but drop it due to being not associated with any coordinator yet" << endl;
            delete msg;
            delete control_info;
            numUpperPktLost++;
            reqtMsgFromIFq();
            return;
        }
        else if (dataTransMode == 3)    // GTS mode
        {
            ASSERT(!isPANCoor);
            // check if I have a transmit GTS
            if (gtsStartSlot == 0 || isRecvGTS)
            {
                EV << "[MAC]: an " << msg->getName() <<" requesting GTS transmission destined for the PAN Coordinator received from the upper layer, but drop it due to no transmit GTS allocated by the PAN coordinator yet" << endl;
                delete msg;
                delete control_info;
                numUpperPktLost++;
                reqtMsgFromIFq();
                return;
            }
            else
                destAddr = mpib.macCoordShortAddress;
        }
        else        // other transfer mode
        {
            destAddr = mpib.macCoordShortAddress;
        }
    }
    else        //  pkt destined for device
    {
        if (!simulation.getModuleByPath(control_info->getDestName()))
            error("[MAC]: address conversion fails, destination host does not exist!");
        cModule* module = simulation.getModuleByPath(control_info->getDestName())->getModuleByRelativePath("nic.mac");
        Ieee802154Mac* macModule = check_and_cast<Ieee802154Mac *>(module);
        destAddr = macModule->getMacAddr();

        // check if dest node is in my device list (associated or not)
        if (deviceList.find(destAddr) == deviceList.end())
        {
            EV << "[MAC]: an " << msg->getName() <<" destined for the device with MAC address " << destAddr << " received from the upper layer, but drop it due to no device with this address found in my device list" << endl;
            delete msg;
            delete control_info;
            numUpperPktLost++;
            reqtMsgFromIFq();
            return;
        }

        // if GTS, check in my GTS list if the dest device has a receive GTS
        if (dataTransMode == 3)
        {
            ASSERT(isPANCoor);  // i must be the PAN coordinator
            // check if there is a receive GTS allocated for the dest node in my GTS list
            for (index = 0; index<gtsCount; index++)
            {
                if (gtsList[index].devShortAddr == destAddr && gtsList[index].isRecvGTS)
                {
                    gtsFound = true;
                    break;
                }
            }
            if (gtsFound)
            {
                // set isTxPending true in corresponding GTS descriptor
                gtsList[index].isTxPending = true;
            }
            else
            {
                EV << "[MAC]: an " << msg->getName() <<" requesting GTS transmission destined for the device received from the upper layer, but drop it due to no valid GTS for this device found in my GTS list" << endl;
                delete msg;
                delete control_info;
                numUpperPktLost++;
                reqtMsgFromIFq();
                return;
            }
        }
    }

    EV << "[MAC]: an " << msg->getName() <<" (#" << numUpperPkt << ", " << PK(msg)->getByteLength() << " Bytes, destined for " << control_info->getDestName() << " with MAC address " << destAddr << ", transfer mode " << dataTransMode << ") received from the upper layer" << endl;

    // here always use short address:defFrmCtrl_AddrMode16
    cPacket * pkt = PK(msg);
    MCPS_DATA_request(defFrmCtrl_AddrMode16,    mpib.macPANId,  mpib.macShortAddress,
                      defFrmCtrl_AddrMode16,      mpib.macPANId,  destAddr,
                      pkt,(Ieee802154TxOption)dataTransMode);
    delete control_info;
}

void Ieee802154Mac::handleLowerMsg(cMessage* msg)
{
    bool noAck;
    int i;
    Ieee802154Frame* frame = dynamic_cast<Ieee802154Frame *>(msg);
    if (!frame)
    {
        EV << "[MAC]: message from physical layer (" <<  msg->getClassName() << ")" << msg->getName() << " is not a subclass of Ieee802154Frame, drop it" << endl;
        delete frame;
        return;
    }

    FrameCtrl frmCtrl = frame->getFrmCtrl();
    Ieee802154FrameType frmType = frmCtrl.frmType;

    EV << "[MAC]: an " << frmType << " frame received from the PHY layer, performing filtering now ..." << endl;
    // perform MAC frame filtering
    if (frameFilter(frame))
    {
        EV << "The received frame is filtered, drop frame" << endl;
        delete frame;
        return;
    }

    // check timing for GTS (debug)
    if (frmType == Ieee802154_DATA && frame->getIsGTS())
    {
        if (isPANCoor)
        {
            // check if I'm supposed to receive the data from this device in this GTS
            if (indexCurrGts == 99 || gtsList[indexCurrGts].isRecvGTS || gtsList[indexCurrGts].devShortAddr != frame->getSrcAddr())
                error("[GTS]: timing error, PAN coordinator is not supposed to receive this DATA pkt at this time!");
        }
        else
        {
            if (index_gtsTimer != 99 || !isRecvGTS || frame->getSrcAddr() != mpib.macCoordShortAddress)
                error("[GTS]: timing error, the device is not supposed to receive this DATA pkt at this time!");
        }
    }

    EV << "[MAC]: checking if the received frame requires an ACK" << endl;
    //send an acknowledgement if needed (no matter this is a duplicated packet or not)
    if ((frmType == Ieee802154_DATA) || (frmType == Ieee802154_CMD))
    {
        if (frmCtrl.ackReq) //acknowledgement required
        {
            /*
            //association request command will be ignored under following cases
            if (frmType == Ieee802154_CMD && check_and_cast<Ieee802154CmdFrame *>(frame)->getCmdType() == Ieee802154_ASSOCIATION_REQUEST)
            if ((!capability.FFD)           //not an FFD
             || (mpib.macShortAddress == 0xffff)    //not yet joined any PAN
             || (!macAssociationPermit))        //association not permitted
            {
                delete frame;
                return;
            }*/

            noAck = false;
            // MAC layer can process only one command (rx or tx) at a time
            if (frmType == Ieee802154_CMD)
                if ((rxCmd)||(txBcnCmd))
                    noAck = true;
            if (!noAck)
            {
                EV << "[MAC]: yes, constructing the ACK frame" << endl;
                constructACK(frame);
                //stop CSMA-CA if it is pending (it will be restored after the transmission of ACK)
                if (backoffStatus == 99)
                {
                    EV << "[MAC]: CSMA-CA is pending, stop it, it will resume after sending ACK" << endl;
                    backoffStatus = 0;
                    csmacaCancel();
                }
                EV << "[MAC]: prepare to send the ACK, ask PHY layer to turn on the transmitter first" << endl;
                PLME_SET_TRX_STATE_request(phy_TX_ON);
            }
        }
        else        // no ACK required
        {
            if (frame->getIsGTS())  // received in GTS
            {
                if (isPANCoor)
                {
                    // the device may transmit more pkts in this GTS, turn on radio
                    PLME_SET_TRX_STATE_request(phy_RX_ON);
                }
                else
                {
                    // PAN coordinator can transmit only one pkt to me in my GTS, turn off radio now
                    PLME_SET_TRX_STATE_request(phy_TRX_OFF);
                }
            }
            else
                resetTRX();
        }
    }

    // drop new received cmd pkt if mac is current processing a cmd
    if (frmType == Ieee802154_CMD)
        if ((rxCmd)||(txBcnCmd))
        {
            EV << "[MAC]: the received CMD frame is dropped, because MAC is currently processing a MAC CMD" << endl;
            delete frame;
            return;
        }

    // drop new received data pkt if mac is current processing last received data pkt
    if (frmType == Ieee802154_DATA)
        if (rxData)
        {
            EV << "[MAC]: the received DATA frame is dropped, because MAC is currently processing the last received DATA frame" << endl;
            delete frame;
            return;
        }

    EV << "[MAC]: checking duplication ..." << endl;
    //check duplication -- must be performed AFTER all drop's
    if (frmType == Ieee802154_BEACON)
        i = chkAddUpdHListLink(&hlistBLink1, &hlistBLink2, frame->getSrcAddr(), frame->getBdsn());
    else if (frmType != Ieee802154_ACK) // data or cmd
        i = chkAddUpdHListLink(&hlistDLink1, &hlistDLink2, frame->getSrcAddr(), frame->getBdsn());
    else    // ACK
    {
        // check ACK in <handleAck()>
    }
    if (i == 2) // duplication found in the HListLink
    {
        EV << "[MAC]: duplication detected, drop frame" << endl;
        delete frame;
        return;
    }

    switch (frmType)
    {
    case Ieee802154_BEACON:
        EV << "[MAC]: continue to process received BEACON pkt" << endl;
        handleBeacon(frame);
        break;

    case Ieee802154_DATA:
        EV << "[MAC]: continue to process received DATA pkt" << endl;
        handleData(frame);
        break;

    case Ieee802154_ACK:
        EV << "[MAC]: continue to process received ACK pkt" << endl;
        handleAck(frame);
        break;

    case Ieee802154_CMD:
        EV << "[MAC]: continue to process received CMD pkt" << endl;
        handleCommand(frame);
        break;

    default:
        error("[MAC]: undefined MAC frame type: %d", frmType);
    }
}

void Ieee802154Mac::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind())
    {
    case START_PAN_COOR_TIMER:
        startPANCoor();
        delete msg;     // it's a dynamic timer
        break;

    case MAC_BACKOFF_TIMER:
        handleBackoffTimer();
        break;

    case MAC_DEFER_CCA_TIMER:
        handleDeferCCATimer();
        break;

    case MAC_BCN_RX_TIMER:
        handleBcnRxTimer();
        break;

    case MAC_BCN_TX_TIMER:
        handleBcnTxTimer();
        break;

    case MAC_ACK_TIMEOUT_TIMER:
        handleAckTimeoutTimer();
        break;

    case MAC_TX_ACK_BOUND_TIMER:
        handleTxAckBoundTimer();
        break;

    case MAC_TX_CMD_DATA_BOUND_TIMER:
        handleTxCmdDataBoundTimer();
        break;

    case MAC_IFS_TIMER:
        handleIfsTimer();
        break;

    case MAC_TX_SD_TIMER:
    case MAC_RX_SD_TIMER:
        handleSDTimer();
        break;

    case MAC_FINAL_CAP_TIMER:
        handleFinalCapTimer();
        break;

    case MAC_GTS_TIMER:
        handleGtsTimer();
        break;

    default:
        error("[MAC]: unknown MAC timer type!");
    }
}

void Ieee802154Mac::sendDown(Ieee802154Frame* frame)
{
    /*if (updateNFailLink(fl_oper_est,index_) == 0)
    {
        if (txBeacon)
        {
            beaconWaiting = false;
            Packet::free(txBeacon);
            txBeacon = 0;
        }
        return;
    }
    else if (updateLFailLink(fl_oper_est,index_,p802_15_4macDA(p)) == 0)
    {
        dispatch(p_UNDEFINED,"PD_DATA_confirm");
        return;
    }*/

    // TBD: energy model
    inTransmission = true;              // cleared by PD_DATA_confirm
    send(frame, mLowergateOut);     // send a duplication
    EV << "[MAC]: sending frame " << frame->getName() << " (" << frame->getByteLength() << " Bytes) to PHY layer" << endl;
    EV << "[MAC]: the estimated transmission time is " << calDuration(frame) << " s" << endl;
}

void Ieee802154Mac::reqtMsgFromIFq()
{
    if (queueModule)
    {
        // tell queue module that we've become idle
        EV << "[MAC]: requesting another frame from queue module" << endl;
        queueModule->requestPacket();
    }
}

//-------------------------------------------------------------------------------/
/************************ <MAC Frame Reception Handler> *************************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::handleBeacon(Ieee802154Frame* frame)
{
    EV << "[MAC]: starting processing received Beacon frame" << endl;
    Ieee802154BeaconFrame *bcnFrame = check_and_cast<Ieee802154BeaconFrame *>(frame);
    FrameCtrl frmCtrl;
    bool pending;
    simtime_t now,  tmpf, w_time,duration;
    UINT_8 ifs;
    int  dataFrmLength;
    now = simTime();

    frmCtrl = frame->getFrmCtrl();
    //update beacon parameters
    rxSfSpec = bcnFrame->getSfSpec();
    rxBO = rxSfSpec.BO;
    rxSO = rxSfSpec.SO;
    rxSfSlotDuration = aBaseSlotDuration * (1 << rxSO);

    //calculate the time when the first bit of the beacon was received
    duration = calDuration(frame);
    bcnRxTime = now - duration;
    schedBcnRxTime = bcnRxTime;     // important: this value is calculated in <csmacaStart()>, if later on a CSMA-CA is pending for this bcn and backoff will resume without calling <csmacaStart()> (see <csmacaTrxBeacon()>) , therefore this value will not be updated, but  <csmacaCanProceed()> and other functions will use it and needs to be updated here
    EV << "The first bit of this beacon was received by PHY layer at " << bcnRxTime << endl;

    //calculate <rxBcnDuration>
    if (bcnFrame->getByteLength() <= aMaxSIFSFrameSize)
        ifs = aMinSIFSPeriod;
    else
        ifs = aMinLIFSPeriod;
    tmpf = duration * phy_symbolrate;
    tmpf += ifs;
    rxBcnDuration = (UINT_8)(SIMTIME_DBL(tmpf) / aUnitBackoffPeriod);
    if (fmod(tmpf, aUnitBackoffPeriod) > 0.0)
        rxBcnDuration++;

    // TBD: store GTS fields
    //gtsFields = bcnFrame->getGtsFields();

    //update PAN descriptor
    rxPanDescriptor.CoordAddrMode       = frmCtrl.srcAddrMode;
    rxPanDescriptor.CoordPANId      = frame->getSrcPanId();
    rxPanDescriptor.CoordAddress_16_or_64   = frame->getSrcAddr();
    rxPanDescriptor.LogicalChannel      = ppib.phyCurrentChannel;
    // rxPanDescriptor.SuperframeSpec           // ignored, store in rxSfSpec above
    //rxPanDescriptor.GTSPermit     = gtsFields.permit;
    // rxPanDescriptor.LinkQuality              // TBD link quality at PHY layer
    // rxPanDescriptor.TimeStamp                    // ignored, store in bcnRxTime above
    // rxPanDescriptor.SecurityUse              // security TBD
    // rxPanDescriptor.ACLEntry                     // security TBD
    // rxPanDescriptor.SecurityFailure      // security TBD

    // start rxSDTimer
    startRxSDTimer();

    // reset lost beacon counter
    bcnLossCounter = 0;

    // temporary solution for association process, to be modified in later version
    if (notAssociated) // this is my first rxed beacon, associate with this one,
    {
        ASSERT(mpib.macCoordShortAddress == def_macCoordShortAddress);
        ASSERT(mpib.macPANId == def_macPANId);
        notAssociated = false;

        mpib.macPANId = frame->getSrcPanId();           // store PAN id
        mpib.macCoordShortAddress = frame->getSrcAddr();    // store coordinator address, always use short address
        mpib.macCoordExtendedAddress = frame->getSrcAddr(); // PAN coordinator uses the same address for both its own 16 and 64 bit address
        //mpib.macShortAddress = aExtendedAddress;  // set my short address the same as the extended address by myself, instead from an association response, TBD

        EV << "This is my first beacon, associate with it" << endl;
        cModule* module = simulation.getModuleByPath(panCoorName)->getModuleByRelativePath("nic.mac");
        Ieee802154Mac* macModule = check_and_cast<Ieee802154Mac *>(module);
        mpib.macShortAddress = macModule->associate_request_cmd(aExtendedAddress, capability);

        startBcnRxTimer();              // start tracking beacon, always on
        // start sending beacon from here, if I want to be a coordinator
        /*
        if ((mpib.macBeaconOrder != 15) && capability.FFD)
        {
            // actually started by mlme-start.request
            startBcnTxTimer(true, simtime_t startTime);     // TBD
        }*/

        // if GTS, calculate the required GTS length for transmitting pkts with constant length
        if (dataTransMode == 3)
        {
            // calculate the length of data frame transmitted in GTS
            // MHR(11) + gtsPayload + MFR(2)
            dataFrmLength = 11 + gtsPayload + 2;
            if (dataFrmLength <= aMaxSIFSFrameSize)
                ifs = aMinSIFSPeriod;
            else
                ifs = aMinLIFSPeriod;

            // calculate duration of the entire data transaction
            duration = (def_phyHeaderLength + dataFrmLength)*8/phy_bitrate;
            if (ack4Gts)
                duration += (mpib.macAckWaitDuration + ifs)/phy_symbolrate;
            else                    // no ACK required
                duration += (aTurnaroundTime + ifs)/phy_symbolrate;
            // store duration value for later evaluation in gtsCanProceed()
            gtsTransDuration = duration;

            // duration of one superframe slot (one GTS slot)
            tmpf = rxSfSlotDuration/phy_symbolrate;
            if (duration < tmpf)
                gtsLength = 1;
            else
            {
                gtsLength = (UINT_8)(duration/tmpf);
                if (fmod(duration, tmpf) > 0.0)
                    gtsLength++;
            }
            EV << "[GTS]: gtsTransDuration = " << gtsTransDuration << " s, duration of one GTS slot = " << tmpf << " s" << endl;

            // call gts_request_cmd() at the PAN coordinator to apply for GTS
            EV << "[GTS]: request " << (int)gtsLength << " GTS slots from the PAN coordinator" << endl;
            gtsStartSlot = macModule->gts_request_cmd(mpib.macShortAddress, gtsLength, isRecvGTS);
            if (gtsStartSlot != 0)      // successfully
                EV << "[GTS]: my GTS start slot is " << (int)gtsStartSlot << endl;
            else        // failed
            {
                // TBD: what to do if failed
                // EV << "[GTS]: request for GTS failed" << endl;
                error("[GTS]: request for GTS failed!");
            }
        }
    }
    // can start my GTS timer only after receiving the second beacon
    else if (gtsStartSlot != 0)
    {
        tmpf = bcnRxTime + gtsStartSlot * rxSfSlotDuration / phy_symbolrate;
        w_time = tmpf - now;
        // should turn on radio receiver aTurnaroundTime symbols berfore GTS starts, if I have a receive GTS
        if (isRecvGTS)
            w_time = w_time - aTurnaroundTime / phy_symbolrate;
        EV << "[GTS]: schedule for my GTS with start slot #" << (int)gtsStartSlot << endl;
        startGtsTimer(w_time);

        // if my GTS is not the first one in the CFP, should turn radio off at the end of CAP using finalCAPTimer
        if (gtsStartSlot != rxSfSpec.finalCap + 1)
        {
            ASSERT(gtsStartSlot > rxSfSpec.finalCap);
            tmpf = bcnRxTime + (rxSfSpec.finalCap + 1) * rxSfSlotDuration / phy_symbolrate;
            w_time = tmpf - now;
            EV << "[GTS]: my GTS is not the first one in the CFP, schedule a timer to turn off radio at the end of CAP" << endl;
            startFinalCapTimer(w_time);
        }
    }

    dispatch(phy_SUCCESS,__FUNCTION__);

    //CSMA-CA may be waiting for the new beacon
    if (backoffStatus == 99)
        csmacaTrxBeacon('r');

    // TBD process pengding address
    /*
    #ifdef test_802154_INDIRECT_TRANS
    rxPaFields = bcnFrame->getPaFields();       //store pending address fields
    if (mpib.macAutoRequest)
    {
        //handle the pending packet
        pending = false;
        for (i=0;i<rxPaFields.numShortAddr;i++)
        {
            if (rxPaFields.addrList[i] == mpib.macShortAddress)
            {
                pending = true;
                break;
            }
        }
        if (!pending)
        for (i=0;i<rxPaFields.numExtendedAddr;i++)
        {
            if (rxPaFields.addrList[rxPaFields.numShortAddr + i] == aExtendedAddress)
            {
                pending = true;
                break;
            }
        }

        if (pending)    // mlme_poll_request TBD
            mlme_poll_request(frmCtrl.srcAddrMode,wph->MHR_SrcAddrInfo.panID,wph->MHR_SrcAddrInfo.addr_64,capability.secuCapable,true,true);
    }
    #endif
    */
    resetTRX();
    delete bcnFrame;
    numRxBcnPkt++;
}

void Ieee802154Mac::handleData(Ieee802154Frame* frame)
{
    FrameCtrl frmCtrl = frame->getFrmCtrl();
    bool isSIFS = false;

    //pass the data packet to upper layer
    //(we need some time to process the packet -- so delay SIFS/LIFS symbols from now or after finishing sending the ack.)
    //(refer to Figure 60 for details of SIFS/LIFS)
    ASSERT(rxData == NULL);
    rxData = frame;
    //rxDataTime = simTime();
    if (!frmCtrl.ackReq)
    {
        if (frame->getByteLength() <= aMaxSIFSFrameSize)
            isSIFS = true;
        startIfsTimer(isSIFS);
    }
    //else  //schedule and dispatch after finishing ack. transmission
}

void Ieee802154Mac::handleAck(Ieee802154Frame* frame)
{


    if ((txBcnCmd==NULL)&&(txBcnCmdUpper==NULL)&&(txData==NULL)&&(txGTS==NULL))
    {
        EV << "[MAC]: no pending transmission task is waiting for this ACK, drop it!" << endl;
        delete frame;
        return;
    }

    if ((txPkt != txBcnCmd) && (txPkt != txBcnCmdUpper) && (txPkt != txData) && (txPkt != txGTS))
        // ack received after corresponding task has failed duo to reaching max retries
    {
        EV << "[MAC]: this is a late ACK received after corresponding task has failed, drop it!" << endl;
        delete frame;
        return;
    }

    //check the sequence number in the ACK. to see if it matches that in the <txPkt>
    if (frame->getBdsn() != check_and_cast<Ieee802154Frame *>(txPkt)->getBdsn())
    {
        EV << "[MAC]: the SN in the ACK does not match, drop it!" << endl;
        delete frame;
        return;
    }

    if (ackTimeoutTimer->isScheduled())         // ACK arrives before ACK timeout expires
    {
        numRxAckPkt++;
        EV << "[MAC]: the ACK arrives before timeout, cancel timeout timer" << endl;
        cancelEvent(ackTimeoutTimer);
        // reset retry counter
        if (txPkt == txBcnCmd)
            numBcnCmdRetry = 0;
        else if (txPkt == txBcnCmdUpper)
            numBcnCmdUpperRetry = 0;
        else if (txPkt == txData)
            numDataRetry = 0;
        else if (txPkt == txGTS)
            numGTSRetry = 0;
    }
    else
    {
        //only handle late ack. for data packet not in GTS
        if (txPkt != txData)
        {
            EV << "[MAC]: this is a late ACK, but not for a DATA pkt, drop it!" << endl;
            delete frame;
            return;
        }

        if (backoffStatus == 99)
        {
            EV << "[MAC]: this is a late ACK for " << txData->getName() << ":#" << (int)txData->getBdsn() << ", stop retrying" << endl;
            backoffStatus = 0;
            csmacaCancel();
        }
    }

    // TBD
    /*
    // If ack is for data request cmd (through data poll primitive), set pending flag for data polling
    if (txPkt == txBcnCmdUpper)
    if ((taskP.taskStatus(TP_mlme_poll_request))
     && (strcmp(taskP.taskFrFunc(TP_mlme_poll_request),__FUNCTION__) == 0))
    {
        frmCtrl = frame->getFrmCtrl();
        taskP.mlme_poll_request_pending = frmCtrl.frmPending;   // indicating whether a data is pending at the coordinator for me
    }*/

    dispatch(phy_SUCCESS,__FUNCTION__);

    delete frame;
}

void Ieee802154Mac::handleCommand(Ieee802154Frame* frame)
{
    bool ackReq = false;        // flag indicating if cmd needs to be released at the end of this function

    Ieee802154CmdFrame* tmpCmd = check_and_cast<Ieee802154CmdFrame *>(frame);
    //FrameCtrl frmCtrl = tmpCmd->getFrmCtrl();

    // all cmd pkts requiring ACK are put in rxCmd and
    // will be handled by <handleIfsTimer()> after the transmission of ACK and delay of ifs
    // other cmd pkts are processed here
    switch (tmpCmd->getCmdType())
    {
    case Ieee802154_ASSOCIATION_REQUEST:    //Association request
        ASSERT(rxCmd == NULL);
        rxCmd = frame;
        ackReq = true;
        break;

    case Ieee802154_ASSOCIATION_RESPONSE:   //Association respons
        // TBD
        /*ASSERT(rxCmd == NULL);
        rxCmd = frame;
        ackReq = true;
        wph = HDR_LRWPAN(p);
        rt_myNodeID = *((UINT_16 *)wph->MSDU_Payload);
        #ifdef ZigBeeIF
        sscs->setGetClusTreePara('g',p);
        #endif*/
        break;

    case Ieee802154_DISASSOCIATION_NOTIFICATION:    //Disassociation notification
        // TBD
        break;

    case Ieee802154_DATA_REQUEST:   //Data request
        ASSERT(rxCmd == NULL);
        rxCmd = frame;
        ackReq = true;
        break;

    case Ieee802154_PANID_CONFLICT_NOTIFICATION:    //PAN ID conflict notification
        // TBD
        break;

    case Ieee802154_ORPHAN_NOTIFICATION:    //Orphan notification
        // TBD
        /*wph = HDR_LRWPAN(p);
        sscs->MLME_ORPHAN_indication(wph->MHR_SrcAddrInfo.addr_64,false,0);*/
        break;

    case Ieee802154_BEACON_REQUEST: //Beacon request
        /*if (capability.FFD                        //I am an FFD
         && (mpib.macAssociationPermit)                 //association permitted
         && (mpib.macShortAddress != 0xffff)                //allow to send beacons
         && (mpib.macBeaconOrder == 15))                //non-beacon enabled mode
        {
            //send a beacon using unslotted CSMA-CA
            ASSERT(rxCmd == NULL);
            txBcnCmd = Packet::alloc();
            if (!txBcnCmd) break;
            wph = HDR_LRWPAN(txBcnCmd);
            frmCtrl.FrmCtrl = 0;
            frmCtrl.setFrmType(defFrmCtrl_Type_Beacon);
            frmCtrl.setSecu(secuBeacon);
            frmCtrl.setFrmPending(false);
            frmCtrl.setAckReq(false);
            frmCtrl.setDstAddrMode(defFrmCtrl_AddrModeNone);
            if (mpib.macShortAddress == 0xfffe)
            {
                frmCtrl.setSrcAddrMode(defFrmCtrl_AddrMode64);
                wph->MHR_SrcAddrInfo.panID = mpib.macPANId;
                wph->MHR_SrcAddrInfo.addr_64 = aExtendedAddress;
            }
            else
            {
                frmCtrl.setSrcAddrMode(defFrmCtrl_AddrMode16);
                wph->MHR_SrcAddrInfo.panID = mpib.macPANId;
                wph->MHR_SrcAddrInfo.addr_16 = mpib.macShortAddress;
            }
            sfSpec.SuperSpec = 0;
            sfSpec.setBO(15);
            sfSpec.setBLE(mpib.macBattLifeExt);
            sfSpec.setPANCoor(isPANCoor);
            sfSpec.setAssoPmt(mpib.macAssociationPermit);
            wph->MSDU_GTSFields.spec = 0;
            wph->MSDU_PendAddrFields.spec = 0;
            wph->MSDU_PayloadLen = 0;
        #ifdef ZigBeeIF
            sscs->setGetClusTreePara('s',txBcnCmd);
        #endif
            constructMPDU(4,txBcnCmd,frmCtrl.FrmCtrl,mpib.macBSN++,wph->MHR_DstAddrInfo,wph->MHR_SrcAddrInfo,sfSpec.SuperSpec,0,0);
            hdr_dst((char *)HDR_MAC(txBcnCmd),p802_15_4macSA(p));
            hdr_src((char *)HDR_MAC(txBcnCmd),index_);
            HDR_CMN(txBcnCmd)->ptype() = PT_MAC;
            //for trace
            HDR_CMN(txBcnCmd)->next_hop_ = p802_15_4macDA(txBcnCmd);        //nam needs the nex_hop information
            p802_15_4hdrBeacon(txBcnCmd);
            csmacaBegin('c');
        }*/
        break;

    case Ieee802154_COORDINATOR_REALIGNMENT:    //Coordinator realignment
        // TBD
        /*wph = HDR_LRWPAN(p);
        frmCtrl.FrmCtrl = wph->MHR_FrmCtrl;
        frmCtrl.parse();
        if (frmCtrl.dstAddrMode == defFrmCtrl_AddrMode64)       //directed to an orphan device
        {
            //recv() is in charge of sending ack.
            //further handling continues after the transmission of ack.
            assert(rxCmd == 0);
            rxCmd = p;
            ackReq = true;
        }
        else                                //broadcasted realignment command
        if ((wph->MHR_SrcAddrInfo.addr_64 == macCoordExtendedAddress)
        && (wph->MHR_SrcAddrInfo.panID == mpib.macPANId))
        {
            //no specification in the draft as how to handle this packet, so use our discretion
            mpib.macPANId = *((UINT_16 *)wph->MSDU_Payload);
            mpib.macCoordShortAddress = *((UINT_16 *)(wph->MSDU_Payload + 2));
            tmp_ppib.phyCurrentChannel = wph->MSDU_Payload[4];
            phy->PLME_SET_request(phyCurrentChannel,&tmp_ppib);
        }*/
        break;

    case Ieee802154_GTS_REQUEST:                //GTS request
        // TBD
        break;

    default:
        error("Undefined MAC command type (frmType=%d)");
        break;
    }

    if (!ackReq)        // all cmds requiring no ACK sink here
        delete frame;
    // other cmds will be released by <handleIfsTimer()>
}

bool Ieee802154Mac::frameFilter(Ieee802154Frame* frame)
// perform AMC frame filtering, return true if frame is filtered
{
    FrameCtrl frmCtrl = frame->getFrmCtrl();
    Ieee802154FrameType frmType = frmCtrl.frmType;
    /*
    Ieee802154MacCmdType cmdType;
    if (frmType == Ieee802154_CMD)
        cmdType = check_and_cast<Ieee802154CmdFrame *>(frame)->getCmdType();
    */

    // Fisrt check flag set by Phy layer, COLLISION or
    if (frame->getKind() == COLLISION)
    {
        EV << "[MAC]: frame corrupted due to collision, dropped" << endl;
        numCollision++;
        return true;
    }
    else if  (frame->getKind() == RX_DURING_CCA)
    {
        EV << "[MAC]: frame corrupted due to being received during CCA, dropped" << endl;
        return true;
    }
    /*
    // TBD: check if received during channel scanning
    if (taskP.taskStatus(TP_mlme_scan_request))
    {
        if (taskP.mlme_scan_request_ScanType == ED_SCAN)                            //ED scan, drop all received pkts
        {
            return true;
        }
        else if (((taskP.mlme_scan_request_ScanType == ACTIVE_SCAN)     //Active scan or Passive scan, drop all except for beacon
                ||(taskP.mlme_scan_request_ScanType == PASSIVE_SCAN))
             && (frmType != Ieee802154_BEACON))
        {
            return true;
        }
        else if ((taskP.mlme_scan_request_ScanType == ORPHAN_SCAN)      //Orphan scan, drop all except for coordinator realignment cmd
             && ((frmType != Ieee802154_CMD)||(cmdType != Ieee802154_COORDINATOR_REALIGNMENT)))
        {
            return true;
        }
    }*/

    //perform further filtering only if the PAN is currently not in promiscuous mode
    if (!mpib.macPromiscuousMode)
    {
        //check packet type
        if ((frmType != Ieee802154_BEACON)
                &&(frmType != Ieee802154_DATA)
                &&(frmType != Ieee802154_ACK)
                &&(frmType != Ieee802154_CMD))
        {
            return true;
        }

        //check source PAN ID for beacon frame
        if ((frmType == Ieee802154_BEACON)
                &&(mpib.macPANId != 0xffff)       // associated
                &&(frame->getSrcPanId() != mpib.macPANId)) // PAN id not match
        {
            return true;
        }

        //check dest. PAN ID (beacon has no dest address fields)
        if ((frmCtrl.dstAddrMode == defFrmCtrl_AddrMode16)
                ||(frmCtrl.dstAddrMode == defFrmCtrl_AddrMode64))
            if ((frame->getDstPanId() != 0xffff)                        // PAN id do not match for other pkts
                    &&(frame->getDstPanId() != mpib.macPANId))
            {
                return true;
            }

        //check dest. address
        if (frmCtrl.dstAddrMode == defFrmCtrl_AddrMode16)   // short address
        {
            if ((frame->getDstAddr() != 0xffff)
                    && (frame->getDstAddr() != mpib.macShortAddress))
            {
                return true;
            }
        }
        else if (frmCtrl.dstAddrMode == defFrmCtrl_AddrMode64)      // extended address
        {
            if (frame->getDstAddr() != aExtendedAddress)
            {
                return true;
            }
        }

        //*****************************************************************************
        //***************************Important!!!!!!!!***********************************
        //*****************************************************************************
        // the ACk pkt , which is not destined for me, should be filtered here
        // According to spec, ACK pkts have no addressing fields (no src and dst address info)
        // it should be checked when received only by its sn (bdsn)
        // but for convenience, we add a hidden dsr address to ACK when constructing it
        // it's called hidden, because its dstAddrMode is still set to 0
        if (frmType == Ieee802154_ACK && frame->getDstAddr() != aExtendedAddress)
            // if dsr addr does not match
            return true;

        //check for Data/Cmd frame only with source address:: destined for PAN coordinator
        // temporary solution, consider only star topology
        if ((frmType == Ieee802154_DATA) || (frmType == Ieee802154_CMD))
            if (frmCtrl.dstAddrMode == defFrmCtrl_AddrModeNone)     // dest address fileds not included
            {
                if (!isPANCoor)     // not a PAN coordinator
                    return true;
                /*
                if (((!capability.FFD)||(numberDeviceLink(&deviceLink1) == 0))  //I am not a coordinator (nor a PAN coordinator)
                  ||(wph->MHR_SrcAddrInfo.panID != mpib.macPANId))
                {
                    return true;
                }*/
            }
    }
    return false;
}

void Ieee802154Mac::constructACK(Ieee802154Frame* rxFrame)
// called by <handleLowerMsg()> if the received pkt requires an ACK
// this function constructs an ACK and puts it in txAck
{
    int i;
    FrameCtrl origFrmCtrl, ackFrmCtrl;
    origFrmCtrl = rxFrame->getFrmCtrl();

    /*#ifdef test_802154_INDIRECT_TRANS
    //if it is a data request command, then need to check if there is any packet pending.
    //In implementation, we may not have enough time to check if packets pending. If this is the case,
    //then the pending flag in the ack. should be set to 1, and then send a zero-length data packet
    //if later it turns out there is no packet actually pending.
    //In simulation, we assume having enough time to determine the pending status -- so zero-length packet will never be sent.
    //(refer to page 155, line 46-50)
    if ((origFrmCtrl.frmType == Ieee802154_CMD)     //command packet
    && (check_and_cast<Ieee802154MacCmdType *>(rxFrame)->getCmdType() == Ieee802154_DATA_REQUEST))      //data request command
    {
        i = updateTransacLink(tr_oper_est, &transacLink1, &transacLink2, origFrmCtrl.srcAddrMode, rxFrame->getSrcAddr());
        // if pengind pkt found in transaction list, return 0
    }
    else
    #endif*/
    i = 1;

    // construct frame control field
    ackFrmCtrl.frmType          = Ieee802154_ACK;
    ackFrmCtrl.secu                 = false;
    ackFrmCtrl.frmPending           = (i==0)?true:false;
    ackFrmCtrl.ackReq               = false;
    ackFrmCtrl.intraPan             = origFrmCtrl.intraPan;         // copy from original frame
    ackFrmCtrl.dstAddrMode      = defFrmCtrl_AddrModeNone;  // dst address fields empty
    ackFrmCtrl.srcAddrMode      = defFrmCtrl_AddrModeNone;  // src address fields empty

    Ieee802154AckFrame* tmpAck = new Ieee802154AckFrame();
    tmpAck->setName("Ieee802154ACK");
    // for the convenience of filtering ACK pkts in <frameFilter>
    // and locating backoff boundary before txing ACK in <handle_PLME_SET_TRX_STATE_confirm>
    // we still set a hidden dsr addr (it's hidden, because dstAddrMode has been set to 0 above)
    tmpAck->setDstAddr(rxFrame->getSrcAddr());              // TO CHECK: if src addr alway exists

    tmpAck->setFrmCtrl(ackFrmCtrl);
    tmpAck->setBdsn(rxFrame->getBdsn());                                // copy from original frame
    tmpAck->setByteLength(calFrmByteLength(tmpAck));    // constant size for ACK

    if (rxFrame->getIsGTS())
        tmpAck->setIsGTS(true);

    ASSERT(!txAck);     //it's impossilbe to receive the second packet before the Ack has been sent out.
    txAck = tmpAck;
}

//-------------------------------------------------------------------------------/
/***************************** <Radio State Control> ****************************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::resetTRX()
{
    PHYenum t_state;
    EV << "[MAC]: reset radio state after a task has completed" << endl;
    if ((mpib.macBeaconOrder != 15)||(rxBO != 15))  //beacon enabled
    {
        if ((!inTxSD_txSDTimer) && (!inRxSD_rxSDTimer))     // in inactive portion, go to sleep
        {
            EV << "[MAC]: it's now in inactive period, should turn off radio and go to sleep" << endl;
            t_state = phy_TRX_OFF;
        }
        else if (inTxSD_txSDTimer)      // should not go to sleep
        {
            EV << "[MAC]: it's now in outgoing active period (as a coordinator), should stay awake and turn on receiver" << endl;
            t_state = phy_RX_ON;
        }
        else                                                    // in rx SD, according to macRxOnWhenIdle
        {
            EV << "[MAC]: it's now in incoming active period (as a device), whether go to sleep depending on parameter macRxOnWhenIdle" << endl;
            t_state = mpib.macRxOnWhenIdle?phy_RX_ON:phy_TRX_OFF;
        }
    }
    else // non-beacon
    {
        EV << "[MAC]: non-beacon, wether go to sleep depending on parameter macRxOnWhenIdle" << endl;
        t_state = mpib.macRxOnWhenIdle?phy_RX_ON:phy_TRX_OFF;
    }
    PLME_SET_TRX_STATE_request(t_state);
}

//-------------------------------------------------------------------------------/
/**************************** <MAC2PHY primitive> *******************************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::PLME_SET_TRX_STATE_request(PHYenum state)
{
    EV << "[MAC]: sending PLME_SET_TRX_STATE_request <" << state <<"> to PHY layer" << endl;
    // construct PLME_SET_TRX_STATE_request primitive
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_SET_TRX_STATE_REQUEST);
    primitive->setStatus(state);
    send(primitive, mLowergateOut);
    trx_state_req = state;      // store requested radio state
}

void Ieee802154Mac::PLME_SET_request(PHYPIBenum attribute)
{
    // construct PLME_SET_request primitive
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_SET_REQUEST);

    switch (attribute)
    {
    case PHY_CURRENT_CHANNEL:
        primitive->setAttribute(PHY_CURRENT_CHANNEL);
        primitive->setChannelNumber(tmp_ppib.phyCurrentChannel);
        break;

        /*case PHY_CHANNELS_SUPPORTED:
            primitive->setAttribute(PHY_CHANNELS_SUPPORTED);
            primitive->setPhyChannelsSupported(tmp_ppib.phyChannelsSupported);
            break;

        case PHY_TRANSMIT_POWER:
            primitive->setAttribute(PHY_TRANSMIT_POWER);
            primitive->setPhyTransmitPower(tmp_ppib.phyTransmitPower);
            break;

        case PHY_CCA_MODE:
            primitive->setAttribute(PHY_CCA_MODE);
            primitive->setPhyCCAMode(tmp_ppib.phyCCAMode);
            break; */

    default:
        error("invalid PHY PIB attribute");
        break;
    }

    send(primitive, mLowergateOut);
}

void Ieee802154Mac::PLME_CCA_request()
{
    // construct PLME_CCA_request primitive
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_CCA_REQUEST);
    send(primitive, mLowergateOut);
    EV << "[MAC]: send PLME_CCA_request to PHY layer" << endl;
}

void Ieee802154Mac::PLME_bitrate_request()
{
    // construct PLME_CCA_request primitive
    Ieee802154MacPhyPrimitives *primitive = new Ieee802154MacPhyPrimitives();
    primitive->setKind(PLME_GET_BITRATE);
    send(primitive, mLowergateOut);
    EV << "[MAC]: send PLME_GET_BITRATE to PHY layer" << endl;
}


//-------------------------------------------------------------------------------/
/************************* <PHY2MAC primitive Handler> **************************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::handleMacPhyPrimitive(int msgkind, cMessage* msg)
{
    Ieee802154MacPhyPrimitives* primitive = check_and_cast<Ieee802154MacPhyPrimitives *>(msg);
    switch (msgkind)
    {
        if (primitive->getBitRate()>0)
            bitrate = primitive->getBitRate();

    case PD_DATA_CONFIRM:
        handle_PD_DATA_confirm(PHYenum(primitive->getStatus()));
        delete primitive;
        break;

    case PLME_CCA_CONFIRM:
        handle_PLME_CCA_confirm(PHYenum(primitive->getStatus()));
        delete primitive;
        break;

    case PLME_ED_CONFIRM:
        // TBD
        break;

    case PLME_SET_TRX_STATE_CONFIRM:
        handle_PLME_SET_TRX_STATE_confirm(PHYenum(primitive->getStatus()));
        delete primitive;
        break;

    case PLME_SET_CONFIRM:
        // TBD
        break;

    case PLME_GET_BITRATE:
        delete primitive;
        break;


    default:
        error("unknown primitive (msgkind=%d)", msgkind);
        break;
    }
}

void Ieee802154Mac::handle_PD_DATA_confirm(PHYenum status)
{
    inTransmission = false;

    if (backoffStatus == 1)
        backoffStatus = 0;
    if (status == phy_SUCCESS)
    {
        dispatch(status,__FUNCTION__);
    }
    // sening pkt at phy layer failed, it may happen only when txing is terminated by FORCE_TRX_OFF
    else if (txPkt == txBeacon)
    {
        beaconWaitingTx = false;
        delete txBeacon;
        txBeacon = NULL;
    }
    else if (txPkt == txAck)
    {
        delete txAck;
        txAck = NULL;
    }
    else if (txPkt == txGTS)
    {
        error("[GTS]: GTS transmission failed");
    }
    else    // TBD: RX_ON/TRX_OFF -- possible if the transmisstion is terminated by a FORCE_TRX_OFF or change of channel, or due to energy depletion
        {}
}

void Ieee802154Mac::handle_PLME_CCA_confirm(PHYenum status)
{
    if (taskP.taskStatus(TP_CCA_CSMACA))
    {
        taskP.taskStatus(TP_CCA_CSMACA) = false;

        // the following from CsmaCA802_15_4::CCA_confirm
        if (status == phy_IDLE)                     // idle channel
        {
            if (!beaconEnabled)                 // non-beacon, unslotted
            {
                tmpCsmaca = NULL;
                csmacaCallBack(phy_IDLE);           // unslotted successfully, callback
                return;
            }
            else                            // beacon-enabled, slotted
            {
                CW--;                       // Case C1, beacon-enabled, slotted
                if (CW == 0)                    // Case D1
                {
                    //timing condition may not still hold -- check again
                    if (csmacaCanProceed(0.0, true))    // Case E1
                    {
                        tmpCsmaca = 0;
                        csmacaCallBack(phy_IDLE);    // slotted CSMA-CA successfully
                        return;
                    }
                    else                    // Case E2: postpone until next beacon sent or received
                    {
                        CW = 2;
                        bPeriodsLeft = 0;
                        csmacaWaitNextBeacon = true;    // Debugged
                    }
                }
                else    handleBackoffTimer();           // Case D2: perform CCA again, this function sends a CCA_request
            }
        }
        else    // busy channel
        {
            if (beaconEnabled) CW = 2;              // Case C2
            NB++;
            if (NB > mpib.macMaxCSMABackoffs)   // Case F1
            {
                tmpCsmaca = 0;
                csmacaCallBack(phy_BUSY);
                return;
            }
            else    // Case F2: backoff again
            {
                BE++;
                if (BE > aMaxBE) BE = aMaxBE;       // aMaxBE defined in Ieee802154Const.h
                csmacaStart(false);             // not the first time
            }
        }
    }
}

void Ieee802154Mac::handle_PLME_SET_TRX_STATE_confirm(PHYenum status)
{
    EV << "[MAC]: a PLME_SET_TRX_STATE_confirm with " << status << " reveived from PHY, the requested state is " << trx_state_req << endl;
    simtime_t delay;

    PHYenum recStatus = status;
    if (status == phy_SUCCESS)
        status = trx_state_req;

    if (backoffStatus == 99)
    {
        if (trx_state_req == phy_RX_ON)
        {
            if (taskP.taskStatus(TP_RX_ON_CSMACA))
            {
                taskP.taskStatus(TP_RX_ON_CSMACA) = false;
                csmaca_handle_RX_ON_confirm(status);
            }
        }
    }
    /*else
        dispatch(status,__FUNCTION__,trx_state_req);*/

    if (status != phy_TX_ON) return;
    // wait the phy_succes
    if (recStatus!= phy_SUCCESS) return;

    //transmit the packet
    if (beaconWaitingTx)        // periodically tx beacon
    {
        // to synchronize better, we don't transmit the beacon here
    }
    else if (txAck)
    {
        //although no CSMA-CA required for the transmission of ack.,
        //but we still need to locate the backoff period boundary if beacon enabled
        //(refer to page 157, line 25-31)
        if ((mpib.macBeaconOrder == 15)&&(rxBO == 15))  //non-beacon enabled
            delay = 0.0;
        else if (txAck->getIsGTS())
            delay = 0.0;
        else                                //beacon enabled
        {
            // here we use the hidden dst addr that we already set in ACK on purpose
            delay  = csmacaLocateBoundary((txAck->getDstAddr() == mpib.macCoordShortAddress),0.0);
            ASSERT(delay < bPeriod);
        }

        if (delay == 0.0)
            handleTxAckBoundTimer();
        else
            startTxAckBoundTimer(delay);
    }
    else if (txGTS)
    {
        // send data frame in GTS here, no delay
        txPkt = txGTS;
        sendDown(check_and_cast<Ieee802154Frame *>(txGTS->dup()));
    }
    else        // tx cmd or data
    {
        if ((mpib.macBeaconOrder == 15)&&(rxBO == 15))  //non-beacon enabled
            delay = 0.0;
        else
            delay = csmacaLocateBoundary(toParent(txCsmaca),0.0);

        if (delay == 0.0)
            handleTxCmdDataBoundTimer();            //transmit immediately
        else
            startTxCmdDataBoundTimer(delay);
    }
}

//-------------------------------------------------------------------------------/
/***************************** <SSCS-MAC Primitive> *****************************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::MCPS_DATA_request(UINT_8 SrcAddrMode, UINT_16 SrcPANId, IE3ADDR SrcAddr,
                                      UINT_8 DstAddrMode, UINT_16 DstPANId, IE3ADDR DstAddr,
                                      cPacket* msdu,  Ieee802154TxOption TxOption)
{

    Ieee802154MacTaskType task = TP_MCPS_DATA_REQUEST;
    FrameCtrl frmCtrl;

    checkTaskOverflow(task);    // reset step to 0 if no task of this type is pending

    taskP.taskStatus(task) = true;
    taskP.mcps_data_request_TxOption = TxOption;

    /* construct MPDU */

    // construct frame control field
    frmCtrl.frmType = Ieee802154_DATA;      //data type
    frmCtrl.secu = secuData;
    frmCtrl.frmPending = false;
    frmCtrl.intraPan = (SrcPANId == DstPANId)? true:false;
    frmCtrl.dstAddrMode = DstAddrMode;
    frmCtrl.srcAddrMode = SrcAddrMode;
    if (TxOption == DIRECT_TRANS)
        frmCtrl.ackReq = ack4Data;
    else if (TxOption == GTS_TRANS)
        frmCtrl.ackReq = ack4Gts;

    Ieee802154DataFrame* tmpData = new Ieee802154DataFrame();
    tmpData->setName("Ieee802154DATA");
    tmpData->setFrmCtrl(frmCtrl);
    tmpData->setBdsn(mpib.macDSN++);
    tmpData->setDstPanId(DstPANId);
    tmpData->setDstAddr(DstAddr);
    tmpData->setSrcPanId(SrcPANId);
    tmpData->setSrcAddr(SrcAddr);
    if (TxOption == GTS_TRANS)
        tmpData->setIsGTS(true);

    // set length and encapsulate msdu
    tmpData->setByteLength(calFrmByteLength(tmpData));
    tmpData->encapsulate(msdu);     // the length of msdu is added to mpdu
    EV << "[MAC]: MPDU constructed: " << tmpData->getName() << ", #" << (int)tmpData->getBdsn() << ", " << tmpData->getByteLength() << " Bytes" << endl;
    switch (TxOption)
    {
    case DIRECT_TRANS:
    {
        taskP.taskStep(task)++;
        strcpy(taskP.taskFrFunc(task),"csmacaCallBack");
        ASSERT(txData == NULL);
        txData = tmpData;
        csmacaEntry('d');
        break;
    }

    case INDIRECT_TRANS:
        // TBD
        break;

    case GTS_TRANS:
    {
        taskP.taskStep(task)++;
        // waiting for GTS arriving, callback from handleGtsTimer()
        strcpy(taskP.taskFrFunc(task),"handleGtsTimer");
        ASSERT(txGTS == NULL);
        txGTS = tmpData;
        numGTSRetry = 0;

        // if i'm the PAN coordinator, should defer the transmission until the start of the receive GTS
        // if i'm the device, should try to transmit if now is in my GTS
        // refer to spec. 7.5.7.3
        if (!isPANCoor && index_gtsTimer == 99)
        {
            ASSERT(gtsTimer->isScheduled());
            // first check if the requested transaction can be completed before the end of current GTS
            if (gtsCanProceed())
            {
                // directly hand over to FSM, which will go to next step, state parameters are ignored
                FSM_MCPS_DATA_request();
            }
        }
        break;
    }

    default:
        error("[MAC]: undefined txOption: %d!", TxOption);
    }
}

void Ieee802154Mac::MCPS_DATA_confirm(MACenum status)
{
    // should report to upper layer here
    return;
}

void Ieee802154Mac::MCPS_DATA_indication(Ieee802154Frame* tmpData)
{
    if (tmpData->getIsGTS())
    {
        numRxGTSPkt++;
    }
    else
    {
        numRxDataPkt++;
    }
    cMessage *appframe = tmpData->decapsulate();
    delete tmpData;
    EV << "[MAC]: sending received " << appframe->getName() << " frame to upper layer" << endl;
    send(appframe, mUppergateOut);
}

//-------------------------------------------------------------------------------/
/***************************** <CSMA/CA Functions> ******************************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::csmacaEntry(char pktType)
{
    if (pktType == 'c')     // txBcnCmd
    {
        waitBcnCmdAck = false;          // beacon packet not yet transmitted
        numBcnCmdRetry = 0;
        if (backoffStatus == 99)        // backoffing for data packet
        {
            backoffStatus = 0;
            csmacaCancel();
        }
        csmacaResume();
    }
    else if (pktType == 'u')    // txBcnCmdUpper
    {
        waitBcnCmdUpperAck = false;         //command packet not yet transmitted
        numBcnCmdUpperRetry = 0;
        if ((backoffStatus == 99)&&(txCsmaca != txBcnCmd))  //backoffing for data packet
        {
            backoffStatus = 0;
            csmacaCancel();
        }
        csmacaResume();

    }
    else //if (pktType == 'd')  //txData
    {
        waitDataAck = false;            //data packet not yet transmitted
        numDataRetry = 0;
        csmacaResume();
    }
}

void Ieee802154Mac::csmacaResume()
{
    bool ackReq;

    if ((backoffStatus != 99)           //not during backoff
            &&  (!inTransmission))              //not during transmission
        if ((txBcnCmd) && (!waitBcnCmdAck))
        {
            backoffStatus = 99;
            ackReq = txBcnCmd->getFrmCtrl().ackReq;
            txCsmaca = txBcnCmd;
            csmacaStart(true, txBcnCmd, ackReq);
        }
        else if ((txBcnCmdUpper) && (!waitBcnCmdUpperAck))
        {
            backoffStatus = 99;
            ackReq = txBcnCmdUpper->getFrmCtrl().ackReq;
            txCsmaca = txBcnCmdUpper;
            csmacaStart(true, txBcnCmdUpper, ackReq);
        }
        else if ((txData) && (!waitDataAck))
        {
            strcpy(taskP.taskFrFunc(TP_MCPS_DATA_REQUEST), "csmacaCallBack");   //the transmission may be interrupted and need to backoff again
            taskP.taskStep(TP_MCPS_DATA_REQUEST) = 1;               //also set the step

            backoffStatus = 99;
            ackReq = txData->getFrmCtrl().ackReq;
            txCsmaca = txData;
            csmacaStart(true, txData, ackReq);
        }
}

void Ieee802154Mac::csmacaStart(bool firsttime, Ieee802154Frame* frame, bool ackReq)
{
    bool backoff;
    simtime_t wtime, rxBI;

    if (txAck)
    {
        backoffStatus = 0;
        tmpCsmaca = 0;
        return;
    }

    ASSERT(!backoffTimer->isScheduled());

    if (firsttime)
    {
        EV << "[CSMA-CA]: starting CSMA-CA for " << frame->getName() << ":#" << (int)frame->getBdsn() << endl;
        beaconEnabled = ((mpib.macBeaconOrder != 15)||(rxBO != 15));
        csmacaReset(beaconEnabled);
        ASSERT(tmpCsmaca == 0);
        tmpCsmaca = frame;
        csmacaAckReq = ackReq;
    }

    if (rxBO != 15)     // is receiving beacon
    {
        // set schedBcnRxTime: the latest time that the beacon should have arrived
        // schedBcnRxTime may not be bcnRxTime, when missing some beacons
        // or being in the middle of rxing a beacon
        schedBcnRxTime = bcnRxTime;
        rxBI = rxSfSpec.BI / phy_symbolrate;
        while (schedBcnRxTime + rxBI < simTime())
            schedBcnRxTime += rxBI;
    }

    wtime = intrand(1<<BE) * bPeriod;   // choose a number in [0, pow(2, BE)-1]
    EV << "[CSMA-CA]: choosing random number of backoff periods: " << wtime/bPeriod << endl;
    EV << "[CSMA-CA]: backoff time before adjusting: " << wtime*1000 << " ms" << endl;
    wtime = csmacaAdjustTime(wtime);    // if scheduled backoff ends before CAP, wtime should be adjusted
    EV << "[CSMA-CA]: backoff time after adjusting: " << wtime*1000 << " ms" << endl;
    backoff = true;

    if (beaconEnabled)
    {
        if (firsttime)
        {
            wtime = csmacaLocateBoundary(toParent(tmpCsmaca), wtime);
            EV << "[CSMA-CA]: backoff time after locating boundary: " << wtime*1000 << " ms" << endl;
        }
        if (!csmacaCanProceed(wtime))   // check if backoff can be applied now. If not, canProceed() will decide how to proceed.
            backoff = false;
    }
    if (backoff)
    {
        startBackoffTimer(wtime);
    }
}

void Ieee802154Mac::csmacaCancel()
{
    if (backoffTimer->isScheduled())
        cancelEvent(backoffTimer);
    else if (deferCCATimer->isScheduled())
        cancelEvent(deferCCATimer);
    else
        taskP.taskStatus(TP_CCA_CSMACA) = false;
    tmpCsmaca = 0;
}

void Ieee802154Mac::csmacaCallBack(PHYenum status)
{
    if (((!txBcnCmd)||(waitBcnCmdAck))
            &&((!txBcnCmdUpper)||(waitBcnCmdUpperAck))
            &&((!txData)||(waitDataAck)))
        return;

    backoffStatus = (status == phy_IDLE)?1:2;

    dispatch(status,__FUNCTION__);
}

void Ieee802154Mac::csmacaReset(bool bcnEnabled)
{
    if (bcnEnabled)
    {
        NB = 0;
        CW = 2;
        BE = mpib.macMinBE;
        if ((mpib.macBattLifeExt)&&(BE > 2))
            BE = 2;
    }
    else
    {
        NB = 0;
        BE = mpib.macMinBE;
    }
}

simtime_t Ieee802154Mac::csmacaAdjustTime(simtime_t wtime)
{
    //find the beginning point of CAP and adjust the scheduled time
    //if it comes before CAP
    simtime_t neg;
    simtime_t tmpf;

    ASSERT(tmpCsmaca);
    if (!toParent(tmpCsmaca))       // as a coordinator
    {
        if (mpib.macBeaconOrder != 15)
        {
            /* Linux floating number compatibility
            neg = (CURRENT_TIME + wtime - bcnTxTime) - mac->beaconPeriods * bPeriod;
            */
            {
                tmpf = txBcnDuration * bPeriod;
                tmpf = simTime() - tmpf;
                tmpf += wtime;
                neg = tmpf - mpib.macBeaconTxTime;
            }

            if (neg < 0.0)
                wtime -= neg;
            return wtime;
        }
        else
            return wtime;
    }
    else    // as a device
    {
        if (rxBO != 15)
        {
            /* Linux floating number compatibility
            neg = (CURRENT_TIME + wtime - bcnRxTime) - mac->beaconPeriods2 * bPeriod;
            */
            {
                tmpf = rxBcnDuration * bPeriod;
                tmpf = simTime() - tmpf;
                tmpf += wtime;
                neg = tmpf - schedBcnRxTime;
            }

            if (neg < 0.0)
                wtime -= neg;
            return wtime;
        }
        else
            return wtime;
    }
}

simtime_t Ieee802154Mac::csmacaLocateBoundary(bool toParent, simtime_t wtime)
{
    int align;
    simtime_t bcnTxRxTime, tmpf, newtime, rxBI;

    if ((mpib.macBeaconOrder == 15) && (rxBO == 15))
        return wtime;

    if (toParent)
        align = (rxBO == 15)?1:2;
    else
        align = (mpib.macBeaconOrder == 15)?2:1;

    // we need to do this here, because this function may be called outside the CSMA-CA algorithm, e.g. in handle_PLME_SET_TRX_STATE_confirm while preparing to send an ACK
    if (rxBO != 15)     // is receiving beacon
    {
        // set schedBcnRxTime: the latest time that the beacon should have arrived
        // schedBcnRxTime may not be bcnRxTime, when missing some beacons
        // or being in the middle of rxing a beacon
        schedBcnRxTime = bcnRxTime;
        rxBI = rxSfSpec.BI / phy_symbolrate;
        while (schedBcnRxTime + rxBI < simTime())
            schedBcnRxTime += rxBI;
    }

    bcnTxRxTime = (align == 1)? mpib.macBeaconTxTime:schedBcnRxTime;

    /* Linux floating number compatibility
    newtime = fmod(CURRENT_TIME + wtime - bcnTxRxTime, bPeriod);
    */
    tmpf = simTime() + wtime;
    tmpf -= bcnTxRxTime;
    newtime = fmod(tmpf, bPeriod);
    //EV << "simTime() = " << simTime() << "; wtime = " << wtime << "; newtime = " << newtime << endl;

    if (newtime > 0.0)
    {
        /* Linux floating number compatibility
        newtime = wtime + (bPeriod - newtime);
        */

        tmpf = bPeriod - newtime;
        EV << "[CSMA-CA]: deviation to backoff boundary: " << tmpf << " s" <<endl;
        newtime = wtime + tmpf;
    }
    else
        newtime = wtime;

    return newtime;
}

bool Ieee802154Mac::csmacaCanProceed(simtime_t wtime, bool afterCCA)
{
    bool ok;
    UINT_16 t_bPeriods, t_CAP;
    simtime_t tmpf, rxBI, t_fCAP, t_CCATime, t_IFS, t_transacTime;
    simtime_t now = simTime();
    csmacaWaitNextBeacon = false;
    t_transacTime = 0;
    wtime = csmacaLocateBoundary(toParent(tmpCsmaca), wtime);

    // check if CSMA-CA can proceed within the current superframe
    // in the case the node acts as both a coordinator and a device, both the superframes from and to this node should be taken into account)
    // Check 1: if now is in CAP
    // Check 2: if the number of backoff periods greater than the remaining number of backoff periods in the CAP
    // check 3: if the entire transmission can be finished before the end of current CAP

    EV << "[CSMA-CA]: starting to evaluate whether CSMA-CA can proceed ..." << endl;
    if (!toParent(tmpCsmaca))       // as a coordinator
    {
        if (mpib.macBeaconOrder != 15)      // beacon enabled as a coordinator
        {
            t_fCAP = mpib.macBeaconTxTime + (txSfSpec.finalCap + 1) * txSfSlotDuration / phy_symbolrate;
            if (now >= t_fCAP)      // Check 1
            {
                csmacaWaitNextBeacon = true;
                bPeriodsLeft = 0;   // rechoose random backoff
                EV <<"[CSMA-CA]: cannot porceed because it's now NOT in CAP (outgoing), resume at the beginning of next CAP" << endl;
                return false;
            }
            else                    // Check 2
            {
                // calculate num of Backoff periods in CAP except for txBcnDuration
                t_CAP = (txSfSpec.finalCap + 1) * txSfSlotDuration / aUnitBackoffPeriod - txBcnDuration;
                EV << "[CSMA-CA]: total number of backoff periods in CAP (except for txBcnDuration): " << t_CAP << endl;

                // calculate num of Backoff periods from the first backoff in CAP to now+wtime
                tmpf = now + wtime;
                tmpf -= mpib.macBeaconTxTime;
                EV << "[CSMA-CA]: wtime = " << wtime << "; mpib.macBeaconTxTime = " << mpib.macBeaconTxTime << endl;
                tmpf = tmpf / bPeriod;
                EV << "[CSMA-CA]: tmpf = " << tmpf << "; bPeriod  = " << bPeriod << endl;
                t_bPeriods = (UINT_16)(SIMTIME_DBL(tmpf) - txBcnDuration);
                EV << "[CSMA-CA]: t_bPeriods = " << t_bPeriods << "; txBcnDuration = " << (int)txBcnDuration << endl;

                tmpf = now + wtime;
                tmpf -= mpib.macBeaconTxTime;
                if (fmod(tmpf, bPeriod) > 0.0)
                    t_bPeriods++;

                // calculate the difference
                bPeriodsLeft = t_bPeriods - t_CAP;      // backoff periods left for next superframe
            }
        }
        else        // non-beacon as a coordinator, but beacon enabled as a device
            bPeriodsLeft = -1;
    }
    else        // as a device
    {
        if (rxBO != 15)     // beacon enabled as a device
        {
            // we assume that no beacon will be lost
            /* If max beacon loss reached, use unslotted version
            rxBI = rxSfSpec.BI / phy_symbolrate;
            tmpf = (rxSfSpec.finalCap + 1) * (aBaseSlotDuration * (1 << rxSfSpec.SO))/phy_symbolrate;
            tmpf += bcnRxTime;
            tmpf += aMaxLostBeacons * rxBI;

            if (tmpf < now)
                bPeriodsLeft = -1;
            else
            {*/

            t_fCAP = schedBcnRxTime + (rxSfSpec.finalCap + 1) * rxSfSlotDuration / phy_symbolrate;
            if (now >= t_fCAP)      // Check 1
            {
                csmacaWaitNextBeacon = true;
                bPeriodsLeft = 0;   // rechoose random backoff
                EV <<"[CSMA-CA]: cannot porceed because it's now NOT in CAP (incoming), resume at the beginning of next CAP" << endl;
                return false;
            }
            else
            {
                //  TBD: battLifeExt
                /*if (rxSfSpec.battLifeExt)
                    ;//t_CAP = getBattLifeExtSlotNum();
                else*/
                t_CAP = (rxSfSpec.finalCap + 1) * rxSfSlotDuration / aUnitBackoffPeriod - rxBcnDuration;
                EV << "[CSMA-CA]: total number of backoff periods in CAP (except for rxBcnDuration): " << t_CAP << endl;
                // calculate num of Backoff periods from the first backoff in CAP to now+wtime
                tmpf = now + wtime;
                tmpf -= schedBcnRxTime;
                double temp = SIMTIME_DBL(tmpf);
                tmpf =(temp/bPeriod);
                EV << "[CSMA-CA]: tmpf = " << tmpf << "; bPeriod  = " << bPeriod << endl;
                t_bPeriods = (UINT_16)(SIMTIME_DBL(tmpf) - rxBcnDuration);
                EV << "[CSMA-CA]: t_bPeriods = " << t_bPeriods << "; rxBcnDuration = " << (int)rxBcnDuration << endl;

                tmpf = now + wtime;
                tmpf -= schedBcnRxTime;
                if (fmod(tmpf, bPeriod) > 0.0)
                    t_bPeriods++;
                bPeriodsLeft = t_bPeriods - t_CAP;      // backoff periods left for next superframe
            }
        }
        else        // non-beacon as a device, but beacon enabled as a coordinator
            bPeriodsLeft = -1;
    }

    EV << "[CSMA-CA]: bPeriodsLeft = " << bPeriodsLeft << endl;
    ok = true;
    if (bPeriodsLeft > 0)
        ok = false;
    else if (bPeriodsLeft == 0)
    {
        if ((!toParent(tmpCsmaca))
                &&  (!txSfSpec.battLifeExt))
            ok = false;
        else if ((toParent(tmpCsmaca))
                 &&  (!rxSfSpec.battLifeExt))
            ok = false;
    }
    if (!ok)
    {
        EV << "[CSMA-CA]: cannot porceed because the chosen num of backoffs cannot finish before the end of current CAP, resume at the beginning of next CAP" << endl;
        // if as a device, need to track next beacon if it is not being tracked
        if (rxBO != 15)
            if (!bcnRxTimer->isScheduled())
                startBcnRxTimer();
        csmacaWaitNextBeacon = true;
        return false;
    }

    // check 3
    // If backoff can finish before the end of CAP or sent in non-beacon, calculate the time needed to finish the transaction and evaluate it
    t_CCATime = 8.0/phy_symbolrate;
    if (tmpCsmaca->getByteLength() <= aMaxSIFSFrameSize)
        t_IFS = aMinSIFSPeriod;
    else
        t_IFS = aMinLIFSPeriod;
    t_IFS = t_IFS/phy_symbolrate;

    if (!afterCCA)
    {
        t_transacTime = t_CCATime;              //first CCA time
        t_transacTime += csmacaLocateBoundary(toParent(tmpCsmaca), t_transacTime) - (t_transacTime);    // locate boundary for second CCA
        t_transacTime += t_CCATime;             //second CCA time
    }
    t_transacTime += csmacaLocateBoundary(toParent(tmpCsmaca), t_transacTime) - (t_transacTime);        // locate boundary for transmission
    t_transacTime += calDuration(tmpCsmaca);            // calculate packet transmission time

    if (csmacaAckReq)       // if ACK required
    {
        t_transacTime += mpib.macAckWaitDuration/phy_symbolrate;    //ack. waiting time (this value does not include round trip propagation delay)
        //t_transacTime += 2*max_pDelay;    //round trip propagation delay (802.15.4 ignores this, but it should be there even though it is very small)
        t_transacTime += t_IFS;             //IFS time -- not only ensure that the sender can finish the transaction, but also the receiver
    }
    else                    // no ACK required
    {
        t_transacTime += aTurnaroundTime/phy_symbolrate;        //transceiver turn-around time (receiver may need to do this to transmit next beacon)
        //t_transacTime += max_pDelay;      //one-way trip propagation delay (802.15.4 ignores this, but it should be there even though it is very small)
        t_transacTime += t_IFS;         //IFS time -- not only ensure that the sender can finish the transaction, but also the receiver
    }

    tmpf = now + wtime;
    tmpf += t_transacTime;
    EV << "[CSMA-CA]: the entire transmission is estimated to finish at " << tmpf << " s" << endl;
    // calculate the time for the end of cap
    if (!toParent(tmpCsmaca))               // sent as a coordinator
    {
        if (mpib.macBeaconOrder != 15)      // sent in beacon-enabled, check the end of tx cap
            t_fCAP = getFinalCAP('t');
        else                        // rxBO != 15, this case should be reconsidered, check the end of rx cap
            t_fCAP = getFinalCAP('r');
    }
    else                            // sent as a device
    {
        if (rxBO != 15)                 // sent in beacon-enabled, check the end of rx cap
        {
            t_fCAP = getFinalCAP('r');
        }
        else                        // mpib.macBeaconOrder != 15, this case should be reconsidered, check the end of rx cap
            t_fCAP = getFinalCAP('t');
    }

    EV << "[CSMA-CA]: the current CAP will end at " << t_fCAP << " s" << endl;
    // evaluate if the entire transmission process can be finished before end of current CAP
    if (tmpf > t_fCAP)
    {
        ok = false;
        EV << "[CSMA-CA]: cannot proceed because the entire transmission cannot finish before the end of current cap" << endl;
    }
    else
    {
        ok = true;
        EV << "[CSMA-CA]: can proceed" << endl;
    }

    //check if have enough CAP to finish the transaction
    if (!ok)
    {
        bPeriodsLeft = 0;       // in the next superframe, apply a further backoff delay before evaluating once again
        // if as a device, need to track next beacon if it is not being tracked
        if (rxBO != 15)
            if (!bcnRxTimer->isScheduled()) startBcnRxTimer();
        csmacaWaitNextBeacon = true;
        return false;
    }
    else
    {
        bPeriodsLeft = -1;
        return true;
    }
}

simtime_t Ieee802154Mac::getFinalCAP(char trxType)
{
    simtime_t txSlotDuration ,rxSlotDuration, rxBI, txCAP, rxCAP;
    simtime_t now,oneDay,tmpf;

    now = simTime();
    oneDay = now + 24.0*3600;

    if ((mpib.macBeaconOrder == 15)&&(rxBO == 15))          //non-beacon enabled
        return oneDay;                                  //transmission can always go ahead

    txSlotDuration = txSfSlotDuration / phy_symbolrate;
    rxSlotDuration = rxSfSlotDuration / phy_symbolrate;
    rxBI = rxSfSpec.BI / phy_symbolrate;

    if (trxType == 't')                             // check tx CAP
    {
        if (mpib.macBeaconOrder != 15)  // beacon enabled
        {
            if (txSfSpec.battLifeExt)
            {
                error("[MAC]: this function TBD");
                /*tmpf = (beaconPeriods + getBattLifeExtSlotNum()) * aUnitBackoffPeriod;
                t_CAP = bcnTxTime + tmpf;*/
            }
            else
            {
                tmpf = (txSfSpec.finalCap + 1) * txSlotDuration;
                txCAP = mpib.macBeaconTxTime + tmpf;
            }
            return (txCAP >= now)? txCAP:oneDay;
        }
        else
            return oneDay;
    }
    else                                                    // check rx CAP
    {
        if (rxBO != 15)                         // beacon enabled
        {
            if (rxSfSpec.battLifeExt)
            {
                error("[MAC]: this function TBD");
                /*
                tmpf = (beaconPeriods2 + getBattLifeExtSlotNum()) * aUnitBackoffPeriod;
                t_CAP2 = bcnRxTime + tmpf;
                */
            }
            else
            {
                tmpf = (rxSfSpec.finalCap + 1) * rxSlotDuration;
                EV << "[getfinalcap]: now = " << now << endl;
                EV << "[getfinalcap]: tmpf = " << tmpf << endl;
                rxCAP = bcnRxTime + tmpf;
                EV << "[getfinalcap]: rxCAP = " << rxCAP << endl;
            }

            tmpf = aMaxLostBeacons * rxBI;
            // adjust if beacon loss occurs or during rceiving bcn
            if ((rxCAP < now)&&(rxCAP + tmpf >= now))   //no more than <aMaxLostBeacons> beacons missed
            {
                EV << "[MAC]: during receiving a beacon, now is: " << now << ", last beacon is rceived at " << bcnRxTime << endl;
                while (rxCAP < now) rxCAP += rxBI;
            }
            return (rxCAP>=now)?rxCAP:oneDay;
        }
        else
            return oneDay;
    }
}

void Ieee802154Mac::csmaca_handle_RX_ON_confirm(PHYenum status)
{
    simtime_t now, wtime;

    if (status != phy_RX_ON)
    {
        if (status == phy_BUSY_TX)
        {
            EV << "[CSMA-CA]: radio is busy Txing now, RX_ON will be set later" << endl;
            taskP.taskStatus(TP_RX_ON_CSMACA) = true;
            EV  << "[MAC-TASK]: TP_RX_ON_CSMACA = true" << endl;
        }
        else
        {
            EV << "[CSMA-CA]: RX_ON request did not succeed, try again" << endl;
            handleBackoffTimer();
        }
        return;
    }

    // phy_RX_ON
    // locate backoff boundary if needed
    EV << "[CSMA-CA]: ok, radio is set to RX_ON " << endl;
    now = simTime();
    if (beaconEnabled)
    {
        EV << "[CSMA-CA]: locate backoff boundary before sending CCA request" << endl;
        wtime = csmacaLocateBoundary(toParent(tmpCsmaca), 0.0);
    }
    else
        wtime = 0.0;

    if (wtime == 0.0)
    {
        taskP.taskStatus(TP_CCA_CSMACA) = true;
        EV  << "[MAC-TASK]: TP_CCA_CSMACA = true" << endl;
        PLME_CCA_request();
    }
    else
        startDeferCCATimer(wtime);
}

void Ieee802154Mac::csmacaTrxBeacon(char trx)
// To be called each time that a beacon received or transmitted if backoffStatus == 99
{
    simtime_t wtime;
    if (!txAck)
        PLME_SET_TRX_STATE_request(phy_RX_ON);

    //update values
    beaconEnabled = ((mpib.macBeaconOrder != 15)||(rxBO != 15));
    csmacaReset(beaconEnabled);

    if (csmacaWaitNextBeacon)
        if (tmpCsmaca && (!backoffTimer->isScheduled()))
        {
            EV << "[CSMA-CA]:  triggered again by the starting of the new CAP" << endl;
            ASSERT(bPeriodsLeft >= 0);
            if (bPeriodsLeft == 0)      // backoff again
            {
                EV << "[CSMA-CA]: bPeriodsLeft == 0, need to rechoose random number of backoffs" << endl;
                csmacaStart(false);
            }
            else        // resume backoff: this is the second entry to CSMA-CA without calling <csmacaStart()>
            {
                /*wtime = csmacaAdjustTime(0.0);
                EV << "wtime = " << wtime << endl;*/
                EV << "[CSMA-CA]: bPeriodsLeft > 0, continue with the number of backoffs left from last time" << endl;
                wtime += bPeriodsLeft * bPeriod;
                if (csmacaCanProceed(wtime))
                    startBackoffTimer(wtime);
            }

            return;             // debug here
        }
    csmacaWaitNextBeacon = false;   // really necessary??
}

//-------------------------------------------------------------------------------/
/***************************** <Timer starter> **********************************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::startBackoffTimer(simtime_t wtime)
{
    EV << "[CSMA-CA]: #"<< (int)tmpCsmaca->getBdsn()<< ": starting backoff for " << wtime << " s" << endl;
    if (backoffTimer->isScheduled())
        cancelEvent(backoffTimer);
    scheduleAt(simTime() + wtime, backoffTimer);
}

void Ieee802154Mac::startDeferCCATimer(simtime_t wtime)
{
    if (deferCCATimer->isScheduled())
        cancelEvent(deferCCATimer);
    scheduleAt(simTime() + wtime, deferCCATimer);
    EV << "[CSMA-CA]: delay " << wtime << " s before performing CCA" << endl;
}

void Ieee802154Mac::startBcnRxTimer()
{
    simtime_t rxBI, t_bcnRxTime, now, len12s, wtime, tmpf;
    now = simTime();

    rxBI = rxSfSpec.BI / phy_symbolrate;
    t_bcnRxTime = bcnRxTime;

    while (now - t_bcnRxTime > rxBI)
        t_bcnRxTime += rxBI;
    len12s = aTurnaroundTime / phy_symbolrate;

    tmpf = (now - t_bcnRxTime);
    wtime = rxBI - tmpf;

    if (wtime >= len12s)
        wtime -= len12s;

    tmpf = now + wtime;
    if (tmpf - lastTime_bcnRxTimer < rxBI - len12s)
    {
        tmpf = 2 * rxBI;
        tmpf = tmpf - now;
        tmpf = tmpf + t_bcnRxTime;
        wtime = tmpf - len12s;
        //wtime = 2* BI - (now - bcnRxTime) - len12s;
    }
    lastTime_bcnRxTimer = now + wtime;
    if (bcnRxTimer->isScheduled())
        cancelEvent(bcnRxTimer);
    scheduleAt(now + wtime, bcnRxTimer);
}

void Ieee802154Mac::startBcnTxTimer(bool txFirstBcn, simtime_t startTime)
{
    simtime_t wtime, now, tmpf;
    now = simTime();

    if (txFirstBcn)     // tx my first beacon
    {
        ASSERT(mpib.macBeaconOrder != 15);
        if (startTime == 0.0)   // transmit beacon right now
        {
            txNow_bcnTxTimer = true;
            beaconWaitingTx = true;
            PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            handleBcnTxTimer(); // no delay
            return;
        }
        else        // relative to bcnRxTime
        {
            txNow_bcnTxTimer = false;
            // wtime = startTime - (now - bcnRxTime) - 12.0/phy_symbolrate;
            tmpf = now - bcnRxTime;
            tmpf = startTime - tmpf;
            wtime = tmpf - aTurnaroundTime/phy_symbolrate;
            ASSERT(wtime >= 0.0);

            if (bcnTxTimer->isScheduled())
                cancelEvent(bcnTxTimer);
            scheduleAt(now + wtime, bcnTxTimer);
            return;
        }
    }

    if (!txNow_bcnTxTimer)
    {
        if (bcnTxTimer->isScheduled())
            cancelEvent(bcnTxTimer);
        scheduleAt(now + aTurnaroundTime/phy_symbolrate, bcnTxTimer);
    }
    else if (mpib.macBeaconOrder != 15)
    {
        wtime = (aBaseSuperframeDuration * (1 << mpib.macBeaconOrder) - aTurnaroundTime) / phy_symbolrate;
        if (bcnTxTimer->isScheduled())
            cancelEvent(bcnTxTimer);
        scheduleAt(now + wtime, bcnTxTimer);
    }

    txNow_bcnTxTimer = (!txNow_bcnTxTimer);
}

void Ieee802154Mac::startAckTimeoutTimer()
{
    if (ackTimeoutTimer->isScheduled())
        cancelEvent(ackTimeoutTimer);
    scheduleAt(simTime() + mpib.macAckWaitDuration/phy_symbolrate, ackTimeoutTimer);
}

void Ieee802154Mac::startTxAckBoundTimer(simtime_t wtime)
{
    if (txAckBoundTimer->isScheduled())
        cancelEvent(txAckBoundTimer);
    scheduleAt(simTime() + wtime, txAckBoundTimer);
}

void Ieee802154Mac::startTxCmdDataBoundTimer(simtime_t wtime)
{
    if (txCmdDataBoundTimer->isScheduled())
        cancelEvent(txCmdDataBoundTimer);
    scheduleAt(simTime() + wtime, txCmdDataBoundTimer);
}

void Ieee802154Mac::startIfsTimer(bool sifs)
{
    simtime_t wtime;
    if (sifs)                   // SIFS
        wtime = aMinSIFSPeriod/phy_symbolrate;
    else                                // LIFS
        wtime = aMinLIFSPeriod/phy_symbolrate;

    if (ifsTimer->isScheduled())
        cancelEvent(ifsTimer);
    scheduleAt(simTime() + wtime, ifsTimer);
}

void Ieee802154Mac::startTxSDTimer()
// this function should be called when txing a new beacon is received in <handleBcnTxTimer()>
{
    ASSERT(!inTxSD_txSDTimer);
    simtime_t wtime;
    // calculate CAP duration
    wtime = aNumSuperframeSlots * txSfSlotDuration / phy_symbolrate;

    if (txSDTimer->isScheduled())
        cancelEvent(txSDTimer);
    scheduleAt(simTime() + wtime, txSDTimer);
    inTxSD_txSDTimer = true;
}

void Ieee802154Mac::startRxSDTimer()
// this function should be called when a  new beacon is received in <handleBeacon()>
{
    ASSERT(!inRxSD_rxSDTimer);
    simtime_t wtime;
    // calculate the remaining time in current CAP
    wtime = aNumSuperframeSlots * rxSfSlotDuration / phy_symbolrate;
    wtime = wtime + bcnRxTime - simTime();
    ASSERT(wtime>0.0);

    if (rxSDTimer->isScheduled())
        cancelEvent(rxSDTimer);
    scheduleAt(simTime() + wtime, rxSDTimer);
    //EV << "The current SD will end at " << simTime() + wtime << endl;
    inRxSD_rxSDTimer = true;
}

//--------------------------------------------------------------------------------/
/******************************* <Timer Handler> ********************************/
//--------------------------------------------------------------------------------/
void Ieee802154Mac::handleBackoffTimer()
{
    EV << "[CSMA-CA]: before sending CCA primative, tell PHY to turn on the receiver" << endl;
    taskP.taskStatus(TP_RX_ON_CSMACA) = true;
    PLME_SET_TRX_STATE_request(phy_RX_ON);
}

void Ieee802154Mac::handleDeferCCATimer()
{
    taskP.taskStatus(TP_CCA_CSMACA) = true;
    EV << "[MAC-TASK]: TP_CCA_CSMACA = true" << endl;
    PLME_CCA_request();
}

void Ieee802154Mac::handleBcnRxTimer()
{
    if (rxBO != 15)     //beacon enabled (do nothing if beacon not enabled)
    {
        if (txAck)
        {
            delete txAck;
            txAck = NULL;
        }
        //enable the receiver
        PLME_SET_TRX_STATE_request(phy_RX_ON);

        if (bcnLossCounter != 0)
        {
            numCollision++;//
        }
        bcnLossCounter++;
        startBcnRxTimer();
    }
}

void Ieee802154Mac::handleBcnTxTimer()
{
    FrameCtrl frmCtrl;
    /*TRANSACLINK *tmp;
    int i;*/

    if (mpib.macBeaconOrder != 15)      //beacon enabled
        if (!txNow_bcnTxTimer)          //enable the transmitter
        {
            beaconWaitingTx = true;
            PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);  //finish your job before this!
            if (txAck)      // transmitting beacon has highest priority
            {
                delete txAck;
                txAck = NULL;
            }
            PLME_SET_TRX_STATE_request(phy_TX_ON);
        }
        else        // it's time to transmit beacon
        {
            ASSERT(txBeacon == NULL);
            //--- construct a beacon ---
            Ieee802154BeaconFrame* tmpBcn = new Ieee802154BeaconFrame();
            tmpBcn->setName("Ieee802154BEACON");

            // construct frame control field
            frmCtrl.frmType = Ieee802154_BEACON;
            frmCtrl.secu = secuBeacon;
            frmCtrl.frmPending = false; // TBD: indirect trans
            frmCtrl.ackReq = false;         // ignored upon reception
            frmCtrl.intraPan = false;       // ignored upon reception
            frmCtrl.dstAddrMode = defFrmCtrl_AddrModeNone; // 0x00, ignored upon reception
            if (mpib.macShortAddress == 0xfffe)
            {
                frmCtrl.srcAddrMode = defFrmCtrl_AddrMode64;
                tmpBcn->setSrcAddr(aExtendedAddress);
            }
            else
            {
                frmCtrl.srcAddrMode = defFrmCtrl_AddrMode16;
                tmpBcn->setSrcAddr(mpib.macShortAddress);
            }

            tmpBcn->setFrmCtrl(frmCtrl);
            tmpBcn->setBdsn(mpib.macBSN++);
            tmpBcn->setDstPanId(0);     // ignored upon reception
            tmpBcn->setDstAddr(0);  // ignored upon reception
            tmpBcn->setSrcPanId(mpib.macPANId);

            // construct superframe specification
            txSfSpec.BO = mpib.macBeaconOrder;
            txSfSpec.BI = aBaseSuperframeDuration * (1 << mpib.macBeaconOrder);
            txSfSpec.SO = mpib.macSuperframeOrder;
            txSfSpec.SD = aBaseSuperframeDuration * (1 << mpib.macSuperframeOrder);
            txSfSpec.battLifeExt = mpib.macBattLifeExt;
            txSfSpec.panCoor = isPANCoor;
            txSfSpec.assoPmt = mpib.macAssociationPermit;

            // this parameter may vary each time when new GTS slots were allocated in last superframe
            txSfSpec.finalCap = tmp_finalCap;
            tmpBcn->setSfSpec(txSfSpec);

            // TBD
            //populate the GTS fields -- more TBD when considering GTS
            //tmpBcn->setGtsFields(gtsFields);
            //pendingAddrFields
            //tmpBcn->setPaFields(txPaFields);

            tmpBcn->setByteLength(calFrmByteLength(tmpBcn));

            txBeacon = tmpBcn;      // released in taskSuccess or in PD_DATA_confirm (if tx failure)
            txPkt = tmpBcn;
            mpib.macBeaconTxTime = SIMTIME_DBL(simTime());      // no delay
            sendDown(check_and_cast<Ieee802154Frame *>(txBeacon->dup()));
            startTxSDTimer();

            // schedule for GTS if they exist
            if (gtsCount > 0)
            {
                index_gtsTimer = gtsCount - 1;  // index of GTS in the list that will come first
                gtsScheduler();
            }
        }
    startBcnTxTimer();  //don't disable this even beacon not enabled (beacon may be temporarily disabled like in channel scan, but it will be enabled again)
}

void Ieee802154Mac::handleAckTimeoutTimer()
{
    //ASSERT(txBcnCmd||txBcnCmdUpper||txData);
    bool error = true;
    if (txBcnCmd)
        error = false;
    else if (txBcnCmdUpper)
        error = false;
    else if (txData)
        error = false;
    else if (txGTS && waitGTSAck)
        error = false;
    if (error)
        opp_error("handleAckTimeoutTimer");

    dispatch(phy_BUSY,__FUNCTION__);    //the status p_BUSY will be ignore
}

void Ieee802154Mac::handleTxAckBoundTimer()
{
    if (!beaconWaitingTx)
        if (txAck)      //<txAck> may have been cancelled by <macBeaconRxTimer>
        {
            txPkt = txAck;
            sendDown(check_and_cast<Ieee802154Frame *>(txAck->dup()));
        }
}


void Ieee802154Mac::handleTxCmdDataBoundTimer()
// called when txCmdDataBoundTimer expires or directly by <handle_PLME_SET_TRX_STATE_confirm()>
// data or cmd is sent to Phy layer here
{
    //int i;

    // TBD: TP_mlme_scan_request
    /*
    if (taskP.taskStatus(TP_mlme_scan_request))
    if (txBcnCmd2 != txCsmaca)
        return;         //terminate all other transmissions (will resume afte channel scan)
    */

#ifdef test_802154_INDIRECT_TRANS
    // need to update first to see if this transaction expired already
    if (check_and_cast<Ieee802154Frame *>(txCsmaca)->indirect)
    {
        i = updateTransacLinkByPktOrHandle(tr_oper_est,&transacLink1,&transacLink2,txCsmaca); // if expired, txCsmaca will be deleted here
        if (i != 0) //transaction expired
        {
            resetTRX();
            if (txBcnCmd == txCsmaca)
                txBcnCmd = NULL;
            else if (txBcnCmd2 == txCsmaca)
                txBcnCmd2 = NULL;
            else if (txData == txCsmaca)
                txData = NULL;
            //Packet::free(txCsmaca);   //don't do this, since the packet will be automatically deleted when expired
            csmacaResume();
            return;
        }
    }
#endif

    if (txBcnCmd == txCsmaca)
    {
        txPkt = txBcnCmd;
        sendDown(check_and_cast<Ieee802154Frame *>(txBcnCmd->dup()));
    }
    else if (txBcnCmdUpper == txCsmaca)
    {
        txPkt = txBcnCmdUpper;
        sendDown(check_and_cast<Ieee802154Frame *>(txBcnCmdUpper->dup()));
    }
    else if (txData == txCsmaca)
    {
        txPkt = txData;
        sendDown(check_and_cast<Ieee802154Frame *>(txData->dup()));
    }
}

void Ieee802154Mac::handleIfsTimer()
// this function is called after delay of ifs following sending out the ack requested by reception of a data or cmd pkt
// or following reception of a data pkt requiring no ACK
// other cmd pkts requirng no ACK are proccessed in <handleCommand()>
{
    FrameCtrl frmCtrl;
    MACenum status;
    int i;

    ASSERT(rxData != NULL || rxCmd != NULL || txGTS != NULL);

    if (rxCmd)      // MAC command, all cmds requiring ACK are handled here
    {
    }
    else if (rxData)
    {
        MCPS_DATA_indication(rxData);
        rxData = NULL;
    }
    else if (txGTS)
    {
        ASSERT(taskP.taskStatus(TP_MCPS_DATA_REQUEST) && (taskP.taskStep(TP_MCPS_DATA_REQUEST) == 4));
        FSM_MCPS_DATA_request();
    }
}

void Ieee802154Mac::handleSDTimer()     // we must make sure that outgoing CAP and incoming CAP never overlap
{
    inTxSD_txSDTimer = false;
    inRxSD_rxSDTimer = false;

    // If PAN coordinator uses GTS, this is the end of CFP, reset indexCurrGts
    if (isPANCoor)
        indexCurrGts = 99;
    EV << "[MAC]: entering inactive period, turn off radio and go to sleep" << endl;
    if (!txAck)
    {
        PLME_SET_TRX_STATE_request(phy_TRX_OFF);
    }
    else
        numTxAckInactive++;
    /*if ((!waitBcnCmdAck) && (!waitBcnCmdUpperAck) && (!waitDataAck))      // not waiting for ACK
    {
        EV << "[MAC]: entering inactive period, turn off radio and go to sleep" << endl;
        PLME_SET_TRX_STATE_request(phy_TRX_OFF);
    }
    else
        EV << "[MAC]: entering inactive period, but can not go to sleep" << endl;*/
}

//-------------------------------------------------------------------------------/
/********************** <FSM and Pending Task Processing> ***********************/
//-------------------------------------------------------------------------------/
void Ieee802154Mac::taskSuccess(char type, bool csmacaRes)
{
    UINT_16 t_CAP;
    UINT_8 ifs;
    simtime_t tmpf;

    if (type == 'b')    //beacon
    {
        EV << "[MAC]: taskSuccess for sending beacon" << endl;
        /*if (!txBeacon)
        {
            assert(txBcnCmdUpper);
            txBeacon = txBcnCmdUpper;
            txBcnCmdUpper = 0;
        }*/
        //--- calculate CAP ---
        if (txBeacon->getByteLength() <= aMaxSIFSFrameSize)
            ifs = aMinSIFSPeriod;
        else
            ifs = aMinLIFSPeriod;

        // calculate <txBcnDuration>
        tmpf = calDuration(txBeacon) * phy_symbolrate;
        tmpf += ifs;
        txBcnDuration = (UINT_8)(SIMTIME_DBL(tmpf) / aUnitBackoffPeriod);
        if (fmod(tmpf,aUnitBackoffPeriod) > 0.0)
            txBcnDuration++;
        EV << "[MAC]: calculating txBcnDuration = " << (int)txBcnDuration << endl;

        t_CAP = (txSfSpec.finalCap + 1) * txSfSlotDuration / aUnitBackoffPeriod - txBcnDuration;

        if (t_CAP == 0)     // it's possible that there is no time left in current CAP after txing beacon
        {
            csmacaRes = false;
            PLME_SET_TRX_STATE_request(phy_TRX_OFF);
            EV << "[MAC]: no time left in current CAP after txing beacon, trying to turn off radio" << endl;
        }
        else
        {
            PLME_SET_TRX_STATE_request(phy_RX_ON);
            EV << "[MAC]: turn on radio receiver after txing beacon" << endl;
        }
        //CSMA-CA may be waiting for the new beacon
        if (backoffStatus == 99)
            csmacaTrxBeacon('t');
        //----------------------
        beaconWaitingTx = false;
        delete txBeacon;
        txBeacon = NULL;
        /*
        //send out delayed ack.
        if (txAck)
        {
            csmacaRes = false;
            plme_set_trx_state_request(p_TX_ON);
        }
        */
        numTxBcnPkt++;
    }
    else if (type == 'a')   //ack.
    {
        numTxAckPkt++;
        ASSERT(txAck);
        delete txAck;
        txAck = NULL;
    }
    /*else if (task == 'c') //command
    {
        ASSERT(txBcnCmd);
        //if it is a pending packet, delete it from the pending list
        updateTransacLinkByPktOrHandle(tr_oper_del,&transacLink1,&transacLink2,txBcnCmd);
        delete txBcnCmd;
        txBcnCmd = NULL;
    }
    else if (task == 'C')   //command
    {
        ASSERT(txBcnCmdUpper);
        delete txBcnCmdUpper;
        txBcnCmdUpper = NULL;
    }*/
    else if (type == 'd')   //data
    {
        ASSERT(txData);
        EV << "[MAC]: taskSuccess for sending : " << txData->getName() <<":#" << (int)txData->getBdsn() << endl;
        //Packet *p = txData;
        delete txData;
        txData = NULL;
        MCPS_DATA_confirm(mac_SUCCESS);
        numTxDataSucc++;
        // request another msg from ifq (if it exists), only when MAC is idle for data transmission
        if (!taskP.taskStatus(TP_MCPS_DATA_REQUEST))
            reqtMsgFromIFq();
        //if it is a pending packet, delete it from the pending list
        //updateTransacLinkByPktOrHandle(tr_oper_del,&transacLink1,&transacLink2,p);
    }
    else if (type == 'g')   // GTS
    {
        csmacaRes = false;
        ASSERT(txGTS);
        EV << "[GTS]: taskSuccess for sending : " << txGTS->getName() <<":#" << (int)txGTS->getBdsn() << endl;
        // if PAN coordinator, need to clear isTxPending in corresponding GTS descriptor
        if (isPANCoor)
        {
            ASSERT(gtsList[indexCurrGts].isTxPending);
            gtsList[indexCurrGts].isTxPending = false;
        }
        delete txGTS;
        txGTS = NULL;
        MCPS_DATA_confirm(mac_SUCCESS);
        numTxGTSSucc++;
        // request another msg from ifq (if it exists), only when MAC is idle for data transmission
        if (!taskP.taskStatus(TP_MCPS_DATA_REQUEST))
            reqtMsgFromIFq();
    }
    else
        ASSERT(0);

    if (csmacaRes)
        csmacaResume();
}

void Ieee802154Mac::taskFailed(char type, MACenum status, bool csmacaRes)
{
    if ((type == 'b')   //beacon
            || (type == 'a')    //ack.
            || (type == 'c'))   //command
    {
        ASSERT(0);  //we don't handle the above failures here
    }
    else if (type == 'C')   //command from Upper
    {
        delete txBcnCmdUpper;
        txBcnCmdUpper = NULL;
    }
    else if (type == 'd')   //data
    {
        ASSERT(txData);
        EV << "[MAC]: taskFailed for sending : " << txData->getName() <<":#" << (int)txData->getBdsn() << endl;
        delete txData;
        txData = NULL;
        numTxDataFail++;
        MCPS_DATA_confirm(status);
        if (csmacaRes)
            csmacaResume();
        // request another msg from ifq (if it exists), only when MAC is idle for data transmission
        if (!taskP.taskStatus(TP_MCPS_DATA_REQUEST))
            reqtMsgFromIFq();
    }
    else if (type == 'g')   // GTS
    {
        ASSERT(txGTS);
        EV << "[MAC]: taskFailed for sending : " << txGTS->getName() <<":#" << (int)txGTS->getBdsn() << endl;
        // if PAN coordinator, need to clear isTxPending in corresponding GTS descriptor
        if (isPANCoor)
        {
            ASSERT(gtsList[indexCurrGts].isTxPending);
            gtsList[indexCurrGts].isTxPending = false;
        }
        delete txGTS;
        txGTS = NULL;
        numTxGTSFail++;
        MCPS_DATA_confirm(status);
        // request another msg from ifq (if it exists), only when MAC is idle for data transmission
        if (!taskP.taskStatus(TP_MCPS_DATA_REQUEST))
            reqtMsgFromIFq();
    }
}

void Ieee802154Mac::checkTaskOverflow(Ieee802154MacTaskType task)
// check if there is a task of the specified type is now pending
{
    if (taskP.taskStatus(task))
        error("[MAC-TASK]: task overflow: %d!", task);
    else
    {
        taskP.taskStep(task) = 0;
        (taskP.taskFrFunc(task))[0] = 0;
    }
}

void Ieee802154Mac::FSM_MCPS_DATA_request(PHYenum pStatus, MACenum mStatus)
{
    Ieee802154MacTaskType task = TP_MCPS_DATA_REQUEST;
    Ieee802154TxOption txOption = taskP.mcps_data_request_TxOption;

    if (txOption == DIRECT_TRANS)
    {
        switch (taskP.taskStep(task))
        {
        case 0:
            // impossible happen here
            break;

        case 1:
            if (pStatus == phy_IDLE)        // CSMA/CA succeeds
            {
                taskP.taskStep(task)++;
                strcpy(taskP.taskFrFunc(task),"handle_PD_DATA_confirm");
                //enable the transmitter
                PLME_SET_TRX_STATE_request(phy_TX_ON);
            }
            else    // CSMA/CA reports channel access failure, should report to SSCS through MCPS_DATA_confirm in spec
                // here simply retry
            {
                csmacaResume();
            }
            break;

        case 2:
            if (txData->getFrmCtrl().ackReq)    //ack. required
            {
                taskP.taskStep(task)++;
                strcpy(taskP.taskFrFunc(task),"handleAck");
                //enable the receiver
                PLME_SET_TRX_STATE_request(phy_RX_ON);
                startAckTimeoutTimer();
                waitDataAck = true;
            }
            else        //assume success if ack. not required
            {
                taskP.taskStatus(task) = false; // task ends successfully
                EV << "[MAC-TASK-SUCCESS]: reset TP_MCPS_DATA_REQUEST" << endl;
                resetTRX();
                taskSuccess('d');
            }
            break;

        case 3:
            if (pStatus == phy_SUCCESS) //ack. received
            {
                EV << "[MAC]: ACK for " << txData->getName() << ":#" << (int)txData->getBdsn() << " received successfully" << endl;
                taskP.taskStatus(task) = false; // task ends successfully
                EV << "[MAC-TASK-SUCCESS]: reset TP_MCPS_DATA_REQUEST" << endl;
                waitDataAck = false;    // debug
                resetTRX();
                taskSuccess('d');
            }
            else                // time out when waiting for ack.
            {
                EV << "[MAC]: ACK timeout for " << txData->getName() << ":#" << (int)txData->getBdsn()  << endl;
                numDataRetry++;
                if (numDataRetry <= aMaxFrameRetries)
                {
                    EV << "[MAC]: starting " << numDataRetry << "th retry for " << txData->getName() << ":#" << (int)txData->getBdsn()  << endl;
                    taskP.taskStep(task) = 1;   // go back to step 1
                    strcpy(taskP.taskFrFunc(task),"csmacaCallBack");
                    waitDataAck = false;
                    csmacaResume();
                }
                else
                {
                    EV << "[MAC]: the max num of retries reached" << endl;
                    taskP.taskStatus(task) = false; //task fails
                    EV << "[MAC-TASK-FAIL]: reset TP_MCPS_DATA_REQUEST" << endl;
                    waitDataAck = false;        // debug
                    resetTRX();
                    taskFailed('d', mac_NO_ACK);
                }
            }
            break;

        default:
            break;
        }
    }
    else if (txOption == INDIRECT_TRANS)
    {
        // TBD
    }
    else if (txOption == GTS_TRANS)
    {
        switch (taskP.taskStep(task))
        {
        case 0:
            // impossible happen here
            break;

        case 1:
            // two possible callbacks, one from handleGtsTimer() at the starting of one GTS
            // the other directly from MCPS_DATA_request(), only possible for devices when receiving a data from upper layer during the GTS
            taskP.taskStep(task)++;
            strcpy(taskP.taskFrFunc(task),"handle_PD_DATA_confirm");
            // should transmit right now, since the timing is very strictly controlled in GTS,
            // we can simply use phy_FORCE_TRX_OFF and then phy_TX_ON to turn on transmitter immediately
            PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            // data will be sent to PHY in handle_PLME_SET_TRX_STATE_confirm()
            break;

        case 2: // data successfully transmitted
            if (txGTS->getFrmCtrl().ackReq) //ack. required
            {
                taskP.taskStep(task)++;
                strcpy(taskP.taskFrFunc(task),"handleAck");
                //enable the receiver
                EV << "[GTS]: data successfully transmitted, turn on radio and wait for ACK" << endl;
                PLME_SET_TRX_STATE_request(phy_RX_ON);
                startAckTimeoutTimer();
                waitGTSAck = true;
            }
            else        //assume success if ack. not required
            {
                EV << "[GTS]: data successfully transmitted, no ACK required, turn off radio now" << endl;
                PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                EV << "[GTS]: need to delay IFS before next GTS transmission can proceed" << endl;
                taskP.taskStep(task) = 4;
                strcpy(taskP.taskFrFunc(task),"handleIfsTimer");
                if (txGTS->getByteLength() <= aMaxSIFSFrameSize)
                    startIfsTimer(true);
                else
                    startIfsTimer(false);
            }
            break;

        case 3:
            if (pStatus == phy_SUCCESS) //ack. received
            {
                waitGTSAck = false;
                EV << "[GTS]: ACK for " << txGTS->getName() << ":#" << (int)txGTS->getBdsn() << " received successfully, turn off radio now" << endl;
                PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                EV << "[GTS]: need to delay IFS before next GTS transmission can proceed" << endl;
                taskP.taskStep(task)++;
                strcpy(taskP.taskFrFunc(task),"handleIfsTimer");
                if (txGTS->getByteLength() <= aMaxSIFSFrameSize)
                    startIfsTimer(true);
                else
                    startIfsTimer(false);
            }
            else        // time out when waiting for ack, normally impossible in GTS
            {
                EV << "[MAC]: ACK timeout for " << txGTS->getName() << ":#" << (int)txGTS->getBdsn()  << endl;
                numGTSRetry++;
                if (numGTSRetry <= aMaxFrameRetries)
                {
                    // retry in next GTS
                    EV << "[GTS]: retry in this GTS of next superframe, turn off radio now" << endl;
                    taskP.taskStep(task) = 1;   // go back to step 1
                    strcpy(taskP.taskFrFunc(task),"handleGtsTimer");
                    waitGTSAck = false;
                    // to avoid several consecutive PLME_SET_TRX_STATE_request are called at the same time, which may lead to error operation,
                    // use phy_FORCE_TRX_OFF to turn off radio, because PHY will not send back a confirm from it
                    PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                }
                else
                {
                    EV << "[GTS]: the max num of retries reached, task failed" << endl;
                    taskP.taskStatus(task) = false; //task fails
                    EV << "[MAC-TASK-FAIL]: reset TP_MCPS_DATA_REQUEST" << endl;
                    waitGTSAck = false;
                    // to avoid several consecutive PLME_SET_TRX_STATE_request are called at the same time, which may lead to error operation,
                    // use phy_FORCE_TRX_OFF to turn off radio, because PHY will not send back a confirm from it
                    PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                    taskFailed('g', mac_NO_ACK);
                }
            }
            break;

        case 4:
            taskP.taskStatus(task) = false; // task ends successfully
            EV << "[GTS]: GTS transmission completes successfully, prepared for next GTS request" << endl;
            taskSuccess('g');

        default:
            break;
        }
    }
    else
        error("[MAC]: undefined txOption: %d!", txOption);
}

void Ieee802154Mac::dispatch(PHYenum pStatus, const char *frFunc, PHYenum req_state, MACenum mStatus)
{
    FrameCtrl frmCtrl;
    UINT_8 ifs;
    bool isSIFS = false;
    //int i;

    /*****************************************/
    /************<csmacaCallBack()>**********/
    /***************************************/
    if (strcmp(frFunc,"csmacaCallBack") == 0)
    {
        if (txCsmaca == txBcnCmdUpper)
        {
        }
        else if (txCsmaca == txData)
        {
            ASSERT(taskP.taskStatus(TP_MCPS_DATA_REQUEST)
                   && (strcmp(taskP.taskFrFunc(TP_MCPS_DATA_REQUEST),frFunc) == 0));

            FSM_MCPS_DATA_request(pStatus); // mStatus ignored
        }
    }

    /*****************************************/
    /*******<handle_PD_DATA_confirm()>*******/
    /***************************************/
    else if (strcmp(frFunc,"handle_PD_DATA_confirm") == 0)  // only process phy_SUCCESS here
    {
        if (txPkt == txBeacon)
        {
            {
                resetTRX();
                taskSuccess('b');   // beacon transmitted successfully
            }
        }
        else if (txPkt == txAck)
        {
            if (rxCmd != NULL)      // ack for cmd
            {
                if (rxCmd->getByteLength() <= aMaxSIFSFrameSize)
                    isSIFS = true;
                startIfsTimer(isSIFS);
                resetTRX();
                taskSuccess('a');
            }
            else if (rxData != NULL)    //default handling (virtually the only handling needed) for <rxData>
            {
                if (rxData->getByteLength() <= aMaxSIFSFrameSize)
                    isSIFS = true;
                startIfsTimer(isSIFS);
                if (rxData->getIsGTS()) // received in GTS
                {
                    if (isPANCoor)
                    {
                        // the device may transmit more pkts in this GTS, turn on radio
                        PLME_SET_TRX_STATE_request(phy_RX_ON);
                    }
                    else
                    {
                        // PAN coordinator can transmit only one pkt to me in my GTS, turn off radio now
                        PLME_SET_TRX_STATE_request(phy_TRX_OFF);
                    }
                }
                else
                    resetTRX();
                taskSuccess('a');
            }
            else    //ack. for duplicated packet
            {
                resetTRX();
                taskSuccess('a');
            }
        }
        else if (txPkt == txData)
        {
            ASSERT((taskP.taskStatus(TP_MCPS_DATA_REQUEST))
                   && (strcmp(taskP.taskFrFunc(TP_MCPS_DATA_REQUEST),frFunc) == 0));

            frmCtrl = txData->getFrmCtrl();
            if (taskP.taskStatus(TP_MCPS_DATA_REQUEST))
            {
                FSM_MCPS_DATA_request(pStatus); // mStatus ignored
            }
        }
        else if (txPkt == txGTS)
        {
            ASSERT((taskP.taskStatus(TP_MCPS_DATA_REQUEST))
                   && (strcmp(taskP.taskFrFunc(TP_MCPS_DATA_REQUEST),frFunc) == 0));
            ASSERT(taskP.mcps_data_request_TxOption == GTS_TRANS);
            // hand over back to FSM
            FSM_MCPS_DATA_request(pStatus); // mStatus ignored
        }
        //else      //may be purged from pending list
    }

    /*****************************************/
    /**************<handleAck()>*************/
    /***************************************/
    else if (strcmp(frFunc,"handleAck") == 0)       //always check the task status if the dispatch comes from recvAck()
    {
        if (txPkt == txData)
        {
            if ((taskP.taskStatus(TP_MCPS_DATA_REQUEST))
                    && (strcmp(taskP.taskFrFunc(TP_MCPS_DATA_REQUEST),frFunc) == 0))
                FSM_MCPS_DATA_request(pStatus); // mStatus ignored
            else    //default handling for <txData>
            {
                if (taskP.taskStatus(TP_MCPS_DATA_REQUEST)) //seems late ACK received
                    taskP.taskStatus(TP_MCPS_DATA_REQUEST) = false;
                resetTRX();
                taskSuccess('d');
            }
        }
        else if (txPkt == txGTS)
        {
            if ((taskP.taskStatus(TP_MCPS_DATA_REQUEST))
                    && (strcmp(taskP.taskFrFunc(TP_MCPS_DATA_REQUEST),frFunc) == 0))
                FSM_MCPS_DATA_request(pStatus); // mStatus ignored
        }
    }

    /*****************************************/
    /*******<handleAckTimeoutTimer()>********/
    /***************************************/
    else if (strcmp(frFunc,"handleAckTimeoutTimer") == 0)   //always check the task status if the dispatch comes from a timer handler
    {
        if (txPkt == txData)
        {
            if ((!taskP.taskStatus(TP_MCPS_DATA_REQUEST))
                    || (strcmp(taskP.taskFrFunc(TP_MCPS_DATA_REQUEST),"handleAck") != 0))
                return;

            if (taskP.taskStatus(TP_MCPS_DATA_REQUEST))
                FSM_MCPS_DATA_request(phy_BUSY); // mStatus ignored, pStatus can be anything but phy_SUCCESS
        }
        else if (txPkt == txGTS)
        {
            ASSERT((taskP.taskStatus(TP_MCPS_DATA_REQUEST))
                   && (strcmp(taskP.taskFrFunc(TP_MCPS_DATA_REQUEST), "handleAck") == 0));
            ASSERT(taskP.mcps_data_request_TxOption == GTS_TRANS);
            FSM_MCPS_DATA_request(phy_BUSY); // mStatus ignored, pStatus can be anything but phy_SUCCESS
        }
    }
}

//--------------------------------------------------------------------------------/
/****************************** <Utility Functions> *****************************/
//-------------------------------------------------------------------------------/
int Ieee802154Mac::calFrmByteLength(Ieee802154Frame* frame)
{
    //EV << "[MAC]: calculating size of " << frame->getName() << endl;
    Ieee802154FrameType frmType = frame->getFrmCtrl().frmType;
    int byteLength; // MHR + MAC payload + MFR
    int MHRLength = calMHRByteLength(frame->getFrmCtrl().srcAddrMode + frame->getFrmCtrl().dstAddrMode);

    if (frmType == Ieee802154_BEACON)       // Fig 37
    {
        Ieee802154BeaconFrame* beaconFrm = check_and_cast<Ieee802154BeaconFrame *>(frame);
        //byteLength = MHRLength + 2 + (2+ gtsCount * 3) + (1 + beaconFrm->getPaFields().numShortAddr * 2 + beaconFrm->getPaFields().numExtendedAddr * 8) + 2;
        byteLength = MHRLength + 6;
    }
    else if (frmType == Ieee802154_DATA)    // Fig 45, MAC payload not included here, will be added automatically while encapsulation later on
    {
        //byteLength = MHRLength + 2;
        byteLength = 11 + 2;    // constant header length for we always use short address
    }
    else if (frmType == Ieee802154_ACK)
    {
        byteLength = SIZE_OF_802154_ACK;
    }
    else if (frmType == Ieee802154_CMD)
    {
        Ieee802154CmdFrame* cmdFrm = check_and_cast<Ieee802154CmdFrame *>(frame);
        switch (cmdFrm->getCmdType())
        {
        case    Ieee802154_ASSOCIATION_REQUEST: // Fig 48: MHR (17/23) + Payload (2) + FCS (2)
            byteLength = MHRLength + 4;
            break;

        case    Ieee802154_ASSOCIATION_RESPONSE:
            byteLength = SIZE_OF_802154_ASSOCIATION_RESPONSE;
            break;

        case    Ieee802154_DISASSOCIATION_NOTIFICATION:
            byteLength = SIZE_OF_802154_DISASSOCIATION_NOTIFICATION;
            break;

        case    Ieee802154_DATA_REQUEST:    // Fig 52: MHR (7/11/13/17) + Payload (1) + FCS (2)
            byteLength = MHRLength + 3;
            break;

        case    Ieee802154_PANID_CONFLICT_NOTIFICATION:
            byteLength = SIZE_OF_802154_PANID_CONFLICT_NOTIFICATION;
            break;

        case    Ieee802154_ORPHAN_NOTIFICATION:
            byteLength = SIZE_OF_802154_ORPHAN_NOTIFICATION;
            break;

        case    Ieee802154_BEACON_REQUEST:
            byteLength = SIZE_OF_802154_BEACON_REQUEST;
            break;

        case    Ieee802154_COORDINATOR_REALIGNMENT: // Fig 56: MHR (17/23) + Payload (8) + FCS (2)
            byteLength = MHRLength + 10;
            break;

        case    Ieee802154_GTS_REQUEST:
            byteLength = SIZE_OF_802154_GTS_REQUEST;
            break;

        default:
            error("[MAC]: cannot calculate the size of a MAC command frame with unknown type!");
            break;
        }
    }
    else
        error("[MAC]: cannot calculate the size of a MAC frame with unknown type!");

    //EV << "[MAC]: header size is " << MHRLength << " Bytes" << endl;
    //EV << "[MAC]: MAC frame size is " << byteLength << " Bytes" << endl;
    return byteLength;
}

int Ieee802154Mac::calMHRByteLength(UINT_8 addrModeSum)
{
    switch (addrModeSum)
    {
    case 0:
        return 3;
    case 2:
        return 7;
    case 3:
        return 13;
    case 4:
        return 11;
    case 5:
        return 17;
    case 6:
        return 23;
    default:
        error("[MAC]: impossible address mode sum!");
    }
    return 0;
}

simtime_t Ieee802154Mac::calDuration(Ieee802154Frame* frame)
{
    return (def_phyHeaderLength*8 + frame->getBitLength())/phy_bitrate;
}

double Ieee802154Mac::getRate(char bitOrSymbol)
{
    double rate;

    if (ppib.phyCurrentChannel == 0)
    {
        if (bitOrSymbol == 'b')
            rate = BR_868M;
        else
            rate = SR_868M;
    }
    else if (ppib.phyCurrentChannel <= 10)
    {
        if (bitOrSymbol == 'b')
            rate = BR_915M;
        else
            rate = SR_915M;
    }
    else
    {
        if (bitOrSymbol == 'b')
            rate = BR_2_4G;
        else
            rate = SR_2_4G;
    }
    return (rate*1000);
}

bool Ieee802154Mac::toParent(Ieee802154Frame* frame)
{
    if (((frame->getFrmCtrl().dstAddrMode == defFrmCtrl_AddrMode16)&&(frame->getDstAddr() == mpib.macCoordShortAddress))
            ||  ((frame->getFrmCtrl().dstAddrMode == defFrmCtrl_AddrMode64)&&(frame->getDstAddr() == mpib.macCoordExtendedAddress)))
        return true;
    else
        return false;
}

//-----------------------------------------------------------------------------------------------------------/
/****************************** <GTS Functions> *****************************/
//----------------------------------------------------------------------------------------------------------/
/**
 *  PAN coordiantor uses this function to schedule for starting of each GTS in the GTS list
 */
void Ieee802154Mac::gtsScheduler()
{
    UINT_8 t_SO;
    simtime_t w_time, tmpf;

    ASSERT(isPANCoor);
    tmpf = mpib.macBeaconTxTime + gtsList[index_gtsTimer].startSlot * txSfSlotDuration / phy_symbolrate;
    w_time = tmpf - simTime();

    // should turn on radio receiver aTurnaroundTime symbols berfore GTS starts, if it is a transmit GTS relative to device
    if (!gtsList[index_gtsTimer].isRecvGTS)
        w_time = w_time - aTurnaroundTime/phy_symbolrate;

    EV << "[GTS]: schedule for starting of GTS index: " << index_gtsTimer << ", slot:#" << (int)gtsList[index_gtsTimer].startSlot << " with " << w_time << " s" << endl;
    startGtsTimer(w_time);
}

void Ieee802154Mac::startGtsTimer(simtime_t w_time)
{
    if (gtsTimer->isScheduled())
        cancelEvent(gtsTimer);
    scheduleAt(simTime() + w_time, gtsTimer);
}

void Ieee802154Mac::handleGtsTimer()
{
    simtime_t w_time;

    // for PAN coordinator
    if (isPANCoor)
    {
        indexCurrGts = index_gtsTimer;
        EV << "[GTS]: GTS with index = " << (int)indexCurrGts << " in my GTS list starts now!" << endl;
        EV << "allocated for MAC address = " << (int)gtsList[indexCurrGts].devShortAddr << ", startSlot = " << (int)gtsList[indexCurrGts].startSlot << ", length = " << (int)gtsList[indexCurrGts].length << ", isRecvGTS = " << gtsList[indexCurrGts].isRecvGTS << ", isTxPending = " << gtsList[indexCurrGts].isTxPending << endl;

        // is transmit GTS relative to device , turn on radio receiver
        if (!gtsList[indexCurrGts].isRecvGTS)
        {
            EV << "[GTS]: tell PHY to turn on radio receiver and prepare for receiving pkts from device" << endl;
            PLME_SET_TRX_STATE_request(phy_RX_ON);
        }
        // my time to transmit pkts to certain device
        else
        {
            // if there is a data buffered for transmission in current GTS
            if (txGTS && gtsList[indexCurrGts].isTxPending)
            {
                ASSERT(taskP.taskStatus(TP_MCPS_DATA_REQUEST) && taskP.taskStep(TP_MCPS_DATA_REQUEST) == 1);
                // hand over to FSM, which will go to next step
                // no need to call gtsCanProceed() at this time, timing is already guaranteed when allocating GTS
                EV << "[GTS]: a data pending for this GTS found in the buffer, starting GTS transmission now" << endl;
                FSM_MCPS_DATA_request();    // state parameters are ignored
            }
            // turn off radio
            else
            {
                EV << "[GTS]: no data pending for current transmit GTS, turn off radio now" << endl;
                PLME_SET_TRX_STATE_request(phy_TRX_OFF);
            }
        }

        if (index_gtsTimer > 0)
        {
            index_gtsTimer--;
            gtsScheduler();
        }
        // index_gtsTimer == 0, now is the starting of the last GTS in current CFP. need to do nothing here
        // at the end of CFP, indexCurrGts will be reset in handleSDTimer()
    }

    // for devices
    else
    {
        // my GTS starts
        if (index_gtsTimer != 99)
        {
            EV << "[GTS]: my GTS starts now, isRecvGTS = " << isRecvGTS << endl;

            // is receive GTS, turn on radio receiver
            if (isRecvGTS)
            {
                EV << "[GTS]: tell PHY to turn on radio receiver and prepare for receiving pkts from PAN coordinator" << endl;
                PLME_SET_TRX_STATE_request(phy_RX_ON);
            }
            // my GTS to transmit pkts to PAN coordinator
            else
            {
                // each device can have at most one GTS
                if (txGTS)
                {
                    ASSERT(taskP.taskStatus(TP_MCPS_DATA_REQUEST) && taskP.taskStep(TP_MCPS_DATA_REQUEST) == 1);
                    // hand over to FSM, which will go to next step
                    // no need to call gtsCanProceed() at this time, timing is already guaranteed when applying for GTS
                    EV << "[GTS]: a data is pending for this GTS in the buffer, starting GTS transmission now" << endl;
                    FSM_MCPS_DATA_request();    // state parameters are ignored
                }
                else
                {
                    EV << "[GTS]: no data pending for GTS transmission, turn off radio now" << endl;
                    // to avoid several consecutive PLME_SET_TRX_STATE_request are called at the same time, which may lead to error operation,
                    // use phy_FORCE_TRX_OFF to turn off radio, because PHY will not send back a confirm from it
                    PLME_SET_TRX_STATE_request(phy_FORCE_TRX_OFF);
                    // if data from upper layer arrives during this GTS, radio may be turned on again
                }
            }

            // schdule for the end of my GTS slot in order to put radio into sleep when GTS of current CFP expires
            index_gtsTimer = 99;
            // calculate the duration of my GTS
            w_time = gtsLength * rxSfSlotDuration / phy_symbolrate;
            if (isRecvGTS)
                w_time += aTurnaroundTime/phy_symbolrate;
            EV << "[GTS]: scheduling for the end of my GTS slot" << endl;
            startGtsTimer(w_time);
        }
        // my GTS expired, turn off radio
        else
        {
            index_gtsTimer = 0;
            EV << "[GTS]: now is the end of my GTS, turn off radio now" << endl;
            PLME_SET_TRX_STATE_request(phy_TRX_OFF);
        }
    }
}

/**
 *  This function accepts GTS request from devices and allocates GTS slots
 *  The return value indicates the GTS start slot for corresponding device
 *  Note: devices are allowed to call this function only in CAP
 */
UINT_8 Ieee802154Mac::gts_request_cmd(UINT_16 macShortAddr, UINT_8 length, bool isReceive)
{
    Enter_Method_Silent();
    UINT_8 startSlot;
    UINT_32 t_cap;
    ASSERT(isPANCoor);
    EV << "[GTS]: the PAN coordinator is processing GTS request from MAC address " << macShortAddr << endl;
    // check whether this device is device list
    if (deviceList.find(macShortAddr) == deviceList.end())
    {
        EV << "[GTS]: the address " << macShortAddr << " not found in the device list, no GTS allocated" << endl;
        return 0;
    }
    else if (gtsCount >= 7)
    {
        EV << "[GTS]: max number of GTSs reached, no GTS available for the MAC address " << macShortAddr << endl;
        return 0;
    }

    // check if the min CAP length can still be satisfied after this allocation
    t_cap = txSfSlotDuration * (tmp_finalCap - length + 1);
    if (t_cap < aMinCAPLength)
    {
        EV << "[GTS]: violation of the min CAP length, no GTS allocated for the MAC address " << macShortAddr << endl;
        return 0;
    }

    // update final CAP and calculate start slot for this GTS
    tmp_finalCap = tmp_finalCap - length;
    startSlot = tmp_finalCap + 1;
    // add new GTS descriptor
    gtsList[gtsCount].devShortAddr = macShortAddr;
    gtsList[gtsCount].startSlot = startSlot;
    gtsList[gtsCount].length = length;
    gtsList[gtsCount].isRecvGTS = isReceive;
    EV << "[GTS]: add a new GTS descriptor with index = " << (int)gtsCount << " for MAC address = " << macShortAddr << ", startSlot = " <<  (int)startSlot << ", length = " << (int)length << ", isRecvGTS = " << isReceive << endl;
    gtsCount++;
    // new paramters put into effect when txing next beacon
    //isGtsUpdate = true;
    return  startSlot;
}

void Ieee802154Mac::startFinalCapTimer(simtime_t w_time)
{
    if (finalCAPTimer->isScheduled())
        cancelEvent(finalCAPTimer);
    scheduleAt(simTime() + w_time, finalCAPTimer);
}

void Ieee802154Mac::handleFinalCapTimer()
{
    // only be called when my GTS is not the first one in the CFP
    ASSERT(gtsStartSlot > rxSfSpec.finalCap + 1);
    EV << "[GTS]: it's now the end of CAP, turn off radio and wait for my GTS coming" << endl;
    PLME_SET_TRX_STATE_request(phy_TRX_OFF);
}

UINT_16 Ieee802154Mac::associate_request_cmd(IE3ADDR extAddr, const DevCapability& capaInfo)
{
    Enter_Method_Silent();
    // store MAC address and capability info
    // here simply assign short address with its extended address
    deviceList[extAddr] = capaInfo;
    EV << "[MAC]: associate request received from " << capaInfo.hostName << " with MAC address: " << extAddr << endl;
    return extAddr;
}

/**
 *  Devices should check if the requested GTS transaction can be finished  before the end of corresponding GTS
 *  no need for PAN coordinator to do this, because it only transmits at the starting of the GTS,
 *  of which the timing is already guaranteed by device side when requesting for receive GTS
 */
bool Ieee802154Mac::gtsCanProceed()
{
    simtime_t t_left;
    EV << "[GTS]: checking if the data transaction can be finished befor the end of current GTS ... ";
    // to be called only by the device in its GTS
    ASSERT(index_gtsTimer == 99 && gtsTimer->isScheduled());
    // calculate left time in current GTS
    t_left = gtsTimer->getArrivalTime() - simTime();

    if (t_left > gtsTransDuration)
    {
        EV << "yes" << endl;
        return true;
    }
    else
    {
        EV << "no" << endl;
        return false;
    }
}

