# Shader overview - concepts

A shader is a program that describes how to produce part of an image or
other graphical result. The important idea is not that a shader is a special
kind of syntax. The important idea is that a shader turns a visual rule into
a mathematical function that can be evaluated many times in parallel.

A normal program often starts with one problem and executes a sequence of
instructions to produce one result. A shader usually starts with a large
collection of independent work items and applies the same general program to
each one. A pixel shader, for example, can receive the position and other data
for one pixel and calculate that pixel's final color without needing to know
what another pixel is doing.

This changes how visual problems are approached. Instead of thinking
"draw this complicated picture", shader programming often asks "what
mathematical function should produce the value at this location?"

## The GPU sees many independent invocations

Consider an image that is 1920 by 1080 pixels. There are more than two million
pixel locations in that image. A fragment or compute shader can be invoked for
a huge number of those locations, with each invocation working on its own
input data.

The same shader code might conceptually be evaluated like this:

```
color(0, 0)
color(1, 0)
color(2, 0)
...
color(1919, 1079)
```

The important part is that these are not separate programs that the developer
has to write. They are separate invocations of the same program.

That independence is one of the central reasons GPUs are useful for graphics.
The hardware is designed to execute enormous numbers of similar operations
concurrently. A shader therefore works best when the result for one invocation
can be calculated mostly from the data belonging to that invocation.

For a pixel shader, the question is usually something like:

"Given the position of this pixel and the scene information available to me,
what should this pixel look like?"

The answer is a calculation.

## A picture can become a function

Suppose we want the center of an image to be white and the rest of the image
to become darker as it gets farther away.

We do not need to manually specify the color of every pixel. We can describe
the rule mathematically.

Let a point on the image be represented by a coordinate p. Let the center be
c. The distance from the point to the center is:

```
d = length(p - c)
```

Now the distance itself becomes useful information. We can turn it into a
color:

```
brightness = 1 - d
```

The exact function is not important yet. What matters is the transformation
of the problem.

We started with a visual statement:

"Make a bright circle-like region around the center."

We turned that into:

"Calculate the distance from the current position to the center and map that
distance to brightness."

The shader does not understand the concept of a glowing center in the way a
human does. It evaluates the mathematical relationship.

This is the foundation of procedural graphics.

## Coordinates tell the shader where it is

A shader needs some description of the location associated with its current
invocation. Without a location, there is no way to make the result vary
spatially.

For an image, a natural coordinate is the pixel position.

A 2D position might look like:

```
float2 pixel = float2(x, y);
```

For a 1920 by 1080 image, the coordinate range in pixel space is roughly:

```
x = 0 ... 1919
y = 0 ... 1079
```

Those coordinates are useful, but they are tied to the image resolution.
A circle described using a radius of 100 pixels will have a different
relative size in a 640 by 480 image than it will in a 4K image.

Shaders therefore commonly transform coordinates into more useful spaces.

One useful representation is normalized coordinates:

```
float2 uv = pixel / imageSize;
```

Now the coordinates describe a relative position in the image instead of a
specific number of pixels.

The upper-left corner is approximately:

```
(0, 0)
```

and the opposite corner is approximately:

```
(1, 1)
```

The exact orientation and conventions depend on the rendering system, but
the important idea is that coordinates can be transformed into a space where
the mathematics is easier to reason about.

## A coordinate is more than a location

A coordinate is not merely information that tells us where a pixel happens to
be. It is an input to a mathematical function.

Imagine a shader that receives a coordinate p and produces a color:

```
color = f(p)
```

This tiny expression captures an enormous amount of procedural graphics.

If f produces a gradient, the image becomes a gradient.

If f measures distance to a point, the image can become a circle.

If f measures distance to several shapes, the image can describe a collection
of objects.

If f uses sine and cosine, the image can contain waves.

If f samples noise, the image can contain irregular variation.

If f evaluates lighting equations, the image can describe how a surface
responds to light.

The shader is therefore not necessarily storing the picture. It is storing
the rule from which the picture can be calculated.

