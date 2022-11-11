#pragma once
#include <objidl.h>
#include <strmif.h>

/// <summary>
/// A video capture device is acquired 
/// </summary>
class VideoCaptureDevice
{
public:
    IMoniker* getSourceMoniker();

    IBaseFilter* bindBaseFilter();

private:
    IMoniker* _sourceMoniker;
    IBaseFilter* _cachedFilter = NULL;

};

