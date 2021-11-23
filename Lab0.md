# Lab0: networking warmup

## 1 Set up GNU/Linux on your computer

配置环境，官方指定OS为`Ubuntu18.04`，并且提供了环境配置文件，推荐使用`VirtualBox`，但本次Lab使用的是`docker`容器

## 2 Networking by hand

### 2.1 Fetch a Web Page

```shell
$ telnet cs144.keithw.org http
Trying 104.196.238.229...
Connected to cs144.keithw.org.
Escape character is '^]'.
GET /hello HTTP/1.1     # Method URL Http_version
Host: cs144.keithw.org  # Part of URL
Connection: close       # finished
                        # Empty line means http request is done
HTTP/1.1 200 OK
Date: Tue, 23 Nov 2021 04:09:45 GMT
Server: Apache
Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
ETag: "e-57ce93446cb64"
Accept-Ranges: bytes
Content-Length: 14
Connection: close
Content-Type: text/plain

Hello, CS144!
Connection closed by foreign host.
```
没有sunetid,跳过Assignment

### 2.2 Send yourself an email

同样没有sunetid，尝试用gmail和qq邮箱通信

```shell
$ telnet smtp.qq.com 587     # 只支持465/587端口
Trying 183.47.101.192...
Connected to smtp.qq.com.
Escape character is '^]'.
220 newxmesmtplogicsvrszc8.qq.com XMail Esmtp QQ Mail Server.
EHLO smtp                    # EHLO表明身份
250-newxmesmtplogicsvrszc8.qq.com
250-PIPELINING
250-SIZE 73400320
250-STARTTLS
250-AUTH LOGIN PLAIN XOAUTH XOAUTH2     # 支持LOGIN PLAIN等认证方式
250-AUTH=LOGIN
250-MAILCOMPRESS
250 8BITMIME
AUTH LOGIN
334 VXNlcm5hbWU6                        # 内容为Username:，经过Base64加密
NDIxNTU0MDM3QHFxLmNvbQ==                # 自己邮箱*********@qq.com经过Base64加密
334 UGFzc3dvcmQ6                        # 内容为Password:，经过Base64加密
cWpid3R6YWFudmJiYmlkYw==                # 不是密码，是qq邮箱授权码经过Base64加密
235 Authentication successful
mail from:<*********@qq.com>            # 发件人，填明文
250 OK.
rcpt to:<*********@gmail.com>           # 收件人
250 OK.
data                                    # 表明要开始发送了
354 End data with <CR><LF>.<CR><LF>.
From: *********@qq.com                  # 发送人
To: *********@*gmail.com                # 收件人
Subject: Hello from CS144 Lab 0!        # 主题
                                        # 空一行表明邮件头结束
Try to use smtp!                        # 邮件内容
.                                       # 单独一行.表明结束
250 OK: queued as.                      
quit                                    # 退出
221 Bye.
Connection closed by foreign host.from:
```

### 2.3 Listening and connecting

开启两个terminal，任一方发出的消息另一方都会收到并显示
```shell
$ netcat -v -l -p 9090      # 服务器，在本地9090端口开始监听
Listening on [0.0.0.0] (family 0, port 9090)
Connection from localhost 57578 received!
hello world
hello from cs144 lab0
hello from server(netcat)
hello from client(telnet)
bye
^C
```

```shell
$ telnet localhost 9090    # 客户端
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
hello world
hello from cs144 lab0
hello from server(netcat)
hello from client(telnet)
bye
Connection closed by foreign host.
```

## 3 Writing a network program using an OS stream socket

写一个`webget`程序创建tcp套接字，连接web服务器并获取网页

### 3.1 Let’s get started—fetching and building the starter code

clone仓库，编译

### 3.2 Modern C++: mostly safe but still fast and low-level

使用C++完成Lab

不要使用成对操作，例如`malloc/free`或者`new/delete`这种，例如在操作1后抛出了异常导致操作2不会发生。使用RAII特性，在构造函数中操作，在析构函数中做对应相反操作

基本要点：
- 不要使用`malloc()/free()`
- 不要使用`new/delete`
- 基本上不要使用裸指针，仅在必要时使用智能指针
- 避免使用模板、线程、锁、虚函数
- 避免使用C语言风格字符串及相关操作，用`std::string`代替
- 不要使用C语言强制转换，如果需要使用`statci_cast`
- 函数参数优先传递`const`引用
- 尽可能使用`const`变量
- 尽可能让成员函数是`const`
- 避免使用全局变量，使变量作用域最小化

### 3.3 Writing `webget`

