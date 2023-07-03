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

// The vertices data structures
struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct VertexPlain {
	glm::vec3 pos;
	glm::vec2 UV;
};

/* A16 */
/* Add the C++ datastructure for the required vertex format */
struct VertexMonoColor {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

// MAIN ! 
class A16 : public BaseProject {
protected:

	// Current aspect ratio (used by the callback that resized the window)
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLGubo;
	/* A16 */
	/* Add the variable that will contain the required Descriptor Set Layout */
	DescriptorSetLayout DSLMonoColor;
	DescriptorSetLayout DSLVertexPlain;

	// Vertex formats

	/* A16 */
	/* Add the variable that will contain the required Vertex format definition */
	VertexDescriptor VMonoColor;
	VertexDescriptor VVertexPlain;

	// Pipelines [Shader couples]
	/* A16 */
	/* Add the variable that will contain the new pipeline */
	Pipeline PMonoColor;
	Pipeline PVertexPlain;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	/* A16 */
	/* Add the variable that will contain the model for the room */
	Model<VertexMonoColor> MTank;
	Model<VertexPlain> MFloor;

	DescriptorSet DSGubo;
	/* A16 */
	/* Add the variable that will contain the Descriptor Set for the room */
	DescriptorSet DSTank;
	DescriptorSet DSFloor;

	Texture TTank;
	Texture TFloor;

	// C++ storage for uniform variables
	/* A16 */
	/* Add the variable that will contain the Uniform Block in slot 0, set 1 of the room */
	MeshUniformBlock uboTank;
	PlainUniformBlock uboFloor;

	GlobalUniformBlock gubo;

	// Other application parameters
	float CamH, CamRadius, CamPitch, CamYaw;
	int gameState;
	float HandleRot = 0.0;
	float Wheel1Rot = 0.0;
	float Wheel2Rot = 0.0;
	float Wheel3Rot = 0.0;


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Enhanced RiSiKo!";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.0f, 0.005f, 0.01f, 1.0f };

		// Descriptor pool sizes
		/* A16 */
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
		/* A16 */
		/* Init the new Data Set Layout */
		DSLVertexPlain.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLMonoColor.init(this, { 
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		// Vertex descriptors

		/* A16 */
		/* Define the new Vertex Format */
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

		VVertexPlain.init(this, {
			{0, sizeof(VertexPlain), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPlain, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPlain, UV),
					   sizeof(glm::vec2), UV}
			});

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		/* A16 */
		/* Create the new pipeline, using shaders "VColorVert.spv" and "VColorFrag.spv" */
		PMonoColor.init(this, &VMonoColor, "shaders/Tank/MonoColorVert.spv", "shaders/Tank/MonoColorFrag.spv", { &DSLGubo, &DSLMonoColor });
		PVertexPlain.init(this, &VVertexPlain, "shaders/Floor/FloorVert.spv", "shaders/Floor/FloorFrag.spv", { &DSLGubo, &DSLVertexPlain });
		PVertexPlain.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);
		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		/* A16 */
		/* load the mesh for the room, contained in OBJ file "Room.obj" */
		MTank.init(this, &VMonoColor, "Models/Tank.obj", OBJ);
		MFloor.vertices = 
			//		POS			   UV
			{ { {-50.0f, 25.0f, 0} , {1,1} } , 
			  { { 50.0f, 25.0f, 0} , {1,0} } , 			
			  { {-50.0f,-25.0f, 0} , {0,1} } ,
			  { { 50.0f,-25.0f, 0} , {0,0} } };
		MFloor.indices = 
			{ 0, 1, 2, 
			  1, 2, 3 };
		MFloor.initMesh(this, &VVertexPlain);

		// Create the textures
		// The second parameter is the file name
		TTank.init(this, "textures/Red.png");
		TFloor.init(this, "textures/RisikoMap.png");

		// Init local variables
		// TODO UPDATE THOSE VARIABLES WITH THE STUFF IN GAME LOGIC
		CamH = 1.0f;
		CamRadius = 3.0f;
		CamPitch = glm::radians(15.f);
		CamYaw = glm::radians(30.f);
		gameState = 0;
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		/* A16 */
		/* Create the new pipeline */
		PMonoColor.create();
		PVertexPlain.create();
		// Here you define the data set

