/*********************************************************************************************************************

-MODULE-
Backstage: Provides a REST service for interacting with running processes.

Backstage provides an embedded REST service that allows a Kōtuku program to be interrogated and modified while it is
running.  Backstage does not export its own API functions, but is instead enabled by the user by specifying
`--backstage [port]` on the commandline.  If the parameter is omitted then Backstage will not be loaded.  The default
port for accessing Backstage is 8765.

The REST API and documentation on how to use Backstage is documented in the Kōtuku Wiki.

-END-

See interface.tiri for the REST interface.

*********************************************************************************************************************/

#define PRV_BACKSTAGE

#include <kotuku/main.h>
#include <kotuku/modules/backstage.h>
#include <kotuku/modules/network.h>
#include <kotuku/modules/regex.h>
#include <kotuku/strings.hpp>

#include <array>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

using namespace kt;

static OBJECTPTR modNetwork = nullptr;

JUMPTABLE_CORE
JUMPTABLE_NETWORK

static ERR init_backstage(int = 8765);

class objNetSocket *glServer = nullptr;
static std::mutex glRequestLock;
static std::unordered_map<OBJECTID, std::string> glRequestBuffers;

static constexpr size_t MAX_REQUEST_HEADER = 16 * 1024;
static constexpr CSTRING PING_BODY = "{\"status\":\"pong\"}";

struct BackstageRoute {
   std::string_view method;  // HTTP method
   std::string_view path;    // Human readable path (decorative)
   APTR handler;             // Function handling this route
   Regex *regex = nullptr;

   BackstageRoute() {}

   BackstageRoute(std::string_view pMethod, std::string_view pPath, Regex *pRegex, APTR pHandler) :
      method(pMethod), path(pPath), handler(pHandler), regex(pRegex) {}

   BackstageRoute(std::string_view pMethod, std::string_view pPath, std::string_view pRegex, APTR pHandler) :
      method(pMethod), path(pPath), handler(pHandler) {
      if (rx::Compile(pRegex, REGEX::NIL, nullptr, &regex) != ERR::Okay) {
         kt::Log().warning("Failed to compile regex: %.*s", pRegex.data(), int(pRegex.size()));
      }
   }

   ~BackstageRoute() {
      if (regex) FreeResource(regex);
   }
};

struct BackstageRequest {
   BackstageRoute   *route;
   objNetServer     *server;
   objClientSocket  *client;
   std::string_view path;
   std::string_view rawPath;
   std::string_view queryString;
   std::span<const uint8_t> body;
   std::unordered_map<std::string_view, std::string_view> pathParams;
   std::unordered_map<std::string_view, std::string_view> queryParams;
};

struct BackstageResponse {
   int status;
   std::string contentType;
   std::string body;
};

//********************************************************************************************************************

struct RequestLine {
   std::string_view method;
   std::string_view path;
   std::string_view version;
};

//********************************************************************************************************************

static bool parse_request_line(std::string_view Request, RequestLine &Line)
{
   auto line_end = Request.find("\r\n");
   if (line_end IS std::string_view::npos) return false;

   auto request_line = Request.substr(0, line_end);
   auto method_end = request_line.find(' ');
   if ((method_end IS std::string_view::npos) or (method_end <= 0)) return false;

   auto path_start = request_line.find_first_not_of(' ', method_end);
   if (path_start IS std::string_view::npos) return false;

   auto path_end = request_line.find(' ', path_start);
   if ((path_end IS std::string_view::npos) or (path_end <= path_start)) return false;

   auto version_start = request_line.find_first_not_of(' ', path_end);
   if (version_start IS std::string_view::npos) return false;

   auto version_end = request_line.find(' ', version_start);
   if (version_end != std::string_view::npos) return false;

   Line.method = request_line.substr(0, method_end);
   Line.path = request_line.substr(path_start, path_end - path_start);
   Line.version = request_line.substr(version_start);

   if (!Line.version.starts_with("HTTP/")) return false;

   auto query = Line.path.find('?');
   if (query != std::string_view::npos) Line.path = Line.path.substr(0, query);

   return true;
}

//********************************************************************************************************************

static void write_response(objClientSocket *Client, int Status, std::string_view StatusText, std::string_view Body,
   std::string_view ContentType = "text/plain", std::string_view ExtraHeaders = {})
{
   std::string response;
   response.reserve(128 + Body.size() + ExtraHeaders.size());
   response.append("HTTP/1.1 ");
   response.append(std::to_string(Status));
   response.append(" ");
   response.append(StatusText);
   response.append("\r\nContent-Type: ");
   response.append(ContentType);
   response.append("\r\nContent-Length: ");
   response.append(std::to_string(Body.size()));
   response.append("\r\nConnection: close\r\n");
   response.append(ExtraHeaders);
   response.append("\r\n");
   response.append(Body);

   int result;
   Client->write(response.c_str(), int(response.size()), &result);
   Client->deactivate();
}

//********************************************************************************************************************

static void process_request(objClientSocket *Client, std::string_view Request)
{
   RequestLine line;

   if (not parse_request_line(Request, line)) {
      write_response(Client, 400, "Bad Request", "Bad Request");
      return;
   }

   if (line.path.compare("/ping") IS 0) {
      if (line.method.compare("GET") IS 0) {
         write_response(Client, 200, "OK", PING_BODY, "application/json");
      }
      else write_response(Client, 405, "Method Not Allowed", "Method Not Allowed", "text/plain", "Allow: GET\r\n");
   }
   else write_response(Client, 404, "Not Found", "Not Found");
}

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
   {
      std::lock_guard<std::mutex> lock(glRequestLock);
      glRequestBuffers.clear();
   }
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
      std::lock_guard<std::mutex> lock(glRequestLock);
      glRequestBuffers.erase(Client->UID);
   }
   else log.msg("Unknown state: %d", int(State));
}

//********************************************************************************************************************

ERR server_incoming(objNetServer *Server, objClientSocket *Client, APTR Meta)
{
   kt::Log log(__FUNCTION__);
   std::array<char, 4096> buffer;
   int len = 0;

   auto error = Client->read(buffer.data(), buffer.size(), &len);
   if (error IS ERR::Disconnected) return ERR::Terminate;
   if (error != ERR::Okay) return error;
   if (not len) return ERR::Okay;

   log.trace("Received %d bytes from client socket #%d", len, Client->UID);

   std::string request;
   bool complete = false;
   bool overflow = false;

   {
      std::lock_guard<std::mutex> lock(glRequestLock);
      auto &client_buffer = glRequestBuffers[Client->UID];
      client_buffer.append(buffer.data(), size_t(len));
      overflow = client_buffer.size() > MAX_REQUEST_HEADER;
      complete = client_buffer.find("\r\n\r\n") != std::string::npos;
      if (complete or overflow) {
         request = client_buffer;
         glRequestBuffers.erase(Client->UID);
      }
   }

   if (overflow) {
      write_response(Client, 400, "Bad Request", "Bad Request");
      return ERR::Okay;
   }

   if (complete) process_request(Client, request);

   return ERR::Okay;
}

//********************************************************************************************************************

ERR init_backstage(int Port)
{
   kt::Log log(__FUNCTION__);

   glServer = objNetServer::create::global({
      fl::Port(Port),
      fl::Flags(int(NSF::MULTI_CONNECT|NSF::KEEP_ALIVE)),
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
