//序列series: 先执行第一个任务,第二个任务不会抢跑,需要等待第一个任务完成
//第一个任务插入数据,第二个任务将插入的数据读出

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <workflow/WFFacilities.h>  // WaitGroup 让某个线程处于等待状态
#include <workflow/WFTaskFactory.h> // WFTaskFactory.h文件里面包含了所有创建任务相关的函数
#include <workflow/Workflow.h>
using std::cout;
using std::cerr;

static WFFacilities::WaitGroup waitGroup(1);

void sigHandler(int signum){
    cout << "done!\n";
    waitGroup.done();
}

void redisCallback1(WFRedisTask *redisTask){
    cout << "I am callback1\n";
    sleep(3);

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
        cerr << "Failed. Press Crtl+C to exit.\n";
        return;
    }

    //展示读到的结果
    if(result.is_string()){
        cout << "result is " << result.string_value() << "\n";
    }
    else if(result.is_array()){
        cout << "result is array\n";
        for(size_t i = 0; i < result.arr_size(); ++i){
            cout << "i = " << i << " value = " << result.arr_at(i).string_value() << "\n";
        }
    }
}

void redisCallback2(WFRedisTask *redisTask){
    cout << "I am callback2\n";

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
        cerr << "Failed. Press Crtl+C to exit.\n";
        return;
    }

    //展示读到的结果
    if(result.is_string()){
        cout << "result is " << result.string_value() << "\n";
    }
    else if(result.is_array()){
        cout << "result is array\n";
        for(size_t i = 0; i < result.arr_size(); ++i){
            cout << "i = " << i << " value = " << result.arr_at(i).string_value() << "\n";
        }
    }
}

int main(){
    signal(SIGINT,sigHandler);

    WFRedisTask *redisTask1 = WFTaskFactory::create_redis_task( // 创建任务
        "redis://127.0.0.1:6379", // url
        10, 
        redisCallback1); 
    protocol::RedisRequest *req1 = redisTask1->get_req();
    req1->set_request("SET", {"Ed", "ward"});
   
    WFRedisTask *redisTask2 = WFTaskFactory::create_redis_task( // 创建任务
        "redis://127.0.0.1:6379", // url
        10, 
        redisCallback2); 
    protocol::RedisRequest *req2 = redisTask2->get_req();
    req2->set_request("GET", {"Ed"});
    
    //创建一个序列
    SeriesWork *series = Workflow::create_series_work(redisTask1, nullptr);
    //往序列的末尾添加一个任务
    series->push_back(redisTask2);
    //将序列交给框架调度
    series->start();
    
    waitGroup.wait();
    cout << "finished!\n";
    return 0;
}
