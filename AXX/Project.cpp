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

// Ubo for the phong reflection model used for vehicles
struct MeshUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

// Ubo for the GGX BRDF used for the monuments
struct GGXMeshUniformBlock {
	alignas(4) float amb;
	alignas(4) float metallic;
	alignas(4) float roughness;
	alignas(4) float fresnel;
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

// Ubo for the GGX BRDF used for the monuments (without color)
struct GGXTextureMeshUniformBlock {
	alignas(4) float amb;
	alignas(4) float metallic;
	alignas(4) float roughness;
	alignas(4) float fresnel;
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

// Gubo for the direct light 
struct DirectGlobalUniformBlock {
	alignas(16) glm::vec3 DlightDir;
	alignas(16) glm::vec3 DlightColor;
	alignas(16) glm::vec3 AmbLightColor;
	alignas(16) glm::vec3 eyePos;
};

// Gubo for the point light
struct PointGlobalUniformBlock {
	alignas(4) float beta;
	alignas(4) float g;
	alignas(16) glm::vec3 PlightPos;
	alignas(16) glm::vec3 PlightColor;
	alignas(16) glm::vec3 AmbLightColor;
	alignas(16) glm::vec3 eyePos;
	
};

// Ubo for the floor
struct UniformBufferObject {
	alignas(4) float amb;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

// Gamestate enum structure
enum GameState {
	WALK,
	TANK,
	CAR,
	HELI
};

// Vertex format used by vehicles and monuments
struct VertexStandard {
	glm::vec3 pos;
	glm::vec3 norm;
};

// Vertex format used for the floor
struct VertexFloor {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

#define M_PI 3.141595f

// MAIN ! 
class FinalProject : public BaseProject {
protected:

	// Current aspect ratio 
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLDGubo;			// direct light gubo
	DescriptorSetLayout DSLPGubo[3];		// point light gubo - one for each monument
	DescriptorSetLayout DSLMonoColor;
	DescriptorSetLayout DSLVertexFloor;

	// Vertex formats
	VertexDescriptor VMonoColor;
	VertexDescriptor VVertexFloor;

	// Pipelines [Shader couples] - each monuments has its pipeline because it uses a different light
	Pipeline PMonoColor;
	Pipeline PVertexFloor;
	Pipeline PPagoda;
	Pipeline PEiffel;
	Pipeline PRushmore;

	// Models, textures and Descriptors
	// Vehicles models
	Model<VertexStandard> MTank, MCar, MCarSingleTire, MHeliBody, MHeliTopBlade, MHeliBackBlade;
	// Monument models
	Model<VertexStandard> MPagoda;
	Model<VertexStandard> MEiffel;
	Model<VertexStandard> MPyramid;
	Model<VertexFloor> MRushmore;		// This monument uses a different vertex format because it has a texture
	// Floor model
	Model<VertexFloor> MFloor;

	// Descriptor Sets
	DescriptorSet DSDGubo;				// direct light gubo
	DescriptorSet DSPGubo[3];			// point light gubo
	DescriptorSet DSTank, DSCar, DSCarBackLeftTire, DSCarBackRightTire, DSCarLeftTire, DSCarRightTire,
					DSHeliBody, DSHeliTopBlade, DSHeliBackBlade;
	DescriptorSet DSPagoda, DSEiffel, DSRushmore, DSFloor;

	// Textures
	Texture TRushmore, TFloor;

	// C++ storage for uniform variables
	MeshUniformBlock uboTank, uboCar, uboHeliBody, uboHeliTopBlade, uboBackLeftTire, uboBackRightTire, uboLeftTire, uboRightTire, uboHeliBackBlades;
	GGXMeshUniformBlock uboPagoda, uboEiffel;
	GGXTextureMeshUniformBlock uboRushmore;
	UniformBufferObject uboFloor;

	DirectGlobalUniformBlock dGubo;				// direct light gubo
	PointGlobalUniformBlock pGubo[3];			// point light gubo

	// ------------------------------------------------------------------------------------

	// Other application parameters
	
	const bool RENDER_RUSHMORE = false;

	// Actual gamestate - signals whether the player is walking or using a vehicle
	GameState gameState = GameState::WALK;
	// True if the player is getting into a vehicle or getting off it
	bool transition = false;
	// actionCheck tracks the state of the fire button: it ignore new presses if the button is held down
	bool actionCheck = true;
	// Duration of the transition while getting into a vehicle or getting off it
	const float TRANS_DURATION = 0.9f;
	// Used to save the time that has passed since the start of the transition
	float transitionTimer = 0.0f;


