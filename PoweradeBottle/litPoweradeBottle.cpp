/*
 * litPoweradeBottle.cpp
 *
 *  Created on: Aug 11, 2020
 *      Author: jeffrey.tung_snhu
 */


//Header Inclusions
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

//GLM Math Header inclusions
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>

//SOIL image loader Inclusion
#include "SOIL2/SOIL2.h"

using namespace std; //Standard namespace

#define WINDOW_TITLE "Powerade Bottle" //Window Title Macro

//Shader Program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

//Variable declarations for shader, window size initialization, buffer and array objects
GLint shaderProgram, textureShaderProgram, lampShaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, textureVBO, TextureVAO, BottleVAO, LightVAO, texture;

GLfloat cameraSpeed = 0.5f; //Movement speed per frame

GLchar currentKey; //Will store key pressed
bool orbitLogic = false; //Will store left mouse button logic for navigation, initialized to false
bool zoomLogic = false; //Will store right mouse button logic for zoom, initialized to true
bool zkeyViewLogic = false; //Will Store z key button logic to initialize navigation or zoom
bool rotateLogic = false; //Store rotate logic


GLfloat lastMouseX = 400, lastMouseY = 300; //Locks mouse cursor at the center of the screen
GLfloat mouseXOffset, mouseYOffset, zoomD = 10.0f, zoomInScalar = 0.8f, zoomOutScalar = 1.2f, yaw = 0.0f, pitch = 0.0f; //mouse offset, zoom scalars, yaw, and pitch variables
GLfloat sensitivity = 0.005f; //Used for mouse / camera rotation sensitivity
bool mouseDetected = true; //Initially true when mouse movement is detected

glm::vec3 bottlePosition = glm::vec3(0.0f, 0.0f, 0.0f);
//Global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); //Initial camera position. Placed 5 units in Z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); //Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); //Temporary z unit vector
glm::vec3 front; //Temporary z unit vector for mouse
float cameraRotation = glm::radians(-25.0f);

//Bottle and light color
glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
glm::vec3 textureColor(1.0f, 1.0f, 1.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f); //Right Light
glm::vec3 leftLightColor(1.0f, 1.0f, 1.0f); //Left Light

//Right Light position and scale
glm::vec3 lightPosition(-5.0f, 8.0f, -5.0f);
glm::vec3 lightScale(0.3f);

//Left Light position and scale
glm::vec3 leftLightPosition(-5.0f, 5.0f, 5.0f);
glm::vec3 leftLightScale(0.3f);

//Initialize function prototypes
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UMouseClick(int button, int state, int x, int y);
void UMousePressedMove(int x, int y);
void UMouseMove(int x, int y);
void UKeyboard(unsigned char key, int x, int y);
void UKeyReleased(unsigned char key, int x, int y);
void UGenerateTexture(void);

const GLchar * vertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; //Vertex data from Vertex Attrib Pointer 0
	layout (location = 1) in vec3 color; //Color data from Vertex attrib Pointer 1
	layout (location = 3) in vec3 normal; //VAP position 3 for normals

	out vec3 FragmentPos; //For outgoing color / pixels to fragment shader
	out vec3 Normal; //For outgoing normals to fragment shader
	out vec3 mobileColor; //Variable to transfer color data to the fragment shader

	//Global variables for the transfrom matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main() {
		gl_Position = projection * view * model * vec4(position, 1.0f); //transforms vertices to clip coordinates
		FragmentPos = vec3(model * vec4(position, 1.0f)); //Gets fragment / pixel position in world space only
		Normal = mat3(transpose(inverse(model))) * normal;
		mobileColor = color; //references incoming color data
		}
);

const GLchar * fragmentShaderSource = GLSL(330,
	in vec3 mobileColor; //Variable to hold incoming color data from vertex shader
	in vec3 FragmentPos; //For incoming fragment position
	in vec3 Normal; // For incoming normals

	out vec4 gpuColor; //Variable to pass color data to the GPU

	uniform vec3 lightColor; //Right light color
	uniform vec3 lightPos; //Right light position
	uniform vec3 leftLightColor; //left light color
	uniform vec3 leftLightPos; //left light position
	uniform vec3 viewPosition; //View


	void main() {
		//Phong lighting model calculations to generate ambient, diffuse, and specular components
		vec3 norm = normalize(Normal); //Normalize vectors to 1 unit
		vec3 viewDir = normalize(viewPosition - FragmentPos); //Calculate view direction

		//Right Lighting
		//Calculate Ambient Lighting
		float ambientStrength = 0.5f; //Set ambient of global lighting strength
		vec3 ambient = ambientStrength * lightColor; //Generate ambient light color

		//Calculate Diffuse Lighting
		vec3 lightDirection = normalize(lightPos - FragmentPos); //Calculate distance light direction)
		float impact = max(dot(norm, lightDirection), 0.0); //Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse = impact * lightColor; //Generate diffuse light color

		//Calculate Specular lighting
		float specularIntensity = 1.0f; //Set specular light strength
		float highlightSize = 10.0f; //Set specular highlight size
		vec3 reflectDir = reflect(-lightDirection, norm); //Calculate reflection vector

		//Calculate specular component
		float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
		vec3 specular = specularIntensity * specularComponent * lightColor;

		//Left Lighting
		//Calculate Ambient Lighting
		float leftAmbientStrength = 0.1f; //Set ambient or global lighting strength
		vec3 leftAmbient = leftAmbientStrength * leftLightColor; //Generate ambient light color

		//Calculate Diffuse Lighting
		vec3 leftLightDirection = normalize(leftLightPos - FragmentPos); //Calculate distance (light direction between light source and fragments/pixels
		float leftImpact = max(dot(norm, leftLightDirection), 0.0); //Calculate diffuse impact by generating dot product of normal and light
		vec3 leftDiffuse = leftImpact * leftLightColor; //Generate diffuse light color

		//Calculate Speculat lighting
		float leftSpecularIntensity = 0.1f; //Set specular light strength
		float leftHighlightSize = 5.0f; //Set specular highlight size
		vec3 leftReflectDir = reflect(-leftLightDirection, norm); //Calculate reflection vector

		//Calculate specular component
		float leftSpecularComponent = pow(max(dot(viewDir, leftReflectDir), 0.0), leftHighlightSize);
		vec3 leftSpecular = leftSpecularIntensity * leftSpecularComponent * leftLightColor;

		//Calculate phong result
		vec3 rightLightResult = (ambient + diffuse + specular);
		vec3 leftLightResult = (leftAmbient + leftDiffuse + leftSpecular);
		vec3 phong = (rightLightResult + leftLightResult) * mobileColor;

		gpuColor = vec4(phong, 1.0); //Sends color data to the GPU for rendering
	}
);

