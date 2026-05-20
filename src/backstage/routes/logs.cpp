//********************************************************************************************************************
// @doc PUT /logs/level Change the logging level for the program (affects console output).

static bool parse_log_level(std::string_view Value, int &Level)
{
   if (Value.empty()) return false;

   auto result = std::from_chars(Value.data(), Value.data() + Value.size(), Level);
   if ((not (result.ec IS std::errc())) or (result.ptr != Value.data() + Value.size())) return false;

   return (Level >= 0) and (Level <= 9);
}

//********************************************************************************************************************

static ERR put_logs_level(const BackstageRequest &Request, BackstageResponse &Response)
{
   auto level_param = Request.queryParams.find("level");
   int level;

   if ((level_param IS Request.queryParams.end()) or (not parse_log_level(level_param->second, level))) {
      Response.status = 400;
      Response.body = "{\"error\":\"invalid_level\",\"message\":\"Expected level query parameter from 0 to 9\"}";
      return ERR::Okay;
   }

   SetResource(RES::LOG_LEVEL, level);

   Response.status = 200;
   Response.contentType = "application/json";
   Response.body = "{\"level\":";
   Response.body.append(std::to_string(level));
   Response.body.push_back('}');
   return ERR::Okay;
}

//********************************************************************************************************************
// @doc POST /logs/start Activates internal log recording.

static ERR post_logs_start(const BackstageRequest &Request, BackstageResponse &Response)
{
   return ERR::NoSupport;
}

//********************************************************************************************************************
// @doc POST /logs/stop Stops internal log recording.

static ERR post_logs_stop(const BackstageRequest &Request, BackstageResponse &Response)
{
   return ERR::NoSupport;
}

//********************************************************************************************************************
// @doc GET /logs Returns all logged messages, then clears the log message stack (the client is responsible for maintaining a permanent record).

static ERR get_logs(const BackstageRequest &Request, BackstageResponse &Response)
{
   return ERR::NoSupport;
}
