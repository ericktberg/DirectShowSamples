#include "CaptureGraphBuilder.h"
#include <uuids.h>
#include <strmif.h>
#include <control.h>
#include <iostream>

CaptureGraphBuilder* createCaptureBuilder()
{
    ICaptureGraphBuilder2* captureSource;
    HRESULT result = CoCreateInstance(
        CLSID_CaptureGraphBuilder2,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICaptureGraphBuilder2,
        (void**) &captureSource);

    if (!SUCCEEDED(result)) {
        std::wcout << "Could not create capture graph builder." << std::endl;
        return nullptr;
    }

    IGraphBuilder* filterSource = nullptr;
    result = CoCreateInstance(
        CLSID_FilterGraph,
        NULL, CLSCTX_INPROC,
        IID_IGraphBuilder,
        (LPVOID*) &filterSource);

    if (!SUCCEEDED(result)) {
        std::wcout << "Could not create filter graph builder." << std::endl;
        captureSource->Release();
        return nullptr;
    }

    result = captureSource->SetFiltergraph(filterSource);
    if (!SUCCEEDED(result)) {
        std::wcout << "Could not associated filter graph builder with capture builder." << std::endl;
        captureSource->Release();
        filterSource->Release();
        return nullptr;
    }

    return new CaptureGraphBuilder(captureSource, filterSource);
}

BOOL CaptureGraphBuilder::AddFilter(IBaseFilter* filter, std::wstring filterName)
{
    HRESULT result = _filterGraphSource->AddFilter(filter, filterName.c_str());
    if (!SUCCEEDED(result)) {
        std::wcout << "Could not add filter: " << filterName << std::endl;
        return FALSE;
    }
    else {
        return TRUE;
    }
}

BOOL CaptureGraphBuilder::EstablishPreview(IBaseFilter* videoCaptureFilter)
{
    if (!AddFilter(videoCaptureFilter, L"Video Capture")) {
        return FALSE;
    }

    HRESULT result = _captureGraphSource->RenderStream(
        &PIN_CATEGORY_PREVIEW, 
        &MEDIATYPE_Video, 
        videoCaptureFilter,
        NULL,
        NULL);
    if (!SUCCEEDED(result)) {
        std::wcout << "Could not render preview." << std::endl;
        return FALSE;
    }
    else {
        return TRUE;
    }
}

BOOL CaptureGraphBuilder::Run()
{
    IMediaControl* mediaControl;
    HRESULT hr = _filterGraphSource->QueryInterface(IID_IMediaControl, (void**)&mediaControl);
	if (SUCCEEDED(hr))
	{
		hr = mediaControl->Run();
		if (FAILED(hr))
		{
			// stop parts that ran
			mediaControl->Stop();
		}
		mediaControl->Release();
	}
    return SUCCEEDED(hr);
}
