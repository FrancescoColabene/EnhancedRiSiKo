// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)

struct MeshUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct PlainUniformBlock {
	alignas(4) float visible;
};

struct GlobalUniformBlock {
	alignas(16) glm::vec3 DlightDir;
	alignas(16) glm::vec3 DlightColor;
	alignas(16) glm::vec3 AmbLightColor;
	alignas(16) glm::vec3 eyePos;
};

// The uniform buffer object copied from A07
struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
};



// Gamestate enum structure - it starts from 0
enum GameState {
	WALK,
	TANK, 
	CAR,
	HELI
};

// The vertices data structures
struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

/* FinalProject */
/* Add the C++ datastructure for the required vertex format */
struct VertexMonoColor {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

// MAIN ! 
class FinalProject : public BaseProject {
protected:

	// Current aspect ratio (used by the callback that resized the window)
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLGubo;
	/* FinalProject */
	/* Add the variable that will contain the required Descriptor Set Layout */
	DescriptorSetLayout DSLMonoColor;
	DescriptorSetLayout DSLVertexFloor;

	// Vertex formats

	/* FinalProject */
	/* Add the variable that will contain the required Vertex format definition */
	VertexDescriptor VMonoColor;
	VertexDescriptor VVertexFloor;

	// Pipelines [Shader couples]
	/* FinalProject */
	/* Add the variable that will contain the new pipeline */
	Pipeline PMonoColor;
	Pipeline PVertexFloor;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	/* FinalProject */
	/* Add the variable that will contain the model for the room */
	Model<VertexMonoColor> MTank;
	Model<VertexMonoColor> MFloor;

	DescriptorSet DSGubo;
	/* FinalProject */
	/* Add the variable that will contain the Descriptor Set for the room */
	DescriptorSet DSTank;
	DescriptorSet DSFloor;

	Texture TTank;
	Texture TFloor;

	// C++ storage for uniform variables
	/* FinalProject */
	/* Add the variable that will contain the Uniform Block in slot 0, set 1 of the room */
	MeshUniformBlock uboTank;
	UniformBufferObject uboFloor;

	GlobalUniformBlock gubo;

	// ------------------------------------------------------------------------------------

	// Other application parameters
	GameState gameState = GameState::WALK;


	// guardando A16 e A07 non sto capendo dove andrebbero messe queste variabili - qui o dove c'è la logica effettiva

