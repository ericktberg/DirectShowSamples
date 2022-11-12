#pragma once
#include <FilterGraphBuilder.h>
#include <string>
#include <iostream>

class CaptureGraphBuilder
{
public:

    CaptureGraphBuilder(ICaptureGraphBuilder2* captureGraphSource, IGraphBuilder* filterGraphSource) {
        _captureGraphSource = captureGraphSource;
        _filterGraphSource = filterGraphSource;
    }

    ~CaptureGraphBuilder() {
        if (_captureGraphSource) {
            std::wcout << "Releasing capture graph source." << std::endl;
            _captureGraphSource->Release();
        }

        if (_filterGraphSource) {
            std::wcout << "Releasing filter graph source." << std::endl;
            _filterGraphSource->Release();
        }
    }

    BOOL AddFilter(IBaseFilter* filter, std::wstring filterName);

    BOOL EstablishPreview(IBaseFilter* videoCaptureFilter);

    BOOL Run();

    inline ICaptureGraphBuilder2* getCaptureBuilder() {
        return _captureGraphSource;
    }

    inline IGraphBuilder* getFilterGraph() {
        return _filterGraphSource;
    }

private:
    ICaptureGraphBuilder2* _captureGraphSource;
    IGraphBuilder* _filterGraphSource;
};

CaptureGraphBuilder* createCaptureBuilder();
