#include <Windows.h>
#include <WinInet.h>
#include <stdio.h>

#include <string>
#include <atlbase.h>
#include <atlconv.h>


#pragma comment (lib, "Wininet.lib")
#pragma comment (lib, "urlmon.lib")

bool PostHttp()
{
    HINTERNET hInternet = InternetOpen("HTTPTEST", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(hInternet == NULL)
        return false;;

    HINTERNET hInternetConnect = InternetConnect(hInternet, "localhost", 8881, "", "", INTERNET_SERVICE_HTTP, 0, 0);
    if(hInternetConnect == NULL)
    {
        InternetCloseHandle(hInternetConnect);
        return false;
    }

    HINTERNET hOpenRequest = HttpOpenRequest( hInternetConnect, "POST", "/login.jsp", HTTP_VERSION, "", 
                                              NULL,
                                              INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION |INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, // �̰� ��
                                              0);

    //�̺κп� internalSetoption�� �ش�, Flag�� �ٲ� ���鼭 ���� �غ���. 
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

    char szPostData[2048] = {0};
    lstrcpy(szPostData, "user_name=���ϳ�&user_id=hong&user_address=korea");
    std::string strPostData = CW2A(CT2W(szPostData), CP_UTF8);
    int postDataLength = strPostData.length();

    // post header
    char szLen[MAX_PATH] = {0};
    char szHeader[2048] = {0};

    wsprintf(szLen, "%d", postDataLength);
    strcat(szHeader, "Accept: text/*\r\n");
    strcat(szHeader, "User-Agent: Mozilla/4.0 (compatible; MSIE 5.0;* Windows NT)\r\n");
    strcat(szHeader, "Content-type: application/x-www-form-urlencoded\r\n");
    strcat(szHeader, "Content-length: ");
    strcat(szHeader, szLen);
    strcat(szHeader, "\r\n\n");


    BOOL bHeaderRequest = HttpAddRequestHeaders(hOpenRequest, szHeader, -1L, HTTP_ADDREQ_FLAG_ADD);
    if(!bHeaderRequest)
    {
        DWORD errId = GetLastError();  //error code 87
        printf("Fail HttpAddRequestHeaders %d \n", errId);
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

            //MessageBox(NULL, CW2T(CA2W(szBuf, CP_UTF8)), "post test", MB_OK);
            printf(CW2T(CA2W(szBuf, CP_UTF8)));
        }
        else
	    {
             DWORD errId = GetLastError();  //error code 12029
             printf("Fail HttpSendRequest %d \n", errId);
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