#ifndef _HTTP_REQUEST
class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();

    bool Open();
    bool Close();
    void SetData(TCHAR* szData);

private:
    bool OpenInternet();
    bool OpenConnect();
    bool OpenRequest();
    bool SetInternetOption();
    bool SendPostHeader();
    bool SendPostData();
    bool InternetReadFile();

private:
    HINTERNET m_hAgent;
    HINTERNET m_hSession;
    HINTERNET m_hOpenRequest;
    std::string m_strData;
};


#endif