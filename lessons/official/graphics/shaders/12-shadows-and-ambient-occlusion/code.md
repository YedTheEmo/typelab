# Shadows and ambient occlusion - typing

This lesson types secondary SDF queries: cast a shadow ray, estimate soft
visibility, sample nearby geometry, and combine direct and ambient lighting.

## Define the scene

The scene function supplies the distance used by every spatial query.

```slang
// describe the complete scene
float scene(float3 point) {
    // calculate the distance from the sphere center
    float sphereDistance = length(point) - 0.75;

    // return the nearest scene distance
    return sphereDistance;
}
```

## Estimate the surface normal

The normal identifies the exposed side of the raymarched surface.

```slang
// define the normal estimation distance
float normalEpsilon = 0.001;

// sample the scene along the x axis
float dx = scene(hitPoint + float3(normalEpsilon, 0.0, 0.0))
         - scene(hitPoint - float3(normalEpsilon, 0.0, 0.0));

// sample the scene along the y axis
float dy = scene(hitPoint + float3(0.0, normalEpsilon, 0.0))
         - scene(hitPoint - float3(0.0, normalEpsilon, 0.0));

// sample the scene along the z axis
float dz = scene(hitPoint + float3(0.0, 0.0, normalEpsilon))
         - scene(hitPoint - float3(0.0, 0.0, normalEpsilon));

// assemble the distance field gradient
float3 gradient = float3(dx, dy, dz);

// normalize the gradient into a surface normal
float3 normal = normalize(gradient);
```

## Build the shadow ray

The shadow ray travels from the surface toward a specific light.

```slang
// define the point light position
float3 lightPosition = float3(2.0, 3.0, 2.0);

// calculate the vector from the surface to the light
float3 lightVector = lightPosition - hitPoint;

// calculate the distance to the light
float lightDistance = length(lightVector);

// normalize the direction toward the light
float3 lightDirection = normalize(lightVector);

// offset the ray origin away from the surface
float3 shadowOrigin = hitPoint + normal * normalEpsilon * 2.0;
```

## March the shadow ray

The shadow ray uses the SDF to determine whether another surface blocks
the light.

```slang
// define the maximum number of shadow steps
int shadowSteps = 64;

// define the shadow visibility
float shadow = 1.0;

// initialize the distance traveled by the shadow ray
float travel = 0.0;

// march from the surface toward the light
for (int step = 0; step < shadowSteps; step++) {
    // calculate the current shadow ray position
    float3 point = shadowOrigin + lightDirection * travel;

    // evaluate the nearest scene surface
    float sceneDistance = scene(point);

    // stop when another surface blocks the light
    if (sceneDistance < normalEpsilon) {
        shadow = 0.0;

        // stop searching after finding an occluder
        break;
    }

    // advance by the safe scene distance
    travel += sceneDistance;

    // stop after reaching the light
    if (travel >= lightDistance) {
        break;
    }
}
```

## Calculate direct lighting

Direct illumination depends on both surface orientation and light visibility.

```slang
// calculate diffuse alignment with the light
float diffuse = max(dot(normal, lightDirection), 0.0);

// apply the shadow visibility to direct illumination
float direct = diffuse * shadow;
```

## Define ambient occlusion

Ambient occlusion measures how much nearby space is occupied by geometry.

```slang
// define the number of ambient occlusion samples
int aoSamples = 6;

// define the spacing between ambient samples
float aoSpacing = 0.08;

// initialize accumulated occlusion
float occlusion = 0.0;

// initialize accumulated sample weight
float totalWeight = 0.0;
```

## Accumulate nearby geometry

Compare each expected sample distance with the actual distance returned by
the scene.

```slang
// sample several distances above the surface
for (int step = 1; step <= aoSamples; step++) {
    // calculate the current sample distance
    float sampleDistance = step * aoSpacing;

    // move the sample outward along the normal
    float3 samplePoint = hitPoint + normal * sampleDistance;

    // evaluate the actual distance to nearby geometry
    float actual = scene(samplePoint);

    // calculate how much closer geometry is than expected
    float contribution = max(sampleDistance - actual, 0.0);

    // reduce the influence of distant samples
    float weight = 1.0 / (1.0 + sampleDistance);

    // accumulate the weighted occlusion
    occlusion += contribution * weight;

    // accumulate the sample weight
    totalWeight += weight;
}
```