const GLchar * textureVertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; //Vertex data from VAP 3 for textured object
	layout (location = 2) in vec2 textureCoordinate; //Color data from VAP 2
	layout (location = 3) in vec3 normal; //VAP position 3 for normals

	out vec3 FragmentPos; //For outgoint color / pixels to fragment shader
	out vec3 Normal; //For outgoing normals to fragment shader
	out vec2 mobileTextureCoordinate;

	//Global variables for the transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main() {
		gl_Position = projection * view * model * vec4(position, 1.0f); //transforms vertices to clip coordinates
		FragmentPos = vec3(model * vec4(position, 1.0f)); //Gets fragment / pixel position in world space only ( exclude view and projection)
		Normal = mat3(transpose(inverse(model))) * normal; //get normal vectors in world space only and exclude normal translation properties
		mobileTextureCoordinate = vec2(textureCoordinate.x, 1 - textureCoordinate.y); //flips the texture horizontal
	}
);

const GLchar * textureFragmentShaderSource = GLSL(330,
	in vec2 mobileTextureCoordinate;
	in vec3 Normal; //For incoming normals
	in vec3 FragmentPos; //For incoming fragment position

	out vec4 poweradeTexture; //Outgoing powerade texture to the GPU

	//Uniform /Global variables for object color, light color, light position, and camera/view position
	uniform vec3 lightColor; //Right light color
	uniform vec3 lightPos; //Right light position
	uniform vec3 leftLightColor; //left light color
	uniform vec3 leftLightPos; //left light position
	uniform vec3 viewPosition; //light view

	uniform sampler2D uTexture;

	void main() {
		//Phong lighting model calculations to generate ambient, diffuse, and specular components
		vec3 norm = normalize(Normal); //Normalize vectors to 1 unit
		vec3 viewDir = normalize(viewPosition - FragmentPos); //Calculate view direction

		//Right Lighting
		//Calculate Ambient Lighting
		float ambientStrength = 0.5f; //Set ambient of global lighting strength
		vec3 ambient = ambientStrength * lightColor; //Generate ambient light color

		//Calculate Diffuse Lighting
		vec3 lightDirection = normalize(lightPos - FragmentPos); //Calculate distance light direction)
		float impact = max(dot(norm, lightDirection), 0.0); //Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse = impact * lightColor; //Generate diffuse light color

		//Calculate Specular lighting
		float specularIntensity = 1.0f; //Set specular light strength
		float highlightSize = 10.0f; //Set specular highlight size
		vec3 reflectDir = reflect(-lightDirection, norm); //Calculate reflection vector

		//Calculate specular component
		float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
		vec3 specular = specularIntensity * specularComponent * lightColor;

		//Left Lighting
		//Calculate Ambient Lighting
		float leftAmbientStrength = 0.1f; //Set ambient or global lighting strength
		vec3 leftAmbient = leftAmbientStrength * leftLightColor; //Generate ambient light color

		//Calculate Diffuse Lighting
		vec3 leftLightDirection = normalize(leftLightPos - FragmentPos); //Calculate distance (light direction between light source and fragments/pixels
		float leftImpact = max(dot(norm, leftLightDirection), 0.0); //Calculate diffuse impact by generating dot product of normal and light
		vec3 leftDiffuse = leftImpact * leftLightColor; //Generate diffuse light color

		//Calculate Speculat lighting
		float leftSpecularIntensity = 0.1f; //Set specular light strength
		float leftHighlightSize = 5.0f; //Set specular highlight size
		vec3 leftReflectDir = reflect(-leftLightDirection, norm); //Calculate reflection vector

		//Calculate specular component
		float leftSpecularComponent = pow(max(dot(viewDir, leftReflectDir), 0.0), leftHighlightSize);
		vec3 leftSpecular = leftSpecularIntensity * leftSpecularComponent * leftLightColor;

		//Calculate phong result
		vec3 rightLightResult = (ambient + diffuse + specular);
		vec3 leftLightResult = (leftAmbient + leftDiffuse + leftSpecular);
		vec3 textureColor = texture(uTexture, mobileTextureCoordinate).xyz;

		//vec3 phong2 = (rightLightResult + lefLightResult) * textureColor;

		poweradeTexture = vec4(textureColor, 1.0f); //Send lighting results to GPU*/
		//poweradeTexture = texture(uTexture, mobileTextureCoordinate);
	}
);

/*Lamp Shader Source Code*/
const GLchar * lampVertexShaderSource = GLSL(330,

        layout (location = 0) in vec3 position; //VAP position 0 for vertex position data

        //Uniform / Global variables for the transform matrices
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main()
        {
            gl_Position = projection * view * model * vec4(position, 1.0f); //Transforms vertices into clip coordinates
        }
);


