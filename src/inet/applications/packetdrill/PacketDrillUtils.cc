//
// Copyright (C) 2014 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <fcntl.h>
#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
 #include <netinet/in.h>
#else
#include "winsock2.h"
#include "sys/types.h"
#include "sys/stat.h"
#define stat _stat
#endif
#include <assert.h>

#include "inet/applications/packetdrill/PacketDrillUtils.h"

namespace inet {
using namespace sctp;

/* A table of platform-specific string->int mappings. */
struct int_symbol platform_symbols_table[] = {
    /* cross_platform_symbols */
    { AF_INET,                          "AF_INET"                         },

    { SOCK_DGRAM,                       "SOCK_DGRAM"                      },
    { SOCK_STREAM,                      "SOCK_STREAM"                     },

    { IPPROTO_IP,                       "IPPROTO_IP"                      },
    { IPPROTO_UDP,                      "IPPROTO_UDP"                     },
    { IPPROTO_TCP,                      "IPPROTO_TCP"                     },
    { IPPROTO_SCTP,                     "IPPROTO_SCTP"                    },

    /* /usr/include/netinet/sctp.h */
    { SCTP_RTOINFO,                     "SCTP_RTOINFO"                    },
    { SCTP_INITMSG,                     "SCTP_INITMSG"                    },
    { SCTP_NODELAY,                     "SCTP_NODELAY"                    },
    { SCTP_MAXSEG,                      "SCTP_MAXSEG"                     },
    { SCTP_DELAYED_SACK,                "SCTP_DELAYED_SACK"               },
    { SCTP_MAX_BURST,                   "SCTP_MAX_BURST"                  },
    { SCTP_FRAGMENT_INTERLEAVE,         "SCTP_FRAGMENT_INTERLEAVE"        },
    { SCTP_INTERLEAVING_SUPPORTED,      "SCTP_INTERLEAVING_SUPPORTED"     },
    { SCTP_UNORDERED,                   "SCTP_UNORDERED"                  },
    { SCTP_ASSOCINFO,                   "SCTP_ASSOCINFO"                  },
    { SCTP_PEER_ADDR_PARAMS,            "SCTP_PEER_ADDR_PARAMS"           },

    { SCTP_COOKIE_WAIT,                 "SCTP_COOKIE_WAIT"                },

    /* sctp stream reconfiguration */
    { SCTP_ENABLE_STREAM_RESET,         "SCTP_ENABLE_STREAM_RESET"        },
    { SCTP_ENABLE_RESET_STREAM_REQ,     "SCTP_ENABLE_RESET_STREAM_REQ"    },
    { SCTP_ENABLE_RESET_ASSOC_REQ,      "SCTP_ENABLE_RESET_ASSOC_REQ"     },
    { SCTP_ENABLE_CHANGE_ASSOC_REQ,     "SCTP_ENABLE_CHANGE_ASSOC_REQ"    },
    { SCTP_RESET_STREAMS,               "SCTP_RESET_STREAMS"              },
    { SCTP_STREAM_RESET_INCOMING,       "SCTP_STREAM_RESET_INCOMING"      },
    { SCTP_STREAM_RESET_OUTGOING,       "SCTP_STREAM_RESET_OUTGOING"      },
    { SCTP_RESET_ASSOC,                 "SCTP_RESET_ASSOC"                },
    { SCTP_ADD_STREAMS,                 "SCTP_ADD_STREAMS"                },