		/* A16 */
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
		printf("\n\n ZIODIAOSDIASD \n\n");
		DSFloor.init(this, &DSLVertexPlain, {
					{0, UNIFORM, sizeof(PlainUniformBlock), nullptr},
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
		/* A16 */
		/* cleanup the new pipeline */
		PMonoColor.cleanup();
		PVertexPlain.cleanup();
		// Cleanup datasets
		/* A16 */
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
		/* A16 */
		/* Cleanup the mesh for the room */
		MTank.cleanup();
		// Cleanup descriptor set layouts
		/* A16 */
		/* Cleanup the new Descriptor Set Layout */
		DSLMonoColor.cleanup();
		DSLVertexPlain.cleanup();
		DSLGubo.cleanup();

		/* A16 */
		/* Destroy the new pipeline */
		PMonoColor.destroy();
		PVertexPlain.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		/* A16 */
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
		PVertexPlain.bind(commandBuffer);
		// binds the gubo to the command buffer to our new pipeline and set 0
		DSGubo.bind(commandBuffer, PVertexPlain, 0, currentImage);
		// binds the mesh
		MFloor.bind(commandBuffer);
		// binds the descriptor set layout
		DSFloor.bind(commandBuffer, PVertexPlain, 1, currentImage);
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

		// LOGIC OF THE APPLICATION
		//TODO implement first person view
		const float FOVy = glm::radians(45.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.f;

		// Player starting point
		const glm::vec3 StartingPosition = glm::vec3(0.0, 0.0, 0.0);

		// Camera target height and distance
		const float camHeight = 2.5f;
		const float camDist = 9.0f;
		// Camera Pitch limits
		const float minPitch = glm::radians(-8.75f);
		const float maxPitch = glm::radians(60.0f);
		// Rotation and motion speed
		const float ROT_SPEED = glm::radians(120.0f);
		const float MOVE_SPEED = 2.0f;


		// Game Logic implementation
		// Current Player Position - static variables make sure that their value remain unchanged in subsequent calls to the procedure
		static glm::vec3 playerPosition = StartingPosition, oldPos = StartingPosition, newPos = StartingPosition, newPos2 = StartingPosition;

		// To be done in the assignment
		glm::mat4 ViewMatrix, ProjectionMatrix, WorldMatrix;

		const float LAMBDAROT = 20.0f,
			LAMBDAMOV = 10.0f,
			DEADZONE = 0.2f;

		static bool updatePos = false;

		static float yaw = 0.0f, pitch = 0.0f, roll = 0.0f,
			yawOld = 0.0f, pitchOld = 0.0f,
			yawNew = 0.0f, pitchNew = 0.0f,
			playerYaw = 0.0f, playerYawOld = 0.0;

		// computing angles
		pitch -= ROT_SPEED * r.x * deltaT;
		pitch = pitch < minPitch ? minPitch :
			(pitch > maxPitch ? maxPitch : pitch);
		yaw -= ROT_SPEED * r.y * deltaT;
		roll += ROT_SPEED * r.z * deltaT;

		// computing the versors and updating the player position
		glm::vec3 ux = glm::vec3(glm::rotate(glm::mat4(1),
			yaw, glm::vec3(0, 1, 0)) *
			glm::vec4(1, 0, 0, 1));
		glm::vec3 uy = glm::vec3(0, 1, 0);
		glm::vec3 uz = glm::vec3(glm::rotate(glm::mat4(1),
			yaw, glm::vec3(0, 1, 0)) *
			glm::vec4(0, 0, -1, 1));
		playerPosition += ux * MOVE_SPEED * m.x * deltaT;
		playerPosition += uz * MOVE_SPEED * m.z * deltaT;
		// blocking the player from going under the terrain
		if ((playerPosition + uy * MOVE_SPEED * m.y * deltaT).y < 0.0f) {
			playerPosition.y = 0.0f;
		}
		else
		{
			playerPosition += uy * MOVE_SPEED * m.y * deltaT;
		}


		// damping implementation
		pitchNew = (pitchOld * exp(-LAMBDAROT * deltaT)) + pitch * (1 - exp(-LAMBDAROT * deltaT));
		pitchOld = pitchNew;

		yawNew = (yawOld * exp(-LAMBDAROT * deltaT)) + yaw * (1 - exp(-LAMBDAROT * deltaT));
		yawOld = yawNew;

		// deadzone implementation
		if (glm::length((playerPosition - oldPos)) > DEADZONE) {
			updatePos = true;
		}

		if (updatePos) {
			newPos = (oldPos * exp(-LAMBDAMOV * deltaT)) + playerPosition * (1 - exp(-LAMBDAMOV * deltaT));
			oldPos = newPos;
			if (oldPos == playerPosition) {
				updatePos = false;
			}
		}

		// player rotating independently from the camera
		//playerYaw = yaw + (atan2(m.z, m.x) - glm::radians(90.0f));
		if (m.x != 0 || m.z != 0)
			playerYaw = yaw + (atan2(m.z, m.x));
		playerYawOld = playerYaw;

		// World Matrix
		WorldMatrix =
			glm::translate(glm::mat4(1), playerPosition) *
			glm::mat4(glm::quat(glm::vec3(0, playerYaw, 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1));
		glm::mat4 World = WorldMatrix;

		// World Matrix used for the camera
		WorldMatrix = glm::translate(glm::mat4(1), oldPos) *
			glm::mat4(glm::quat(glm::vec3(0, yawNew, 0))) *
			glm::scale(glm::mat4(1), glm::vec3(1));

		// calculating the View-Projection Matrix
		glm::vec3 cameraPosition;
		glm::vec4 temp = WorldMatrix * glm::vec4(0, camHeight + (camDist * sin(pitchNew)), camDist * cos(pitchNew), 1);
		cameraPosition = glm::vec3(temp.x, temp.y, temp.z);

		glm::vec3 targetPointedPosition;
		targetPointedPosition = glm::vec3(newPos.x, newPos.y + camHeight, newPos.z);

		ViewMatrix = glm::lookAt(cameraPosition, targetPointedPosition, glm::vec3(0, 1, 0));
		ViewMatrix = glm::rotate(glm::mat4(1), roll, glm::vec3(0, 0, 1)) * ViewMatrix;

		ProjectionMatrix = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		ProjectionMatrix[1][1] *= -1;

		glm::mat4 ViewPrj = ProjectionMatrix * ViewMatrix;
		

		// Parameters
		// Camera FOV-y, Near Plane and Far Plane
		//const float FOVy = glm::radians(90.0f);
		//const float nearPlane = 0.1f;
		//const float farPlane = 100.0f;
		const float rotSpeed = glm::radians(90.0f);
		const float movSpeed = 1.0f;

		CamH += m.z * movSpeed * deltaT;
		CamRadius -= m.x * movSpeed * deltaT;
		CamPitch -= r.x * rotSpeed * deltaT;
		CamYaw += r.y * rotSpeed * deltaT;

		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;
		glm::vec3 camTarget = glm::vec3(0, CamH, 0);
		glm::vec3 camPos = camTarget +
			CamRadius * glm::vec3(cos(CamPitch) * sin(CamYaw),
				sin(CamPitch),
				cos(CamPitch) * cos(CamYaw));
		glm::mat4 View = glm::lookAt(camPos, camTarget, glm::vec3(0, 1, 0));

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

		
		/* A16 */
		/* fill the uniform block for the room. Identical to the one of the body of the slot machine */
		// THE TANK IS THE PLAYER RN, BCS I USE THE WORLD AND VIEWPROJ MATRIX CALCULATED FOR THE PLAYER FOR ITS COORDINATES.
		uboTank.amb = 1.0f; uboTank.gamma = 180.0f; uboTank.sColor = glm::vec3(1.0f);
		uboTank.mvpMat = ViewPrj * World;
		uboTank.mMat = World;
		uboTank.nMat = glm::inverse(glm::transpose(World));
		/* map the uniform data block to the GPU */
		DSTank.map(currentImage, &uboTank, sizeof(uboTank), 0);

		uboFloor.visible = 1;
		DSFloor.map(currentImage, &uboFloor, sizeof(uboFloor), 0);

	}
};


// This is the main: probably you do not need to touch this!
int main() {
	A16 app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}