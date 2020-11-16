#include <windows.h>
#include <winInet.h>
#include <stdio.h>

#include <string>
#include <atlbase.h>
#include <atlconv.h>

#include "AsyncHttp.h"

#pragma comment (lib, "Wininet.lib")
#pragma comment (lib, "urlmon.lib")

#define Return_Error(msg)  {   printf(msg); return false; }

#define TIME_OUT (20*1000)

AsyncHttp::AsyncHttp() : m_hInternetSession(NULL), m_httpSession(NULL), m_hHttpFile (NULL), m_port(0)
, hConnectEvent(NULL), hRequestOpenEvent(NULL), hRequestCompleteEvent(NULL)
{
	context.obj = NULL;
	context.dwContext = 0;
	context.error = 0;
}

AsyncHttp::~AsyncHttp()
{
}

bool AsyncHttp::Open(TCHAR* url, int port)
{
    m_url = url;
    m_port = port;

	if(!this->hConnectEvent)
	{
		this->hConnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		this->hRequestOpenEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		this->hRequestCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	ResetEvent(this->hConnectEvent);
	ResetEvent(this->hRequestOpenEvent);
	ResetEvent(this->hRequestCompleteEvent);

    if(!OpenInternet())
        Return_Error("FAILED: error in OpenInternet\n");

    if(!OpenConnect())
        Return_Error("FAILED: error in OpenConnect\n");

    return true;
}

bool AsyncHttp::RequestPost(TCHAR* data)
{
    if(!IsValidSession())
        return false;
    SetData(data);

    if(!OpenRequest())
        Return_Error("FAILED: error in OpenRequest\n");

    //if(!SetInternetOption())
    //    Return_Error("FAILED: error in SetInternetOption\n");

    if(!SendPostData())
        Return_Error("FAILED: error in SendPostData\n");

    return true;
}

bool AsyncHttp::Close()
{
    if (this->hConnectEvent)
        CloseHandle(this->hConnectEvent);

    if (this->hRequestOpenEvent)
        CloseHandle(this->hRequestOpenEvent);

    if (this->hRequestCompleteEvent)
        CloseHandle(this->hRequestCompleteEvent);

    BOOL bRet = TRUE;
    if(m_hHttpFile )
        bRet = InternetCloseHandle(m_hHttpFile );
    if(bRet)
        m_hHttpFile  = 0;

    if(m_httpSession)
        bRet = InternetCloseHandle(m_httpSession);
    if(bRet)
        m_httpSession = 0;

    if(m_hInternetSession)
        bRet = InternetCloseHandle(m_hInternetSession);
    if(bRet)
        m_hInternetSession = 0;
    return (bRet != FALSE);
}

bool AsyncHttp::OpenInternet()
{
    if(m_hInternetSession)
    {
        BOOL bRet = InternetCloseHandle(m_hInternetSession);
        if(bRet)
            m_hInternetSession = 0;
        else
            return false;
    }

    HINTERNET hInternet = InternetOpen(_T("HTTPPOST"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
    if(hInternet == NULL)
        return false;;

    m_hInternetSession = hInternet;

    if (InternetSetStatusCallback(m_hInternetSession, (INTERNET_STATUS_CALLBACK)&CallbackFunction))
    {
        return false;
    }

    this->context.dwContext = AsyncHttp::CONTEXT_CONNECT;
    this->context.obj = this;

    return true;
}


#pragma warning( push )
#pragma warning( disable : 4311)

bool AsyncHttp::OpenConnect()
{
	DWORD dwTimeout = TIME_OUT;
    if(m_httpSession)
    {
        BOOL bRet = InternetCloseHandle(m_httpSession);
        if(bRet)
            m_httpSession = 0;
        else
            return false;
    }
    
	m_httpSession = InternetConnect(m_hInternetSession, m_url.c_str(), m_port, _T(""), _T(""), INTERNET_SERVICE_HTTP, 
		INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE , reinterpret_cast<DWORD>(&this->context));
    if(m_httpSession == NULL)
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (WaitForSingleObject(this->hConnectEvent, dwTimeout))
			{
				printf("InternetConnect error: time out\n");
                return false;
			}
        }
    }

    if(m_httpSession == NULL)
		return false;
    return true;
}

bool AsyncHttp::OpenRequest()
{
	DWORD dwTimeout = TIME_OUT;
    if(m_hHttpFile )
    {
        BOOL bRet = InternetCloseHandle(m_hHttpFile );
        if(bRet)
            m_hHttpFile  = 0;
        else
            return false;
    }

	this->context.dwContext = AsyncHttp::CONTEXT_REQUESTHANDLE;
	this->context.obj = this;

    //localhost에서 테스트시 HttpSendRequest 함수 실행이 잘됨
    DWORD dwFlags = INTERNET_FLAG_RELOAD;
	//DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE  
	//	| INTERNET_FLAG_FORMS_SUBMIT | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP;
    //DWORD dwFlags = INTERNET_LAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION |INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;
    //DWORD dwFlags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;

    //localhost에서 테스트시 HttpSendRequest 함수 실행이 안됨. INTERNET_FLAG_SECURE 플래그와 연관있다.
    //DWORD dwFlags = INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

    m_hHttpFile = HttpOpenRequest( m_httpSession, _T("POST"), _T("/login.jsp"), HTTP_VERSION, _T(""), 
                                              NULL,
                                             dwFlags,
                                             reinterpret_cast<DWORD>(&this->context));

    if(m_hHttpFile == NULL)
	{
        printf("HttpOpenRequest error %d\n", GetLastError());
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if(WaitForSingleObject(this->hRequestOpenEvent, dwTimeout))
                return false;
        };
	}

    if(m_hHttpFile == NULL)
		return false;
    return true;
}

