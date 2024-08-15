//json对象：object 或 array
//dump()用来序列化为字符串， parse()用来将字符串反序列化json对象

#include <iostream>
#include "nlohmann/json.hpp"
using std::cout;

//json对象是obejct: []中是字符串, 整体是{  }
void test1()
{
    nlohmann::json json_object;  //创建一个空的json对象
    json_object["key"] = "value";  //json对象是一个object
    cout << json_object.dump() << "\n";  //dump() : 序列化
}

//json对象是array: []中是数字, 整体是 [  ]
void test2()
{
    nlohmann::json json_object; 

    json_object[0] = "value";
    json_object[1] = 2204;
    json_object.push_back(1234); //json对象是array,则支持push_back()方法 
    
    //加入一个object {  }
    json_object[3]["key1"] = "value1";
    
    //加入一个array [  ]
    nlohmann::json child_object;
    child_object[0] = "000";
    child_object.push_back("111");
    child_object.push_back("222");
    child_object.push_back("333");
    json_object.push_back(child_object);

    cout << json_object.dump() << "\n";
}

//nlohmann::json::parse 解析字符串
void test3()
{
    char str[] = "[1,2,3,{\"key\":456}]";
    nlohmann::json json_object = nlohmann::json::parse(str);    
    cout << "json_object[0] = " << json_object[0] << "\n";
    cout << "json_object[1] = " << json_object[1] << "\n";
    cout << "json_object[2] = " << json_object[2] << "\n";
    cout << "json_object[3][\"key\"] = " << json_object[3]["key"] << "\n";
}

int main()
{
    /* test1(); */
    /* test2(); */
    test3();
    return 0;
}
