# Distance fields and primitives - concepts

A shader does not need a mesh to describe a shape.

A shape can instead be represented by a mathematical function that receives a
point and returns a number describing that point's relationship to the shape.
A particularly useful version of this idea is the signed distance function,
or SDF.

An SDF turns geometry into a scalar field.

Instead of asking a shape to give us a collection of vertices, we ask:

"Given this point, how far is it from the surface?"

The answer is useful not only for deciding whether a point belongs to a shape,
but also for determining how close the point is to the shape, which side of
the surface it occupies, and how the shape interacts with other mathematical
operations.

This makes distance fields an unusually powerful representation for
procedural graphics.

## A shape as a function

Consider a circle centered at the origin.

We already know that the distance from a point to the origin is:

```
float distance = length(p);
```

If the circle has radius `r`, points exactly on its boundary satisfy:

```
length(p) = r
```

We can therefore define a function whose value is:

```
float d = length(p) - r;
```

This one equation describes the circle's surface and its surrounding space.

At the surface:

```
d = 0
```

Inside the circle:

```
d < 0
```

Outside the circle:

```
d > 0
```

The sign tells us which side of the surface we are on.

The magnitude tells us how far away we are.

That combination is what makes the field signed.

## Why not just return inside or outside?

A simple shape test could return a boolean:

```
inside = length(p) < r;
```

That answers one question:

"Is this point inside the circle?"

An SDF provides much more information:

```
d = length(p) - r;
```

A value of `-0.2` says the point is inside and close to the boundary.

A value of `-5.0` says the point is much deeper inside.

A value of `0.0` identifies the surface.

A value of `0.2` says the point is outside but close to the surface.

The continuous numerical value can then be used by other calculations.

We can turn it into a glow, use it to blend shapes, estimate normals, or
determine how far a ray can safely travel.

This is why an SDF is more useful than a yes-or-no membership test.

## The surface is the zero set

The most important property of an SDF is that the surface exists where:

```
d(p) = 0
```

This is sometimes called the zero level set or zero set of the function.

The function fills the surrounding space with values, but the visible surface
is the boundary where the value reaches zero.

For a circle:

```
d(p) = length(p) - r
```

For every point on the circle:

```
length(p) = r
```

and therefore:

```
d(p) = 0
```

The shape is consequently encoded implicitly.

We do not list the points that make up the surface.

We define a function whose zero crossing describes the surface.

## A field exists everywhere

An important difference between an SDF and a mesh is that the function can be
evaluated at arbitrary coordinates.

Suppose:

```
float d = sphereSDF(p);
```

There does not need to be a sphere vertex at `p`.

The function can still tell us the point's relationship to the sphere.

This means the shape exists as a continuous mathematical field rather than
as a stored collection of discrete geometry.

A shader can evaluate the field at thousands or millions of positions.

The geometry emerges from those evaluations.

This is especially useful for procedural rendering because the shape can be
defined by a small equation instead of a large data structure.

## The sphere is almost the same equation

The idea extends naturally into three dimensions.

For a sphere centered at the origin:

```
float d = length(p) - radius;
```

The only difference is that `p` is now a `float3`.

The length measures the distance from the point to the sphere's center.

Subtracting the radius moves the zero crossing outward to the sphere's surface.

If:

```
length(p) > radius
```

the point is outside.

If:

```
length(p) = radius
```

the point is on the surface.

If:

```
length(p) < radius
```

the point is inside.

The same distance relationship therefore produces the entire sphere.

## Moving the primitive

An SDF is usually easiest to define around the origin.

Suppose we want a sphere centered at:

```
float3 center = float3(2.0, 0.0, 0.0);
```

We can transform the point into the sphere's local coordinate system:

```
float3 local = p - center;
```

Then evaluate the unchanged sphere equation:

```
float d = length(local) - radius;
```

This is the domain transformation idea from the previous lesson.

The sphere function does not need to know where the sphere lives.

The coordinate is moved into the sphere's local space first.

This separation makes primitive functions reusable.

## A plane is a different kind of distance field

Not every primitive is based on distance from a center.

Consider a horizontal plane passing through the origin.

For a point:

```
float3 p;
```

its signed distance from the plane can simply be:

```
float d = p.y;
```

Points above the plane have positive values.

Points below the plane have negative values.

Points exactly on the plane have zero.

The equation is remarkably simple because the Y coordinate already represents
the perpendicular distance from this particular plane.

A plane can therefore be understood as a scalar field where one coordinate
directly provides the signed distance.

## A box uses distance along multiple axes

A box is more interesting because it has boundaries along multiple axes.

For a box centered at the origin, let:

```
float3 halfSize;
```

describe half its width, height, and depth.

We can compare the point's absolute coordinates against those extents.

The key idea is:

```
float3 q = abs(p) - halfSize;
```

Now each component tells us how far the point lies beyond the corresponding
box boundary.

If a component is positive, the point has gone outside that axis's extent.

If it is negative, the point is still inside that extent.

The complete box SDF combines these values to produce a signed distance.

One common formulation is:

```
float3 q = abs(p) - halfSize;
float outside = length(max(q, 0.0));
float inside = min(max(q.x, max(q.y, q.z)), 0.0);
float d = outside + inside;
```

This equation looks more complicated than the sphere equation because a box
has corners and flat faces rather than one continuous radial boundary.

The important thing to understand is not the expression as a recipe.

It is the strategy.

First transform the point into the box's local frame.