#pragma warning( pop )

bool AsyncHttp::SetInternetOption()
{
    //이부분에 internalSetoption을 준다, Flag를 바꿔 가면서 실행 해본다. 
    DWORD dwFlags = 0 ;
    dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA   |
           SECURITY_FLAG_IGNORE_REVOCATION       |
           SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP   |
           SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS |
           SECURITY_FLAG_IGNORE_CERT_DATE_INVALID   |
           SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
 
    BOOL bRet = InternetSetOption(m_hHttpFile , INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
    return (bRet != FALSE);
}

void AsyncHttp::SetData(TCHAR* szData)
{
    std::string& strData = m_strData;
    TCHAR szPostData[2048] = {0};
    lstrcpy(szPostData, szData);
    strData = CW2A(CT2W(szPostData), CP_UTF8);
}

bool AsyncHttp::SendPostData()
{
	DWORD dwTimeout = TIME_OUT;
    std::string& strData = m_strData;
    TCHAR* szHeader = _T("Content-Type: application/x-www-form-urlencoded");
    BOOL bRet = HttpSendRequest(m_hHttpFile ,
                                        szHeader,
                                        lstrlen(szHeader),
                                        (LPVOID)strData.c_str(),
                                        (DWORD)strData.length());

    if(!bRet)
    {
        if(GetLastError() == ERROR_IO_PENDING)
        {
            if(WaitForSingleObject(this->hRequestCompleteEvent, dwTimeout) == WAIT_TIMEOUT)
            {
                return false;
            }
        }
    }

	if(context.error)
		return false;
	return true;
}

bool AsyncHttp::InternetReadFile()
{
	std::string szResponse;
    char szBuf[1024];
    int iLength;

    while ((iLength = ReadData(reinterpret_cast<PBYTE>(szBuf), 1024)) > 0)
    {
        szBuf[iLength] = 0;               
        szResponse += szBuf;
    }

    //디버깅용 출력
    //MessageBox(NULL, CW2T(CA2W(szResponse.c_str(), CP_UTF8)), _T("post test"), MB_OK);
    printf(CW2A(CA2W(szResponse.c_str(), CP_UTF8)));
	return (!szResponse.empty());
}

#pragma warning( push )
#pragma warning( disable : 4311)

unsigned long AsyncHttp::ReadData(PBYTE lpBuffer, DWORD dwSize)
{
	DWORD dwTimeout = TIME_OUT;
    INTERNET_BUFFERS inetBuffers;
    memset(&inetBuffers, 0, sizeof(inetBuffers));

    inetBuffers.dwStructSize = sizeof(inetBuffers);
    inetBuffers.lpvBuffer = lpBuffer;
    inetBuffers.dwBufferLength = dwSize - 1;

    this->context.dwContext = AsyncHttp::CONTEXT_REQUESTHANDLE;
    this->context.obj = this;

    if (!InternetReadFileEx(this->m_hHttpFile, &inetBuffers, 0, reinterpret_cast<DWORD>(&this->context)))
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            if (WaitForSingleObject(this->hRequestCompleteEvent, dwTimeout) == WAIT_TIMEOUT)
            {
                return 0;
            }
        }
    }

    return inetBuffers.dwBufferLength;
}
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4312)

