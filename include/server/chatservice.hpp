#ifndef CHATSERVICE_H////头文件不重复包含
#define CHATSERVICE_H

#include<muduo/net/TcpConnection.h>
#include<unordered_map>
#include<functional>
#include<mutex>//业务锁，处理进程问题
using namespace std;
using namespace muduo;
using namespace muduo::net;
#include"redis.hpp"
#include"groupmodel.hpp"
#include"friendmodel.hpp"
#include"offlinemessagemodel.hpp"
#include"usermodel.hpp"
#include"json.hpp"
using json = nlohmann::json;

//处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

//聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService *instance();
    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);//因为是网络层派发过来的处理器回调，他们的参数都一样
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //服务器异常，业务重置方法
    void reset();

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);

private:
    ChatService();//构造函数私有化

    //存储消息ID，和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap; // 消息ID对应的处理操作

    //存储在线用户的通信连接（连接相关的mao表）
    unordered_map<int, TcpConnectionPtr> _userConnMap;//第一个int是用户id
    //会随着用户上线/下线改变，要注意线程安全

    //定义互斥锁，保证_userconnMap的线程安全
    mutex _connMutex;

    //数据操作类对象
    UserModel _userModel;

    OfflineMsgModel _offlineMsgModel;

    FriendModel _friendModel;

    GroupModel _groupModel;

    //redis操作对象
    Redis _redis;
};

#endif