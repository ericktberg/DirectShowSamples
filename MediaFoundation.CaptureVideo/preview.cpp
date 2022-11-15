//////////////////////////////////////////////////////////////////////////
//
// preview.cpp: Manages video preview.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "MFCaptureD3D.h"
#include <shlwapi.h>

//-------------------------------------------------------------------
//  CreateInstance
//
//  Static class method to create the CPreview object.
//-------------------------------------------------------------------

HRESULT CPreview::CreateInstance(
    HWND hVideo,        // Handle to the video window.
    HWND hEvent,        // Handle to the window to receive notifications.
    CPreview** ppPlayer // Receives a pointer to the CPreview object.
)
{
    assert(hVideo != NULL);
    assert(hEvent != NULL);

    if (ppPlayer == NULL)
    {
        return E_POINTER;
    }

    CPreview* pPlayer = new (std::nothrow) CPreview(hVideo, hEvent);

    // The CPlayer constructor sets the ref count to 1.

    if (pPlayer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pPlayer->Initialize();

    if (SUCCEEDED(hr))
    {
        *ppPlayer = pPlayer;
        (*ppPlayer)->AddRef();
    }

    SafeRelease(&pPlayer);
    return hr;
}

//-------------------------------------------------------------------
//  constructor
//-------------------------------------------------------------------

CPreview::CPreview(HWND hVideo, HWND hEvent) :
    m_pReader(NULL),
    m_hwndVideo(hVideo),
    m_hwndEvent(hEvent),
    m_nRefCount(1),
    m_pwszSymbolicLink(NULL),
    m_cchSymbolicLink(0)
{
    InitializeCriticalSection(&m_critsec);
}

//-------------------------------------------------------------------
//  destructor
//-------------------------------------------------------------------

CPreview::~CPreview()
{
    CloseDevice();

    m_draw.DestroyDevice();

    DeleteCriticalSection(&m_critsec);
}

//-------------------------------------------------------------------
//  Initialize
//
//  Initializes the object.
//-------------------------------------------------------------------

HRESULT CPreview::Initialize()
{
    HRESULT hr = S_OK;

    hr = m_draw.CreateDevice(m_hwndVideo);

    return hr;
}

//-------------------------------------------------------------------
//  CloseDevice
//
//  Releases all resources held by this object.
//-------------------------------------------------------------------

HRESULT CPreview::CloseDevice()
{
    EnterCriticalSection(&m_critsec);

    SafeRelease(&m_pReader);

    CoTaskMemFree(m_pwszSymbolicLink);
    m_pwszSymbolicLink = NULL;
    m_cchSymbolicLink = 0;

    LeaveCriticalSection(&m_critsec);
    return S_OK;
}

/////////////// IUnknown methods ///////////////

//-------------------------------------------------------------------
//  AddRef
//-------------------------------------------------------------------

ULONG CPreview::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

//-------------------------------------------------------------------
//  Release
//-------------------------------------------------------------------

ULONG CPreview::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}

//-------------------------------------------------------------------
//  QueryInterface
//-------------------------------------------------------------------

HRESULT CPreview::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CPreview, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

/////////////// IMFSourceReaderCallback methods ///////////////

//-------------------------------------------------------------------
// OnReadSample
//
// Called when the IMFMediaSource::ReadSample method completes.
//-------------------------------------------------------------------


int readFrame = 10;
int framesRead = 0;

HRESULT CPreview::OnReadSample(
    HRESULT hrStatus,
    DWORD  dwStreamIndex,
    DWORD dwStreamFlags,
    LONGLONG llTimestamp,
    IMFSample* pSample      // Can be NULL
)
{
    UNREFERENCED_PARAMETER(dwStreamIndex);
    UNREFERENCED_PARAMETER(dwStreamFlags);
    UNREFERENCED_PARAMETER(llTimestamp);

    HRESULT hr = S_OK;
    IMFMediaBuffer* pBuffer = NULL;
    

    EnterCriticalSection(&m_critsec);

    if (FAILED(hrStatus))
    {
        hr = hrStatus;
    }

    if (SUCCEEDED(hr))
    {
        if (pSample)
        {
            // Get the video frame buffer from the sample.

            hr = pSample->GetBufferByIndex(0, &pBuffer);

            if (framesRead++ == readFrame) {
                
            }

            // Draw the frame.

            if (SUCCEEDED(hr))
            {
                hr = m_draw.DrawFrame(pBuffer);
            }
        }
    }

    DWORD streamIndex = (DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM;

    // Request the next frame.
    if (SUCCEEDED(hr))
    {
        hr = m_pReader->ReadSample(
            streamIndex,
            0,
            NULL,   // actual
            NULL,   // flags
            NULL,   // timestamp
            NULL    // sample
        );
    }

    if (FAILED(hr))
    {
        NotifyError(hr);
    }
    SafeRelease(&pBuffer);

    LeaveCriticalSection(&m_critsec);
    return hr;
}

//-------------------------------------------------------------------
// TryMediaType
//
// Test a proposed video format.
//-------------------------------------------------------------------

HRESULT CPreview::TryMediaType(IMFMediaType* pType, DWORD streamIndex)
{
    HRESULT hr = S_OK;

    BOOL bFound = FALSE;
    GUID subtype = { 0 };

    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);

    if (FAILED(hr))
    {
        return hr;
    }

    // Do we support this type directly?
    if (m_draw.IsFormatSupported(subtype))
    {
        bFound = TRUE;
    }
    else
    {
        // Can we decode this media type to one of our supported
        // output formats?

        for (DWORD i = 0; ; i++)
        {
            // Get the i'th format.
            hr = m_draw.GetFormat(i, &subtype);
            if (FAILED(hr)) { break; }

            hr = pType->SetGUID(MF_MT_SUBTYPE, subtype);

            if (FAILED(hr)) { break; }

            // Try to set this type on the source reader.
            hr = m_pReader->SetCurrentMediaType(
                streamIndex,
                NULL,
                pType
            );

            if (SUCCEEDED(hr))
            {
                bFound = TRUE;
                break;
            }
        }
    }

    if (bFound)
    {
        hr = m_draw.SetVideoType(pType);
    }

    return hr;
}

