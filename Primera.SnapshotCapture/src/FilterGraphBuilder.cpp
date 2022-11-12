#include "FilterGraphBuilder.h"

#include <uuids.h>


FilterGraphBuilder* createFilterGraphBuilder() {
	IGraphBuilder* sourceGraphBuilder = nullptr;
	HRESULT result = CoCreateInstance(
		CLSID_FilterGraph,
		NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, 
		(LPVOID*)&sourceGraphBuilder);

	if (result == NOERROR) {
		return new FilterGraphBuilder(sourceGraphBuilder);
	}
	else {
		return nullptr;
	}
}

FilterGraphBuilder::FilterGraphBuilder(IGraphBuilder* captureBuilder)
{
	_sourceGraphBuilder = captureBuilder;
}

FilterGraphBuilder::~FilterGraphBuilder()
{
	if (_sourceGraphBuilder) {
		_sourceGraphBuilder->Release();
	}
}
