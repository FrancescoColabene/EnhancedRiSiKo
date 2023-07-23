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
	alignas(16) glm::vec3 color;
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

// Gamestate enum structure - it starts from 0
enum GameState {
	WALK,
	TANK,
	CAR,
	HELI
};

// Vertex format used by vehicles
struct VertexMonoColor {
	glm::vec3 pos;
	glm::vec3 norm;
};

/* FinalProject */
/* Add the C++ datastructure for the required vertex format */
struct VertexFloor {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

#define M_PI 3.141595f

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
	DescriptorSetLayout DSLVertexMonument;

	// Vertex formats

	/* FinalProject */
	/* Add the variable that will contain the required Vertex format definition */
	VertexDescriptor VMonoColor;
	VertexDescriptor VVertexFloor;
	VertexDescriptor VVertexMonument;

	// Pipelines [Shader couples]
	/* FinalProject */
	/* Add the variable that will contain the new pipeline */
	Pipeline PMonoColor;
	Pipeline PVertexFloor;
	Pipeline PPagoda;
	Pipeline PEiffel;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	/* FinalProject */
	/* Add the variable that will contain the model for the room */
	Model<VertexMonoColor> MTank, MCar, MCarSingleTire, MHeliFull, MHeliBody, MHeliTopBlade, MHeliBackBlade;
	Model<VertexFloor> MFloor;
	Model<VertexMonoColor> MPagoda;
	Model<VertexMonoColor> MEiffel;


	DescriptorSet DSGubo;
	/* FinalProject */
	/* Add the variable that will contain the Descriptor Set for the room */
	DescriptorSet DSTank, DSCar, DSCarBackLeftTire, DSCarBackRightTire, DSCarLeftTire, DSCarRightTire, DSHeliFull, DSHeliBody, DSHeliTopBlade, DSHeliBackBlade, DSPagoda, DSEiffel, DSFloor;


	Texture TTank, TCar, TCarSingleTire, THeliFull, THeliBody, THeliTopBlade, TFloor;

	// C++ storage for uniform variables
	/* FinalProject */
	/* Add the variable that will contain the Uniform Block in slot 0, set 1 of the room */
	MeshUniformBlock uboTank, uboCar, uboHeliBody, uboHeliTopBlade, uboBackLeftTire, uboBackRightTire, uboLeftTire, uboRightTire, uboHeliFull, uboGlass, uboHeliBackBlades;
	MeshUniformBlock uboPagoda, uboEiffel;
	MeshUniformBlock uboFloor;

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

	// TODO fare un minimo di ordine nelle dichiarazioni delle costanti

	// si potrebbe togliere il const e permettere di cambiare fov al player quando si trova in prima persona - magari con un'altra enum
	const float FOVy = glm::radians(70.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 150.0f;

	// Player starting point + initialization
	const float PLAYER_HEIGHT = 1.7f;
	const float HELI_GROUND = 1.68f; // 1.68f è l'altezza giusta dal pavimento
	const float CAR_HEIGHT = 0.7f;

	const glm::vec3 PlayerStartingPos = glm::vec3(0.0f, PLAYER_HEIGHT, 0.0f);
	const glm::vec3 TankStartingPos = glm::vec3(-7.0f, 0.0f, -12.0f);
	const glm::vec3 CarStartingPos = glm::vec3(0.0f, 1.05f, -12.0f);
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
	const float carCamHeight = 2.0f;
	const float carCamDist = 6.0f;
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

	// Gravity force for player/heli
	const float GRAVITY = 6.0f;

	// Rotation speed of the camera, rotation speed of the vehicles and their movement speed
	const float CAM_ROT_SPEED = glm::radians(120.0f);

	const float PLAYER_ACC_SPEED = 100.0f; // forse molto alta sarebbe ok
	const float PLAYER_MOVE_SPEED = 10.0f;

	const float TANK_ACC_SPEED = 3.0f;
	const float TANK_DEC_SPEED = 5.0f;
	const float TANK_ROT_SPEED = glm::radians(35.0f);
	const float TANK_MAX_SPEED = 3.0f;
	float tankMoveSpeed = 0.0f; // I don't need a vector because the tank can cannot move sideways

	const float CAR_ACC_SPEED = 8.0f;
	const float CAR_DEC_SPEED = 7.0f;
	const float CAR_MAX_SPEED = 15.0f;
	const float CAR_ROT_SPEED = glm::radians(20.0f);
	const float CAR_SCALE = 0.775f;
	float carMoveSpeed = 0.0f; // I don't need a vector because the car cannot move sideways
	const glm::vec3 RIGHT_TIRE_OFFSET = glm::vec3(0.8f, -0.425f, -1.025f);
	const glm::vec3 LEFT_TIRE_OFFSET = glm::vec3(-0.8f, -0.425f, -1.025f);
	const glm::vec3 BACK_RIGHT_TIRE_OFFSET = glm::vec3(0.8f, -0.425f, 1.15f);
	const glm::vec3 BACK_LEFT_TIRE_OFFSET = glm::vec3(-0.8f, -0.425f, 1.15f);



	const float HELI_ACC_SPEED = 15.0f;
	const float HELI_DEC_SPEED = 10.0f;
	const float HELI_MAX_SPEED = 10.0f;
	const float HELI_VERT_SPEED = 5.0f;
	const float HELI_DAMP_ANGLE = glm::radians(10.0f);
	const float HELI_DAMP_SPEED = 1.2f;
	const float HELI_BLADE_SPEED = -10.0f;
	glm::vec3 heliMoveSpeed = glm::vec3(0.0f);
	float tempRotation = HELI_BLADE_SPEED;
	const glm::vec3 TOP_BLADE_OFFSET = glm::vec3(0.0f, 1.25f, 0.0f);
	const glm::vec3 BACK_BLADE_OFFSET = glm::vec3(0.175f, 1.075f, 5.425f);


	// Parameters needed in the damping implementation - 3rd person view
	const float LAMBDAROT = 20.0f,
		LAMBDAMOV = 5.0f,
		DEADZONE = 1.0f,
		LAMBDATRANS = 8.0f,
		LAMBDAANGLE = 10.0f;

	// queste molto probabilmente andrebbero messe sotto
	// Angles and variables needed to implement damping - independet player rotation from the camera
	float camYaw = 0.0f, camPitch = 0.0f, camRoll = 0.0f,  // il roll sarebbe da togliere
		camYawOld = 0.0f, camYawNew = 0.0f,
		camPitchOld = 0.0f, camPitchNew = 0.0f,
		// tank angles
		tankYaw = glm::radians(45.0f),
		// heli angles
		heliYaw = glm::radians(90.0f), heliRoll = 0.0f, heliPitch = 0.0f,
		tempHeliYaw = glm::radians(90.0f), heliTopBladeYaw = 0.0f, heliBackBladeRoll = 0.0f,
		// car angles
		carYaw = glm::radians(45.0f);


	// ------------------------------------------------------------------------------------


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1400;
		windowHeight = 900;
		windowTitle = "Enhanced RiSiKo!";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.50f, 1.0f, 1.0f, 1.0f };

