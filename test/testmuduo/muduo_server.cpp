/*
    muduo网络库为用户提供了两个主要的类
    TcpServer：用于编写服务器程序
    TcpClient：用于编写客户端程序


    网络库就是封装了“epoll+线程池”的模型
    好处：能够把网络I/O代码和业务代码区分开，使用户专注于业务代码。
            只需要关注：用户的连接与断开    用户的可读写事件

*/
#include <muduo/net/TcpServer.h> //服务器的开发
#include <muduo/net/EventLoop.h>
#include <iostream>
#include<functional>///绑定器都是在functional头文件中
#include<string>
using namespace std;
using namespace muduo;//muduo下有很多作用域
using namespace muduo::net;
using namespace placeholders;

/// 基于muduo网络库开发服务器程序
/*
要做的事情：(使用TcpServer开发服务器的步骤，步骤是固定的，主要精力在onconnection和onmessage上)
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4.在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写事件的回调函数
5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程

*/

///使用muduo库的tcpserver，性能不错的，基于事件驱动的，I/O复用epoll+线程池模型的网络服务器

class ChatServer
{
public:
    ChatServer(EventLoop *loop,//事件循环，可以理解为reactor
               const InetAddress &listenAddr,//IP+port
               const string &nameArg)//服务器的名字，给线程绑定的名字
        : _server(loop, listenAddr, nameArg), _loop(loop) // 保存事件循环，以便操作epoll
    {//对应L9使用网络库还需要关注哪些

        //需要网络库监听这件事什么时候发生。目前只知道发生后该做什么，但不知道什么时候发生


        //给服务器注册用户连接的创建和断开回调（对应上面用户需要关注的东西）
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));//_1是参数占位符，onConnection方法有一个参数
        //监听到用户的连接、创建、断开时，就会调用onConnection函数，只需要关注onConnection中连接的创建和断开就可以了





        //给服务器注册用户读写回调（对应上面用户需要关注的东西）
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
        // placeholder就是帮函数占参数位置的，是参数占位符，_1，_2就是告诉编译器，先占个坑，以后填

        // 设置服务器端的线程数量;1个I/O线程，3个worker线程
        _server.setThreadNum(4);
    }

    //开启事件循环
    void start(){
        _server.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn){//专门处理用户的连接创建和断开////写成成员方法，是为了访问对象的成员变量
    //成员方法会有this指针，会和setConnectionCallback的类型不同，所以用绑定器绑定

        

        if(conn->connected()){
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:online"<<endl;
        }
        else{//连接断开
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:offline"<<endl;
            conn->shutdown();//close fd，回收服务端fd的资源
            // _loop->quit();
        }
        ///这里toIpPort就是把IP和端口号都打印出来
        
    }


    //专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn,     //连接
                    Buffer*buffer,     //缓冲区
                    Timestamp time)      //接收到数据的时间信息
    {
        string buf = buffer->retrieveAllAsString();///将接收到的数据，全放到字符串中
        cout << "recv data:" << buf << "time" << time.toString() << endl;//time.toString()time时间戳中方法，可以将time信息转成字符串
        conn->send(buf);///将收到的再发回去
    }
    TcpServer _server; // #1 //可以通过查看TcpServer的头文件，看其返回的参数
    EventLoop *_loop;  // #2，也可以看作是epoll，事件循环的指针
};

int main(){
    EventLoop loop;//epoll
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop,addr,"ChatServer");///创建server对象

    server.start();///listenfd  epoll_ctl=>epoll
    loop.loop();///以阻塞方式，等待新用户连接，已连接用户的读写事件等

    return 0;
}
