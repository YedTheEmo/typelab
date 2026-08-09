# Time and dynamic simulation - concepts

A shader does not need to remember what happened on the previous frame to
produce animation.

Instead, the host application can provide the current time as an input. The
shader evaluates its mathematical functions using that value, producing a
different result for every frame.

This turns a static spatial function into a function of both position and time.

## Time is another input

A static shader can be thought of as a function of position.

```slang
float value = field(position);
```

The result depends only on where the current invocation is evaluating the
function.

Animation adds time as another independent input:

```slang
float value = field(position, time);
```

The important idea is that time is not inherently an animation system. It is
just a changing number supplied to the mathematical model.

If the same position is evaluated at different times, the function can produce
different values.

## The simplest animation

The simplest use of time is to add it directly to a coordinate.

```slang
float x = position.x + time;
```

As time increases, x also increases.

A pattern evaluated using x therefore appears to move across the screen.

For example, a repeating pattern can use a fractional coordinate:

```slang
float pattern = frac(position.x + time);
```

The fractional part repeatedly wraps the increasing value back into the range
from zero to one.

The geometry did not move. The coordinate used to evaluate the geometry moved.

This distinction is fundamental to procedural animation.

## Translation through the domain

Instead of moving an object, move the point at which the object is evaluated.

Suppose a field describes a circle centered at the origin:

```slang
float distance = length(position) - radius;
```

To make the circle travel along x, subtract a time-dependent position:

```slang
float3 movingPoint = position - float3(time, 0.0, 0.0);
float distance = length(movingPoint) - radius;
```

The circle remains mathematically centered around movingPoint's origin.

From the screen's perspective, however, the circle appears to move.

This is called domain transformation: change the coordinates before evaluating
the underlying mathematical object.

## Speed is a scale on time

Using time directly gives one unit of movement per unit of time.

A speed parameter changes that relationship:

```slang
float positionX = time * speed;
```

If speed is two, the object travels twice as far for the same amount of time.

This is not a special animation operation. It is ordinary multiplication.

The important relationship is:

```slang
distance = time * speed;
```

Time provides elapsed duration, speed provides units per duration, and their
product provides displacement.

## Periodic motion

Linear time produces motion that continues indefinitely.

Many visual effects instead oscillate between two states.

Sine is useful because its output continuously varies between negative one and
one:

```slang
float oscillation = sin(time);
```

The argument of sine is an angle measured in radians, so increasing time
creates a repeating cycle.

A value between zero and one can be produced by remapping the result:

```slang
float cycle = sin(time) * 0.5 + 0.5;
```

The multiplication reduces the range from negative one through one into
negative one half through one half.

The addition then shifts that range into zero through one.

This pattern is useful because many shader parameters naturally expect a
normalized value.

## Frequency controls repetition

The argument of sine determines how quickly the cycle repeats.

```slang
float oscillation = sin(time * frequency);
```

Increasing frequency compresses more cycles into the same amount of time.

A useful way to think about this is that time is the input axis and frequency
controls how quickly we travel along the sine wave.

If a full cycle should take a known period, frequency can be derived rather
than guessed.

```slang
float angularFrequency = 6.2831853 / period;
```

The constant is approximately two pi.

A period of two seconds therefore gives an angular frequency that completes one
cycle every two seconds.

## Phase shifts

Two oscillators can have the same frequency but begin at different points in
their cycles.

That difference is phase.

```slang
float wave = sin(time * frequency + phase);
```

Changing phase shifts the wave along its cycle without changing its speed.

This is extremely useful for coordinating repeated elements.

For example, a grid of points can each receive a different phase based on its
position:

```slang
float phase = position.x * spacing;
float wave = sin(time * frequency + phase);
```

Now neighboring positions reach their peaks at different times.

A spatial pattern has become a traveling wave.

## Space and time can form one equation

A moving wave often has the form:

