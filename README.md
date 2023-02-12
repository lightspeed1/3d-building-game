# 3d-building-game
This is a 3D building game written in C++. I used OpenGL with GLEW to draw graphics, and GLM (OpenGL mathematics library) for linear algebra computations. I have written the game from (nearly) complete scratch. This includes collision detection, graphics (with the help of learnopengl.com), and physics.
#### Libraries Used:
- **GLEW** (for rendering graphics)
- **GLFW** (for creating a window and handling input)
- **GLM** (for vector and matrix operations)
- **stb_image** (for loading textures)
- **FreeType** (for loading fonts)

#### Overview:
In this game, the user can build worlds with 3 different kinds of parts; cubes, slants, and cylinders. You can change the size, rotation, and color of the parts you build. You can save and load these worlds into the scene. Also, you can choose to "unfreeze" the parts which applies physics to them. They will fall due to gravity and collide with other parts. In unfreeze mode, you can create explosions which blows objects apart.


#### Notable features:
- **Physics**: The most difficult feature to implement was likely the physics. Physics in this game entailed calculating the final linear and angular velocity of two objects that collide with eachother. I have taken a physics class in the past (which taught me about momentum transfer), but what I learned wasn't entirely sufficient to writing the physics solver. I had only learned about momentum transfer in the case of two objects colliding with eachother head on, where the collision was perfectly elastic (total kinetic energy was conserved). I had to think about a way to correctly calculate momentum in every situation. Eventually I realized that I could treat each object as a point mass. First, I took the first object to be a point mass. I would calculate the linear velocity it has, and also the angular velocity relative to the center of the second object. I could then use this and the respective velocities of the second object with the traditional angular and linear momentum formulas. However, both of these equations have 2 unknown variables. Which means we must use another equation. I had to use the conservation of energy equation. I didn't want 100% of the kinetic energy to be conserved in a collision, so I needed to modify this equation for an inelastic collision. I could then find the final velocities of the first object in the collision. I repeated the process for the second object, while treating the first one as a point mass this time. 

- **Collision**: Another feature that was difficult to create was figuring out whether or not two objects are colliding. This works for convex shapes, but not concave shapes. The core idea of my method is just like the SAT (separating axis theorem). This was difficult because there were many cases that I didn't initially consider. The cases you must check include:
    1.  **Point penetrates**: For every point on each object, check if it is past the plane of every normal vector that belongs to each face of the other object. If it is past all of the planes, then there is a collision. The collision location is halfway between the plane that the point is passing the least, and the point.
    2. **Edges overlap**: For each pair of edges (2 vertices per edge) on the parts, form a plane from the pair of edges. Project the edges onto the plane and see if they intersect, if so, the objects collide. The collision location here is where the edges intersect.
    3. **Face to face collision**: If multiple points (at least 3) on each of the objects go past a plane the same amount, and are past the other planes, there is a face on collision. The collision location is the middle of the shared area of the faces that are touching.

#### Features to Possibly Implement in the Future:
 - Softbodies
 - Particles

#### Challenges/Lessons:

- **Wanting to do everything myself**: I started this project with the intention of every idea being my own. This became a hinderance during more challenging parts of the project like collision detection and solving physics. I wanted to use as few online resources as possible, but there were times where I did a quick Google search to skim commonly used techniques for whatever I was implementing.

- **Writing vs checking code**: Every time I implemented a feature, I immediately wanted to move onto the next. My initial priority was getting the code completed as quickly as possible, even if it would sacrifice the code quality. Soon enough, I realized that this was not sustainable, and that by the end of the project, my code would not be very readable nor easy to debug if I only prioritized development speed. I had to learn to fine tune the time between writing, and checking/revising code. Even if my code worked, I had to resist the urge to move onto the next feature, and make my code more readable for myself and everyone else who would look at it.

- **Feature creep/learning to give up**: I wanted to add a few features that ultimately didn't pan out. Most of the time, this was because I tried to implement them, but they would've taken too much time to solve. Naturally, when I start trying to solve a problem, I hate having to stop, and I just keep thinking about it all day. I had to learn to ignore this obsessive need to solve everything and make my goals more realistic. I had to cut down on the number of things I wanted in the final project for the sake of time and effort.

