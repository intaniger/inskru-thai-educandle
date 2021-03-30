#ifndef MAIN_MODULE
#define MAIN_MODULE
#endif

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>

#include "instruction.h"
#include "idecode.cpp"
#include "drawer.h"
#include "view_coord.cpp"
#include "region_manager.cpp"
#include "candles_manager.cpp"

// data to be consider when decoding huffman tree
// { M: 316, l: 8277, v: 37, h: 38, L: 271, z: 129 }
// data size ~70kb(vertex) + instruction (huffman encoded)
// {
//   l: 00000000,
//   t: 00000100,
//   M: 00000101,
//   L: 00000110,

//   z: 00001110,
//   h: 00011110,
//   v: 00111110,
//   T: 00111111,
// }

char thHuffmanDefinition[] = {
    DRAW_CMD_REL_LINE_TO,
    DRAW_CMD_END_OF_CHUNK,
    DRAW_CMD_ABS_MOVE_TO,
    DRAW_CMD_ABS_LINE_TO,
    DRAW_CMD_BACK_TO_FIRST_POINT,
    DRAW_CMD_REL_H_LINE_TO,
    DRAW_CMD_REL_V_LINE_TO,
    DRAW_CMD_EOF,
};

InstructionsDecoder regionIDecoder(thHuffmanDefinition, 8);
ScreenViewState *view;
RegionManager *regionMgr;
CandlesManager *candlesMgr;

using namespace emscripten;

#ifndef WASM_EXPORTER
#include "export.cpp"
#endif

val initialize(uintptr_t shapes_data_addr, float screenWidth, float screenHeight)
{
  float initCenter[] = {0.5, 0.5};
  float initScale = 1;

  char *shapes_data = reinterpret_cast<char *>(shapes_data_addr);
  auto instructions = regionIDecoder.Decode(shapes_data);
  regionMgr = new RegionManager(
      new StaticShapeDrawer(instructions, reinterpret_cast<float *>(shapes_data)),
      screenWidth, screenHeight,
      initCenter, initScale);

  candlesMgr = new CandlesManager(
      screenWidth, screenHeight,
      regionMgr->getWidth(), regionMgr->getHeight(),
      initCenter, initScale);

  candlesMgr->registerCandle(new Candle(initCenter));

  view = new ScreenViewState(
      initCenter, initScale,
      regionMgr->getWidth() / regionMgr->getHeight(),
      screenWidth / screenHeight);

  return getRegionUniformPtr();
}

void handleZoomEvent(float deltaY, float mouseX, float mouseY)
{
  float step = 1 - 0.001 * deltaY;

  view->ZoomTo(mouseX, mouseY, step);

  auto ctr = view->getCenter();
  auto scle = view->getScale();
  regionMgr->setCenter(ctr);
  regionMgr->setScale(scle);

  candlesMgr->setCenter(ctr);
  candlesMgr->setScale(scle);
}

void handleMoveEvent(float deltaX, float deltaY)
{

  view->RelMoveTo(deltaX, deltaY);

  auto ctr = view->getCenter();
  regionMgr->setCenter(ctr);
  candlesMgr->setCenter(ctr);
}

void handleClickEvent(float mouseX, float mouseY)
{
  float *ctxPos = view->getContextPosition(mouseX, mouseY);
  candlesMgr->registerCandle(new Candle(ctxPos));
}

EMSCRIPTEN_BINDINGS(MAIN)
{
  function("getRegionUniformPtr", &getRegionUniformPtr);
  function("getRegionVertexBufferPtr", &getRegionVertexBufferPtr);

  function("getCandlesConstantlyUniformPtr", &getCandlesConstantlyUniformPtr);
  function("getCandlesIndividuallyUniformPtr", &getCandlesIndividuallyUniformPtr);
  function("getCandlesIndividuallyLightPosition", &getCandlesIndividuallyLightPosition);
  function("getCandlesIndividuallyRenderRectangle", &getCandlesIndividuallyRenderRectangle);

  function("candleRenderNext", &candleRenderNext);
  function("candleResetIndex", &candleResetIndex);
  function("candleGetCount", &candleGetCount);
  // candleGetCount
  function("renderRegionVertices", &renderRegionVertices);

  function("handleZoomEvent", &handleZoomEvent);
  function("handleMoveEvent", &handleMoveEvent);
  function("handleClickEvent", &handleClickEvent);

  function("initialize", &initialize, allow_raw_pointers());
}