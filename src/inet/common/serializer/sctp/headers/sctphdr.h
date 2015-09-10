//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_SCTPHDR_H
#define __INET_SCTPHDR_H

namespace inet {

namespace serializer {

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

#define CRC32C(c, d)     (c = (c >> 8) ^ crc_c[(c ^ (d)) & 0xFF])

static uint32_t crc_c[256] = {
    0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,
    0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
    0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
    0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
    0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
    0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
    0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,
    0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
    0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
    0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
    0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,
    0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
    0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,
    0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
    0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
    0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
    0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,
    0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
    0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,
    0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
    0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
    0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
    0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,
    0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
    0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
    0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
    0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
    0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
    0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,
    0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
    0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,
    0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
    0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
    0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
    0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
    0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
    0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,
    0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
    0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
    0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
    0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,
    0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
    0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,
    0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
    0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
    0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
    0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,
    0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
    0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,
    0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
    0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
    0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
    0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,
    0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
    0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
    0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
    0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
    0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
    0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,
    0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
    0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,
    0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
    0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
    0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L,
};

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
    uint8_t random[];
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
        uint8_t info[];
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

struct data_vector
{
    uint8_t data[0];
} __PACKED__;

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_SCTPHDR_H

