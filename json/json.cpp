#include <iostream>
#include <string>
#include "nlohmann/json.hpp"
using std::cout;
using std::string;

//json对象是obejct: []中是字符串, 整体是{  }
void test1()
{
    nlohmann::json json_object;           //创建一个空的json对象
    json_object["key"] = "value";         //json对象是一个object
    string context = json_object.dump();  //dump() : 序列化
    cout << context << "\n";
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
//解析json数组
void test3()
{
    char str[] = "[1,2,3,{\"key\":456}]";
    nlohmann::json json_object = nlohmann::json::parse(str);    
    cout << "json_object[0] = " << json_object[0] << "\n";
    cout << "json_object[1] = " << json_object[1] << "\n";
    cout << "json_object[2] = " << json_object[2] << "\n";
    cout << "json_object[3][\"key\"] = " << json_object[3]["key"] << "\n";
}

//解析json对象
void test4()
{
    string str = "{\"1\":\"h\"}";
    cout << str << "\n";
    nlohmann::json json_object = nlohmann::json::parse(str);
    cout << json_object << "\n";
    
    //迭代器遍历
    for(nlohmann::json::iterator it = json_object.begin(); it != json_object.end(); ++it){
        cout << "key = " << it.key() << ", value = " << it.value() << "\n";
    }
    
    //增强for循环
    for(auto &p: json_object.items()){
        cout << "key = " << p.key() << ", value = " << p.value() << "\n";
    }
}

int main()
{
    /* test1(); */
    /* test2(); */
    /* test3(); */
    test4();
    return 0;
}
