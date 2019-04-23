#ifndef __TCP_CLIENT_SIDE_SERVICE_H_
#define __TCP_CLIENT_SIDE_SERVICE_H_

#include <omnetpp.h>
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/icancloud/Base/Messages/icancloud_Message.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/INETDefs.h"

namespace inet {

namespace icancloud {


using namespace inet;

class NetworkService;

class TCP_ClientSideService : public TcpSocket::ICallback{
	
   /**
	* Structure that represents a connection between two TCP applications (client-side).
	*/
	struct icancloud_TCP_Client_Connector{
		TcpSocket *socket;
		Packet *msg;
	};
	typedef struct icancloud_TCP_Client_Connector clientTCP_Connector;


	protected:
		
		/** Client connections vector*/
		vector <clientTCP_Connector> connections;
		     
	    /** Socket map to manage TCP sockets*/
		SocketMap socketMap;
	    	    
	    /** Pointer to NetworkService object */
	    NetworkService *networkService;
	    
	    /** Local IP */
	    string localIP;	    	    
	    
	    /** Output gate to TCP */
	    cGate* outGate_TCP;	    

	public:
  
	  	/**
	  	 * Constructor.
	  	 * @param newLocalIP Local IP	  	 
	  	 * @param toTCP Gate to TCP module
	  	 * @param netService NetworkService object
	  	 */
	    TCP_ClientSideService(string newLocalIP,    						 
	    						cGate* toTCP,
	    						NetworkService *netService);
	    
	    /**
	     * Destructor
	     */
	    virtual ~TCP_ClientSideService();	    
	    
	    /**
	     * Create a connection with server.
	     * @param sm Message that contains the data to establish connection with server.
	     */
	    void createConnection(Packet *);
	    
	    /**
	     * Send a packet to corresponding destination.
	     * @param sm Message to send to the corresponding server.
	     */
	    void sendPacketToServer(Packet *);
	       
	    /**
	     * Close a connection.	     
	     */
	    void closeConnection(Packet *);
	    
	    /**
	     * Search a connection by ID.
	     * @param connId Connection ID.
	     * @return Index of the connection in the connection vector if this connection exists, or
	     * NOT_FOUND in other case.
	     */
	     int searchConnectionByConnId(int connId);
	     
	     /**
	      * Calculates if a given message correspond to an outcomming connection.
	      * @param msg Message.
	      * @return Corresponding TCPSocket if msg is involved in an existing connection or
	      * nullptr in other case.
	      */
	     TcpSocket* getInvolvedSocket (cMessage *msg);
    
    
	protected:    
	     virtual void socketDataArrived(TcpSocket* socket, Packet *packet, bool urgent) override;
	     virtual void socketEstablished(TcpSocket *socket) override;
	     virtual void socketPeerClosed(TcpSocket *socket) override;
	     virtual void socketClosed(TcpSocket *socket) override;
	     virtual void socketFailure(TcpSocket *socket, int code) override;
	     virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override { socket->accept(availableInfo->getNewSocketId()); }
	     virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override { }
	     virtual void socketDeleted(TcpSocket *socket) override {}
	    // TCPSocket::CallbackInterface methods
	    /*virtual void socketEstablished(int connId, void *yourPtr);
	    virtual void socketDataArrived(int connId, void *yourPtr, Packet *msg, bool urgent);
	    virtual void socketPeerClosed(int connId, void *yourPtr);
	    virtual void socketClosed(int connId, void *yourPtr);    
	    virtual void socketFailure(int connId, void *yourPtr, int code);
	    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
	    */
	    // Debug methods	    
	    void printSocketInfo (inet::TcpSocket *socket);
};

} // namespace icancloud
} // namespace inet

#endif


