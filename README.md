# 3d-building-game
This is a 3D building game written in C++. I used OpenGL with GLEW to draw graphics, and GLM (OpenGL mathematics library) for linear algebra computations. I have written the game from (nearly) complete scratch. This includes collision detection, graphics (with the help of learnopengl.com), and physics.

#### Video Demo
https://user-images.githubusercontent.com/49415592/221490806-c5b232fc-55a0-49d7-b6ed-f608b4f90c64.mp4



#### Libraries Used:
- **GLEW** (for rendering graphics)
- **GLFW** (for creating a window and handling input)
- **GLM** (for vector and matrix operations)
- **stb_image** (for loading textures)
- **FreeType** (for loading fonts)

#### Overview:
In this game, the user can build worlds with 3 different kinds of parts; cubes, slants, and cylinders. You can change the size, rotation, and color of the parts you build. You can save and load these worlds into the scene. Also, you can choose to "unfreeze" the parts which applies physics to them. They will fall due to gravity and collide with other parts. In unfreeze mode, you can create explosions which blows objects apart.


#### Notable features:
- **Physics**:
    - Goal: calculate the final linear and angular velocity of two colliding objects
    - Treat objects as point masses
    - Use momentum and conservation of energy equation to calculate final velocities separately

- **Collision Detection**:
    - Works for convex shapes
    - Different cases for collision: edges overlap, face to face collision, and point penetration.

#### Features to Possibly Implement in the Future:
 - Softbodies
 - Particles

#### Compile Instructions:
- Specify src/include as an include directory to your compiler.
- Link to freetype.lib, glew32s.lib, and glfw3_mt.lib in the lib directory. If you are not using Windows x64, and/or you want to link dynamically instead of statically, this will not work for you. You will have to fetch the .lib files for your operating system yourself.
- Compile and link all of the .cpp files in the src directory.
