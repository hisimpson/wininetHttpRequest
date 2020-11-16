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

HttpRequest::HttpRequest() : m_hSession(0), m_httpSession(0), m_hHttpFile (0)
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

bool HttpRequest::Close()
{
    BOOL bRet = TRUE;
    if(m_hHttpFile )
        bRet = InternetCloseHandle(m_hHttpFile );
    if(bRet)
        m_hHttpFile  = 0;

    if(m_httpSession)
        bRet = InternetCloseHandle(m_httpSession);
    if(bRet)
        m_httpSession = 0;

    if(m_hSession)
        bRet = InternetCloseHandle(m_hSession);
    if(bRet)
        m_hSession = 0;
    return (bRet != FALSE);
}

bool HttpRequest::OpenInternet()
{
    if(m_hSession)
    {
        BOOL bRet = InternetCloseHandle(m_hSession);
        if(bRet)
            m_hSession = 0;
        else
            return false;
    }

    HINTERNET hInternet = InternetOpen(_T("HTTPPOST"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(hInternet == NULL)
        return false;;

    m_hSession = hInternet;
    return true;
}

bool HttpRequest::OpenConnect()
{
    if(m_httpSession)
    {
        BOOL bRet = InternetCloseHandle(m_httpSession);
        if(bRet)
            m_httpSession = 0;
        else
            return false;
    }
    
    HINTERNET hInternetConnect = InternetConnect(m_hSession, m_url.c_str(), m_port, _T(""), _T(""), INTERNET_SERVICE_HTTP, 0, 0);
    if(hInternetConnect == NULL)
    {
        InternetCloseHandle(hInternetConnect);
        return false;
    }

    m_httpSession = hInternetConnect;
    return true;
}

bool HttpRequest::OpenRequest()
{
    if(m_hHttpFile )
    {
        BOOL bRet = InternetCloseHandle(m_hHttpFile );
        if(bRet)
            m_hHttpFile  = 0;
        else
            return false;
    }

    //localhost에서 테스트시 HttpSendRequest 함수 실행이 잘됨
    DWORD dwFlags = INTERNET_FLAG_RELOAD;
    //DWORD dwFlags = INTERNET_LAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION |INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;
    //DWORD dwFlags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;

    //localhost에서 테스트시 HttpSendRequest 함수 실행이 안됨. INTERNET_FLAG_SECURE 플래그와 연관있다.
    //DWORD dwFlags = INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

    HINTERNET hOpenRequest = HttpOpenRequest( m_httpSession, _T("POST"), _T("/login.jsp"), HTTP_VERSION, _T(""), 
                                              NULL,
                                             dwFlags,
                                             0);

    if(hOpenRequest == NULL)
        return false;;

    m_hHttpFile  = hOpenRequest;
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
 
    BOOL bRet = InternetSetOption(m_hHttpFile , INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
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

    BOOL bRet = HttpAddRequestHeaders(m_hHttpFile , szHeader, -1L, HTTP_ADDREQ_FLAG_ADD);
    return (bRet != FALSE);
}

bool HttpRequest::SendPostData()
{
    std::string& strData = m_strData;

    BOOL bRet = HttpSendRequest(m_hHttpFile ,
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
    BOOL bRead = ::InternetReadFile( m_hHttpFile ,
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

bool TestPostHttp()
{
    HttpRequest httpPost;
    httpPost.Open( _T("localhost"), 8881);
    httpPost.RequestPost(_T("user_name=철수&user_id=hong&user_address=korea"));
    httpPost.RequestPost(_T("user_name=영희&user_id=hong&user_address=korea"));
    httpPost.RequestPost(_T("user_name=홍길동&user_id=hong&user_address=korea"));
    httpPost.Close();

    return true;
}


/*
참조

InternetReadFile 함수로 파일 다운로드
https://silvermask.tistory.com/165

WinINet HttpSendRequest을 이용하여 HTTP/HTTPS로 POST하는방법  *** '&' 중간에 구분 기호로 넣는다.
https://lunaticlina.tistory.com/25

C++ http 클라이언트 요청하기 WinHttp
https://jacking75.github.io/HttpClient_WinHttp/

HttpSendRequest 12029 에러 해결
https://stackoverflow.com/questions/30319288/using-internetconnect-with-ip-address-fails-error-12029

ASCII, UNICODE, UTF8 상호 변환, 한글깨짐
https://m.blog.naver.com/chodadoo/220444225049
*/

