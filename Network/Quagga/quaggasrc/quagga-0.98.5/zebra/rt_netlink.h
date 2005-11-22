#ifndef RT_NETLINK_H_
#define RT_NETLINK_H_

struct nlsock
{
  int sock;
  int seq;
  struct sockaddr_nl snl;
  const char *name;
};

#endif /*RT_NETLINK_H_*/
