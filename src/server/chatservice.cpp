#include"chatservice.hpp"
#include"public.hpp"
#include<muduo/base/Logging.h>//muduo库中的日志打印
#include<vector>
using namespace std;
using namespace muduo;

//获取单例对象的接口函数
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

//注册消息，以及对应的回调操作
ChatService::ChatService()//构造方法
{
    //用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg,this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)}); // 绑定相应的消息ID和事件处理器
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (_redis.connect())///通道上有消息发生，就会向相应服务器上报
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));//init_notify_handler需要接受两个参数
    }
}

//服务器异常，业务重置方法
void ChatService::reset()
{
    //把online状态的用户，设置成offline
    _userModel.resetState();
}

//获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);///不用中括号查询，万一不在，中括号会引起副作用
    if(it==_msgHandlerMap.end())//msgid在map表中没有预制过相应的业务处理器
    {
        //返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {//[=]按值获取
            LOG_ERROR << "msgid: " << msgid << " can not find handler!";
        };

        

    }
    else{
        return _msgHandlerMap[msgid];
    }
    
}

// 处理登录业务（只知道如果有人登陆该做什么，什么时候发生不知道） id pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // LOG_INFO << "do login service!!!";
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);
    if(user.getId()==id&&user.getPwd()==pwd){//用户存在且密码正确
        if(user.getState()=="online")
        {
            //该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;//响应是否成功
            response["errmsg"] = "This account is using, input another";
            conn->send(response.dump());
        }
        else
        {
            //登录成功，记录用户连接信息
            {//大括号是互斥锁的作用域
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id); 

            //登录成功，更新用户状态信息off line=》online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;//响应是否成功
            response["id"] = user.getId();
            response["name"] = user.getName();///用户昵称
            //查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                //读取该用户的离线消息后，把该用户的所有离线消息删除
                _offlineMsgModel.remove(id);
            }

            //查询该用户的好友信息，并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty())
            {
                vector<string> vec2;
                for(User &user:userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
            
        }
        
    }
    else
    {
        //该用户不存在/用户存在但是密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;//响应是否成功
        response["errmsg"] = "id or password is invalid";
        conn->send(response.dump());
    }
}
//处理注册业务  name  password（ID是注册成功返回的）
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // LOG_INFO << "do reg service!!!";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state=_userModel.insert(user);
    if(state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;//响应是否成功
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;//响应是否成功，有错了就不用读ID字段了
        conn->send(response.dump());
    }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    //用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid); 

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    //能下线，说明肯定在map表中有记录
    //大括号是互斥锁的作用域
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if(it->second==conn)
            {
                //从map表删除用户的链接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId()); 
        
    //更新用户的状态信息
    if(user.getId()!=-1){//确认是有效的用户
        user.setState("offline");
        _userModel.updateState(user);
    }
    
}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();//然后要在表上找这个id对应的connection


    // 要访问连接信息表，必须保证线程安全
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);//互斥锁线程安全是围绕map表的操作
        if(it!=_userConnMap.end())//这里是在同一台服务器注册的，直接通信
        {
            //toid在线，转发消息,服务器主动推送消息给toid用户
            it->second->send(js.dump()); // it->second是to用户对应的connection
            return;
        }
        
    }
    //查询toid是否在线 （当前主机没找着，可能是不在线，或者在其他主机登录）
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }
    //toid不在线,存储离线消息
    _offlineMsgModel.insert(toid, js.dump());

}

//添加好友业务 msgid, id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    _friendModel.insert(userid, friendid);

}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();//获取要创建群所需的消息
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);//一开始并不知道群组id，通过createGroup获取群组id（这里是allgroup表）
    if (_groupModel.createGroup(group))//不能重复名称
    {
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");//groupuser表
        //这里也可以向客户端作出响应，参考登录/注册成功的响应
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");//加入群身份就是normal
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())//是否在线
        {
            // 转发群消息
            it->second->send(js.dump());///键是id，值就是connection
        }
        else
        {
            // 查询toid是否在线 （当前主机不在线，还有可能在其他主机上）
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());//用id作为通道号，发布消息
            }
            else
            {
                // 存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }

    
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)//由redis调用，用户在哪台服务器登录，服务器就会收到消息
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);//能接收到消息，肯定使能找到connection的
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);//向通道发布消息，到从通道取消息的过程中，该用户下线
}