	// si potrebbe togliere il const e permettere di cambiare fov al player quando si trova in prima persona - magari con un'altra enum
	const float FOVy = glm::radians(70.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 150.0f;

	// Player starting point + initialization
	const glm::vec3 StartingPosition = glm::vec3(5.0f, 1.6f, 5.0f);
	const glm::vec3 TankStartingPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 playerPosition = StartingPosition,
			  oldPos = TankStartingPosition,
			  newPos = TankStartingPosition,
			  tankPosition = glm::vec3(0.0f, 0.0f, 0.0f);

	// si potrebbe usare una sola variabile e cambiarla dentro allo switch, not sure
	// Camera target height and distance for the tank view
	const float tankCamHeight = 2.5f;
	const float tankCamDist = 9.0f;
	// Camera target height and distance for the tank view
	const float carCamHeight = 2.5f;
	const float carCamDist = 9.0f;
	// Camera target height and distance for the tank view
	const float heliCamHeight = 2.5f;
	const float heliCamDist = 9.0f;
	// Camera Pitch limits - valid for every view 
	const float vehicleMinPitch = glm::radians(-8.75f);
	const float vehicleMaxPitch = glm::radians(60.0f);
	// Camera Pitch limits - valid for every view 
	const float playerMinPitch = glm::radians(-50.0f);
	const float playerMaxPitch = glm::radians(70.0f);
	// Rotation and motion speed - ancora da definire per bene
	const float ROT_SPEED = glm::radians(120.0f);
	const float MOVE_SPEED = 20.0f;
	const float TANK_ROT_SPEED = glm::radians(0.1f);
	const float TANK_MOVE_SPEED = 3.0f;
	

	// Parameters needed in the damping implementation - 3rd person view
	const float LAMBDAROT = 20.0f,
				LAMBDAMOV = 10.0f,
				DEADZONE = 1.0f;
	
	// queste molto probabilmente andrebbero messe sotto
	// Angles and variables needed to implement damping - independet player rotation from the camera
	float yaw = 0.0f,	  pitch = 0.0f,		roll = 0.0f,  // il roll sarebbe da togliere
		  yawOld = 0.0f,  pitchOld = 0.0f,
		  yawNew = 0.0f,  pitchNew = 0.0f,
		  tankYaw = 0.0f;


	// ------------------------------------------------------------------------------------


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Enhanced RiSiKo!";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.80f, 1.0f, 1.0f, 1.0f };

		// Descriptor pool sizes
		/* FinalProject */
		/* Update the requirements for the size of the pool */
		uniformBlocksInPool = 3; // prima era 9
		texturesInPool = 2;
		setsInPool = 3; // prima era 9

		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
	}

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		/* FinalProject */
		/* Init the new Data Set Layout */
		DSLVertexFloor.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLMonoColor.init(this, { 
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		// Vertex descriptors

		/* FinalProject */
		VMonoColor.init(this, { 
			// this array contains the bindings
			// first  element : the binding number
			// second element : the stride of this binding
			// third  element : whether this parameter change per vertex or per instance
			//                  using the corresponding Vulkan constant
			{0, sizeof(VertexMonoColor), VK_VERTEX_INPUT_RATE_VERTEX }
			}, {
				// this array contains the location
				// first  element : the binding number
				// second element : the location number
				// third  element : the offset of this element in the memory record
				// fourth element : the data type of the element
				//                  using the corresponding Vulkan constant
				// fifth  elmenet : the size in byte of the element
				// sixth  element : a constant defining the element usage
				//                   POSITION - a vec3 with the position
				//                   NORMAL   - a vec3 with the normal vector
				//                   UV       - a vec2 with a UV coordinate
				//                   COLOR    - a vec4 with a RGBA color
				//                   TANGENT  - a vec4 with the tangent vector
				//                   OTHER    - anything else
				//
				// ***************** DOUBLE CHECK ********************
				//    That the Vertex data structure you use in the "offsetoff" and
				//	in the "sizeof" in the previous array, refers to the correct one,
				//	if you have more than one vertex format!
				// ***************************************************
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMonoColor, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMonoColor, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMonoColor, UV),
					   sizeof(glm::vec2), UV}
			});

