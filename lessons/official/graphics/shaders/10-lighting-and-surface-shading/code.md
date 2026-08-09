# Lighting and surface shading - typing

This lesson types surface lighting: calculate normals, construct light and
view directions, measure diffuse alignment, and add specular reflection.

## Establish the surface

Begin with a point on a sphere and derive its normal from the surface position.

```slang id="p3x7mc"
// read the current surface position
float3 position = point;

// calculate the outward direction from the sphere center
float3 normal = normalize(position);
```

The normal now describes the orientation of the surface at this point.

## Construct the light direction

Create the direction from the surface toward a point light.

```slang id="k8r2vn"
// define the position of the point light
float3 lightPosition = float3(2.0, 3.0, 2.0);

// calculate the vector from the surface to the light
float3 lightVector = lightPosition - position;

// normalize the light direction
float3 lightDirection = normalize(lightVector);
```

The direction changes across the surface because every point has a different
vector toward the light.

## Calculate diffuse lighting

Compare the normal and light direction using their dot product.

```slang id="w5m9qd"
// measure how directly the surface faces the light
float diffuse = dot(normal, lightDirection);

// remove illumination from the back-facing side
diffuse = max(diffuse, 0.0);
```

The result is a scalar illumination field over the surface.

## Add distance attenuation

Make the point light weaker as the surface moves away from it.

```slang id="f4c7xp"
// calculate the distance from the surface to the light
float distance = length(lightVector);

// calculate inverse-square attenuation
float attenuation = 1.0 / (distance * distance);

// apply distance attenuation to diffuse lighting
diffuse *= attenuation;
```

Distance is now another input to the lighting calculation.

## Construct the view direction

Calculate the direction from the surface toward the camera.

```slang id="n6v2ka"
// define the camera position
float3 cameraPosition = float3(0.0, 0.0, 3.0);

// calculate the vector from the surface to the camera
float3 viewVector = cameraPosition - position;

// normalize the view direction
float3 viewDirection = normalize(viewVector);
```

The shader now knows both the direction of illumination and the direction of
observation.

## Calculate the reflection

Reflect the incoming light direction around the surface normal.

```slang id="q9m4bt"
// calculate the reflected light direction
float3 reflection = reflect(-lightDirection, normal);
```

The negative sign converts the surface-to-light direction into the incident
direction expected by the reflection operation.

## Calculate the specular highlight

Compare the reflection direction with the view direction.

```slang id="r7k3zf"
// measure alignment between reflection and viewer
float highlight = dot(reflection, viewDirection);

// prevent negative specular values
highlight = max(highlight, 0.0);

// define the concentration of the highlight
float shininess = 32.0;

// sharpen the highlight with a power curve
float specular = pow(highlight, shininess);
```

The exponent determines how concentrated the reflected highlight becomes.

## Combine the lighting terms

Add diffuse, specular, and a small ambient contribution.

```slang id="c5x8qn"
// define the minimum ambient illumination
float ambient = 0.08;

// combine ambient and diffuse illumination
float lighting = ambient + diffuse;

// add the specular highlight
lighting += specular;
```

Each term represents a different part of the surface response.

## Apply the surface color

Use the lighting result to illuminate a material color.

```slang id="m2v6rd"
// define the base surface color
float3 surfaceColor = float3(0.2, 0.45, 0.8);

// multiply the material color by the lighting
float3 color = surfaceColor * lighting;

// return the lit surface
return float4(color, 1.0);
```

The material color and lighting calculation remain separate.

## Use a half-vector

The same general specular idea can be expressed with the halfway direction.

```slang id="z8q4kp"
// combine light and view directions
float3 halfVector = normalize(lightDirection + viewDirection);

// measure normal alignment with the half-vector
float halfAlignment = max(dot(normal, halfVector), 0.0);

// concentrate the half-vector response
float halfSpecular = pow(halfAlignment, shininess);
```

The half-vector lies between the light and viewing directions. A surface whose
normal aligns with it produces a strong specular response.

## Complete lighting calculation

Assemble the main lighting model into one coherent sequence.

```slang id="v4m8sx"
// read the current surface position
float3 position = point;

// calculate the outward direction from the sphere center
float3 normal = normalize(position);

// define the position of the point light
float3 lightPosition = float3(2.0, 3.0, 2.0);

// calculate the vector from the surface to the light
float3 lightVector = lightPosition - position;

// normalize the light direction
float3 lightDirection = normalize(lightVector);

// calculate the distance from the surface to the light
float distance = length(lightVector);

// calculate inverse-square attenuation
float attenuation = 1.0 / (distance * distance);

// measure how directly the surface faces the light
float diffuse = max(dot(normal, lightDirection), 0.0);

// apply distance attenuation to diffuse lighting
diffuse *= attenuation;

// define the camera position
float3 cameraPosition = float3(0.0, 0.0, 3.0);

// calculate the vector from the surface to the camera
float3 viewVector = cameraPosition - position;

// normalize the view direction
float3 viewDirection = normalize(viewVector);

// calculate the reflected light direction
float3 reflection = reflect(-lightDirection, normal);

// measure alignment between reflection and viewer
float highlight = max(dot(reflection, viewDirection), 0.0);

// define the concentration of the highlight
float shininess = 32.0;

// sharpen the highlight with a power curve
float specular = pow(highlight, shininess);

// define the minimum ambient illumination
float ambient = 0.08;

// combine all illumination terms
float lighting = ambient + diffuse + specular;

// define the base surface color
float3 surfaceColor = float3(0.2, 0.45, 0.8);

// multiply the material color by the lighting
float3 color = surfaceColor * lighting;

// return the lit surface
return float4(color, 1.0);
```

The calculation flows from geometry to directions, from directions to scalar
lighting terms, and finally from lighting to color.

## Now type it again

Re-drill the core diffuse calculation.

```slang id="t6q2nw"
// calculate the vector from the surface to the light
float3 lightVector = lightPosition - position;

// normalize the light direction
float3 lightDirection = normalize(lightVector);

// measure how directly the surface faces the light
float diffuse = dot(normal, lightDirection);

// remove illumination from the back-facing side
diffuse = max(diffuse, 0.0);
```

Then drill the reflection and specular calculation.

```slang id="h3v7pk"
// calculate the reflected light direction
float3 reflection = reflect(-lightDirection, normal);

// measure alignment between reflection and viewer
float highlight = max(dot(reflection, viewDirection), 0.0);

// define the concentration of the highlight
float shininess = 32.0;

// sharpen the highlight with a power curve
float specular = pow(highlight, shininess);
```

Finally, rebuild the complete lighting-to-color chain.

```slang id="q8m5xd"
// define the minimum ambient illumination
float ambient = 0.08;

// combine all illumination terms
float lighting = ambient + diffuse + specular;

// define the base surface color
float3 surfaceColor = float3(0.2, 0.45, 0.8);

// multiply the material color by the lighting
float3 color = surfaceColor * lighting;

// return the lit surface
return float4(color, 1.0);
```

## Wrap up

The flow: surface position -> normal -> light/view directions -> dot products
-> diffuse/specular -> lighting -> surface color.

Lighting is geometry evaluated through vectors: the shader repeatedly asks how
the surface is oriented relative to the light and the observer.

