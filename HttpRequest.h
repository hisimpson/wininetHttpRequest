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
        return (m_httpSession != 0L);
    }
    bool InternetReadFile();
    
private:
    bool OpenInternet();
    bool OpenConnect();
    bool OpenRequest();
    bool SetInternetOption();
    bool SendPostHeader();
    bool SendPostData();
    void SetData(TCHAR* szData);

private:
    HINTERNET m_hSession;
    HINTERNET m_httpSession;
    HINTERNET m_hHttpFile ;
    std::string m_strData;

    std::basic_string<TCHAR> m_url;
    int m_port;
};


#endif