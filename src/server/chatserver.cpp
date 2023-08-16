// #include"/home/zzj/code/include/server/chatserver.hpp"
#include"chatserver.hpp"
#include"json.hpp"
#include"chatservice.hpp"
#include <functional>
#include<string>
using namespace std;
using namespace placeholders; // 参数占位符
using json = nlohmann::json;

// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    /// 注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
    //主reactor负责新用户的连接
    //子reactor负责已连接用户读写事件的处理
}

// 启动服务
void ChatServer::start()
{
    _server.start();
}

// 上报连接创建/断开信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    //客户端断开连接
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown(); /// 用户下线，日志会打印输出，这里就不打印了
    }
}

// 上报读写事件相关信息的回调函数（在多个线程中回调，在多线程环境中执行）
void ChatServer::onMessage(const TcpConnectionPtr & conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();//将缓冲区中数据，拿到字符串中
    //数据的反序列化
    json js = json::parse(buf);

    //想要达到的目的：完全解耦网络模块的代码，和业务模块的代码（不直接调用业务模块的方法）
    //通过js["msgid"]获取=》业务处理器（handler）=》conn   js   time

    //网络层只有这两行，没有业务层的方法调用，只要在服务层内部，将相应的消息和回调，进行绑定
    auto msgHandler= ChatService::instance()->getHandler(js["msgid"].get<int>());/////js["msgid"]得出来还是json类型，需要用get来实例化,强制转换成指定类型
    //回调消息绑定好的事件处理器，来进行相应的业务处理
    msgHandler(conn, js, time);
}
