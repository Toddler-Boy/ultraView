#include <juce_core/juce_core.h>

//-----------------------------------------------------------------------------

class NetworkHardwareChecker
{
public:
	NetworkHardwareChecker ();

	[[ nodiscard ]] bool isWiredAndActive ( const juce::IPAddress& addr ) const;

private:
	struct InterfaceInfo
	{
		bool    isWired = false;
		bool    isActive = false;
	};

	std::map<juce::IPAddress, InterfaceInfo>	interfaceMap;

	void createMap ();
};
//-----------------------------------------------------------------------------
