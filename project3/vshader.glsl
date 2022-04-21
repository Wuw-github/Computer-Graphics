#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec2 vTexCoord;
attribute vec4 vNormal;

varying vec4 color;
varying vec2 texCoord;
varying vec4 N;
varying vec4 L;
varying vec4 V;
varying float distance;
varying float shadow;

uniform mat4 model_view_matrix; 
uniform mat4 projection_matrix;
uniform mat4 ctm;
uniform vec4 light_position;

void main()
{
	if(is_shadow == 0){
		gl_Position = projection_matrix * model_view_matrix * ctm * vPosition; 

	    texCoord = vTexCoord;
	    color = vColor;
	    
	    N = normalize(model_view_matrix * ctm * vNormal);
	    vec4 L_temp = model_view_matrix * (light_position - ctm * vPosition);
	    L = normalize(L_temp);
	    vec4 eye_point = vec4(0.0, 0.0, 0.0, 1.0);
	    V = normalize(eye_point - (model_view_matrix * ctm * vPosition));
	    distance = length(L_temp);
	}
	else
	{
		float xl = light_position.x;
		float yl = light_position.y;
		float zl = light_position.z;
		vec4 temp = ctm * vPosition;
		float xp = temp.x;
		float yp = temp.y;
		float zp = temp.z;

		float x = xl - yl * (xl - xp) / (yl - yp);
		float z = zl - yl * (zl - zp) / (yl - yp);
		gl_Position = projection_matrix * model_view_matrix * vec4(x, 0.001, z, 1);
		color = vec4(0, 0, 0, 1);
	}
}