/*Fragment Shader Source Code*/
const GLchar * lampFragmentShaderSource = GLSL(330,

        out vec4 color; //For outgoing lamp color

        void main()
        {
            color = vec4(1.0f); //Set color to white (1.0f, 1.0f, 1.0f) with alpha 1.0

        }
);

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	UCreateShader();

	UCreateBuffers();

	UGenerateTexture();

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f); //Set background color
	glutDisplayFunc(URenderGraphics);

	glutMouseFunc(UMouseClick); //Detects mouse click

	glutKeyboardFunc(UKeyboard); //Detect keyboard keys

	glutKeyboardUpFunc(UKeyReleased); //Detects key release

	glutPassiveMotionFunc(UMouseMove); //Detects mouse movement

	glutMotionFunc(UMousePressedMove); //Detects mouse press and movement

	glutMainLoop();

	//Destroys Buffer objects once used
	glDeleteVertexArrays(1, &BottleVAO);
	glDeleteVertexArrays(1, &TextureVAO);
	glDeleteVertexArrays(1, &LightVAO);
	glDeleteBuffers(1, &VBO);


	return 0;
}

//Resizes the window
void UResizeWindow(int w, int h) {
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

//Renders graphics
void URenderGraphics(void) {
	glEnable(GL_DEPTH_TEST); //Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clears the screen

	glUseProgram(shaderProgram);
	glBindVertexArray(BottleVAO); //Activate the Vertex Array object before rendering and transforming them


	CameraForwardZ = front; //Replaces camera forward vector with Radians normalized as a unit vector

	//Transforms the object
	glm::mat4 model;
	model = glm::translate(model,glm::vec3(0.0f, 0.0f, 0.0f)); //Place the object at the center of the viewport
	//model = glm::rotate(model, 90.0f, glm::vec3(-1.0f, 0.0f, 0.5f)); //Rotate the object 45 degrees on the z axis and -90 degrees on the y axis
	model = glm::rotate(model, 90.0f, glm::vec3(-1.0f, -1.0f, -1.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); //Increase the object size by a scale of 2

	//rotate logic conversion with hotkey r
	if (rotateLogic == true) {
		model = glm::rotate(model, glutGet(GLUT_ELAPSED_TIME) * -0.001f, glm::vec3(0.0, 0.0f, 1.0f));
	} else if (rotateLogic == false) {
		model = glm::rotate(model, glutGet(GLUT_ELAPSED_TIME) * -0.000f, glm::vec3(0.0, 0.0f, 1.0f));
	}
	//Transforms the camera
	glm::mat4 view;
	view = glm::lookAt(CameraForwardZ, cameraPosition, CameraUpY);

	//Creates a perspective projection
	glm::mat4 projection;
	if (zkeyViewLogic == false) {
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	} else if (zkeyViewLogic == true) {
		projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);
	}
	//Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//Reference matrix uniforms from the shader program for the color, light colors, light positions, and camera position
	GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor"); //right light color
	GLint lightPositionLoc = glGetUniformLocation(shaderProgram, "lightPos"); //right light position
	GLint leftLightColorLoc = glGetUniformLocation(shaderProgram, "leftLightColor"); //left light color
	GLint leftLightPositionLoc = glGetUniformLocation(shaderProgram, "leftLightPos"); //left light position
	GLint viewPositionLoc = glGetUniformLocation(shaderProgram, "viewPosition");

	//Pass color light, and camera data to the shader programs corresponding uniforms
	glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b); //right light color
	glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z); //right light position
	glUniform3f(leftLightColorLoc, leftLightColor.r, leftLightColor.g, leftLightColor.b); //left light color
	glUniform3f(leftLightPositionLoc, leftLightPosition.x, leftLightPosition.y, leftLightPosition.z); //left light position
	glUniform3f(viewPositionLoc, front.x, front.y, front.z);

	//Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, 402);

	glBindVertexArray(0); //Deactivate the Vertex array object

	//Use the texture shader program and activate the textureVAO for rendering and transforming
	glUseProgram(textureShaderProgram);
	glBindVertexArray(TextureVAO);

	//Reference matrix uniforms from the texture shader program
	modelLoc = glGetUniformLocation(textureShaderProgram, "model");
	viewLoc = glGetUniformLocation(textureShaderProgram, "view");
	projLoc = glGetUniformLocation(textureShaderProgram, "projection");

	//Pass matrix uniforms from the texture header program
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//Reference matrix uniforms from the shader program for the color, light colors, light positions, and camera position
	GLint uTextureLoc = glGetUniformLocation(textureShaderProgram, "uTexture");
	lightColorLoc = glGetUniformLocation(textureShaderProgram, "lightColor"); //right light color
	lightPositionLoc = glGetUniformLocation(textureShaderProgram, "lightPos"); //right light position
	leftLightColorLoc = glGetUniformLocation(textureShaderProgram, "leftLightColor"); //left light color
	leftLightPositionLoc = glGetUniformLocation(textureShaderProgram, "leftLightPos"); //left light position
	viewPositionLoc = glGetUniformLocation(textureShaderProgram, "viewPosition");

	//Pass color light, and camera data to the shader programs corresponding uniforms
	glUniform1i(uTextureLoc, 0); //texture unit 0
	glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b); //right light color
	glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z); //right light position
	glUniform3f(leftLightColorLoc, leftLightColor.r, leftLightColor.g, leftLightColor.b); //left light color
	glUniform3f(leftLightPositionLoc, leftLightPosition.x, leftLightPosition.y, leftLightPosition.z); //left light position
	glUniform3f(viewPositionLoc, front.x, front.y, front.z);

	glDrawArrays(GL_TRIANGLES, 402, 12); //draws the textured powerade object

	glBindVertexArray(0);

	//Use the Lamp shader program and activate the lamp vertex array object for rendering and transforming
	glUseProgram(lampShaderProgram);
	glBindVertexArray(LightVAO);

	//Transform the smaller pyramid used as a visual cue for the light source
	model = glm::translate(model, lightPosition);
	model = glm::scale(model, lightScale);

	//Reference matrix uniforms from the lmap shader program
	modelLoc = glGetUniformLocation(lampShaderProgram, "model");
	viewLoc = glGetUniformLocation(lampShaderProgram, "view");
	projLoc = glGetUniformLocation(lampShaderProgram, "projection");

	//Pass matrix uniforms from the lamp shader program
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glDrawArrays(GL_TRIANGLES, 0, 402); //Draws the triangles

	glBindVertexArray(0); //Deactivate the lamp VAO

	glutPostRedisplay();
	glutSwapBuffers(); //Flips the back buffer with the front buffer every frame. similar to GL flush
}

