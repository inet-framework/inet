/*****************************************************************************
 *
 * Copyright (C) Alfonso Ariza Quintana, Universidad de Malaga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Alfonso Ariza, <aarizaq@uma.es>
 *
 *****************************************************************************/
#ifndef _DYMO_MSG_OMNET_H
#define _DYMO_MSG_OMNET_H

#include <stdio.h>
#include <stdlib.h>
#include  <string.h>
#include "inet/routing/extras/base/compatibility.h"
#include "inet/networklayer/common/L3Address.h"

#include "inet/common/INETDefs.h"

namespace inet {

namespace inetmanet {

//#define STATIC_BLOCK
#define STATIC_BLOCK_SIZE 600
// this structures are a redifinition of def.h struct for omnet

/* A generic Dymo packet header struct... */ /* Omnet++ definition */
struct DYMO_element : public cPacket
{
    /* Common part */
    u_int32_t   m : 1;
    u_int32_t   h : 2;
    u_int32_t   type : 5;
    u_int32_t   len : 12;
    u_int32_t   ttl : 6;
    u_int32_t   i : 1;
    u_int32_t   res : 5;
    L3Address notify_addr; // if M bit set
    L3Address target_addr; // if not a DYMOcast addr in IP dest addr
    uint8_t blockAddressGroup;
    bool previousStatic;

    //explicit AODV_msg(const char *name="AodvMgs") : cMessage(name) {extensionsize=0;extension=NULL;}
#ifdef STATIC_BLOCK
    explicit DYMO_element(const char *name = NULL) : cPacket(name) {setBitLength(0); extensionsize = 0; memset(extension, 0, STATIC_BLOCK_SIZE); blockAddressGroup = 0; previousStatic = false;}
#else
    explicit DYMO_element(const char *name = NULL) : cPacket(name) {setBitLength(0); extensionsize = 0; extension = NULL; blockAddressGroup = 0; previousStatic = false;}
#endif
    ~DYMO_element();
    DYMO_element(const DYMO_element  &m);
    DYMO_element &  operator=(const DYMO_element &m);
    virtual DYMO_element *dup() const {return new DYMO_element(*this);}
    char *  addExtension(int);
    char *  delExtension(int);
    void clearExtension();
    char * getFirstExtension() {return extension;}
    int getSizeExtension() {return extensionsize;}
    virtual std::string detailedInfo() const;

  private:
    void copy(const DYMO_element& other);
    void clean() { clearExtension(); }

  protected:
#ifdef  STATIC_BLOCK
    char extension[STATIC_BLOCK_SIZE];
#else
    char *extension;
#endif
    unsigned int extensionsize;
};

// RE message definition
struct re_block
{
    u_int16_t   res : 2;
    u_int16_t   re_hopcnt : 6;
    u_int16_t   g : 1;
    u_int16_t   prefix : 7;
    u_int32_t   useAp : 1;
    L3Address     re_node_addr;
    u_int32_t   re_node_seqnum;
    unsigned char from_proactive;
    bool        staticNode;
    uint32_t    cost;
    uint8_t     re_hopfix;
};

#define MAX_RERR_BLOCKS 50L
#define RE_BLOCK_SIZE (this->isInMacLayer()?12:10)
#define RE_BASIC_SIZE  (this->isInMacLayer()? 17U: 13U)
//#define RE_BLOCK_SIZE  10U
#define RE_BLOCK_LENGTH sizeof(struct re_block)
//#define RE_SIZE     (23+(RE_BLOCK_SIZE*MAX_RERR_BLOCKS))U
#define RE_SIZE    (this->isInMacLayer()? (25+(RE_BLOCK_SIZE*MAX_RERR_BLOCKS))U:(23+(RE_BLOCK_SIZE*MAX_RERR_BLOCKS))U)
//#define RE_BASIC_SIZE 11U
//#define RE_BASIC_SIZE   13U

#define RE Dymo_RE

struct Dymo_RE : public DYMO_element
{

    u_int32_t   a : 1;
    u_int32_t   s : 1;
    u_int32_t   res1 : 3;

    //u_int32_t target_addr;
    u_int32_t   target_seqnum;

    u_int8_t    thopcnt : 6;
    u_int8_t    res2 : 2;
    struct re_block *re_blocks;
    explicit Dymo_RE(const char *name = "RE_DymoMsg");

    Dymo_RE(const Dymo_RE &m);
    Dymo_RE &   operator=(const Dymo_RE &m);
    virtual Dymo_RE *dup() const {return new Dymo_RE(*this);}

    void newBocks(int n);
    void delBocks(int n);
    void delBlockI(int);
    int numBlocks() const {return ((int)ceil((double)extensionsize/(double)sizeof(struct re_block)));}

  private:
    void copy(const Dymo_RE& other);
    void clean();
};

#define UERR_SIZE 17U
#define UERR Dymo_UERR

struct rerr_block
{
    L3Address unode_addr;
    u_int32_t   unode_seqnum;
};

struct Dymo_UERR : public DYMO_element
{
    explicit Dymo_UERR(const char *name = "UERR_DymoMsg") : DYMO_element(name) {}
    Dymo_UERR &     operator=(const Dymo_UERR &m);
    Dymo_UERR(const Dymo_UERR &m);
    virtual Dymo_UERR *dup() const {return new Dymo_UERR(*this);}
    L3Address uelem_target_addr;
    L3Address uerr_node_addr;
    u_int8_t    uelem_type;

    private:
      void copy(const Dymo_UERR& other);
      void clean() {};
};



//#define RERR_BLOCK_SIZE   (isInMacLayer()?10:8)
#define RERR_BLOCK_SIZE 8U
#define RERR_BASIC_SIZE 7U // before 8
#define RERR_BLOCK_LENGTH   sizeof(struct rerr_block)
#define DYMO_RERR_SIZE      (8+(RERR_BLOCK_SIZE*MAX_RERR_BLOCKS))U

#ifdef RERR
#undef RERR
#endif

#define RERR  Dymo_RERR

struct Dymo_RERR : public DYMO_element
{
    struct rerr_block *rerr_blocks;

    explicit Dymo_RERR(const char *name = "RERR_DymoMsg") : DYMO_element(name) {rerr_blocks = (struct rerr_block *)extension;}

    Dymo_RERR(const Dymo_RERR &m);
    Dymo_RERR &     operator=(const Dymo_RERR &m);
    virtual Dymo_RERR *dup() const {return new Dymo_RERR(*this);}

    void newBocks(int n);
    void delBocks(int n);
    int numBlocks() const {return ((int)ceil((double)extensionsize/(double)sizeof(struct rerr_block)));}

  private:
    void copy(const Dymo_RERR& other);
    void clean() {};
};

#define HELLO_BASIC_SIZE 2U
#define HELLO Dymo_HELLO

struct Dymo_HELLO : public DYMO_element
{
    explicit Dymo_HELLO(const char *name = "HELLO_DymoMsg") : DYMO_element(name) {}
};

} // namespace inetmanet

} // namespace inet

#endif