## Convert occlusion into visibility

Normalize the samples and turn the occlusion amount into an illumination
factor.

```slang
// normalize the accumulated occlusion
occlusion /= totalWeight;

// prevent the occlusion value from exceeding one
occlusion = clamp(occlusion, 0.0, 1.0);

// convert occlusion into ambient visibility
float ambientVisibility = 1.0 - occlusion;
```

## Combine direct and ambient light

The two visibility calculations control different parts of the lighting.

```slang
// define the ambient illumination strength
float ambient = 0.12;

// calculate ambient illumination after occlusion
float indirect = ambient * ambientVisibility;

// combine direct and ambient illumination
float lighting = direct + indirect;
```

## Apply the material

Use the resulting lighting to shade the raymarched surface.

```slang
// define the surface material color
float3 surfaceColor = float3(0.2, 0.45, 0.8);

// apply the lighting to the surface color
float3 color = surfaceColor * lighting;

// return the lit surface
return float4(color, 1.0);
```

## Add soft shadow estimation

Soft shadows reduce visibility when the shadow ray passes close to geometry.

```slang
// define the maximum number of soft shadow steps
int shadowSteps = 64;

// define the shadow softness
float softness = 8.0;

// start with full shadow visibility
float shadow = 1.0;

// initialize the shadow ray distance
float travel = normalEpsilon * 2.0;

// march toward the light
for (int step = 0; step < shadowSteps; step++) {
    // calculate the current shadow ray position
    float3 point = hitPoint + normal * normalEpsilon * 2.0
        + lightDirection * travel;

    // evaluate the nearest scene surface
    float sceneDistance = scene(point);

    // calculate relative clearance around the ray
    float ratio = sceneDistance / max(travel, normalEpsilon);

    // reduce visibility near nearby geometry
    shadow = min(shadow, softness * ratio);

    // stop when the ray reaches a surface
    if (sceneDistance < normalEpsilon) {
        break;
    }

    // advance by the safe scene distance
    travel += sceneDistance;

    // stop after reaching the light
    if (travel >= lightDistance) {
        break;
    }
}
```

## Complete shadow calculation

Rebuild the shadow query as one continuous operation.

```slang
// define the point light position
float3 lightPosition = float3(2.0, 3.0, 2.0);

// calculate the vector from the surface to the light
float3 lightVector = lightPosition - hitPoint;

// calculate the distance to the light
float lightDistance = length(lightVector);

// normalize the direction toward the light
float3 lightDirection = normalize(lightVector);

// offset the shadow ray away from the surface
float3 shadowOrigin = hitPoint + normal * normalEpsilon * 2.0;

// define the shadow softness
float softness = 8.0;

// start with full shadow visibility
float shadow = 1.0;

// initialize the shadow ray distance
float travel = 0.0;

// march from the surface toward the light
for (int step = 0; step < shadowSteps; step++) {
    // calculate the current shadow ray position
    float3 point = shadowOrigin + lightDirection * travel;

    // evaluate the nearest scene surface
    float sceneDistance = scene(point);

    // calculate relative clearance around the ray
    float ratio = sceneDistance / max(travel, normalEpsilon);

    // reduce visibility near nearby geometry
    shadow = min(shadow, softness * ratio);

    // stop when another surface blocks the light
    if (sceneDistance < normalEpsilon) {
        shadow = 0.0;

        // stop searching after finding an occluder
        break;
    }

    // advance by the safe scene distance
    travel += sceneDistance;

    // stop after reaching the light
    if (travel >= lightDistance) {
        break;
    }
}

// keep the shadow inside its visibility range
shadow = clamp(shadow, 0.0, 1.0);
```

## Complete ambient occlusion

Rebuild the ambient query independently from the light.

