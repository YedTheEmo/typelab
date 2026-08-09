# Time and dynamic simulation - typing

This lesson types temporal shader mathematics: animate coordinates, build
periodic motion, create waves, and evolve values through iterative updates.

## Animate a coordinate

Time can be used as a changing coordinate input.

```slang
// define the current position
float3 position = inputPosition;

// move the position along the x axis over time
float animatedX = position.x + time;

// wrap the moving coordinate into a repeating range
float pattern = frac(animatedX);
```

## Control movement speed

Speed changes how quickly time produces spatial displacement.

```slang
// define the movement speed
float speed = 0.5;

// convert elapsed time into displacement
float displacement = time * speed;

// move the coordinate by the displacement
float movingX = position.x + displacement;
```

## Create an oscillation

Sine converts continuously increasing time into repeating motion.

```slang
// define the oscillation frequency
float frequency = 2.0;

// convert time into a repeating wave
float wave = sin(time * frequency);

// remap the wave from negative one through one into zero through one
float cycle = wave * 0.5 + 0.5;
```

## Control the period

A period describes how long one complete oscillation should take.

```slang
// define the desired period in seconds
float period = 2.0;

// convert the period into angular frequency
float angularFrequency = 6.2831853 / period;

// evaluate the oscillator using the desired period
float oscillator = sin(time * angularFrequency);
```

## Add phase

Phase shifts an oscillator without changing its frequency.

```slang
// define the phase offset
float phase = 1.0;

// shift the oscillator by its phase
float phasedWave = sin(time * angularFrequency + phase);
```

## Build a traveling wave

Space can modify the phase so different positions animate at different times.

```slang
// define the spatial wave frequency
float spatialFrequency = 4.0;

// define the temporal wave speed
float waveSpeed = 2.0;

// combine space and time into one phase
float wavePhase = position.x * spatialFrequency
    + time * waveSpeed;

// evaluate the traveling wave
float travelingWave = sin(wavePhase);
```

## Reverse the direction

Changing the sign of the time term reverses the wave direction.

```slang
// define the reversed wave phase
float reversedPhase = position.x * spatialFrequency
    - time * waveSpeed;

// evaluate the reversed traveling wave
float reversedWave = sin(reversedPhase);
```

## Create circular motion

A changing angle can drive two coordinates at once.

```slang
// define the orbital speed
float angularSpeed = 1.5;

// calculate the current orbital angle
float angle = time * angularSpeed;

// define the orbital radius
float radius = 0.75;

// calculate the orbital x coordinate
float orbitX = cos(angle) * radius;

// calculate the orbital y coordinate
float orbitY = sin(angle) * radius;
```

## Create a radial wave

Distance from the origin can become the spatial coordinate of a wave.

```slang
// calculate the distance from the origin
float radialDistance = length(position.xy);

// define the radial wave frequency
float radialFrequency = 6.0;

// define the radial wave speed
float radialSpeed = 3.0;

// combine radial distance and time into one phase
float radialPhase = radialDistance * radialFrequency
    - time * radialSpeed;

// evaluate the radial wave
float radialWave = sin(radialPhase);
```

## Animate a distance field

A distance field can change by making one of its parameters depend on time.

```slang
// define the base sphere radius
float baseRadius = 0.5;

// define the amount of radius variation
float radiusVariation = 0.2;

// calculate the animated radius
float animatedRadius = baseRadius
    + sin(time) * radiusVariation;

// calculate the sphere distance
float sphereDistance = length(position) - animatedRadius;
```

## Wrap time into cycles

Fractional time creates a repeating ramp.

```slang
// define the cycle duration
float cycleDuration = 2.0;

// convert time into the current cycle
float cycleTime = time / cycleDuration;

// wrap the cycle into the range from zero to one
float cycle = frac(cycleTime);
```

## Ease a cycle

Smoothstep changes the rate of movement without changing its range.

```slang
// define the eased cycle
float easedCycle = smoothstep(0.0, 1.0, cycle);
```

## Combine oscillators

Several simple waves can produce more complex deterministic motion.

```slang
// calculate the primary oscillator
float waveA = sin(time);

// calculate a faster secondary oscillator
float waveB = sin(time * 2.0);

// combine the oscillators with a smaller secondary contribution
float combinedWave = waveA + waveB * 0.5;
```

## Add spatial phase

