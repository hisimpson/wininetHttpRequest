#include <windows.h>
#include <winInet.h>
#include <stdio.h>

#include <string>
#include <atlbase.h>
#include <atlconv.h>

#include "HttpRequest.h"

#pragma comment (lib, "Wininet.lib")
#pragma comment (lib, "urlmon.lib")

#define Return_Error(msg)  {   printf(msg); return false; }

HttpRequest::HttpRequest() : m_hAgent(0), m_hSession(0), m_hOpenRequest(0)
{

}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::Open(TCHAR* url, int port)
{
    m_url = url;
    m_port = port;

    if(!OpenInternet())
        Return_Error("FAILED: error in OpenInternet\n");

    if(!OpenConnect())
        Return_Error("FAILED: error in OpenConnect\n");

    return true;
}

bool HttpRequest::RequestPost(TCHAR* data)
{
    if(!IsValidSession())
        return false;
    SetData(data);

    if(!OpenRequest())
        Return_Error("FAILED: error in OpenRequest\n");

    if(!SetInternetOption())
        Return_Error("FAILED: error in SetInternetOption\n");

    if(!SendPostHeader())
        Return_Error("FAILED: error in SendPostHeader\n");

    if(!SendPostData())
        Return_Error("FAILED: error in SendPostData\n");

    return true;
}

#if  0
bool HttpRequest::Open()
{
    /*
    if(!OpenInternet())
        Return_Error("FAILED: error in OpenInternet\n");

    if(!OpenConnect())
        Return_Error("FAILED: error in OpenConnect\n");

    if(!OpenRequest())
        Return_Error("FAILED: error in OpenRequest\n");

    if(!SetInternetOption())
        Return_Error("FAILED: error in SetInternetOption\n");

    if(!SendPostHeader())
        Return_Error("FAILED: error in SendPostHeader\n");

    if(!SendPostData())
        Return_Error("FAILED: error in SendPostData\n");

    if(!InternetReadFile())
        Return_Error("FAILED: error in InternetReadFile\n");
    */

    if(!OpenInternet())
        Return_Error("FAILED: error in OpenInternet\n");

    if(!OpenConnect())
        Return_Error("FAILED: error in OpenConnect\n");

    if(!OpenRequest())
        Return_Error("FAILED: error in OpenRequest\n");

    if(!SetInternetOption())
        Return_Error("FAILED: error in SetInternetOption\n");

    SetData(_T("user_name=홍test&user_id=hong&user_address=korea"));
    if(!SendPostHeader())
        Return_Error("FAILED: error in SendPostHeader\n");

    if(!SendPostData())
        Return_Error("FAILED: error in SendPostData\n");

    //DDD
    BOOL retOpenRequestRet = InternetCloseHandle(m_hOpenRequest);
    if(retOpenRequestRet)
        m_hOpenRequest = 0;
    BOOL retConnect = InternetCloseHandle(m_hSession);
    if(retConnect)
        m_hSession = 0;

    if(!OpenConnect())
        Return_Error("FAILED: error in OpenConnect\n");

    if(!OpenRequest())
        Return_Error("FAILED: error in OpenRequest\n");

    if(!SetInternetOption())
        Return_Error("FAILED: error in SetInternetOption\n");

    SetData(_T("user_name=홍길동&user_id=hong&user_address=korea"));
    if(!SendPostHeader())
        Return_Error("FAILED: error in SendPostHeader\n");

    if(!SendPostData())
        Return_Error("FAILED: error in SendPostData\n");
    return true;
}
#endif
bool HttpRequest::Close()
{
    BOOL bRet = TRUE;
    if(m_hOpenRequest)
        bRet = InternetCloseHandle(m_hOpenRequest);
    if(bRet)
        m_hOpenRequest = 0;

    if(m_hSession)
        bRet = InternetCloseHandle(m_hSession);
    if(bRet)
        m_hSession = 0;

    if(m_hAgent)
        bRet = InternetCloseHandle(m_hAgent);
    if(bRet)
        m_hAgent = 0;
    return (bRet != FALSE);
}