```slang
float wave = sin(position.x * frequency + time * speed);
```

The spatial coordinate determines how rapidly the pattern changes across space.

Time determines how the pattern changes from frame to frame.

The two terms together describe a wave moving through the domain.

Changing the sign of the time term reverses the direction:

```slang
float wave = sin(position.x * frequency - time * speed);
```

This is one of the simplest examples of how spatial mathematics and temporal
mathematics combine into motion.

## Circular motion

Time can also control an angle.

```slang
float angle = time * angularSpeed;
```

Sine and cosine convert that angle into coordinates on a circle:

```slang
float x = cos(angle) * radius;
float y = sin(angle) * radius;
```

Together, these equations describe a point orbiting the origin.

The important relationship is that one angle controls two coordinates.

This is why trigonometry is so common in shader animation: a changing scalar
can drive coordinated motion through multiple dimensions.

## Reusing temporal functions

The same time value can drive many independent parameters.

```slang
float pulse = sin(time);
float orbit = cos(time);
float wave = sin(position.x + time);
```

These expressions are deterministic.

At a particular time and position, they always produce the same result.

This is different from randomness. Animation does not require unpredictable
values. It requires a predictable function whose input changes.

Determinism is one of the reasons procedural animation works so well in
shaders.

## Remapping time

Time is often more useful after being transformed into another range.

A common operation is fractional wrapping:

```slang
float cycle = frac(time);
```

The result repeatedly travels from zero toward one and then jumps back to zero.

This creates a repeating ramp.

The ramp can then drive another function:

```slang
float pulse = smoothstep(0.0, 1.0, cycle);
```

Now a repeating time interval controls a smooth transition.

The shader can therefore construct complex temporal behavior by composing
simple mathematical functions.

## Easing

Linear change can look mechanical.

An easing function changes how quickly a value moves through its range without
changing the endpoints.

For example:

```slang
float eased = smoothstep(0.0, 1.0, cycle);
```

The input still travels from zero to one, but the output accelerates gradually,
moves fastest around the middle, and decelerates near the end.

The important concept is that animation can be separated into two concerns.

Time determines where we are in the cycle.

An easing function determines how that cycle should be perceived.

## Combining oscillators

Complex motion can emerge by adding several simple waves.

```slang
float waveA = sin(time);
float waveB = sin(time * 2.0);
float motion = waveA + waveB * 0.5;
```

The first oscillator supplies the main movement.

The second supplies a smaller variation at a different frequency.

This is a basic form of signal composition.

The resulting motion is still completely deterministic, but it is less
uniform than a single sine wave.

Procedural animation often works this way: simple functions are layered rather
than replaced by one enormous equation.

## Position-dependent animation

Time does not have to affect every point equally.

A spatial value can modify the temporal input:

```slang
float localTime = time + position.x;
float wave = sin(localTime);
```

Different positions now experience the same animation at different phases.

This is the mathematical basis of waves, ripples, scanning effects, and many
other traveling patterns.

The important structure is:

```slang
localTime = globalTime + spatialOffset
```

The spatial offset turns a global clock into a local clock.

## Propagating waves

A wave can be described using distance from a source.

```slang
float distance = length(position);
float wave = sin(distance * frequency - time * speed);
```

Every point at the same distance from the origin receives the same phase.

The result is therefore circular or spherical depending on the dimensionality
of the position.

The distance field itself becomes the spatial coordinate of the animation.

This connects temporal simulation directly to the distance-field mathematics
used throughout this track.

## Time inside a distance field

A distance function does not have to describe a permanently static object.

Time can modify its parameters:

```slang
float radius = 0.5 + sin(time) * 0.2;
float distance = length(position) - radius;
```

The underlying equation still describes a sphere.

Its radius simply becomes a function of time.

The object therefore expands and contracts without storing any explicit state.

This is a powerful general pattern:

```slang
parameter = function(position, time);
```

