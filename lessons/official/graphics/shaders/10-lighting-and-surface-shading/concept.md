# Lighting and surface shading - concepts

A geometric surface only tells us where something exists. It does not tell us
how bright that surface should appear.

Lighting adds another mathematical layer. Given a surface position, a surface
normal, a light direction, and a viewing direction, the shader can estimate
how the surface responds to illumination.

The important idea is that lighting is mostly geometry expressed through
vectors. The shader compares directions using dot products, constructs
reflection directions from those vectors, and turns the resulting scalar
values into visible intensity.

## The surface normal

A surface normal is a vector perpendicular to a surface at a particular point.

For a circle centered at the origin, the vector from the center to a surface
point already points outward:

```slang id="n8q3mv"
float2 normal = normalize(p);
```

Normalization makes the vector's length equal to one while preserving its
direction.

For a sphere, the same idea works in three dimensions:

```slang id="j4t7kp"
float3 normal = normalize(position);
```

The normal does not describe the color of the surface. It describes its local
orientation.

That orientation is the information lighting calculations need.

## Why direction matters

Imagine a flat surface facing directly toward a light. The light rays arrive
perpendicular to the surface.

Now rotate the surface. The same light arrives at an angle, so the surface
receives less direct illumination.

The shader can measure this relationship with a dot product:

```slang id="q6m2xz"
float lightAmount = dot(normal, lightDirection);
```

When both vectors point in the same direction, the dot product is large. When
they are perpendicular, it is zero. When they point in opposite directions,
it is negative.

This gives the dot product a direct geometric meaning: it measures directional
alignment.

## Why normalize first

The geometric interpretation of a dot product as an alignment measure assumes
that the vectors have known lengths, normally one.

```slang id="v9c5rf"
float3 normal = normalize(surfaceNormal);
float3 lightDirection = normalize(lightPosition - position);
```

The subtraction creates a vector pointing from the surface toward the light.
Normalization removes its distance information so the dot product measures
directional alignment rather than combining direction and arbitrary vector
length.

This distinction matters because lighting intensity and light distance are
separate concepts.

## Clamp the diffuse response

A surface facing away from the light should not receive negative diffuse
illumination.

```slang id="m3w7qa"
float diffuse = max(dot(normal, lightDirection), 0.0);
```

The dot product produces the geometric response. `max` converts the negative
half into zero.

This is commonly called a Lambertian or diffuse response.

The result is a scalar field over the surface. Every point can have a different
normal, so every point can produce a different lighting value.

## A sphere demonstrates the model

Consider a sphere centered at the origin.

```slang id="r5k8yd"
float3 position = point;
float3 normal = normalize(position);
```

At the center-facing point closest to the light, the normal and light
direction align strongly. Toward the side of the sphere, the normal becomes
more perpendicular to the light direction.

The diffuse calculation therefore produces a bright region that gradually
falls toward the sides:

```slang id="b2f6xn"
float diffuse = max(dot(normal, lightDirection), 0.0);
```

No texture is required to create this gradient. It emerges from the geometry
and the relationship between two vectors.

## Position determines the light direction

A directional light uses one constant direction across the entire surface.

A point light is different. Every surface point has its own direction toward
the light.

```slang id="c7p4mw"
float3 lightDirection = normalize(lightPosition - position);
```

The subtraction is important. It constructs the vector from the current
surface point to the light.

Two points on the same object can therefore receive different illumination
even when their normals are identical, because the light direction changes
with position.

This is how a local point light differs mathematically from a distant
directional light.

## Distance attenuation

A point light can also become weaker as distance increases.

The squared distance can be calculated as:

```slang id="k3v9sd"
float distance = length(lightPosition - position);
```

A simple attenuation model is inverse square:

```slang id="p8m4hj"
float attenuation = 1.0 / (distance * distance);
```

The intensity decreases rapidly as the surface moves farther from the light.

Real rendering systems often use additional coefficients or physically based
models, but the mathematical idea is the same: distance can become another
input to the lighting function.

## The view direction

Diffuse lighting only considers the surface and the light. To model a
highlight, the shader also needs to know where the viewer is.

Construct the view direction from the surface toward the camera:

```slang id="x5r2bn"
float3 viewDirection = normalize(cameraPosition - position);
```

Now the shader has three important directions:

```slang id="w7q9kc"
float3 normal = normalize(surfaceNormal);
float3 lightDirection = normalize(lightPosition - position);
float3 viewDirection = normalize(cameraPosition - position);
```

The normal describes the surface. The light direction describes illumination.
The view direction describes observation.

Specular lighting uses all three.