void UCreateShader() {
	//Vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); //Creates the Vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); //Attaches the Vertex shader to the source code
	glCompileShader(vertexShader); //Compiles the vertex shader

	//Fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Creates the Fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); //Attaches the fragment shader to the source code
	glCompileShader(fragmentShader); //compiles the fragment shader

	//Shader program
	shaderProgram = glCreateProgram(); //Creates the Shader program and returns an id
	glAttachShader(shaderProgram, vertexShader); //Attach Vertex shader to the shader program
	glAttachShader(shaderProgram, fragmentShader); //Attach fragment shader to the shader program
	glLinkProgram(shaderProgram); //Link Vertex and Fragment shaders to shader program

	//Delete the Vertex and Fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//texture shader program
	//Vertex shader
	GLint textureVertexShader = glCreateShader(GL_VERTEX_SHADER); //Creates the vertex shader
	glShaderSource(textureVertexShader, 1, &textureVertexShaderSource, NULL); //attaches the vertex shader to the source code
	glCompileShader(textureVertexShader); //compiles the vertex shader

	//Fragment shader
	GLint textureFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Creates the fragment shader
	glShaderSource(textureFragmentShader, 1, &textureFragmentShaderSource, NULL); //Attaches the fragment shader to the source code
	glCompileShader(textureFragmentShader); //compiles the fragment shader

	//shader program
	textureShaderProgram = glCreateProgram(); //Creates the shader program and returns an id
	glAttachShader(textureShaderProgram, textureVertexShader); //Attach vertex shader to shader program
	glAttachShader(textureShaderProgram, textureFragmentShader); //Attach fragment shader to shader program
	glLinkProgram(textureShaderProgram); //Link Vertex and Fragment shaders to shader program

	//Delete the vertex and fragment shaders once linked
	glDeleteShader(textureVertexShader);
	glDeleteShader(textureFragmentShader);

    //Lamp Vertex shader
    GLint lampVertexShader = glCreateShader(GL_VERTEX_SHADER); //Creates the Vertex shader
    glShaderSource(lampVertexShader, 1, &lampVertexShaderSource, NULL); //Attaches the Vertex shader to the source code
    glCompileShader(lampVertexShader); //Compiles the Vertex shader

    //Lamp Fragment shader
    GLint lampFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Creates the Fragment shader
    glShaderSource(lampFragmentShader, 1, &lampFragmentShaderSource, NULL); //Attaches the Fragment shader to the source code
    glCompileShader(lampFragmentShader); //Compiles the Fragment shader

    //Lamp Shader Program
    lampShaderProgram = glCreateProgram(); //Creates the Shader program and returns an id
    glAttachShader(lampShaderProgram, lampVertexShader); //Attach Vertex shader to the Shader program
    glAttachShader(lampShaderProgram, lampFragmentShader); //Attach Fragment shader to the Shader program
    glLinkProgram(lampShaderProgram); //Link Vertex and Fragment shaders to the Shader program

    //Delete the lamp shaders once linked
    glDeleteShader(lampVertexShader);
    glDeleteShader(lampFragmentShader);
}

