const resizeCanvasToDisplaySize = (canvas) => {
  // Lookup the size the browser is displaying the canvas in CSS pixels.
  const displayWidth = canvas.clientWidth
  const displayHeight = canvas.clientHeight

  // Check if the canvas is not the same size.
  const needResize =
    canvas.width !== displayWidth || canvas.height !== displayHeight

  if (needResize) {
    // Make the canvas the same size
    canvas.width = displayWidth
    canvas.height = displayHeight
  }

  return needResize
}

const setupCanvas = () => {
  window.appCanvasElement = document.querySelector('#artboard')
  resizeCanvasToDisplaySize(window.appCanvasElement)
  return window.appCanvasElement
}

const initializeGL = async (canvas) => {
  const vs = await (await fetch('shaders/vertex.vs')).text()
  const fs = await (await fetch('shaders/fragment.fs')).text()

  window.appGL = canvas.getContext('webgl')
  window.region_prg = window.appGL.createProgram()

  const gl = window.appGL
  const region_program = window.region_prg

  const vertexShader = gl.createShader(gl.VERTEX_SHADER)
  gl.shaderSource(vertexShader, vs)
  const fragmentShader = gl.createShader(gl.FRAGMENT_SHADER)
  gl.shaderSource(fragmentShader, fs)

  gl.compileShader(vertexShader)
  gl.compileShader(fragmentShader)

  if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
    throw new Error(gl.getShaderInfoLog(vertexShader))
  }
  if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
    throw new Error(gl.getShaderInfoLog(fragmentShader))
  }

  gl.attachShader(region_program, vertexShader)
  gl.attachShader(region_program, fragmentShader)
  gl.linkProgram(region_program)

  const candle_vs = await (await fetch('shaders/candle_vertex.vs')).text()
  const candle_fs = await (await fetch('shaders/candle_fragment.fs')).text()

  window.candle_prg = gl.createProgram()
  const candle_program = window.candle_prg

  const candleVertexShader = gl.createShader(gl.VERTEX_SHADER)
  gl.shaderSource(candleVertexShader, candle_vs)
  const candleFragmentShader = gl.createShader(gl.FRAGMENT_SHADER)
  gl.shaderSource(candleFragmentShader, candle_fs)

  gl.compileShader(candleVertexShader)
  gl.compileShader(candleFragmentShader)

  if (!gl.getShaderParameter(candleVertexShader, gl.COMPILE_STATUS)) {
    throw new Error(gl.getShaderInfoLog(candleVertexShader))
  }
  if (!gl.getShaderParameter(candleFragmentShader, gl.COMPILE_STATUS)) {
    throw new Error(gl.getShaderInfoLog(candleFragmentShader))
  }

  gl.attachShader(candle_program, candleVertexShader)
  gl.attachShader(candle_program, candleFragmentShader)
  gl.linkProgram(candle_program)

  resizeCanvasToDisplaySize(canvas)
  gl.viewport(0, 0, gl.canvas.width, gl.canvas.height)
  gl.clearColor(0, 0, 0, 1.0)
  gl.lineWidth(2.0)
  return [gl, region_program, candle_program]
}

const setRegionUniformValFromWASMArray = (gl, program, uniforms) => {
  // console.log(uniforms)
  gl.useProgram(program)

  const containerUniformLoc = gl.getUniformLocation(program, 'container')
  const offsetUniformLoc = gl.getUniformLocation(program, 'container_offset')
  const centerUniformLoc = gl.getUniformLocation(program, 'center')
  const screenUniformLoc = gl.getUniformLocation(program, 'screen')
  const scaleUniformLoc = gl.getUniformLocation(program, 'scale')

  gl.uniform2fv(containerUniformLoc, uniforms.slice(0, 2))
  gl.uniform2fv(offsetUniformLoc, uniforms.slice(2, 4))
  gl.uniform2fv(centerUniformLoc, uniforms.slice(4, 6))
  gl.uniform2fv(screenUniformLoc, uniforms.slice(6, 8))
  gl.uniform1fv(scaleUniformLoc, uniforms.slice(8, 9))
}

const initializeRegionProgram = (uniforms, vertices) => {
  const gl = window.appGL
  const prg = window.region_prg
  const canvas = window.appCanvasElement

  // const vBuf =
  window.regionBuffer = gl.createBuffer()
  const buffer = window.regionBuffer
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer)

  gl.useProgram(prg)
  const positionLoc = gl.getAttribLocation(prg, 'position')
  gl.enableVertexAttribArray(positionLoc)
  gl.vertexAttribPointer(positionLoc, 2, gl.FLOAT, false, 0, 0)

  gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW)

  setRegionUniformValFromWASMArray(gl, prg, uniforms)

  canvas.addEventListener(
    'wheel',
    (e) => {
      const gl = window.appGL
      const prg = window.region_prg
      Module.handleZoomEvent(
        e.deltaY,
        e.offsetX / canvas.width,
        e.offsetY / canvas.height,
      )

      render(gl, prg, window.candle_prg)
    },
    { passive: true },
  )

  canvas.addEventListener('mousedown', (e) => {
    window.isPureClick = true
    window.dragPossible = true
  })

  canvas.addEventListener('mousemove', (e) => {
    if (!window.dragPossible) return
    window.isPureClick = false

    const gl = window.appGL
    const prg = window.region_prg
    Module.handleMoveEvent(
      (-1 * e.movementX) / canvas.width,
      (-1 * e.movementY) / canvas.height,
    )

    render(gl, prg, window.candle_prg)
  })

  canvas.addEventListener('mouseup', (e) => {
    window.dragPossible = false
  })

  canvas.addEventListener('click', (e) => {
    if (window.isPureClick) {
      Module.handleClickEvent(
        e.offsetX / canvas.width,
        e.offsetY / canvas.height,
      )

      render(gl, window.region_prg, window.candle_prg)
    }
  })
}

