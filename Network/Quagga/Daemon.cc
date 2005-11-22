/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#include "Daemon.h"

#include "Socket_m.h"

#include "IPv4InterfaceData.h"

#include "IPControlInfo_m.h"

#define	QUAGGA_UID	100
#define	QUAGGA_GID	100

Define_Module(Daemon);

extern "C" {
	
int zebra_main_entry (int argc, char **argv);
int ripd_main_entry (int argc, char **argv);
int ospfd_main_entry (int argc, char **argv);

};

#include "oppsim_kernel.h"

void Daemon::activity()
{
	// cached pointer used by DAEMON macro
	current_module = this;
	// global variable structure pointer
	varp = GlobalVars_createActiveSet();
	ASSERT(varp);
	__activeVars = varp;
	// initialize global variables
	GlobalVars_initializeActiveSet_lib();
	
	ev << "Quagga daemon starting" << endl;
	ev << " with variable context " << varp << endl;
	
	blocked = false;
	cwd = "/";
	rootprefix = par("rootfs").stringValue();
	euid = 0; // daemon starts as root
	
	pwd_entry.pw_uid = QUAGGA_UID;
	grp_entry.gr_gid = QUAGGA_GID;
	
	for(int i = 0; i < 32; i++)
	{
		struct sigaction sa;
		sa.sa_handler = SIG_DFL;
		//sigemptyset(&sa.sa_mask); XXX FIXME memset to zero?
		sa.sa_flags = 0;
		sig.push_back(sa);
	}
	
	const char *server = par("server").stringValue();
	
	char **cmdline;
	cmdline = (char**)malloc(2);
	cmdline[0] = const_cast<char *>(server);
	cmdline[1] = NULL;
	
	if(!strcmp(server, "zebra"))
	{
		// randomize start
		wait(uniform(0, 0.001));
		current_module = this;
		__activeVars = varp;
		GlobalVars_initializeActiveSet_zebra();
		
		ev << "ready for zebra_main_entry()" << endl;
		
		zebra_main_entry(1, cmdline);
	}
	else if(!strcmp(server, "ripd"))
	{
		// randomize start
		wait(uniform(0.001, 0.002));
		current_module = this;
		__activeVars = varp;
		GlobalVars_initializeActiveSet_ripd();
		
		ev << "ready for ripd_main_entry()" << endl;
		
		ripd_main_entry(1, cmdline);
	}
	else if(!strcmp(server, "ospfd"))
	{
		// randomize start
		wait(uniform(0.002, 0.003));
		current_module = this;
		__activeVars = varp;
		GlobalVars_initializeActiveSet_ospfd();
		
		ev << "ready for ospfd_main_entry()" << endl;
		
		ospfd_main_entry(1, cmdline);
	}
	else
	{
		ev << "daemon " << server << " not recognized" << endl;
		
		ASSERT(false);
	}
}

TCPSocket* Daemon::gettcpsocket(int socket)
{
	int idx = FD_SUB(socket);
	ASSERT(idx >= 0);
	ASSERT(idx < fd.size());
	ASSERT(fd[idx].type == FD_TCP);
	return fd[idx].tcp;
}

UDPSocket* Daemon::getudpsocket(int socket)
{
	int idx = FD_SUB(socket);
	ASSERT(idx >= 0);
	ASSERT(idx < fd.size());
	ASSERT(fd[idx].type == FD_UDP);
	return fd[idx].udp;
}

RawSocket* Daemon::getrawsocket(int socket)
{
	int idx = FD_SUB(socket);
	ASSERT(idx >= 0);
	ASSERT(idx < fd.size());
	ASSERT(fd[idx].type == FD_RAW);
	return fd[idx].raw;
}

Netlink* Daemon::getnlsocket(int socket)
{
	int idx = FD_SUB(socket);
	ASSERT(idx >= 0);
	ASSERT(idx < fd.size());
	ASSERT(fd[idx].type == FD_NETLINK);
	return fd[idx].netlink;
}

bool Daemon::istcpsocket(int socket)
{
	int idx = FD_SUB(socket);
	return (idx >= 0 && idx < fd.size() && fd[idx].type == FD_TCP);
}

bool Daemon::isudpsocket(int socket)
{
	int idx = FD_SUB(socket);
	return (idx >= 0 && idx < fd.size() && fd[idx].type == FD_UDP);
}

bool Daemon::israwsocket(int socket)
{
	int idx = FD_SUB(socket);
	return (idx >= 0 && idx < fd.size() && fd[idx].type == FD_RAW);
}

bool Daemon::isnlsocket(int socket)
{
	int idx = FD_SUB(socket);
	return (idx >= 0 && idx < fd.size() && fd[idx].type == FD_NETLINK);
}

std::string Daemon::getcwd()
{
	return cwd;
}