A static mathematical model becomes a dynamic one by making its parameters
functions of time.

## Iteration can create temporal behavior

A loop can repeatedly apply a transformation to a value.

```slang
float value = position.x;

for (int step = 0; step < 4; step++) {
    value = sin(value + time);
}
```

Each iteration feeds its output into the next iteration.

The result is no longer simply one sine wave. It is a composition of the same
operation applied repeatedly.

This is useful for creating increasingly complex procedural patterns.

Iteration is especially important when building effects where a simple formula
does not provide enough structure.

## Iteration does not automatically create state

A shader invocation begins with its inputs.

If a loop runs four times, those four iterations happen during the current
evaluation. They do not automatically remember the result from the previous
frame.

For example:

```slang
for (int step = 0; step < 4; step++) {
    value = value * 0.5;
}
```

This modifies value four times in one invocation.

When the next frame is evaluated, value starts again from whatever initial value
the shader computes for that frame.

This distinction matters when discussing simulation.

A mathematical iteration is not the same thing as persistent memory.

## True temporal state

A simulation such as fluid motion or particle movement may require information
from a previous frame.

That requires some form of persistent storage outside the ordinary local
variables of a shader invocation.

Conceptually, the relationship becomes:

```slang
newState = update(oldState, position, time);
```

The previous state becomes an input to the next evaluation.

Textures, buffers, or other GPU resources can store that state between frames.

The shader then reads the previous result, applies an update rule, and writes
the new result.

This is fundamentally different from merely evaluating a time-dependent
function.

## Time-dependent function versus simulation

A procedural animation has the form:

```slang
value = function(position, time);
```

The current frame can be computed directly from position and time.

A stateful simulation has the form:

```slang
newState = update(oldState, inputs);
```

The current result depends on what happened previously.

The distinction is important because procedural animation is usually easier,
deterministic, and inexpensive, while simulation can represent evolving
systems whose state cannot be reconstructed from time alone.

Both are mathematical, but they encode different kinds of information.

## Numerical integration

A simple simulation can update a quantity using a small time step.

Suppose position changes according to velocity:

```slang
position = position + velocity * deltaTime;
```

This is an Euler integration step.

Velocity tells us the rate of change.

Delta time tells us how long the update represents.

Their product gives the approximate displacement during that step.

Repeated updates can therefore approximate continuous motion.

## Why delta time matters

Elapsed time and frame duration are different quantities.

A shader may receive total time:

```slang
float time;
```

It may also receive the duration since the previous frame:

```slang
float deltaTime;
```

Total time is useful for deterministic functions such as sine and cosine.

Delta time is useful when updating persistent state because the amount of
change should depend on how much time actually passed.

Using frame count instead of delta time can make simulations depend on frame
rate.

## Damping

Simulation values often need to lose energy over time.

A simple damping update is:

```slang
velocity *= damping;
```

If damping is slightly below one, velocity gradually decreases.

A more time-aware form uses delta time:

```slang
velocity *= exp(-drag * deltaTime);
```

The exponential keeps the decay tied to elapsed time rather than the number of
frames.

This is another example of turning a visual behavior into a mathematical
relationship.

## The deeper mental model

Time is not a special animation command.

It is another coordinate supplied to the mathematical system.

A shader can use that coordinate to translate domains, change parameters,
shift phases, generate waves, drive periodic functions, or update persistent
state.

The resulting mental model is:

```slang
visual = function(position, time)
```

for procedural animation, and:

```slang
stateNext = update(statePrevious, inputs, deltaTime)
```

for stateful simulation.

The first reconstructs an image from the current inputs.

The second evolves information from one frame into the next.

Once this distinction is understood, animated shaders stop looking like
collections of tricks. They become mathematical systems whose inputs include
space, time, and sometimes stored state.

## Next step

This completes the Shaders track. The next step is to review the complete
sequence and practice the mathematical connections between the lessons.
