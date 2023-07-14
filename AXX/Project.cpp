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
	Model<VertexMonoColor> MHeliFull;
	Model<VertexMonoColor> MHeliBody;
	Model<VertexMonoColor> MHeliBladeFront;
	Model<VertexMonoColor> MHeliBladeBack;
	Model<VertexMonoColor> MCar;
	Model<VertexMonoColor> MFloor;

	DescriptorSet DSGubo;
	/* FinalProject */
	/* Add the variable that will contain the Descriptor Set for the room */
	DescriptorSet DSTank;
	DescriptorSet DSHeliFull;
	DescriptorSet DSHeliBody;
	DescriptorSet DSHeliBladeFront;
	DescriptorSet DSHeliBladeBack;
	DescriptorSet DSCar;
	DescriptorSet DSFloor;

	Texture TTank;
	Texture THeliFull;
	Texture THeliBody;
	Texture THeliBladeFront;
	Texture THeliBladeBack;
	Texture TCar;
	Texture TFloor;

	// C++ storage for uniform variables
	/* FinalProject */
	/* Add the variable that will contain the Uniform Block in slot 0, set 1 of the room */
	MeshUniformBlock uboTank;
	MeshUniformBlock uboHeliFull; 
	MeshUniformBlock uboHeliBody;
	MeshUniformBlock uboHeliBladeFront;
	MeshUniformBlock uboHeliBladeBack;
	MeshUniformBlock uboCar;

	UniformBufferObject uboFloor;

	GlobalUniformBlock gubo;

	// ------------------------------------------------------------------------------------

	// Other application parameters
	GameState gameState = GameState::WALK;
	bool transition = false;
	bool possoScendere = true;
	const float TRANS_DURATION = 0.9f;
	float transitionTimer = 0.0f;
	float closestDistance = 0.0f;


	// guardando A16 e A07 non sto capendo dove andrebbero messe queste variabili - qui o dove c'è la logica effettiva

	// si potrebbe togliere il const e permettere di cambiare fov al player quando si trova in prima persona - magari con un'altra enum
	const float FOVy = glm::radians(70.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 150.0f;

	// Player starting point + initialization
	const float PLAYER_HEIGHT = 1.6f;
	const float HELI_GROUND = 1.68f; // 1.68f è l'altezza giusta dal pavimento

	const glm::vec3 PlayerStartingPos = glm::vec3(0.0f, PLAYER_HEIGHT, 0.0f);
	const glm::vec3 TankStartingPos = glm::vec3(-7.0f, 0.0f, -12.0f);
	const glm::vec3 CarStartingPos = glm::vec3(0.0f, 0.0f, -12.0f);
	const glm::vec3 HeliStartingPos = glm::vec3(7.0f, HELI_GROUND, -12.0f);
	glm::vec3
		playerPosition = PlayerStartingPos,
		// Tank, heli and car need 3 variables for the damping implementation
		tankPosition = TankStartingPos,
		oldTankPos = TankStartingPos,
		newTankPos = TankStartingPos,

		carPosition = CarStartingPos,
		oldCarPos = CarStartingPos,
		newCarPos = CarStartingPos,

		heliPosition = HeliStartingPos,
		oldHeliPos = HeliStartingPos,
		newHeliPos = HeliStartingPos;

	// The ViewPrj of the previous frame: it's used during the transition 
	glm::mat4 oldViewPrj = glm::mat4(1);


	// si potrebbe usare una sola variabile e cambiarla dentro allo switch, not sure
	// Camera target height and distance for the tank view
	const float tankCamHeight = 2.5f;
	const float tankCamDist = 9.0f;
	// Camera target height and distance for the car view
	const float carCamHeight = 2.5f;
	const float carCamDist = 9.0f;
	// Camera target height and distance for the heli view
	const float heliCamHeight = 3.0f;
	const float heliCamDist = 12.0f;
	
	// Camera Pitch limits for every vehicle
	const float vehicleMinPitch = glm::radians(-8.75f);
	const float vehicleMaxPitch = glm::radians(60.0f);
	const float vehicleStandardPitch = glm::radians(15.0f);
	// Camera Pitch limits for the player
	const float playerMinPitch = glm::radians(-50.0f);
	const float playerMaxPitch = glm::radians(70.0f);

	// Interaction range to ride vehicles
	const float INTERACTION_RANGE = 4.0f;

	// Rotation speed of the camera, rotation speed of the vehicles and their movement speed
	const float CAM_ROT_SPEED = glm::radians(120.0f);

	const float PLAYER_MOVE_SPEED = 20.0f;

	const float TANK_ROT_SPEED = glm::radians(35.0f);
	const float TANK_MOVE_SPEED = 3.0f;

	const float CAR_MOVE_SPEED = 12.0f;
	const float CAR_ROT_SPEED = glm::radians(60.0f);

	const float HELI_MOVE_SPEED = 8.0f; 
	const float HELI_VERT_SPEED = 5.0f;

	// Parameters needed in the damping implementation - 3rd person view
	const float LAMBDAROT = 20.0f,
				LAMBDAMOV = 7.5f,
				DEADZONE = 1.0f,
				LAMBDATRANS = 8.0f;
	
	// queste molto probabilmente andrebbero messe sotto
	// Angles and variables needed to implement damping - independet player rotation from the camera
	float camYaw = 0.0f, camPitch = 0.0f, camRoll = 0.0f,  // il roll sarebbe da togliere
		  camYawOld = 0.0f, camYawNew = 0.0f, 
		  camPitchOld = 0.0f, camPitchNew = 0.0f,
		// tank angles
		  tankYaw = glm::radians(45.0f),
		// heli angles
		  heliYaw = glm::radians(90.0f), heliRoll = 0.0f, heliPitch = 0.0f,
		// car angles
		  carYaw = glm::radians(45.0f);


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
		uniformBlocksInPool = 8; // prima era 9
		texturesInPool = 7;
		setsInPool = 8; // prima era 9

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
		PVertexFloor.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,VK_CULL_MODE_NONE, false);

		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		/* FinalProject */
		/* load the mesh for the room, contained in OBJ file "Room.obj" */
		MTank.init(this, &VMonoColor, "Models/Tank.obj", OBJ);
		MHeliFull.init(this, &VMonoColor, "Models/HeliFull.obj", OBJ);
		MHeliBody.init(this, &VMonoColor, "Models/HeliBody.obj", OBJ);
		MHeliBladeFront.init(this, &VMonoColor, "Models/HeliBladeFront.obj", OBJ);
		MHeliBladeBack.init(this, &VMonoColor, "Models/HeliBladeBack.obj", OBJ);
		MCar.init(this, &VMonoColor, "Models/Tank.obj", OBJ);
		MFloor.vertices = 
			//		POS			   UV
		{     { {-100.0f, 0.2f, 50.0f} , {0,1,0} , {0,1} } ,
			  { { 100.0f, 0.2f, 50.0f} , {0,1,0} , {1,1} } ,
			  { {-100.0f, 0.2f,-50.0f} , {0,1,0} , {0,0} } ,
			  { { 100.0f, 0.2f,-50.0f} , {0,1,0} , {1,0} } };
		MFloor.indices = 
			{ 0, 1, 2, 
			  1, 2, 3 };
		MFloor.initMesh(this, &VVertexFloor);

		// Create the textures
		// The second parameter is the file name
		TTank.init(this, "textures/Red.png");
		THeliFull.init(this, "textures/Red.png");
		THeliBody.init(this, "textures/Red.png");
		THeliBladeFront.init(this, "textures/Red.png");
		THeliBladeBack.init(this, "textures/Red.png");
		TCar.init(this, "textures/Yellow.png");
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

		DSHeliFull.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &THeliFull}
			});
		
		DSHeliBody.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &THeliBody}
			});

		DSHeliBladeFront.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &THeliBladeFront}
			});

		DSHeliBladeBack.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &THeliBladeBack}
			});

		DSCar.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TCar}
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
		DSHeliFull.cleanup();
		DSHeliBody.cleanup();
		DSHeliBladeFront.cleanup();
		DSHeliBladeBack.cleanup();
		DSCar.cleanup();
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
		THeliFull.cleanup();
		THeliBody.cleanup();
		THeliBladeFront.cleanup();
		THeliBladeBack.cleanup();
		TCar.cleanup();
		TFloor.cleanup();

		// Cleanup models
		/* FinalProject */
		/* Cleanup the mesh for the room */
		MTank.cleanup();
		MHeliFull.cleanup();
		MHeliBody.cleanup();
		MHeliBladeFront.cleanup();
		MHeliBladeBack.cleanup();
		MCar.cleanup();
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

		// binds the mesh
		MHeliFull.bind(commandBuffer);
		// binds the descriptor set layout
		DSHeliFull.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MHeliFull.indices.size()), 1, 0, 0, 0);

		// binds the mesh
		MHeliBody.bind(commandBuffer);
		// binds the descriptor set layout
		DSHeliBody.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MHeliBody.indices.size()), 1, 0, 0, 0);

		// binds the mesh
		MHeliBladeFront.bind(commandBuffer);
		// binds the descriptor set layout
		DSHeliBladeFront.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MHeliBladeFront.indices.size()), 1, 0, 0, 0);

		// binds the mesh
		MHeliBladeBack.bind(commandBuffer);
		// binds the descriptor set layout
		DSHeliBladeBack.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MHeliBladeBack.indices.size()), 1, 0, 0, 0);

		// binds the mesh
		MCar.bind(commandBuffer);
		// binds the descriptor set layout
		DSCar.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCar.indices.size()), 1, 0, 0, 0);


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
		uboTank.amb = 1.0f; uboTank.gamma = 180.0f; uboTank.sColor = glm::vec3(1.0f);
		uboTank.mvpMat = ViewPrj * WorldTank;
		uboTank.mMat = WorldTank;
		uboTank.nMat = glm::inverse(glm::transpose(WorldTank));
		/* map the uniform data block to the GPU */
		DSTank.map(currentImage, &uboTank, sizeof(uboTank), 0);

		uboHeliFull.amb = 1.0f; uboHeliFull.gamma = 180.0f; uboHeliFull.sColor = glm::vec3(1.0f);
		uboHeliFull.mvpMat = ViewPrj * WorldHeli;
		uboHeliFull.mMat = WorldHeli;
		uboHeliFull.nMat = glm::inverse(glm::transpose(WorldHeli));
		/* map the uniform data block to the GPU */
		DSHeliFull.map(currentImage, &uboHeliFull, sizeof(uboHeliFull), 0);

		uboCar.amb = 1.0f; uboCar.gamma = 180.0f; uboCar.sColor = glm::vec3(1.0f);
		uboCar.mvpMat = ViewPrj * WorldCar;
		uboCar.mMat = WorldCar;
		uboCar.nMat = glm::inverse(glm::transpose(WorldCar));
		/* map the uniform data block to the GPU */
		DSCar.map(currentImage, &uboCar, sizeof(uboCar), 0);


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
		bool action = false;
		getSixAxis(deltaT, m, r, action);
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

		// TODO capire come gestire gli angoli che superano 2pi CONSIDERANDO il damping!
		// computing camera angles (except pitch) when not transitioning
		if (!transition) {
			camYaw -= CAM_ROT_SPEED * r.y * deltaT;
			camRoll += CAM_ROT_SPEED * r.z * deltaT;
			camPitch -= CAM_ROT_SPEED * r.x * deltaT;
			// TODO forse il pitch si può gestire in modo migliore. Ora updato qua (sempre uguale) e poi limito in base al mezzo
		}
		// damping implementation
		camPitchNew = (camPitchOld * exp(-LAMBDAROT * deltaT)) + camPitch * (1 - exp(-LAMBDAROT * deltaT));
		camPitchOld = camPitchNew;
		camYawNew = (camYawOld * exp(-LAMBDAROT * deltaT)) + camYaw * (1 - exp(-LAMBDAROT * deltaT));
		camYawOld = camYawNew;

		glm::vec3 ux, uy, uz;

		switch (gameState)
		{
		case WALK:

			// computing the movement versors 
			ux = glm::vec3(glm::rotate(glm::mat4(1),
				camYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));

			uz = glm::vec3(glm::rotate(glm::mat4(1),
				camYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

			if (!transition) {
				// updating player position
				playerPosition += ux * PLAYER_MOVE_SPEED * m.x * deltaT;
				playerPosition += uz * PLAYER_MOVE_SPEED * m.z * deltaT;
				// limiting pitch
				camPitch = camPitch < playerMinPitch ? playerMinPitch :
					(camPitch > playerMaxPitch ? playerMaxPitch : camPitch);
			}

			break;

		case TANK:

			// computing the movement versors 
			ux = glm::vec3(glm::rotate(glm::mat4(1),
				tankYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
			uz = glm::vec3(glm::rotate(glm::mat4(1),
				tankYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

			if (!transition) {
				// updating tank position
				tankPosition += ux * TANK_MOVE_SPEED * m.z * deltaT;
				tankPosition += uz * TANK_MOVE_SPEED * m.z * deltaT;
				// limiting pitch
				camPitch = camPitch < vehicleMinPitch ? vehicleMinPitch :
					(camPitch > vehicleMaxPitch ? vehicleMaxPitch : camPitch);
			}
			else {
				SetPitches(&camPitch, &camPitchOld, &camPitchNew, vehicleStandardPitch);
			}

			// deadzone implementation
			if (glm::length((tankPosition - oldTankPos)) > DEADZONE) {
				updatePos = true;
			}
			//TODO newPos e oldPos devono diventare oldTankPos e newOldPos idem per pitchnew e pitchold e yaw...
			if (updatePos) {
				newTankPos = (oldTankPos * exp(-LAMBDAMOV * deltaT)) + tankPosition * (1 - exp(-LAMBDAMOV * deltaT));
				oldTankPos = newTankPos;
				if (glm::length((tankPosition - oldTankPos)) < (DEADZONE / 50.0f)) {
					updatePos = false;
				}
			}

			// tank orientation
			tankYaw -= m.x * TANK_ROT_SPEED * deltaT;

			break;

		case CAR:

			// computing the movement versors 
			ux = glm::vec3(glm::rotate(glm::mat4(1),
				carYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
			uz = glm::vec3(glm::rotate(glm::mat4(1),
				carYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

			if (!transition) {
				// updating car position
				carPosition += ux * CAR_MOVE_SPEED * m.z * deltaT;
				carPosition += uz * CAR_MOVE_SPEED * m.z * deltaT;
				// limiting pitch
				camPitch = camPitch < vehicleMinPitch ? vehicleMinPitch :
					(camPitch > vehicleMaxPitch ? vehicleMaxPitch : camPitch);

				// Rotate only when moving
				if (m.z != 0 && m.x != 0)
					carYaw -= m.z * m.x * CAR_ROT_SPEED * deltaT;
			}
			else {
				SetPitches(&camPitch, &camPitchOld, &camPitchNew, vehicleStandardPitch);
			}

			// deadzone implementation
			if (glm::length((carPosition - oldCarPos)) > DEADZONE) {
				updatePos = true;
			}
			//TODO newPos e oldPos devono diventare oldTankPos e newOldPos idem per pitchnew e pitchold e yaw...
			if (updatePos) {
				newCarPos = (oldCarPos * exp(-LAMBDAMOV * deltaT)) + carPosition * (1 - exp(-LAMBDAMOV * deltaT));
				oldCarPos = newCarPos;
				if (glm::length((carPosition - oldCarPos)) < (DEADZONE / 50.0f)) {
					updatePos = false;
				}
			}


			break;

		case HELI:

			// we use camYaw because the movement of the heli depends on the camera
			ux = glm::vec3(glm::rotate(glm::mat4(1),
				camYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
			uz = glm::vec3(glm::rotate(glm::mat4(1),
				camYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));
			uy = glm::vec3(0, 1, 0);

			if (!transition) {
				// updating heli position and inclination only if it is high enough
				if (heliPosition.y > 2 * HELI_GROUND) {
					heliPosition += ux * HELI_MOVE_SPEED * m.x * deltaT;
					heliPosition += uz * HELI_MOVE_SPEED * m.z * deltaT;

					// updating heli inclination
					heliRoll -= m.z * 0.5f * deltaT;
					heliPitch -= (-m.x) * 0.5f * deltaT;
				}
				
				// blocking the helicopter from going under the terrain
				if ((heliPosition + uy * HELI_VERT_SPEED * m.y * deltaT).y < HELI_GROUND) {
					heliPosition.y = HELI_GROUND;
				}
				else
				{
					heliPosition += uy * HELI_VERT_SPEED * m.y * deltaT;
				}

				// updating pitch
				camPitch = camPitch < vehicleMinPitch ? vehicleMinPitch :
					(camPitch > vehicleMaxPitch ? vehicleMaxPitch : camPitch);
				heliYaw = camYaw + glm::radians(90.0f);

				
				// limiting the inclination
				if (heliRoll < -0.2f)
					heliRoll = -0.2f;
				else if (heliRoll > 0.2f)
					heliRoll = 0.2f;

				if (m.z == 0 && heliRoll < 0.01f) {
					heliRoll -= -0.5f * deltaT;
					if (heliRoll > 0) heliRoll = 0.0f;
				}
				else if (m.z == 0 && heliRoll > 0.01f) {
					heliRoll -= 0.5f * deltaT;
					if (heliRoll < 0) heliRoll = 0.0f;
				}

				if (heliPitch < -0.2f)
					heliPitch = -0.2f;
				else if (heliPitch > 0.2f)
					heliPitch = 0.2f;

				if (m.x == 0 && heliPitch < 0.01f) {
					heliPitch -= -0.5f * deltaT;
					if (heliPitch > 0) heliPitch = 0.0f;
				}
				else if (m.x == 0 && heliPitch > 0.01f) {
					heliPitch -= 0.5f * deltaT;
					if (heliPitch < 0) heliPitch = 0.0f;
				}

			}
			else {
				SetPitches(&camPitch, &camPitchOld, &camPitchNew, vehicleStandardPitch);
			}
			
			// deadzone implementation
			if (glm::length((heliPosition - oldHeliPos)) > DEADZONE) {
				updatePos = true;
			}

			if (updatePos) {
				newHeliPos = (oldHeliPos * exp(-LAMBDAMOV * deltaT)) + heliPosition * (1 - exp(-LAMBDAMOV * deltaT));
				oldHeliPos = newHeliPos;
				if (glm::length((heliPosition - oldHeliPos)) < (DEADZONE / 50.0f)) {
					updatePos = false;
				}
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
			glm::mat4(glm::quat(glm::vec3(0.0f, tankYaw + glm::radians(45.0f), 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1));
		WorldTank = tempWorld;
		
		// Car World Matrix
		tempWorld =
			glm::translate(glm::mat4(1), carPosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw + glm::radians(45.0f), 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1));
		WorldCar = tempWorld;

		// Heli World Matrix
		tempWorld =
			glm::translate(glm::mat4(1), heliPosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, heliYaw, 0.0f))) *
			glm::mat4(glm::quat(glm::vec3(0.0f, 0.0f, heliRoll))) *
			glm::mat4(glm::quat(glm::vec3(heliPitch, 0.0f, 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(1));
		WorldHeli = tempWorld;

		glm::vec4 tempCamPos; //TODO sistemare la posizione 
		glm::vec3 targetPointedPosition;
		switch (gameState)
		{
		case WALK:

			ViewPrj = MakeViewProjectionMatrix(Ar, camYaw, camPitch, camRoll, playerPosition);

			break;
		case TANK:

			// World Matrix used for the camera
			tempWorld = glm::translate(glm::mat4(1), oldTankPos) *
				glm::mat4(glm::quat(glm::vec3(0, camYawNew, 0))) *
				glm::scale(glm::mat4(1), glm::vec3(1));

			// calculating the View-Projection Matrix
			tempCamPos = tempWorld * glm::vec4(0, tankCamHeight + (tankCamDist * sin(camPitchNew)), tankCamDist * cos(camPitchNew), 1);
			camPos = glm::vec3(tempCamPos.x, tempCamPos.y, tempCamPos.z);


			targetPointedPosition = glm::vec3(newTankPos.x, newTankPos.y + tankCamHeight, newTankPos.z);

			ViewMatrix = glm::lookAt(camPos, targetPointedPosition, glm::vec3(0, 1, 0));
			ViewMatrix = glm::rotate(glm::mat4(1), camRoll, glm::vec3(0, 0, 1)) * ViewMatrix;

			ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
			ProjectionMatrix[1][1] *= -1;

			ViewPrj = ProjectionMatrix * ViewMatrix;

			if (possoScendere && !transition && action) {
				transition = true;
				gameState = GameState::WALK;
				camYaw = tankYaw - glm::radians(45.0f);
				camPitch = 0.0f;
				playerPosition = tankPosition;
				playerPosition.y = PLAYER_HEIGHT;
				playerPosition.x += 3 * sin(tankYaw);
				playerPosition.z += 3 * cos(tankYaw);

			}

			break;
		case CAR:

			// World Matrix used for the camera
			tempWorld = glm::translate(glm::mat4(1), oldCarPos) *
				glm::mat4(glm::quat(glm::vec3(0, camYawNew, 0))) *
				glm::scale(glm::mat4(1), glm::vec3(1));

			// calculating the View-Projection Matrix
			tempCamPos = tempWorld * glm::vec4(0, carCamHeight + (carCamDist * sin(camPitchNew)), carCamDist * cos(camPitchNew), 1);
			camPos = glm::vec3(tempCamPos.x, tempCamPos.y, tempCamPos.z);


			targetPointedPosition = glm::vec3(newCarPos.x, newCarPos.y + carCamHeight, newCarPos.z);

			ViewMatrix = glm::lookAt(camPos, targetPointedPosition, glm::vec3(0, 1, 0));
			ViewMatrix = glm::rotate(glm::mat4(1), camRoll, glm::vec3(0, 0, 1)) * ViewMatrix;

			ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
			ProjectionMatrix[1][1] *= -1;

			ViewPrj = ProjectionMatrix * ViewMatrix;

			if (possoScendere && !transition && action) {
				transition = true;
				gameState = GameState::WALK;
				camYaw = carYaw - glm::radians(45.0f);
				camPitch = 0.0f;
				playerPosition = carPosition;
				playerPosition.y = PLAYER_HEIGHT;
				playerPosition.x += 3 * sin(carYaw);
				playerPosition.z += 3 * cos(carYaw);

			}

			break;
		case HELI:

			// World Matrix used for the camera
			tempWorld = glm::translate(glm::mat4(1), oldHeliPos) *
				glm::mat4(glm::quat(glm::vec3(0, camYawNew, 0))) *
				glm::scale(glm::mat4(1), glm::vec3(1));

			// calculating the View-Projection Matrix
			tempCamPos = tempWorld * glm::vec4(0, heliCamHeight + (heliCamDist * sin(camPitchNew)), heliCamDist * cos(camPitchNew), 1);
			camPos = glm::vec3(tempCamPos.x, tempCamPos.y, tempCamPos.z);


			targetPointedPosition = glm::vec3(newHeliPos.x, newHeliPos.y + heliCamHeight, newHeliPos.z);

			ViewMatrix = glm::lookAt(camPos, targetPointedPosition, glm::vec3(0, 1, 0));
			ViewMatrix = glm::rotate(glm::mat4(1), camRoll, glm::vec3(0, 0, 1)) * ViewMatrix;

			ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
			ProjectionMatrix[1][1] *= -1;

			ViewPrj = ProjectionMatrix * ViewMatrix;

			if (possoScendere && !transition && action) {
				transition = true;	
				gameState = GameState::WALK;
				camYaw = heliYaw - glm::radians(90.0f);
				camPitch = 0.0f;
				playerPosition = heliPosition;
				playerPosition.y = PLAYER_HEIGHT;
				playerPosition.x += 3 * sin(heliYaw);
				playerPosition.z += 3 * cos(heliYaw);

			}

			break;
		default:
			printf("\n\nSOMETHING'S WRONG I CAN FEEL IT\n");
			break;
		}

		// si potrebbe parametrizzare
		if (transition) {
			transitionTimer += deltaT;
			ViewPrj = (oldViewPrj * exp(-LAMBDATRANS * deltaT)) + ViewPrj * (1 - exp(-LAMBDATRANS * deltaT));
			if (transitionTimer > TRANS_DURATION) {
				transition = false;
				transitionTimer = 0.0f;
			}
		}
		oldViewPrj = ViewPrj;

		// If i'm not driving a vehicle, check for the closest one
		if (gameState == GameState::WALK) {
			GameState tempClosest = ClosestObject();
			glm::vec3 posClosest = glm::vec3(0);
			switch (tempClosest)
			{
			case WALK:
				printf("\nCASINI FRA\n");
				break;
			case TANK:
				posClosest = tankPosition;
				break;
			case CAR:
				posClosest = carPosition;
				break;
			case HELI:
				posClosest = heliPosition;
				break;
			default:
				printf("\nC A S I N I   F R A\n");
				break;
			}
			closestDistance = glm::length((playerPosition - posClosest));

			// If i'm not transitioning and the closest vehicle is at a certain range, display the prompt to use it
			if (!transition && closestDistance < INTERACTION_RANGE) {
				// display prompt to drive a vehicle

				if (action) {
					possoScendere = false;
					transition = true;
					float angle = 0.0f;
					switch (tempClosest)
					{
					case WALK:
						printf("\nCASINI FRA\n");
						break;
					case TANK:
						gameState = GameState::TANK;
						angle = tankYaw - glm::radians(45.0f);
						break;
					case CAR:
						gameState = GameState::CAR;
						angle = carYaw - glm::radians(45.0f);
						break;
					case HELI:
						gameState = GameState::HELI;
						angle = heliYaw - glm::radians(90.0f);
						break;
					default:
						printf("\nC A S I N I   F R A\n");
						break;
					}
					camYaw = angle;
				}
			}

		} 
		
		// possoScendere tracks the state of the fire button, to ignore new presses if the button is held down
		if (!action)
			possoScendere = true;

	}




	// Return the closest object to the player
	GameState ClosestObject() {
		return  glm::length((playerPosition - tankPosition)) < glm::length((playerPosition - carPosition)) ?
				(glm::length((playerPosition - tankPosition)) < glm::length((playerPosition - heliPosition)) ? GameState::TANK : GameState::HELI) :
				(glm::length((playerPosition - carPosition)) < glm::length((playerPosition - heliPosition)) ? GameState::CAR : GameState::HELI);
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

	// Sets the three floats to the value of the last one
	void SetPitches(float* pa, float* pb, float* pc, float value) {
		*pa = value;
		*pb = value;
		*pc = value;
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