注意每行结尾是`\r\n`，要多一个空行即可
```cpp
void get_URL(const string &host, const string &path) {
    // Your code here.
    TCPSocket sock;
    sock.connect(Address(host, "http"));
    const string http_get = "GET " + path + " HTTP/1.1\r\n" +
                            "Host: " + host + "\r\n" +
                            "Connection: close\r\n" +
                            "\r\n";
    sock.write(http_get);
    while(!sock.eof())
        cout << sock.read();
    sock.close();
    // You will need to connect to the "http" service on
    // the computer whose name is in the "host" string,
    // then request the URL path given in the "path" string.

    // Then you'll need to print out everything the server sends back,
    // (not just one call to read() -- everything) until you reach
    // the "eof" (end of file).

    // cerr << "Function called: get_URL(" << host << ", " << path << ").\n";
    // cerr << "Warning: get_URL() has not been implemented yet.\n";
}
```

## 4 An in-memory reliable byte stream

### 目的

有一个容量为`capacity`的内存缓冲区`buffer`，写者可以将数据写到`buffer`内，读者可以从`buffer`读取数据，实现读者写者相关接口：

写者：
```cpp
// Write a string of bytes into the stream. Write as many
// as will fit, and return the number of bytes written.
size_t write(const std::string &data);
// Returns the number of additional bytes that the stream has space for
size_t remaining_capacity() const;
// Signal that the byte stream has reached its ending
void end_input();
// Indicate that the stream suffered an error
void set_error();
```

读者：
```cpp
// Peek at next "len" bytes of the stream
std::string peek_output(const size_t len) const;
// Remove ``len'' bytes from the buffer
void pop_output(const size_t len);
// Read (i.e., copy and then pop) the next "len" bytes of the stream
std::string read(const size_t len);
bool input_ended() const; // `true` if the stream input has ended
bool eof() const; // `true` if the output has reached the ending
bool error() const; // `true` if the stream has suffered an error
size_t buffer_size() const; // the maximum amount that can currently be peeked/read
bool buffer_empty() const; // `true` if the buffer is empty
size_t bytes_written() const; // Total number of bytes written
size_t bytes_read() const; // Total number of bytes poppe
```

### 问题分析

数据是`FIFO`的，采用循环队列实现，循环队列相关操作如下：

```cpp
vector<Element> buffer;
size_t capacity;    // 队列容量，考虑到需要留一个空位来区分空还是满，所以需要capacity+1的空间 
size_t front, rear; // front指向队首，读者从这端取数据，rear指向尾后，写者从这端写入数据
void init() {front = rear = 0;}
bool empty() {return front == rear;} // 队列为空时两个指针重叠
bool full() {return (rear + 1) % (capacity + 1) == front;} // 队列为满时rear的下一个位置为front
bool size() {return (rear - front + capacity + 1) % (capacity + 1)} // 队列当前元素个数
void push(Element e) {buffer[rear] = e; rear = (rear + 1) % (capacity + 1);} // 写者写一个元素
Element pop() {Element e = buffer[front]; front = (front + 1) % (capacity + 1); return e;} // 读者读一个元素
```

由于是单线程，不用考虑多读者多写者多线程，不用考虑数据同步互斥

### 实现

#### ByteStream类
```cpp
class ByteStream {
  private:
    // Your code here -- add private members as necessary.

    // Hint: This doesn't need to be a sophisticated data structure at
    // all, but if any of your tests are taking longer than a second,
    // that's a sign that you probably want to keep exploring
    // different approaches.
    size_t _capacity;   // 缓冲区大小
    size_t _front, _rear;   // 头尾指针
    vector<char> _buffer;   // 缓冲区
    size_t _read_count, _write_count;   // 读写字节数
    bool _eof{};    // 文件尾标志表示写者结束写操作
    bool _error{};  //!< Flag indicating that the stream suffered an error.

    size_t buffer_used() const { return (_rear + _capacity + 1 - _front) % (_capacity + 1); }

  public:
    //! Construct a stream with room for `capacity` bytes.
    ByteStream(const size_t capacity);

    //! \name "Input" interface for the writer
    //!@{

    //! Write a string of bytes into the stream. Write as many
    //! as will fit, and return how many were written.
    //! \returns the number of bytes accepted into the stream
    size_t write(const std::string &data);

    //! \returns the number of additional bytes that the stream has space for
    size_t remaining_capacity() const;

    //! Signal that the byte stream has reached its ending
    void end_input();

    //! Indicate that the stream suffered an error.
    void set_error() { _error = true; }
    //!@}

    //! \name "Output" interface for the reader
    //!@{

    //! Peek at next "len" bytes of the stream
    //! \returns a string
    std::string peek_output(const size_t len) const;

    //! Remove bytes from the buffer
    void pop_output(const size_t len);

    //! Read (i.e., copy and then pop) the next "len" bytes of the stream
    //! \returns a string
    std::string read(const size_t len);

    //! \returns `true` if the stream input has ended
    bool input_ended() const;

    //! \returns `true` if the stream has suffered an error
    bool error() const { return _error; }

    //! \returns the maximum amount that can currently be read from the stream
    size_t buffer_size() const;

    //! \returns `true` if the buffer is empty
    bool buffer_empty() const;

    //! \returns `true` if the output has reached the ending
    bool eof() const;
    //!@}

    //! \name General accounting
    //!@{

    //! Total number of bytes written
    size_t bytes_written() const;

    //! Total number of bytes popped
    size_t bytes_read() const;
    //!@}
};
```