		// Descriptor pool sizes
		/* FinalProject */
		/* Update the requirements for the size of the pool */
		uniformBlocksInPool = 14;
		texturesInPool = 11;
		setsInPool = 14;

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
		// BINDING 0: 
		DSLVertexFloor.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLMonoColor.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		DSLGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		DSLVertexMonument.init(this, {
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
					   sizeof(glm::vec3), NORMAL}
			});

		VVertexMonument.init(this, { 
			{0, sizeof(VertexMonoColor), VK_VERTEX_INPUT_RATE_VERTEX }
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMonoColor, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMonoColor, norm),
					   sizeof(glm::vec3), NORMAL}
			});

		VVertexFloor.init(this, {
			{0, sizeof(VertexFloor), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexFloor, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexFloor, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexFloor, UV),
					   sizeof(glm::vec2), UV}
			});

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		/* FinalProject */
		/* Create the new pipeline, using shaders "VColorVert.spv" and "VColorFrag.spv" */

		// SET 0: DSLGubo, SET 1: DSLMonoColor
		PMonoColor.init(this, &VMonoColor, "shaders/Pieces/MonoColorVert.spv", "shaders/Pieces/MonoColorFrag.spv", { &DSLGubo, &DSLMonoColor });
		PVertexFloor.init(this, &VVertexFloor, "shaders/Floor/FloorVert.spv", "shaders/Floor/FloorFrag.spv", { &DSLGubo, &DSLVertexFloor });
		PVertexFloor.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
		PPagoda.init(this, &VVertexMonument, "shaders/Monuments/PagodaVert.spv", "shaders/Monuments/PagodaFrag.spv", { &DSLGubo, &DSLVertexMonument });
		PEiffel.init(this, &VVertexMonument, "shaders/Monuments/EiffelVert.spv", "shaders/Monuments/EiffelFrag.spv", { &DSLGubo, &DSLVertexMonument });

		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		/* FinalProject */
		/* load the mesh for the room, contained in OBJ file "Room.obj" */
		MTank.init(this, &VMonoColor, "Models/Tank.obj", OBJ);
		MCar.init(this, &VMonoColor, "Models/Jeep.obj", OBJ);
		MCarSingleTire.init(this, &VMonoColor, "Models/JeepTire.obj", OBJ);
		MHeliFull.init(this, &VMonoColor, "Models/HeliFull.obj", OBJ);
		MHeliBody.init(this, &VMonoColor, "Models/HeliBody.obj", OBJ);
		MHeliTopBlade.init(this, &VMonoColor, "Models/HeliTopBlade.obj", OBJ);
		MHeliBackBlade.init(this, &VMonoColor, "Models/HeliBackBlade.obj", OBJ);
		MPagoda.init(this, &VVertexMonument, "Models/Pagoda.obj", OBJ);
		MEiffel.init(this, &VVertexMonument, "Models/Eiffel.obj", OBJ);
		MFloor.vertices =
			//			 POS			  NORM		UV
		{	  { {-200.0f, 0.2f, 100.0f} , {0,1,0} , {0,1} } ,
			  { { 200.0f, 0.2f, 100.0f} , {0,1,0} , {1,1} } ,
			  { {-200.0f, 0.2f,-100.0f} , {0,1,0} , {0,0} } ,
			  { { 200.0f, 0.2f,-100.0f} , {0,1,0} , {1,0} } };
		MFloor.indices =
		{ 0, 1, 2,
		  1, 2, 3 };
		MFloor.initMesh(this, &VVertexFloor);

		// Create the textures
		// The second parameter is the file name
		TTank.init(this, "textures/Red.png");
		TCar.init(this, "textures/Red.png");
		TCarSingleTire.init(this, "textures/Red.png");
		THeliFull.init(this, "textures/Red.png");
		THeliBody.init(this, "textures/Red.png");
		THeliTopBlade.init(this, "textures/Red.png");

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
		PPagoda.create();
		PEiffel.create();
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
			});

		DSCar.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSCarBackLeftTire.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSCarBackRightTire.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSCarLeftTire.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSCarRightTire.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});

		DSHeliFull.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSHeliBody.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSHeliTopBlade.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSHeliBackBlade.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});

		DSPagoda.init(this, &DSLVertexMonument, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSEiffel.init(this, &DSLVertexMonument, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});

		DSFloor.init(this, &DSLVertexFloor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TFloor}
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
		PPagoda.cleanup();
		PEiffel.cleanup();
		// Cleanup datasets
		/* FinalProject */
		/* cleanup the dataset for the room */
		DSTank.cleanup();
		DSCar.cleanup();
		DSCarBackLeftTire.cleanup();
		DSCarBackRightTire.cleanup();
		DSCarRightTire.cleanup();
		DSCarLeftTire.cleanup();
		DSHeliFull.cleanup();
		DSHeliBody.cleanup();
		DSHeliTopBlade.cleanup();
		DSHeliBackBlade.cleanup();
		DSPagoda.cleanup();
		DSEiffel.cleanup();
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
		TCar.cleanup();
		TCarSingleTire.cleanup();
		THeliFull.cleanup();
		THeliBody.cleanup();
		THeliTopBlade.cleanup();
		TFloor.cleanup();

		// Cleanup models
		/* FinalProject */
		/* Cleanup the mesh for the room */
		MTank.cleanup();
		MCar.cleanup();
		MCarSingleTire.cleanup();
		MHeliFull.cleanup();
		MHeliBody.cleanup();
		MHeliTopBlade.cleanup();
		MHeliBackBlade.cleanup();
		MPagoda.cleanup();
		MEiffel.cleanup();

		MFloor.cleanup();
		// Cleanup descriptor set layouts
		/* FinalProject */
		/* Cleanup the new Descriptor Set Layout */
		DSLMonoColor.cleanup();
		DSLVertexFloor.cleanup();
		DSLVertexMonument.cleanup();
		DSLGubo.cleanup();

		/* FinalProject */
		/* Destroy the new pipeline */
		PMonoColor.destroy();
		PVertexFloor.destroy();
		PPagoda.destroy();
		PEiffel.destroy();
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
		MCar.bind(commandBuffer);
		// binds the descriptor set layout
		DSCar.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCar.indices.size()), 1, 0, 0, 0);



		// binds the mesh
		MCarSingleTire.bind(commandBuffer);
		// binds the descriptor set layout
		DSCarBackLeftTire.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCarSingleTire.indices.size()), 1, 0, 0, 0);

		DSCarBackRightTire.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCarSingleTire.indices.size()), 1, 0, 0, 0);

		DSCarLeftTire.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCarSingleTire.indices.size()), 1, 0, 0, 0);
		DSCarRightTire.bind(commandBuffer, PMonoColor, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCarSingleTire.indices.size()), 1, 0, 0, 0);


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
		MHeliTopBlade.bind(commandBuffer);
		// binds the descriptor set layout
		DSHeliTopBlade.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MHeliTopBlade.indices.size()), 1, 0, 0, 0);
		// binds the mesh
		MHeliBackBlade.bind(commandBuffer);
		// binds the descriptor set layout
		DSHeliBackBlade.bind(commandBuffer, PMonoColor, 1, currentImage);
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MHeliBackBlade.indices.size()), 1, 0, 0, 0);

		// binds the pipeline
		PPagoda.bind(commandBuffer);

		// binds the descriptor set layout (gubo)
		DSGubo.bind(commandBuffer, PPagoda, 0, currentImage);
		// binds the mesh
		MPagoda.bind(commandBuffer);
		// binds the descriptor set layout
		DSPagoda.bind(commandBuffer, PPagoda, 1, currentImage);
		// record the drawing command in the commnad buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPagoda.indices.size()), 1, 0, 0, 0);

		// binds the pipeline
		PEiffel.bind(commandBuffer);

		// binds the descriptor set layout (gubo)
		DSGubo.bind(commandBuffer, PEiffel, 0, currentImage);
		// binds the mesh
		MEiffel.bind(commandBuffer);
		// binds the descriptor set layout
		DSEiffel.bind(commandBuffer, PEiffel, 1, currentImage);
		// record the drawing command in the commnad buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MEiffel.indices.size()), 1, 0, 0, 0);

		// binds the pipeline
		PVertexFloor.bind(commandBuffer);
		// binds the mesh
		MFloor.bind(commandBuffer);
		// binds the descriptor set layout
		DSGubo.bind(commandBuffer, PVertexFloor, 0, currentImage);
		DSFloor.bind(commandBuffer, PVertexFloor, 1, currentImage);
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
		glm::mat4 WorldPlayer, WorldTank, WorldCar, WorldBackLeftTire, WorldBackRightTire, WorldLeftTire, WorldRightTire, WorldHeli, WorldHeliTopBlade, WorldHeliBackBlade;

		glm::vec3 ROJO = glm::vec3(0.65f, 0.01f, 0.01f);
		glm::vec3 GRIJO = glm::vec3(0.75f);

		// Function that contains all the logic of the game
		Logic(Ar, camPos, ViewPrj, 
			WorldPlayer, WorldTank, 
			WorldCar, WorldBackLeftTire, WorldBackRightTire, WorldLeftTire, WorldRightTire, 
			WorldHeli, WorldHeliTopBlade, WorldHeliBackBlade);

		// gubo values
		gubo.DlightDir = glm::normalize(glm::vec3(0.0f, 1.0f, 4.0f));
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
		uboTank.amb = 1.0f; uboTank.gamma = 180.0f; uboTank.color = ROJO; uboTank.sColor = glm::vec3(1.0f);
		uboTank.mvpMat = ViewPrj * WorldTank;
		uboTank.mMat = WorldTank;
		uboTank.nMat = glm::inverse(glm::transpose(WorldTank));
		/* map the uniform data block to the GPU */
		DSTank.map(currentImage, &uboTank, sizeof(uboTank), 0);

		uboCar.amb = 1.0f; uboCar.gamma = 180.0f; uboCar.color = ROJO; uboCar.sColor = glm::vec3(1.0f);
		uboCar.mvpMat = ViewPrj * WorldCar;
		uboCar.mMat = WorldCar;
		uboCar.nMat = glm::inverse(glm::transpose(WorldCar));
		/* map the uniform data block to the GPU */
		DSCar.map(currentImage, &uboCar, sizeof(uboCar), 0);


		uboBackLeftTire.amb = 1.0f; uboBackLeftTire.gamma = 180.0f; uboBackLeftTire.color = ROJO; uboBackLeftTire.sColor = glm::vec3(1.0f);
		uboBackLeftTire.mvpMat = ViewPrj * WorldBackLeftTire;
		uboBackLeftTire.mMat = WorldBackLeftTire;
		uboBackLeftTire.nMat = glm::inverse(glm::transpose(WorldBackLeftTire));
		/* map the uniform data block to the GPU */
		DSCarBackLeftTire.map(currentImage, &uboBackLeftTire, sizeof(uboBackLeftTire), 0);

		uboBackRightTire.amb = 1.0f; uboBackRightTire.gamma = 180.0f; uboBackRightTire.color = ROJO; uboBackRightTire.sColor = glm::vec3(1.0f);
		uboBackRightTire.mvpMat = ViewPrj * WorldBackRightTire;
		uboBackRightTire.mMat = WorldBackRightTire;
		uboBackRightTire.nMat = glm::inverse(glm::transpose(WorldBackRightTire));
		/* map the uniform data block to the GPU */
		DSCarBackRightTire.map(currentImage, &uboBackRightTire, sizeof(uboBackRightTire), 0);


		uboLeftTire.amb = 1.0f; uboLeftTire.gamma = 180.0f; uboLeftTire.color = ROJO; uboLeftTire.sColor = glm::vec3(1.0f);
		uboLeftTire.mvpMat = ViewPrj * WorldLeftTire;
		uboLeftTire.mMat = WorldLeftTire;
		uboLeftTire.nMat = glm::inverse(glm::transpose(WorldLeftTire));
		/* map the uniform data block to the GPU */
		DSCarLeftTire.map(currentImage, &uboLeftTire, sizeof(uboLeftTire), 0);

		uboRightTire.amb = 1.0f; uboRightTire.gamma = 180.0f; uboRightTire.color = ROJO; uboRightTire.sColor = glm::vec3(1.0f);
		uboRightTire.mvpMat = ViewPrj * WorldRightTire;
		uboRightTire.mMat = WorldRightTire;
		uboRightTire.nMat = glm::inverse(glm::transpose(WorldRightTire));
		/* map the uniform data block to the GPU */
		DSCarRightTire.map(currentImage, &uboRightTire, sizeof(uboRightTire), 0);

		uboHeliBody.amb = 1.0f; uboHeliBody.gamma = 500.0f; uboHeliBody.color = ROJO; uboHeliBody.sColor = glm::vec3(1.0f);
		uboHeliBody.mvpMat = ViewPrj * WorldHeli;
		uboHeliBody.mMat = WorldHeli;
		uboHeliBody.nMat = glm::inverse(glm::transpose(WorldHeli));
		/* map the uniform data block to the GPU */
		DSHeliBody.map(currentImage, &uboHeliBody, sizeof(uboHeliBody), 0);

		uboHeliTopBlade.amb = 1.0f; uboHeliTopBlade.gamma = 180.0f; uboHeliTopBlade.color = ROJO; uboHeliTopBlade.sColor = glm::vec3(1.0f);
		uboHeliTopBlade.mvpMat = ViewPrj * WorldHeliTopBlade;
		uboHeliTopBlade.mMat = WorldHeliTopBlade;
		uboHeliTopBlade.nMat = glm::inverse(glm::transpose(WorldHeliTopBlade));
		/* map the uniform data block to the GPU */
		DSHeliTopBlade.map(currentImage, &uboHeliTopBlade, sizeof(uboHeliTopBlade), 0);

		uboHeliBackBlades.amb = 1.0f; uboHeliBackBlades.gamma = 180.0f; uboHeliBackBlades.color = ROJO; uboHeliBackBlades.sColor = glm::vec3(1.0f);
		uboHeliBackBlades.mvpMat = ViewPrj * WorldHeliBackBlade;
		uboHeliBackBlades.mMat = WorldHeliBackBlade;
		uboHeliBackBlades.nMat = glm::inverse(glm::transpose(WorldHeliBackBlade));
		/* map the uniform data block to the GPU */
		DSHeliBackBlade.map(currentImage, &uboHeliBackBlades, sizeof(uboHeliBackBlades), 0);

		glm::mat4 WorldPagoda = 
			glm::translate(glm::mat4(1), glm::vec3(160.0f, 0.0f, -40.0f)) *
			glm::mat4(glm::quat(glm::vec3(0.0f, -glm::radians(30.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.4f));;

		uboPagoda.amb = 1.0f; uboPagoda.gamma = 180.0f; uboPagoda.color = GRIJO; uboPagoda.sColor = glm::vec3(1.0f);
		uboPagoda.mvpMat = ViewPrj * WorldPagoda;
		uboPagoda.mMat = WorldPagoda;
		uboPagoda.nMat = glm::inverse(glm::transpose(WorldPagoda));
		DSPagoda.map(currentImage, &uboPagoda, sizeof(uboPagoda), 0);

		glm::mat4 WorldEiffel =
			glm::translate(glm::mat4(1), glm::vec3(-0.5f, 0.05f, -27.5f)) *
			glm::mat4(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.4f));;

		uboEiffel.amb = 1.0f; uboEiffel.gamma = 180.0f; uboEiffel.color = GRIJO; uboEiffel.sColor = glm::vec3(1.0f);
		uboEiffel.mvpMat = ViewPrj * WorldEiffel;
		uboEiffel.mMat = WorldEiffel;
		uboEiffel.nMat = glm::inverse(glm::transpose(WorldEiffel));
		DSEiffel.map(currentImage, &uboEiffel, sizeof(uboEiffel), 0);


		glm::mat4 WorldFloor = glm::mat4(1);
		uboFloor.amb = 1.0f; uboFloor.gamma = 180.0f; uboFloor.color = ROJO; uboFloor.sColor = glm::vec3(1.0f);
		uboFloor.mMat = WorldFloor;
		uboFloor.mvpMat = ViewPrj * WorldFloor;
		uboFloor.nMat = glm::inverse(glm::transpose(WorldFloor));
		DSFloor.map(currentImage, &uboFloor, sizeof(uboFloor), 0);

	}

	//Logic(Ar, camPos, ViewPrj, WorldPlayer, WorldTank, WorldCar, WorldLeftTire, WorldRightTire, WorldGlass, WorldHeli, WorldHeliTopBlade, WorldHeliBackBlade);


	void Logic(float Ar, glm::vec3& camPos,
		glm::mat4& ViewPrj,
		glm::mat4& WorldPlayer,
		glm::mat4& WorldTank,
		glm::mat4& WorldCar, glm::mat4& WorldBackLeftTire, glm::mat4& WorldBackRightTire, glm::mat4& WorldLeftTire, glm::mat4& WorldRightTire, 
		glm::mat4& WorldHeli, glm::mat4& WorldHeliTopBlade, glm::mat4& WorldHeliBackBlade) {

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
		static float tireRotation = 0.0f;
		static float tireAngle = 0.0f;
		float temp = 0.0f;
		// for debugging
		// deltaT = 0.01f;

		// LOGIC OF THE APPLICATION
		
		// computing camera angles (except pitch) when not transitioning
		if (!transition) {

			camYaw -= CAM_ROT_SPEED * r.y * deltaT;
			//camRoll += CAM_ROT_SPEED * r.z * deltaT;
			camPitch -= CAM_ROT_SPEED * r.x * deltaT;

			// Limiting the yaw to a value between 2 * M_PI and -2 * M_PI
			if (camYaw > 2 * M_PI) {
				camYaw -= 2 * M_PI;
				camYawOld -= 2 * M_PI;
				// if we're on the helicopter, we have to update its yaw too, otherwise it will spin
				if (gameState == GameState::HELI && heliPosition.y > 2 * HELI_GROUND) {
					heliYaw -= 2 * M_PI;
					tempHeliYaw -= 2 * M_PI;
				}
			}
			if (camYaw < -2 * M_PI) {
				camYaw += 2 * M_PI;
				camYawOld += 2 * M_PI;
				// if we're on the helicopter, we have to update its yaw too, otherwise it will spin
				if (gameState == GameState::HELI && heliPosition.y > 2 * HELI_GROUND) {
					heliYaw += 2 * M_PI;
					tempHeliYaw += 2 * M_PI;
				}
			}
			// TODO forse il pitch si può gestire in modo migliore. Ora updato qua (sempre uguale) e poi limito in base al mezzo
		}
		// damping implementation
		camPitchNew = (camPitchOld * exp(-LAMBDAROT * deltaT)) + camPitch * (1 - exp(-LAMBDAROT * deltaT));
		camPitchOld = camPitchNew;
		camYawNew = (camYawOld * exp(-LAMBDAROT * deltaT)) + camYaw * (1 - exp(-LAMBDAROT * deltaT));
		camYawOld = camYawNew;

		glm::vec3 ux, uy, uz;
		// used by heli
		static bool damp = false;
		// used for the player's gravity
		static float playerVerticalVelocity = 0.0f;

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

			// make the player fall if it's higher than the ground
			if (playerPosition.y > PLAYER_HEIGHT) {
				playerVerticalVelocity += GRAVITY * deltaT;
				playerPosition.y -= playerVerticalVelocity * deltaT;
				if (playerPosition.y < PLAYER_HEIGHT) {
					playerPosition.y = PLAYER_HEIGHT;
					playerVerticalVelocity = 0.0f;
				}
			}

			break;

		case TANK:

			// computing the movement versors for the tank
			ux = glm::vec3(glm::rotate(glm::mat4(1),
				tankYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
			uz = glm::vec3(glm::rotate(glm::mat4(1),
				tankYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

			if (!transition) {
				// If i'm not pressing any button, I should lose velocity till I stop moving
				if (m.z == 0) {
					if (tankMoveSpeed < 0.0f) {
						tankMoveSpeed += TANK_DEC_SPEED * deltaT;
						if (tankMoveSpeed > 0.0f) tankMoveSpeed = 0.0f;
					}
					else {
						tankMoveSpeed -= TANK_DEC_SPEED * deltaT;
						if (tankMoveSpeed < 0.0f) tankMoveSpeed = 0.0f;
					}
				}

				// Updating tank's velocity 
				tankMoveSpeed += m.z * TANK_ACC_SPEED * deltaT;

				// Limiting the maximum movement speed 
				if (abs(tankMoveSpeed) > TANK_MAX_SPEED) {
					tankMoveSpeed = TANK_MAX_SPEED * glm::sign(tankMoveSpeed);
				}

				// updating tank position
				tankPosition += ux * tankMoveSpeed * deltaT;
				tankPosition += uz * tankMoveSpeed * deltaT;

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

			// computing the movement versors for the car
			ux = glm::vec3(glm::rotate(glm::mat4(1),
				carYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
			uz = glm::vec3(glm::rotate(glm::mat4(1),
				carYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

			if (!transition) {
				// If i'm not pressing any button, I should lose velocity till I stop moving
				if (m.z == 0) {
					if (carMoveSpeed < 0.0f) {
						carMoveSpeed += CAR_DEC_SPEED * deltaT;
						if (carMoveSpeed > 0.0f) carMoveSpeed = 0.0f;
					}
					else {
						carMoveSpeed -= CAR_DEC_SPEED * deltaT;
						if (carMoveSpeed < 0.0f) carMoveSpeed = 0.0f;
					}
				}

				// Updating tank's velocity 
				carMoveSpeed += m.z * CAR_ACC_SPEED * deltaT;

				// Limiting the maximum movement speed 
				if (abs(carMoveSpeed) > CAR_MAX_SPEED) {
					carMoveSpeed = CAR_MAX_SPEED * glm::sign(carMoveSpeed);
				}

				// updating car position
				carPosition += ux * carMoveSpeed * deltaT;
				carPosition += uz * carMoveSpeed * deltaT;

				// limiting pitch
				camPitch = camPitch < vehicleMinPitch ? vehicleMinPitch :
					(camPitch > vehicleMaxPitch ? vehicleMaxPitch : camPitch);

				// Rotate only when moving
				if (carMoveSpeed != 0 && m.x != 0)
					carYaw -= m.x * carMoveSpeed * CAR_ROT_SPEED * deltaT;
			}
			else {
				SetPitches(&camPitch, &camPitchOld, &camPitchNew, vehicleStandardPitch);
			}

			// deadzone implementation
			if (glm::length((carPosition - oldCarPos)) > DEADZONE) {
				updatePos = true;
			}
			if (updatePos) {
				newCarPos = (oldCarPos * exp(-LAMBDAMOV * deltaT)) + carPosition * (1 - exp(-LAMBDAMOV * deltaT));
				oldCarPos = newCarPos;
				if (glm::length((carPosition - oldCarPos)) < (DEADZONE / 50.0f)) {
					updatePos = false;
				}
			}
			temp = m.x * glm::radians(40.0f); // da definire
			tireAngle = (tireAngle * exp(-LAMBDAANGLE * deltaT)) + temp * (1 - exp(-LAMBDAANGLE * deltaT));

			tireRotation -= carMoveSpeed * glm::radians(0.05f); // todo da definire
			if (tireRotation < 2 * M_PI) tireRotation += 2 * M_PI;

			break;

		case HELI:

			// computing the movement versors for the heli
			// we use camYaw because the movement of the heli depends on the camera
			ux = glm::vec3(glm::rotate(glm::mat4(1),
				camYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
			uz = glm::vec3(glm::rotate(glm::mat4(1),
				camYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));
			uy = glm::vec3(0, 1, 0);

			// Update the heli's position only if the camera is not transitioning
			if (!transition) {
				// updating heli position and inclination only if it is high enough
				if (heliPosition.y >= 2 * HELI_GROUND) {

					// If i'm not pressing any button, I should lose velocity till I stop moving
					if (m.x == 0) {
						float angle = atan2(heliMoveSpeed.x, heliMoveSpeed.z);
						if (heliMoveSpeed.x < 0) {
							heliMoveSpeed.x += abs(sin(angle)) * HELI_DEC_SPEED * deltaT;
							if (heliMoveSpeed.x > 0) heliMoveSpeed.x = 0;
						}
						else {
							heliMoveSpeed.x -= abs(sin(angle)) * HELI_DEC_SPEED * deltaT;
							if (heliMoveSpeed.x < 0) heliMoveSpeed.x = 0;
						}
					}
					if (m.z == 0) {
						float angle = atan2(heliMoveSpeed.x, heliMoveSpeed.z);
						if (heliMoveSpeed.z < 0) {
							heliMoveSpeed.z += abs(cos(angle)) * HELI_DEC_SPEED * deltaT;
							if (heliMoveSpeed.z > 0) heliMoveSpeed.z = 0;
						}
						else {
							heliMoveSpeed.z -= abs(cos(angle)) * HELI_DEC_SPEED * deltaT;
							if (heliMoveSpeed.z < 0) heliMoveSpeed.z = 0;
						}
					}
					// update the heli's movespeed based on inputs
					heliMoveSpeed += glm::vec3(m.x * HELI_ACC_SPEED * deltaT, 0.0f, m.z * HELI_ACC_SPEED * deltaT);

					// If the player is moving, reset the position to center
					if (m.z != 0 || m.x != 0)
						damp = true;

					// Maximum velocity and diagonal velocity
					if (glm::length(heliMoveSpeed) > HELI_MAX_SPEED) {
						heliMoveSpeed = HELI_MAX_SPEED * glm::normalize(heliMoveSpeed);
					}

					heliPosition += ux * heliMoveSpeed.x * deltaT;
					heliPosition += uz * heliMoveSpeed.z * deltaT;

					// updating heli inclination
					heliRoll -= m.z * 0.5f * deltaT;
					heliPitch -= (-m.x) * 0.5f * deltaT;

					// updating heli rotation - deadzone implementation for the yaw 
					// SE MI GIRO VERSO DESTRA, IL CAMYAW DIMINUISCE
					// SE MI GIRO VERSO SINISTRA, IL CAMYAW AUMENTA
					if (damp || abs(tempHeliYaw - (camYaw + glm::radians(90.0f))) > HELI_DAMP_ANGLE) {
						// This is to avoid the spinning if you rotate by an angle > 180 degrees while on the ground
						if (camYaw - heliYaw + glm::radians(90.0f) > glm::radians(180.0f)) {
							camYaw -= 2 * M_PI;
							camYawOld -= 2 * M_PI;
						}
						else if (camYaw - heliYaw + glm::radians(90.0f) < -glm::radians(180.0f)) {
							camYaw += 2 * M_PI;
							camYawOld += 2 * M_PI;
						}
						//if (camYaw + glm::radians(90.0f) - heliYaw < glm::radians(180.0f)) camYaw += 2 * M_PI;
						tempHeliYaw = (heliYaw * exp(-HELI_DAMP_SPEED * deltaT)) +
							(camYaw + glm::radians(90.0f)) * (1 - exp(-HELI_DAMP_SPEED * deltaT));
						heliYaw = tempHeliYaw;
						damp = true;
						if (abs(tempHeliYaw - (camYaw + glm::radians(90.0f))) < glm::radians(0.1f))
							damp = false;
					}

				}

				// Updating heliMoveSpeed velocity
				if (m.y == 0) {
					if (heliMoveSpeed.y < 0) {
						heliMoveSpeed.y += HELI_DEC_SPEED * deltaT;
						if (heliMoveSpeed.y > 0) heliMoveSpeed.y = 0;
					}
					else {
						heliMoveSpeed.y -= HELI_DEC_SPEED * deltaT;
						if (heliMoveSpeed.y < 0) heliMoveSpeed.y = 0;
					}
				}

				heliMoveSpeed.y += m.y * HELI_ACC_SPEED * deltaT;

				// updating heli position 
				heliPosition += uy * heliMoveSpeed.y * deltaT;

				// blocking the helicopter from going under the terrain
				if ((heliPosition).y < HELI_GROUND) {
					heliPosition.y = HELI_GROUND;
					heliMoveSpeed.y = 0.0f;
				}
				// blocking the helicopter from going lower than a certain height while moving (security reasons)
				else if (heliPosition.y < 2 * HELI_GROUND && (abs(heliMoveSpeed.x) > 0 || abs(heliMoveSpeed.z) > 0))
				{
					heliPosition.y = 2 * HELI_GROUND;
					heliMoveSpeed.y = 0.0f;
				}

				// updating pitch
				camPitch = camPitch < vehicleMinPitch ? vehicleMinPitch :
					(camPitch > vehicleMaxPitch ? vehicleMaxPitch : camPitch);

				// limiting the inclination
				if (heliRoll < -0.2f)
					heliRoll = -0.2f;
				else if (heliRoll > 0.2f)
					heliRoll = 0.2f;

				if (heliPitch < -0.2f)
					heliPitch = -0.2f;
				else if (heliPitch > 0.2f)
					heliPitch = 0.2f;
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

			// update the rotation of both blades - the top one spin faster and slower if the player is going up or down
			if (m.y) tempRotation = (1 + (m.y / 4.0f)) * HELI_BLADE_SPEED;
			else tempRotation = HELI_BLADE_SPEED;
			heliTopBladeYaw += tempRotation * deltaT;
			if (heliTopBladeYaw > 2 * M_PI) heliTopBladeYaw -= 2 * M_PI;

			heliBackBladeRoll += HELI_BLADE_SPEED * deltaT;
			if (heliBackBladeRoll > 2 * M_PI) heliBackBladeRoll -= 2 * M_PI;


			break;
		default:
			printf("\n\nSOMETHING'S WRONG I CAN FEEL IT\n");
			break;
		}

		// UPDATE ALL WORLD MATRICES (except for the player, since it doesn't have a model)

		// Tank World Matrix
		tempWorld =
			glm::translate(glm::mat4(1), tankPosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, tankYaw + glm::radians(45.0f), 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1.0f));
		WorldTank = tempWorld;


		// Car World Matrix
		tempWorld =
			glm::translate(glm::mat4(1), carPosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw + glm::radians(45.0f), 0))) *
			glm::scale(glm::mat4(1), glm::vec3(CAR_SCALE));
		WorldCar = tempWorld;


		// Front Right Tire World Matrix
		glm::vec3 rightTirePosition = carPosition + RIGHT_TIRE_OFFSET;

		tempWorld =
			glm::translate(glm::mat4(1), rightTirePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw - glm::radians(45.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.375f));

		glm::mat4 RyC = glm::rotate(glm::mat4(1), carYaw - glm::radians(45.0f), glm::vec3(0, 1, 0));
		glm::mat4 TPosOffsetsC = glm::translate(glm::mat4(1), rightTirePosition);
		glm::mat4 TPosC = glm::translate(glm::mat4(1), carPosition);
		glm::mat4 TOffsetsC = glm::translate(glm::mat4(1), RIGHT_TIRE_OFFSET);
		glm::mat4 applyRotation = glm::rotate(glm::mat4(1), tireRotation, glm::vec3(1, 0, 0));
		glm::mat4 applyAngle = glm::rotate(glm::mat4(1), -tireAngle, glm::vec3(0, 1, 0));

		tempWorld =
			TPosC *
			RyC *
			TOffsetsC *
			applyAngle *
			applyRotation *
			glm::inverse(RyC) *
			glm::inverse(TPosOffsetsC) *
			tempWorld;

		WorldRightTire = tempWorld;


		// Front Left Tire World Matrix
		glm::vec3 leftTirePosition = carPosition + LEFT_TIRE_OFFSET;
		tempWorld =
			glm::translate(glm::mat4(1), leftTirePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw - glm::radians(45.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.375f));

		RyC = glm::rotate(glm::mat4(1), carYaw - glm::radians(45.0f), glm::vec3(0, 1, 0));
		TPosOffsetsC = glm::translate(glm::mat4(1), leftTirePosition);
		TPosC = glm::translate(glm::mat4(1), carPosition);
		TOffsetsC = glm::translate(glm::mat4(1), LEFT_TIRE_OFFSET);
		applyRotation = glm::rotate(glm::mat4(1), tireRotation, glm::vec3(1, 0, 0));
		applyAngle = glm::rotate(glm::mat4(1), -tireAngle, glm::vec3(0, 1, 0));

		tempWorld =
			TPosC *
			RyC *
			TOffsetsC *
			applyAngle *
			applyRotation *
			glm::inverse(RyC) *
			glm::inverse(TPosOffsetsC) *
			tempWorld;

		WorldLeftTire = tempWorld;

		glm::vec3 backRightTirePosition = carPosition + BACK_RIGHT_TIRE_OFFSET;

		tempWorld =
			glm::translate(glm::mat4(1), backRightTirePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw - glm::radians(45.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.375f));

		RyC = glm::rotate(glm::mat4(1), carYaw - glm::radians(45.0f), glm::vec3(0, 1, 0));
		TPosOffsetsC = glm::translate(glm::mat4(1), backRightTirePosition);
		TPosC = glm::translate(glm::mat4(1), carPosition);
		TOffsetsC = glm::translate(glm::mat4(1), BACK_RIGHT_TIRE_OFFSET);
		applyRotation = glm::rotate(glm::mat4(1), tireRotation, glm::vec3(1, 0, 0));
		applyAngle = glm::rotate(glm::mat4(1), -tireAngle, glm::vec3(0, 1, 0));

		tempWorld =
			TPosC *
			RyC *
			TOffsetsC *
			applyRotation *
			glm::inverse(RyC) *
			glm::inverse(TPosOffsetsC) *
			tempWorld;

		WorldBackRightTire = tempWorld;

		glm::vec3 backLeftTirePosition = carPosition + BACK_LEFT_TIRE_OFFSET;

		tempWorld =
			glm::translate(glm::mat4(1), backLeftTirePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw - glm::radians(45.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.375f));

		RyC = glm::rotate(glm::mat4(1), carYaw - glm::radians(45.0f), glm::vec3(0, 1, 0));
		TPosOffsetsC = glm::translate(glm::mat4(1), backLeftTirePosition);
		TPosC = glm::translate(glm::mat4(1), carPosition);
		TOffsetsC = glm::translate(glm::mat4(1), BACK_LEFT_TIRE_OFFSET);
		applyRotation = glm::rotate(glm::mat4(1), tireRotation, glm::vec3(1, 0, 0));
		applyAngle = glm::rotate(glm::mat4(1), -tireAngle, glm::vec3(0, 1, 0));

		tempWorld =
			TPosC *
			RyC *
			TOffsetsC *
			applyRotation *
			glm::inverse(RyC) *
			glm::inverse(TPosOffsetsC) *
			tempWorld;

		WorldBackLeftTire = tempWorld;

		// helicopter back to normal inclination
		if ((m.x == 0 || gameState != GameState::HELI) && heliPitch < 0.01f) {
			heliPitch -= -0.5f * deltaT;
			if (heliPitch > 0) heliPitch = 0.0f;
		}
		else if ((m.x == 0 || gameState != GameState::HELI) && heliPitch > 0.01f) {
			heliPitch -= 0.5f * deltaT;
			if (heliPitch < 0) heliPitch = 0.0f;
		}
		if ((m.z == 0 || gameState != GameState::HELI) && heliRoll < 0.01f) {
			heliRoll -= -0.5f * deltaT;
			if (heliRoll > 0) heliRoll = 0.0f;
		}
		else if ((m.z == 0 || gameState != GameState::HELI) && heliRoll > 0.01f) {
			heliRoll -= 0.5f * deltaT;
			if (heliRoll < 0) heliRoll = 0.0f;
		}

		// TODO: THIS FEELS LIKE A WORKAROUND - this should be part of the helicopter movement handling, but right now it's only
		// working while in GameState::HELI. The heli should also return to the normal inclination when the player drops off.
		// and it should be inserted somewhere here - again, feels like a patch instead of a good implementation 
		// helicopter gravity
		if (gameState != GameState::HELI && heliPosition.y > HELI_GROUND) {
			heliMoveSpeed.y -= 2 * GRAVITY * deltaT;
			heliPosition.y += heliMoveSpeed.y * deltaT;
			if (heliPosition.y < HELI_GROUND) heliPosition.y = HELI_GROUND;
		}
		// Heli World Matrix
		tempWorld =
			glm::translate(glm::mat4(1), heliPosition) *
			glm::mat4(
				glm::quat(glm::vec3(0.0f, heliYaw, 0.0f)) *
				glm::quat(glm::vec3(0.0f, 0.0f, heliRoll)) *
				glm::quat(glm::vec3(heliPitch, 0.0f, 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(1.0f));
		WorldHeli = tempWorld;



		// Top Blade World Matrix
		glm::vec3 topBladePosition = heliPosition + TOP_BLADE_OFFSET;
		tempWorld =
			glm::translate(glm::mat4(1), topBladePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.925f));

		glm::mat4 RzH = glm::rotate(glm::mat4(1), -heliPitch, glm::vec3(0, 0, 1));
		glm::mat4 RxH = glm::rotate(glm::mat4(1), heliRoll, glm::vec3(1, 0, 0));
		glm::mat4 RyH = glm::rotate(glm::mat4(1), heliYaw - glm::radians(90.0f), glm::vec3(0, 1, 0));
		glm::mat4 TPosOffsetsH = glm::translate(glm::mat4(1), topBladePosition);
		glm::mat4 TPosH = glm::translate(glm::mat4(1), heliPosition);
		glm::mat4 TOffsetsH = glm::translate(glm::mat4(1), TOP_BLADE_OFFSET);
		glm::mat4 rotation = glm::rotate(glm::mat4(1), heliTopBladeYaw, glm::vec3(0, 1, 0));

		tempWorld =
			TPosH *
			RyH *
			RxH *
			RzH *
			TOffsetsH *
			rotation *
			glm::inverse(TPosOffsetsH) *
			tempWorld;
		WorldHeliTopBlade = tempWorld;


		// Back Blade World Matrix
		glm::vec3 backBladeOffset = heliPosition + TOP_BLADE_OFFSET;
		tempWorld =
			glm::translate(glm::mat4(1), backBladeOffset) *
			glm::mat4(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(1.0f));

		RzH = glm::rotate(glm::mat4(1), -heliPitch, glm::vec3(0, 0, 1));
		RxH = glm::rotate(glm::mat4(1), heliRoll, glm::vec3(1, 0, 0));
		RyH = glm::rotate(glm::mat4(1), heliYaw - glm::radians(90.0f), glm::vec3(0, 1, 0));
		glm::mat4 AdaptationMatrix = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 1, 0));
		TPosOffsetsH = glm::translate(glm::mat4(1), backBladeOffset);
		TPosH = glm::translate(glm::mat4(1), heliPosition);
		TOffsetsH = glm::translate(glm::mat4(1), BACK_BLADE_OFFSET);
		rotation = glm::rotate(glm::mat4(1), heliBackBladeRoll, glm::vec3(1, 0, 0));

		tempWorld =
			TPosH *
			RyH *
			RxH *
			RzH *
			TOffsetsH *
			rotation *
			glm::inverse(TPosOffsetsH) *
			tempWorld * AdaptationMatrix;

		WorldHeliBackBlade = tempWorld;

		// Update the ViewPrj and the camPos based on the gamestate
		switch (gameState)
		{
		case WALK:
			camPos = playerPosition;
			ViewPrj = MakeViewPrjWalk();

			break;
		case TANK:
			ViewPrj = MakeViewPrjTank(camPos);

			if (possoScendere && !transition && action) {
				transition = true;
				gameState = GameState::WALK;
				camPitch = 0.0f;
				playerPosition = tankPosition;
				playerPosition.y = PLAYER_HEIGHT;
				playerPosition.x += 3 * sin(tankYaw);
				playerPosition.z += 3 * cos(tankYaw);
				tankMoveSpeed = 0.0f;
			}

			break;
		case CAR:
			ViewPrj = MakeViewPrjCar(camPos);

			if (possoScendere && !transition && action) {
				transition = true;
				gameState = GameState::WALK;
				camPitch = 0.0f;
				playerPosition = carPosition;
				//playerPosition += glm::vec3(-2 * cos(carYaw), 0 , - 2 *cos(carYaw));
				playerPosition.y = PLAYER_HEIGHT;
				playerPosition.x += 2.2f * sin(carYaw + glm::radians(65.0f));
				playerPosition.z += 2.2f * cos(carYaw + glm::radians(65.0f));
				carMoveSpeed = 0.0f;
			}

			break;
		case HELI:
			ViewPrj = MakeViewPrjHeli(camPos);

			if (possoScendere && !transition && action) {
				transition = true;
				gameState = GameState::WALK;
				camPitch = 0.0f;
				playerPosition = heliPosition;
				playerPosition.y -= HELI_GROUND;
				if (playerPosition.y < PLAYER_HEIGHT) playerPosition.y = PLAYER_HEIGHT;
				playerPosition.x += 3 * sin(heliYaw);
				playerPosition.z += 3 * cos(heliYaw);
				heliMoveSpeed = glm::vec3(0.0f);
			}
			break;
		default:
			printf("\n\nSOMETHING'S WRONG I CAN FEEL IT\n");
			break;
		}

		// If a transition (i.e. interaction with a vehicle) is going on, damp to the arrival ViewPrj
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
			// Save the distance from the closest object
			closestDistance = glm::length((playerPosition - posClosest));

			// If i'm not transitioning and the closest vehicle is at a certain range, display the prompt to use it
			if (!transition && closestDistance < INTERACTION_RANGE) {
				// display prompt to drive a vehicle

				if (action) {
					playerVerticalVelocity = 0.0f;
					possoScendere = false;
					transition = true;
					switch (tempClosest)
					{
						// without updating the oldPos, the view-prj too late with the damping 
						// if the object moves after the player left it
					case WALK:
						printf("\nCASINI FRA\n");
						break;
					case TANK:
						gameState = GameState::TANK;
						oldTankPos = tankPosition;
						break;
					case CAR:
						gameState = GameState::CAR;
						oldCarPos = carPosition;
						break;
					case HELI:
						gameState = GameState::HELI;
						oldHeliPos = heliPosition;
						break;
					default:
						printf("\nC A S I N I   F R A\n");
						break;
					}
				}
			}

		}

		// possoScendere tracks the state of the fire button: it ignore new presses if the button is held down
		if (!action)
			possoScendere = true;

	}

	// Return the closest object to the player
	GameState ClosestObject() {
		return glm::length((playerPosition - tankPosition)) < glm::length((playerPosition - carPosition)) ?
			(glm::length((playerPosition - tankPosition)) < glm::length((playerPosition - heliPosition)) ? GameState::TANK : GameState::HELI) :
			(glm::length((playerPosition - carPosition)) < glm::length((playerPosition - heliPosition)) ? GameState::CAR : GameState::HELI);
	}

	// Compute the ViewProjection matrix with the look-in model for the first person view
	glm::mat4 MakeViewPrjWalk() {

		glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

		ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		ProjectionMatrix[1][1] *= -1;

		ViewMatrix =
			glm::rotate(glm::mat4(1), -camRoll, glm::vec3(0, 0, 1)) *
			glm::rotate(glm::mat4(1), -camPitch, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1), -camYaw, glm::vec3(0, 1, 0)) *
			glm::translate(glm::mat4(1), glm::vec3(-playerPosition.x, -playerPosition.y, -playerPosition.z));

		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		return ViewProjectionMatrix;
	}

	// Compute the ViewProjection matrix for the look-at model for the third person view of the tank
	glm::mat4 MakeViewPrjTank(glm::vec3& camPos) {

		// World Matrix used for the camera
		glm::mat4 tempWorld = glm::translate(glm::mat4(1), oldTankPos) *
			glm::mat4(glm::quat(glm::vec3(0, camYawNew, 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1));

		// calculating the View-Projection Matrix
		glm::vec4 tempCamPos = tempWorld * glm::vec4(0, tankCamHeight + (tankCamDist * sin(camPitchNew)), tankCamDist * cos(camPitchNew), 1);
		camPos = glm::vec3(tempCamPos.x, tempCamPos.y, tempCamPos.z);


		glm::vec3 targetPointedPosition = glm::vec3(newTankPos.x, newTankPos.y + tankCamHeight, newTankPos.z);

		glm::mat4 ViewMatrix = glm::lookAt(camPos, targetPointedPosition, glm::vec3(0, 1, 0));
		ViewMatrix = glm::rotate(glm::mat4(1), camRoll, glm::vec3(0, 0, 1)) * ViewMatrix;

		glm::mat4 ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		ProjectionMatrix[1][1] *= -1;

		return ProjectionMatrix * ViewMatrix;
	}

	// Compute the ViewProjection matrix for the look-at model for the third person view of the car
	glm::mat4 MakeViewPrjCar(glm::vec3& camPos) {

		// World Matrix used for the camera
		glm::mat4 tempWorld = glm::translate(glm::mat4(1), oldCarPos) *
			glm::mat4(glm::quat(glm::vec3(0, camYawNew, 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1));

		// calculating the View-Projection Matrix
		glm::vec4 tempCamPos = tempWorld * glm::vec4(0, carCamHeight + (carCamDist * sin(camPitchNew)), carCamDist * cos(camPitchNew), 1);
		camPos = glm::vec3(tempCamPos.x, tempCamPos.y, tempCamPos.z);


		glm::vec3 targetPointedPosition = glm::vec3(newCarPos.x, newCarPos.y + carCamHeight, newCarPos.z);

		glm::mat4 ViewMatrix = glm::lookAt(camPos, targetPointedPosition, glm::vec3(0, 1, 0));
		ViewMatrix = glm::rotate(glm::mat4(1), camRoll, glm::vec3(0, 0, 1)) * ViewMatrix;

		glm::mat4 ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		ProjectionMatrix[1][1] *= -1;

		return ProjectionMatrix * ViewMatrix;
	}

	// Compute the ViewProjection matrix for the look-at model for the third person view of the helicopter
	glm::mat4 MakeViewPrjHeli(glm::vec3& camPos) {

		// World Matrix used for the camera
		glm::mat4 tempWorld = glm::translate(glm::mat4(1), oldHeliPos) *
			glm::mat4(glm::quat(glm::vec3(0, camYawNew, 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1));

		// calculating the View-Projection Matrix
		glm::vec4 tempCamPos = tempWorld * glm::vec4(0, heliCamHeight + (heliCamDist * sin(camPitchNew)), heliCamDist * cos(camPitchNew), 1);
		camPos = glm::vec3(tempCamPos.x, tempCamPos.y, tempCamPos.z);


		glm::vec3 targetPointedPosition = glm::vec3(newHeliPos.x, newHeliPos.y + heliCamHeight, newHeliPos.z);

		glm::mat4 ViewMatrix = glm::lookAt(camPos, targetPointedPosition, glm::vec3(0, 1, 0));
		ViewMatrix = glm::rotate(glm::mat4(1), camRoll, glm::vec3(0, 0, 1)) * ViewMatrix;

		glm::mat4 ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		ProjectionMatrix[1][1] *= -1;

		return ProjectionMatrix * ViewMatrix;
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