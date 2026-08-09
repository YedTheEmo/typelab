# Raymarching fundamentals - typing

This lesson types raymarching: construct a camera ray, evaluate an SDF, advance
by safe distances, detect hits and misses, and recover a surface normal.

## Define the sphere

Start with the distance function that describes the object.

```slang id="c4m8qx"
// calculate the signed distance to a sphere
float sphereSDF(float3 point, float radius) {
    // measure distance from the sphere center
    float distance = length(point);

    // move the surface outward by the radius
    return distance - radius;
}
```

The function returns the distance needed by the raymarcher.

## Define the scene

Wrap the object in a scene function.

```slang id="v7p3nk"
// describe the complete scene
float scene(float3 point) {
    // evaluate the sphere distance
    float distance = sphereSDF(point, 0.75);

    // return the nearest scene surface
    return distance;
}
```

The raymarcher only needs one function representing the entire scene.

## Construct the camera ray

Create a direction for the current pixel.

```slang id="m5q9rx"
// define the camera position
float3 rayOrigin = float3(0.0, 0.0, 3.0);

// define the current point on the camera plane
float3 screenPoint = float3(pixel.x, pixel.y, 0.0);

// calculate the direction from the camera through the pixel
float3 rayDirection = normalize(screenPoint - rayOrigin);
```

Every pixel now has a ray entering the scene from the camera.

## Set the marching limits

Define the numerical limits for the search.

```slang id="k2v6sd"
// define the maximum number of raymarch iterations
int maxSteps = 128;

// define the minimum distance considered a surface hit
float surfaceEpsilon = 0.001;

// define the maximum distance the ray may travel
float maxDistance = 10.0;

// initialize the accumulated ray distance
float distance = 0.0;
```

These values control accuracy, range, and computation cost.

## March through the scene

Evaluate the SDF and use its result as the step size.

```slang id="q8r4mx"
// begin the bounded raymarch loop
for (int step = 0; step < maxSteps; step++) {
    // calculate the current point along the ray
    float3 point = rayOrigin + rayDirection * distance;

    // evaluate the nearest scene distance
    float sceneDistance = scene(point);

    // stop when the ray reaches a surface
    if (sceneDistance < surfaceEpsilon) {
        break;
    }

    // advance by the safe distance returned by the scene
    distance += sceneDistance;

    // stop when the ray leaves the useful scene range
    if (distance > maxDistance) {
        break;
    }
}
```

The distance field determines how far the ray advances on every iteration.

## Record a hit

Track whether the loop actually reached a surface.

```slang id="n3w7kp"
// start with no surface hit
bool hit = false;

// reset the accumulated ray distance
distance = 0.0;

// begin the bounded raymarch loop
for (int step = 0; step < maxSteps; step++) {
    // calculate the current point along the ray
    float3 point = rayOrigin + rayDirection * distance;

    // evaluate the nearest scene distance
    float sceneDistance = scene(point);

    // record a surface hit when the distance is small enough
    if (sceneDistance < surfaceEpsilon) {
        hit = true;

        // stop searching after finding the surface
        break;
    }

    // advance by the safe distance
    distance += sceneDistance;

    // stop when the ray leaves the scene
    if (distance > maxDistance) {
        break;
    }
}
```

The boolean separates a successful intersection from a ray that simply
exhausted its search.

## Recover the hit point

Use the accumulated ray distance to reconstruct the surface position.

```slang id="r6p2vd"
// calculate the final point reached by the ray
float3 hitPoint = rayOrigin + rayDirection * distance;
```

The hit point is the bridge between raymarching and surface shading.

## Estimate the normal

Use central differences to recover the gradient of the distance field.

```slang id="t9k5qm"
// define the finite difference size
float epsilon = 0.001;

// measure the distance change along the x axis
float dx = scene(hitPoint + float3(epsilon, 0.0, 0.0))
         - scene(hitPoint - float3(epsilon, 0.0, 0.0));

// measure the distance change along the y axis
float dy = scene(hitPoint + float3(0.0, epsilon, 0.0))
         - scene(hitPoint - float3(0.0, epsilon, 0.0));

// measure the distance change along the z axis
float dz = scene(hitPoint + float3(0.0, 0.0, epsilon))
         - scene(hitPoint - float3(0.0, 0.0, epsilon));

// assemble the distance field gradient
float3 gradient = float3(dx, dy, dz);

// normalize the gradient into a surface normal
float3 normal = normalize(gradient);
```

The gradient converts the scalar distance field into the directional
information needed for lighting.

## Light the raymarched surface

Use the normal with the lighting mathematics from the previous lesson.

```slang id="b7x4nf"
// define the position of the light
float3 lightPosition = float3(2.0, 3.0, 2.0);

// calculate the direction from the surface to the light
float3 lightDirection = normalize(lightPosition - hitPoint);

// calculate diffuse alignment
float diffuse = max(dot(normal, lightDirection), 0.0);

// define the base surface color
float3 surfaceColor = float3(0.2, 0.45, 0.8);

// apply diffuse lighting to the surface
float3 color = surfaceColor * diffuse;
```

Raymarching found the surface; the lighting equations determine how it
appears.

## Color hits and misses