void UCreateBuffers() {
	GLfloat vertices[] = {
			//Positions           //Color                   //Texture Coordinates Placeholder //Normals
			//Narrow Bottom Hexagon consisting of 6 triangles
			 //Top right triangle                                                             //Negative Z normals
			 0.0f,  0.0f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 0.0f,  0.45f, -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 //Middle right triangle
			 0.0f,  0.0f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 //bottom right triangle
			 0.0f,  0.0f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 0.0f, -0.45f, -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 //bottom left triangle
			 0.0f,  0.0f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 0.0f, -0.45f, -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			-0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 //middle left triangle
			 0.0f,  0.0f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			-0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			-0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 //top left triangle
			 0.0f,  0.0f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			-0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,
			 0.0f,  0.45f, -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       0.0f, 0.0f, -1.0f,

			//rectangles to reach Wide  Bottom Hexagon and z of -0.7
			 //top right rectangle                                                            //Positive X, Positive Y Normals
			 0.0f, 0.45f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.0f,  0.6f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.0f, 0.45f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 //middle right rectangle                                                         //Positive X Normals
			 0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 //bottom right rectangle                                                         //Positive X, Negative Y Normals
			 0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.0f, -0.6f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.0f, -0.6f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.0f, -0.45f, -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 //bottom left rectangle                                                          //Negative X, Negative Y Normals
			 0.0f, -0.6f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			 0.0f, -0.45f, -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			-0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			-0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			-0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			 0.0f, -0.45f, -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			 //middle left rectangle                                                         //Negative X Normals
			-0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			-0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			-0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			-0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			-0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			-0.4f, -0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 //top left rectangle                                                             //Negative x, Positive Y Normals
			-0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			-0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 0.0f,  0.6f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 0.0f,  0.6f,  -0.7f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 0.0f,  0.45f, -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			-0.4f,  0.2f,  -0.8f, 0.537f, 0.812f, 0.902f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,

			//rectangle to reach middle hexagon before folding grooves
			//top right rectangle                                                             //Positive X, Positive Y Normals
			 0.0f,  0.6f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.0f,  0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			 0.0f,  0.6f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 1.0f, 0.0f,
			//middle right rectangle                                                          //Positive X Normals
			 0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			 //bottom right rectangle                                                         //Positive X, Negative Y Normals
			 0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.0f, -0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.0f, -0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.0f, -0.6f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			 //bottom left rectangle                                                          //Negative X, NEgative X Normals
			 0.0f, -0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			 0.0f, -0.6f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			-0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			-0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			-0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			 0.0f, -0.6f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, -1.0f, 0.0f,
			 //middle left rectangle                                                          //Negative X Normals
			 -0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 //top left rectangle                                                             //Negative X, Positive Y Normals
			 -0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,  -0.7f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,

			 //3 hand grooves for right side
			  //1st hand groove middle right                                                  //Positive X Normals
			  0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.5f,  0.3f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.4f, -0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.5f,  0.3f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,

			  0.5f,  0.3f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.4f, -0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.4f, -0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  //1st hand groove bottom right                                                  //Positive X Normals, Negative Y
			  0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.4f, -0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.4f, -0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,

			  0.4f, -0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  //2nd hand groove middle right                                                  //Positive X Normals
			  0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f,  0.3f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.4f, -0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f,  0.3f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,

  			  0.5f,  0.3f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.4f, -0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.4f, -0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
			  //2nd hand groove bottom right                                                  //Positive X Normals, Negative Y
			  0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
	 		  0.4f, -0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
	 		  0.0f, -0.6f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
	 		  0.0f, -0.6f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
	 		  0.0f, -0.6f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.4f, -0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,

 			  0.4f, -0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
 			  0.0f, -0.6f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
 			  0.0f, -0.6f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
 			  0.0f, -0.6f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  //3rd hand groove middle right                                                  //Positive X Normals
			  0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f,  0.3f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
 			  0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
   			  0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
   			  0.4f, -0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
   			  0.5f,  0.3f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,

   			  0.5f,  0.3f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
   			  0.5f,  0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
   			  0.4f, -0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
   			  0.4f, -0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
  			  0.5f, -0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
 			  0.5f,  0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, 0.0f, 0.0f,
 			  //3rd hand groove bottom right                                                  //Positive X Normals, Negative Y
 			  0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
 	 		  0.4f, -0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
	 		  0.0f, -0.6f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
 	 		  0.0f, -0.6f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
	 		  0.0f, -0.6f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
 			  0.4f, -0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,

			  0.4f, -0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
			  0.5f, -0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
  			  0.0f, -0.6f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
  			  0.0f, -0.6f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
  			  0.0f, -0.6f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,
 			  0.5f, -0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                       1.0f, -1.0f, 0.0f,

			  //3 hand grooves for left side
			  //1st hand groove middle left                                                    //Negative X Normals
			 -0.5f, -0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,

			 -0.5f, -0.3f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 //1st hand groove top left                                                        //Negative X, Positive Y Normals
			  0.0f,  0.6f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,  -0.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,

			  0.0f,  0.6f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,  -0.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  //2nd hand groove middle left                                                    //Negative X Normals
			 -0.5f, -0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,

			 -0.5f, -0.3f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 //2nd hand groove top left                                                        //Negative X, Positive Y Normals
			  0.0f,  0.6f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,   0.2f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,

			  0.0f,  0.6f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,   0.5f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  //3rd hand groove middle left                                                    //Negative X Normals
			 -0.5f, -0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,

			 -0.5f, -0.3f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.4f,  0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f,  0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 -0.5f, -0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 0.0f, 0.0f,
			 //3rd hand groove top left                                                        //Negative X, Positive Y Normals
			  0.0f,  0.6f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,   0.8f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,

			  0.0f,  0.6f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.4f,  0.2f,   1.1f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			 -0.5f,  0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,
			  0.0f,  0.6f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      -1.0f, 1.0f, 0.0f,

			  //rectangles to top outter hexagon, missing top right and bottom left due to powerade textured logo

			  //middle right rectangle, all black                                             //Postive X Normals
			  0.5f,  0.3f,   1.4f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, 0.0f, 0.0f,
			  0.5f,  0.3f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, 0.0f, 0.0f,
			  0.5f, -0.3f,   1.4f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, 0.0f, 0.0f,
			  0.5f, -0.3f,   1.4f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, 0.0f, 0.0f,
			  0.5f, -0.3f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, 0.0f, 0.0f,
			  0.5f,  0.3f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, 0.0f, 0.0f,
			  //bottom right rectangle, all black                                             //Positive X, Negative Y Normals
			  0.5f, -0.3f,   1.4f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, -1.0f, 0.0f,
			  0.5f, -0.3f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,   1.4f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,   1.4f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, -1.0f, 0.0f,
			  0.0f, -0.6f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, -1.0f, 0.0f,
			  0.5f, -0.3f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                             1.0f, -1.0f, 0.0f,

			  //middle left rectangle, half baby blue, half black                             //Negative X Normals
			  -0.5f, -0.3f,   1.4f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           -1.0f, 0.0f, 0.0f,
			  -0.5f, -0.3f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           -1.0f, 0.0f, 0.0f,
			  -0.5f,  0.3f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           -1.0f, 0.0f, 0.0f,
			  -0.5f,  0.3f,   1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 0.0f,
			  -0.5f,  0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 0.0f,
			  -0.5f, -0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 0.0f,
			  //top left rectangle, half baby blue, half black                                //Negative X, Positive Y Normals
			   0.0f,  0.6f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           -1.0f, 1.0f, 0.0f,
			   0.0f,  0.6f,   1.4f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           -1.0f, 1.0f, 0.0f,
			  -0.5f,  0.3f,   1.9f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           -1.0f, 1.0f, 0.0f,
			  -0.5f,  0.3f,   1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 0.0f,
			  -0.5f,  0.3f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 0.0f,
			   0.0f,  0.6f,   1.4f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 0.0f,

			  //rectangles to top inner hexagon
			  //top right rectangle                                                           //Positive X, Positive Y, Positive Z Normals
			   0.0f, 0.6f,   1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 1.0f, 1.0f,
			   0.0f, 0.45f,  2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 1.0f, 1.0f,
			   0.5f, 0.3f,   1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 1.0f, 1.0f,
			   0.5f, 0.3f,   1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 1.0f, 1.0f,
			   0.4f, 0.2f,   2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 1.0f, 1.0f,
			   0.0f, 0.45f,  2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 1.0f, 1.0f,
			   //middle right rectangle                                                       //Positive X, Positive Z Normals
			   0.5f, 0.3f,   1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 0.0f, 1.0f,
			   0.4f, 0.2f,   2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 0.0f, 1.0f,
			   0.5f, -0.3f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 0.0f, 1.0f,
			   0.5f, -0.3f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 0.0f, 1.0f,
			   0.4f, -0.2f,  2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 0.0f, 1.0f,
			   0.4f, 0.2f,   2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, 0.0f, 1.0f,
			   //bottom right rectangle                                                       //Positive X, Negative Y, Positive Z Normals
			   0.5f, -0.3f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, -1.0f, 1.0f,
			   0.4f, -0.2f,  2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, -1.0f, 1.0f,
			   0.0f, -0.6f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, -1.0f, 1.0f,
			   0.0f, -0.6f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, -1.0f, 1.0f,
			   0.0f, -0.45f, 2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, -1.0f, 1.0f,
			   0.4f, -0.2f,  2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                      1.0f, -1.0f, 1.0f,
			   //bottom left rectangle                                                        //Negative X, Negative Y Normals, Positive Z Normals
			   0.0f, -0.6f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, -1.0f, 1.0f,
			   0.0f, -0.45f, 2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, -1.0f, 1.0f,
			   -0.5f, -0.3f, 1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, -1.0f, 1.0f,
			   -0.5f, -0.3f, 1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, -1.0f, 1.0f,
			   -0.4f, -0.2f, 2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, -1.0f, 1.0f,
			   0.0f, -0.45f, 2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, -1.0f, 1.0f,
			   //middle left rectangle                                                        //Negative X, Positive Z Normals
			   -0.5f, -0.3f, 1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 1.0f,
			   -0.4f, -0.2f, 2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 1.0f,
			   -0.5f, 0.3f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 1.0f,
			   -0.5f, 0.3f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 1.0f,
			   -0.4f, 0.2f,  2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 1.0f,
			   -0.4f, -0.2f, 2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 0.0f, 1.0f,
			   //top left rectangle                                                           //Negative X, Positive Y, Positive Z Normals
			   -0.5f, 0.3f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 1.0f,
			   -0.4f, 0.2f,  2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 1.0f,
			    0.0f, 0.6f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 1.0f,
				0.0f, 0.6f,  1.9f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 1.0f,
				0.0f, 0.45f, 2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 1.0f,
			   -0.4f, 0.2f,  2.0f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     -1.0f, 1.0f, 1.0f,

				//Rectangles to top cap hexagon, all black
				//top right rectangle                                                         //Positive X, Positive Y, Positive Z Normals
				0.0f,   0.45f, 2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.0f,   0.35f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.4f,   0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.4f,   0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.35f,  0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.0f,   0.35f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				//middle right rectangle                                                      //Positive X, Positive Z Normals
				0.4f,   0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.35f,  0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.4f,  -0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.4f,  -0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.35f, -0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.35f,  0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				//bottom right rectangle                                                      //Positive X, Negative Y, Positive Z Normals
				0.4f,  -0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.35f, -0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.0f,  -0.45f, 2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.0f,  -0.45f, 2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.0f,  -0.35f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.35f, -0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				//bottom left rectangle                                                       //Negative X, Negative Y, Positive Z Normals
				0.0f,  -0.45f, 2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
				0.0f,  -0.35f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
			   -0.4f,  -0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
			   -0.4f,  -0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
			   -0.35f, -0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
			    0.0f,  -0.35f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
				//middle left rectangle                                                       //Negative X, Positive Z Normals
			   -0.4f,  -0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.35f, -0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.4f,   0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.4f,   0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.35f,  0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.35f, -0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			    //top left rectangle                                                          //Negative X, Positive Y, Positive Z Normals
			   -0.4f,   0.2f,  2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
			   -0.35f,  0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
			    0.0f,   0.45f, 2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
				0.0f,   0.45f, 2.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
				0.0f,   0.35f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
			   -0.35f,  0.15f, 2.1f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,

			   //rectangles to bottom squirt hexagon, all black
			   //top right rectangle                                                           //Positive X, Positive Y, Positive Z Normals
			    0.0f,   0.35f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.0f,   0.15f,  2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.35f,  0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.35f,  0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.15f,  0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
				0.0f,   0.15f,  2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          1.0f, 1.0f, 1.0f,
			   //middle right rectangle                                                        //Positive X, Positive Z Normals
				0.35f,  0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.15f,  0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.35f, -0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.35f, -0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.15f, -0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				0.15f,  0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, 0.0f, 1.0f,
				//bottom right rectangle                                                       //Positive x, Negative Y, Positive Z Normals
				0.35f, -0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.15f, -0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.0f,  -0.35f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.0f,  -0.35f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.0f,  -0.15f,  2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				0.15f, -0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                           1.0f, -1.0f, 1.0f,
				//bottom left rectangle                                                        //Negative x, Negative Y, Positive Z Normals
				0.0f,  -0.35f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
				0.0f,  -0.15f,  2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
			   -0.35f, -0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
			   -0.35f, -0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
			   -0.15f, -0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
			    0.0f,  -0.15f,  2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, -1.0f, 1.0f,
				//middle left rectangle                                                        //Negative x, Positive Z Normals
			   -0.35f, -0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.15f, -0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.35f,  0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.35f,  0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.15f,  0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   -0.15f, -0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 0.0f, 1.0f,
			   // top left rectangle                                                           //Negative X, Positive Y, Positive Z Normals
			   -0.35f,  0.15f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
			   -0.15f,  0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
			    0.0f,   0.35f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
				0.0f,   0.35f,  2.1f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
				0.0f,   0.15f,  2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,
			   -0.15f,  0.075f, 2.15f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,                          -1.0f, 1.0f, 1.0f,

			   //rectangles to top inner squirt hexagon, baby blue color
			    //top right rectangle                                                           //Positive X, Positive Y Normals
			    0.0f,   0.15f,  2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 1.0f, 0.0f,
				0.0f,   0.1f,   2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 1.0f, 0.0f,
				0.15f,  0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 1.0f, 0.0f,
				0.15f,  0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 1.0f, 0.0f,
				0.1f,   0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 1.0f, 0.0f,
				0.0f,   0.1f,   2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 1.0f, 0.0f,
				//middle right rectangle                                                        //Positive X Normals
				0.15f,  0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 0.0f, 0.0f,
				0.1f,   0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 0.0f, 0.0f,
				0.15f, -0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 0.0f, 0.0f,
				0.15f, -0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 0.0f, 0.0f,
				0.1f,  -0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 0.0f, 0.0f,
				0.1f,   0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, 0.0f, 0.0f,
				//bottom right rectangle                                                        //Positive X, Negative y Normals
				0.15f, -0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, -1.0f, 0.0f,
				0.1f,  -0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, -1.0f, 0.0f,
				0.0f,  -0.15f,  2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, -1.0f, 0.0f,
				0.0f,  -0.15f,  2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, -1.0f, 0.0f,
				0.0f,  -0.1f,   2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, -1.0f, 0.0f,
				0.1f,  -0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                     1.0f, -1.0f, 0.0f,
				//bottom left rectangle                                                         //Negative x, Negative y Normals
				0.0f,  -0.15f,  2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, -1.0f, 0.0f,
				0.0f,  -0.1f,   2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, -1.0f, 0.0f,
			   -0.15f, -0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, -1.0f, 0.0f,
			   -0.15f, -0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, -1.0f, 0.0f,
			   -0.1f,  -0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, -1.0f, 0.0f,
			    0.0f,  -0.1f,   2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, -1.0f, 0.0f,
				//middle left rectangle                                                         //Negative x Normals
			   -0.15f, -0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 0.0f, 0.0f,
			   -0.1f,  -0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 0.0f, 0.0f,
			   -0.15f,  0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 0.0f, 0.0f,
			   -0.15f,  0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 0.0f, 0.0f,
			   -0.1f,   0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 0.0f, 0.0f,
			   -0.1f,  -0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 0.0f, 0.0f,
			   //top left rectangle                                                             //Negative x, Positive y Normals
			   -0.15f,  0.075f, 2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 1.0f, 0.0f,
			   -0.1f,   0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 1.0f, 0.0f,
			    0.0f,   0.15f,  2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 1.0f, 0.0f,
				0.0f,   0.15f,  2.15f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 1.0f, 0.0f,
				0.0f,   0.1f,   2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 1.0f, 0.0f,
			   -0.1f,   0.05f,  2.25f, 0.537f, 0.812f, 0.942f,   0.0f, 0.0f,                    -1.0f, 1.0f, 0.0f,

				//Black bars with powerade logo texture going up top-right and bottom-left side
				 //top right rectangle
										                  //texture coordinates                //Positive x, Positive y Normals
				  0.0f,  0.6f,   1.9f,  0.0f, 0.0f, 0.0f, 0.85f, 0.0f,                          1.0f, 1.0f, 0.0f,//top right
				  0.0f,  0.6f,  -0.4f,  0.0f, 0.0f, 0.0f, -0.15f, 0.0f,                         1.0f, 1.0f, 0.0f,//bottom right
				  0.5f,  0.3f,   1.9f,  0.0f, 0.0f, 0.0f, 0.85f, 1.0f,                          1.0f, 1.0f, 0.0f,//top left
				  0.5f,  0.3f,   1.9f,  0.0f, 0.0f, 0.0f, 0.85f, 1.0f,                          1.0f, 1.0f, 0.0f,//top left
				  0.5f,  0.3f,  -0.4f,  0.0f, 0.0f, 0.0f, -0.15f, 1.0f,                         1.0f, 1.0f, 0.0f,//bottom left
				  0.0f,  0.6f,  -0.4f,  0.0f, 0.0f, 0.0f, -0.15f, 0.0f,                         1.0f, 1.0f, 0.0f,//bottom right
				 // bottom-left rectangle                                                      //Negative x, Negative y Normals
				  0.0f, -0.6f,   1.9f,  0.0f, 0.0f, 0.0f, 0.85f, 0.0f,                         -1.0f, -1.0f, 0.0f,//top right
				  0.0f, -0.6f,  -0.4f,  0.0f, 0.0f, 0.0f, -0.15f, 0.0f,                        -1.0f, -1.0f, 0.0f,//bottom right
				 -0.5f, -0.3f,   1.9f,  0.0f, 0.0f, 0.0f, 0.85f, 1.0f,                         -1.0f, -1.0f, 0.0f,//top left
				 -0.5f, -0.3f,   1.9f,  0.0f, 0.0f, 0.0f, 0.85f, 1.0f,                         -1.0f, -1.0f, 0.0f,//top left
				 -0.5f, -0.3f,  -0.4f,  0.0f, 0.0f, 0.0f, -0.15f, 1.0f,                        -1.0f, -1.0f, 0.0f,//bottom left
				  0.0f, -0.6f,  -0.4f,  0.0f, 0.0f, 0.0f, -0.15f, 0.0f,                        -1.0f, -1.0f, 0.0f,//bottom right
	};

	/*Buffer for vertices with color*/
	//Generate buffer ids
	glGenVertexArrays(1, &BottleVAO);
	glGenBuffers(1, &VBO);

	//Activate the Vertex Array Object before binding and setting any VBOs and Vertex Attribute Pointers
	glBindVertexArray(BottleVAO);

	//Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //Copy vertices to VBO

	//Set attribute pointer 0 to hold position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); //Enables vertex attribute

	//Set attribute point 1 to hold color data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); //Enables vertex attribute

	//Set attribute pointer 3 to hold Normals
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); //Deactivates the VAO

	//Generate buffer ids for texture
	glGenVertexArrays(1, &TextureVAO);

	//Activate the Texture VAO before binding and setting any VBOs and VAPs
	glBindVertexArray(TextureVAO);

	//REferencing the smae VBO for its vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Set attribute pointer 0 to hold position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); //Enables vertex attribute

	//Set attribute point 2 to hold texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2); //Enables vertex attribute

	//Set attribute pointer 3 to hold Normals
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); //Deactivates the TextureVAO which is good practice

	//Generate buffer ids for lamp (smaller pyramid)
	glGenVertexArrays(1, &LightVAO); //AO of small pyramid as light source

	//Activate the VAO before binding and setting any VBOs and VAPs
	glBindVertexArray(LightVAO);

	//Referencing the same VBO for its vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Set attribute pointer to 4 to hold position data used for the lamp
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0); //Deactivates the LightVAO

}