## Reflection direction

A mirror reflects an incoming direction around a surface normal.

The reflection vector can be calculated with:

```slang id="h6c3vz"
float3 reflection = reflect(-lightDirection, normal);
```

The negative sign matters because `lightDirection` points from the surface
toward the light, while `reflect` expects the incident direction traveling
toward the surface.

The resulting vector points in the direction that the light would reflect.

This is another example of a visual effect reducing to vector mathematics.

## Specular highlights

A surface produces a strong specular highlight when the reflected light points
toward the viewer.

The dot product measures that alignment:

```slang id="t4m8qx"
float highlight = max(dot(reflection, viewDirection), 0.0);
```

If the reflection points directly toward the viewer, the value approaches one.
As the viewer moves away from the reflection direction, the value decreases.

The highlight can then be sharpened with a power function:

```slang id="s9n5wk"
float specular = pow(highlight, shininess);
```

A high shininess value produces a narrow highlight. A lower value produces a
broader one.

The exponent therefore controls the distribution of reflected intensity.

## Diffuse and specular are different information

Diffuse lighting describes how strongly the surface faces the light.

Specular lighting describes how closely the reflected light direction aligns
with the viewer.

They should therefore be treated as separate terms:

```slang id="a7d2rf"
float diffuse = max(dot(normal, lightDirection), 0.0);
float specular = pow(max(dot(reflection, viewDirection), 0.0), shininess);
```

The final appearance can combine them:

```slang id="e5k8cp"
float lighting = diffuse + specular;
```

The surface color can then scale that lighting:

```slang id="y3m6vt"
float3 color = surfaceColor * lighting;
```

The geometric response and the material color remain conceptually distinct.

## Ambient illumination

Pure diffuse lighting produces completely dark regions when the light is
behind the surface.

A simple ambient term prevents the entire unlit side from reaching zero:

```slang id="f8q2jm"
float lighting = ambient + diffuse;
```

Ambient light is not a directional calculation. It is a simplified approximation
of illumination coming from the surrounding environment.

More advanced rendering replaces this with techniques such as image-based
lighting and ambient occlusion, but the basic mathematical role remains:
provide illumination that does not depend directly on one light direction.

## The half-vector

There is another useful way to describe specular alignment.

Instead of explicitly reflecting the light direction, construct the direction
halfway between the light and viewer:

```slang id="d6v4px"
float3 halfVector = normalize(lightDirection + viewDirection);
```

Then compare the surface normal to that half-vector:

```slang id="z2r8kf"
float specular = pow(max(dot(normal, halfVector), 0.0), shininess);
```

This is the basis of many common specular models.

Geometrically, when the normal points toward the halfway direction between
the viewer and light, the surface is oriented correctly to produce a strong
highlight.

The half-vector is therefore another way of expressing the same directional
relationship.

## Lighting is a field over geometry

The important mental model is that lighting is not a property of the entire
object.

Each surface point has a position and orientation. The shader evaluates the
lighting equations independently at that point.

```slang id="g9w5qn"
float3 lightDirection = normalize(lightPosition - position);
float diffuse = max(dot(normal, lightDirection), 0.0);
```

Move to another surface point and both the normal and light direction may
change. The lighting result changes with them.

This is why a sphere can produce a smooth gradient from a handful of equations.
The equations are evaluated at many different positions across the surface.

## Lighting as function composition

A useful way to think about the entire calculation is as a chain.

First, geometry provides the surface position and normal. Directions are then
constructed from the light and camera positions. Dot products turn directional
relationships into scalar values. Those scalars are shaped and combined into
illumination.

```slang id="u4c8sd"
float diffuse = max(dot(normal, lightDirection), 0.0);
float highlight = max(dot(reflection, viewDirection), 0.0);
float specular = pow(highlight, shininess);
```

Finally, illumination is mapped onto the surface color.

```slang id="p5k3wy"
float3 color = surfaceColor * (ambient + diffuse + specular);
```

The apparent complexity of lighting comes from composing these simple
geometric relationships.

## The deeper mental model

Lighting is fundamentally about comparing directions.

The normal says which way the surface faces. The light direction says where
illumination comes from. The view direction says where the observer is.
Dot products measure alignment, reflection constructs the direction of
specular reflection, and power curves control the concentration of highlights.

The complete conceptual pipeline is:

```slang id="m7q2xf"
surface -> normal -> light/view directions -> dot products -> lighting -> color
```

Once this becomes intuitive, lighting stops looking like a collection of
special formulas. Each equation answers a geometric question about the
surface.

## Next step

Now type the code version of this lesson.

