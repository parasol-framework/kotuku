/*********************************************************************************************************************

-MODULE-
Backstage: Provides a REST backend for interacting with the process over the network.

Backstage provides a REST backend for users and applications to interact with a Kōtuku program while it is running.
The module does not expose any API functionality, and is instead enabled by the user by specifying
`--backstage [port]` on the commandline.  If the command is omitted then backstage will do nothing.

The REST API and documentation on how to use Backstage is documented in the Kotuku Wiki.

-END-

See interface.tiri for the REST interface.

*********************************************************************************************************************/

#define PRV_BACKSTAGE

#include <kotuku/main.h>
#include <kotuku/modules/backstage.h>
#include <kotuku/modules/network.h>
#include <kotuku/strings.hpp>

using namespace kt;

static OBJECTPTR modNetwork = nullptr;

JUMPTABLE_CORE
JUMPTABLE_NETWORK

static ERR init_backstage(int = 8765);

class objNetSocket *glServer = nullptr;

//********************************************************************************************************************

static ERR MODInit(OBJECTPTR argModule, struct CoreBase *argCoreBase)
{
   kt::Log log("Backstage");

   CoreBase = argCoreBase;

   if (objModule::load("network", &modNetwork, &NetworkBase) != ERR::Okay) return ERR::InitModule;

   // Parse commandline arguments to confirm if the user wants to enable Backstage.

   if (auto state = GetSystemState()) {
      for (int i=0; i < state->OpenInfo->ArgCount; i++) {
         if (kt::iequals(state->OpenInfo->Args[i], "--backstage")) {
            if (i + 1 < state->OpenInfo->ArgCount) {
               int port = atoi(state->OpenInfo->Args[i + 1]);
               if (port > 0) {
                  init_backstage(port);
                  break;
               }
               else {
                  init_backstage();
                  break;
               }
            }
            else {
               init_backstage();
               break;
            }
         }
      }
   }

   return(ERR::Okay);
}

//********************************************************************************************************************

static ERR MODExpunge(void)
{
   if (modNetwork) { FreeResource(modNetwork); modNetwork = nullptr; }
   return ERR::Okay;
}

//********************************************************************************************************************

void server_feedback(objNetServer *Server, class objClientSocket *Client, NTC State)
{
   kt::Log log(__FUNCTION__);

   if (State IS NTC::CONNECTED) {
      log.msg("Client socket #%d connected.", Client->UID);
   }
   else if (State IS NTC::DISCONNECTED) {
      log.msg("Client socket #%d disconnected.", Client->UID);
   }
   else log.msg("Unknown state: %d", int(State));
}

//********************************************************************************************************************

ERR server_incoming(objNetServer *Server, objClientSocket *Client, APTR Meta)
{
   kt::Log log(__FUNCTION__);
   log.msg("Received incoming data from client socket #%d", Client->UID);
   return ERR::Okay;
}

//********************************************************************************************************************

ERR init_backstage(int Port)
{
   kt::Log log(__FUNCTION__);

   glServer = objNetServer::create::global({
      fl::Port(Port),
      fl::Flags(int(NSF::MULTI_CONNECT)),
      fl::Feedback((CPTR)server_feedback),
      fl::Incoming((CPTR)server_incoming)
   });

   if (not glServer) {
      log.msg("Failed to initialise backstage server on port %d", Port);
      return ERR::CreateObject;
   }
   else {
      log.msg("Backstage is enabled at http://localhost:%d/", Port);
      return ERR::Okay;
   }
}

//********************************************************************************************************************

KOTUKU_MOD(MODInit, nullptr, nullptr, MODExpunge, nullptr, MOD_IDL, nullptr)
extern "C" struct ModHeader * register_backstage_module() { return &ModHeader; }