```slang
// define the number of ambient occlusion samples
int aoSamples = 6;

// define the spacing between ambient samples
float aoSpacing = 0.08;

// initialize accumulated occlusion
float occlusion = 0.0;

// initialize accumulated sample weight
float totalWeight = 0.0;

// sample several distances above the surface
for (int step = 1; step <= aoSamples; step++) {
    // calculate the current sample distance
    float sampleDistance = step * aoSpacing;

    // move the sample outward along the normal
    float3 samplePoint = hitPoint + normal * sampleDistance;

    // evaluate the actual distance to nearby geometry
    float actual = scene(samplePoint);

    // calculate how much closer geometry is than expected
    float contribution = max(sampleDistance - actual, 0.0);

    // reduce the influence of distant samples
    float weight = 1.0 / (1.0 + sampleDistance);

    // accumulate the weighted occlusion
    occlusion += contribution * weight;

    // accumulate the sample weight
    totalWeight += weight;
}

// normalize the accumulated occlusion
occlusion /= totalWeight;

// prevent the occlusion value from exceeding one
occlusion = clamp(occlusion, 0.0, 1.0);

// convert occlusion into ambient visibility
float ambientVisibility = 1.0 - occlusion;
```

## Combine both effects

The final lighting keeps direct shadowing and ambient occlusion conceptually
separate.

```slang
// calculate diffuse alignment with the light
float diffuse = max(dot(normal, lightDirection), 0.0);

// apply the shadow visibility to direct illumination
float direct = diffuse * shadow;

// define the ambient illumination strength
float ambient = 0.12;

// apply ambient occlusion to environmental illumination
float indirect = ambient * ambientVisibility;

// combine direct and ambient illumination
float lighting = direct + indirect;

// define the surface material color
float3 surfaceColor = float3(0.2, 0.45, 0.8);

// apply the lighting to the material
float3 color = surfaceColor * lighting;

// return the final shaded surface
return float4(color, 1.0);
```

## Now type it again

Re-drill the hard shadow query without re-explaining it.

```slang
// define the shadow visibility
float shadow = 1.0;

// initialize the distance traveled by the shadow ray
float travel = 0.0;

// march from the surface toward the light
for (int step = 0; step < shadowSteps; step++) {
    // calculate the current shadow ray position
    float3 point = shadowOrigin + lightDirection * travel;

    // evaluate the nearest scene surface
    float sceneDistance = scene(point);

    // stop when another surface blocks the light
    if (sceneDistance < normalEpsilon) {
        shadow = 0.0;

        // stop searching after finding an occluder
        break;
    }

    // advance by the safe scene distance
    travel += sceneDistance;

    // stop after reaching the light
    if (travel >= lightDistance) {
        break;
    }
}
```

Then drill the ambient occlusion calculation.

```slang
// initialize accumulated occlusion
float occlusion = 0.0;

// initialize accumulated sample weight
float totalWeight = 0.0;

// sample several distances above the surface
for (int step = 1; step <= aoSamples; step++) {
    // calculate the current sample distance
    float sampleDistance = step * aoSpacing;

    // move the sample outward along the normal
    float3 samplePoint = hitPoint + normal * sampleDistance;

    // evaluate the actual distance to nearby geometry
    float actual = scene(samplePoint);

    // calculate how much closer geometry is than expected
    float contribution = max(sampleDistance - actual, 0.0);

    // reduce the influence of distant samples
    float weight = 1.0 / (1.0 + sampleDistance);

    // accumulate the weighted occlusion
    occlusion += contribution * weight;

    // accumulate the sample weight
    totalWeight += weight;
}

// normalize the accumulated occlusion
occlusion /= totalWeight;

// prevent the occlusion value from exceeding one
occlusion = clamp(occlusion, 0.0, 1.0);

// convert occlusion into ambient visibility
float ambientVisibility = 1.0 - occlusion;
```

Finally, combine the two lighting terms.

```slang
// calculate diffuse alignment with the light
float diffuse = max(dot(normal, lightDirection), 0.0);

// apply the shadow visibility to direct illumination
float direct = diffuse * shadow;

// define the ambient illumination strength
float ambient = 0.12;

// apply ambient occlusion to environmental illumination
float indirect = ambient * ambientVisibility;

// combine direct and ambient illumination
float lighting = direct + indirect;

// define the surface material color
float3 surfaceColor = float3(0.2, 0.45, 0.8);

// apply the lighting to the material
float3 color = surfaceColor * lighting;
```

## Wrap up

The flow: surface -> shadow ray -> direct visibility -> nearby samples ->
ambient visibility -> lighting.

The same distance field can answer whether light reaches a surface and how much
nearby geometry surrounds it.
