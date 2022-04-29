// Author:   Benjamin Pickhardt
// Project:  LinWarrior 3d
// Home:     hackcraft.de

#version 120

varying vec3 normal, position, lightdir;
varying float direction;

void main()
{
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = gl_Vertex; //(gl_ProjectionMatrix * (gl_Vertex + vec4(0,-0.55, -1.0, 0.0)));
	//gl_Position = ftransform();
	//position = vec3(gl_ModelViewMatrix * gl_Vertex);
	position = (gl_Vertex + vec4(0,-0.55, -1.0, 0.0)).xyz;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	vec3 lightdir = normalize(vec3(gl_LightSource[0].position.xyz - position));
	direction = max(dot(lightdir,normalize(normal)),0.0);
}


