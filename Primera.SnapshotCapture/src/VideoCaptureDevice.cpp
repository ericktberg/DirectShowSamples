#include "VideoCaptureDevice.h"
#include "InteropStrings.h"
#include <iostream>

VideoCaptureDevice::VideoCaptureDevice(IMoniker* deviceMoniker)
{
    deviceMoniker->AddRef();
    _sourceMoniker = deviceMoniker;
}

VideoCaptureDevice::~VideoCaptureDevice()
{
    if (_sourceMoniker) {
        std::wcout << "Releasing video capture moniker object." << std::endl;
        _sourceMoniker->Release();
    }

    if (_cachedFilter) {
        std::wcout << "Video Capture filter." << std::endl;
        _cachedFilter->Release();
    }
}

std::wstring VideoCaptureDevice::getFriendlyName()
{
    if (_cachedFriendlyName.empty()) {
        IPropertyBag* deviceProperties = NULL;
        HRESULT bindResult = _sourceMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**) &deviceProperties);
        if (bindResult >= 0) {
            VARIANT var;
            var.vt = VT_BSTR;
            bindResult = deviceProperties->Read(L"FriendlyName", &var, NULL);

            _cachedFriendlyName = interopString(var.bstrVal);

            if (bindResult == NOERROR)
            {
                SysFreeString(var.bstrVal);
            }
        }

        deviceProperties->Release();
    }

    return _cachedFriendlyName;
}

IMoniker* VideoCaptureDevice::getSourceMoniker()
{
    return _sourceMoniker;
}

IBaseFilter* VideoCaptureDevice::bindBaseFilter()
{
    if (_cachedFilter == nullptr) {
        // TODO: Get the base filter
        std::wcout << "Binding base filter..." << std::endl;
        HRESULT result = _sourceMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**) &_cachedFilter);
    }

    return _cachedFilter;
}