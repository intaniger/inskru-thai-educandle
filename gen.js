console.log(
  JSON.stringify(
    [...Array(1000).keys()].map(() => [Math.random(), Math.random()]),
  ),
)