std::string Daemon::getrootprefix()
{
	return rootprefix;
}

struct sigaction * Daemon::sigactionimpl(int signo)
{
	ASSERT(signo >= 0);
	ASSERT(signo < sig.size());
	return &sig[signo];
}

bool Daemon::isfile(int fildes)
{
	if(fildes==0)
		return true;
	
	int idx = FD_SUB(fildes);
	return (idx >= 0 && idx < fd.size() && fd[idx].type == FD_FILE);
}

bool Daemon::issocket(int fildes)
{
	int idx = FD_SUB(fildes);
	return (idx >= 0 && idx < fd.size() && (fd[idx].type == FD_TCP || fd[idx].type == FD_NETLINK ||
				fd[idx].type == FD_UDP || fd[idx].type == FD_RAW));
}

FILE* Daemon::getstream(int fildes)
{
	if(fildes==0)
		return stdout;
		
	ASSERT(isfile(fildes));
	int idx = FD_SUB(fildes);
	ASSERT(fd[idx].stream);
	return fd[idx].stream;
}

int Daemon::findemptydesc()
{
	// return existing unused descriptor
	for(unsigned int i = 0; i < fd.size(); i++)
	{
		if(fd[i].type == FD_EMPTY)
			return FD_ADD(i);
	}
	
	// allocate new, make sure there is enough room
	ASSERT(FD_ADD(fd.size()) < FD_SETSIZE);
	lib_descriptor_t newItem;
	newItem.type = FD_EMPTY;
	fd.push_back(newItem);
	return FD_ADD(fd.size()-1);
}

int Daemon::createFile(const char *path, char *mode)
{
	int fdesc = findemptydesc();
	
	ASSERT(fdesc < FD_SETSIZE);
	ASSERT(FD_SUB(fdesc) < fd.size());
	ASSERT(fd[FD_SUB(fdesc)].type == FD_EMPTY);
	
	lib_descriptor_t newItem;
	newItem.type = FD_FILE;
	newItem.stream = fopen(path, mode);
	fd[FD_SUB(fdesc)] = newItem;
	
	ASSERT(newItem.stream);
	
	ev << "created new file descriptor=" << fdesc << " for file=" << path << endl;
	
	return fdesc;
}

int Daemon::createRawSocket(int protocol)
{
	int fdesc = findemptydesc();

	ASSERT(fdesc < FD_SETSIZE);
	ASSERT(FD_SUB(fdesc) < fd.size());
	ASSERT(fd[FD_SUB(fdesc)].type == FD_EMPTY);
	
	RawSocket *raw = new RawSocket(fdesc, protocol);
	
	cGate *g = gate("to_ip_interface");
	ASSERT(g);
	raw->setOutputGate(g);
	
	lib_descriptor_t newItem;
	newItem.type = FD_RAW;
	newItem.raw = raw;
	fd[FD_SUB(fdesc)] = newItem;
	
	return fdesc;
}

int Daemon::createUdpSocket()
{
	int fdesc = findemptydesc();
	
	ASSERT(fdesc < FD_SETSIZE);
	ASSERT(FD_SUB(fdesc) < fd.size());
	ASSERT(fd[FD_SUB(fdesc)].type == FD_EMPTY);
	
	UDPSocket *udp = new UDPSocket(fdesc);
	
	cGate *g = gate("to_udp_interface");
	ASSERT(g);
	udp->setOutputGate(g);
	
	lib_descriptor_t newItem;
	newItem.type = FD_UDP;
	newItem.udp = udp;
	fd[FD_SUB(fdesc)] = newItem;
	
	ev << "created new UDP socket=" << fdesc << endl;
	
	return fdesc;
}

int Daemon::createTcpSocket(cMessage *msg)
{
	int fdesc = findemptydesc();
	
	ASSERT(fdesc < FD_SETSIZE);
	ASSERT(FD_SUB(fdesc) < fd.size());
	ASSERT(fd[FD_SUB(fdesc)].type == FD_EMPTY);
	
	TCPSocket *tcp;
	
	if(msg)
	{
		tcp = new TCPSocket(msg);
	}
	else
	{
		tcp = new TCPSocket();
	}
		
	cGate *g = gate("to_tcp_interface");
	ASSERT(g);
	tcp->setOutputGate(g);
	tcp->setCallbackObject(this, (void*)fdesc);
	
	lib_descriptor_t newItem;
	newItem.type = FD_TCP;
	newItem.tcp = tcp;
	fd[FD_SUB(fdesc)] = newItem;
	
	ev << "created new TCP socket=" << fdesc << endl;
	
	socketMap.addSocket(tcp);
	
	return fdesc;
}

