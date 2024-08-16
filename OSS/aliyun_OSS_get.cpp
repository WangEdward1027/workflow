//下载文件:生成下载链接
//从阿里云网盘下载文件

#include <string>
#include <iostream>
#include <alibabacloud/oss/OssClient.h>
using std::string;
using std::cout;
using std::cerr;
using namespace AlibabaCloud::OSS;

struct OSSInfo{
    string AccessKeyID = "LTAI5tEcymYtk27icLnq3T";   //账号, 删除最后两位
    string AccessKeySecret = "Q3SfWjJ6blME8U4TIL1iztDFOtZ8";  //密码,删除了最后两位
    string EndPoint = "oss-cn-wuhan-lr.aliyuncs.com";  //OSS网盘网址
    string Bucket = "58edward";   //网盘名
};

int main(void){
    // 初始化 账户信息、OSS的网址
    OSSInfo ossInfo;

    // 申请网络操作的内存
    InitializeSdk();
    
    // 创建一个OssClient对象
    ClientConfiguration conf;
    
    // EndPoint 是OSSDE网址
    // AccessKey ID Secret 用户 密码
    // conf 客户端属性
    OssClient client(ossInfo.EndPoint,ossInfo.AccessKeyID,ossInfo.AccessKeySecret,conf);
    
    //设置过期时间
    time_t now = time(nullptr);
    time_t expires = now + 600;
    StringOutcome outcome = client.GeneratePresignedUrl(ossInfo.Bucket,"tmp/test.txt",expires);
    
    if(outcome.isSuccess()){
        cout << "url = " << outcome.result() << "\n";
    }
    else{
        cerr << "fail, message = " << outcome.error().Message() << "\n";
    }

    // 释放内存
    ShutdownSdk();
    
    return 0;
}
