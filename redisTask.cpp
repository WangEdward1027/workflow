//redisTask：写任务，向启动的redis数据库中插入一条数据。不接返回值。

#include <signal.h>
#include <iostream>
#include <string>
#include <workflow/WFFacilities.h>  // WaitGroup 让某个线程处于等待状态
#include <workflow/WFTaskFactory.h> // WFTaskFactory.h文件里面包含了所有创建任务相关的函数
using std::cout;
using std::cerr;
using std::string;

static WFFacilities::WaitGroup waitGroup(1);

void sigHandler(int signum){
    cout << "done!\n";
    waitGroup.done();
}

void redisCallback(WFRedisTask *redisTask){
    // 检查报错
    int state = redisTask->get_state(); //获取状态
	int error = redisTask->get_error(); //获取errno

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

    //先获取redis的结果
    protocol::RedisResponse *resp = redisTask->get_resp();  
    protocol::RedisValue result;  //默认构造一个result对象,将来可以存储redis指令的执行结果
    resp->get_result(result);  //获取指令执行的结果
    //指令是否执行成功
    if(result.is_error()){
        cerr << "Redis error\n";
        state = WFT_STATE_TASK_ERROR;
    }

    if(state != WFT_STATE_SUCCESS){
        cerr << "Failed. Press Crtl-C to exit.\n";
        return;
    }
}

int main(){
    signal(SIGINT,sigHandler);

    WFRedisTask *redisTask = WFTaskFactory::create_redis_task( // 创建任务
        "redis://127.0.0.1:6379", // url
        10, 
        redisCallback); 
    protocol::RedisRequest *req = redisTask->get_req();
    req->set_request("SET", {"58key", "value"});
    redisTask->start();  // 启动任务
    
    waitGroup.wait();
    cout << "finished!\n";
    return 0;
}
