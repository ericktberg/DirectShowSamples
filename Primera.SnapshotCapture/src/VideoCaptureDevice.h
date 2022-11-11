#pragma once
#include <objidl.h>
#include <strmif.h>
#include <string>

/// <summary>
/// A video capture device is acquired
/// </summary>
class VideoCaptureDevice
{
public:
    VideoCaptureDevice(IMoniker* deviceMoniker);
    ~VideoCaptureDevice();
    IMoniker* getSourceMoniker();
    IBaseFilter* bindBaseFilter();

private:
    IMoniker* _sourceMoniker;
    IBaseFilter* _cachedFilter = NULL;
};
