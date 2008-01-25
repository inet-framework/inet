
#include "UnitTest.h"

#include "XMLUtils.h"

#include "oppsim_kernel.h"

/*
 * Basic socket communication functionality.
 */
 
enum event_kind { read_event, write_event };
enum read_kind { read_syscall, recvfrom_syscall, recvmsg_syscall };
enum write_kind { write_syscall, sendto_syscall, sendmsg_syscall };

int waitForEvent(int socket, event_kind t)
{
	fd_set myset;
	FD_ZERO(&myset); 
    FD_SET(socket, &myset);
    
    return oppsim_select(socket + 1, t == read_event? &myset: NULL, t == write_event? &myset: NULL, NULL, NULL); 
}	

int checkSocket(int socket)
{
	socklen_t len = sizeof(int);
	int valopt; 
	int ret = oppsim_getsockopt(socket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &len);
	ASSERT(ret == 0);
	return valopt;	
}

int doConnect(int socket, const sockaddr *addr, int len, bool blocking)
{
	int ret = oppsim_connect(socket, addr, len);
	
	if(ret < 0 && !blocking && *GlobalVars_errno() == EINPROGRESS) 
	{
		ret = waitForEvent(socket, write_event);
		if(ret < 0) return ret;
		if(checkSocket(socket)) return -1;
		else return 0;
	}
	else
		return ret;
}

int doAccept(int socket, sockaddr *addr, socklen_t *len, bool blocking)
{
	while(true)
	{
		int ret = oppsim_accept(socket, addr, len);
		if(ret < 0 && !blocking && (*GlobalVars_errno() == EAGAIN || *GlobalVars_errno() == EWOULDBLOCK))
		{
			if(waitForEvent(socket, read_event) < 0 || checkSocket(socket)) return -1;
		}
		else
		{
			return ret;
		}
	}
}

ssize_t doRead(int socket, void *buf, size_t nbyte, bool blocking, read_kind method, sockaddr *addr, socklen_t *len)
{
	while(true) // XXX FIXME: refactor while (is in doAccept too)
	{
		int ret;
		switch(method)
		{
			case read_syscall:
				ret = oppsim_read(socket, buf, nbyte);
				break;
				
			case recvmsg_syscall:
			{
				// XXX FIXME: currently not supported by oppsim_kernel
				struct iovec iov;
				iov.iov_base = buf;
				iov.iov_len = nbyte;
				
				struct msghdr message;
				message.msg_name = 0;
				message.msg_namelen = 0;
				message.msg_iov = &iov;
				message.msg_iovlen = 1;
				message.msg_control = 0;
				message.msg_controllen = 0;
				 
				ret = oppsim_recvmsg(socket, &message, 0);
				break; 
			}
			
			case recvfrom_syscall:
			{
				ret = oppsim_recvfrom(socket, buf, nbyte, 0, addr, len);
				break;
			}
		}
		
		
		if(ret < 0 && !blocking && (*GlobalVars_errno() == EAGAIN || *GlobalVars_errno() == EWOULDBLOCK))
		{
			if(waitForEvent(socket, read_event) < 0 || checkSocket(socket)) return -1;
		}
		else
		{
			return ret;
		}
	}
}

ssize_t doWrite(int socket, const void *buf, size_t nbyte, write_kind method, const sockaddr *addr, int addrlen)
{
	switch(method)
	{
		case write_syscall:
			return oppsim_write(socket, buf, nbyte);
			
		case sendto_syscall:
			return oppsim_sendto(socket, buf, nbyte, 0, addr, addrlen); 
			
		case sendmsg_syscall:
			ASSERT(false);
	}
} 

void setBlocking(int socket, bool blocking)
{
	long arg = oppsim_fcntl(socket, F_GETFL, NULL);
	ASSERT(arg >= 0);
	if(blocking) arg &= ~O_NONBLOCK; else arg |= O_NONBLOCK;
  	arg = oppsim_fcntl(socket, F_SETFL, arg);
  	ASSERT(arg == 0);
}  

