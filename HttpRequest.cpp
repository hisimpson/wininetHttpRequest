//�ҽ� ��ũ : https://otland.net/threads/c-asynchronous-wininet-client.260001/
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

HttpRequest::HttpRequest() : m_hInternetSession(0), m_httpSession(0), m_hHttpFile (0)
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

    if(m_hInternetSession)
        bRet = InternetCloseHandle(m_hInternetSession);
    if(bRet)
        m_hInternetSession = 0;
    return (bRet != FALSE);
}

bool HttpRequest::OpenInternet()
{
    if(m_hInternetSession)
    {
        BOOL bRet = InternetCloseHandle(m_hInternetSession);
        if(bRet)
            m_hInternetSession = 0;
        else
            return false;
    }

    HINTERNET hInternet = InternetOpen(_T("HTTPPOST"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(hInternet == NULL)
        return false;;

    m_hInternetSession = hInternet;
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
    
    HINTERNET hInternetConnect = InternetConnect(m_hInternetSession, m_url.c_str(), m_port, _T(""), _T(""), INTERNET_SERVICE_HTTP, 0, 0);
    if(hInternetConnect == NULL)
    {
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

    //localhost���� �׽�Ʈ�� HttpSendRequest �Լ� ������ �ߵ�
    DWORD dwFlags = INTERNET_FLAG_RELOAD;
    //DWORD dwFlags = INTERNET_LAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION |INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;
    //DWORD dwFlags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;

    //localhost���� �׽�Ʈ�� HttpSendRequest �Լ� ������ �ȵ�. INTERNET_FLAG_SECURE �÷��׿� �����ִ�.
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
    //�̺κп� internalSetoption�� �ش�, Flag�� �ٲ� ���鼭 ���� �غ���. 
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

bool HttpRequest::SendPostData()
{
    std::string& strData = m_strData;
    TCHAR* szHeader = _T("Content-Type: application/x-www-form-urlencoded");
    BOOL bRet = HttpSendRequest(m_hHttpFile ,
                                        szHeader,
                                        lstrlen(szHeader),
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

    //������ ���
    //MessageBox(NULL, CW2T(CA2W(szBuf, CP_UTF8)), _T("post test"), MB_OK);
    printf(CW2A(CA2W(szBuf, CP_UTF8)));
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------

bool TestPostHttp()
{
    HttpRequest httpPost;
    httpPost.Open( _T("localhost"), 8881);
    httpPost.RequestPost(_T("user_name=ö��&user_id=hong&user_address=korea"));
	//httpPost.InternetReadFile();
    httpPost.RequestPost(_T("user_name=����&user_id=hong&user_address=korea"));
    httpPost.RequestPost(_T("user_name=ȫ�浿&user_id=hong&user_address=korea"));
    httpPost.Close();

    return true;
}


/*
����

InternetReadFile �Լ��� ���� �ٿ�ε�
https://silvermask.tistory.com/165

WinINet HttpSendRequest�� �̿��Ͽ� HTTP/HTTPS�� POST�ϴ¹��  *** '&' �߰��� ���� ��ȣ�� �ִ´�.
https://lunaticlina.tistory.com/25

C++ http Ŭ���̾�Ʈ ��û�ϱ� WinHttp
https://jacking75.github.io/HttpClient_WinHttp/

HttpSendRequest 12029 ���� �ذ�
https://stackoverflow.com/questions/30319288/using-internetconnect-with-ip-address-fails-error-12029

ASCII, UNICODE, UTF8 ��ȣ ��ȯ, �ѱ۱���
https://m.blog.naver.com/chodadoo/220444225049
*/



/*
WinInet �񵿱� ���
�˻� Ű���� : wininet async example

InternetOpen INTERNET_FLAG_ASYNC InternetSetStatusCallback                          
https://zerry82.tistory.com/228


WinInet API ����
https://phiru.tistory.com/50

WinHttp �̾�ޱ� (�ٿ�ε�)
https://yamoe.tistory.com/219

���� �̸�¢��
http://speed.eik.bme.hu/help/html/Web_Programming_Unleashed/ch17.htm#InitializingaWinInetSession

sample
https://github.com/Codeh4ck/AsyncWinInet
https://docs.microsoft.com/en-us/windows/win32/wininet/asynchronous-example-application

����
Using WinInet HTTP functions in Full Asynchronous Mode
https://www.codeproject.com/Articles/822/Using-WinInet-HTTP-functions-in-Full-Asynchronous
https://docs.microsoft.com/en-us/windows/win32/wininet/asynchronous-operation

*/


/*
http "Content-Length"�� telnet���� ����
https://b.pungjoo.com/entry/Transfer-Encoding-chunked-VS-Content-Length?category=221149
*/