//泥鳅代码:workflow写登录服务器

#include <signal.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFHttpServer.h>
using std::cout;
using std::cerr;
using std::string;

static WFFacilities::WaitGroup waitGroup(1);

struct SeriesContext{
    string name;
    string password;
    protocol::HttpResponse *resp;
};

void sighandler(int signum){
    cout << "done!\n";
    waitGroup.done();
}

void redisCallback(WFRedisTask *redisTask){
    protocol::RedisResponse * respRedis = redisTask->get_resp();
    protocol::RedisValue result;
    respRedis->get_result(result);
    // 找到context 提取用户传入的密码
    SeriesContext *context = static_cast<SeriesContext*>( series_of(redisTask)->get_context());

    // result里面存储了数据库的密码
    if(result.is_string() && result.string_value() == context->password){
        // 在redisTask的回调中去修改serverTask的响应
        context->resp->append_output_body("<html>login Success</html>");
    }
    else{
        context->resp->append_output_body("<html>login Fail</html>");
    }
}

void process(WFHttpTask * serverTask){
    // serverTask是一个特殊的任务：基本工作是用户代码process，基本工作和callback是分离的
    protocol::HttpRequest *req = serverTask->get_req();    //req是客户端发来的请求
    protocol::HttpResponse *resp = serverTask->get_resp(); // resp是将要回复给客户端的响应

    string method = req->get_method();
    if(method == "GET"){
        //WFTaskFactory::create_pread_task()
        int fd = open("postform.html",O_RDWR);
        char *buf = new char[615]{0};
        //ssize_t length = read(fd,buf,sizeof(buf));
        ssize_t length = read(fd,buf,615);
        cout << "length = " << length << "\n";
        resp->append_output_body(buf,length);
        delete [] buf;
    }
    else if(method == "POST"){
        const void *body;
        size_t size;
        req->get_parsed_body(&body,&size);
        string bodystr(static_cast<const char *>(body));
        string nameKV = bodystr.substr(0,bodystr.find("&"));
        string passwordKV = bodystr.substr(bodystr.find("&")+1);
        string name = nameKV.substr(nameKV.find("=")+1);
        string password = passwordKV.substr(passwordKV.find("=")+1);
        
        // 申请内存 保存用户名和密码信息 共享给redisTask
        SeriesContext *context = new SeriesContext;
        context->name = name;
        context->password = password;
        context->resp = resp;
        series_of(serverTask)->set_context(context);
        
        //创建一个redis任务，查询
        WFRedisTask * redisTask = WFTaskFactory::create_redis_task(
            "redis://127.0.0.1:6379",10,
            redisCallback
        );
        redisTask->get_req()->set_request("HGET", {"user", name});
        // process是特殊的serverTask的基本工作
        // redisTask会在process执行之后， serverTask的回调之前执行
        series_of(serverTask)->push_back(redisTask);
    }
}

int main(){
    signal(SIGINT,sighandler);
    WFHttpServer server(process);
    if(server.start(12345) == 0){
        // start是非阻塞的
        waitGroup.wait();
        cout << "finished!\n";
        server.stop();
        return 0;
    }
    else{
        perror("server start fail!");
        return -1;
    }
}
