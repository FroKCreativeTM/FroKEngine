// FroKEngine.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "FroKEngine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // COM 객체를 사용하기 위해서는 먼저 초기화를 해야한다.
    HRESULT hr = CoInitialize(nullptr);

    if (FAILED(hr))
    {
        ErrorLogger::Log("Error CoInitialize()");
        return -1;
    }

    // return value
    int nRev;
    try 
    {
        FroKEngine app;

        if (!app.Init(hInstance))
        {
            return 0;
        }

        // Reture Value를 받는다.
        nRev = app.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }

    return nRev;
}

