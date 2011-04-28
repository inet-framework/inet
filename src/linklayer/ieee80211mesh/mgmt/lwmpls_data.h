/* lwmpls_data.h                */
/* File that defines data structures and constants used */
/* for ligh wireles mpls.   */

/****************************************/
/****************************************/

/* Protect against multiple includes.   */
#ifndef _LWMPLS_DATA_H_INCLUDED_
#define _LWMPLS_DATA_H_INCLUDED_


/*-------------------------------------------------------------------*/

#include <omnetpp.h>
#include <map>
#include <vector>

#define LWMPLS_OUTPUT_LABEL 0
#define LWMPLS_INPUT_LABEL  1
#define LWMPLS_OUTPUT_LABEL_RETURN 2
#define LWMPLS_INPUT_LABEL_RETURN  3
#define LWMPLS_MAX_LABEL     1000
/* Tiempo para garantizar que si me encuentro un etiqueta */
/*libre esta tambien habr� sido liberada por el resto de los nodos */
#define LWMPLS_MIN_REUSE_TIME   LWMPLS_MAX_TIME*3

#define LWMPLS_EXTRACT  1
#define LWMPLS_CHANGE   0
#define LWMPLS_ACUMULATE   2



#define LWMPLS_STATUS_NOT_USE 0
#define LWMPLS_STATUS_STBL   1
#define LWMPLS_STATUS_PROC  2
#define LWMPLS_STATUS_NOT_INIT 3




typedef struct LWMPLSKey
{
    int label;
    uint64_t mac_addr;

    inline bool operator<(const LWMPLSKey& b) const
    {
        if (label!=b.label)
            return label<b.label;
        else
            return mac_addr<b.mac_addr;
    }

    inline LWMPLSKey & operator = (const LWMPLSKey& b)
    {
        label=b.label;
        mac_addr=b.mac_addr;
        return *this;
    }

    inline bool operator==(const LWMPLSKey& b) const
    {
        if ((label==b.label) && (mac_addr==b.mac_addr))
            return true;
        else
            return false;
    }
} LWMPLSKey;


typedef std::vector<uint64_t> RouteVector;

typedef struct   LWmpls_Forwarding_Structure : public cOwnedObject
{
    int     input_label;
    int     output_label;
    int     order;
    uint64_t     mac_address;
    uint64_t     input_mac_address;
    /* Return label, it's used in the return path */
    int     return_label_input;
    int     return_label_output;
    simtime_t   last_use;
    LWMPLSKey     key_output;
    LWMPLSKey     return_key_output;
    simtime_t     label_life_limit;
    RouteVector path;
    void keyOutput(int label, uint64_t mac_add) { key_output.label = label; key_output.mac_addr= mac_add;}
    void reverseKeyOutput (int label, uint64_t mac_add) { return_key_output.label = label; return_key_output.mac_addr= mac_add;}
    LWmpls_Forwarding_Structure()
    {
        mac_address=0;
        input_mac_address=0;
        last_use=-1;
        order=-1;
        output_label=0;
        input_label=0;
        return_label_input=0; // Acumular
        return_label_output=0;
        key_output.label=-1;
        key_output.mac_addr=0;
        return_key_output.label=-1;
        return_key_output.mac_addr=0;
        path.clear();

    }
    ~LWmpls_Forwarding_Structure()
    {
        path.clear();

    }
} LWmpls_Forwarding_Structure;

class LWMPLSDataStructure;

class LWmpls_Interface_Structure : public cOwnedObject
{
  private:
    uint64_t  mac_address;
    simtime_t   last_use;
    int      num_rtr;
    unsigned int      num_labels;
    friend   class LWMPLSDataStructure;
  public:
    LWmpls_Interface_Structure()
    {
        mac_address=0;
        last_use=0;
        num_rtr=0;
        num_labels=0;
    }
    uint64_t  &   macAddress() {return mac_address;}
    simtime_t  & lastUse() {return last_use;}
    int     & numRtr () {return num_rtr;}
    unsigned int & numLabels () {return num_labels;}

};

typedef struct LWmpls_label_list : public cOwnedObject
{
    simtime_t time;
    bool in_use;
    simtime_t capture_time;
    LWmpls_Forwarding_Structure* data_f_ptr;
    inline  LWmpls_label_list & operator = (const  LWmpls_label_list& b)
    {
        time=b.time;
        in_use=b.in_use;
        time=b.time;
        capture_time=b.capture_time;
        data_f_ptr=b.data_f_ptr;
        return *this;
    }
} LWmpls_label_list;

typedef std::map<LWMPLSKey, LWmpls_Forwarding_Structure*> LWmplsFwMap;
typedef std::map<uint64_t,LWmpls_Interface_Structure *> LWmplsInterfaceMap;
typedef std::map<uint64_t,int> DestinationList;
typedef std::map<uint64_t,uint32_t> BroadcastList;
typedef std::vector<LWmpls_label_list> LWmplslabelArray;



class Ieee80211Mesh;

