# Shading-Developer-Tool (Oasis)


## Versions

Visual Studio 2019

Windows 10


## Video Walkthrough

https://youtu.be/5xtaGhe8j14

## GitHub Repo

https://github.com/BonusLoaf/Shading-Developer-Tool


## How It Works

Open the executable file called 'Project_Template.exe' under the folder 'Build' to run the build

The program loads a plane, tree, pond, sphinx, cactus and camel model, then renders the first 4 using the Blinn-Phong Model. (The pond has a separate shader to animate its vertices).
This image is rendered to a texture and that texture is used to apply a Guassian blur to the image.
A GUI is then rendered over the top of this image, giving the user options to change out the models, rotate the camera or swap the filter between Blur and Edge(making the scene look more catoon-like).
Changing the filter to Edge will swap out the shader and setup the Edge detection frame buffers. Vice Versa for selecting Blur.


## Code Functions


Open 'Project_Template.sln' to open the solution.


[scenebasic_uniform.cpp]

initScene - Creates a view quad, declares all shader properties, sets up the GUI then calls setupGaussFBO

gauss - Gaussian distrobution function

compile - Retrives all shader files and sets them to their respective GLSLProgram variables

update - Constantly rotates the view around the centre of the scene, and updates the 'time' variable for use in the animation shader

activateGaussShader - Swaps to gauss shader and sets up the gauss frame buffers

activateEdgeShader - Swaps to edge shader and sets up the edge frame buffer

renderGUI - Sets up the gui and its buttons

render - Calls all 3 passes to render the models and apply lighting, textures and filters (2 if edge shader is active)

setMatrices - Creates and sets ModelViewMatrix, NormalMatrix and ModelViewProjection uniforms in the shader

setupEdgeFBO - Sets up the edge filter frame buffer

setupGaussFBO - Sets up the guassian blur filter frame buffers

pass1 - Renders scene to a texture, sets parameters for the shader currently in use (gauss or edge)

pass2 - Applies edge filter if edge shader is active, applies gauss blur to verticle pixels if gauss shader is active

pass3 - Applies gauss blur to horizontal pixels if gauss shader is active


[gauss_uniform.frag]

Using the Blinn-Phong Model, renders models, lights and their textures then converts that image to a texture. 
Pass 2 then applies a blur filter to the verticle pixels.
Pass 3 then applies a blur filter to the horizontal pixels.


[gauss_uniform.vert]

Sets all the parameters necessary for the gauss fragment shader.
Passes on the texture coordinates for setting textures.


[edge_uniform.frag]

Using the Blinn-Phong Model, renders models, lights and their textures then converts that image to a texture. 
Pass 2 then applies a filter to the image by reading the texture and using the lightest parts to colour the pixels.


[edge_uniform.vert]

Sets all the parameters necessary for the gauss fragment shader.


[animation_uniform.frag]

Same as gauss_uniform, but without pass 2 and 3.


[animation_uniform.vert]

Sets all the parameters necessary for the gauss fragment shader.
Transforms vertices on y coordinates using on 'time' variable.












