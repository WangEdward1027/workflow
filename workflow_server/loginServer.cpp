#include <signal.h>
#include <iostream> 
#include <string>
#include <workflow/WFFacilities.h>
#include <workflow/WFHttpServer.h>
using std::cout;
using std::endl;
using std::string;

struct SeriesContext
{
    string name;
    string password;
    protocol::HttpResponse *resp;
};

static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum)
{
    cout << "done!\n";
    waitGroup.done();
}

void redisCallback(WFRedisTask *redisTask)
{
    protocol::RedisResponse *respRedis = redisTask->get_resp();
    protocol::RedisValue result;
    respRedis->get_result(result);

    SeriesContext *context = static_cast<SeriesContext*>(series_of(redisTask)->get_context());
    
    if(result.is_string() && result.string_value() == context->password){
        context->resp->append_output_body("login Success");
    }
    else{
        context->resp->append_output_body("login Fail");
    }
}

void process(WFHttpTask *serverTask)
{
    //serverTask是一个特殊的任务：基本工作是用户代码process，基本工作和callback是分离的
    //http://localhost:12345/name=Edward&password=2204
    protocol::HttpRequest *req = serverTask->get_req();     //请求
    protocol::HttpResponse *resp = serverTask->get_resp();  //响应
    
    string uri = req->get_request_uri();
    cout << "uri = " << uri << "\n";
    string nameKV = uri.substr(0, uri.find("&"));
    string passwordKV = uri.substr(uri.find("&")+1);
    string name = nameKV.substr(nameKV.find("=")+1);
    string password = passwordKV.substr(passwordKV.find("=")+1);
    cout << "name = " << name << ", password = " << password << "\n";

    SeriesContext *context = new SeriesContext;
    context->name = name;
    context->password = password;
    context->resp = resp;  //保存http响应报文
    series_of(serverTask)->set_context(context);
    
    //创建一个redis任务,进行redis查询
    WFRedisTask *redisTask = WFTaskFactory::create_redis_task(
                "redis://127.0.0.1:6379", 10, redisCallback);
    redisTask->get_req()->set_request("HGET", {"user", name});

    series_of(serverTask)->push_back(redisTask);
}

int main()
{
    signal(SIGINT, sighandler);
    WFHttpServer server(process);
    if(server.start(12345) == 0){
        waitGroup.wait();
        cout << "finished!\n";
        server.stop();
        return 0;
    }else{
        perror("server start failed.");
        return -1;
    }   
}
