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
    // ��������
    stCoCond_t* cond;
    // ���ݳ�
    std::vector<int> vecData;
    // ����ID
    int id;
    // Э��id
    int cid;
};

int main()
{
    stPara_t p;
    p.cond = co_cond_alloc();
    p.cid = p.id = 0;

    srand(time(NULL));
    // Э�̶���(CCB)��һ�������ߣ����������
    const int nConsumer = 2;
    stCoRoutine_t* pProducerCo = NULL;
    stCoRoutine_t* pConsumerCo[nConsumer] = { NULL };

    // ��������������Э��
    // ��Դ���֪�ú����ط���0
    co_create(&pProducerCo, NULL, Producer, &p);
    co_resume(pProducerCo);
    std::cout << "start producer coroutine success" << std::endl;

    // ��������������Э��
    for (int i = 0; i < nConsumer; i++)
    {
        co_create(&pConsumerCo[i], NULL, Consumer, &p);
        co_resume(pConsumerCo[i]);
    }
    std::cout << "start consumer coroutine success" << std::endl;

    // ����ѭ���¼�
    co_eventloop(co_get_epoll_ct(), NULL, NULL);

    return 0;
}


void* Producer(void* arg)
{
    // ����Э��HOOK��
    co_enable_hook_sys();

    stPara_t* p = (stPara_t*)arg;
    int cid = ++p->cid;
    while (true)
    {
        // �������������
        for (int i = rand() % 5 + 1; i > 0; i--)
        {
            p->vecData.push_back(++p->id);
            std::cout << "[" << cid << "] + add new data:" << p->id << std::endl;
        }
        // ֪ͨ������
        co_cond_signal(p->cond);
        // ����ʹ��poll�ȴ�
        poll(NULL, 0, 1000);
    }
    return NULL;
}
void* Consumer(void* arg)
{
    // ����Э��HOOK��
    co_enable_hook_sys();

    stPara_t* p = (stPara_t*)arg;
    int cid = ++p->cid;
    while (true)
    {
        // ������ݳأ���������ȴ�֪ͨ
        if (p->vecData.empty())
        {
            co_cond_timedwait(p->cond, -1);
            continue;
        }
        // ��������
        std::cout << "[" << cid << "] - del data:" << p->vecData.front() << std::endl;
        p->vecData.erase(p->vecData.begin());
    }
    return NULL;
}