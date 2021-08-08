#include <chrono>
#include <cstdio>
#include "httplib.h"
#include "include/configor/json.hpp"
#include "job.h"

using namespace configor;
using namespace httplib;

#define HTTP_MAX_ALIVE_COUNT 100
#define SERVER_LISTEN_PORT 8080

std::string dump_headers(const Headers &headers) {
  std::string s;
  char buf[BUFSIZ];

  for (auto it = headers.begin(); it != headers.end(); ++it) {
    const auto &x = *it;
    snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
    s += buf;
  }

  return s;
}

std::string log(const Request &req, const Response &res) {
  std::string s;
  char buf[BUFSIZ];

  s += "================================\n";

  snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
           req.version.c_str(), req.path.c_str());
  s += buf;

  std::string query;
  for (auto it = req.params.begin(); it != req.params.end(); ++it) {
    const auto &x = *it;
    snprintf(buf, sizeof(buf), "%c%s=%s",
             (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
             x.second.c_str());
    query += buf;
  }
  snprintf(buf, sizeof(buf), "%s\n", query.c_str());
  s += buf;

  s += dump_headers(req.headers);

  s += "--------------------------------\n";

  snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
  s += buf;
  s += dump_headers(res.headers);
  s += "\n";

  if (!res.body.empty()) { s += res.body; }

  s += "\n";

  return s;
}

int main(void) {
  Server svr;

  if (!svr.is_valid()) {
    printf("server has an error...\n");
    return -1;
  }

  svr.set_keep_alive_max_count(HTTP_MAX_ALIVE_COUNT);
  svr.new_task_queue = [] { return new ThreadPool(100); };

  svr.Get("/", [=](const Request & /*req*/, Response &res) {
    res.set_redirect("/hi");
  });

  svr.Get("/hi", [](const Request & /*req*/, Response &res) {
    res.set_content("Hello World~\n", "text/plain");
  });

  svr.Get("/dump", [](const Request &req, Response &res) {
    res.set_content(dump_headers(req.headers), "text/plain");
  });



  svr.Post("/config", [](const Request &req, Response &res) {
    try
    {
      JobReq jobreq;
      std::istringstream iss(req.body);
      iss >> json::wrap(jobreq);
      res.set_content(AddJob(jobreq), "text/plain");
    }
    catch(const std::exception& e)
    {
      res.status = 408;
    }
    
    
  });

  svr.Post("/config/del", [](const Request &req, Response &res) {
    try
    {
      JobDelReq job;
      std::istringstream iss(req.body);
      iss >> json::wrap(job);
      DelJob(job.id);
    }
    catch(const std::exception& e)
    {
      res.status = 408;
    }
    
  
  });

  svr.Get("/config", [](const Request &req, Response &res) {
    std::ostringstream ret;    
    ret << json::wrap(AllJob());
    
    res.set_content(ret.str(), "application/json");
  });

  svr.Get("/stop",
          [&](const Request & /*req*/, Response & /*res*/) { svr.stop(); });

  svr.set_error_handler([](const Request & /*req*/, Response &res) {
    const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
    char buf[BUFSIZ];
    snprintf(buf, sizeof(buf), fmt, res.status);
    res.set_content(buf, "text/html");
  });

  svr.set_logger([](const Request &req, const Response &res) {
    printf("%s", log(req, res).c_str());
  });
  
  startJobSchuler();

  svr.listen("0.0.0.0", SERVER_LISTEN_PORT);

  return 0;
}