		VVertexFloor.init(this, {
			{0, sizeof(VertexMonoColor), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMonoColor, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMonoColor, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMonoColor, UV),
					   sizeof(glm::vec2), UV}
			});

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		/* FinalProject */
		/* Create the new pipeline, using shaders "VColorVert.spv" and "VColorFrag.spv" */
		PMonoColor.init(this, &VMonoColor, "shaders/Tank/MonoColorVert.spv", "shaders/Tank/MonoColorFrag.spv", { &DSLGubo, &DSLMonoColor });
		PVertexFloor.init(this, &VVertexFloor, "shaders/Floor/FloorVert.spv", "shaders/Floor/FloorFrag.spv", { &DSLVertexFloor });
		PVertexFloor.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);
		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		/* FinalProject */
		/* load the mesh for the room, contained in OBJ file "Room.obj" */
		MTank.init(this, &VMonoColor, "Models/Tank.obj", OBJ);
		MFloor.vertices = 
			//		POS			   UV
		{     { {-50.0f, 0.2f, 25.0f} , {0,1,0} , {0,1} } ,
			  { { 50.0f, 0.2f, 25.0f} , {0,1,0} , {1,1} } ,
			  { {-50.0f, 0.2f,-25.0f} , {0,1,0} , {0,0} } ,
			  { { 50.0f, 0.2f,-25.0f} , {0,1,0} , {1,0} } };
		MFloor.indices = 
			{ 0, 1, 2, 
			  1, 2, 3 };
		MFloor.initMesh(this, &VVertexFloor);

		// Create the textures
		// The second parameter is the file name
		TTank.init(this, "textures/Red.png");
		TFloor.init(this, "textures/RisikoMap.png");

		// Init local variables
		// TODO ci sarebbe da inizializzare variabili, ma non mi convince molto
		gameState = GameState::WALK;
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		/* FinalProject */
		/* Create the new pipeline */
		PMonoColor.create();
		PVertexFloor.create();
		// Here you define the data set

		/* FinalProject */
		/* Define the data set for the room */
		DSTank.init(this, &DSLMonoColor, {
			// the second parameter, is a pointer to the Uniform Set Layout of this set
			// the last parameter is an array, with one element per binding of the set.
			// first  elmenet : the binding number
			// second element : UNIFORM or TEXTURE (an enum) depending on the type
			// third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
			// fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TTank}
			});
		
		DSFloor.init(this, &DSLVertexFloor, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{2, TEXTURE, 0, &TFloor}
			});

		DSGubo.init(this, &DSLGubo, {
					{0, UNIFORM, sizeof(GlobalUniformBlock), nullptr}
			});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		/* FinalProject */
		/* cleanup the new pipeline */
		PMonoColor.cleanup();
		PVertexFloor.cleanup();
		// Cleanup datasets
		/* FinalProject */
		/* cleanup the dataset for the room */
		DSTank.cleanup();
		DSFloor.cleanup();
		DSGubo.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		TTank.cleanup();
		TFloor.cleanup();

		// Cleanup models
		/* FinalProject */
		/* Cleanup the mesh for the room */
		MTank.cleanup();
		MFloor.cleanup();
		// Cleanup descriptor set layouts
		/* FinalProject */
		/* Cleanup the new Descriptor Set Layout */
		DSLMonoColor.cleanup();
		DSLVertexFloor.cleanup();
		DSLGubo.cleanup();

		/* FinalProject */
		/* Destroy the new pipeline */
		PMonoColor.destroy();
		PVertexFloor.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		/* FinalProject */
		/* Insert the commands to draw the room */

		// binds the pipeline
		PMonoColor.bind(commandBuffer);
		// binds the gubo to the command buffer to our new pipeline and set 0
		DSGubo.bind(commandBuffer, PMonoColor, 0, currentImage);
		// binds the mesh
		MTank.bind(commandBuffer);
		// binds the descriptor set layout
		DSTank.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MTank.indices.size()), 1, 0, 0, 0);

		// binds the pipeline
		PVertexFloor.bind(commandBuffer);
		// binds the mesh
		MFloor.bind(commandBuffer);
		// binds the descriptor set layout
		DSFloor.bind(commandBuffer, PVertexFloor, 0, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MFloor.indices.size()), 1, 0, 0, 0);
		
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		
		glm::vec3 camPos;
		glm::mat4 ViewPrj;
		glm::mat4 WorldPlayer, WorldTank, WorldCar, WorldHeli;

		// Function that contains all the logic of the game
		Logic(Ar, ViewPrj, WorldPlayer, WorldTank, WorldCar, WorldHeli ,camPos);




		// gubo values
		gubo.DlightDir = glm::normalize(glm::vec3(1, 2, 3));
		gubo.DlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.AmbLightColor = glm::vec3(0.1f);
		gubo.eyePos = camPos;

		// Writes value to the GPU
		DSGubo.map(currentImage, &gubo, sizeof(gubo), 0);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block

		
		/* FinalProject */
		/* fill the uniform block for the room. Identical to the one of the body of the slot machine */
		// THE TANK IS THE PLAYER RN, BCS I USE THE WORLD AND VIEWPROJ MATRIX CALCULATED FOR THE PLAYER FOR ITS COORDINATES.
		uboTank.amb = 1.0f; uboTank.gamma = 180.0f; uboTank.sColor = glm::vec3(1.0f);
		uboTank.mvpMat = ViewPrj * WorldTank;
		uboTank.mMat = WorldTank;
		uboTank.nMat = glm::inverse(glm::transpose(WorldTank));
		/* map the uniform data block to the GPU */
		DSTank.map(currentImage, &uboTank, sizeof(uboTank), 0);

		uboFloor.mMat = glm::mat4(1);
		uboFloor.mvpMat = ViewPrj * uboFloor.mMat;
		uboFloor.nMat = glm::inverse(glm::transpose(uboFloor.mMat));
		DSFloor.map(currentImage, &uboFloor, sizeof(uboFloor), 0);

	}

	void Logic(float Ar, glm::mat4 &ViewPrj, glm::mat4 &WorldPlayer, glm::mat4 &WorldTank, glm::mat4 &WorldCar, glm::mat4 &WorldHeli, glm::vec3 &camPos) {

		// Integration with the timers and the controllers
			// returns:
			// <float deltaT> the time passed since the last frame
			// <glm::vec3 m> the state of the motion axes of the controllers (-1 <= m.x, m.y, m.z <= 1)
			// <glm::vec3 r> the state of the rotation axes of the controllers (-1 <= r.x, r.y, r.z <= 1)
			// <bool fire> if the user has pressed a fire button (not required in this assignment)
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);
		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button


		// placeholder variables for matrices
		glm::mat4 ViewMatrix, ProjectionMatrix, tempWorld;


		static bool updatePos = false;


		// LOGIC OF THE APPLICATION
		//TODO implement first person view

		// computing camera angles (except pitch) 
		yaw -= ROT_SPEED * r.y * deltaT;
		roll += ROT_SPEED * r.z * deltaT;

		// computing the movement versors (the player position update it's dependant on the gameState)
		glm::vec3 ux = glm::vec3(glm::rotate(glm::mat4(1),
								 yaw,
								 glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
		glm::vec3 uy = glm::vec3(0, 1, 0);
		glm::vec3 uz = glm::vec3(glm::rotate(glm::mat4(1),
								 yaw,
								 glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));


		switch (gameState)
		{
		case WALK:

			// updating player position
			playerPosition += ux * MOVE_SPEED * m.x * deltaT;
			playerPosition += uz * MOVE_SPEED * m.z * deltaT;
			// updating pitch
			pitch -= ROT_SPEED * r.x * deltaT;
			pitch = pitch < playerMinPitch ? playerMinPitch :
				(pitch > playerMaxPitch ? playerMaxPitch : pitch);

			break;

		case TANK:

			ux = glm::vec3(	glm::rotate(glm::mat4(1),
							tankYaw,
							glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
			uy = glm::vec3(0, 1, 0);
			uz = glm::vec3( glm::rotate(glm::mat4(1),
							tankYaw,
							glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

			// updating tank position - DA MODIFICARE
			tankPosition += ux * TANK_MOVE_SPEED * m.z * deltaT;
			tankPosition += uz * TANK_MOVE_SPEED * m.z * deltaT;

			// updating pitch
			pitch -= ROT_SPEED * r.x * deltaT;
			pitch = pitch < vehicleMinPitch ? vehicleMinPitch :
				(pitch > vehicleMaxPitch ? vehicleMaxPitch : pitch);

			// damping implementation
			pitchNew = (pitchOld * exp(-LAMBDAROT * deltaT)) + pitch * (1 - exp(-LAMBDAROT * deltaT));
			pitchOld = pitchNew;

			yawNew = (yawOld * exp(-LAMBDAROT * deltaT)) + yaw * (1 - exp(-LAMBDAROT * deltaT));
			yawOld = yawNew;

			// deadzone implementation
			if (glm::length((tankPosition - oldPos)) > DEADZONE) {
				updatePos = true;
			}

			if (updatePos) {
				newPos = (oldPos * exp(-LAMBDAMOV * deltaT)) + tankPosition * (1 - exp(-LAMBDAMOV * deltaT));
				oldPos = newPos;
				if (oldPos == tankPosition) {
					updatePos = false;
				}
			}

			// player rotating independently from the camera
			if (m.z == 0 && m.x != 0 )
				tankYaw -= m.x * TANK_ROT_SPEED;

			break;

		case CAR:

			

			break;

		case HELI:


			// updating player position
			
			// blocking the player from going under the terrain
			if ((playerPosition + uy * MOVE_SPEED * m.y * deltaT).y < 0.0f) {
				playerPosition.y = 0.0f;
			}
			else
			{
				playerPosition += uy * MOVE_SPEED * m.y * deltaT;
			}

			break;

		default:
			printf("\n\nSOMETHING'S WRONG I CAN FEEL IT\n");
			break;
		}

		// UPDATE ALL WORLD MATRICES (except for the player, since it doesn't have a model)

		

		// Tank World Matrix -- capire se è necessario quel +45 gradi o se si può sistemare in altra maniera
		tempWorld =
			glm::translate(glm::mat4(1), tankPosition) *
			glm::mat4(glm::quat(glm::vec3(0, tankYaw + glm::radians(45.0f), 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1));
		WorldTank = tempWorld;

		// Car World Matrix


		// Heli World Matrix



		switch (gameState)
		{
		case WALK:

			ViewPrj = MakeViewProjectionMatrix(Ar, yaw, pitch, roll, playerPosition);

			if (glfwGetKey(window, GLFW_KEY_T)) {
				gameState = GameState::TANK;

			}

			break;
		case TANK:

			// World Matrix used for the camera
			tempWorld = glm::translate(glm::mat4(1), oldPos) *
				glm::mat4(glm::quat(glm::vec3(0, yawNew, 0))) *
				glm::scale(glm::mat4(1), glm::vec3(1));

			// calculating the View-Projection Matrix
			glm::vec4 temp = tempWorld * glm::vec4(0, tankCamHeight + (tankCamDist * sin(pitchNew)), tankCamDist * cos(pitchNew), 1);
			camPos = glm::vec3(temp.x, temp.y, temp.z);


			glm::vec3 targetPointedPosition;
			targetPointedPosition = glm::vec3(newPos.x, newPos.y + tankCamHeight, newPos.z);

			ViewMatrix = glm::lookAt(camPos, targetPointedPosition, glm::vec3(0, 1, 0));
			ViewMatrix = glm::rotate(glm::mat4(1), roll, glm::vec3(0, 0, 1)) * ViewMatrix;

			ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
			ProjectionMatrix[1][1] *= -1;

			ViewPrj = ProjectionMatrix * ViewMatrix;

			if (glfwGetKey(window, GLFW_KEY_Y)) {
				gameState = GameState::WALK;

			}

			break;
		case CAR:

			if (glfwGetKey(window, GLFW_KEY_Y)) {
				gameState = GameState::WALK;

			}

			break;
		case HELI:

			if (glfwGetKey(window, GLFW_KEY_Y)) {
				gameState = GameState::WALK;

			}

			break;
		default:
			printf("\n\nSOMETHING'S WRONG I CAN FEEL IT\n");
			break;
		}

	}

	glm::mat4 MakeViewProjectionMatrix(float Ar, float Yaw, float Pitch, float Roll, glm::vec3 Pos) {
		// The view matrix uses the Look-in-Direction model, with vector <pos> specifying the 
		// position of the camera and yaw, pitch and roll specifying the various angles.

		glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

		ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		ProjectionMatrix[1][1] *= -1;

		ViewMatrix =
			glm::rotate(glm::mat4(1), -Roll, glm::vec3(0, 0, 1)) *
			glm::rotate(glm::mat4(1), -Pitch, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1), -Yaw, glm::vec3(0, 1, 0)) *
			glm::translate(glm::mat4(1), glm::vec3(-Pos.x, -Pos.y, -Pos.z));

		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		return ViewProjectionMatrix;
	}

};




// This is the main: probably you do not need to touch this!
int main() {
	FinalProject app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}