#include <iostream>
#include <string>
#include "nlohmann/json.hpp"
using std::cout;
using std::string;

//json对象: 下标是字符串, 整体是{  }
void test1()
{
    nlohmann::json json_object;           //创建一个空的json对象
    json_object["key"] = "value";         //json对象是一个object
    string context = json_object.dump();  //dump() : 序列化
    cout << context << "\n";
}

//json数组:下标是数字,整体是[  ]
void test2()
{
    //生成一个json数组,支持两种插入:下标插入、push_back插入
    nlohmann::json json_array; 
    
    //json_array[0]是字符串
    json_array[0] = "value";
    //json_array[1]是数字
    json_array[1] = 2204;
    //json_array[2]是数字
    json_array.push_back(1234); //json对象是array,则支持push_back()方法 
    
    //json_array[3]是json对象: object {  }
    json_array[3]["key1"] = "value1";
    
    //json_array[4]是json数组: array [  ]
    nlohmann::json child_object;
    child_object[0] = "000";
    child_object.push_back("111");
    child_object.push_back("222");
    child_object.push_back("333");
    json_array.push_back(child_object); //json_array[4]

    string msg = json_array.dump();
    cout << msg << "\n";
}

//解析json对象
void test3()
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

//解析json数组 : nlohmann::json::parse 
void test4()
{
    char str[] = "[1,2,3,{\"key\":456}]";
    nlohmann::json json_array = nlohmann::json::parse(str);    
    cout << "json_object[0] = " << json_array[0] << "\n";
    cout << "json_object[1] = " << json_array[1] << "\n";
    cout << "json_object[2] = " << json_array[2] << "\n";
    cout << "json_object[3][\"key\"] = " << json_array[3]["key"] << "\n";
}

int main()
{
    /* test1(); */
    test2();
    /* test3(); */
    /* test4(); */
    return 0;
}
