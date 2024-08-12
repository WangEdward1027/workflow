#include <workflow/WFFacilities.h>
#include <signal.h>
#include <iostream>
using std::cout;

static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum)
{
    cout << "signum == " << signum << "\n";
    cout << "done!\n";    //尽量避免使用endl,效率太低
    waitGroup.done();
}

int main()
{
    signal(SIGINT, sighandler);  //信号处理函数的注册
    waitGroup.wait();            //阻塞等待
    cout << "finished!\n";    
}
