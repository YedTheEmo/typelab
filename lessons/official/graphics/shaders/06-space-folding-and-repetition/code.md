# Space folding and repetition - typing

This lesson types coordinate manipulation: center a domain, repeat it, fold it,
and evaluate the same shape inside every transformed cell.

## Normalize the fragment position

Start by converting the incoming pixel position into a centered coordinate.

```slang
// read the current fragment position
float2 pixel = position.xy;

// convert the pixel position into a centered coordinate
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;
```

The coordinate `p` now represents the screen as a mathematical plane centered
around the origin.

## Repeat the coordinate space

Next, map every point into a centered square cell.

```slang
// define the width and height of each repeated cell
float2 cellSize = float2(0.8, 0.8);

// find the centered position inside the current cell
float2 q = p - cellSize * floor(p / cellSize + 0.5);
```

The shader now has one local cell that represents every cell in the plane.

## Fold the cell

Reflect the local coordinate so every quadrant shares the same region.

```slang
// reflect both axes into the positive quadrant
q = abs(q);
```

The point `q` is now restricted to one quarter of the repeated cell.

## Build a local shape

Use the folded coordinate to describe a circle around the local origin.

```slang
// choose the radius of the local circle
float radius = 0.25;

// measure the distance from the local origin
float distance = length(q) - radius;
```

The same circle is now evaluated in every repeated and reflected region.

## Turn distance into a visible shape

Convert the signed distance into a soft mask.

```slang
// create a soft transition around the circle boundary
float shape = 1.0 - smoothstep(0.0, 0.02, abs(distance));
```

The absolute distance makes both sides of the boundary contribute to the mask.

## Add the cell structure

A coordinate transformation can also expose the repeated cell itself.

```slang
// measure distance from the nearest cell edge
float edge = min(cellSize.x * 0.5 - q.x,
                 cellSize.y * 0.5 - q.y);

// create a thin line near each cell boundary
float grid = 1.0 - smoothstep(0.0, 0.015, edge);
```

The grid is not another object stored by the shader. It is a consequence of
the coordinate domain that the shader constructed.

## Combine the visual fields

Mix the local shape and the cell boundary into one grayscale value.

```slang
// combine the circle and cell boundary masks
float value = max(shape, grid);

// return the scalar field as grayscale
return float4(value, value, value, 1.0);
```

The final image contains repeated circles with symmetric cell boundaries.
Only one circle was mathematically defined.

## Complete shader

Put the transformations into one complete shader so the flow is visible.

```slang
// read the current fragment position
float2 pixel = position.xy;

// convert the pixel position into a centered coordinate
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;

// define the width and height of each repeated cell
float2 cellSize = float2(0.8, 0.8);

// find the centered position inside the current cell
float2 q = p - cellSize * floor(p / cellSize + 0.5);

// reflect both axes into the positive quadrant
q = abs(q);

// choose the radius of the local circle
float radius = 0.25;

// measure the distance from the local origin
float distance = length(q) - radius;

// create a soft transition around the circle boundary
float shape = 1.0 - smoothstep(0.0, 0.02, abs(distance));

// measure distance from the nearest cell edge
float edge = min(cellSize.x * 0.5 - q.x,
                 cellSize.y * 0.5 - q.y);

// create a thin line near each cell boundary
float grid = 1.0 - smoothstep(0.0, 0.015, edge);

// combine the circle and cell boundary masks
float value = max(shape, grid);

// return the scalar field as grayscale
return float4(value, value, value, 1.0);
```

## Change the repetition

The important part of the shader is not the particular circle. It is the
coordinate transformation that decides how often that circle appears.

Change the cell size and observe how the same model fills more or less of the
screen.

```slang
// make the repeated cells smaller
float2 cellSize = float2(0.5, 0.5);
```

A smaller cell means more copies fit into the same global coordinate space.

## Change the fold

The reflection is also an independent transformation. Remove it and the
circle becomes centered around every cell without the quadrant reduction.

```slang
// reflect both axes into the positive quadrant
q = abs(q);
```

The shape itself has not changed. Only the domain supplied to the shape has
changed.

## Now type it again

Re-drill the centered repetition without changing the mathematics.

```slang
// define the width and height of each repeated cell
float2 cellSize = float2(0.8, 0.8);

// find the centered position inside the current cell
float2 q = p - cellSize * floor(p / cellSize + 0.5);

// reflect both axes into the positive quadrant
q = abs(q);
```

Then rebuild the local shape from the folded coordinate.

```slang
// choose the radius of the local circle
float radius = 0.25;

// measure the distance from the local origin
float distance = length(q) - radius;

// create a soft transition around the circle boundary
float shape = 1.0 - smoothstep(0.0, 0.02, abs(distance));
```

Finally, drill the complete transformation and output chain.

```slang
// convert the pixel position into a centered coordinate
float2 p = pixel / resolution;

// move the origin to the center of the screen
p -= 0.5;

// preserve the visual aspect ratio
p.x *= resolution.x / resolution.y;

// find the centered position inside the current cell
float2 q = p - cellSize * floor(p / cellSize + 0.5);

// reflect both axes into the positive quadrant
q = abs(q);

// measure the distance from the local origin
float distance = length(q) - radius;

// create a soft transition around the circle boundary
float shape = 1.0 - smoothstep(0.0, 0.02, abs(distance));

// return the scalar field as grayscale
return float4(shape, shape, shape, 1.0);
```

## Wrap up

The flow: global coordinates -> repeat -> center -> reflect -> evaluate shape.

The shape stays simple because the coordinate domain does the repetition and
symmetry.

