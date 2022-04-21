#version 120

varying vec2 texCoord;
varying vec4 color;
varying vec4 N, L, V;
varying float distance;

uniform sampler2D texture;
uniform int use_texture;

vec4 ambient, diffuse, specular;

void main()
{
	vec4 the_color = color;
	if (use_texture == 1){
		the_color = texture2D(texture, texCoord);
	}
	if(color.x == 0 && color.y == 0 && color.z == 0){
		gl_FragColor = color;
	}
	else{
		ambient = the_color * 0.3;
		vec4 H = normalize(L + V);
		float temp = max(dot(L, N), 0.0);
		diffuse = max(dot(L, N), 0.0) * 0.7 * the_color;
		specular = pow(max(dot(N, H), 0.0), 40) * vec4(1.0, 1.0, 1.0, 1.0);
		gl_FragColor = ambient + diffuse + specular;
	}
	
}