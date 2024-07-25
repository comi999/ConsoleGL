Welcome to the ConsoleGL

This is graphics library that runs in a console window, that mimics OpenGL, in development by Len Farag.

To use this project, you can create an application by placing a folder with your source files inside of the Applications folder.
Running GenerateProjectFiles.bat will then produce a project and solution file called ConsoleGL.sln in the root directory you can run.
It uses Premake5 to generate the project, and that is supplied in the project, so no need to install Premake5 if you don't already
have it.

In your Application folder, you can create a Resources folder to store all of your resource files inside. This will automatically be linked
via a proprocessor define.

Using CleanProjectFiles.bat will clear the .vs files, the Generated/Project files and the Lengine.sln.
Everything inside of the Generated folder is completely ignored by the .gitignore, so you shouldn't have any issues submitting rubbish.

For the shader compiler to work, you need to have the following cmd commands available:
"premake5" - used to make shader project.
"devenv" - used to determine Visual Studio version.
"msbuild" - used to compile shader project.

These are used by the ShaderCompiler.exe to build a project around your shader code, and then compile it into a dll.

Specifying a shader file:

Shaders are written similar to glsl
A run function should be found: void run() {}
In this function, you write the code that will execute.
It can not have any parameters.

VARIABLES
You declare variables in global scope. 
There are 3 types of variable:
- uniform
- attribute
- param
- inbuilt

Uniforms:
Uniforms are constant variables. These stay the same across all shader stages.
They can be a struct.
They can be an array.
Uniforms are of the form uniform# type name;
Examples:
struct Light { vec3 Direction; vec3 Colour; };
uniform(0) Light Lights[8];
uniform(1) mat4 PVM;
uniform(2) mat4 M;

Attributes:
Attributes are inputs that come from vertex arrays per vertex.
They can only be used in the vertex shader.
They are of the form "attrib# [type] [name];"
Examples:
attrib(0) vec3 Position;
attrib(1) vec3 Normal;
attrib(2) vec2 UV;

In:
Input variables are of the form "in [type] [name];"
They can only be used in the fragment shader and must have a matching out in the vertex shader
Examples include:
in vec4 VertexColours;
in float Depth;
in vec3 Pos;

Out:
Output variables are in the form of "{flat|affn|prsp| } out [type] [name];" 
The preceeding flat,affn,prsp, or blank, determins how this out variable will be interpolated.
When there is no flat,affn,prsp, prsp is chosen as default mode.
affn,prsp must be used with floating point types.
If flat is used, the provoking vertex will set for all fragment invocations.
Examples:
out vec3 FragPos;
affn out vec2 TexCoords;
flat out vec3 VertexColour;

Inbuilt variables:
Inbuilts are a form of output or input that communicates to the shading pipeline.
They can not be qualified like the other in and outs. They must have the exact name, type
and should be in the right shader.
Vertex inbuilts:
out vec4 VertPos

Fragment inbuilts:
out vec4  FragColor
out float FragDepth
in  vec4  FragCoord

Below is a basic phong shader written in cgsl:

#Vertex:
	attrib0 vec3 aPos;         // Position attribute
	attrib1 vec3 aNormal;      // Normal attribute
	attrib2 vec2 aTexCoords;   // Texture coordinates attribute
	
	uniform0 mat4 model;      // Model matrix
	uniform1 mat4 view;       // View matrix
	uniform2 mat4 projection; // Projection matrix
	
	out vec3 FragPos;       // Position of the fragment in world space
	out vec3 Normal;        // Normal of the fragment in world space
	out vec2 TexCoords;     // Texture coordinates for the fragment
	
	out vec4 VertPos; 		// The output vert position in clip space. This is an inbuilt.
	
	void main() {
		FragPos = vec3(model * vec4(aPos, 1.0));  // Transform the vertex position to world space
		Normal = mat3(transpose(inverse(model))) * aNormal; // Transform the normal to world space
		TexCoords = aTexCoords; // Pass the texture coordinates to the fragment shader
		
		VertPos = projection * view * vec4(FragPos, 1.0); // Transform the vertex position to clip space
	}
	
#Fragment:
	in vec3 FragPos;       // Position of the fragment in world space
	in vec3 Normal;        // Normal of the fragment in world space
	in vec2 TexCoords;     // Texture coordinates for the fragment
	
	out vec4 FragColor;    // Output color of the fragment, this is an inbuilt
	
	uniform3 vec3 lightPos; // Position of the light source
	uniform4 vec3 viewPos;  // Position of the camera
	uniform5 vec3 lightColor; // Color of the light source
	uniform6 vec3 objectColor; // Color of the object
	uniform7 sampler2D texture_diffuse1; // Diffuse texture
	
	void main() {
		// Ambient lighting
		float ambientStrength = 0.1;
		vec3 ambient = ambientStrength * lightColor;
	
		// Diffuse lighting
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * lightColor;
	
		// Specular lighting
		float specularStrength = 0.5;
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = specularStrength * spec * lightColor;
	
		// Combine results
		vec3 lighting = (ambient + diffuse + specular) * texture(texture_diffuse1, TexCoords).rgb;
		FragColor = vec4(lighting, 1.0);
	}