let candleVertices = new Float32Array()
const lightPositions = [0.5, 0.5]
const indices = new Uint16Array([0, 1, 2, 2, 1, 3])

const initializeCandleProgram = (uniforms, rectangle) => {
  const gl = window.appGL
  const prg = window.candle_prg

  gl.useProgram(prg)
  window.candleBuffer = gl.createBuffer()
  const buffer = window.candleBuffer
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer)
  gl.bufferData(gl.ARRAY_BUFFER, rectangle, gl.DYNAMIC_DRAW)

  window.candleIndicesBuffer = gl.createBuffer()
  const indexBuffer = window.candleIndicesBuffer
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer)
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.DYNAMIC_DRAW)

  const corrds = gl.getAttribLocation(prg, 'coords')
  gl.enableVertexAttribArray(corrds)
  gl.vertexAttribPointer(corrds, 2, gl.FLOAT, false, 0, 0)

  const screenRatioUniformLoc = gl.getUniformLocation(prg, 'screen_ratio')
  const containerRatioUniformLoc = gl.getUniformLocation(prg, 'container_ratio')
  const centerUniformLoc = gl.getUniformLocation(prg, 'center')
  const scaleUniformLoc = gl.getUniformLocation(prg, 'scale')

  gl.uniform1f(screenRatioUniformLoc, uniforms[0])
  gl.uniform1f(containerRatioUniformLoc, uniforms[1])
  gl.uniform2fv(centerUniformLoc, uniforms.slice(2, 4))
  gl.uniform1f(scaleUniformLoc, uniforms[4])
}

const renderEachCandle = (
  gl,
  lnUniformLoc,
  lpUniformLoc,
  lumUniformLoc,
  radiusUniformLoc,
) => {
  const uniforms = Module.getCandlesIndividuallyUniformPtr()

  gl.uniform1i(lnUniformLoc, uniforms[0])
  gl.uniform1f(lumUniformLoc, uniforms[1])
  gl.uniform1f(radiusUniformLoc, uniforms[2])
  gl.uniform2fv(lpUniformLoc, Module.getCandlesIndividuallyLightPosition())

  gl.bufferSubData(
    gl.ARRAY_BUFFER,
    0,
    Module.getCandlesIndividuallyRenderRectangle(),
  )

  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, window.candleIndicesBuffer)
  gl.bufferSubData(gl.ELEMENT_ARRAY_BUFFER, 0, indices)

  gl.drawElements(gl.TRIANGLES, indices.length, gl.UNSIGNED_SHORT, 0)
  Module.candleRenderNext()
}

const updateCandleCenterAndScale = (gl, centerUniformLoc, scaleUniformLoc) => {
  const uniforms = Module.getCandlesConstantlyUniformPtr()
  gl.uniform2fv(centerUniformLoc, uniforms.slice(2, 4))
  gl.uniform1f(scaleUniformLoc, uniforms[4])
}

async function render(
  // gl: WebGLRenderingContext,
  gl,
  region_program,
  candle_program,
) {
  requestAnimationFrame(() => {
    gl.clear(gl.COLOR_BUFFER_BIT)

    gl.useProgram(candle_program)
    Module.candleResetIndex()
    const candleCount = Module.candleGetCount()

    const centerLoc = gl.getUniformLocation(candle_program, 'center')
    const scaleLoc = gl.getUniformLocation(candle_program, 'scale')
    updateCandleCenterAndScale(gl, centerLoc, scaleLoc)

    const coordsLoc = gl.getAttribLocation(candle_program, 'coords')
    const lightNumbersUniformLoc = gl.getUniformLocation(
      candle_program,
      'lightNumbers',
    )
    const luminanceUniformLoc = gl.getUniformLocation(candle_program, 'lum')
    const radiusUniformLoc = gl.getUniformLocation(candle_program, 'radius')
    const lightPositionsUniformLoc = gl.getUniformLocation(
      candle_program,
      'lightPositions',
    )

    gl.bindBuffer(gl.ARRAY_BUFFER, window.candleBuffer)
    gl.vertexAttribPointer(coordsLoc, 2, gl.FLOAT, false, 0, 0)

    const it = [...Array(candleCount).keys()]
    it.forEach(() =>
      renderEachCandle(
        gl,
        lightNumbersUniformLoc,
        lightPositionsUniformLoc,
        luminanceUniformLoc,
        radiusUniformLoc,
      ),
    )

    gl.useProgram(region_program)
    setRegionUniformValFromWASMArray(
      gl,
      region_program,
      Module.getRegionUniformPtr(),
    )

    const verticeLength = Module.renderRegionVertices()
    const positionLoc = gl.getAttribLocation(region_program, 'position')

    gl.bindBuffer(gl.ARRAY_BUFFER, window.regionBuffer)
    gl.vertexAttribPointer(positionLoc, 2, gl.FLOAT, false, 0, 0)

    gl.bufferSubData(gl.ARRAY_BUFFER, 0, Module.getRegionVertexBufferPtr())
    gl.drawArrays(gl.LINES, 0, verticeLength)
  })
}
