#include <windows.h>
#include <iostream>
#include <conio.h>   //for _getch function

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

#define STRING_NUM  300
#define TEST1_COUNT 100
#define THREAD_COUNT 1

HANDLE g_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

DWORD WINAPI WriteLogFile(void *lpVoid)
{
    int b = 0;
    char cbuffer[STRING_NUM] = {0};

    while(1)
    {
        Sleep(1);

        std::string temp;
        if(g_queue.pop(temp))
            log::printf("mytest.txt", temp.c_str());

		if( WaitForSingleObject( g_hStopEvent, 0 ) == WAIT_OBJECT_0 )
			break;
    }

    return 0;
}

void Test1()
{
     char cbuffer[STRING_NUM] = {0};
    for(int n = 0; n < 100; ++n)
    {
        sprintf_s(cbuffer, STRING_NUM, "queue push %d", n);
        g_queue.push( cbuffer);
    }

    HANDLE hThread[THREAD_COUNT];
    hThread[0] = CreateThread(NULL, 0, ::WriteLogFile, NULL, 0, NULL);
   
    char buff[STRING_NUM];
    while(1)
    {
        scanf_s("%s", buff, STRING_NUM);
        if(!strcmp(buff, "quit"))
            break;

        for(int n = 0; n < 10; ++n)
            g_queue.push(buff);
    }
    SetEvent( g_hStopEvent);

    WaitForMultipleObjects(THREAD_COUNT, hThread, true, INFINITE);

    for(int n = 0; n < THREAD_COUNT; ++n)
         CloseHandle(hThread[n]);

    CloseHandle( g_hStopEvent );
    printf("Test quit !!!!!!\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
    Test1();
    //log::printf("mytest.txt", "aaaaa %d \n", 100);
    return 0;
}