#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVit;

attribute vec4 aPos; // vert in object space
attribute vec3 aNor; // vert norm in object space
varying vec4 cPos; // vert in camera space
varying vec3 cNor; // vert norm in camera space

void main()
{
	gl_Position = P * MV * aPos;
	cPos = MV * aPos;
	vec4 cNor4 = MVit * vec4(aNor, 0.0);
	cNor = normalize( vec3(cNor4[0],cNor4[1],cNor4[2]) );
}
