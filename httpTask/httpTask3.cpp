#include <workflow/WFFacilities.h>  // WaitGroup 让某个线程处于等待状态
#include <workflow/WFTaskFactory.h> // WFTaskFactory.h文件里面包含了所有创建任务相关的函数
#include <workflow/HttpUtil.h>      // HttpUtil.h 文件包含了处理HTTP的工具
#include <iostream>
#include <string>
#include <signal.h>
using std::cout;
using std::cerr;
using std::string;

static WFFacilities::WaitGroup waitGroup(1);

void sigHandler(int signum){
    cout << "done!\n";
    waitGroup.done();
}

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

    // 请求的信息
    protocol::HttpRequest * req = httpTask->get_req();  //获取请求
    cout << "http request method = " << req->get_method() << "\n";         //获取请求的方法
    cout << "http request uri = " << req->get_request_uri() << "\n";       //获取path和query
    cout << "http request version = " << req->get_http_version() << "\n";  //获取http版本
    
    // 遍历首部字段
    protocol::HttpHeaderCursor reqCursor(req); //是一个访问所有首部字段的迭代器
    string key,value;
    while(reqCursor.next(key,value)){ //next: 取出下一个首部字段
        cout << "key = " << key << " value = " << value << "\n";
    }
    
    // 响应的信息
    cout << "--------------------------------------------------\n";
    protocol::HttpResponse *resp = httpTask->get_resp();  //获取响应
    cout << "http response version = " << resp->get_http_version() << "\n";  //获取http版本
    cout << "http response status code = " << resp->get_status_code() << "\n"; //获取状态码
    cout << "http response reason phrase = " << resp->get_reason_phrase() << "\n"; //获取原因字符串
    protocol::HttpHeaderCursor respCursor(resp);
    while(respCursor.next(key,value)){
        cout << "key = " << key << " value = " << value << "\n";
    }

    // workflow框架里面 http报文头和报文体不在一起
    const void *body; //不可修改指向的内容，可以修改指向
    size_t size;
    resp->get_parsed_body(&body,&size);  //找到报文体内存的首地址
    cout << static_cast<const char *>(body) << "\n";
}

int main(){
    signal(SIGINT,sigHandler);

    WFHttpTask * httpTask = WFTaskFactory::create_http_task( // 创建任务
        "http://www.baidu.com", // url
        10, // 重定向次数上限
        10, // 重试次数
        //nullptr); //回调函数
        httpCallback); //新的回调函数
    // 在启动任务之前，设置任务的属性
    // 只能改请求，不能改响应
    protocol::HttpRequest * req = httpTask->get_req();
    req->add_header_pair("key1","value1");
    
    httpTask->start();  // 启动任务
    
    waitGroup.wait();
    cout << "finished!\n";
    return 0;
}