## Geometry can be reduced to mathematics

A particularly useful way to think about shader programming is to ask how a
geometric idea can be represented numerically.

A point is a coordinate.

A direction is a vector.

The distance between two points is a scalar.

A circle can be described using the distance from its center.

A plane can be described using a point and a normal.

A sphere can be described using the distance from its center.

For example, suppose p is the current position and c is the center of a
sphere with radius r. The signed distance to the sphere can be expressed as:

```
d = length(p - c) - r
```

The equation gives us more than a simple inside-or-outside answer.

When d is positive, the point is outside the sphere.

When d is zero, the point lies on the surface.

When d is negative, the point is inside the sphere.

That single scalar value can therefore describe the relationship between a
point and an entire geometric object.

This idea becomes extremely powerful later when distance fields are used to
construct scenes entirely from mathematical functions.

## Visual effects are transformations of values

A shader rarely jumps directly from "position" to "beautiful image". Instead,
it transforms useful values through several stages.

A typical conceptual chain might look like:

```
position -> distance -> shape -> lighting -> color
```

Each stage changes the meaning of the value.

A position tells us where we are.

A distance tells us how far we are from something.

A shape function tells us how that distance relates to a surface.

A lighting calculation tells us how that surface should respond to light.

A color mapping turns the resulting scalar or vector values into visible
output.

This decomposition is important because complicated effects can be understood
as compositions of smaller mathematical operations.

For example, a glowing sphere might involve:

```
position -> sphere distance -> glow falloff -> color
```

A water-like surface might involve:

```
position -> waves -> displaced surface -> normal -> lighting -> color
```

The visual complexity comes from combining relatively simple mathematical
relationships.

## Vectors describe the geometry

Most interesting shader mathematics operates on vectors.

A 2D position can be represented as a vector:

```
float2 p;
```

A 3D position can be represented as:

```
float3 p;
```

A vector contains several related numerical components that describe something
in space.

For example:

```
float3 p = float3(2.0, 4.0, 1.0);
```

can represent a point in three-dimensional space.

We can subtract two positions:

```
float3 direction = target - p;
```

The result is a vector pointing from p toward target.

We can calculate its length:

```
float distance = length(direction);
```

Now we have a scalar describing how far apart the two positions are.

We can normalize it:

```
float3 direction = normalize(target - p);
```

Now the vector represents direction without retaining its original magnitude.

These operations are basic building blocks for almost every serious shader
effect. Later lessons will build the geometric intuition behind them rather
than treating them as formulas to memorize.

## The dot product measures alignment

Another fundamental operation is the dot product.

Given two vectors a and b:

```
float alignment = dot(a, b);
```

When both vectors are normalized, the result describes how closely their
directions align.

A value near 1 means they point in nearly the same direction.

A value near 0 means they are roughly perpendicular.

A value near -1 means they point in opposite directions.

This becomes particularly useful for lighting.

Suppose n is a surface normal and l is a direction toward a light. Their dot
product tells us how directly the surface faces that light.

```
float brightness = max(dot(n, l), 0.0);
```

A surface facing the light receives a large value.

A surface turned away from the light receives zero after the clamp.

Again, a visual phenomenon has been reduced to a mathematical relationship.

## The same mathematics can describe many effects

One of the most useful properties of shader mathematics is that the same
operations repeatedly appear in different visual contexts.

Distance can describe a circle, a sphere, a glow, a shadow falloff, or the
relationship between a ray and a surface.

A dot product can describe lighting, angles, projection, visibility, or
directional relationships.

Sine and cosine can describe waves, rotations, oscillations, patterns, and
periodic motion.

Interpolation can blend colors, positions, shapes, and animation states.

Modulo can repeat a coordinate and turn one local pattern into an infinite
arrangement.

The goal of learning shader mathematics is therefore not to memorize a list
of tricks. It is to understand what these operations do to space and values
so that they can be recombined into new effects.

## The shader does not need to know the whole picture

A useful mental model is to imagine that a shader is asked the same question
at many different locations.