A position-dependent phase turns one global oscillator into many local phases.

```slang
// calculate a phase offset from the x coordinate
float spatialPhase = position.x * 3.0;

// combine the spatial phase with global time
float localPhase = time + spatialPhase;

// evaluate the local oscillator
float localWave = sin(localPhase);
```

## Iterate a mathematical transformation

A loop can repeatedly feed one mathematical result into the next iteration.

```slang
// initialize the iterative value from position
float value = position.x;

// apply the transformation several times
for (int step = 0; step < 4; step++) {
    // feed the current value through the next sine transformation
    value = sin(value + time);
}
```

## Understand local iteration

The loop above does not preserve its value between frames.

```slang
// initialize a temporary simulation value
float temporaryValue = position.x;

// update the value four times during this invocation
for (int step = 0; step < 4; step++) {
    // apply one local update
    temporaryValue = temporaryValue * 0.5;
}
```

The variable exists only for the current shader evaluation.

## Update persistent state

A stateful simulation conceptually reads an old value and produces a new one.

```slang
// define the previous frame position
float oldPosition = previousState;

// define the velocity of the simulated object
float velocity = 0.75;

// calculate the displacement for this frame
float displacement = velocity * deltaTime;

// calculate the next frame position
float newPosition = oldPosition + displacement;
```

The important difference is that previousState must come from storage that
survives beyond the current shader invocation.

## Apply damping

Damping reduces a changing quantity over time.

```slang
// define the drag strength
float drag = 1.5;

// calculate the time-dependent damping factor
float damping = exp(-drag * deltaTime);

// reduce the previous velocity
float newVelocity = oldVelocity * damping;
```

The exponential form makes the decay depend on elapsed time.

## Build a simple update

Combine velocity, displacement, and damping into one update rule.

```slang
// define the previous position
float positionState = previousPosition;

// define the previous velocity
float velocityState = previousVelocity;

// define the acceleration
float acceleration = -0.5;

// update the velocity from acceleration
float nextVelocity = velocityState + acceleration * deltaTime;

// update the position from velocity
float nextPosition = positionState + nextVelocity * deltaTime;

// calculate the damping factor
float damping = exp(-0.8 * deltaTime);

// damp the updated velocity
nextVelocity *= damping;
```

The update rule now evolves a state rather than simply evaluating an animated
function.

## Combine procedural animation

Use spatial and temporal inputs together to construct a final procedural value.

```slang
// calculate the distance from the origin
float distance = length(position.xy);

// define the spatial frequency
float spatialFrequency = 5.0;

// define the animation speed
float animationSpeed = 2.0;

// combine space and time into a wave phase
float phase = distance * spatialFrequency
    - time * animationSpeed;

// evaluate the wave
float wave = sin(phase);

// remap the wave into a normalized range
float normalizedWave = wave * 0.5 + 0.5;

// apply smooth easing to the normalized wave
float animatedValue = smoothstep(0.0, 1.0, normalizedWave);
```

## Now type it again

Re-drill the basic time-driven movement.

```slang
// define the movement speed
float speed = 0.5;

// convert elapsed time into displacement
float displacement = time * speed;

// move the coordinate by the displacement
float movingX = position.x + displacement;
```

Then drill the periodic oscillator and phase relationship.

```slang
// define the oscillation frequency
float frequency = 2.0;

// define the phase offset
float phase = 1.0;

// evaluate the phased oscillator
float wave = sin(time * frequency + phase);

// remap the wave into a normalized range
float cycle = wave * 0.5 + 0.5;
```

Now drill the spatial wave.

```slang
// define the spatial wave frequency
float spatialFrequency = 4.0;

// define the temporal wave speed
float waveSpeed = 2.0;

// combine space and time into one phase
float wavePhase = position.x * spatialFrequency
    + time * waveSpeed;

// evaluate the traveling wave
float travelingWave = sin(wavePhase);
```

Finally, drill the distinction between procedural animation and state
evolution.

```slang
// define the previous frame position
float oldPosition = previousState;

// define the velocity of the simulated object
float velocity = 0.75;

// calculate the displacement for this frame
float displacement = velocity * deltaTime;

// calculate the next frame position
float newPosition = oldPosition + displacement;
```

## Wrap up

The flow: position + time -> phase -> periodic function -> animation.

For stateful systems: previous state -> update rule -> delta time -> next state.
