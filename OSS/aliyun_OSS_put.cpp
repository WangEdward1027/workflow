//上传文件：使用cpp的SDK,上传文件到阿里云OSS网盘
//编译指令: g++ aliyun_OSS_put.cpp -fno-rtti -lalibabacloud-oss-cpp-sdk -lcurl -lcrypto -lpthread

#include <alibabacloud/oss/OssClient.h>
#include <string>
#include <iostream>
#include <alibabacloud/oss/OssClient.h>
using std::string;
using std::cerr;
using namespace AlibabaCloud::OSS;

struct OSSInfo{
    string AccessKeyID = "LTAI5tEcymYtk27icLnq3T";    //账号，我去的了最后两位字母数字
    string AccessKeySecret = "Q3SfWjJ6blME8U4TIL1iztDFOtZ8";  //密码,我去掉了最后2位字母
    string EndPoint = "oss-cn-wuhan-lr.aliyuncs.com";   //OSS网址
    string Bucket = "58edward";   //网盘名
};

int main(void){
    // 初始化 账户信息、OSS的网址
    OSSInfo ossInfo;
    
    // 申请网络操作的内存
    InitializeSdk();

    // 创建一个OssClient对象
    ClientConfiguration conf;
    OssClient client(ossInfo.EndPoint, ossInfo.AccessKeyID, ossInfo.AccessKeySecret, conf);// conf:客户端属性

    //上传文件: 调用OssClient对象的PutObject 方法 
    // Bucket 网盘的名字
    // key "tmp/test.txt" 在OSS的路径
    // file "file1.txt" 在本地的路径
    PutObjectOutcome outcome = client.PutObject(ossInfo.Bucket, "tmp/test.txt", "file1.txt");
    
    // 检查报错
    if(outcome.isSuccess()){
        cerr << "Success!\n";
    }else{
        cerr << "Fail, message = " << outcome.error().Message() << "\n";
    }
    
    // 释放内存
    ShutdownSdk();
    return 0;
}
