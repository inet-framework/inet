//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPHDR_H
#define __INET_SCTPHDR_H

namespace inet {

namespace sctp {

#ifdef _MSC_VER
#define __PACKED__
#else // ifdef _MSC_VER
#define __PACKED__       __attribute__((packed))
#endif // ifdef _MSC_VER

#define I_BIT            0x08
#define UNORDERED_BIT    0x04
#define BEGIN_BIT        0x02
#define END_BIT          0x01
#define T_BIT            0x01
#define C_FLAG           0x08
#define T_FLAG           0x04
#define B_FLAG           0x02
#define M_FLAG           0x01
#define NAT_M_FLAG       0x02
#define NAT_T_FLAG       0x01

struct common_header
{
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t verification_tag;
    uint32_t checksum;
} __PACKED__;

struct chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
} __PACKED__;

struct data_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t tsn;
    uint16_t sid;
    uint16_t ssn;
    uint32_t ppi;
    uint8_t user_data[0];
} __PACKED__;

struct init_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t initiate_tag;
    uint32_t a_rwnd;
    uint16_t mos;
    uint16_t mis;
    uint32_t initial_tsn;
    uint8_t parameter[0];
} __PACKED__;

struct init_ack_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t initiate_tag;
    uint32_t a_rwnd;
    uint16_t mos;
    uint16_t mis;
    uint32_t initial_tsn;
    uint8_t parameter[0];
} __PACKED__;

struct sack_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t cum_tsn_ack;
    uint32_t a_rwnd;
    uint16_t nr_of_gaps;
    uint16_t nr_of_dups;
    uint8_t tsns[0];
} __PACKED__;

struct nr_sack_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t cum_tsn_ack;
    uint32_t a_rwnd;
    uint16_t nr_of_gaps;
    uint16_t nr_of_nr_gaps;
    uint16_t nr_of_dups;
    uint16_t reserved;
    uint8_t tsns[0];
} __PACKED__;

struct heartbeat_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint8_t heartbeat_info[0];
} __PACKED__;

struct heartbeat_ack_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint8_t heartbeat_info[0];
} __PACKED__;

struct abort_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint8_t error_causes[0];
} __PACKED__;

struct shutdown_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t cumulative_tsn_ack;
} __PACKED__;

struct shutdown_ack_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
} __PACKED__;

struct shutdown_complete_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
} __PACKED__;

struct cookie_echo_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint8_t state_cookie[0];
} __PACKED__;

struct cookie_ack_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
} __PACKED__;

struct error_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint8_t error_causes[0];
} __PACKED__;

struct forward_tsn_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t cum_tsn;
    uint8_t stream_info[0];
} __PACKED__;

struct asconf_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t serial;
    uint8_t parameters[0];
} __PACKED__;

struct asconf_ack_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t serial;
    uint8_t parameters[0];
} __PACKED__;

struct auth_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint16_t shared_key;
    uint16_t hmac_identifier;
    uint8_t hmac[0];
} __PACKED__;

struct stream_reset_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint8_t parameters[0];
} __PACKED__;

struct pktdrop_chunk
{
    uint8_t type;
    uint8_t flags;
    uint16_t length;
    uint32_t max_rwnd;
    uint32_t queued_data;
    uint16_t trunc_length;
    uint16_t reserved;
    uint8_t dropped_data[0];
} __PACKED__;

// variable length parameters in INIT chunk:
#define INIT_PARAM_IPV4           5
#define INIT_PARAM_IPV6           6
#define INIT_PARAM_COOKIE         7
#define INIT_SUPPORTED_ADDRESS    12

struct init_ipv4_address_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t address;
} __PACKED__;

struct init_ipv6_address_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t address[4];
} __PACKED__;

struct init_cookie_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t creationTime;
    uint32_t localTag;
    uint32_t peerTag;
    uint8_t localTieTag[32];
    uint8_t peerTieTag[32];
} __PACKED__;

struct cookie_parameter
{
    uint32_t creationTime;
    uint32_t localTag;
    uint32_t peerTag;
    uint8_t localTieTag[32];
    uint8_t peerTieTag[32];
} __PACKED__;

struct add_ip_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t correlation_id;
    uint8_t parameters[0];
} __PACKED__;

struct hmac_algo
{
    uint16_t type;
    uint16_t length;
    uint16_t ident[0];
} __PACKED__;

struct supported_extensions_parameter
{
    uint16_t type;
    uint16_t length;
    uint8_t chunk_type[0];
} __PACKED__;

struct forward_tsn_supported_parameter
{
    uint16_t type;
    uint16_t length;
} __PACKED__;

struct supported_address_types_parameter
{
    uint16_t type;
    uint16_t length;
    uint16_t address_type_1;
    uint16_t address_type_2;
} __PACKED__;

struct random_parameter
{
    uint16_t type;
    uint16_t length;
    uint8_t random[0];
} __PACKED__;

struct tlv
{
    uint16_t type;
    uint16_t length;
    uint8_t value[0];
} __PACKED__;

// Heartbeat info TLV:
struct heartbeat_info
{
    uint16_t type;
    uint16_t length;
    union
    {
        uint8_t info[0];
        struct
        {
            union
            {
                struct init_ipv4_address_parameter v4addr;
                struct init_ipv6_address_parameter v6addr;
            } addr;
            uint32_t time;
        } addr_and_time;
    } heartbeat_info_union;
} __PACKED__;

#define HBI_INFO(hbi)    ((hbi)->heartbeat_info_union.info)
#define HBI_ADDR(hbi)    ((hbi)->heartbeat_info_union.addr_and_time.addr)
#define HBI_TIME(hbi)    ((hbi)->heartbeat_info_union.addr_and_time.time)

struct error_cause
{
    uint16_t cause_code;
    uint16_t length;
    uint8_t info[0];
} __PACKED__;

struct error_cause_with_int
{
    uint16_t cause_code;
    uint16_t length;
    uint16_t info;
    uint16_t reserved;
} __PACKED__;

// SACK GAP:
struct sack_gap
{
    uint16_t start;
    uint16_t stop;
} __PACKED__;

// SACK DUP TSN:
struct sack_duptsn
{
    uint32_t tsn;
} __PACKED__;

// Forward_TSN streams
struct forward_tsn_streams
{
    uint16_t sid;
    uint16_t ssn;
} __PACKED__;

// Parameters for Stream Reset
struct outgoing_reset_request_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t srReqSn;    // Stream Reset Request Sequence Number
    uint32_t srResSn;    // Stream Reset Response Sequence Number
    uint32_t lastTsn;    // Senders last assigned TSN
    uint16_t streamNumbers[0];
} __PACKED__;

struct incoming_reset_request_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t srReqSn;    // Stream Reset Request Sequence Number
    uint16_t streamNumbers[0];
} __PACKED__;

struct ssn_tsn_reset_request_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t srReqSn;    // Stream Reset Request Sequence Number
} __PACKED__;

struct stream_reset_response_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t srResSn;    // Stream Reset Response Sequence Number
    uint32_t result;
    uint32_t sendersNextTsn;
    uint32_t receiversNextTsn;
} __PACKED__;

struct add_streams_request_parameter
{
    uint16_t type;
    uint16_t length;
    uint32_t srReqSn;    // Stream Reset Request Sequence Number
    uint16_t numberOfStreams;
    uint16_t reserved;
} __PACKED__;

struct data_vector
{
    uint8_t data[0];
} __PACKED__;

} // namespace sctp

} // namespace inet

#endif

