

#include  <string.h>

#include "dymo_msg_struct.h"
#include "IPv4Address.h"


#define DYMO_RE_TYPE    1
#define DYMO_RERR_TYPE  2
#define DYMO_UERR_TYPE  3
#define DYMO_HELLO_TYPE 4

Register_Class(DYMO_element);


DYMO_element::DYMO_element(const DYMO_element& m) : cPacket(m)
{
    copy(m);
}


DYMO_element& DYMO_element::operator=(const DYMO_element& msg)
{
    if (this==&msg) return *this;
    clean();
    cPacket::operator=(msg);
    copy(msg);
    return *this;
}

void DYMO_element::copy(const DYMO_element& msg)
{
    m = msg.m;
    h = msg.h;
    type = msg.type;
    len = msg.len;
    ttl = msg.ttl;
    i = msg.i;
    res = msg.res;
    notify_addr = msg.notify_addr;
    target_addr = msg.target_addr;
    blockAddressGroup = msg.blockAddressGroup;
    extensionsize = msg.extensionsize;
    previousStatic = msg.previousStatic;
#ifdef  STATIC_BLOCK
    memset(extension, 0, STATIC_BLOCK_SIZE);
#else
    if (extensionsize==0)
    {
        extension = NULL;
        return;
    }
    extension = new char[extensionsize];
#endif
    memcpy(extension, msg.extension, extensionsize);
}

void DYMO_element:: clearExtension()
{
    if (extensionsize==0)
    {
        return;
    }
#ifdef  STATIC_BLOCK
    memset(extension, 0, STATIC_BLOCK_SIZE);
#else
    delete [] extension;
#endif
    extensionsize = 0;
}

DYMO_element::~DYMO_element()
{
#ifndef STATIC_BLOCK
    clearExtension();
#endif
}

char * DYMO_element::addExtension(int len)
{
    char * extension_aux;
    if (len<0)
    {
        return NULL;
    }
#ifndef STATIC_BLOCK
    extension_aux = new char [extensionsize+len];
    memset(extension_aux,0,extensionsize+len);
    memcpy(extension_aux, extension, extensionsize);
    delete [] extension;
    extension = extension_aux;
#else
    memset(extension+extensionsize, 0, extensionsize-len);
#endif
    extensionsize += len;
    //setBitLength(getBitLength ()+(len*8));
    return extension;
}

char * DYMO_element::delExtension(int len)
{
    char * extension_aux;
    if (len<0)
    {
        return NULL;
    }
#ifndef STATIC_BLOCK
    extension_aux = new char [extensionsize-len];
    memcpy(extension_aux, extension, extensionsize-len);
    delete [] extension;
    extension = extension_aux;
#endif
    extensionsize -= len;
    // setBitLength(getBitLength ()-(len*8));
    return extension;
}

//=== registration
Register_Class(Dymo_RE);

Dymo_RE::Dymo_RE(const char *name) : DYMO_element(name)
{
    //int bs = RE_BASIC_SIZE;
    // int size = bs*8;
    // setBitLength(size);
    re_blocks = (struct re_block *)extension;
}

//=========================================================================

Dymo_RE::Dymo_RE(const Dymo_RE& m) : DYMO_element(m)
{
    copy(m);
}

Dymo_RE& Dymo_RE::operator=(const Dymo_RE& msg)
{
    if (this==&msg) return *this;

    DYMO_element::operator=(msg);
    copy(msg);
    return *this;
}

void Dymo_RE::copy(const Dymo_RE& msg)
{
    target_seqnum = msg.target_seqnum;
    thopcnt = msg.thopcnt;
    a = msg.a;
    s = msg.s;
    res1 = msg.res1;
    res2 = msg.res2;
    re_blocks = (struct re_block *)extension;
    setBitLength(msg.getBitLength());
}

void Dymo_RE::newBocks(int n)
{
    if (n<=0)
        return;
    int new_add_size;
    if (ceil((double)extensionsize/(double)sizeof(struct re_block))+n>MAX_RERR_BLOCKS)
        new_add_size = (int)((MAX_RERR_BLOCKS - ceil((double)extensionsize/(double)sizeof(struct re_block)))*sizeof(struct re_block));
    else
        new_add_size = n*sizeof(struct re_block);
    re_blocks = (struct re_block *) addExtension(new_add_size);
}

void Dymo_RE::delBocks(int n)
{
    if (n<=0)
        return;
    int del_blk;
    if (((unsigned int) n * sizeof(struct re_block))>extensionsize)
        del_blk = extensionsize;
    else
        del_blk = n*sizeof(struct re_block);
    re_blocks = (struct re_block *) delExtension(del_blk);
}

void Dymo_RE::delBlockI(int blockIndex)
{
    if (blockIndex<=0)
        return;

    if (((unsigned int) blockIndex * sizeof(struct re_block))>extensionsize)
        return;
    else
    {
        int new_add_size;
        new_add_size = extensionsize-sizeof(struct re_block);
#ifdef  STATIC_BLOCK
        char extensionAux[STATIC_BLOCK_SIZE];
        memcpy(extensionAux, extension, STATIC_BLOCK_SIZE);
#else
        char *extensionAux = new char[new_add_size];
        memcpy(extensionAux, extension, new_add_size);
#endif
        struct re_block *re_blocksAux = (struct re_block *) extensionAux;
        int numBloks;
        if ((extensionsize) % sizeof(struct re_block) != 0)
        {
            opp_error("re size error");
        }
        else
            numBloks = extensionsize/ sizeof(struct re_block);

        int n = numBloks - blockIndex - 1;
        memmove(&re_blocksAux[blockIndex], &re_blocks[blockIndex+1],
                n * sizeof(struct re_block));
#ifdef  STATIC_BLOCK
        memset(extension, 0, STATIC_BLOCK_SIZE);
        memcpy(extension, extensionAux, new_add_size);
#else
        delete [] extension;
        extension = extensionAux;
        re_blocks = (struct re_block *)  extension;

#endif
        extensionsize = new_add_size;
    }

}

