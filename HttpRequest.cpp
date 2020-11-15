#include <windows.h>
#include <winInet.h>
#include <stdio.h>

#include <string>
#include <atlbase.h>
#include <atlconv.h>

#pragma comment (lib, "Wininet.lib")
#pragma comment (lib, "urlmon.lib")

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
    char szLen[MAX_PATH] = {0};
    char szHeader[2048] = {0};

    wsprintf(szLen, _T("%d"), postDataLength);
    strcat(szHeader, _T("Accept: text/*\r\n"));
    strcat(szHeader, _T("User-Agent: Mozilla/4.0 (compatible; MSIE 5.0;* Windows NT)\r\n"));
    strcat(szHeader, _T("Content-type: application/x-www-form-urlencoded\r\n"));
    strcat(szHeader, _T("Content-length: "));
    strcat(szHeader, szLen);
    strcat(szHeader, _T("\r\n\n"));


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

void TestHttp()
{
    PostHttp();
}