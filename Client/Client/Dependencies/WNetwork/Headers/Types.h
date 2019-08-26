#pragma once

#include <WinSock2.h>

#define Family_Unspecified AF_UNSPEC
#define Family_IPv4 AF_INET
#define Family_IPX AF_IPX
#define Family_AppleTalk AF_APPLETALK
#define Family_NetBIOS AF_NETBIOS
#define Family_IPv6 AF_INET6
#define Family_Infrared AF_IRDA
#define Family_Bluetooth AF_BTH

#define	SocketType_Stream SOCK_STREAM
#define SocketType_Datagram SOCK_DGRAM
#define SocketType_Raw SOCK_RAW
#define SocketType_ReliableDatagram SOCK_RDM
#define SocketType_SeqPacket SOCK_SEQPACKET

#define Protocol_ICMP IPPROTO_ICMP
#define Protocol_IGMP IPPROTO_IGMP
#define Protocol_TCP IPPROTO_TCP
#define Protocol_UDP IPPROTO_UDP
#define Protocol_ICMPv6 IPPROTO_ICMPV6	

typedef int SocketAddressFamily;
typedef int SocketType;
typedef int SocketProtocol;
typedef SOCKET SocketHandle;
typedef sockaddr_in InSocketAddress;
typedef in_addr InAddress;
typedef addrinfo AddressInfo;
typedef sockaddr SocketAddress;
typedef timeval TimeVal;
