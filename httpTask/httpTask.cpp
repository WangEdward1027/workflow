#include <workflow/WFFacilities.h>
#include <workflow/WFTaskFactory.h>
#include <signal.h>
#include <iostream>
using std::cout;

static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum)
{
    cout << "done!\n";
    waitGroup.done();
}

int main()
{
    signal(SIGINT, sighandler);
    
    WFHttpTask *httpTask = WFTaskFactory::create_http_task(
            "http://www.baidu.com", 
            10, 10, nullptr); //给任务对象申请内存并初始化
    protocol::HttpRequest *req = httpTask->get_req();  //找到请求的地址
    req->add_header_pair("Agent", "Workflow"); //给请求添加一个首部字段
    req->add_header_pair("name", "Edward"); //给请求添加一个首部字段
    httpTask->start(); //异步启动任务,把任务交给框架,由框架调度资源运行
    
    waitGroup.wait();
    cout << "finished!\n";
    return 0;
}
