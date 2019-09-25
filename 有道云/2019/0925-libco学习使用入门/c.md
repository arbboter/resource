## 简介
libco是微信后台大规模使用的c/c++协程库，2013年至今稳定运行在微信后台的数万台机器上。

libco通过仅有的几个函数接口 co_create/co_resume/co_yield 再配合 co_poll，可以支持同步或者异步的写法，如线程库一样轻松。同时库里面提供了socket族函数的hook，使得后台逻辑服务几乎不用修改逻辑代码就可以完成异步化改造。

libco是一个源码简洁而且性能高效的协程库，可以通过阅读源码学习理解协程的概念及技术实现方案，也可根据项目需要使用协程方案，本章节试图做一个入门示例，从源码编译到项目使用，力争记录libco的入门使用方法。

>说明：因为平台的问题，可能实际操作中遇到的问题与本文章记录的不一样，请自行分析解决，不可完全照搬


## libco库下载编译
从Github下下载并编译`libco`，编译很简单，直接`make`则可以，如下：
```bash
tony:~/code/github$ git clone https://github.com/Tencent/libco.git
Cloning into 'libco'...
remote: Enumerating objects: 256, done.
remote: Total 256 (delta 0), reused 0 (delta 0), pack-reused 256
Receiving objects: 100% (256/256), 180.23 KiB | 151.00 KiB/s, done.
Resolving deltas: 100% (143/143), done.
tony:~/code/github$ cd libco/
tony:~/code/github/libco$ make
xxx编译过程xxx
tony:~/code/github/libco$ ls lib/*
lib/libcolib.a
tony:~/code/github/libco$ ls solib/*
solib/libcolib.so
```
编译完成后，可以看到生成了对应的静态库文件`lib/libcolib.a`和动态库文件`solib/libcolib.so`，后续项目编码使用时，只需要包含头文件`co_routine.h`即可。

需要注意的是，因为平台为问题，可能用默认的配置编译成功后的库有问题，会提示

## 新建Linux控制台项目测试libco
在多线程编程教程中，有一个经典的例子：生产者消费者问题。事实上，生产者消费者问题也是最适合协程的应用场景，因此我们这次使用生产者消费者场景来测试libco。
因为使用静态库文件开发方便，因此本次使用静态库来开发测试，首先需要将唯一引用的头文件引入到项目中，可直接拷贝到项目目录并添加到项目文件中，并根据`libco`开源项目及示例说明，学习使用方案。

1.创建协程对象
```cpp
// 声明一个协程对象类型指针
stCoRoutine_t* pProducerCo = NULL;

// 调用函数创建协程对象，函数内会分配对象指针
// 看源码可知该函数必返回0，因此不必判断返回值
co_create(&pProducerCo, NULL, Producer, &p);
```
函数原型声明为:
```cpp
int co_create( stCoRoutine_t **co,const stCoRoutineAttr_t *attr,void *(*routine)(void*),void *arg )
```
参数`stCoRoutine_t`为出参，返回创建的协程对象
参数`stCoRoutineAttr_t`为入参，执行创建协程的属性，本次使用默认属性，传空
参数`routine`为入参，指定协程执行函数
参数`arg`为入参，指定协议执行函数的参数

2.创建生产者和消费者协程执行函数
```cpp
void* Producer(void* arg);
void* Consumer(void* arg);
```
根据`co_create`函数声明的协程执行函数原型，分别创建生产者和消费者的函数，需要注意的是，协程函数体内需要先启用协程HOOK，如下所示:
```cpp
void* Producer(void* arg)
{
    // 启用协程HOOK项
    co_enable_hook_sys();
    stPara_t* p = (stPara_t*)arg;
    while (true)
    {
    }
    return NULL;
}
```

3.指定协程执行参数
生产者消费者之间的通信需要使用条件变量，且需要有个公共池用于存取数据，因此可声明协程参数为:
```cpp
struct stPara_t
{
    // 条件变量
    stCoCond_t* cond;
    // 数据池
    std::vector<int> vecData;
    // 数据ID
    int id;
    // 协程id
    int cid;
};
```
在创建协程前，创建该参数对象，斌初始化：
```cpp
stPara_t p;
p.cond = co_cond_alloc();
p.cid = p.id = 0;
```

4.启动协程
使用`co_create`创建的协程并未启用为执行，需要我们使用`co_resume`显示启动协程，函数`co_resume`声明原型如下:
```cpp
void co_resume(stCoRoutine_t *co)
```
参数`stCoRoutine_t`为入参，是`co_create`的出参，该函数用于首次启动协程或者使得当前协程让出执行权限给其他协程，因此是`resume`而不是`start`

5.启动协程事件处理循环
协程创建启动完之后，我们需要执行`epoll`的事件循环处理，协助协程的调度及异步操作，代码如下:
```cpp

```

最后我们可以编写以下测试代码：

