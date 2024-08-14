//workflow的核心机制二:并行任务 paralelWork
//三个httpTask合为一个并行任务,并行任务的大序列后接一个redisTask

#include <signal.h>
#include <iostream>
#include <string>
#include <vector>
#include <workflow/WFFacilities.h>  // WaitGroup 让某个线程处于等待状态
#include <workflow/WFTaskFactory.h> // WFTaskFactory.h文件里面包含了所有创建任务相关的函数
#include <workflow/Workflow.h>
using std::cout;
using std::cerr;
using std::string;
using std::vector;

struct SeriesContext{
    string url;
    size_t size;
};

static WFFacilities::WaitGroup waitGroup(1);

void sigHandler(int signum){
    cout << "done!\n";
    waitGroup.done();
}

//在回调中,已经获取了报文体的长度
void httpCallback(WFHttpTask *httpTask){
    // 检查报错
    int state = httpTask->get_state(); //获取状态
	int error = httpTask->get_error(); //获取errno

    switch (state){
	case WFT_STATE_SYS_ERROR: //系统错误
		cerr << "system error: " << strerror(error) << "\n";
		break;
	case WFT_STATE_DNS_ERROR: //DNS错误
		cerr << "DNS error: " << gai_strerror(error) << "\n";
		break;
	case WFT_STATE_SSL_ERROR: //SSL错误
		cerr <<"SSL error: " << error << "\n";
		break;
	case WFT_STATE_TASK_ERROR: //任务错误
		cerr <<"Task error: " << error << "\n";
		break;
	case WFT_STATE_SUCCESS:   //成功
		break;
	}
	if (state != WFT_STATE_SUCCESS){
		cerr << "Failed. Press Ctrl-C to exit.\n";
		return;
	}
     
    protocol::HttpResponse *resp = httpTask->get_resp();  //获取响应

    // workflow框架里面 http报文头和报文体不在一起
    const void *body; //不可修改指向的内容，可以修改指向
    size_t size;
    resp->get_parsed_body(&body,&size);  //找到报文体内存的首地址
    
    SeriesContext *context = static_cast<SeriesContext *>(series_of(httpTask)->get_context());
    context->size = size;
    cout << "I am callback url = " << context->url << "size = " << size << "\n";
}

void parallelCallback(const ParallelWork *parallel) //回调中不能修改序列的信息,故使用const
{  //可以查询序列的信息,如序列的context
    string url;
    size_t size = 0;
    for(int i = 0; i < parallel->size(); ++i){
        const SeriesWork *series = parallel->series_at(i);
        SeriesContext *context = static_cast<SeriesContext *>(series->get_context());
        if(size < context->size){
            size = context->size;
            url = context->url;
        }
        delete context;
    }

    cout << "biggest url = " << url << " size = " << size << "\n";

    //找到并行任务所在的大序列,往后面添加一个redis任务
    WFRedisTask * redisTask = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 10, nullptr);
    redisTask->get_req()->set_request("SET", {url, std::to_string(size)});
    series_of(parallel)->push_back(redisTask); //redisTask插在并行任务之后
}

int main(){
    signal(SIGINT,sigHandler);

    //创建一个空的并行任务
    ParallelWork *parallel = Workflow::create_parallel_work(parallelCallback);
    //先创建3个http任务,再根据任务创建小序列,再将小序列加入到并行任务中
    vector<string> urls ={
        "http://www.taobao.com",
        "http://www.jd.com",
        "http://www.baidu.com"
    };

    for(int i = 0; i < 3; ++i){
        //创建一个任务
        WFHttpTask *httpTask = WFTaskFactory::create_http_task(urls[i], 10, 10, httpCallback);
        SeriesWork *series = Workflow::create_series_work(httpTask, nullptr);

        //在序列创建好后申请context
        SeriesContext *context = new SeriesContext;
        context->url = urls[i];
        series->set_context(context);
        
        //将小序列加入到并行任务中
        parallel->add_series(series);
    }
    
    //先启动并行任务
    parallel->start();

    waitGroup.wait();
    cout << "finished!\n";
    return 0;
}
