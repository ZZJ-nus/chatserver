#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include<string>
using namespace std;

// json序列化示例1
void func1(){
    json js;//看做是容器
    //要组装的数据格式
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello world,what are you doing now";
    //将js看做容器，  键+值  的形式

    // string sendBuf = js.dump();
    // cout << sendBuf.c_str() << endl;

    cout << js << endl;//能这样输出，肯定是定义了json类型的重载函数
}

// json序列化示例2
void func2(){
    json js;
    // 添加数组
    js["id"] = {1,2,3,4,5};/////值可以是数组类型
    // 添加key-value
    js["name"] = "zhang san";
    // 添加对象
    js["msg"]["zhang san"] = "hello world";/////这里msg这个对应的值js["msg"]，还可以看成一个json字符串，访问其键张三，等于嵌套json字符串
    js["msg"]["liu shuo"] = "hello china";
    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
    cout << js << endl;
}

///json序列化示例3
void func3(){
    json js;
    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;////////////直接给键添加了一个数组类型
    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m;

    string sendBuf = js.dump();//json数据对象===>序列化成json字符串
    cout << sendBuf << endl;///这样就可以通过网络进行发送
    // cout << js << endl;
}

///json反序列化示例1
string func4(){
    json js;//看做是容器
    //要组装的数据格式
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello world,what are you doing now";
    //将js看做容器，  键+值  的形式

    string sendBuf = js.dump();
    // cout << sendBuf.c_str() << endl;

    return sendBuf;
}

//json反序列化示例2
string func5(){
    json js;
    // 添加数组
    js["id"] = {1,2,3,4,5};/////值可以是数组类型
    // 添加key-value
    js["name"] = "zhang san";
    // 添加对象
    js["msg"]["zhang san"] = "hello world";/////这里msg这个对应的值js["msg"]，还可以看成一个json字符串，访问其键张三，等于嵌套json字符串
    js["msg"]["liu shuo"] = "hello china";
    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
    return js.dump();
}

///json反序列化示例3
string func6(){
    json js;
    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;////////////直接给键添加了一个数组类型
    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m;

    string sendBuf = js.dump();//json数据对象===>序列化成json字符串
    return sendBuf;
}

int main(){
    // func1();
    // func2();
    // func3();

    ///数据的反序列化1  json字符串反序列化成数据对象（看作容器，方便访问）
    // string recvBuf = func4();
    // json jsbuf = json::parse(recvBuf);
    // cout << jsbuf["msg_type"] << endl;
    // cout << jsbuf["from"] << endl;
    // cout << jsbuf["to"] << endl;
    // cout << jsbuf["msg"] << endl;

    //数据反序列化2
    // string recvBuf = func5();
    // json jsbuf = json::parse(recvBuf);
    // cout << jsbuf["id"] << endl;//数组
    // cout << jsbuf["id"][0] << endl;
    // auto msgjs = jsbuf["msg"];
    // cout << msgjs["zhang san"] << endl;//msg存放的还是json字符串
    // cout << msgjs["liu shuo"] << endl;

    //数据反序列化3
    string recvBuf = func6();
    json jsbuf = json::parse(recvBuf);
    vector<int> vec = jsbuf["list"];////将json对象中的数组类型，直接放入vector容器中
    for(auto &it:vec){
        cout << it << " ";
    }
    cout << endl;

    map<int, string> mymap = jsbuf["path"];
    for(auto &it:mymap){
        cout << it.first << " " << it.second << endl;
    }
    

    return 0;
}