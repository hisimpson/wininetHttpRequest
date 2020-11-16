#ifndef _HTTP_REQUEST

#include <string>

class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();

    bool Open(TCHAR* url, int port = INTERNET_SERVICE_HTTP);
    bool Close();
    bool RequestPost(TCHAR* data);
    bool IsValidSession()
    {
        return (m_hSession != 0L);
    }
    
private:
    bool OpenInternet();
    bool OpenConnect();
    bool OpenRequest();
    bool SetInternetOption();
    bool SendPostHeader();
    bool SendPostData();
    bool InternetReadFile();
    void SetData(TCHAR* szData);

private:
    HINTERNET m_hAgent;
    HINTERNET m_hSession;
    HINTERNET m_hOpenRequest;
    std::string m_strData;

    std::basic_string<TCHAR> m_url;
    int m_port;
};


#endif