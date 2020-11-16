#ifndef _ASYNC_HTTP
#define _ASYNC_HTTP
#include <string>

class AsyncHttp;

typedef struct _InetContext
{
    AsyncHttp *obj;
    DWORD dwContext;
	DWORD error;
} InetContext;


class AsyncHttp
{
public:
    enum 
	{
        CONTEXT_CONNECT,
        CONTEXT_REQUESTHANDLE
    };
public:
    AsyncHttp();
    ~AsyncHttp();

    bool Open(TCHAR* url, int port = INTERNET_SERVICE_HTTP);
    bool Close();
    bool RequestPost(TCHAR* data);
    bool IsValidSession()
    {
        return (m_httpSession != 0L);
    }
    bool InternetReadFile();
    
private:
    bool OpenInternet();
    bool OpenConnect();
    bool OpenRequest();
    bool SetInternetOption();
    bool SendPostData();
    void SetData(TCHAR* szData);
	unsigned long ReadData( PBYTE lpBuffer, DWORD dwSize);

    static void WINAPI CallbackFunction( HINTERNET hInternet, DWORD dwContext, DWORD dwInetStatus, LPVOID lpStatusInfo, DWORD dwStatusInfoLength);

private:
	HANDLE hConnectEvent, hRequestOpenEvent, hRequestCompleteEvent;
    HINTERNET m_hInternetSession, m_httpSession, m_hHttpFile ;
    std::string m_strData;

	InetContext context;
    std::basic_string<TCHAR> m_url;
    int m_port;
};


#endif