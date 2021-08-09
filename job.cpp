#include <mutex>
#include <vector>
#include <random>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>

#include "httplib.h"
#include "job.h"
#include "ThreadPool.h"

using std::mutex; 
using std::vector;
#define CPPHTTPLIB_OPENSSL_SUPPORT

std::string GetUUID() {
     static std::random_device rd;
     static std::uniform_int_distribution<uint64_t> dist(0ULL, 0xFFFFFFFFFFFFFFFFULL);
     uint64_t ab = dist(rd);
     uint64_t cd = dist(rd);
     uint32_t a, b, c, d;
     std::stringstream ss;
     ab = ( ab & 0xFFFFFFFFFFFF0FFFULL ) | 0x0000000000004000ULL;
     cd = ( cd & 0x3FFFFFFFFFFFFFFFULL ) | 0x8000000000000000ULL;
     a  = ( ab >> 32U );
     b  = ( ab & 0xFFFFFFFFU);
     c  = ( cd >> 32U );
     d  = ( cd & 0xFFFFFFFFU);
     ss << std::hex << std::nouppercase << std::setfill('0');
     ss << std::setw(8) << (a) ;//<< '-';
     ss << std::setw(4) << (b >> 16U) ;//<< '-';
     ss << std::setw(4) << (b & 0xFFFFU);// << '-';
     ss << std::setw(4) << (c >> 16U) ;//<< '-';
     ss << std::setw(4) << (c & 0xFFFFU);
     ss << std::setw(8) << d;

    return ss.str();
}



#define DeleteRecord_C "./record/job.log" 
std::mutex logMutex;

ThreadPool job_thread_pool(100);

void writeLog(string log)  
{    
    printf("doJob  %s",log.c_str());
    logMutex.lock();
    try
    {
        if (access("./record", 0) != F_OK) {
            mkdir("./record", 0755);
        }
        std::fstream f;
        f.open(DeleteRecord_C, std::ios::out | std::ios::app);
        f << log << std::endl;
        f.close();
    }
    catch(const std::exception& e)
    {
        
    }    

    logMutex.unlock();
	   
} 

std::mutex mtx; 
Jobs jobs;
vector<string> prepared_jobs;

std::shared_ptr<Jobs> AllJob(){
    mtx.lock();
    std::shared_ptr<Jobs> p = std::make_shared<Jobs>();
    *p = jobs;

    mtx.unlock();

    return p;
}
string AddJob(JobReq jobreq){
    string id = GetUUID();
    mtx.lock();
    Job job;
    job.id = id;
    job.time = jobreq.time;
    job.scheme = jobreq.scheme;
    job.host = jobreq.host;
    job.url = jobreq.url;
    job.headers = jobreq.headers;
    job.mothed = jobreq.mothed;
    job.contentType = jobreq.contentType;
    job.body = jobreq.body;
    job.port = jobreq.port;
    
    jobs.jobs.push_back(job);

    mtx.unlock();

    return id;

}
void DelJob(string id){
    mtx.lock();
    for (std::vector<Job>::iterator it = jobs.jobs.begin(); it != jobs.jobs.end(); it++)
    {
        if(it->id == id){
            jobs.jobs.erase(it);
            break;
        }
    }
    mtx.unlock();
}

void* doJob(void* p){
    Job* pJob = (Job*)p;

    httplib::Client cli(pJob->host, pJob->port);
    httplib::SSLClient ssl_cli(pJob->host, pJob->port);

    if (pJob->scheme.compare("http") == 0)
    {
        cli.init_socket();
        cli.set_close_socket_immediate(false);
        cli.set_follow_location(true);
    }
    else if(pJob->scheme.compare("https") == 0)
    {
        ssl_cli.init_socket();
        ssl_cli.set_close_socket_immediate(false);
        ssl_cli.set_follow_location(true);
        ssl_cli.enable_server_certificate_verification(false);
    }
    else 
    {
        return 0;
    }


    while(true)
    {
        int64_t _timems = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (pJob->time > _timems)
        {
            std::this_thread::yield();
            continue;
        }
	
	    std::ostringstream logstr;    
	    logstr <<"current time:" << _timems << " start job:" << json::wrap(*pJob) << std::endl;
	
        try
        {	    
            if(pJob->scheme.compare("http") == 0)
            {
                if(pJob->mothed.compare("GET") == 0 ){            
                    auto res = cli.Get(pJob->url.c_str());
                    if (res) 
                    { 
                        logstr << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << " return " << res->status << std::endl;
                        logstr << res->get_header_value("Content-Type") << std::endl;
                        logstr << res->body << std::endl;

                    } 
                } 
                else if (pJob->mothed.compare("POST") == 0 )
                {
                    auto res = cli.Post(pJob->url.c_str(),pJob->body, pJob->contentType.c_str());
                    if (res) 
                    {
                        logstr << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << " return " << res->status << std::endl;
                        logstr << res->get_header_value("Content-Type") << std::endl;
                        logstr << res->body << std::endl;
                    } 
                }
            
            } 
            else if (pJob->scheme.compare("https") == 0)
            {
                if(pJob->mothed.compare("GET") == 0 ){   
                    auto res = ssl_cli.Get(pJob->url.c_str());
                    if (res) 
                    { 
                        logstr << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << " return " << res->status << std::endl;
                        logstr << res->get_header_value("Content-Type") << std::endl;
                        logstr << res->body << std::endl;
                    } 
                } 
                else if (pJob->mothed.compare("POST") == 0 )
                {
                    auto res = ssl_cli.Post(pJob->url.c_str(),pJob->body, pJob->contentType.c_str());
                    if (res) {
                    
                        logstr << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << " return " << res->status << std::endl;
                        logstr << res->get_header_value("Content-Type") << std::endl;
                        logstr << res->body << std::endl;

                    } 
                }
            }
            delete pJob;
        }
        catch(...){
            printf("doJob error\n");
        }

        usleep(5000);
        writeLog(logstr.str());
        if (pJob->scheme.compare("http") == 0)
        {
            cli.clear_socket();
        }
        else if (pJob->scheme.compare("https") == 0)
        {
            ssl_cli.clear_socket();
        }
        break;
    }
    
    return 0;
}

void *threadJobSchuler(void* n)
{
    while (true)
    {
	    try
        {
            std::shared_ptr<Jobs> pjobs = AllJob();
            int64_t timems = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            int64_t needSleep = 50000;
            for (std::vector<Job>::iterator it = (*pjobs).jobs.begin(); it != (*pjobs).jobs.end(); it++)
            {           
                if(it->time <= timems + 3000)
                {
                    Job* pJob = new Job();
                    *pJob = *it;
                    job_thread_pool.enqueue([](void* job) {doJob(job); }, (void*)pJob);
                    prepared_jobs.emplace_back(it->id);
                }
                else
                {
	                needSleep = it->time - timems > needSleep ? it->time - timems : needSleep;
                }
            }

            if (prepared_jobs.size() != 0)
            {
                for(auto& job_id :prepared_jobs)
                    DelJob(job_id);
                prepared_jobs.clear();
            }

            if(needSleep > 5000)
	            needSleep = 500;
            else
	            needSleep = 0;

            usleep(needSleep);
	    }
	    catch(...)
        {
	    }
    }
    
    return 0;
}

void startJobSchuler() 
{
    printf("JobSchuler start");

    pthread_t id;
    pthread_create(&id, NULL, threadJobSchuler, NULL);
}

