/*********************************************************************************************************************

This is an internal function used by the audio server to wait for audio drivers to start.  It does not return until the
drivers have been initialised or the indicated TimeOut has expired.

*********************************************************************************************************************/

#ifdef ALSA_ENABLED
#include "device_enum.h"
#endif

static ERR sndWaitDrivers(int TimeOut)
{
#ifdef ALSA_ENABLED
   // Use the unified device enumeration to wait for drivers
   return ALSADeviceEnumerator::wait_for_devices(TimeOut);
#else
   return ERR::Okay;
#endif
}
