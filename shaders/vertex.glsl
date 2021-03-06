#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec3 texCoord;
layout(location = 4) in float drawEdge;

// varying variables
out vec4 vColor;
out vec3 vTexCoord;
out vec3 vNormal;
out vec3 vPosition;
out float vDrawEdge;

// uniform variables
uniform int mode;	// 1 -- color / 2 -- texture
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;

uniform int depthComputation;	// 1 -- depth computation / 0 -- otherwise
uniform mat4 light_mvpMatrix;
uniform vec3 lightDir;

void main(){
	vColor = color;
	vTexCoord = texCoord;
	vPosition = position;
	vDrawEdge = drawEdge;

	// SHADOW: From light
	if (depthComputation == 1) {
		gl_Position = light_mvpMatrix * vec4(position, 1.0);
		return;
	}
	
	//varNormal = normalize(normal).xyz;
	vNormal = normalize(normal).xyz;

	gl_Position = mvpMatrix * vec4(position, 1.0);
}