void WINAPI AsyncHttp::CallbackFunction(
    HINTERNET hInternet,
    DWORD dwContext,
    DWORD dwInetStatus,
    LPVOID lpStatusInfo,
    DWORD dwStatusInfoLength)
{
#if 1
    InetContext *ctx = reinterpret_cast<InetContext*>(dwContext);

    switch (ctx->dwContext)
    {
    case AsyncHttp::CONTEXT_CONNECT:
        if (dwInetStatus == INTERNET_STATUS_HANDLE_CREATED)
        {
            INTERNET_ASYNC_RESULT *inetResult = reinterpret_cast<INTERNET_ASYNC_RESULT*>(lpStatusInfo);
            ctx->obj->m_httpSession = reinterpret_cast<HINTERNET>(inetResult->dwResult);
            SetEvent(ctx->obj->hConnectEvent);
        }
        break;
    case AsyncHttp::CONTEXT_REQUESTHANDLE:
        switch (dwInetStatus)
        {
        case INTERNET_STATUS_HANDLE_CREATED:
            {
                INTERNET_ASYNC_RESULT *inetResult1 = reinterpret_cast<INTERNET_ASYNC_RESULT*>(lpStatusInfo);
                ctx->obj->m_hHttpFile = reinterpret_cast<HINTERNET>(inetResult1->dwResult);
                SetEvent(ctx->obj->hRequestOpenEvent);
            }
            break;
        case INTERNET_STATUS_REQUEST_COMPLETE:
			{
				INTERNET_ASYNC_RESULT *pAsyncRes = (INTERNET_ASYNC_RESULT *)lpStatusInfo;
				ctx->error = pAsyncRes->dwError;
			}
            SetEvent(ctx->obj->hRequestCompleteEvent);
            break;
        }
        break;
    }
#endif
}

#pragma warning( pop )


//----------------------------------------------------------------------------------------------------------------------------------

bool TestAsyncHttp()
{
    AsyncHttp httpPost;
    httpPost.Open( _T("localhost"), 8881);
    httpPost.RequestPost(_T("user_name=철수&user_id=hong&user_address=korea"));
	//httpPost.InternetReadFile();
    //httpPost.RequestPost(_T("user_name=영희&user_id=hong&user_address=korea"));
    //httpPost.RequestPost(_T("user_name=홍길동&user_id=hong&user_address=korea"));
    httpPost.Close();

    return true;
}


/*
[C++] Asynchronous WinInet client
https://otland.net/threads/c-asynchronous-wininet-client.260001/

InternetSetStatusCallback 자세히
http://egloos.zum.com/dudtnrttl/v/993126
*/