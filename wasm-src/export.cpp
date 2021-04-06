#ifndef WASM_EXPORTER
#define WASM_EXPORTER
#endif
#ifndef MAIN_MODULE
#include "main.cpp"
#endif

val getRegionVertexBufferPtr()
{
  return val(typed_memory_view(regionRndr->getVerticesSize(), regionRndr->getVerticesPtr()));
}

val getRegionUniformPtr()
{
  return val(typed_memory_view(9, regionRndr->uniforms));
}

val getCandlesConstantlyUniformPtr()
{
  return val(typed_memory_view(5, candlesRndr->constanlyUniforms));
}

val getCandlesIndividuallyUniformPtr()
{
  return val(typed_memory_view(3, candlesRndr->individualUniforms));
}

val getCandlesIndividuallyLightPosition()
{
  return val(typed_memory_view(candlesRndr->lightNumber * 2, candlesRndr->lightPositions));
}

val getCandlesIndividuallyRenderRectangle()
{
  return val(typed_memory_view(8, candlesRndr->renderRectangle));
}

int candleResetIndex()
{
  return candlesRndr->resetIndex();
}

int candleGetCount()
{
  return candlesRndr->getRenderGroupCounts();
}

int candleRenderNext()
{
  return candlesRndr->renderNextGroup();
}

int renderRegionVertices()
{
  return regionRndr->getVerticesSize();
}