//-------------------------------------------------------------------
// SetDevice
//
// Set up preview for a specified video capture device.
//-------------------------------------------------------------------

#define CHECK_HR(x) if (FAILED(x)) { goto done; }

HRESULT CPreview::SetDevice(IMFActivate* pActivate)
{
    HRESULT hr = S_OK;

    IMFMediaSource* mediaSource = NULL;
    IMFAttributes* pAttributes = NULL;
    IMFMediaType* pType = NULL;

    EnterCriticalSection(&m_critsec);

    // Release the current device, if any.
    CHECK_HR(hr = CloseDevice());

    // Create the media source for the device.
    CHECK_HR(hr = pActivate->ActivateObject(__uuidof(IMFMediaSource), (void**) &mediaSource));

    // Get the symbolic link.
    CHECK_HR(hr = pActivate->GetAllocatedString(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
        &m_pwszSymbolicLink,
        &m_cchSymbolicLink
    ));

    IMFPresentationDescriptor* descriptor;
    CHECK_HR(hr = mediaSource->CreatePresentationDescriptor(&descriptor));

    DWORD descriptorCount;
    CHECK_HR(hr = descriptor->GetStreamDescriptorCount(&descriptorCount));

    for (DWORD i = 0; i < descriptorCount; i++) {
        BOOL isDescriptorSelected;
        IMFStreamDescriptor* streamDescriptor;
        hr = descriptor->GetStreamDescriptorByIndex(i, &isDescriptorSelected, &streamDescriptor);

        if (FAILED(hr)) {
            goto done;
        }
        else {
            DWORD streamId;

            streamDescriptor->GetStreamIdentifier(&streamId);

            SafeRelease(&streamDescriptor);
        }
    }

    //
    // Create the source reader.
    //

    // Create an attribute store to hold initialization settings.

    CHECK_HR(hr = MFCreateAttributes(&pAttributes, 2));
    CHECK_HR(hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE));

    // Set the callback pointer.
    CHECK_HR(hr = pAttributes->SetUnknown(
        MF_SOURCE_READER_ASYNC_CALLBACK,
        this
    ));

    CHECK_HR(hr = MFCreateSourceReaderFromMediaSource(
        mediaSource,
        pAttributes,
        &m_pReader
    ));

    DWORD streamIndex = (DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM;

    // Try to find a suitable output type.
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; ; i++)
        {
            hr = m_pReader->GetNativeMediaType(
                streamIndex,
                i,
                &pType
            );

            if (FAILED(hr)) { break; }

            hr = TryMediaType(pType, streamIndex);

            SafeRelease(&pType);

            if (SUCCEEDED(hr))
            {
                // Found an output type.
                break;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // Ask for the first sample.
        hr = m_pReader->ReadSample(
            streamIndex,
            0,
            NULL,
            NULL,
            NULL,
            NULL
        );
    }

done:
    if (FAILED(hr))
    {
        if (mediaSource)
        {
            mediaSource->Shutdown();

            // NOTE: The source reader shuts down the media source
            // by default, but we might not have gotten that far.
        }
        CloseDevice();
    }

    SafeRelease(&mediaSource);
    SafeRelease(&descriptor);
    SafeRelease(&pAttributes);
    SafeRelease(&pType);

    LeaveCriticalSection(&m_critsec);
    return hr;
}

//-------------------------------------------------------------------
//  ResizeVideo
//  Resizes the video rectangle.
//
//  The application should call this method if the size of the video
//  window changes; e.g., when the application receives WM_SIZE.
//-------------------------------------------------------------------

HRESULT CPreview::ResizeVideo(WORD /*width*/, WORD /*height*/)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critsec);

    hr = m_draw.ResetDevice();

    if (FAILED(hr))
    {
        MessageBox(NULL, L"ResetDevice failed!", NULL, MB_OK);
    }

    LeaveCriticalSection(&m_critsec);

    return hr;
}

//-------------------------------------------------------------------
//  CheckDeviceLost
//  Checks whether the current device has been lost.
//
//  The application should call this method in response to a
//  WM_DEVICECHANGE message. (The application must register for
//  device notification to receive this message.)
//-------------------------------------------------------------------

HRESULT CPreview::CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, BOOL* pbDeviceLost)
{
    DEV_BROADCAST_DEVICEINTERFACE* pDi = NULL;

    if (pbDeviceLost == NULL)
    {
        return E_POINTER;
    }

    *pbDeviceLost = FALSE;

    if (pHdr == NULL)
    {
        return S_OK;
    }

    if (pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
    {
        return S_OK;
    }

    pDi = (DEV_BROADCAST_DEVICEINTERFACE*) pHdr;

    EnterCriticalSection(&m_critsec);

    if (m_pwszSymbolicLink)
    {
        if (_wcsicmp(m_pwszSymbolicLink, pDi->dbcc_name) == 0)
        {
            *pbDeviceLost = TRUE;
        }
    }

    LeaveCriticalSection(&m_critsec);

    return S_OK;
}