#ifndef USER_H
#define USER_H
#include<string>
using namespace std;

//匹配User表的ORM映射关系类
class User{
public:
    User(int id=-1,string name="",string pwd="",string state="offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }
    //业务操作的对象，无法访问私有成员变量
    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string state) { this->state = state; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getPwd() { return this->password; }
    string getState() { return this->state; }


private:
    int id;
    string name;
    string password;
    string state;
};

#endif