For a simple image:

```
What color belongs at this coordinate?
```

The shader receives a coordinate, performs its calculations, and produces an
answer.

At another coordinate, the same program runs again with different input.

This is fundamentally different from manually constructing a bitmap in which
every pixel is individually chosen and stored.

A procedural shader describes a relationship.

The image is the visible consequence of evaluating that relationship over a
domain of coordinates.

This is why a surprisingly small amount of shader code can produce an
enormously detailed image.

## Parallelism changes how problems should be designed

The GPU's parallel execution model also imposes constraints.

A shader should not normally depend on the previous pixel having finished.
If the result for pixel A must be calculated before pixel B can continue,
those operations are no longer naturally independent.

Independent calculations are ideal because the GPU can execute many of them
at once.

For example, calculating the color of every pixel from its own coordinate is
highly parallel:

```
color = f(pixelPosition);
```

Every invocation can evaluate f independently.

By contrast, an algorithm that repeatedly modifies one shared value and
requires a strict sequence of operations does not map as naturally onto this
model.

This does not mean GPUs cannot perform dependent algorithms. It means that
shader programming becomes much more powerful when the problem can be
expressed as many related evaluations rather than one long sequential
calculation.

## Different shader stages answer different questions

The word shader describes a family of programs rather than one specific
operation.

A vertex shader generally determines what happens to individual vertices.

A fragment or pixel shader determines the value associated with rasterized
fragments.

A compute shader performs general-purpose GPU work without requiring the
traditional vertex-to-fragment graphics pipeline.

The details of how these stages connect will depend on the graphics API and
pipeline being used, but the mathematical idea remains consistent.

A shader receives inputs, transforms them through calculations, and produces
outputs that become inputs to another stage or become the final result.

The important distinction is therefore not "shader means this syntax".

It is:

"shader means a GPU program executed according to a particular stage's
parallel work model."

## From coordinates to an image

We can now describe the basic procedural graphics pipeline as a mathematical
idea.

The GPU identifies a work item.

That work item provides a coordinate or other input.

The shader transforms that input through mathematical operations.

The resulting values determine something visible or useful.

For an image, the simplest version is:

```
coordinate -> mathematical function -> color
```

A more complex effect might become:

```
coordinate
    -> transformed space
    -> geometric function
    -> distance
    -> surface information
    -> lighting
    -> color
```

The later lessons in this track progressively build each part of this chain.

We will first develop the vector and trigonometric mathematics required to
manipulate coordinates. Then we will use those tools to describe geometry,
combine shapes, transform spaces, distort domains, generate variation,
calculate lighting, and eventually trace rays through entire mathematical
scenes.

## Why shaders are mathematical

It is tempting to approach shaders by collecting visual recipes:

"Use this formula for a circle."

"Use this noise function for clouds."

"Use this trick for water."

That approach can produce effects, but it does not explain why the effects
work or how to invent variations.

A stronger approach is to understand the mathematical representation behind
the effect.

A circle is not a special shader object. It can be represented through a
distance relationship.

A glow is not a special rendering primitive. It can be represented as a
function whose value changes with distance.

A wave is not a special texture. It can be represented by a periodic function.

A repeating pattern is not necessarily a collection of duplicated objects.
Coordinates themselves can be transformed so that one mathematical region
represents many regions.

Once these representations become intuitive, shader programming becomes less
about searching for effects and more about constructing them.

## The central mental model

The most important idea in this lesson can be reduced to one question:

"What mathematical value should exist at this location?"

The location might be a pixel, a vertex, a point in space, or a compute
invocation.

The answer might depend on distance, direction, angle, time, noise, geometry,
or information supplied by the application.

The shader evaluates that relationship independently across many invocations.

The visual effect emerges from the collection of those evaluations.

In that sense, a shader is a mathematical description of a visual process.
The GPU supplies enormous amounts of parallel evaluation, while mathematics
provides the rules that determine what each invocation produces.

That is the foundation for everything that follows.

## Next step

Now type the code version of this lesson.