	// Parameters of the camera
	const float FOVy = glm::radians(70.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 200.0f;

	// Starting positiong and related costants 
	const float PLAYER_HEIGHT = 1.7f;
	const float HELI_GROUND = 1.68f;
	const float CAR_HEIGHT = 0.7f;

	const glm::vec3 PlayerStartingPos = glm::vec3(0.0f, PLAYER_HEIGHT, 0.0f);
	const glm::vec3 TankStartingPos = glm::vec3(-7.0f, 0.0f, -12.0f);
	const glm::vec3 CarStartingPos = glm::vec3(0.0f, 1.075f, -12.0f);
	const glm::vec3 HeliStartingPos = glm::vec3(7.0f, HELI_GROUND, -12.0f);
	// The tank, heli and car need 3 variables for the damping implementation
	glm::vec3
		playerPosition = PlayerStartingPos,

		tankPosition = TankStartingPos,
		oldTankPos = TankStartingPos,
		newTankPos = TankStartingPos,

		carPosition = CarStartingPos,
		oldCarPos = CarStartingPos,
		newCarPos = CarStartingPos,

		heliPosition = HeliStartingPos,
		oldHeliPos = HeliStartingPos,
		newHeliPos = HeliStartingPos;

	// Position and light offsets of monuments
	const glm::vec3 EIFFEL_POSITION = glm::vec3(-0.5f, 0.05f, -57.5f);
	const glm::vec3 PAGODA_POSITION = glm::vec3(158.0f, 0.0f, -40.0f);
	const glm::vec3 RUSHMORE_POSITION = glm::vec3(-145.0f, 0.0f, -40.0f);
	// Eiffel tower doesn't have one because its light it's underneath it
	const glm::vec3 OFFSET_PAGODAS_LIGHT = glm::vec3(-7.0f, 4.0f, 12.0f);
	const glm::vec3 OFFSET_RUSHMORES_LIGHT = glm::vec3(+25.0f, 15.0f, 15.0f);

	// The ViewPrj of the previous frame: it's used during the transition 
	glm::mat4 oldViewPrj = glm::mat4(1);


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

	// Interaction range to get inside a vehicles
	const float INTERACTION_RANGE = 4.0f;

	// Gravity force for player/helicopter
	const float GRAVITY = 6.0f;

	// Rotation speed of the camera, for both the player and the vehicles
	const float CAM_ROT_SPEED = glm::radians(120.0f);
	
	// Player movement speed
	const float PLAYER_MOVE_SPEED = 10.0f;

	// Vertical velocity of the player
	float playerVerticalVelocity = 0.0f;

	// Tank movement constants
	const float TANK_ACC_SPEED = 3.0f;
	const float TANK_DEC_SPEED = 5.0f;
	const float TANK_ROT_SPEED = glm::radians(35.0f);
	const float TANK_MAX_SPEED = 3.0f;
	// Tank movement speed: it's not a vector because the tank can cannot move sideways
	float tankMoveSpeed = 0.0f;

	// Car movement constants
	const float CAR_ACC_SPEED = 8.0f;
	const float CAR_DEC_SPEED = 7.0f;
	const float CAR_MAX_SPEED = 15.0f;
	const float CAR_ROT_SPEED = glm::radians(20.0f);
	const float CAR_SCALE = 0.775f;
	// Offsets used to compute the world matrices
	const glm::vec3 RIGHT_TIRE_OFFSET = glm::vec3(0.8f, -0.475f, -1.025f);
	const glm::vec3 LEFT_TIRE_OFFSET = glm::vec3(-0.8f, -0.475f, -1.025f);
	const glm::vec3 BACK_RIGHT_TIRE_OFFSET = glm::vec3(0.8f, -0.475f, 1.15f);
	const glm::vec3 BACK_LEFT_TIRE_OFFSET = glm::vec3(-0.8f, -0.475f, 1.15f);
	// Rotation speed of the tires
	const float TIRE_ROTATION_SPEED = glm::radians(0.8f);
	// Maximum vertical angle
	const float TIRE_MAX_ANGLE = glm::radians(40.0f);
	// Damping spped for the vertical angle of the tires
	const float LAMBDA_ANGLE = 10.0f;
	// Car movement speed: it's not a vector because the car can cannot move sideways
	float carMoveSpeed = 0.0f; 
	// Tire actual horizontal rotation
	float tireRotation = 0.0f;
	// Tire actual vertical rotation
	float tireAngle = 0.0f;


	// Heli movement constants
	const float HELI_ACC_SPEED = 15.0f;
	const float HELI_DEC_SPEED = 10.0f;
	const float HELI_MAX_SPEED = 25.0f;
	const float HELI_VERT_ACC_SPEED = 5.0f;
	const float HELI_VERT_MAX_SPEED = 10.0f;
	// Deadzone angle for the camera
	const float HELI_DAMP_ANGLE = glm::radians(10.0f);
	// Damping speed while returning to center
	const float HELI_DAMP_SPEED = 1.2f;
	// Rotating speed of heli's blades
	const float HELI_BLADE_SPEED = -10.0f;
	// Offsets used to compute the world matrices
	const glm::vec3 TOP_BLADE_OFFSET = glm::vec3(0.0f, 1.25f, 0.0f);
	const glm::vec3 BACK_BLADE_OFFSET = glm::vec3(0.175f, 1.075f, 5.425f);
	// Heli movement speed
	glm::vec3 heliMoveSpeed = glm::vec3(0.0f);
	// actualRotation values changes based on inputs
	float heliActualBladeSpeed = HELI_BLADE_SPEED;
	// False while the angle deadzone is active
	bool heliReset = false;

	// Parameters needed in the damping implementation - 3rd person view
	const float LAMBDA_ROTATION = 20.0f,
				LAMBDA_MOVEMENT = 5.0f,
				DEADZONE = 1.0f,
				LAMBDA_TRANSITION = 8.0f;
	bool updatePos = false;

	// Angles and variables - 'old' and 'new' are needed to implement damping
	float camYaw = 0.0f, camPitch = 0.0f, 
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
		initialBackgroundColor = { 0.0f, 0.0025f, 0.0125f, 1.0f };

		// Descriptor pool sizes
		/* FinalProject */
		/* Update the requirements for the size of the pool */
		uniformBlocksInPool = 18;
		texturesInPool = 2;
		setsInPool = 18;

		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
	}

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		/* Init the Descriptor Set Layout */
		// GUBOs
		DSLDGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});
		for (int i = 0; i < 3; i++)
		{
			DSLPGubo[i].init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
				});
		}
		// Scene objects
		DSLVertexFloor.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});
		DSLMonoColor.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		// Vertex descriptors
		VMonoColor.init(this, {
			{0, sizeof(VertexStandard), VK_VERTEX_INPUT_RATE_VERTEX }
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexStandard, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexStandard, norm),
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
		// Example: set = 0 -> DSLDGubo, set = 1 -> DSLMonoColor
		PMonoColor.init(this, &VMonoColor, "shaders/Pieces/MonoColorVert.spv", "shaders/Pieces/MonoColorFrag.spv", { &DSLDGubo, &DSLMonoColor });
		
		PVertexFloor.init(this, &VVertexFloor, "shaders/Floor/FloorVert.spv", "shaders/Floor/FloorFrag.spv", { &DSLVertexFloor });
		// Needed to always render the floor, regardless of backface culling
		PVertexFloor.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
		
		PEiffel.init(this, &VMonoColor, "shaders/Monuments/GGXVert.spv", "shaders/Monuments/GGXFrag.spv", { &DSLPGubo[0], &DSLDGubo, &DSLMonoColor });
		PPagoda.init(this, &VMonoColor, "shaders/Monuments/GGXVert.spv", "shaders/Monuments/GGXFrag.spv", { &DSLPGubo[1], &DSLDGubo, &DSLMonoColor });
		PRushmore.init(this, &VVertexFloor, "shaders/Monuments/GGXTextureVert.spv", "shaders/Monuments/GGXTextureFrag.spv", { &DSLPGubo[2], &DSLDGubo, &DSLVertexFloor });

		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		MTank.init(this, &VMonoColor, "Models/Tank.obj", OBJ);
		MCar.init(this, &VMonoColor, "Models/Jeep.obj", OBJ);
		MCarSingleTire.init(this, &VMonoColor, "Models/JeepTire.obj", OBJ);
		MHeliBody.init(this, &VMonoColor, "Models/HeliBody.obj", OBJ);
		MHeliTopBlade.init(this, &VMonoColor, "Models/HeliTopBlade.obj", OBJ);
		MHeliBackBlade.init(this, &VMonoColor, "Models/HeliBackBlade.obj", OBJ);
		MPagoda.init(this, &VMonoColor, "Models/Pagoda.obj", OBJ);
		MEiffel.init(this, &VMonoColor, "Models/Eiffel.obj", OBJ);
		if (RENDER_RUSHMORE)
			MRushmore.init(this, &VVertexFloor, "Models/Rushmore.obj", OBJ);
		else
			MRushmore.init(this, &VVertexFloor, "Models/JeepTire.obj", OBJ);
		// MFloor is created by specifying the position, normals, and UV coordinate of each vertex and how they are connected
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
		TRushmore.init(this, "textures/rushmore.png");
		TFloor.init(this, "textures/RisikoMap.png");
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// Pipeline creation
		PMonoColor.create();
		PVertexFloor.create();
		PPagoda.create();
		PEiffel.create();
		PRushmore.create();

		// Descriptor set initialization
		// Gubos
		DSDGubo.init(this, &DSLDGubo, {
					{0, UNIFORM, sizeof(DirectGlobalUniformBlock), nullptr}
			});
		for (int i = 0; i < 3; i++)
		{
			DSPGubo[i].init(this, &DSLDGubo, {
					{0, UNIFORM, sizeof(PointGlobalUniformBlock), nullptr}
				});
		}

		// Tank
		DSTank.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			});

		// Car
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


		// Heli
		DSHeliBody.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSHeliTopBlade.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});
		DSHeliBackBlade.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr}
			});

		// Monuments
		DSPagoda.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(GGXMeshUniformBlock), nullptr}
			});
		DSEiffel.init(this, &DSLMonoColor, {
					{0, UNIFORM, sizeof(GGXMeshUniformBlock), nullptr}
			});
		DSRushmore.init(this, &DSLVertexFloor, {
					{0, UNIFORM, sizeof(GGXTextureMeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TRushmore}
			});

		// Floor
		DSFloor.init(this, &DSLVertexFloor, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &TFloor}
			});

		
	}

	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PMonoColor.cleanup();
		PVertexFloor.cleanup();
		PPagoda.cleanup();
		PEiffel.cleanup();
		PRushmore.cleanup();
		// Cleanup datasets
		// Gubos
		DSDGubo.cleanup();
		for (int i = 0; i < 3; i++)
		{
			DSPGubo[i].cleanup();
		}
		// Tank
		DSTank.cleanup();
		// Car
		DSCar.cleanup();
		DSCarBackLeftTire.cleanup();
		DSCarBackRightTire.cleanup();
		DSCarLeftTire.cleanup();
		DSCarRightTire.cleanup();
		// Heli
		DSHeliBody.cleanup();
		DSHeliTopBlade.cleanup();
		DSHeliBackBlade.cleanup();
		// Monuments
		DSPagoda.cleanup();
		DSEiffel.cleanup();
		DSRushmore.cleanup();
		// Floor
		DSFloor.cleanup();
		
		
	}

	void localCleanup() {
		// Cleanup textures
		TFloor.cleanup();
		TRushmore.cleanup();

		// Cleanup models
		// Tank
		MTank.cleanup();
		// Car
		MCar.cleanup();
		MCarSingleTire.cleanup();
		// Heli
		MHeliBody.cleanup();
		MHeliTopBlade.cleanup();
		MHeliBackBlade.cleanup();
		// Monuments
		MPagoda.cleanup();
		MEiffel.cleanup();
		MRushmore.cleanup();
		// Floor
		MFloor.cleanup();

		// Cleanup descriptor set layouts
		DSLDGubo.cleanup();
		for (int i = 0; i < 3; i++)
		{
			DSLPGubo[i].cleanup();
		}
		DSLMonoColor.cleanup();
		DSLVertexFloor.cleanup();
		
		// Destroy the pipelines
		PMonoColor.destroy();
		PVertexFloor.destroy();
		PPagoda.destroy();
		PEiffel.destroy();
		PRushmore.destroy();
	}

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

		// Vehicles

		// binds the pipeline
		PMonoColor.bind(commandBuffer);
		// binds the dGubo to the command buffer to our new pipeline and set 0
		DSDGubo.bind(commandBuffer, PMonoColor, 0, currentImage);
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


		// Monuments

		// binds the pipeline
		PEiffel.bind(commandBuffer);
		// binds the descriptor set layout (pGubo)
		DSPGubo[0].bind(commandBuffer, PEiffel, 0, currentImage);
		// binds the descriptor set layout (dGubo)
		DSDGubo.bind(commandBuffer, PEiffel, 1, currentImage);
		// binds the mesh
		MEiffel.bind(commandBuffer);
		// binds the descriptor set layout
		DSEiffel.bind(commandBuffer, PEiffel, 2, currentImage);
		// record the drawing command in the commnad buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MEiffel.indices.size()), 1, 0, 0, 0);


		// binds the pipeline
		PPagoda.bind(commandBuffer);
		// binds the descriptor set layout (pGubo)
		DSPGubo[1].bind(commandBuffer, PPagoda, 0, currentImage);
		// binds the descriptor set layout (dGubo)
		DSDGubo.bind(commandBuffer, PPagoda, 1, currentImage);
		// binds the mesh
		MPagoda.bind(commandBuffer);
		// binds the descriptor set layout
		DSPagoda.bind(commandBuffer, PPagoda, 2, currentImage);
		// record the drawing command in the commnad buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPagoda.indices.size()), 1, 0, 0, 0);


		// binds the pipeline
		PRushmore.bind(commandBuffer);
		// binds the descriptor set layout (pGubo)
		DSPGubo[2].bind(commandBuffer, PRushmore, 0, currentImage);
		// binds the descriptor set layout (dGubo)
		DSDGubo.bind(commandBuffer, PRushmore, 1, currentImage);
		// binds the mesh
		MRushmore.bind(commandBuffer);
		// binds the descriptor set layout
		DSRushmore.bind(commandBuffer, PRushmore, 2, currentImage);
		// record the drawing command in the commnad buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MRushmore.indices.size()), 1, 0, 0, 0);

		// Floor

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
	
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// Variables needed for the game logic
		glm::vec3 camPos;
		glm::mat4 ViewPrj;
		glm::mat4 WorldPlayer, WorldTank, WorldCar, WorldBackLeftTire, WorldBackRightTire, WorldLeftTire, WorldRightTire, WorldHeli, WorldHeliTopBlade, WorldHeliBackBlade;

		// Base color of the vehicles
		glm::vec3 RED = glm::vec3(0.65f, 0.01f, 0.01f);
			

		// Function that contains all the logic of the game
		Logic(Ar, camPos, ViewPrj, 
			WorldPlayer, WorldTank, 
			WorldCar, WorldBackLeftTire, WorldBackRightTire, WorldLeftTire, WorldRightTire, 
			WorldHeli, WorldHeliTopBlade, WorldHeliBackBlade);

		// GUBO VALUES
		// Direct light
		dGubo.DlightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
		dGubo.DlightColor = glm::vec4(0.4f);
		dGubo.AmbLightColor = glm::vec3(0.05f);
		dGubo.eyePos = camPos;
		// Writes value to the GPU
		DSDGubo.map(currentImage, &dGubo, sizeof(dGubo), 0);

		// Point light (Eiffel)
		pGubo[0].PlightPos = EIFFEL_POSITION;
		pGubo[0].PlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		pGubo[0].AmbLightColor = glm::vec3(0.01f);
		pGubo[0].eyePos = camPos;
		pGubo[0].beta = 2.0f;
		pGubo[0].g = 15.0f;
		// Writes value to the GPU
		DSPGubo[0].map(currentImage, &pGubo[0], sizeof(pGubo[0]), 0);

		// Point light (Pagoda)
		pGubo[1].PlightPos = PAGODA_POSITION + OFFSET_PAGODAS_LIGHT;
		pGubo[1].PlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		pGubo[1].AmbLightColor = glm::vec3(0.01f);
		pGubo[1].eyePos = camPos;
		pGubo[1].beta = 2.0f;
		pGubo[1].g = 8.0f;
		// Writes value to the GPU
		DSPGubo[1].map(currentImage, &pGubo[1], sizeof(pGubo[1]), 0);
		
		// Point light (Rushmore)
		pGubo[2].PlightPos = RUSHMORE_POSITION + OFFSET_RUSHMORES_LIGHT;
		pGubo[2].PlightColor = glm::vec4(0.6f, 0.05f, 0.2f, 1.0f);
		pGubo[2].AmbLightColor = glm::vec3(0.5f);
		pGubo[2].eyePos = camPos;
		pGubo[2].beta = 1.0f;
		pGubo[2].g = 25.0f;
		// Writes value to the GPU
		DSPGubo[2].map(currentImage, &pGubo[2], sizeof(pGubo[2]), 0);

		
		// UBOs

		uboTank.amb = 1.0f; uboTank.gamma = 180.0f; uboTank.color = RED; uboTank.sColor = glm::vec3(1.0f);
		uboTank.mvpMat = ViewPrj * WorldTank;
		uboTank.mMat = WorldTank;
		uboTank.nMat = glm::inverse(glm::transpose(WorldTank));
		/* map the uniform data block to the GPU */
		DSTank.map(currentImage, &uboTank, sizeof(uboTank), 0);


		uboCar.amb = 1.0f; uboCar.gamma = 180.0f; uboCar.color = RED; uboCar.sColor = glm::vec3(1.0f);
		uboCar.mvpMat = ViewPrj * WorldCar;
		uboCar.mMat = WorldCar;
		uboCar.nMat = glm::inverse(glm::transpose(WorldCar));
		/* map the uniform data block to the GPU */
		DSCar.map(currentImage, &uboCar, sizeof(uboCar), 0);

		uboBackLeftTire.amb = 1.0f; uboBackLeftTire.gamma = 180.0f; uboBackLeftTire.color = RED; uboBackLeftTire.sColor = glm::vec3(1.0f);
		uboBackLeftTire.mvpMat = ViewPrj * WorldBackLeftTire;
		uboBackLeftTire.mMat = WorldBackLeftTire;
		uboBackLeftTire.nMat = glm::inverse(glm::transpose(WorldBackLeftTire));
		/* map the uniform data block to the GPU */
		DSCarBackLeftTire.map(currentImage, &uboBackLeftTire, sizeof(uboBackLeftTire), 0);

		uboBackRightTire.amb = 1.0f; uboBackRightTire.gamma = 180.0f; uboBackRightTire.color = RED; uboBackRightTire.sColor = glm::vec3(1.0f);
		uboBackRightTire.mvpMat = ViewPrj * WorldBackRightTire;
		uboBackRightTire.mMat = WorldBackRightTire;
		uboBackRightTire.nMat = glm::inverse(glm::transpose(WorldBackRightTire));
		/* map the uniform data block to the GPU */
		DSCarBackRightTire.map(currentImage, &uboBackRightTire, sizeof(uboBackRightTire), 0);

		uboLeftTire.amb = 1.0f; uboLeftTire.gamma = 180.0f; uboLeftTire.color = RED; uboLeftTire.sColor = glm::vec3(1.0f);
		uboLeftTire.mvpMat = ViewPrj * WorldLeftTire;
		uboLeftTire.mMat = WorldLeftTire;
		uboLeftTire.nMat = glm::inverse(glm::transpose(WorldLeftTire));
		/* map the uniform data block to the GPU */
		DSCarLeftTire.map(currentImage, &uboLeftTire, sizeof(uboLeftTire), 0);

		uboRightTire.amb = 1.0f; uboRightTire.gamma = 180.0f; uboRightTire.color = RED; uboRightTire.sColor = glm::vec3(1.0f);
		uboRightTire.mvpMat = ViewPrj * WorldRightTire;
		uboRightTire.mMat = WorldRightTire;
		uboRightTire.nMat = glm::inverse(glm::transpose(WorldRightTire));
		/* map the uniform data block to the GPU */
		DSCarRightTire.map(currentImage, &uboRightTire, sizeof(uboRightTire), 0);


		uboHeliBody.amb = 1.0f; uboHeliBody.gamma = 180.0f; uboHeliBody.color = RED; uboHeliBody.sColor = glm::vec3(1.0f);
		uboHeliBody.mvpMat = ViewPrj * WorldHeli;
		uboHeliBody.mMat = WorldHeli;
		uboHeliBody.nMat = glm::inverse(glm::transpose(WorldHeli));
		/* map the uniform data block to the GPU */
		DSHeliBody.map(currentImage, &uboHeliBody, sizeof(uboHeliBody), 0);

		uboHeliTopBlade.amb = 1.0f; uboHeliTopBlade.gamma = 180.0f; uboHeliTopBlade.color = RED; uboHeliTopBlade.sColor = glm::vec3(1.0f);
		uboHeliTopBlade.mvpMat = ViewPrj * WorldHeliTopBlade;
		uboHeliTopBlade.mMat = WorldHeliTopBlade;
		uboHeliTopBlade.nMat = glm::inverse(glm::transpose(WorldHeliTopBlade));
		/* map the uniform data block to the GPU */
		DSHeliTopBlade.map(currentImage, &uboHeliTopBlade, sizeof(uboHeliTopBlade), 0);

		uboHeliBackBlades.amb = 1.0f; uboHeliBackBlades.gamma = 180.0f; uboHeliBackBlades.color = RED; uboHeliBackBlades.sColor = glm::vec3(1.0f);
		uboHeliBackBlades.mvpMat = ViewPrj * WorldHeliBackBlade;
		uboHeliBackBlades.mMat = WorldHeliBackBlade;
		uboHeliBackBlades.nMat = glm::inverse(glm::transpose(WorldHeliBackBlade));
		/* map the uniform data block to the GPU */
		DSHeliBackBlade.map(currentImage, &uboHeliBackBlades, sizeof(uboHeliBackBlades), 0);

		glm::vec3 GRAY = glm::vec3(0.75f);
		glm::vec3 REFLECTION_GRAY = glm::vec3(0.4f);
		glm::vec3 GOLD = glm::vec3(0.742f, 0.515f, 0.007f);
		glm::vec3 REFLECTION_GOLD = glm::vec3(0.7f, 0.4f, 0.007f);
		glm::vec3 WHITE = glm::vec3(1.0f);

		glm::mat4 WorldEiffel =
			glm::translate(glm::mat4(1), EIFFEL_POSITION) *
			glm::mat4(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.4f));;

		uboEiffel.amb = 1.0f; uboEiffel.metallic = 0.1f; uboEiffel.roughness = 0.33f; uboEiffel.fresnel = 0.3f;
		uboEiffel.color = GRAY; uboEiffel.sColor = REFLECTION_GRAY;
		uboEiffel.mvpMat = ViewPrj * WorldEiffel;
		uboEiffel.mMat = WorldEiffel;
		uboEiffel.nMat = glm::inverse(glm::transpose(WorldEiffel));
		/* map the uniform data block to the GPU */
		DSEiffel.map(currentImage, &uboEiffel, sizeof(uboEiffel), 0);

		glm::mat4 WorldPagoda =
			glm::translate(glm::mat4(1), PAGODA_POSITION) *
			glm::mat4(glm::quat(glm::vec3(0.0f, -glm::radians(30.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.7f));;

		uboPagoda.amb = 1.0f; uboPagoda.metallic = 0.1f; uboPagoda.roughness = 0.1f; uboPagoda.fresnel = 0.4f;
		uboPagoda.color = GOLD; uboPagoda.sColor = REFLECTION_GOLD;
		uboPagoda.mvpMat = ViewPrj * WorldPagoda;
		uboPagoda.mMat = WorldPagoda;
		uboPagoda.nMat = glm::inverse(glm::transpose(WorldPagoda));
		/* map the uniform data block to the GPU */
		DSPagoda.map(currentImage, &uboPagoda, sizeof(uboPagoda), 0);

		glm::mat4 WorldRushmore =
			glm::translate(glm::mat4(1), RUSHMORE_POSITION) *
			glm::mat4(glm::quat(glm::vec3(0.0f, glm::radians(110.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(1.0f));;

		uboRushmore.amb = 1.0f; uboRushmore.metallic = 1.0f; uboRushmore.roughness = 1.0f; uboRushmore.fresnel = 0.1f;
		uboRushmore.sColor = WHITE;
		uboRushmore.mvpMat = ViewPrj * WorldRushmore;
		uboRushmore.mMat = WorldRushmore;
		uboRushmore.nMat = glm::inverse(glm::transpose(WorldRushmore));
		/* map the uniform data block to the GPU */
		DSRushmore.map(currentImage, &uboRushmore, sizeof(uboRushmore), 0);


		glm::mat4 WorldFloor = glm::mat4(1);
		uboFloor.amb = 0.1f;
		uboFloor.mMat = WorldFloor;
		uboFloor.mvpMat = ViewPrj * WorldFloor;
		uboFloor.nMat = glm::inverse(glm::transpose(WorldFloor));
		/* map the uniform data block to the GPU */
		DSFloor.map(currentImage, &uboFloor, sizeof(uboFloor), 0);

	}

	// Logic implementation of the application
	void Logic(float Ar, glm::vec3& camPos,
		glm::mat4& ViewPrj,
		glm::mat4& WorldPlayer,
		glm::mat4& WorldTank,
		glm::mat4& WorldCar, glm::mat4& WorldBackLeftTire, glm::mat4& WorldBackRightTire, glm::mat4& WorldLeftTire, glm::mat4& WorldRightTire, 
		glm::mat4& WorldHeli, glm::mat4& WorldHeliTopBlade, glm::mat4& WorldHeliBackBlade) {

		// Variables needed to manage user's input
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool action = false;
		// getSixAxis populates those variables 
		getSixAxis(deltaT, m, r, action);

		// placeholder variables for matrices
		glm::mat4 ViewMatrix, ProjectionMatrix, tempWorld;

		float temp = 0.0f;
		// for debugging
		// deltaT = 0.01f;
		
		// Computing camera angles when not transitioning
		if (!transition) {

			camYaw -= CAM_ROT_SPEED * r.y * deltaT;
			camPitch -= CAM_ROT_SPEED * r.x * deltaT;

			// Limiting the yaw to a value between 2 * M_PI and -2 * M_PI
			if (camYaw > 2 * M_PI) {
				camYaw -= 2 * M_PI;
				camYawOld -= 2 * M_PI;
				// Ff we're on the helicopter, we have to update its yaw too, otherwise it will spin
				if (gameState == GameState::HELI && heliPosition.y > 2 * HELI_GROUND) {
					heliYaw -= 2 * M_PI;
					tempHeliYaw -= 2 * M_PI;
				}
			}
			if (camYaw < -2 * M_PI) {
				camYaw += 2 * M_PI;
				camYawOld += 2 * M_PI;
				// Ff we're on the helicopter, we have to update its yaw too, otherwise it will spin
				if (gameState == GameState::HELI && heliPosition.y > 2 * HELI_GROUND) {
					heliYaw += 2 * M_PI;
					tempHeliYaw += 2 * M_PI;
				}
			}
		}
		// Damping implementation - those values are ignored if the player is in walking gamestate
		camPitchNew = (camPitchOld * exp(-LAMBDA_ROTATION * deltaT)) + camPitch * (1 - exp(-LAMBDA_ROTATION * deltaT));
		camPitchOld = camPitchNew;
		camYawNew = (camYawOld * exp(-LAMBDA_ROTATION * deltaT)) + camYaw * (1 - exp(-LAMBDA_ROTATION * deltaT));
		camYawOld = camYawNew;

		// Directional vectors
		glm::vec3 ux, uy, uz;
		
		// Update the position based on the gamestate
		switch (gameState)
		{
		case WALK:

			// Computing the movement versors 
			ux = glm::vec3(glm::rotate(glm::mat4(1),
				camYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));

			uz = glm::vec3(glm::rotate(glm::mat4(1),
				camYaw,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

			// Compute the new position if not transitioning
			if (!transition) {
				// updating player position
				playerPosition += ux * PLAYER_MOVE_SPEED * m.x * deltaT;
				playerPosition += uz * PLAYER_MOVE_SPEED * m.z * deltaT;
				// limiting pitch
				camPitch = camPitch < playerMinPitch ? playerMinPitch :
					(camPitch > playerMaxPitch ? playerMaxPitch : camPitch);

				// make the player fall if it's higher than the ground (gravity)
				if (playerPosition.y > PLAYER_HEIGHT) {
					playerVerticalVelocity += GRAVITY * deltaT;
					playerPosition.y -= playerVerticalVelocity * deltaT;
					if (playerPosition.y < PLAYER_HEIGHT) {
						playerPosition.y = PLAYER_HEIGHT;
						playerVerticalVelocity = 0.0f;
					}
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

			// Compute the new position if not transitioning
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
				// Needed while transitioning from walk to tank to avoid problem with the pitch
				SetPitches(&camPitch, &camPitchOld, &camPitchNew, vehicleStandardPitch);
			}

			// deadzone implementation
			if (glm::length((tankPosition - oldTankPos)) > DEADZONE) {
				updatePos = true;
			}
			if (updatePos) {
				newTankPos = (oldTankPos * exp(-LAMBDA_MOVEMENT * deltaT)) + tankPosition * (1 - exp(-LAMBDA_MOVEMENT * deltaT));
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

			// Compute the new position if not transitioning
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

				// Updating car's velocity 
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

				// Rotate the car only when moving
				if (carMoveSpeed != 0 && m.x != 0)
					carYaw -= m.x * carMoveSpeed * CAR_ROT_SPEED * deltaT;
			}
			else {
				// Needed while transitioning from walk to tank to avoid problem with the pitch
				SetPitches(&camPitch, &camPitchOld, &camPitchNew, vehicleStandardPitch);
			}

			// deadzone implementation
			if (glm::length((carPosition - oldCarPos)) > DEADZONE) {
				updatePos = true;
			}
			if (updatePos) {
				newCarPos = (oldCarPos * exp(-LAMBDA_MOVEMENT * deltaT)) + carPosition * (1 - exp(-LAMBDA_MOVEMENT * deltaT));
				oldCarPos = newCarPos;
				if (glm::length((carPosition - oldCarPos)) < (DEADZONE / 50.0f)) {
					updatePos = false;
				}
			}
			// damping implementation of the vertical tire angle: (m.x * TIRE_MAX_ANGLE) is the one the variable is damping towards
			tireAngle = (tireAngle * exp(-LAMBDA_ANGLE * deltaT)) + (m.x * TIRE_MAX_ANGLE) * (1 - exp(-LAMBDA_ANGLE * deltaT));

			// update the horizontal tire angle rotation
			tireRotation -= carMoveSpeed * TIRE_ROTATION_SPEED;
			// limiting the rotation between 0 and -2 * M_PI
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

			// Compute the new position if not transitioning
			if (!transition) {
				// update the heli position and inclination only if it is high enough from the ground
				if (heliPosition.y >= 2 * HELI_GROUND) {

					// If the player is not pressing any button, the heli loses velocity till it stops moving (horizontal movement)
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

					// If the player is moving, reset the position to center (force the angle damping)
					if (m.z != 0 || m.x != 0)
						heliReset = true;

					// Maximum velocity and diagonal velocity
					if (glm::length(heliMoveSpeed) > HELI_MAX_SPEED) {
						heliMoveSpeed = HELI_MAX_SPEED * glm::normalize(heliMoveSpeed);
					}

					// updating heli's position
					heliPosition += ux * heliMoveSpeed.x * deltaT;
					heliPosition += uz * heliMoveSpeed.z * deltaT;

					// updating heli inclination
					heliRoll -= m.z * 0.5f * deltaT;
					heliPitch -= (-m.x) * 0.5f * deltaT;

					// updating heli rotation - deadzone implementation for the yaw
					if (heliReset || abs(tempHeliYaw - (camYaw + glm::radians(90.0f))) > HELI_DAMP_ANGLE) {
						// This is to avoid the spinning if you rotate by an angle > 180 degrees while on the ground
						if (camYaw - heliYaw + glm::radians(90.0f) > glm::radians(180.0f)) {
							camYaw -= 2 * M_PI;
							camYawOld -= 2 * M_PI;
						}
						else if (camYaw - heliYaw + glm::radians(90.0f) < -glm::radians(180.0f)) {
							camYaw += 2 * M_PI;
							camYawOld += 2 * M_PI;
						}
						// Damping implementation of the yaw
						tempHeliYaw = (heliYaw * exp(-HELI_DAMP_SPEED * deltaT)) +
							(camYaw + glm::radians(90.0f)) * (1 - exp(-HELI_DAMP_SPEED * deltaT));
						heliYaw = tempHeliYaw;
						heliReset = true;
						// If the angle is lower than a certain amount, activate the deadzone again
						if (abs(tempHeliYaw - (camYaw + glm::radians(90.0f))) < glm::radians(0.1f))
							heliReset = false;
					}

				}

				// If the player is not pressing any button, the heli loses velocity till it stops moving (vertical movement)
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

				// Updating heliMoveSpeed velocity
				heliMoveSpeed.y += m.y * HELI_VERT_ACC_SPEED * deltaT;
				if (heliMoveSpeed.y > HELI_VERT_MAX_SPEED) heliMoveSpeed.y = HELI_VERT_MAX_SPEED;

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
				// Needed while transitioning from walk to tank to avoid problem with the pitch
				SetPitches(&camPitch, &camPitchOld, &camPitchNew, vehicleStandardPitch);
			}

			// deadzone implementation
			if (glm::length((heliPosition - oldHeliPos)) > DEADZONE) {
				updatePos = true;
			}
			if (updatePos) {
				newHeliPos = (oldHeliPos * exp(-LAMBDA_MOVEMENT * deltaT)) + heliPosition * (1 - exp(-LAMBDA_MOVEMENT * deltaT));
				oldHeliPos = newHeliPos;
				if (glm::length((heliPosition - oldHeliPos)) < (DEADZONE / 50.0f)) {
					updatePos = false;
				}
			}

			// update the rotation of both blades - the top one spin faster and slower if the player is going up or down
			if (m.y) heliActualBladeSpeed = (1 + (m.y / 4.0f)) * HELI_BLADE_SPEED;
			else heliActualBladeSpeed = HELI_BLADE_SPEED;
			heliTopBladeYaw += heliActualBladeSpeed * deltaT;
			if (heliTopBladeYaw > 2 * M_PI) heliTopBladeYaw -= 2 * M_PI;

			heliBackBladeRoll += HELI_BLADE_SPEED * deltaT;
			if (heliBackBladeRoll > 2 * M_PI) heliBackBladeRoll -= 2 * M_PI;

			break;
		}

		// UPDATE ALL WORLD MATRICES

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

		// Back Left Tire Matrix
		glm::vec3 backLeftTirePosition = carPosition + BACK_LEFT_TIRE_OFFSET;
		tempWorld =
			glm::translate(glm::mat4(1), backLeftTirePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw - glm::radians(45.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.375f));

		glm::mat4 RyC = glm::rotate(glm::mat4(1), carYaw - glm::radians(45.0f), glm::vec3(0, 1, 0));
		glm::mat4 TPosOffsetsC = glm::translate(glm::mat4(1), backLeftTirePosition);
		glm::mat4 TPosC = glm::translate(glm::mat4(1), carPosition);
		glm::mat4 TOffsetsC = glm::translate(glm::mat4(1), BACK_LEFT_TIRE_OFFSET);
		glm::mat4 applyRotation = glm::rotate(glm::mat4(1), tireRotation, glm::vec3(1, 0, 0));
		glm::mat4 applyAngle = glm::rotate(glm::mat4(1), -tireAngle, glm::vec3(0, 1, 0));

		tempWorld =
			TPosC *
			RyC *
			TOffsetsC *
			applyRotation *
			glm::inverse(RyC) *
			glm::inverse(TPosOffsetsC) *
			tempWorld;

		WorldBackLeftTire = tempWorld;

		// Back Left Tire World Matrix
		glm::vec3 backRightTirePosition = carPosition + BACK_RIGHT_TIRE_OFFSET;
		tempWorld =
			glm::translate(glm::mat4(1), backRightTirePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw - glm::radians(45.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.375f));

		// Transformation matrices needed to perform the correct series of operation to rotate the wheel
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

		// Front Left Tire World Matrix
		glm::vec3 frontLeftTirePosition = carPosition + LEFT_TIRE_OFFSET;
		tempWorld =
			glm::translate(glm::mat4(1), frontLeftTirePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw - glm::radians(45.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.375f));

		// Transformation matrices needed to perform the correct series of operation to rotate the wheel in both directions
		RyC = glm::rotate(glm::mat4(1), carYaw - glm::radians(45.0f), glm::vec3(0, 1, 0));
		TPosOffsetsC = glm::translate(glm::mat4(1), frontLeftTirePosition);
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

		// Front Right Tire World Matrix
		glm::vec3 frontRightTirePosition = carPosition + RIGHT_TIRE_OFFSET;
		tempWorld =
			glm::translate(glm::mat4(1), frontRightTirePosition) *
			glm::mat4(glm::quat(glm::vec3(0.0f, carYaw - glm::radians(45.0f), 0.0f))) *
			glm::scale(glm::mat4(1), glm::vec3(0.375f));

		// Transformation matrices needed to perform the correct series of operation to rotate the wheel in both directions
		RyC = glm::rotate(glm::mat4(1), carYaw - glm::radians(45.0f), glm::vec3(0, 1, 0));
		TPosOffsetsC = glm::translate(glm::mat4(1), frontRightTirePosition);
		TPosC = glm::translate(glm::mat4(1), carPosition);
		TOffsetsC = glm::translate(glm::mat4(1), RIGHT_TIRE_OFFSET);
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

		WorldRightTire = tempWorld;



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

		// Transformation matrices needed to perform the correct series of operation to rotate the top blades
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

		// Transformation matrices needed to perform the correct series of operation to rotate the back blades
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

			// If the player gets down from the tank
			if (actionCheck && !transition && action) {
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

			// If the player gets down from the car
			if (actionCheck && !transition && action) {
				transition = true;
				gameState = GameState::WALK;
				camPitch = 0.0f;
				playerPosition = carPosition;
				playerPosition.y = PLAYER_HEIGHT;
				playerPosition.x += 2.2f * sin(carYaw + glm::radians(65.0f));
				playerPosition.z += 2.2f * cos(carYaw + glm::radians(65.0f));
				carMoveSpeed = 0.0f;
			}

			break;
		case HELI:
			ViewPrj = MakeViewPrjHeli(camPos);

			// If the player gets down from the heli
			if (actionCheck && !transition && action) {
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
		}

		// If a transition (i.e. interaction with a vehicle) is going on, it damps to the new ViewPrj
		if (transition) {
			transitionTimer += deltaT;
			ViewPrj = (oldViewPrj * exp(-LAMBDA_TRANSITION * deltaT)) + ViewPrj * (1 - exp(-LAMBDA_TRANSITION * deltaT));
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
			case TANK:
				posClosest = tankPosition;
				break;
			case CAR:
				posClosest = carPosition;
				break;
			case HELI:
				posClosest = heliPosition;
				break;
			}
			// Save the distance from the closest object
			float closestDistance = glm::length((playerPosition - posClosest));

			// If i'm not transitioning and the closest vehicle is at a certain range, check for an action
			if (!transition && closestDistance < INTERACTION_RANGE && action) {

				playerVerticalVelocity = 0.0f;
				actionCheck = false;
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
					newHeliPos = heliPosition;
					heliMoveSpeed.y = 0.0f;
					break;
				}
			}
		}

		// actionCheck tracks the state of the fire button: it ignore new presses if the button is held down
		if (!action)
			actionCheck = true;

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