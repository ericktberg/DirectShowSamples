#pragma once
#include <strmif.h>

class FilterGraphBuilder
{

public:
    FilterGraphBuilder(IGraphBuilder* captureBuilder);
    ~FilterGraphBuilder();

    IGraphBuilder* getSource();
    
private:
    IGraphBuilder* _sourceGraphBuilder;
};

FilterGraphBuilder* createFilterGraphBuilder();