Give successful rays a surface color and unsuccessful rays a background.

```slang id="x5m8qc"
// define the background color
float3 background = float3(0.02, 0.03, 0.05);

// choose the surface color when the ray hits
float3 finalColor = hit ? color : background;

// return the rendered pixel
return float4(finalColor, 1.0);
```

The hit test now controls whether the discovered geometry or the empty scene
is displayed.

## Combine the raymarch

Assemble the complete geometric search.

```slang id="p6r3vk"
// define the camera position
float3 rayOrigin = float3(0.0, 0.0, 3.0);

// calculate the camera ray direction
float3 rayDirection = normalize(
    float3(pixel.x, pixel.y, 0.0) - rayOrigin
);

// define the maximum number of raymarch iterations
int maxSteps = 128;

// define the surface hit threshold
float surfaceEpsilon = 0.001;

// define the maximum ray distance
float maxDistance = 10.0;

// initialize the ray distance
float distance = 0.0;

// start with no surface hit
bool hit = false;

// begin the raymarch loop
for (int step = 0; step < maxSteps; step++) {
    // calculate the current point along the ray
    float3 point = rayOrigin + rayDirection * distance;

    // evaluate the scene distance
    float sceneDistance = scene(point);

    // stop when the surface is reached
    if (sceneDistance < surfaceEpsilon) {
        hit = true;

        // stop searching after finding a surface
        break;
    }

    // advance by the safe scene distance
    distance += sceneDistance;

    // stop when the ray leaves the scene
    if (distance > maxDistance) {
        break;
    }
}
```

The loop is the core of the entire technique.

## Recover and shade the surface

Finish the pipeline by deriving a normal and applying diffuse lighting.

```slang id="w8q4nz"
// reconstruct the surface point
float3 hitPoint = rayOrigin + rayDirection * distance;

// define the normal estimation distance
float epsilon = 0.001;

// calculate the distance gradient
float3 gradient = float3(
    scene(hitPoint + float3(epsilon, 0.0, 0.0))
        - scene(hitPoint - float3(epsilon, 0.0, 0.0)),
    scene(hitPoint + float3(0.0, epsilon, 0.0))
        - scene(hitPoint - float3(0.0, epsilon, 0.0)),
    scene(hitPoint + float3(0.0, 0.0, epsilon))
        - scene(hitPoint - float3(0.0, 0.0, epsilon))
);

// normalize the gradient into a surface normal
float3 normal = normalize(gradient);

// define the light position
float3 lightPosition = float3(2.0, 3.0, 2.0);

// calculate the direction toward the light
float3 lightDirection = normalize(lightPosition - hitPoint);

// calculate diffuse lighting
float diffuse = max(dot(normal, lightDirection), 0.0);

// define the surface color
float3 surfaceColor = float3(0.2, 0.45, 0.8);

// apply the lighting to the surface
float3 color = surfaceColor * diffuse;
```

The distance field has now become a visible, lit surface.

## Now type it again

Re-drill the fundamental ray equation and scene step.

```slang id="d5m9xp"
// calculate the current point along the ray
float3 point = rayOrigin + rayDirection * distance;

// evaluate the nearest scene distance
float sceneDistance = scene(point);

// stop when the ray reaches a surface
if (sceneDistance < surfaceEpsilon) {
    hit = true;

    // stop searching after finding the surface
    break;
}

// advance by the safe distance
distance += sceneDistance;
```

Then drill the connection between the hit and the normal.

```slang id="f7q2mk"
// reconstruct the surface point
float3 hitPoint = rayOrigin + rayDirection * distance;

// define the normal estimation distance
float epsilon = 0.001;

// calculate the distance gradient
float3 gradient = float3(
    scene(hitPoint + float3(epsilon, 0.0, 0.0))
        - scene(hitPoint - float3(epsilon, 0.0, 0.0)),
    scene(hitPoint + float3(0.0, epsilon, 0.0))
        - scene(hitPoint - float3(0.0, epsilon, 0.0)),
    scene(hitPoint + float3(0.0, 0.0, epsilon))
        - scene(hitPoint - float3(0.0, 0.0, epsilon))
);

// normalize the gradient into a surface normal
float3 normal = normalize(gradient);
```

Finally, rebuild the complete raymarching loop from memory.

```slang id="n4v8rc"
// initialize the ray distance
float distance = 0.0;

// start with no surface hit
bool hit = false;

// begin the raymarch loop
for (int step = 0; step < maxSteps; step++) {
    // calculate the current point along the ray
    float3 point = rayOrigin + rayDirection * distance;

    // evaluate the scene distance
    float sceneDistance = scene(point);

    // stop when the surface is reached
    if (sceneDistance < surfaceEpsilon) {
        hit = true;

        // stop searching after finding a surface
        break;
    }

    // advance by the safe scene distance
    distance += sceneDistance;

    // stop when the ray leaves the scene
    if (distance > maxDistance) {
        break;
    }
}
```

## Wrap up

The flow: pixel -> ray -> SDF -> safe step -> hit -> gradient -> normal ->
lighting.

Raymarching turns the distance returned by a mathematical scene description
into both a geometric search step and, after a hit, the information needed to
shade the discovered surface.

