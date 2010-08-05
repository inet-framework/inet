#pragma once

#include "BGPCommon.h"
#include "TCPSocket.h"
#include "BGPRouting.h"
#include "BGPFSM.h"
#include <omnetpp.h>
#include <vector>

class  INET_API BGPSession : public cPolymorphic
{
public:
	BGPSession(BGPRouting& _bgpRouting); 

	virtual ~BGPSession();

	void			startConnection();	
	void			restartsHoldTimer();
	void			restartsKeepAliveTimer();
	void			restartsConnectRetryTimer(bool start = true);
	void			sendOpenMessage();
	void			sendKeepAliveMessage();	
	void			addUpdateMsgSent()							{ _updateMsgSent ++;}
	void			listenConnectionFromPeer()					{ _bgpRouting.listenConnectionFromPeer(_info.sessionID);}
	void			openTCPConnectionToPeer()					{ _bgpRouting.openTCPConnectionToPeer(_info.sessionID);}
	BGP::SessionID	findAndStartNextSession(BGP::type type)		{ return _bgpRouting.findNextSession(type, true);}
	//setters pour pouvoir créer et modifier les informations de la session dans BGPRouting
	void			setInfo(BGP::SessionInfo info);
	void			setTimers(simtime_t* delayTab);
	void			setlinkIntf(InterfaceEntry* intf)			{ _info.linkIntf = intf;}
	void			setSocket(TCPSocket* socket)				{ _info.socket = socket;}
	void			setSocketListen(TCPSocket* socket)			{ _info.socketListen = socket;}
	//getters pour avoir accès aux informations de la session
	void			getStatistics(unsigned int* statTab);
	bool			isEstablished()								{ return _info.sessionEstablished;}
	BGP::SessionID	getSessionID()								{ return _info.sessionID;}
	BGP::type		getType()									{ return _info.sessionType;}
	InterfaceEntry*	getLinkIntf()								{ return _info.linkIntf;}
	IPAddress		getPeerAddr()								{ return _info.peerAddr;}
	TCPSocket*		getSocket()									{ return _info.socket;}
	TCPSocket*		getSocketListen()							{ return _info.socketListen;}
	IRoutingTable*	getIPRoutingTable()							{ return _bgpRouting.getIPRoutingTable();}
	std::vector<BGP::RoutingTableEntry*> getBGPRoutingTable()	{ return _bgpRouting.getBGPRoutingTable();}
	Macho::Machine<BGPFSM::TopState>&	 getFSM()				{ return *_fsm;}
	bool checkExternalRoute(const IPRoute* ospfRoute)			{ return _bgpRouting.checkExternalRoute(ospfRoute);}
	void updateSendProcess(BGP::RoutingTableEntry* entry)		{ return _bgpRouting.updateSendProcess(BGP::NEW_SESSION_ESTABLISHED, _info.sessionID, entry);}
	
	
private:
	BGP::SessionInfo	_info;
	BGPRouting&			_bgpRouting;
	
	static const int	BGP_RETRY_TIME				= 120 ;
	static const int	BGP_HOLD_TIME				= 180 ;
	static const int	BGP_KEEP_ALIVE				= 60 ;//1/3 of BGP_HOLD_TIME
	static const int	NB_SEC_START_EGP_SESSION	= 1 ;

	//Timers
	simtime_t		_StartEventTime;
	cMessage *		_ptrStartEvent;
	unsigned int	_connectRetryCounter;
	simtime_t		_connectRetryTime;
	cMessage *		_ptrConnectRetryTimer;
	simtime_t		_holdTime;
	cMessage *		_ptrHoldTimer;
	simtime_t		_keepAliveTime;
	cMessage *		_ptrKeepAliveTimer;

	//Statistics
	unsigned int	_openMsgSent;	
	unsigned int	_openMsgRcv;
	unsigned int	_keepAliveMsgSent;
	unsigned int	_keepAliveMsgRcv;
	unsigned int	_updateMsgSent;
	unsigned int	_updateMsgRcv;


	//FINAL STATE MACHINE
	BGPFSM::TopState::Box*				_box;
	Macho::Machine<BGPFSM::TopState>*	_fsm;

	friend struct BGPFSM::Idle;
	friend struct BGPFSM::Connect;
	friend struct BGPFSM::Active;
	friend struct BGPFSM::OpenSent;
	friend struct BGPFSM::OpenConfirm;
	friend struct BGPFSM::Established;
};