#### 构造函数

```cpp
// 构造函数接受一个容量参数，初始化时队列头尾指针置0表明空，buffer分配空间，读写字节数初始化为0
ByteStream::ByteStream(const size_t capacity)
    : _capacity(capacity), _front(0), _rear(0), _buffer(vector<char>(_capacity)), _read_count(0), _write_count(0) {}
```

#### 写者接口

```cpp
// 写入数据，返回实际写的字节数
size_t ByteStream::write(const string &data) {
    // 实际写的字节数应该为剩余空间和数据大小的较小值
    size_t len = min(data.size(), remaining_capacity());
    size_t index = 0;
    // 写入队列
    while (index != len) {
        _buffer[_rear] = data[index++];
        _rear = (_rear + 1) % (_capacity + 1);
    }
    // 更新总写入字节数
    _write_count += len;
    // 返回本次实际写入字节数
    return len;
}

// 返回队列剩余空间
size_t ByteStream::remaining_capacity() const { return _capacity - buffer_used(); }

// 表明写者已经写完数据，置_eof为true
void ByteStream::end_input() { _eof = true; }

// 查询当前输入是否结束
bool ByteStream::input_ended() const { return _eof; }

// 返回总写入字节数
size_t ByteStream::bytes_written() const { return _write_count; }
```

#### 读者接口

```cpp
// 返回当前队列最大可读字节数
size_t ByteStream::buffer_size() const { return buffer_used(); }

// 查询队列是否为空
bool ByteStream::buffer_empty() const { return _front == _rear; }

// 查询是否读取到文件尾，当前队列为空且_eof为真表明没有新数据可读
bool ByteStream::eof() const { return _eof && buffer_empty(); }

// 查看当前队首若干字节数，不更新队首队尾指针，不更新读取字节数
string ByteStream::peek_output(const size_t len) const {
    size_t peek_len = min(len, buffer_size());
    string peek_str;
    size_t cur = _front;
    while (peek_len > 0) {
        peek_str.push_back(_buffer[cur]);
        cur = (cur + 1) % (_capacity + 1);
        peek_len--;
    }
    return peek_str;
}

// 舍弃当前队首若干字节数，需要更新队首指针，同时更新读取字节数
void ByteStream::pop_output(const size_t len) {
    size_t rm_len = min(len, buffer_size());
    _front = (_front + rm_len) % (_capacity + 1);
    _read_count += rm_len;
}

// 读取当前队首若干字节数，需要更新队首指针，同时更新读取字节数，返回读取到的数据
std::string ByteStream::read(const size_t len) {
    size_t read_len = min(len, buffer_size());
    string read_str;
    while (read_len > 0) {
        read_str.push_back(_buffer[_front]);
        _front = (_front + 1) % (_capacity + 1);
        read_len--, _read_count++;
    }
    return read_str;
}

// 返回总读取字节数
size_t ByteStream::bytes_read() const { return _read_count; }
```

### 问题记录

#### `error: ‘ByteStream::_capacity’ will be initialized after [-Werror=reorder]`

应当按照成员声明时候的顺序在初始化列表中初始化，编译器是按照这个顺序初始化的，如果不按顺序可能出现未定义行为

```cpp
class A {
    private:
        int m, n;
    public:
        A(int i): n(i), m(n)
    // A(10)得到的不是m = n = 10
    // 因为m先初始化，将n的值赋给m，而此时n尚未初始化，m会得到一个脏值，再用i=10初始化n，结果为m = ?, n = 10
}
```

#### `error: ‘ByteStream::_buffer’ should be initialized in the member initialization list [-Werror=effc++]`

应当使用初始化列表来初始化

#### 采用`void ByteStream::pop_output(const size_t len)`后，测试一直卡在`bytes_read()`结果出错无法通过

翻看测试文件源码发现，`pop_output`虽然作用是舍弃数据，但也需要更新读取字节数，和`read()`区别在于它不会返回读取到的数据