int Daemon::getaccepthead(int socket, bool remove)
{
	int idx = FD_SUB(socket);
	ASSERT(idx >= 0);
	ASSERT(idx < fd.size());
	if(!fd[idx].serv.empty())
	{
		ASSERT(fd[idx].type == FD_TCP);
		
		int ret = fd[idx].serv.front();
		if(remove)
		{
			fd[idx].serv.pop_front();
		}
		return ret;
	}
	else
	{
		return -1;
	}
}

cMessage* Daemon::getqueuetail(int socket, bool remove)
{
	int idx = FD_SUB(socket);
	ASSERT(idx >= 0);
	ASSERT(idx < fd.size());
	
	if(!fd[idx].queue.empty())
	{
		cMessage *msg = (cMessage*)fd[idx].queue.tail();
		if(remove)
			fd[idx].queue.remove(msg);
		return msg;
	}
	else
	{
		return NULL;
	}
}

void Daemon::enqueue(int socket, cMessage *msg)
{
	int idx = FD_SUB(socket);
	ASSERT(idx >= 0);
	ASSERT(idx < fd.size());
	fd[idx].queue.insert(msg);
}

void Daemon::setblocked(bool b)
{
	ASSERT(blocked != b);
	blocked = b;
}

int Daemon::createNetlinkSocket()
{
	int fdesc = findemptydesc();

	ASSERT(fdesc < FD_SETSIZE);
	ASSERT(FD_SUB(fdesc) < fd.size());
	ASSERT(fd[FD_SUB(fdesc)].type == FD_EMPTY);
	
	Netlink *nl = new Netlink();
	
	lib_descriptor_t newItem;
	newItem.type = FD_NETLINK;
	newItem.netlink = nl;
	fd[FD_SUB(fdesc)] = newItem;
	
	ev << "created new Netlink socket=" << fdesc << endl;
	
	return fdesc;
}

void Daemon::closefile(int fildes)
{
	ASSERT(isfile(fildes));
	int idx = FD_SUB(fildes);
	ASSERT(fd[idx].stream);
	fclose(fd[idx].stream);

	fd[idx].stream = NULL;
	fd[idx].type = FD_EMPTY;
}

void Daemon::closesocket(int socket)
{
	ASSERT(issocket(socket));
	
	int idx = FD_SUB(socket);
	
	if(fd[idx].type == FD_UDP)
	{
		ASSERT(fd[idx].udp);
		delete fd[idx].udp;
		fd[idx].udp = NULL;
		fd[idx].type = FD_EMPTY;
		return;
	}
	
	ASSERT(false);
}

int Daemon::incomingrawsocket(cMessage *msg)
{
	// find socket with the same protocol or what?
	
	IPControlInfo *ipControlInfo = dynamic_cast<IPControlInfo*>(msg->controlInfo());
	
	ASSERT(ipControlInfo->protocol() == IP_PROT_OSPF);
	
	for(int i = 0; i < fd.size(); i++)
	{
		if(fd[i].type != FD_RAW)
			continue;
			
		if(fd[i].raw->getProtocol() != ipControlInfo->protocol())
			continue;
			
		return FD_ADD(i);
	}
	
	return -1;
}

int Daemon::incomingtcpsocket(cMessage *msg)
{
	ASSERT(TCPSocket::belongsToAnyTCPSocket(msg));

	TCPSocket *socket = socketMap.findSocketFor(msg);
	if(!socket)
	{
		ev << "connection not found" << endl;
		return -1;
	}

	ASSERT(socket);
	
	for(int i = 0; i < fd.size(); i++)
	{
		if(fd[i].type != FD_TCP)
			continue;
			
		if(fd[i].tcp != socket)
			continue;
			
		return FD_ADD(i);
	}
	
	ASSERT(false);
}

void Daemon::enqueuConn(int socket, int csocket)
{
	ASSERT(istcpsocket(socket));
	ASSERT(istcpsocket(csocket));
	ASSERT(socket < csocket);
	fd[FD_SUB(socket)].serv.push_back(csocket);
}

int Daemon::findparentsocket(int socket)
{
	ASSERT(istcpsocket(socket));
	
	int idx = FD_SUB(socket);
	int port = fd[idx].tcp->localPort();
	IPAddress addr = fd[idx].tcp->localAddress().get4();
	
	for(unsigned int i = 0; i < fd.size(); i++)
	{
		if(fd[i].type != FD_TCP)
			continue;
			
		if(idx == i)
			continue;
			
		if(fd[i].tcp->localPort() != port)
			continue;
			
		if(!fd[i].tcp->localAddress().equals(addr))
			continue;
			
		return FD_ADD(i);
	}
	
	ASSERT(false);
}

void Daemon::socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent)
{
	int socket = (long)yourPtr;
	
	ev << "data arrived on socket=" << socket << endl;
	
	enqueue(socket, msg);
}

void Daemon::socketEstablished(int connId, void *yourPtr)
{
	ev << "socket=" << (long)yourPtr << " established" << endl;
}
