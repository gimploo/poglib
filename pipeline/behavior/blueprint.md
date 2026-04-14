@gemini
+=============================================================================+
|                          FRAME LOOP (Sequential Pipeline)                   |
+=============================================================================+
|                                                                             |
|  [STAGE 1: INPUT POLLING]                                                   |
|  +------------------+       +-------------------+                           |
|  | Hardware Devices | ----> |   SDL2 Events     |                           |
|  | (Kbd, Mouse, AI) |       | (Raw Key States)  |                           |
|  +------------------+       +---------+---------+                           |
|                                       |                                     |
|                                       v                                     |
+=============================================================================+
| [STAGE 2: COMMAND BUFFER] (poglib/pipeline/input)                           |
+=============================================================================+
|                                                                             |
|   command_buffer.h                                                          |
|  +-------------------------+                                                |
|  | Bitmask or Queue        |                                                |
|  | {CMD_FWD, CMD_SPRINT...}|                                                |
|  +------------+------------+                                                |
|               | (Read Only)                                                 |
|               V                                                             |
|                                                                             |
+=============================================================================+
| [STAGE 3: BEHAVIOR PIPELINE] (poglib/pipeline/logic)                        |
+=============================================================================+
|                                                                             |
|  state_stack.h                                                              |
| +-------------------------------------------------------------------------+ |
| |                                                                         | |
| |  BEHAVIOR TICK (Pure Data Transformation)                               | |
| |                                                                         | |
| |  INPUT DATA (Struct):                                                   | |
| |  { Active_Commands, Cached_Physics_Flags, DeltaTime }                   | |
| |                                                                         | |
| |               +-----------------------------------+                     | |
| |               |                                   |                     | |
| |  INPUT DATA --+--> [PUSH/POP/PEEK Logic] ---------+--> OUTPUT DATA      | |
| |               |    (State Stack: FSM/PDA)         |    (MovementIntent) | |
| |               |                                   |                     | |
| |               +--------^--------------------------+                     | |
| |                        | (Read/Modify)                                  | |
| |               +--------+-----------+                                    | |
| |               | STATE CALL_BACKS   | (walk, run, jump)                  | |
| |               | enter(), exit()    |                                    | |
| |               | update() <---------+-----[Logic & Contextual Pushes]    | |
| |               | (No Physics/GFX Call)                                   | |
| |               +--------------------+                                    | |
| |                                                                         | |
| +-------------------------+-----------------------------------------------+ |
|                           |                                                 |
|                           V (MovementIntent Struct)                         |
|                                                                             |
+=============================================================================+
| [STAGE 4: PHYSICS PIPELINE] (poglib/pipeline/physics)                       |
+=============================================================================+
|                                                                             |
|   jolt_solver.h                                                             |
|  +-------------------------------------------------------+                  |
|  | PHYSICS RESOLUTION (Jolt Physics Solver)              |                  |
|  |                                                       |                  |
|  | 1. Apply MovementIntent (forces/velocity)             |                  |
|  | 2. Step Physics (poglib_physics_step)                 |                  |
|  | 3. Update Transform Component                         |                  |
|  | 4. Cache Physics Data  -------------------------------+---+
|  |                                                       |   |              |
|  +--------------------------+----------------------------+   |              |
|                             | (Write)                        | (Store)      |
|                             V                                _              |
|                                                                             |
+=============================================================================+
| [STAGE 5: RENDER PIPELINE] (poglib/pipeline/render)                         |
+=============================================================================+
|                                                                             |
|  +------------------+       +-------------------+       +-----------------+ |
|  | ECS Transform    | ----> | Render Queue      | ----> | GPU (OpenGL)    | |
|  | (Position/Rot)   |       | (Draw Calls/UBOs) |       | (Draw Frame)    | |
|  +------------------+       +-------------------+       +-----------------+ |
|                                                                             |
+=============================================================================+
|                       [LOOP REPEATS NEXT FRAME]                             |
+=============================================================================+


typedef struct {
    float distance_to_ground;  // Cached from Jolt's last shape cast
    vec3 surface_normal;       // Direction of the ground
    float vertical_velocity;   // Relative to the ground
    bool contact_active;       // Hard boolean from the solver
} PhysicsCache