Then compare its coordinates against the box's extents.

Then combine the resulting distances into one scalar field.

## Absolute value creates symmetry

The box equation introduces an operation that will become useful throughout
procedural graphics:

```
abs(p)
```

Taking the absolute value removes the sign of each coordinate.

For example:

```
p = float3(-2.0, 3.0, -1.0)
```

becomes:

```
abs(p) = float3(2.0, 3.0, 1.0)
```

This effectively folds all four quadrants of a 2D space or all eight octants
of a 3D space onto the same positive region.

For a centered primitive, this is useful because the shape is symmetric around
its axes.

Instead of separately describing the positive and negative sides, the
function can evaluate one symmetric region.

Later, coordinate folding will build much more elaborate constructions from
this same idea.

## A capsule can combine primitive ideas

A capsule can be thought of as a line segment surrounded by a constant radius.

Instead of defining a complicated surface directly, we can first find the
closest point on the segment and then measure the distance to that point.

Conceptually:

```
closest = closestPointOnSegment(p);
d = length(p - closest) - radius;
```

This illustrates an important general strategy for SDFs.

The function does not necessarily need to describe the final shape directly.
It can construct useful intermediate geometric relationships and then turn
those relationships into a distance.

A shape is often easier to understand as:

```
point
    -> closest relevant feature
    -> distance to that feature
    -> signed result
```

The feature might be a center, a line, a plane, or another geometric boundary.

## SDFs are scalar fields

It is useful to stop thinking of an SDF as "a formula that draws a shape."

The formula actually defines a scalar field throughout space.

Imagine evaluating:

```
d = sphereSDF(p);
```

at every point on a grid.

You would obtain a three-dimensional collection of scalar values.

The surface is merely where that field equals zero.

This perspective is important because many shader techniques operate on the
field rather than directly on the surface.

For example, we can use the field to ask:

"How close am I to the surface?"

or:

"Am I inside or outside?"

or:

"How quickly does the field change around this point?"

The same scalar field can therefore drive several different visual effects.

## Distance becomes useful as a visual value

Because an SDF returns a continuous scalar, we can visualize it directly.

For example:

```
float brightness = 1.0 - abs(d);
```

produces a value that is brightest around the surface.

The exact mapping is not important.

The important idea is that geometry has become a number, and the number can be
fed into another mathematical function.

We could instead use:

```
float brightness = exp(-abs(d) * 10.0);
```

to create a narrow glow around the surface.

Or we could threshold the value:

```
float inside = d < 0.0 ? 1.0 : 0.0;
```

to recover a simple filled shape.

The SDF is therefore not the final image.

It is an intermediate representation from which visual results can be
constructed.

## Multiple primitives can occupy the same space

Suppose we have two distance fields:

```
float a = sphereA(p);
float b = sphereB(p);
```

The two shapes have become two scalar values evaluated at the same point.

This is powerful because scalar values can be combined mathematically.

For example:

```
float d = min(a, b);
```

produces the distance field associated with the union of the two regions in
the basic SDF formulation.

The important conceptual step is that geometry can now be manipulated through
ordinary scalar operations.

A later lesson will explore these operations in detail.

For now, recognize the progression:

```
shape -> distance field -> scalar operation -> new shape
```

This is one of the central ideas behind procedural SDF modeling.

## The quality of a distance matters

A true signed distance function has a particularly useful property: the
magnitude of the value represents the actual shortest distance to the surface.

That property is stronger than simply producing a positive value outside and
a negative value inside.

For many shader techniques, especially raymarching, knowing an actual safe
distance is crucial.

If the field says:

```
d = 2.0
```

then, under the appropriate SDF assumptions, the surface cannot be closer
than two units to the evaluated point.

This allows later algorithms to use the field not merely as a shape test but
as information about how far through space they can move safely.

That is one reason SDFs are particularly valuable for procedural rendering.

## The primitive is a reusable mathematical object

A good primitive function has a simple interface:

```
distance = primitive(point);
```

The caller does not need to know how the primitive calculates its distance.

A sphere can remain:

```
float sphere(float3 p, float radius)
{
    return length(p) - radius;
}
```

The scene can then decide where to evaluate it by transforming `p`.

This separation gives procedural graphics a useful architecture:

```
coordinate transformation
        ↓
primitive function
        ↓
distance value
        ↓
further mathematical operations
```

The primitive describes geometry.

The transformed coordinate describes where and how that geometry exists.

Later operations can combine the resulting fields into increasingly complex
objects.

## From equations to geometry

The important shift in this lesson is conceptual.

A circle is no longer something that must be drawn.

It can be represented as:

```
length(p) - radius
```

A sphere is the same idea in three dimensions.

A plane can be represented directly by its signed coordinate.

A box can be described by comparing coordinates against its extents.

The equations do not contain an image.

They contain a rule for evaluating space.

When the shader evaluates that rule over many points, the geometry becomes
visible.

That is procedural geometry at its core.

## The central mental model

An SDF converts a geometric question into a scalar field.

Given a point:

```
point -> distance field -> signed distance
```

The sign identifies which side of the surface the point occupies.

The magnitude describes its distance from the surface.

The surface itself is where the value reaches zero.

Primitive shapes therefore do not need to be stored as collections of
vertices. They can be defined by functions that describe their relationship
with every point in space.

The next step is to stop treating those fields as isolated shapes and learn
how scalar operations can combine them into larger forms.

## Next step

Now type the code version of this lesson.
