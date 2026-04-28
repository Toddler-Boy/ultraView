#include "NetworkHardwareChecker.h"

#if JUCE_WINDOWS
	#include <winsock2.h>
	#include <ws2tcpip.h>

	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	#include <iphlpapi.h>
#elif JUCE_MAC
	#include <SystemConfiguration/SystemConfiguration.h>
	#include <net/if.h>
	#include <ifaddrs.h>
#elif JUCE_LINUX
	#include <ifaddrs.h>
	#include <net/if.h>
	#include <sys/ioctl.h>
	#include <linux/wireless.h>
#endif

//-----------------------------------------------------------------------------

NetworkHardwareChecker::NetworkHardwareChecker ()
{
	createMap ();
}
//-----------------------------------------------------------------------------

bool NetworkHardwareChecker::isWiredAndActive ( const juce::IPAddress& addr ) const
{
	if ( ! interfaceMap.contains ( addr ) )
		return false;

	const auto&	info = interfaceMap.at ( addr );

	return info.isWired && info.isActive;
}
//-----------------------------------------------------------------------------

void NetworkHardwareChecker::createMap ()
{
	#if JUCE_WINDOWS

	ULONG	outBufLen = 15000;
	auto	pAddresses = (IP_ADAPTER_ADDRESSES*)std::malloc ( outBufLen );

	if ( GetAdaptersAddresses ( AF_INET, 0, nullptr, pAddresses, &outBufLen ) == NO_ERROR )
	{
		for ( auto pCurr = pAddresses; pCurr != nullptr; pCurr = pCurr->Next )
		{
			InterfaceInfo	info;
			info.isWired = ( pCurr->IfType == IF_TYPE_ETHERNET_CSMACD );
			info.isActive = ( pCurr->OperStatus == IfOperStatusUp );

			for ( auto pUnicast = pCurr->FirstUnicastAddress; pUnicast != nullptr; pUnicast = pUnicast->Next )
			{
				auto*	sa = (sockaddr_in*)pUnicast->Address.lpSockaddr;
				interfaceMap[ juce::IPAddress ( ntohl ( sa->sin_addr.S_un.S_addr ) ) ] = info;
			}
		}
	}
	std::free ( pAddresses );

	#elif JUCE_MAC || JUCE_LINUX
	struct ifaddrs* interfaces = nullptr;
	if ( getifaddrs ( &interfaces ) == 0 )
	{
		for ( auto* ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next )
		{
			if ( ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_INET )
				continue;

			InterfaceInfo info;
			info.isActive = ( ifa->ifa_flags & IFF_UP ) && ( ifa->ifa_flags & IFF_RUNNING );

			#if JUCE_MAC
				// Mac Hardware Check
			if ( auto interfaceList = SCNetworkInterfaceCopyAll () )
			{
				for ( CFIndex i = 0; i < CFArrayGetCount ( interfaceList ); ++i )
				{
					auto interface = (SCNetworkInterfaceRef)CFArrayGetValueAtIndex ( interfaceList, i );
					auto bsdName = SCNetworkInterfaceGetBSDName ( interface );
					if ( CFStringCompare ( bsdName, juce::String ( ifa->ifa_name ).toCFString (), 0 ) == kCFCompareEqualTo )
					{
						auto type = SCNetworkInterfaceGetInterfaceType ( interface );
						info.isWired = CFStringCompare ( type, kSCNetworkInterfaceTypeEthernet, 0 ) == kCFCompareEqualTo;
						break;
					}
				}
				CFRelease ( interfaceList );
			}
			#elif JUCE_LINUX
				// Linux Wireless Check
			int sock = socket ( AF_INET, SOCK_DGRAM, 0 );
			struct iwreq pwrq;
			std::memset ( &pwrq, 0, sizeof ( pwrq ) );
			std::strncpy ( pwrq.ifr_name, ifa->ifa_name, IFNAMSIZ );
			info.isWired = ( ioctl ( sock, SIOCGIWNAME, &pwrq ) == -1 );
			close ( sock );
			#endif

			auto* sa = (sockaddr_in*)ifa->ifa_addr;
			interfaceMap[ juce::IPAddress ( ntohl ( sa->sin_addr.s_addr ) ) ] = info;
		}
		freeifaddrs ( interfaces );
	}
	#endif
}
//-----------------------------------------------------------------------------
