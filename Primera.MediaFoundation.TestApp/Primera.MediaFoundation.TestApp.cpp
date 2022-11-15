// Primera.MediaFoundation.TestApp.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Primera.MediaFoundation.TestApp.h"

#include <new>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "Shlwapi")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HRESULT RunSampleGrabber(PCWSTR pszFileName);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        hr = MFStartup(MF_VERSION);
        if (SUCCEEDED(hr))
        {
            std::wstring path = L"C:/Filename.bmp";
            hr = RunSampleGrabber(path.c_str());
            MFShutdown();
        }
        CoUninitialize();
    }
    return 0;

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PRIMERAMEDIAFOUNDATIONTESTAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PRIMERAMEDIAFOUNDATIONTESTAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PRIMERAMEDIAFOUNDATIONTESTAPP));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PRIMERAMEDIAFOUNDATIONTESTAPP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            return (INT_PTR) TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR) TRUE;
            }
            break;
    }
    return (INT_PTR) FALSE;
}

/**/

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#define CHECK_HR(x) if (FAILED(x)) { goto done; }

// The class that implements the callback interface.
class SampleGrabberCB : public IMFSampleGrabberSinkCallback
{
    long m_cRef;

    SampleGrabberCB() : m_cRef(1) {}

public:
    static HRESULT CreateInstance(SampleGrabberCB** ppCB);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFClockStateSink methods
    STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
    STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
    STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);

    // IMFSampleGrabberSinkCallback methods
    STDMETHODIMP OnSetPresentationClock(IMFPresentationClock* pClock);
    STDMETHODIMP OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
                                 LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE* pSampleBuffer,
                                 DWORD dwSampleSize);
    STDMETHODIMP OnShutdown();
};

HRESULT CreateMediaSource(PCWSTR pszURL, IMFMediaSource** ppSource);
HRESULT CreateTopology(IMFMediaSource* pSource, IMFActivate* pSink, IMFTopology** ppTopo);
HRESULT RunSession(IMFMediaSession* pSession, IMFTopology* pTopology);

HRESULT RunSampleGrabber(PCWSTR pszFileName)
{
    IMFMediaSession* pSession = NULL;
    IMFMediaSource* pSource = NULL;
    SampleGrabberCB* pCallback = NULL;
    IMFActivate* pSinkActivate = NULL;
    IMFTopology* pTopology = NULL;
    IMFMediaType* pType = NULL;

    HRESULT hr = S_OK;

    // Configure the media type that the Sample Grabber will receive.
    // Setting the major and subtype is usually enough for the topology loader
    // to resolve the topology.

    CHECK_HR(hr = MFCreateMediaType(&pType));
    CHECK_HR(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
    CHECK_HR(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));

    // Create the sample grabber sink.
    CHECK_HR(hr = SampleGrabberCB::CreateInstance(&pCallback));
    CHECK_HR(hr = MFCreateSampleGrabberSinkActivate(pType, pCallback, &pSinkActivate));

    // To run as fast as possible, set this attribute (requires Windows 7):
    CHECK_HR(hr = pSinkActivate->SetUINT32(MF_SAMPLEGRABBERSINK_IGNORE_CLOCK, TRUE));

    // Create the Media Session.
    CHECK_HR(hr = MFCreateMediaSession(NULL, &pSession));

    // Create the media source.
    CHECK_HR(hr = CreateMediaSource(pszFileName, &pSource));

    // Create the topology.
    CHECK_HR(hr = CreateTopology(pSource, pSinkActivate, &pTopology));

    // Run the media session.
    CHECK_HR(hr = RunSession(pSession, pTopology));

done:
    // Clean up.
    if (pSource)
    {
        pSource->Shutdown();
    }
    if (pSession)
    {
        pSession->Shutdown();
    }

    SafeRelease(&pSession);
    SafeRelease(&pSource);
    SafeRelease(&pCallback);
    SafeRelease(&pSinkActivate);
    SafeRelease(&pTopology);
    SafeRelease(&pType);
    return hr;
}

// Create a media source from a URL.
HRESULT CreateMediaSource(PCWSTR pszURL, IMFMediaSource** ppSource)
{
    IMFSourceResolver* pSourceResolver = NULL;
    IUnknown* pSource = NULL;

    // Create the source resolver.
    HRESULT hr = S_OK;
    CHECK_HR(hr = MFCreateSourceResolver(&pSourceResolver));

    MF_OBJECT_TYPE ObjectType;
    CHECK_HR(hr = pSourceResolver->CreateObjectFromURL(pszURL,
                                                       MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, &pSource));

    hr = pSource->QueryInterface(IID_PPV_ARGS(ppSource));

done:
    SafeRelease(&pSourceResolver);
    SafeRelease(&pSource);
    return hr;
}

// Add a source node to a topology.
HRESULT AddSourceNode(
    IMFTopology* pTopology,           // Topology.
    IMFMediaSource* pSource,          // Media source.
    IMFPresentationDescriptor* pPD,   // Presentation descriptor.
    IMFStreamDescriptor* pSD,         // Stream descriptor.
    IMFTopologyNode** ppNode)         // Receives the node pointer.
{
    IMFTopologyNode* pNode = NULL;

    HRESULT hr = S_OK;
    CHECK_HR(hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode));
    CHECK_HR(hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource));
    CHECK_HR(hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD));
    CHECK_HR(hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD));
    CHECK_HR(hr = pTopology->AddNode(pNode));

    // Return the pointer to the caller.
    *ppNode = pNode;
    (*ppNode)->AddRef();

