# Space folding and repetition - concepts

A shader does not have to change a shape to create a new visual structure.
It can change the coordinates given to the shape instead. The same mathematical
shape can then be evaluated at many different positions, producing repetition,
symmetry, and patterns from a very small amount of mathematics.

This is the central idea of domain manipulation. Instead of asking a circle
equation to describe hundreds of circles, we transform the point being given
to one circle equation. The equation stays simple while the coordinate system
around it becomes more interesting.

## Repeating a coordinate

Suppose a point has an x coordinate that can range over a large interval.
We can reduce that coordinate into a smaller interval with modulo arithmetic.

The basic operation is:

```slang
float repeated = fmod(x, cellSize);
```

The remainder keeps only the part of the coordinate that lies inside one
period. As x continues increasing, the result repeatedly wraps around.
This means one interval can represent every repeated interval.

For a visual pattern, this is useful because a single mathematical model can
be evaluated once per repeated cell. The shader does not need to explicitly
create a list of objects. The coordinate transformation makes every cell look
at the same underlying model.

The important shift is that repetition happens to the input space, not to the
shape itself.

## Centering every cell

A raw remainder usually produces coordinates from zero up to the cell size.
That is awkward for shapes because their natural center is usually at zero.

We can instead construct a centered repetition:

```slang
float centered = x - cellSize * floor(x / cellSize + 0.5);
```

The `floor` operation chooses which cell contains the point. Subtracting that
cell's position moves the point back toward the center of the cell.

For a cell size of 2, the resulting coordinate is approximately constrained
to the interval from -1 to 1. A point at x = 4.7 therefore becomes a point
near x = 0.7 in the local coordinate system of the repeated cell.

This is more than a convenient formula. It establishes a general pattern:
identify a region, subtract its location, and evaluate the same local model
inside every copy.

## Repeating in two dimensions

A two-dimensional point has two coordinates, so the same operation can be
applied independently to both axes.

```slang
float2 cell = float2(2.0, 2.0);
float2 q = p - cell * floor(p / cell + 0.5);
```

Now every point is mapped into the same centered square cell. A circle
evaluated using `q` therefore appears once in every cell.

The original point `p` may be anywhere in the plane, but the shape only sees
its local representative `q`. Points that are separated by whole cell widths
produce the same local coordinates and therefore produce the same result.

This is why domain repetition can create an infinite pattern from one shape.
The shader is not storing infinite geometry. It is mapping infinitely many
positions onto one finite domain.

## The cell boundary matters

The repetition formula has a discontinuity at each cell boundary. A point
crossing that boundary suddenly changes from the positive side of one cell to
the negative side of the next.

That discontinuity is usually exactly what we want for repetition, but it can
also affect later calculations. A pattern that depends on the direction or
history of a coordinate may reveal those boundaries.

The useful mental model is that the repeated coordinate is local, not global.
After folding, the shader has intentionally forgotten which copy of the cell
the point came from.

This is a form of information reduction. Many different global coordinates
become the same local coordinate.

## Reflection creates symmetry

Repetition is not the only way to reduce a coordinate domain. Reflection can
fold multiple regions onto the same side of an axis.

The simplest reflection is:

```slang
float2 q = abs(p);
```

The negative half of the coordinate system is reflected onto the positive
half. A point at x = -0.4 and a point at x = 0.4 therefore become identical
after the transformation.

A shape evaluated with `abs` automatically becomes symmetric around that axis.
The shape itself does not need a special symmetry rule.

Reflection can also be applied after repetition. First map a point into a
cell, then reflect that local point:

```slang
q = abs(q);
```

The result is that only part of the cell needs to be modeled explicitly.
The remaining regions are mirrored copies.

## Folding more than one axis

A two-dimensional fold can reflect both coordinates:

```slang
q = abs(q);
```

The original four quadrants now share the same positive coordinate region.
One quarter of a design can therefore describe all four quadrants.

This idea can be repeated conceptually. A transformation can reduce the
domain before another transformation reduces it again.

For example, repetition can first place a point inside one cell, reflection
can reduce that cell to one quadrant, and another transformation can impose
additional symmetry inside that quadrant.

The resulting image can have a surprising amount of structure even though
the underlying mathematical model remains small.

## Folding is changing the domain

The word "fold" is useful because these operations can be imagined physically.
Imagine drawing a coordinate plane on a sheet of paper, then folding the sheet
along an axis. Points that used to occupy different locations are now placed
on top of one another.

The shader performs the mathematical equivalent by transforming coordinates
before evaluating the visual function.

This distinction matters because the transformation does not create a new
shape. If the original function is a circle, it is still a circle function.
The transformed coordinates simply cause that function to be evaluated from
many different local viewpoints.

The general pipeline is therefore:

```slang
float2 q = transform(p);
float value = shape(q);
```

The first line defines the domain transformation. The second line evaluates
the visual model inside that transformed domain.

## Combining repetition and symmetry

The strongest patterns come from composing simple transformations.

A coordinate can first be repeated into a finite cell, then reflected so only
one quadrant matters, and finally passed into a shape or pattern function.

```slang
float2 q = repeat(p, cellSize);
q = abs(q);
float value = length(q) - radius;
```

The same shape equation now appears throughout an infinite repeated and
symmetric domain.

This is a powerful shader technique because each operation has a very clear
mathematical responsibility. Repetition controls where copies occur,
reflection controls symmetry, and the final function controls what exists
inside the reduced domain.

## Next step

Now type the code version of this lesson.

