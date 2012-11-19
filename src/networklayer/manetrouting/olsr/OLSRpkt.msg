// ***************************************************************************
// *   Copyright (C) 2008 Alfonso Ariza Quintana aarizaq@uma.es              *
// *                                                                         *
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// *   This program is distributed in the hope that it will be useful,       *
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
// *   GNU General Public License for more details.                          *
// *                                                                         *
// *   You should have received a copy of the GNU General Public License     *
// *   along with this program; if not, write to the                         *
// *   Free Software Foundation, Inc.,                                       *
// *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
// ***************************************************************************

cplusplus {{
#include "OLSR_ETX_parameter.h"
#include "ManetAddress.h"
#include <string.h>

#ifndef nsaddr_t
typedef ManetAddress nsaddr_t;
#endif

/********** Message types **********/
/// %OLSR HELLO message type.
#define OLSR_HELLO_MSG      1
/// %OLSR TC message type.
#define OLSR_TC_MSG     2
/// %OLSR MID message type.
#define OLSR_MID_MSG        3


/// %OLSR HELLO message type.
#define OLSR_ETX_HELLO_MSG  OLSR_HELLO_MSG
/// %OLSR TC message type.
#define OLSR_ETX_TC_MSG     OLSR_TC_MSG
/// %OLSR MID message type.
#define OLSR_ETX_MID_MSG    OLSR_MID_MSG

/********** Packets stuff **********/

///
/// \brief Size of the addresses we are using.
///
/// You should always use this macro when interested in calculate addresses'
/// sizes. By default IPv4 addresses are supposed, but you can change to
/// IPv6 ones by recompiling with OLSR_IPv6 constant defined.
///

#ifdef OLSR_IPv6
#define ADDR_SIZE_DEFAULT   16
#else
#define ADDR_SIZE_DEFAULT   4
#endif


/// Maximum number of messages per packet.
#define OLSR_MAX_MSGS       4
#define OLSR_ETX_MAX_MSGS OLSR_MAX_MSGS
/// Maximum number of hellos per message (4 possible link types * 3 possible nb types).
#define OLSR_MAX_HELLOS     12
#define OLSR_ETX_MAX_HELLOS OLSR_MAX_HELLOS
/// Maximum number of addresses advertised on a message.
#define OLSR_MAX_ADDRS      64
#define OLSR_ETX_MAX_ADDRS OLSR_MAX_ADDRS

/// Size (in bytes) of packet header.
#define OLSR_PKT_HDR_SIZE   4
#define OLSR_ETX_PKT_HDR_SIZE OLSR_PKT_HDR_SIZE

/// Size (in bytes) of message header.
#define OLSR_MSG_HDR_SIZE   12
#define OLSR_ETX_MSG_HDR_SIZE OLSR_MSG_HDR_SIZE

/// Size (in bytes) of hello header.
#define OLSR_HELLO_HDR_SIZE 4
#define OLSR_ETX_HELLO_HDR_SIZE OLSR_HELLO_HDR_SIZE

/// Size (in bytes) of hello_msg header.
#define OLSR_HELLO_MSG_HDR_SIZE 4
#define OLSR_ETX_HELLO_MSG_HDR_SIZE OLSR_HELLO_MSG_HDR_SIZE

/// Size (in bytes) of tc header.
#define OLSR_TC_HDR_SIZE    4
#define OLSR_ETX_TC_HDR_SIZE OLSR_TC_HDR_SIZE


class OlsrAddressSize
{
    public:
    static uint32_t ADDR_SIZE;
};

typedef struct OLSR_ETX_iface_address {

  /// Interface Address
  nsaddr_t  iface_address_;

  /// Link quality extension
  double  link_quality_;
  double  nb_link_quality_;

  /// Link delay extension
  double  link_delay_;
  double  nb_link_delay_;
  int parameter_qos_;

  inline nsaddr_t& iface_address() { return iface_address_; }

  /// Link quality extension
  inline double& link_quality() { return link_quality_; }
  inline double& nb_link_quality() { return nb_link_quality_; }

  inline double  etx()  {
    double mult = (double) (link_quality() * nb_link_quality());
    switch (parameter_qos_) {
    case OLSR_ETX_BEHAVIOR_ETX:
      return (mult < 0.01) ? 100.0 : (double) 1.0 / (double) mult;
      break;

    case OLSR_ETX_BEHAVIOR_ML:
      return mult;
      break;

    case OLSR_ETX_BEHAVIOR_NONE:
    default:
      return 0.0;
      break;
    }
  }

  /// Link delay extension
  inline double& link_delay() { return link_delay_; }
  inline double& nb_link_delay() { return nb_link_delay_; }

} OLSR_ETX_iface_address;


/// Auxiliary struct which is part of the %OLSR_ETX HELLO message (struct OLSR_ETX_hello).
typedef struct OLSR_hello_msg {

  /// Link code.
  uint8_t  link_code_;
  /// Reserved.
  uint8_t  reserved_;
  /// Size of this link message.
  uint16_t  link_msg_size_;
  /// List of interface addresses of neighbor nodes.
  OLSR_ETX_iface_address  nb_iface_addrs_[OLSR_MAX_ADDRS];
  /// Number of interface addresses contained in nb_iface_addrs_.
  int    count;

  inline uint8_t&  link_code()    { return link_code_; }
  inline uint8_t&  reserved()    { return reserved_; }
  inline uint16_t&  link_msg_size()    { return link_msg_size_; }
  inline OLSR_ETX_iface_address&  nb_etx_iface_addr(int i)  { return nb_iface_addrs_[i]; }
  inline nsaddr_t  & nb_iface_addr(int i)  { return nb_iface_addrs_[i].iface_address_;}
  inline void set_qos_behaviour(int bh) {for(int i=0;i<OLSR_MAX_ADDRS;i++) nb_iface_addrs_[i].parameter_qos_=bh;}

  inline uint32_t size() { return OLSR_HELLO_MSG_HDR_SIZE + count*OlsrAddressSize::ADDR_SIZE; }

} OLSR_ETX_hello_msg;

#if 0
/// Auxiliary struct which is part of the %OLSR HELLO message (struct OLSR_hello).
typedef struct OLSR_hello_msg {

        /// Link code.
    uint8_t link_code_;
    /// Reserved.
    uint8_t reserved_;
    /// Size of this link message.
    uint16_t    link_msg_size_;
    /// List of interface addresses of neighbor nodes.
    nsaddr_t    nb_iface_addrs_[OLSR_MAX_ADDRS];
    /// Number of interface addresses contained in nb_iface_addrs_.
    int     count;

    inline uint8_t& link_code()     { return link_code_; }
    inline uint8_t& reserved()      { return reserved_; }
    inline uint16_t&    link_msg_size()     { return link_msg_size_; }
    inline nsaddr_t&    nb_iface_addr(int i)    { return nb_iface_addrs_[i]; }

    inline uint32_t size() { return OLSR_HELLO_MSG_HDR_SIZE + count*OlsrAddressSize::ADDR_SIZE; }

} OLSR_hello_msg;
#endif

/// %OLSR HELLO message.

typedef struct OLSR_hello :cObject {

    /// Reserved.
    uint16_t    reserved_;
    /// HELLO emission interval in mantissa/exponent format.
    uint8_t htime_;
    /// Willingness of a node for forwarding packets on behalf of other nodes.
    uint8_t willingness_;
    /// List of OLSR_hello_msg.
    OLSR_hello_msg  hello_body_[OLSR_MAX_HELLOS];
    /// Number of OLSR_hello_msg contained in hello_body_.
    int     count;

    inline uint16_t&    reserved()      { return reserved_; }
    inline uint8_t& htime()         { return htime_; }
    inline uint8_t& willingness()       { return willingness_; }
    inline OLSR_hello_msg&  hello_msg(int i)    { return hello_body_[i]; }

    inline uint32_t size() {
        uint32_t sz = OLSR_HELLO_HDR_SIZE;
        for (int i = 0; i < count; i++)
            sz += hello_msg(i).size();
        return sz;
    }
} OLSR_hello;


/// %OLSR TC message.
typedef struct OLSR_tc :cObject{

        /// Advertised Neighbor Sequence Number.
    uint16_t    ansn_;
    /// Reserved.
    uint16_t    reserved_;
    /// List of neighbors' main addresses.
    OLSR_ETX_iface_address  nb_main_addrs_[OLSR_MAX_ADDRS];
    /// Number of neighbors' main addresses contained in nb_main_addrs_.
    int     count;

    inline  uint16_t&   ansn()          { return ansn_; }
    inline  uint16_t&   reserved()      { return reserved_; }
    inline  nsaddr_t&   nb_main_addr(int i) { return nb_main_addrs_[i].iface_address_; }
    inline  OLSR_ETX_iface_address& nb_etx_main_addr(int i) { return nb_main_addrs_[i]; }
    inline void set_qos_behaviour(int bh) {for(int i=0;i<OLSR_MAX_ADDRS;i++) nb_main_addrs_[i].parameter_qos_=bh;}
    inline  uint32_t size() { return OLSR_TC_HDR_SIZE + count*OlsrAddressSize::ADDR_SIZE; }
} OLSR_tc;

#if 0
/// %OLSR TC message.
typedef struct OLSR_tc :cObject{

        /// Advertised Neighbor Sequence Number.
    uint16_t    ansn_;
    /// Reserved.
    uint16_t    reserved_;
    /// List of neighbors' main addresses.
    nsaddr_t    nb_main_addrs_[OLSR_MAX_ADDRS];
    /// Number of neighbors' main addresses contained in nb_main_addrs_.
    int     count;

    inline  uint16_t&   ansn()          { return ansn_; }
    inline  uint16_t&   reserved()      { return reserved_; }
    inline  nsaddr_t&   nb_main_addr(int i) { return nb_main_addrs_[i]; }

    inline  uint32_t size() { return OLSR_TC_HDR_SIZE + count*OlsrAddressSize::ADDR_SIZE; }

} OLSR_tc;
#endif

/// %OLSR MID message.
typedef struct OLSR_mid :cObject{

    /// List of interface addresses.
    nsaddr_t    iface_addrs_[OLSR_MAX_ADDRS];
    /// Number of interface addresses contained in iface_addrs_.
    int     count;

    inline nsaddr_t & iface_addr(int i) { return iface_addrs_[i]; }
    void    setIface_addr(int i,nsaddr_t a) {iface_addrs_[i]=a; }

    inline uint32_t size()          { return count*OlsrAddressSize::ADDR_SIZE; }

} OLSR_mid;

#define MidSize sizeof (OLSR_mid)
#define HelloSize sizeof (OLSR_hello)
#define TcSize sizeof(OLSR_tc)

#define MAXBODY (HelloSize > TcSize ?  (HelloSize > MidSize? HelloSize: MidSize):(TcSize > MidSize? TcSize:MidSize))


class   MsgBody {
public:
    OLSR_hello  hello_;
    OLSR_tc     tc_;
    OLSR_mid    mid_;


    MsgBody(MsgBody &other){
            memcpy(&hello_,&other.hello_,sizeof(OLSR_hello));
            memcpy(&tc_,&other.tc_,sizeof(OLSR_tc));
            memcpy(&mid_,&other.mid_,sizeof(OLSR_mid));
        }
    MsgBody(){
            memset(&hello_,0,sizeof(OLSR_hello));
            memset(&tc_,0,sizeof(OLSR_tc));
            memset(&mid_,0,sizeof(OLSR_mid));
          }
    ~MsgBody(){}
    MsgBody & operator =  (const MsgBody &other){
            if (this==&other) return *this;
                            memcpy(&hello_,&other.hello_,sizeof(OLSR_hello));
                            memcpy(&tc_,&other.tc_,sizeof(OLSR_tc));
                            memcpy(&mid_,&other.mid_,sizeof(OLSR_mid));
                return *this;
            }
    OLSR_hello  * hello(){return &hello_;}
    OLSR_tc * tc(){return &tc_;}
    OLSR_mid* mid(){return &mid_;}


};

/// %OLSR message.
typedef struct OLSR_msg {

    uint8_t msg_type_;  ///< Message type.
    uint8_t vtime_;     ///< Validity time.
    uint16_t    msg_size_;  ///< Message size (in bytes).
    nsaddr_t    orig_addr_; ///< Main address of the node which generated this message.
    uint8_t ttl_;       ///< Time to live (in hops).
    uint8_t hop_count_; ///< Number of hops which the message has taken.
    uint16_t    msg_seq_num_;   ///< Message sequence number.

    MsgBody msg_body_;          ///< Message body.
    inline  uint8_t&    msg_type()  { return msg_type_; }
    inline  uint8_t&    vtime()     { return vtime_; }
    inline  uint16_t&   msg_size()  { return msg_size_; }
    inline  nsaddr_t    & orig_addr()   { return orig_addr_; }
    inline  void    setOrig_addr(nsaddr_t a)    {orig_addr_=a; }
    inline  uint8_t&    ttl()       { return ttl_; }
    inline  uint8_t&    hop_count() { return hop_count_; }
    inline  uint16_t&   msg_seq_num()   { return msg_seq_num_; }
    inline  OLSR_hello& hello()     { return *(msg_body_.hello()); }
    inline  OLSR_tc&    tc()        { return *(msg_body_.tc()); }
    inline  OLSR_mid&   mid()       { return *(msg_body_.mid()); }


    inline uint32_t size() {
        uint32_t sz = OLSR_MSG_HDR_SIZE;
        if (msg_type() == OLSR_HELLO_MSG)
            sz += hello().size();
        else if (msg_type() == OLSR_TC_MSG)
            sz += tc().size();
        else if (msg_type() == OLSR_MID_MSG)
            sz += mid().size();
        return sz;
    }
    OLSR_msg(){}
    OLSR_msg(const OLSR_msg &other)
    {
        msg_type_=other.msg_type_;  ///< Message type.
        vtime_=other.vtime_;        ///< Validity time.
        msg_size_=other.msg_size_;  ///< Message size (in bytes).
        orig_addr_=other.orig_addr_;    ///< Main address of the node which generated this message.
        ttl_=other.ttl_;        ///< Time to live (in hops).
        hop_count_=other.hop_count_;    ///< Number of hops which the message has taken.
        msg_seq_num_=other.msg_seq_num_;    ///< Message sequence number.
        msg_body_=other.msg_body_;          ///< Message body.
    }

    OLSR_msg & operator = (const OLSR_msg &other)
    {
        if (this==&other) return *this;
        msg_type_=other.msg_type_;  ///< Message type.
        vtime_=other.vtime_;        ///< Validity time.
        msg_size_=other.msg_size_;  ///< Message size (in bytes).
        orig_addr_=other.orig_addr_;    ///< Main address of the node which generated this message.
        ttl_=other.ttl_;        ///< Time to live (in hops).
        hop_count_=other.hop_count_;    ///< Number of hops which the message has taken.
        msg_seq_num_=other.msg_seq_num_;    ///< Message sequence number.
        msg_body_=other.msg_body_;          ///< Message body.
        return *this;
    }

} OLSR_msg;

}}


struct OLSR_msg;

//
// Represents an OLSR packet
//
// Uses the following cMessage fields:
//    - length () / setLength(err)  in bits
//    - hasBitError() / setBitError()
//    - timestamp() / setTimestamp (simtime) used in timestamp option
//
// Additional length fields defined in this class are in bytes
// (totalLength()=length()/8 and header_length), or 8 bytes (fragment_offset).
//
// Only one of the option fields can exist at a time.
//
packet OLSR_pkt
{
    @omitGetVerb(true);
    bool reduceFuncionality=false;
    short pkt_seq_num;          ///< Packet sequence number.
    long sn;                    /// CapProbe packet sequence number
    double send_time;           /// Packet sending timestamp
    OLSR_msg msg[];             ///< Packet body.
}