done:
    SafeRelease(&pNode);
    return hr;
}

// Add an output node to a topology.
HRESULT AddOutputNode(
    IMFTopology* pTopology,     // Topology.
    IMFActivate* pActivate,     // Media sink activation object.
    DWORD dwId,                 // Identifier of the stream sink.
    IMFTopologyNode** ppNode)   // Receives the node pointer.
{
    IMFTopologyNode* pNode = NULL;

    HRESULT hr = S_OK;
    CHECK_HR(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));
    CHECK_HR(hr = pNode->SetObject(pActivate));
    CHECK_HR(hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId));
    CHECK_HR(hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
    CHECK_HR(hr = pTopology->AddNode(pNode));

    // Return the pointer to the caller.
    *ppNode = pNode;
    (*ppNode)->AddRef();

done:
    SafeRelease(&pNode);
    return hr;
}

// Create the topology.
HRESULT CreateTopology(IMFMediaSource* pSource, IMFActivate* pSinkActivate, IMFTopology** ppTopo)
{
    IMFTopology* pTopology = NULL;
    IMFPresentationDescriptor* pPD = NULL;
    IMFStreamDescriptor* pSD = NULL;
    IMFMediaTypeHandler* pHandler = NULL;
    IMFTopologyNode* pNode1 = NULL;
    IMFTopologyNode* pNode2 = NULL;

    HRESULT hr = S_OK;
    DWORD cStreams = 0;

    CHECK_HR(hr = MFCreateTopology(&pTopology));
    CHECK_HR(hr = pSource->CreatePresentationDescriptor(&pPD));
    CHECK_HR(hr = pPD->GetStreamDescriptorCount(&cStreams));

    for (DWORD i = 0; i < cStreams; i++)
    {
        // In this example, we look for audio streams and connect them to the sink.

        BOOL fSelected = FALSE;
        GUID majorType;

        CHECK_HR(hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD));
        CHECK_HR(hr = pSD->GetMediaTypeHandler(&pHandler));
        CHECK_HR(hr = pHandler->GetMajorType(&majorType));

        if (majorType == MFMediaType_Audio && fSelected)
        {
            CHECK_HR(hr = AddSourceNode(pTopology, pSource, pPD, pSD, &pNode1));
            CHECK_HR(hr = AddOutputNode(pTopology, pSinkActivate, 0, &pNode2));
            CHECK_HR(hr = pNode1->ConnectOutput(0, pNode2, 0));
            break;
        }
        else
        {
            CHECK_HR(hr = pPD->DeselectStream(i));
        }
        SafeRelease(&pSD);
        SafeRelease(&pHandler);
    }

    *ppTopo = pTopology;
    (*ppTopo)->AddRef();

done:
    SafeRelease(&pTopology);
    SafeRelease(&pNode1);
    SafeRelease(&pNode2);
    SafeRelease(&pPD);
    SafeRelease(&pSD);
    SafeRelease(&pHandler);
    return hr;
}

HRESULT RunSession(IMFMediaSession* pSession, IMFTopology* pTopology)
{
    IMFMediaEvent* pEvent = NULL;

    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = S_OK;
    CHECK_HR(hr = pSession->SetTopology(0, pTopology));
    CHECK_HR(hr = pSession->Start(&GUID_NULL, &var));

    while (1)
    {
        HRESULT hrStatus = S_OK;
        MediaEventType met;

        CHECK_HR(hr = pSession->GetEvent(0, &pEvent));
        CHECK_HR(hr = pEvent->GetStatus(&hrStatus));
        CHECK_HR(hr = pEvent->GetType(&met));

        if (FAILED(hrStatus))
        {
            printf("Session error: 0x%x (event id: %d)\n", hrStatus, met);
            hr = hrStatus;
            goto done;
        }
        if (met == MESessionEnded)
        {
            break;
        }
        SafeRelease(&pEvent);
    }

done:
    SafeRelease(&pEvent);
    return hr;
}

// SampleGrabberCB implementation

// Create a new instance of the object.
HRESULT SampleGrabberCB::CreateInstance(SampleGrabberCB** ppCB)
{
    *ppCB = new (std::nothrow) SampleGrabberCB();

    if (ppCB == NULL)
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(SampleGrabberCB, IMFSampleGrabberSinkCallback),
        QITABENT(SampleGrabberCB, IMFClockStateSink),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) SampleGrabberCB::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) SampleGrabberCB::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

// IMFClockStateSink methods.

// In these example, the IMFClockStateSink methods do not perform any actions.
// You can use these methods to track the state of the sample grabber sink.

STDMETHODIMP SampleGrabberCB::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnClockStop(MFTIME hnsSystemTime)
{
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnClockPause(MFTIME hnsSystemTime)
{
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnClockRestart(MFTIME hnsSystemTime)
{
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
    return S_OK;
}

// IMFSampleGrabberSink methods.

STDMETHODIMP SampleGrabberCB::OnSetPresentationClock(IMFPresentationClock* pClock)
{
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
                                              LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE* pSampleBuffer,
                                              DWORD dwSampleSize)
{
    // Display information about the sample.
    printf("Sample: start = %I64d, duration = %I64d, bytes = %d\n", llSampleTime, llSampleDuration, dwSampleSize);
    return S_OK;
}

STDMETHODIMP SampleGrabberCB::OnShutdown()
{
    return S_OK;
}