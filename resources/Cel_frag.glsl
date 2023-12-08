#version 120

uniform vec3 lightPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec4 cPos; // vert in camera space
varying vec3 cNor; // vert norm in camera space

void main(){
	vec3 n = normalize(cNor);
	vec3 cPos3 = vec3(cPos[0],cPos[1],cPos[2]);
	
	vec3 lightCol = vec3(1.0f,1.0f,1.0f);

	//cD
	vec3 l = lightPos - normalize(cPos3);
	vec3 lnorm = normalize(l);
	vec3 cD = kd * max(dot(lnorm,n),0.0);

	//cS
	vec3 h_norm = normalize(l - cPos3);
	vec3 cS = ks * pow(max(0, dot(n,h_norm) ), s);


	//Color
	vec3 color = vec3(0.0f,0.0f,0.0f);
	
	//red
	color.r = lightCol.r * (ka.r + cD.r + cS.r);

	if(color.r < 0.2) {
		color.r = 0.0;
	} else if(color.r < 0.45) {
		color.r = 0.25;
	} else if(color.r < 0.65) {
		color.r = 0.5;
	} else if(color.r < 1.0) {
		color.r = 0.75;
	} else {
		color.r = 1.0;
	}

	//green
	color.g = lightCol.g * (ka.g + cD.g + cS.g);

	if(color.g < 0.2) {
		color.g = 0.0;
	} else if(color.g < 0.45) {
		color.g = 0.25;
	} else if(color.g < 0.65) {
		color.g = 0.5;
	} else if(color.g < 1.0) {
		color.g = 0.75;
	} else {
		color.g = 1.0;
	}

	//blue
	color.b = lightCol.b * (ka.b + cD.b + cS.b);

	if(color.b < 0.2) {
		color.b = 0.0;
	} else if(color.b < 0.45) {
		color.b = 0.25;
	} else if(color.b < 0.65) {
		color.b = 0.5;
	} else if(color.b < 1.0) {
		color.b = 0.75;
	} else {
		color.b = 1.0;
	}
	
	gl_FragColor = vec4(color.r, color.g, color.b, 1.0);
}
