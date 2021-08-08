#include "include/configor/json.hpp"
#include <memory>

using namespace configor;
using std::string;
using std::vector;
using std::map;


struct Job
{
    string id;
    int64_t time;
    string scheme;
    string host;
    string mothed;
    string contentType;
    string body;
    int port;
    string url;
    map<string,string> headers;

    JSON_BIND(Job, id, time, scheme, host, mothed, contentType, body, port, url, headers);
};

struct JobReq
{
    int64_t time;
    string scheme;
    string host;
    string mothed;
    string contentType;
    string body;
    int port;
    string url;
    map<string,string> headers;

    JSON_BIND(JobReq, time, scheme, host, mothed, contentType, body, port, url, headers);
};


struct JobDelReq
{
    string id;

    JSON_BIND(JobDelReq, id);
};

struct Jobs
{
    vector<Job> jobs;

    JSON_BIND(Jobs, jobs);
};

extern std::shared_ptr<Jobs> AllJob();
extern string AddJob(JobReq job);
extern void DelJob(string id);
extern void startJobSchuler();
