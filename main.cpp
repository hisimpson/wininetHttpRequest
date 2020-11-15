#include <fstream>
#include <stdarg.h>
#include <ctime>
 
#include <tchar.h>
using namespace std;
 
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


int _tmain(int argc, _TCHAR* argv[])
{
    log::printf("mytest.txt", "aaaaa %d \n", 100);
    return 0;
}