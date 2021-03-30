#ifndef WASM_EXPORTER
#define WASM_EXPORTER
#endif
#ifndef MAIN_MODULE
#include "main.cpp"
#endif

val getRegionVertexBufferPtr()
{
  return val(typed_memory_view(regionMgr->getVerticesSize(), regionMgr->getVerticesPtr()));
}

val getRegionUniformPtr()
{
  return val(typed_memory_view(9, regionMgr->uniforms));
}

val getCandlesConstantlyUniformPtr()
{
  return val(typed_memory_view(5, candlesMgr->constanlyUniforms));
}

val getCandlesIndividuallyUniformPtr()
{
  return val(typed_memory_view(3, candlesMgr->individualUniforms));
}

val getCandlesIndividuallyLightPosition()
{
  return val(typed_memory_view(2, candlesMgr->lightPositions));
}

val getCandlesIndividuallyRenderRectangle()
{
  return val(typed_memory_view(8, candlesMgr->renderRectangle));
}

int candleResetIndex()
{
  return candlesMgr->resetIndex();
}

int candleGetCount()
{
  return candlesMgr->getCandleCounts();
}

int candleRenderNext()
{
  return candlesMgr->renderNextCandle();
}

int renderRegionVertices()
{
  return regionMgr->getVerticesSize();
}