void UKeyboard(unsigned char key, GLint x, GLint y) {
	switch(key) {
	// hotkeys p and o to switch between perspective and orthogonal view
	case 'p':
		zkeyViewLogic = true;
		currentKey = key;
		break;

	case 'o':
		zkeyViewLogic = false;
		currentKey = key;
		break;

	//hotkeys r and s to switch between rotating or still view
	case 'r':
		rotateLogic = true;
		currentKey = key;
		break;

	case 's':
		rotateLogic = false;
		currentKey = key;
		break;
	}
}

//Implements the UKeyReleased function
void UKeyReleased(unsigned char key, GLint x, GLint y) {
	cout<<"Key released!"<<endl;
	currentKey = '0';
}

void UMouseClick(int button, int state, int x, int y) {
	//int mod = glutGetModifiers(); //Initialize glut alt glut modifier
	if((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		orbitLogic = true;
		//cout<<"Left Mouse Button Clicked!"<< orbitLogic  <<endl; //console out statement to check for logic

	}

	if((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		zoomLogic = true;
		//cout<<"Right Mouse Button Clicked!"<<endl;

	}

	if((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
		zoomLogic = false;
		//cout<<"Right Mouse Button Released!"<<endl;

	}

	if((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		orbitLogic = false;
		//cout<<"Left mouse Button Released!"<< orbitLogic <<endl;

	}
}

//Implements the UMouseMove function
void UMouseMove(int x, int y) {
	//Immediately replaces center locked coordinates with new mouse coordinates
			if((mouseDetected == true)) {
				lastMouseX = x;
				lastMouseY = y;
				mouseDetected = false;
			}

			//Gets the direction the mouse was moved in x and y

				mouseXOffset = x - lastMouseX;
				mouseYOffset = lastMouseY - y; //Inverted Y


				//Updates with new mouse coordinates
				lastMouseX = x;
				lastMouseY = y;

				//Applies sensitivity to mouse direction
				mouseXOffset *= sensitivity;
				mouseYOffset *= sensitivity;

				//Accumulates the yaw and pitch variables
				yaw += mouseXOffset;
				pitch += mouseYOffset;



				//Orbits around the center
				front.x = zoomD * cos(yaw);
				front.y = zoomD * sin(pitch);
				front.z = sin(yaw) * cos(pitch) * zoomD;
}
//Implements the UMousePressedMovefunction
void UMousePressedMove(int x, int y) {
	//Immediately replaces center locked coordinates with new mouse coordinates
		if((mouseDetected == true)) {
			lastMouseX = x;
			lastMouseY = y;
			mouseDetected = false;
		}

		//Gets the direction the mouse was moved in x and y

			mouseXOffset = x - lastMouseX;
			mouseYOffset = lastMouseY - y; //Inverted Y


			//Updates with new mouse coordinates
			lastMouseX = x;
			lastMouseY = y;

			//Applies sensitivity to mouse direction
			mouseXOffset *= sensitivity;
			mouseYOffset *= sensitivity;



		/* if condition for the zoomLogic, determined by the right mouse button*/
		if(zoomLogic) {
			if (mouseYOffset < 0) {
				front *= zoomInScalar; //multiply the front view with the zoom in scalar
				zoomD *= zoomInScalar; //update zoomD with appropriate scalar
			}
			if (mouseYOffset > 0) {
				front *= zoomOutScalar; //multiply the front view with the zoom out scalar
				zoomD *= zoomOutScalar; //update zoomD with appropriate scalar
			}
		}
}

/*Generate and load texture*/
void UGenerateTexture(){
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height;

	unsigned char* image = SOIL_load_image("powerade.jpg", &width, &height, 0, SOIL_LOAD_RGB); //Load texture file

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); //unbind texture
}