class   LWMPLSDataStructure : public cOwnedObject
{
  private:
    // This structure use the for mac address like label key
    // Exist the possibility to use the for mac address frame like a label
    typedef struct LWMPSMACAddressKey
    {
        uint64_t srcAddr;
        uint64_t destAddr;
        uint64_t prevAddr;
        int32_t  label;
        inline bool operator<(const LWMPSMACAddressKey& b) const
        {
            if (srcAddr!=b.srcAddr)
                return srcAddr<b.srcAddr;
            else if (destAddr !=b.destAddr)
                return destAddr<b.destAddr;
            else if (prevAddr !=b.prevAddr)
                return prevAddr<b.prevAddr;
            else
                return label<b.label;
        }
        inline LWMPSMACAddressKey & operator = (const LWMPSMACAddressKey& b)
        {
            srcAddr=b.srcAddr;
            destAddr=b.destAddr;
            prevAddr=b.prevAddr;
            label = b.label;
            return *this;
        }
        inline bool operator==(const LWMPSMACAddressKey& b) const
        {
            if ((srcAddr==b.srcAddr) && (destAddr==b.destAddr) &&(prevAddr==b.prevAddr) && (label==b.label))
                return true;
            else
                return false;
        }
    } LWMPSMACAddressKey;
    typedef std::map<LWMPSMACAddressKey, uint64_t> LWmplsFwMacKey;
    LWmplsFwMacKey forwardingMacKey;


  private:
    simtime_t   LWMPLS_MAX_TIME;
    simtime_t   LWMPLS_MAX_TIME_MAC;
    /* timer limit of initialize forwarding struc after capture label */
    simtime_t   LWMPLS_MAX_TIME_CAPTURE_LABEL;
    /* Limits for break path */
    /* Number of reintent before break */
    int LWMPLS_MAX_RTR;
    /* Timer after last message */
    LWmplsFwMap * forwardingTableOutput;
    LWmplsInterfaceMap* interfaceMap;
    LWmplslabelArray label_list;

    int num_label_in_use;
    double bad_pkt_rate;

    // typedef std::vector<int> ListLabel;
    // typedef std::map<uint64_t,listLabel> DestinationList;
    DestinationList destList;
    BroadcastList   broadCastList;
    uint32_t    broadCastCounter;
    friend class Ieee80211Mesh;

    void deleteForwarding(LWmpls_Forwarding_Structure* data_f_ptr);

  public:
    LWMPLSDataStructure();
    ~LWMPLSDataStructure();
    simtime_t & mplsMaxTime() {return LWMPLS_MAX_TIME;}
    simtime_t & mplsMacLimit() {return LWMPLS_MAX_TIME_MAC;}
    /* timer limit of initialize forwarding struc after capture label */
    simtime_t & mplsMaxCaptureLimit() {return LWMPLS_MAX_TIME_CAPTURE_LABEL;}
    /* Limits for break path */
    /* Number of reintent before break */
    int &   mplsMaxMacRetry () {return LWMPLS_MAX_RTR;}

    LWmpls_Forwarding_Structure * lwmpls_forwarding_data(int,int,uint64_t );
    LWmpls_Interface_Structure * lwmpls_interface_structure(uint64_t);


    void   lwmpls_interface_delete_list_mpls(uint64_t);
    int    getLWMPLSLabel();
    void   delLWMPLSLabel(int);

    void   lwmpls_init_interface(LWmpls_Interface_Structure** ,int,uint64_t, int );
    void   lwmpls_forwarding_input_data_add(int,LWmpls_Forwarding_Structure *);
    bool   lwmpls_forwarding_output_data_add(int ,uint64_t ,LWmpls_Forwarding_Structure *,bool);
    LWmpls_Forwarding_Structure * lwmpls_interface_delete_label(int);

    void   lwmpls_interface_delete_old_path();

    bool   lwmpls_label_in_use(int );
    bool   lwmpls_mac_last_access (simtime_t &,uint64_t);
    void   lwmpls_refresh_mac (uint64_t ,simtime_t);
    int    lwmpls_nun_labels_in_use ();
    double lwmpls_label_last_use (int); // Return, in double format the time of use
    int    lwmpls_label_status (int label);
    void   lwmpls_check_label (int label,const char * message);
    void   registerRoute(uint64_t destination,int label);
    int    getRegisterRoute(uint64_t destination);
    void   deleteRegisterRoute(uint64_t dest);
    void   deleteRegisterLabel(int dest);

    bool   getBroadCastCounter(uint64_t dest,uint32_t &counter);
    void   setBroadCastCounter(uint64_t dest,uint32_t counter);
    bool   getBroadCastCounter(uint32_t &counter) {counter = broadCastCounter; return true;}
    void   setBroadCastCounter(uint32_t counter) {broadCastCounter = counter;}
// Use the mac address like a label access methods
    void setForwardingMacKey(uint64_t,uint64_t,uint64_t,uint64_t,int32_t=-1);
    uint64_t getForwardingMacKey(uint64_t,uint64_t,uint64_t,int32_t=-1);
    bool delForwardingMacKey(uint64_t,uint64_t,uint64_t,int32_t=-1);
};


typedef enum
{
    label_break,
    label_begin,
    label_delete,
    label_continue,
    label_path,
    label_ack
} Msg_label_types;


typedef struct Lwmpls_label_msg : public cOwnedObject
{
    int  label;
    int label_return;
    Msg_label_types code;
    int src_addr;
    int dest_addr;
} Lwmpls_label_msg;


typedef struct Lwmpls_label_struct : public cOwnedObject
{
    int label;
    int     in_port;
    int out_port;
    double  percentage;
    double  ocupation;
    int dest_addr;
    int src_addr;
    int next_addr;
} Lwmpls_label_struct;
/*-------------------------------------------------------------------*/
/* End if for protection against multiple includes. */
#endif /* _LWMPLS_DATA_H_INCLUDED_ */