```cpp
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include "co_routine.h"

void* Producer(void* arg);
void* Consumer(void* arg);

struct stPara_t
{
    // 条件变量
    stCoCond_t* cond;
    // 数据池
    std::vector<int> vecData;
    // 数据ID
    int id;
    // 协程id
    int cid;
};

int main()
{
    stPara_t p;
    p.cond = co_cond_alloc();
    p.cid = p.id = 0;

    srand(time(NULL));
    // 协程对象(CCB)，一个生产者，多个消费者
    const int nConsumer = 2;
    stCoRoutine_t* pProducerCo = NULL;
    stCoRoutine_t* pConsumerCo[nConsumer] = { NULL };

    // 创建启动生产者协程
    // 看源码可知该函数必返回0
    co_create(&pProducerCo, NULL, Producer, &p);
    co_resume(pProducerCo);
    std::cout << "start producer coroutine success" << std::endl;

    // 创建启动消费者协程
    for (int i = 0; i < nConsumer; i++)
    {
        co_create(&pConsumerCo[i], NULL, Consumer, &p);
        co_resume(pConsumerCo[i]);
    }
    std::cout << "start consumer coroutine success" << std::endl;

    // 启动循环事件
    co_eventloop(co_get_epoll_ct(), NULL, NULL);

    return 0;
}


void* Producer(void* arg)
{
    // 启用协程HOOK项
    co_enable_hook_sys();

    stPara_t* p = (stPara_t*)arg;
    int cid = ++p->cid;
    while (true)
    {
        // 产生随机个数据
        for (int i = rand() % 5 + 1; i > 0; i--)
        {
            p->vecData.push_back(++p->id);
            std::cout << "[" << cid << "] + add new data:" << p->id << std::endl;
        }
        // 通知消费者
        co_cond_signal(p->cond);
        // 必须使用poll等待
        poll(NULL, 0, 1000);
    }
    return NULL;
}
void* Consumer(void* arg)
{
    // 启用协程HOOK项
    co_enable_hook_sys();

    stPara_t* p = (stPara_t*)arg;
    int cid = ++p->cid;
    while (true)
    {
        // 检查数据池，无数据则等待通知
        if (p->vecData.empty())
        {
            co_cond_timedwait(p->cond, -1);
            continue;
        }
        // 消费数据
        std::cout << "[" << cid << "] - del data:" << p->vecData.front() << std::endl;
        p->vecData.erase(p->vecData.begin());
    }
    return NULL;
}
```

## 编译链接
因为项目引用静态库，因此需要引入`libcolib.a`文件，引入该文件分为两步：1.指定库文件路径，2.指定引用库文件名。在VS中具体操作如下：

![1.配置静态库目录][1]

指定库文件的目录，接着再指定库文件名:

![2.配置静态库文件][2]

点击确定后，可以试着编译项目，发现编译出错，主要信息如下：
```output
1>D:\AppData\PerDoc\temp\demo\libco_demo\libcolib.a(co_hook_sys_call.o) : error : In function `__static_initialization_and_destruction_0(int, int) [clone .constprop.28]':
1>/home/tony/code/github/libco/co_hook_sys_call.cpp(107): error : undefined reference to `dlsym'
1>D:\AppData\PerDoc\temp\demo\libco_demo\libcolib.a(co_hook_sys_call.o) : error : /home/tony/code/github/libco/co_hook_sys_call.cpp:109: more undefined references to `dlsym' follow
1>D:\AppData\PerDoc\temp\demo\libco_demo\libcolib.a(co_routine.o) : error : In function `co_getspecific(unsigned int)':
1>/home/tony/code/github/libco/co_routine.cpp(1062): error : undefined reference to `pthread_getspecific'
```
上述错误主要为【undefined reference to 'dlsym'】和【undefined reference to 'pthread_getspecific'】，此为平台类的库链接错误，需要调整编译选项，需要在链接时添加额外选项`-pthread -Wl,--no-as-needed -ldl`，配置如下：

![链接选项配置][3]

此时再次重新编译链接，则可以发现成功。

## 调试运行
此时按`F5`调试运行，则可以看到程序可以正常执行，结果如下:

![调试运行结果][4]

通过运行结果发现：
1. 协程库有效，我们未通过多线程技术，实现了生产者消费者运行场景，且生产后立即消费，无延时，性能佳
2. 测试代码中有一个生产者，两个消费者，但是在运行过程中，每个批次都是其中一个消费者在工作，但是不同批次则会在两个消费者中交替选取。分析代码可发现，因为`libco`的协程调度是又程序控制，而我们的代码中当某个消费者协程开始消费后，则会一直消费直到没有数据才会让出协程，因此每次消费者都会将数据消费完才将协程让出给生产者。

[1]: 配置静态库目录.jpg
[2]: 配置静态库文件.jpg
[3]: 链接选项配置.jpg
[4]: 调试运行结果.jpg