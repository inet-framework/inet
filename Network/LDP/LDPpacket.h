#ifndef _LDPPACKET_H_
#define _LDPPACKET_H_

#include <string>
#include <omnetpp.h>
#include <iostream>
#include <vector>
#include "IPAddress.h"
#include "ip_address_prefix.h"

using namespace std;


/*
**TLV Specification
*/

const int MAX_ELE_NUM=10;
const int MAX_PATH_LENGTH=20;
const int MAX_FEC_NUM=10;


enum LDP_TLV_TYPES
{
       FEC=0x0100,
       LABEL=0x0200,
       ADDRESS_LIST=0x0101,
       HOP_COUNT=0x0103,
       PATH_VECTOR=0x0104
};

enum TLV_Fields
{
       TLV_FEC_ELE_TYPE,
       TLV_FEC_ELE_ADDRESS_FAMILY,
       TLV_FEC_ELE_LENGTH,
       TLV_FEC_ELE_VALUE,
       TLV_LABEL_VALUE,
       TLV_ADDRESS_FAMILY,
       TLV_ADDRESS,
       TLV_HOPCOUNT_VALUE,
       TLV_LSR_ID,
       TLV_STATUS_CODE,
       TLV_MESSAGE_ID,
       TLV_MESSAGE_TYPE
};

//Ignore not neccessary fields for the simu


//typedef char TLV_STRING[20]; // A string value in tlv coding

struct TLV_HELLO
{
       long hold_time;
       bool Tbit;
       bool Rbit;
};
typedef struct TLV_FEC_ELEMENT
{
       string type;
       string family;
       string value;


} tlv_fec_type;

typedef struct LABEL_MAPPING
{
        tlv_fec_type fec;
        int label;
} label_mapping_type_unused;

typedef struct MAPPING
{
    IPAddressPrefix fec;
    int label;
} label_mapping_type;

/*
** LDP message specification
*/


enum LDP_MESSAGE_TYPES
{
       NOTIFICATION=10,
       HELLO,
       INITIALIZATION,
       KEEP_ALIVE,
       ADDRESS,
       ADDRESS_WITHDRAW,
       LABEL_MAPPING,
       LABEL_REQUEST,
       LABEL_WITHDRAW,
       LABEL_RELEASE,
       UNKNOWN
};




/********************************************************************************
*                                                                                *
*                                LDP GENERIC PACKET                                *
*                                                                                *
*********************************************************************************/

class LDPpacket: public cPacket
{
    public:

      int type;
      LDPpacket();
      LDPpacket(int messageType);
      LDPpacket(const LDPpacket& ldp);
      virtual const char *classname() const {return "LDPpacket";}

      //Override encapsulation and decapsulation for cPacket
      virtual void encapsulate(cPacket *);
      virtual cPacket *decapsulate();




      //Routing Information
    inline void setSenderAddress(int ipAddr)
{
          if(!hasPar("src_lsr_addr"))
          addPar("src_lsr_addr") = ipAddr;
          else
              par("src_lsr_addr")=ipAddr;
}

inline void setReceiverAddress(int ipAddr)
{
    if(!hasPar("peerIP"))
          addPar("peerIP")=ipAddr;
      else
      par("peerIP")=ipAddr;
}


      inline int getSenderAddress()
                                {return par("src_lsr_addr").longValue();}

      inline int getReceiverAddress()
                                {return par("peerIP").longValue();}



      virtual void printInfo(ostream& os)=0;
      virtual int getType()=0;

};



/********************************************************************************
*                                                                                *
*                                LABEL MAPPING MESSAGE                           *
*                                                                                *
*********************************************************************************/

class LabelMappingMessage: public LDPpacket
{

    public:
    LabelMappingMessage();

    inline void setMapping(int label, int fec)
    {
        if(!hasPar("mlabel"))
            addPar("mlabel")=label;
        else
            par("mlabel")=label;

        if(!hasPar("mfec"))
            addPar("mfec")=fec;
        else
            par("mfec")=fec;
    }

    inline int  getLabel()
                        {return par("mlabel").longValue();}

    inline int getFec()
                        {return par("mfec").longValue();}

    void printInfo(ostream& os);

    int getType();

    ~LabelMappingMessage(){}

};




/********************************************************************************
*                                                                                *
*                                LABEL REQUEST MESSAGE                           *
*                                                                                *
*********************************************************************************/

class LabelRequestMessage: public LDPpacket
{

    public:
    LabelRequestMessage();

    inline void setFec(int fec){
        if(!hasPar("fec"))
        addPar("fec")=fec;
    else
        par("fec")=fec;}

    inline int getFec(){return par("fec").longValue();}

    void printInfo(ostream& os);

    int getType();

    ~LabelRequestMessage(){}
} ;





/********************************************************************************
*                                                                                *
*                                HELLO MESSAGE                                   *
*                                                                                *
*********************************************************************************/
class HelloMessage: public LDPpacket
{
     protected:
     double holdTime;
     bool Tbit;
     bool Rbit;

     public:
     HelloMessage();
     HelloMessage(double time, bool tbit, bool rbit);
     void printInfo(ostream & os);

     inline double getHoldTime(){return holdTime;}
     inline void setHoldTime(double newTime){holdTime=newTime;}
     inline bool getTbit(){return Tbit;}
     inline void setTbit(bool newT){Tbit=newT;}
     inline bool getRbit(){return Rbit;}
     inline void setRbit(bool newR){Rbit=newR;}
     int getType();

     ~HelloMessage(){}


};

class IniMessage: public LDPpacket
{
     protected:
     double KeepAliveTime;
     bool Abit;
     bool Dbit;
     int PVLim;
     string Receiver_LDP_Identifier;

     public:
     IniMessage();
     IniMessage(double time, bool abit, bool dbit, int pvlim, string r_ldp_id);
     inline double getKeepAliveTime(){return KeepAliveTime;}
     inline bool getAbit(){return Abit;}
     inline bool getDbit(){return Dbit;}
     inline string getR_LDP_ID(){return Receiver_LDP_Identifier;}
     void printInfo(ostream& os);
     int getType();
     ~IniMessage(){}


};

class AddressMessage: public LDPpacket
{
     protected:
     bool isWithdraw;
     string family;
     vector<string> *addresses;

     public:
     AddressMessage();

     AddressMessage(bool iswithdraw, string addFamily, vector<string> *addressList);
     inline vector<string> *getAddresses(){return addresses;}
     void printInfo(ostream& os);
     int getType();
     ~AddressMessage(){delete addresses;}
};


#endif
