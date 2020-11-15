#include <windows.h>
#include <iostream>

#include <fstream>
#include <stdarg.h>
#include <ctime>
#include <queue> 
#include <tchar.h>
using namespace std;

#if 1
#define LOCK_QUEUE_MUTEX   WaitForSingleObject(m_mutex,INFINITE);
#define UNLOCK_QUEUE_MUTEX ReleaseMutex(m_mutex);

#else
#define LOCK_QUEUE_MUTEX   
#define UNLOCK_QUEUE_MUTEX 
#endif


class SafeQueue
{
private:
    std::queue<std::string> m_queue;
    HANDLE m_mutex;
public:
    SafeQueue()
    {
        m_mutex = NULL;
        m_mutex = CreateMutex(NULL, FALSE, NULL); 
    }

    ~SafeQueue()
    {
        if(m_mutex)
            CloseHandle(m_mutex);
    }

    void push(std::string string)
    {
        LOCK_QUEUE_MUTEX;
        m_queue.push(string);
        UNLOCK_QUEUE_MUTEX;
    }
        
    bool pop(std::string& out)
    {
        LOCK_QUEUE_MUTEX;
        if(m_queue.empty())
        {
            UNLOCK_QUEUE_MUTEX;
            return false;
        }
        out = m_queue.front();
        m_queue.pop();
        UNLOCK_QUEUE_MUTEX;
        return true;;
    };

    bool Empty()
    {
        LOCK_QUEUE_MUTEX;
        bool isEmpty = m_queue.empty();
        UNLOCK_QUEUE_MUTEX;
        return isEmpty;
    }
};

SafeQueue g_queue;

namespace log
{
    //void printf(const char* pszFileName, const char* pszLog, ...)
}

namespace log
{    
    std::string GetTimeStamp()
    {
        struct tm ltm = {};
        std::time_t now = std::time(NULL) ;
        localtime_s(&ltm, &now);

        char cstr[256] = {0,};
        return std::strftime( cstr, sizeof(cstr), "%Y-%m%d-%H:%M:%S>", &ltm) ? cstr : "" ;
    } 

    void printf(const char* pszFileName, const char* pszLog, ...)
    {
#define MAX_LOG_LEN   1024
        fstream _streamOut;
        _streamOut.open(pszFileName, ios::out | ios::app);
     
        va_list argList;
        char cbuffer[MAX_LOG_LEN];
        va_start(argList, pszLog);

        std::string now = GetTimeStamp();
        vsnprintf_s(cbuffer, sizeof(char)*MAX_LOG_LEN, MAX_LOG_LEN, pszLog, argList);
        va_end(argList);
     
        now.append(cbuffer);
        _streamOut << now.c_str() << endl;
        _streamOut.close();
    }
}

#define NUM_STRING_NUM  300
#define TEST1_COUNT 100
#define THREAD_COUNT 4

volatile long g_index = -1;
DWORD WINAPI threadFun1(void *lpVoid)
{
    int b = 0;
    char cbuffer[NUM_STRING_NUM] = {0};

    for (int n = 0; n < TEST1_COUNT; n++)
    {
        Sleep(1);
        long ret = InterlockedIncrement(&g_index);
        sprintf_s(cbuffer, NUM_STRING_NUM, "queue push %d", ret);
        g_queue.push( cbuffer);
        std::cout << n <<" - Fun1()" << std::endl;
    }

    return 0;
}
 
DWORD WINAPI threadFun2(void *lpVoid)
{
    int b = 0;
 
    for (int n = 0; n < TEST1_COUNT; n++)
    {
        Sleep(1);

        std::string temp;
        if(!g_queue.pop(temp))
            continue;
        std::cout << n <<" - Fun2()" << temp.c_str() << std::endl;
    }

    return 0;
}

void Test1()
{
    HANDLE hThread[THREAD_COUNT];
    hThread[0] = CreateThread(NULL, 0, ::threadFun1, NULL, 0, NULL);
    hThread[1] = CreateThread(NULL, 0, ::threadFun2, NULL, 0, NULL);
    hThread[2] = CreateThread(NULL, 0, ::threadFun1, NULL, 0, NULL);
    hThread[3] = CreateThread(NULL, 0, ::threadFun2, NULL, 0, NULL);

    WaitForMultipleObjects(THREAD_COUNT, hThread, true, INFINITE);
    for(int n = 0; n < THREAD_COUNT; ++n)
         CloseHandle(hThread[n]);
}

int _tmain(int argc, _TCHAR* argv[])
{
    Test1();
    log::printf("mytest.txt", "aaaaa %d \n", 100);
    return 0;
}