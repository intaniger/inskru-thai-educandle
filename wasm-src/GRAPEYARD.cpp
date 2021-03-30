void _handleRegionZoomEvent(float deltaY, float mouseX, float mouseY)
{
  float currentScale = scaleUniform[0];

  float objectRatio = regionContainerUniform[0] / regionContainerUniform[1];
  float screenRatio = screenUniform[0] / screenUniform[1];
  float xFactor = screenRatio / objectRatio;

  float step = 1 - 0.001 * deltaY;

  float mousePos[2] = {mouseX, mouseY};

  float *localizedMousePosition, screenCenterPosition[2], screenMousePosition[2];

  screenCenterPosition[0] = 0.5 + (centerUniform[0] - 0.5) / xFactor;
  screenCenterPosition[1] = centerUniform[1];

  localizedMousePosition = localize(mousePos, screenCenterPosition, 1 / currentScale, xFactor);

  centerUniform[0] = localizedMousePosition[0] + (centerUniform[0] - localizedMousePosition[0]) / step;
  centerUniform[1] = localizedMousePosition[1] + (centerUniform[1] - localizedMousePosition[1]) / step;

  scaleUniform[0] = currentScale * step;

  delete[] localizedMousePosition;

  // filtering unused vertices
  // float viewBound[4] = {
  //     // xFactor * ((screenCenterPosition[0] - 0.5 * viewScale) - 0.5) + 0.5,
  //     static_cast<float>(xFactor * screenCenterPosition[0] - 0.5 * viewScale * xFactor - 0.5 * xFactor + 0.5),
  //     static_cast<float>(screenCenterPosition[1] - 0.5 * viewScale),
  //     // xFactor * ((screenCenterPosition[0] + 0.5 * viewScale) - 0.5) + 0.5,
  //     static_cast<float>(xFactor * screenCenterPosition[0] + 0.5 * viewScale * xFactor - 0.5 * xFactor + 0.5),
  //     static_cast<float>(screenCenterPosition[1] + 0.5 * viewScale),
  // };
  // printf("View boundary [%f, %f] -> [%f, %f]\n", viewBound[0], viewBound[1], viewBound[2], viewBound[3]);
}
