#ifndef DRAWER_CTOR
#define DRAWER_CTOR
#endif
#include "drawer.h"

StaticShapeDrawer::StaticShapeDrawer(std::vector<char> instructions, float *argsPtr)
{
  size_t i = 0;
  float *args, *bound;
  float startPoint[2], lastPoint[2];
  VertexGroup *vg = NULL;

  this->frame = NULL;
  this->verticesPtr = (float *)malloc(instructions.size() * sizeof(float) * 4);

  for (auto cmd : instructions)
  {
    switch (cmd)
    {
    case DRAW_CMD_ABS_MOVE_TO:
      args = this->readArgumentsStream(argsPtr, 2);

      startPoint[0] = args[0];
      startPoint[1] = args[1];

      lastPoint[0] = args[0];
      lastPoint[1] = args[1];
      break;

    case DRAW_CMD_REL_LINE_TO:
      args = this->readArgumentsStream(argsPtr, 2);

      this->verticesPtr[i] = lastPoint[0];
      this->verticesPtr[i + 1] = lastPoint[1];

      this->verticesPtr[i + 2] = this->verticesPtr[i] + args[0];
      this->verticesPtr[i + 3] = this->verticesPtr[i + 1] + args[1];
      break;

    case DRAW_CMD_ABS_LINE_TO:
      args = this->readArgumentsStream(argsPtr, 2);

      this->verticesPtr[i] = lastPoint[0];
      this->verticesPtr[i + 1] = lastPoint[1];

      this->verticesPtr[i + 2] = args[0];
      this->verticesPtr[i + 3] = args[1];
      break;

    case DRAW_CMD_BACK_TO_FIRST_POINT:

      this->verticesPtr[i] = lastPoint[0];
      this->verticesPtr[i + 1] = lastPoint[1];

      this->verticesPtr[i + 2] = startPoint[0];
      this->verticesPtr[i + 3] = startPoint[1];
      break;

    case DRAW_CMD_REL_H_LINE_TO:
      args = this->readArgumentsStream(argsPtr, 1);

      this->verticesPtr[i] = lastPoint[0];
      this->verticesPtr[i + 1] = lastPoint[1];

      this->verticesPtr[i + 2] = this->verticesPtr[i] + args[0];
      this->verticesPtr[i + 3] = this->verticesPtr[i + 1];
      break;

    case DRAW_CMD_REL_V_LINE_TO:
      args = this->readArgumentsStream(argsPtr, 1);

      this->verticesPtr[i] = lastPoint[0];
      this->verticesPtr[i + 1] = lastPoint[1];

      this->verticesPtr[i + 2] = this->verticesPtr[i];
      this->verticesPtr[i + 3] = this->verticesPtr[i + 1] + args[0];
      break;

    case DRAW_CMD_END_OF_CHUNK:
    case DRAW_CMD_EOF:
    default:
      break; // NO-OP yet.
    }

    if (vg == NULL)
      vg = new VertexGroup(&this->verticesPtr[i], lastPoint);

    switch (cmd)
    {
    case DRAW_CMD_REL_LINE_TO:
    case DRAW_CMD_ABS_LINE_TO:
    case DRAW_CMD_BACK_TO_FIRST_POINT:
    case DRAW_CMD_REL_H_LINE_TO:
    case DRAW_CMD_REL_V_LINE_TO:
      i += 4;
      lastPoint[0] = this->verticesPtr[i - 2];
      lastPoint[1] = this->verticesPtr[i - 1];

    case DRAW_CMD_ABS_MOVE_TO:
      vg->AddVertex(lastPoint[0], lastPoint[1]);
      break;

    case DRAW_CMD_EOF:
    case DRAW_CMD_END_OF_CHUNK:
      bound = vg->getBoundary();
      if (this->frame == NULL)
        this->frame = new VertexGroup(&this->verticesPtr[0], bound);

      this->frame->AddVertex(bound[0], bound[1]);
      this->frame->AddVertex(bound[2], bound[3]);

      vg->EndAt(&this->verticesPtr[i + 2]);
      this->vgs.push_back(vg);

      // bound = vg->getBoundary();
      // this->verticesPtr[i] = bound[0];
      // this->verticesPtr[i + 1] = bound[3];
      // this->verticesPtr[i + 2] = bound[0];
      // this->verticesPtr[i + 3] = bound[1];

      // this->verticesPtr[i + 4] = bound[0];
      // this->verticesPtr[i + 5] = bound[1];
      // this->verticesPtr[i + 6] = bound[2];
      // this->verticesPtr[i + 7] = bound[1];

      // this->verticesPtr[i + 8] = bound[2];
      // this->verticesPtr[i + 9] = bound[1];
      // this->verticesPtr[i + 10] = bound[2];
      // this->verticesPtr[i + 11] = bound[3];

      // this->verticesPtr[i + 12] = bound[2];
      // this->verticesPtr[i + 13] = bound[3];
      // this->verticesPtr[i + 14] = bound[0];
      // this->verticesPtr[i + 15] = bound[3];
      // i += 16;
      vg = NULL;

      if (cmd == DRAW_CMD_EOF)
      {
        this->verticesSize = i;
        // bound = this->frame->getBoundary();
        // this->verticesPtr[i] = bound[0];
        // this->verticesPtr[i + 1] = bound[3];
        // this->verticesPtr[i + 2] = bound[0];
        // this->verticesPtr[i + 3] = bound[1];

        // this->verticesPtr[i + 4] = bound[0];
        // this->verticesPtr[i + 5] = bound[1];
        // this->verticesPtr[i + 6] = bound[2];
        // this->verticesPtr[i + 7] = bound[1];

        // this->verticesPtr[i + 8] = bound[2];
        // this->verticesPtr[i + 9] = bound[1];
        // this->verticesPtr[i + 10] = bound[2];
        // this->verticesPtr[i + 11] = bound[3];

        // this->verticesPtr[i + 12] = bound[2];
        // this->verticesPtr[i + 13] = bound[3];
        // this->verticesPtr[i + 14] = bound[0];
        // this->verticesPtr[i + 15] = bound[3];
        // this->verticesSize = i + 16;
        return;
      }
      break;

    default:
      break;
    }
  }
}
