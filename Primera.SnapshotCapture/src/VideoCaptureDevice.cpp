#include "VideoCaptureDevice.h"

VideoCaptureDevice::VideoCaptureDevice(IMoniker* deviceMoniker)
{
    deviceMoniker->AddRef();
    _sourceMoniker = deviceMoniker;
}

VideoCaptureDevice::~VideoCaptureDevice()
{
    _sourceMoniker->Release();
}

IMoniker* VideoCaptureDevice::getSourceMoniker()
{
    return _sourceMoniker;
}

IBaseFilter* VideoCaptureDevice::bindBaseFilter()
{
    if (_cachedFilter == nullptr) {
        // TODO: Get the base filter
        _cachedFilter = nullptr;
    }

    return nullptr;
}