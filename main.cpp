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


namespace log
{
    void WriteFile(const char* pszFileName, const char* pszLog, ...);
}

template<typename QTYPE >  
class SafeQueue
{
private:
    std::queue<QTYPE> m_queue;
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

    void push(QTYPE node)
    {
        LOCK_QUEUE_MUTEX;

        m_queue.push(node);
        UNLOCK_QUEUE_MUTEX;
    }
        
    bool pop(QTYPE& out)
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

#define STRING_NUM  300
#define TEST1_COUNT 100
#define THREAD_COUNT 1
#define MAX_LOG_LEN   1024

enum LOG_THREAD_TYPE { LOG_FILE_THREAD };

enum LOG_STORE_TYPE { LOG_STORE_FILE };

struct LogQueueContainer
{
    std::string msg;
    std::string filename;
    LOG_STORE_TYPE storeType;
};

SafeQueue<LogQueueContainer> g_queue;

namespace log
{
    void printf(char* filename, const char* pszLog, ...);
    void Create();
    void Release();
}

namespace log
{    
    HANDLE hStopEvent = NULL;
    HANDLE hThread[THREAD_COUNT] = { NULL };

    namespace Internal
    {
        std::string GetTimeStamp()
        {
            struct tm ltm = {};
            std::time_t now = std::time(NULL) ;
            localtime_s(&ltm, &now);

            char cstr[256] = {0,};
            return std::strftime( cstr, sizeof(cstr), "%Y-%m%d-%H:%M:%S>", &ltm) ? cstr : "" ;
        } 

        void WriteFile(const char* pszFileName, std::string& strLog)
        {
            fstream _streamOut;
            _streamOut.open(pszFileName, ios::out | ios::app);
         
            _streamOut << strLog.c_str() << endl;
            _streamOut.close();
        }

        DWORD WINAPI WriteFileThread(void *lpVoid)
        {
            int b = 0;
            char cbuffer[STRING_NUM] = {0};

            while(1)
            {
                Sleep(1);

                LogQueueContainer temp;
                if(g_queue.pop(temp))
                {
                    if(temp.storeType == LOG_STORE_FILE)
                        log::Internal::WriteFile(temp.filename.c_str(), temp.msg);
                }
                else
                {
		            if( WaitForSingleObject( hStopEvent, 0 ) == WAIT_OBJECT_0 )
			            break;
                }
            }

            return 0;
        }
    }

    void printf(char* filename, const char* pszLog, ...)
    {    
        va_list argList;
        char cbuffer[MAX_LOG_LEN];
        va_start(argList, pszLog);

        vsnprintf_s(cbuffer, sizeof(char)*MAX_LOG_LEN, MAX_LOG_LEN, pszLog, argList);
        va_end(argList);

        std::string now = log::Internal::GetTimeStamp();
        now.append(cbuffer);

        LogQueueContainer temp;
        temp.filename = filename;
        temp.msg = now;
        temp.storeType = LOG_STORE_FILE;
        g_queue.push(temp);
    }

    void Create()
    {
        hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        hThread[LOG_FILE_THREAD] = CreateThread(NULL, 0, log::Internal::WriteFileThread, NULL, 0, NULL);
    }

    void Release()
    {
        SetEvent( hStopEvent);
        WaitForMultipleObjects(THREAD_COUNT, hThread, true, INFINITE);

        for(int n = 0; n < THREAD_COUNT; ++n)
            CloseHandle(hThread[n]);

        CloseHandle( hStopEvent );
    }   
}

void Test1()
{
    log::Create();

    for(int n = 0; n < 1000; n++)
        log::printf("test.txt", "log test %d", n);
    
    log::Release();
    printf("Test quit !!!!!!\n");
}


int _tmain(int argc, _TCHAR* argv[])
{
    Test1();
    //log::printf("mytest.txt", "aaaaa %d \n", 100);
    return 0;
}