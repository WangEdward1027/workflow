//httpTask + redisTask
//读取某个网站的内容，并且存入redis服务端当中
//(比如先访问淘宝，再set www.taobao.com 淘宝的html内容)

#include <signal.h>
#include <iostream>
#include <workflow/WFFacilities.h>
#include <workflow/WFTaskFactory.h>
using std::cout;
using std::cerr;

static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum){
    cout << "done!\n"; // 尽量避免使用endl
    waitGroup.done();
}

void redisCallback(WFRedisTask *redisTask){
    // 检查报错
    int state = redisTask->get_state(); //获取状态
	int error = redisTask->get_error(); //获取errno
	switch (state){
	case WFT_STATE_SYS_ERROR: //操作系统层面的错误
		cerr << "system error: " << strerror(error) << "\n";
		break;
	case WFT_STATE_DNS_ERROR: //网络信息错误
		cerr << "DNS error: " << gai_strerror(error) << "\n";
		break;
	case WFT_STATE_SUCCESS:
		break;
	}

    // 先获取redis的结果
    protocol::RedisResponse *resp = redisTask->get_resp();
    protocol::RedisValue result; //默认构造一个result对象，将来可以存储redis指令的执行结果
    resp->get_result(result);//获取指令执行的结果
    // 指令是否执行成功
    if(result.is_error()){
        cerr << "Redis error\n";
        state = WFT_STATE_TASK_ERROR;
    }
	if (state != WFT_STATE_SUCCESS){
		cerr << "Failed. Press Ctrl-C to exit.\n";
		return;
	}

    //展示读到的结果
    if(result.is_string()){
        cout << "result is " << result.string_value() << "\n";
    }
    else if(result.is_array()){
        cout << "result is array\n";
        for(int i = 0; i < result.arr_size(); ++i){
            cout << "i = " << i << " value =  " << result.arr_at(i).string_value() << "\n";
        }
    }
}

void httpCallback(WFHttpTask *httpTask){
    // 检查报错
    int state = httpTask->get_state(); //获取状态
	int error = httpTask->get_error(); //获取errno
	switch (state){
	case WFT_STATE_SYS_ERROR:
		cerr << "system error: " << strerror(error) << "\n";
		break;
	case WFT_STATE_DNS_ERROR:
		cerr << "DNS error: " << gai_strerror(error) << "\n";
		break;
	case WFT_STATE_SSL_ERROR:
		cerr <<"SSL error: " << error << "\n";
		break;
	case WFT_STATE_TASK_ERROR:
		cerr <<"Task error: " << error << "\n";
		break;
	case WFT_STATE_SUCCESS:
		break;
	}
	if (state != WFT_STATE_SUCCESS){
		cerr << "Failed. Press Ctrl-C to exit.\n";
		return;
	}

    // 响应的信息
    protocol::HttpResponse *resp = httpTask->get_resp();//get_resp 获取响应
    
    // workflow框架里面 http报文头和报文体不在一起
    const void *body; //不可修改指向的内容，可以修改指向
    size_t size;
    resp->get_parsed_body(&body,&size);//get_parsed_body 找到报文体内存的首地址
    WFRedisTask * redisTask = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379",10,redisCallback);
    redisTask->get_req()->set_request("SET",{"www.taobao.com",static_cast<const char *>(body)});
    series_of(httpTask)->push_back(redisTask);
}

int main(){
    signal(SIGINT,sighandler);
    WFHttpTask * httpTask = WFTaskFactory::create_http_task( // 创建任务
        "http://www.taobao.com", // url
        10, // 重定向次数上限
        10, // 重试次数
        httpCallback //新的回调函数
        );
    // 启动任务
    httpTask->start();
    waitGroup.wait();
    cout << "finished!\n";
}