Register_Class(Dymo_UERR);

Dymo_UERR::UERR(const Dymo_UERR& m) : DYMO_element(m)
{
    copy(m);
}

Dymo_UERR& Dymo_UERR::operator=(const Dymo_UERR& msg)
{
    if (this==&msg) return *this;
    clean();
    DYMO_element::operator=(msg);
    copy(msg);
    return *this;
}

void Dymo_UERR::copy(const Dymo_UERR& msg)
{
    uelem_target_addr = msg.uelem_target_addr;
    uerr_node_addr = msg.uerr_node_addr;
    uelem_type = msg.uelem_type;
    setBitLength(msg.getBitLength());
}


Register_Class(Dymo_RERR);

//=========================================================================

Dymo_RERR::Dymo_RERR(const Dymo_RERR& m) : DYMO_element(m)
{
    copy(m);
}

Dymo_RERR& Dymo_RERR::operator=(const Dymo_RERR& msg)
{
    if (this==&msg) return *this;
    clean();
    DYMO_element::operator=(msg);
    copy(msg);
    return *this;
}

void Dymo_RERR::copy(const Dymo_RERR& msg)
{
    rerr_blocks = (struct rerr_block *)extension;
    setBitLength(msg.getBitLength());
}

void Dymo_RERR::newBocks(int n)
{
    if (n<=0)
        return;
    int new_add_size;
    if (ceil((double)extensionsize/(double)sizeof(struct rerr_block))+n>sizeof(struct rerr_block))
        new_add_size = (int)((MAX_RERR_BLOCKS - ceil((double)extensionsize/(double)sizeof(struct rerr_block)))*sizeof(struct rerr_block));
    else
        new_add_size = n*sizeof(struct rerr_block);
    rerr_blocks = (struct rerr_block *) addExtension(new_add_size);
}

void Dymo_RERR::delBocks(int n)
{
    if (n<=0)
        return;
    int del_blk;
    if ((n * sizeof(struct rerr_block))>extensionsize)
        del_blk = (int) extensionsize;
    else
        del_blk = n*sizeof(struct rerr_block);

    rerr_blocks = (struct rerr_block *) delExtension(del_blk);
}


std::string DYMO_element::detailedInfo() const
{
    std::stringstream out;


    Dymo_RE *re_type = NULL;
    Dymo_UERR *uerr_type = NULL;
    Dymo_RERR *rerr_type = NULL;
    Dymo_HELLO *hello_type = NULL;
    switch (type)
    {
    case DYMO_RE_TYPE:
        re_type = (Dymo_RE*) (this);
        if (re_type->a)
            out << "type  : RE rreq  ttl : "<< ttl << "\n";
        else
            out << "type  : RE rrep ttl : "<< ttl << "\n";
        break;
    case DYMO_RERR_TYPE:
        rerr_type = (Dymo_RERR*) (this);
        out << "type  : RERR ttl : "<< ttl << "\n";
        break;
    case DYMO_UERR_TYPE:
        uerr_type = (Dymo_UERR*) (this);
        out << "type  : RERR ttl : "<< ttl << "\n";
        break;
    case DYMO_HELLO_TYPE:
        hello_type = (Dymo_HELLO*) (this);
        out << "type  : HELLO ttl : "<< ttl << "\n";
        break;
    }


    if (re_type)
    {
        IPv4Address addr = IPv4Address(re_type->target_addr.getLo());
        out << " target :" << addr <<"  " << addr.getInt() <<"\n";
        for (int i = 0; i < re_type->numBlocks(); i++)
        {
            out << " --------------- block : " <<  i << "------------------ \n";
            out << "g : " << re_type->re_blocks[i].g << "\n";
            out << "prefix : " <<re_type->re_blocks[i].prefix<< "\n";
            out << "res : " <<re_type->re_blocks[i].res<< "\n";
            out << "re_hopcnt : " <<re_type->re_blocks[i].re_hopcnt<< "\n";
            IPv4Address addr = IPv4Address(re_type->re_blocks[i].re_node_addr.getLo());
            out << "re_node_addr : " << addr << "  " << addr.getInt() << "\n";
            out << "re_node_seqnum : " << re_type->re_blocks[i].re_node_seqnum << "\n";
        }
    }
    else if (rerr_type)
    {
        IPv4Address naddr((uint32_t) notify_addr);
        out << " Notify address " << naddr << "\n"; // Khmm
        for (int i = 0; i < rerr_type->numBlocks(); i++)
        {
            IPv4Address addr = IPv4Address(rerr_type->rerr_blocks[i].unode_addr.getLo());
            out << "unode_addr : " <<addr << "\n";
            out << "unode_seqnum : " << rerr_type->rerr_blocks[i].unode_seqnum << "\n";
        }
    }

    return out.str();
}

