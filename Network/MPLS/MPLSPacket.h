/***************************************************************************
*
*    This library is free software, you can redistribute it and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
***************************************************************************/


/*
*    File Name MPLSPacket.h
*    LDP library
*    This file defines the MPLSPacket class
**/

#ifndef __MPLSPacket_H
#define __MPLSPacket_H

#include <omnetpp.h>
#include <string>
#include <list>
#include <queue>

using namespace std;

//const char* NATIVE_IP_PACKET = "IP";
const int MPLS_HEADER_LENGTH = 32;

/* Different layer 2 technologies*/
enum MPLS_ProtocolFieldId
{
    MPLS_PROT_UNDEF,
    MPLS_PROT_IP,
    MPLS_PROT_ATM

};

typedef queue<int, list<int>  >    IQueue;

/*Packet header*/
struct mpls_hdr
{
  string    label;    //mpls label
  int        cos;    //class of service - reserved bits
  int        ttl;    //time to live
  int        s;        //stack bit

};

/*Packet routing info*/
struct PktInfo
{
  int  dstaddr;
  int  srcnode;
  int  dstnode;
  int  phb;
};



class MPLSPacket: public cPacket
{
  private:
    MPLS_ProtocolFieldId _protocol;
    IQueue label;
    int cos;
    int ttl;
    bool s;
    cMessage* data;


  public:
    /* constructors*/
    MPLSPacket();
    MPLSPacket(const MPLSPacket &p);

    /* assignment operator*/
    virtual MPLSPacket& operator=(const MPLSPacket& p);

    /*dup: clonning function
     *@param: none
     **/
    virtual cObject *dup() const { return new MPLSPacket(*this); }

    /*className: get class name
     *@param: none
     **/
    virtual const char *className() const { return "MPLSPacket"; }

    /*info: set packet info
     *@param: buf - The information
     **/
    virtual void info(char *buf);

    /*writeContents: Write out the packet content
     *@param: os - The ostream
     **/
    virtual void writeContents(ostream& os);

    /*encapsulate:    Encapsulate layer 3 packet
     *@param:    pck- The body to be encapsulated
     **/
    virtual void encapsulate(cMessage* pck){data = pck;}

    /*decapsulate:    Decapsulate the packet
     *@param: none
     **/
    virtual cMessage *decapsulate(){return data;}


    /*heaaderBitLength: Get the header lenght in bit
     *@param:    none
     **/
    int headerBitLength() { return 8 * MPLS_HEADER_LENGTH; }

    /*headerByteLength:    Get the header length in byte
     *@param: none
     **/
    int headerByteLength() { return MPLS_HEADER_LENGTH; }

    /*protocol:    Get the protocol field
     *@param:    none
     **/
    MPLS_ProtocolFieldId protocol() { return _protocol; }

    /*setProtocol:    Set the protocol field
     *@param: none
     **/
    void setProtocol( MPLS_ProtocolFieldId p ) { _protocol = p; }

    /*swapLabel:    Swap Label operation
     *@param: newLabel - The new Label to use
     **/
    inline void swapLabel(int newLabel){label.pop();label.push(newLabel);}

    /*pushLabel:    Push new label
     *@param: newLabel - The added label
     **/
    inline void pushLabel(int newLabel){label.push(newLabel);}

    /*popLabel:    Pop out top label
     *@param: none
     **/
    inline void popLabel(){label.pop();}

    /*noLabel:    Empty the label stack
     **/
    inline bool noLabel(){return label.empty(); }

    /*getLabel:    Get the top label
     *@param: none
     **/
    inline int getLabel(){return label.front();}

    /*getCOS:    Get the COS
    *@param: none
    **/
    inline int getCOS(){return cos;}

    /*SetCOS:    Set the COS
     *@param: newCos - The new cos
     **/
    inline void setCOS(int newCOS){cos = newCOS;} //not implemented

    /*getTTL: Get the Time to live
     *@param: none
     **/
    inline int getTTL() {return ttl;}

    /*setTTL:    Set Time to Live
     *@param: none
     **/
    inline void setTTL(int newTTL) {ttl = newTTL;}

    /*getSbit:    Get the S bit
     *@param: none
     **/
    inline int getSbit() {return s;}

    /*setSbit:    Set the S bit
     *@param newS - Th new S bit
     **/
    inline void setSbit(bool newS) {s = newS;}


};

#endif