    { SPP_HB_DISABLE,                   "SPP_HB_DISABLE"                  },
    { SPP_HB_ENABLE,                    "SPP_HB_ENABLE"                   },
    { SHUT_WR,                          "SHUT_WR"                         },
    { SCTP_STATUS,                      "SCTP_STATUS"                     },
    { F_GETFL,                          "F_GETFL"                         },
    { F_SETFL,                          "F_SETFL"                         },
    { O_RDWR,                           "PD_O_RDWR"                       },
    /* Sentinel marking the end of the table. */
    { 0, NULL },
};

struct int_symbol *platform_symbols(void)
{
    return platform_symbols_table;
}


PacketDrillConfig::PacketDrillConfig()
{
    ip_version = IP_VERSION_4;
    socketDomain = AF_INET;
    wireProtocol = AF_INET;
    tolerance_usecs = 75000;
    mtu = TUN_DRIVER_DEFAULT_MTU;
    scriptPath = nullptr;
}

PacketDrillConfig::~PacketDrillConfig()
{
}

void PacketDrillConfig::parseScriptOptions(cQueue *options)
{
}

PacketDrillPacket::PacketDrillPacket()
{
   inetPacket = nullptr;
   direction = DIRECTION_INVALID;
}

PacketDrillPacket::~PacketDrillPacket()
{
    if (inetPacket)
        delete inetPacket;
}

PacketDrillExpression::PacketDrillExpression(enum expression_t type_)
{
    type = type_;
    format = nullptr;
    if (type == EXPR_SCTP_SNDRCVINFO)
        value.sctp_sndrcvinfo = (struct sctp_sndrcvinfo_expr *)malloc(sizeof(struct sctp_sndrcvinfo_expr));
}

PacketDrillExpression::~PacketDrillExpression()
{
    if (type == EXPR_LIST) {
        for (cQueue::Iterator iter(*list); !iter.end(); iter++)
            list->remove((*iter));
        delete list;
    }
}

/* Fill in 'out' with an unescaped version of the input string. On
 * success, return STATUS_OK; on error, return STATUS_ERR and store
 * an error message in *error.
 */
int PacketDrillExpression::unescapeCstringExpression(const char *input_string, char **error)
{
    int bytes = strlen(input_string) + 1;
    type = EXPR_STRING;
    value.string = (char *)malloc(bytes);
    const char *c_in = input_string;
    char *c_out = value.string;
    while (*c_in != '\0') {
        if (*c_in == '\\') {
            ++c_in;
            switch (*c_in) {
                case '\\':
                    *c_out = '\\';
                    break;

                case '"':
                    *c_out = '"';
                    break;

                case 'f':
                    *c_out = '\f';
                    break;

                case 'n':
                    *c_out = '\n';
                    break;

                case 'r':
                    *c_out = '\r';
                    break;

                case 't':
                    *c_out = '\t';
                    break;

                case 'v':
                    *c_out = '\v';
                    break;

                default:
                    EV_DEBUG << "Unsupported escape code: " << *c_in << endl;
                    return STATUS_ERR;
            }
        } else {
            *c_out = *c_in;
        }
        ++c_in;
        ++c_out;
    }
    return STATUS_OK;
}

/* Sets the value from the expression argument, checking that it is a
 * valid int32 or uint32, and matches the expected type. Returns STATUS_OK on
 * success; on failure returns STATUS_ERR and sets error message.
 */
int PacketDrillExpression::getS32(int32 *val, char **error)
{
    if (type != EXPR_INTEGER)
        return STATUS_ERR;
    if ((value.num > UINT_MAX) || (value.num < INT_MIN)) {
        EV_DEBUG << "Value out of range for 32-bit integer: " << value.num << endl;
        return STATUS_ERR;
    }
    *val = value.num;
    return STATUS_OK;
}

int PacketDrillExpression::getU32(uint32 *val, char **error)
{
    if (type != EXPR_INTEGER)
        return STATUS_ERR;
    if ((value.num > UINT32_MAX) || (value.num < 0)) {
        EV_DEBUG << "Value out of range for 32-bit unsigned integer: " << value.num << endl;
        return STATUS_ERR;
    }
    *val = value.num;
    return STATUS_OK;
}

int PacketDrillExpression::getU16(uint16 *val, char **error)
{
    if (type != EXPR_INTEGER)
        return STATUS_ERR;
    if ((value.num > UINT16_MAX) || (value.num < 0)) {
        EV_DEBUG << "Value out of range for 16-bit unsigned integer: " << value.num << endl;
        return STATUS_ERR;
    }
    *val = value.num;
    return STATUS_OK;
}


/* Do a symbol->int lookup, and return true if we found the symbol. */
bool PacketDrillExpression::lookupIntSymbol(const char *input_symbol, int64 *output_integer, struct int_symbol *symbols)
{
    int i;
    for (i = 0; symbols[i].name != NULL ; ++i) {
        if (strcmp(input_symbol, symbols[i].name) == 0) {
            *output_integer = symbols[i].value;
            return true;
        }
    }
    return false;
}

int PacketDrillExpression::symbolToInt(const char *input_symbol, int64 *output_integer, char **error)
{
    if (lookupIntSymbol(input_symbol, output_integer, platform_symbols()))
        return STATUS_OK;

    return STATUS_ERR;
}

PacketDrillEvent::PacketDrillEvent(enum event_t type_)
{
    lineNumber = -1;
    eventNumber = -1;
    timeType = ANY_TIME;
    type = type_;
    eventTimeEnd = NO_TIME_RANGE;
    eventOffset = NO_TIME_RANGE;
    eventKind.packet = nullptr;
    eventKind.syscall = nullptr;
    eventKind.command = nullptr;
}

PacketDrillEvent::~PacketDrillEvent()
{
}

PacketDrillScript::PacketDrillScript(const char *scriptFile)
{
    eventList = new cQueue("scriptEventList");
    optionList = new cQueue("scriptOptionList");
    buffer = nullptr;
    assert(scriptFile != nullptr);
    scriptPath = scriptFile;
    length = 0;
}

PacketDrillScript::~PacketDrillScript()
{
    delete eventList;
    delete optionList;
}

void PacketDrillScript::readScript()
{
    int size = 0;

    while (buffer == NULL) {
        struct stat script_info;
        int fd = -1;

        /* Allocate a buffer big enough for the whole file. */
        if (stat(scriptPath, &script_info) != 0)
            EV_INFO << "parse error: stat() of script file '" << scriptPath << "': " << strerror(errno) << endl;

        /* Pick a buffer size larger than the file, so we'll
         * know if the file grew.
         */
        size = fmax((int)script_info.st_size, size) + 1;

        buffer = (char *)malloc((int)size);
        assert(buffer != NULL);

        /* Read the file into our buffer. */
        fd = open(scriptPath, O_RDONLY);
        if (fd < 0)
            EV_INFO << "parse error opening script file '" << scriptPath << "': " << strerror(errno) << endl;

        length = read(fd, buffer, size);
        if (length < 0)
            EV_INFO << "parse error reading script file '" << scriptPath << "': " << strerror(errno) << endl;

        /* If we filled the buffer, then probably another
         * process wrote more to the file since our stat call,
         * so we should try again.
         */
        if (length == size) {
            free(buffer);
            buffer = NULL;
            length = 0;
        }

        if (close(fd))
            EV_INFO << "File destriptor was closed\n";
    }
    EV_INFO << "Script " << scriptPath << " was read with " << length << " length\n";
}

int PacketDrillScript::parseScriptAndSetConfig(PacketDrillConfig *config, const char *script_buffer)
{
    int res = 0;
    Invocation invocation = {
        .config = config,
        .script = this,
    };

    EV_DETAIL << "parse_and_run_script: " << scriptPath << endl;

    config->setScriptPath(scriptPath);

    readScript();

    res = parse_script(config, this, &invocation);

    return res;
}

PacketDrillStruct::PacketDrillStruct()
{
    value1 = 0;
    value2 = 0;
    value3 = 0;
    value4 = 0;
    streamNumbers = nullptr;
}

PacketDrillStruct::PacketDrillStruct(int64 v1, int64 v2)
{
    value1 = v1;
    value2 = v2;
    value3 = -2;
    value4 = -2;
    streamNumbers = nullptr;
}

PacketDrillStruct::PacketDrillStruct(int64 v1, int64 v2, int32 v3, int32 v4, cQueue *streams)
{
    value1 = v1;
    value2 = v2;
    value3 = v3;
    value4 = v4;
    if (streams != nullptr) {
        streamNumbers = streams;
    }
}

PacketDrillOption::PacketDrillOption(char *v1, char *v2)
{
    name = strdup(v1);
    value = strdup(v2);
}

PacketDrillTcpOption::PacketDrillTcpOption(uint16 kind_, uint16 length_)
{
    kind = kind_;
    length = length_;
    mss = 0;
    timeStamp.val = 0;
    timeStamp.ecr = 0;
    blockList = nullptr;
    windowScale = 0;
    blockCount = 0;
}

PacketDrillSctpChunk::PacketDrillSctpChunk(uint8 type_, SctpChunk *sctpChunk)
{
    type = type_;
    chunk = sctpChunk->dup();
    delete sctpChunk;
};

PacketDrillBytes::PacketDrillBytes()
{
    listLength = 0;
}

PacketDrillBytes::PacketDrillBytes(uint8_t byte)
{
    listLength = 0;
    appendByte(0x00FF & byte);
}

void PacketDrillBytes::appendByte(uint8_t byte)
{
    byteList.resize(listLength + 1);
    byteList.at(listLength) = byte;
    listLength++;
}

PacketDrillSctpParameter::~PacketDrillSctpParameter()
{
}

PacketDrillSctpParameter::PacketDrillSctpParameter(uint16 type_, int16 len, void* content_)
{
    parameterValue = 0;
    uint32 flgs = 0;
    type = type_;
    if (len == -1)
        flgs |= FLAG_CHUNK_LENGTH_NOCHECK;
    parameterLength = len;
    parameterList = nullptr;
    content = nullptr;

    if (!content_) {
        flgs |= FLAG_CHUNK_VALUE_NOCHECK;
        parameterList = nullptr;
    } else {
        switch (type) {
            case SUPPORTED_EXTENSIONS: {
                PacketDrillBytes *pdb = (PacketDrillBytes *)content_;
                this->setByteArrayPointer(pdb->getByteList());
                break;
            }
            case SUPPORTED_ADDRESS_TYPES: {
                parameterList = (cQueue *)(content_);
                break;
            }
            default:
                content = content_;
                break;
        }
    }

    flags = flgs;
}

}    // namespace inet