bool HttpRequest::OpenInternet()
{
    if(m_hAgent)
    {
        BOOL bRet = InternetCloseHandle(m_hAgent);
        if(bRet)
            m_hAgent = 0;
        else
            return false;
    }

    HINTERNET hInternet = InternetOpen(_T("HTTPPOST"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(hInternet == NULL)
        return false;;

    m_hAgent = hInternet;
    return true;
}

bool HttpRequest::OpenConnect()
{
    if(m_hSession)
    {
        BOOL bRet = InternetCloseHandle(m_hSession);
        if(bRet)
            m_hSession = 0;
        else
            return false;
    }
    
    HINTERNET hInternetConnect = InternetConnect(m_hAgent, m_url.c_str(), m_port, _T(""), _T(""), INTERNET_SERVICE_HTTP, 0, 0);
    if(hInternetConnect == NULL)
    {
        InternetCloseHandle(hInternetConnect);
        return false;
    }

    m_hSession = hInternetConnect;
    return true;
}

bool HttpRequest::OpenRequest()
{
    if(m_hOpenRequest)
    {
        BOOL bRet = InternetCloseHandle(m_hOpenRequest);
        if(bRet)
            m_hOpenRequest = 0;
        else
            return false;
    }

    //localhost에서 테스트시 HttpSendRequest 함수 실행이 잘됨
    DWORD dwFlags = INTERNET_FLAG_RELOAD;
    //DWORD dwFlags = INTERNET_LAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION |INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;
    //DWORD dwFlags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;

    //localhost에서 테스트시 HttpSendRequest 함수 실행이 안됨. INTERNET_FLAG_SECURE 플래그와 연관있다.
    //DWORD dwFlags = INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

    HINTERNET hOpenRequest = HttpOpenRequest( m_hSession, _T("POST"), _T("/login.jsp"), HTTP_VERSION, _T(""), 
                                              NULL,
                                             INTERNET_FLAG_RELOAD,  // 이것 됨
                                             //INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION |INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, // 이것 됨
                                             //INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, //이것 HttpSendRequest됨
                                             //INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, //이것안됨
                                             0);

    if(hOpenRequest == NULL)
        return false;;

    m_hOpenRequest = hOpenRequest;
    return true;
}

bool HttpRequest::SetInternetOption()
{
    //이부분에 internalSetoption을 준다, Flag를 바꿔 가면서 실행 해본다. 
    DWORD dwFlags = 0 ;
    dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA   |
           SECURITY_FLAG_IGNORE_REVOCATION       |
           SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP   |
           SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS |
           SECURITY_FLAG_IGNORE_CERT_DATE_INVALID   |
           SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
 
    BOOL bRet = InternetSetOption(m_hOpenRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
    return (bRet != FALSE);
}

void HttpRequest::SetData(TCHAR* szData)
{
    std::string& strData = m_strData;
    TCHAR szPostData[2048] = {0};
    lstrcpy(szPostData, szData);
    strData = CW2A(CT2W(szPostData), CP_UTF8);
}

bool HttpRequest::SendPostHeader()
{
    std::string& strData = m_strData;
    /*
    std::string& strData = m_strData;
    TCHAR szPostData[2048] = {0};
    lstrcpy(szPostData, _T("user_name=둘하나&user_id=hong&user_address=korea"));
    strData = CW2A(CT2W(szPostData), CP_UTF8);
    */
    int postDataLength = (int)strData.length();

    // post header
    TCHAR szLen[MAX_PATH] = {0};
    TCHAR szHeader[2048] = {0};

    wsprintf(szLen, _T("%d"), postDataLength);
    lstrcat(szHeader, _T("Accept: text/*\r\n"));
    lstrcat(szHeader, _T("User-Agent: Mozilla/4.0 (compatible; MSIE 5.0;* Windows NT)\r\n"));
    lstrcat(szHeader, _T("Content-type: application/x-www-form-urlencoded\r\n"));
    lstrcat(szHeader, _T("Content-length: "));
    lstrcat(szHeader, szLen);
    lstrcat(szHeader, _T("\r\n\n"));

    BOOL bRet = HttpAddRequestHeaders(m_hOpenRequest, szHeader, -1L, HTTP_ADDREQ_FLAG_ADD);
    return (bRet != FALSE);
}

bool HttpRequest::SendPostData()
{
    std::string& strData = m_strData;

    BOOL bRet = HttpSendRequest(m_hOpenRequest,
                                        NULL,
                                        0,
                                        (LPVOID)strData.c_str(),
                                        (DWORD)strData.length());
    return (bRet != FALSE);
}


bool HttpRequest::InternetReadFile()
{
    char szBuf[2048] = {0};
    DWORD dwSize = 0;
    BOOL bRead = ::InternetReadFile( m_hOpenRequest,
                                   szBuf,
                                   sizeof(szBuf),
                                   &dwSize);
    if(bRead == FALSE)
        return false;

    //디버깅용 출력
    //MessageBox(NULL, CW2T(CA2W(szBuf, CP_UTF8)), _T("post test"), MB_OK);
    printf(CW2A(CA2W(szBuf, CP_UTF8)));
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------




bool PostHttp()
{
    HINTERNET hInternet = InternetOpen(_T("HTTPTEST"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(hInternet == NULL)
        return false;;

    HINTERNET hInternetConnect = InternetConnect(hInternet, _T("localhost"), 8881, _T(""), _T(""), INTERNET_SERVICE_HTTP, 0, 0);
    if(hInternetConnect == NULL)
    {
        InternetCloseHandle(hInternetConnect);
        return false;
    }

    HINTERNET hOpenRequest = HttpOpenRequest( hInternetConnect, _T("POST"), _T("/login.jsp"), HTTP_VERSION, _T(""), 
                                              NULL,
                                             INTERNET_FLAG_RELOAD,  // 이것 됨
                                             //INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION |INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, // 이것 됨
                                             //INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, //이것 HttpSendRequest됨
                                             //INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, //이것안됨
                                             0);

    //이부분에 internalSetoption을 준다, Flag를 바꿔 가면서 실행 해본다. 
#if  1
    DWORD dwFlags = 0 ;
    dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA   |
           SECURITY_FLAG_IGNORE_REVOCATION       |
           SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP   |
           SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS |
           SECURITY_FLAG_IGNORE_CERT_DATE_INVALID   |
           SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
 
    InternetSetOption(hOpenRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
#endif

    TCHAR szPostData[2048] = {0};
    lstrcpy(szPostData, _T("user_name=둘하나&user_id=hong&user_address=korea"));
    std::string strPostData = CW2A(CT2W(szPostData), CP_UTF8);
    int postDataLength = (int)strPostData.length();

    // post header
    TCHAR szLen[MAX_PATH] = {0};
    TCHAR szHeader[2048] = {0};

    wsprintf(szLen, _T("%d"), postDataLength);
    lstrcat(szHeader, _T("Accept: text/*\r\n"));
    lstrcat(szHeader, _T("User-Agent: Mozilla/4.0 (compatible; MSIE 5.0;* Windows NT)\r\n"));
    lstrcat(szHeader, _T("Content-type: application/x-www-form-urlencoded\r\n"));
    lstrcat(szHeader, _T("Content-length: "));
    lstrcat(szHeader, szLen);
    lstrcat(szHeader, _T("\r\n\n"));


    BOOL bHeaderRequest = HttpAddRequestHeaders(hOpenRequest, szHeader, -1L, HTTP_ADDREQ_FLAG_ADD);
    if(!bHeaderRequest)
    {
        DWORD errId = GetLastError();  //error code 87
		printf("Failed: Error in HttpAddRequestHeaders %d \n", errId);
    }
    else
    {
        BOOL bSendRequest = HttpSendRequest(hOpenRequest,
                                            NULL,
                                            0,
                                            (LPVOID)strPostData.c_str(),
                                            (DWORD)postDataLength);

        if(bSendRequest)
        {
            char szBuf[2048] = {0};
            DWORD dwSize = 0;
            BOOL bRead = InternetReadFile(  hOpenRequest,
                                           szBuf,
                                           sizeof(szBuf),
                                           &dwSize);

            //MessageBox(NULL, CW2T(CA2W(szBuf, CP_UTF8)), _T("post test"), MB_OK);
            printf(CW2A(CA2W(szBuf, CP_UTF8)));
        }
        else
	    {
             DWORD errId = GetLastError();  //error code 12029, INTERNET_FLAG_SECURE 옵션이 들어가면 발생
			 printf("Failed: Error in HttpSendRequest %d \n", errId);
	    }
    }
    BOOL bRet = InternetCloseHandle(hOpenRequest);
    bRet = InternetCloseHandle(hInternetConnect);
    bRet = InternetCloseHandle(hInternet);

    return true;
}

bool TestPostHttp()
{
#if 1
    HttpRequest httpPost;
    httpPost.Open( _T("localhost"), 8881);
    httpPost.RequestPost(_T("user_name=철수&user_id=hong&user_address=korea"));
    httpPost.RequestPost(_T("user_name=영희&user_id=hong&user_address=korea"));
    httpPost.RequestPost(_T("user_name=똘이&user_id=hong&user_address=korea"));
    httpPost.Close();
#else
    PostHttp();
#endif
    return true;
}


//InternetReadFile 함수로 파일 다운로드
//https://silvermask.tistory.com/165