/*********************************************************************************************************************

Windows-specific SSL implementation using wrapper
This file provides the same interface as ssl.cpp but uses the Windows SSL wrapper internally.

*********************************************************************************************************************/

//********************************************************************************************************************
// Disconnect SSL connection on Windows

template <class T> void tls_disconnect(T *Self)
{
   if (Self->TLS.Handle) {
      kt::Log(__FUNCTION__).traceBranch("Closing SSL connection.");
      ssl_free_context(Self->TLS.Handle);
      Self->TLS.Handle = nullptr;
   }
}

//********************************************************************************************************************
// SSL Debug callback - forwards debug messages from the Windows wrapper to Kotuku log system

extern "C" void ssl_debug_to_kotuku_log(const char* message, int level)
{
   kt::Log log("SSL");

   switch (level) {
      case SSL_DEBUG_ERROR:   log.warning("%s", message); break;
      case SSL_DEBUG_WARNING: log.warning("%s", message); break;
      case SSL_DEBUG_INFO:    log.msg("%s", message); break;
      case SSL_DEBUG_TRACE:   log.trace("%s", message); break;
      default: log.trace("%s", message); break;
   }
}

//********************************************************************************************************************
// Setup SSL context for Windows

static ERR tls_setup(extNetSocket *Self)
{
   kt::Log log(__FUNCTION__);

   if (Self->TLS.Handle) return ERR::Okay;

   log.traceBranch("Setting up SSL context.");

   if (GetResource(RES::LOG_LEVEL) >= 5) ssl_enable_logging();

   bool validate_cert = (Self->Flags & NSF::DISABLE_SERVER_VERIFY) != NSF::NIL ? false : true;
   bool server_mode = ((Self->Flags & NSF::SERVER) != NSF::NIL);
   if (Self->TLS.Handle = ssl_create_context(validate_cert, server_mode); !Self->TLS.Handle) {
      return ERR::Failed;
   }

   // Load custom certificate if specified for server mode
   if (server_mode) {
      if (Self->SSLCertificate) {
         log.msg("Loading custom SSL server certificate: %s", Self->SSLCertificate);

         ssl_certificate_paths paths;
         if (auto error = resolve_ssl_certificate_paths(Self, paths); error != ERR::Okay) {
            ssl_free_context(Self->TLS.Handle);
            Self->TLS.Handle = nullptr;
            return log.warning(error);
         }

         auto error = ssl_load_server_certificate(Self->TLS.Handle, paths.Certificate, paths.PrivateKey,
            paths.Password);
         if (error != SSL_OK) {
            ssl_free_context(Self->TLS.Handle);
            Self->TLS.Handle = nullptr;
            return log.warning(ERR::Failed);
         }
      }
      else {
         if (!load_pkcs12_certificate(Self->TLS.Handle, glCertPath + "localhost.p12")) {
            if (!load_pem_certificate(Self->TLS.Handle, glCertPath + "localhost.pem")) {
               ssl_free_context(Self->TLS.Handle);
               Self->TLS.Handle = nullptr;
               return log.warning(ERR::Failed);
            }
         }
      }
   }

   return ERR::Okay;
}

//********************************************************************************************************************
// Setup SSL state for a newly accepted server-side client socket.

static ERR tls_accept_client(extClientSocket *Self, extNetSocket *Server)
{
   kt::Log log(__FUNCTION__);

   Self->TLS.Handle = ssl_create_context(false, true); // No verification, server mode.
   if (Self->TLS.Handle) {
      if (ssl_set_server_certificate(Server->TLS.Handle, Self->TLS.Handle) IS SSL_OK) {
         ssl_set_socket(Self->TLS.Handle, (void *)(size_t)Self->Handle.socket());
         Self->State = NTC::HANDSHAKING;
         return ERR::Okay;
      }

      ssl_free_context(Self->TLS.Handle);
      Self->TLS.Handle = nullptr;
   }

   Self->State = NTC::DISCONNECTED;
   return log.warning(ERR::SystemCall);
}

//********************************************************************************************************************
// Handle SSL handshake data.
//
// Handshaking can return error code 0x80090308 (SEC_E_INVALID_TOKEN), this can mean that Windows received malformed
// SSL handshake data from the server.  Win32 error 87 (ERROR_INVALID_PARAMETER) can be caused by server
// certificate/SSL configuration issues

template <class T> ERR tls_handshake_received(T *Self, const void *Data, int Length)
{
   kt::Log log(__FUNCTION__);

   if (!Self->TLS.Handle or !Data or Length <= 0) return ERR::Args;

   log.traceBranch("Processing SSL handshake data (%d bytes)", Length);

   int handshake_consumed = 0;
   SSL_ERROR_CODE result = ssl_continue_handshake(Self->TLS.Handle, Data, Length, handshake_consumed);

   switch (result) {
      case SSL_OK:
         log.trace("SSL handshake completed successfully.");
         Self->setState(NTC::CONNECTED);
         return ERR::Okay;

      case SSL_NEED_DATA:
         log.trace("SSL handshake continuing, waiting for more data.");
         // Stay in HANDSHAKING state
         return ERR::Okay;

      case SSL_ERROR_WOULD_BLOCK:
         log.trace("SSL handshake would block.");
         return ERR::Okay;

      default:
         log.warning("SSL handshake failed: %d; SecStatus: 0x%08X; WinError: %d", result,
            ssl_last_security_status(Self->TLS.Handle),
            ssl_last_win32_error(Self->TLS.Handle));
         Self->setState(NTC::DISCONNECTED);
         return ERR::Failed;
   }
}

//********************************************************************************************************************
// Connect SSL using Windows wrapper - called on receipt of a NTE_CONNECT message

template <class T> ERR tls_connect(T *Self)
{
   kt::Log log(__FUNCTION__);

   log.traceBranch("Attempting SSL handshake.");

   if (!Self->TLS.Handle) return ERR::FieldNotSet;

   std::string hostname = Self->Address ? Self->Address : "";
   auto result = ssl_connect(Self->TLS.Handle, (void *)(size_t)Self->Handle.socket(), hostname);

   switch (result) {
      case SSL_OK:
         log.trace("SSL server connection successful (handshaking not required).");
         Self->setState(NTC::CONNECTED);
         return ERR::Okay;

      case SSL_NEED_DATA:
         log.trace("SSL handshaking in progress.");
         Self->setState(NTC::HANDSHAKING);
         return ERR::Okay;

      case SSL_ERROR_WOULD_BLOCK:
         Self->setState(NTC::HANDSHAKING);
         return ERR::Okay;

      default:
         log.warning("SSL connection failed with code %d", result);
         log.warning("Security status: 0x%08X, Win32 error: %d",
            ssl_last_security_status(Self->TLS.Handle),
            ssl_last_win32_error(Self->TLS.Handle));
         Self->setState(NTC::DISCONNECTED);
         return ERR::Failed;
   }
}