UNIT_TEST(TestSocket_writer)
{
	int destport = getParameterIntValue(config, "destport", 8080);
	const char *destaddr = getParameterStrValue(config, "destaddr", "10.0.4.1");
	int msgsize = getParameterIntValue(config, "msgsize", 1000);
	bool blocking = getParameterBoolValue(config, "blocking", true);
	int srcport = getParameterIntValue(config, "srcport", -1);
	
	// TCP vs UDP option
	int proto;
	const char *typestr = getParameterStrValue(config, "type", "stream");
	if(!strcasecmp(typestr, "stream")) proto = SOCK_STREAM;
	else if(!strcasecmp(typestr, "dgram")) proto = SOCK_DGRAM;
	else ASSERT(false); // invalid parameter 
	
	// write vs sendto vs sendmsg option	
	write_kind method;
	const char *methodstr = getParameterStrValue(config, "method", proto==SOCK_STREAM?"write": "sendto");
	if(!strcasecmp(methodstr, "write")) method = write_syscall;
	else if(!strcasecmp(methodstr, "sendto")) method = sendto_syscall;
	else if(!strcasecmp(methodstr, "sendmsg")) method = sendmsg_syscall;
	else ASSERT(false); // invalid parameter
	
	int socket = oppsim_socket(AF_INET, proto, 0);
	if(socket < 0)
	{
		output("socket failed");
		return;
	}
	
	setBlocking(socket, blocking);
	
	struct sockaddr_in addr;
	
	if(srcport >= 0)
	{
		// only bind if local address was specified
		
		addr.sin_family = AF_INET;
		addr.sin_port = htons(srcport);
		addr.sin_addr.s_addr = INADDR_ANY;
		if(oppsim_bind(socket, (const sockaddr*)&addr, sizeof(addr)) < 0)
		{
			output("bind failed");
			return;
		}
		output("bound");
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(destport);
	addr.sin_addr.s_addr = inet_addr(destaddr);
	
	if(proto == SOCK_STREAM)
	{
		// only connect if doing TCP
		
		if(doConnect(socket, (const sockaddr*)&addr, sizeof(addr), blocking) < 0)
		{
			output("connect failed");
			return;
		}

		output("connected");
	}
	
	std::string message = createMessage(msgsize);
	
	oppsim_sleep(1);

	int ret = doWrite(socket, message.c_str(), strlen(message.c_str()), method, (const sockaddr*)&addr, sizeof(addr));
	
	std::stringstream s;
	s << "written " << ret << " bytes";
	output(s.str());

	if(oppsim_close(socket))
	{
		output("close failed");
		return;
	}
	
	output("closed");
}

UNIT_TEST(TestSocket_reader)
{
	int srcport = getParameterIntValue(config, "srcport", 8080);
	bool blocking = getParameterBoolValue(config, "blocking", true);
	
	// TCP vs UDP option
	int proto;
	const char *typestr = getParameterStrValue(config, "type", "stream");
	if(!strcasecmp(typestr, "stream")) proto = SOCK_STREAM;
	else if(!strcasecmp(typestr, "dgram")) proto = SOCK_DGRAM;
	else ASSERT(false); // invalid parameter 
	
	// read vs recvfrom vs recvmsg option	
	read_kind method;
	const char *methodstr = getParameterStrValue(config, "method", proto==SOCK_STREAM?"read": "recvfrom");
	if(!strcasecmp(methodstr, "read")) method = read_syscall;
	else if(!strcasecmp(methodstr, "recvfrom")) method = recvfrom_syscall;
	else if(!strcasecmp(methodstr, "recvmsg")) method = recvmsg_syscall;
	else ASSERT(false); // invalid parameter
	
	int socket = oppsim_socket(AF_INET, proto, 0);
	if(socket < 0)
	{
		output("socket failed");
		return;
	}
	
	setBlocking(socket, blocking);
	
	struct sockaddr_in addr;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(srcport);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(oppsim_bind(socket, (const sockaddr*)&addr, sizeof(addr)))
	{
		output("bind failed");
		return;
	}
	
	output("bound");
	
	std::stringstream s;

	int csocket;
	
	socklen_t len = sizeof(addr);
	
	if(proto == SOCK_STREAM)
	{
		// only listen/accept if doing TCP
	
		if(oppsim_listen(socket, 128))
		{
			output("listen failed");
			return;
		}
	
		output("listening");
		
		if((csocket = doAccept(socket, (sockaddr*)&addr, &len, blocking)) < 0)
		{
			output("accept failed");
			return;
		}
	
		s << "accepted " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port);
		output(s);
		
		setBlocking(csocket, blocking); // flags are not inherited
	}
	else
	{
		csocket = socket;
	}
	
	char buff[4096];
	int n = doRead(csocket, buff, sizeof(buff), blocking, method, (sockaddr*)&addr, &len);
	if(n < 0)
	{
		output("error reading from socket");
		return;
	}
	
	s << "read " << n << " bytes (" << checkMessage(buff, n) << ") from " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port);
	output(s);
		
	if((csocket != socket && oppsim_close(csocket)) || oppsim_close(socket))
	{
		output("close failed");
		return;
	}
	
	output("closed");
}
