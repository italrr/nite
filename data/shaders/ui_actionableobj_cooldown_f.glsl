uniform sampler2D image;
uniform float p_total;
uniform vec2 p_size;
uniform float p_alpha;
uniform vec3 p_color; 

void main() {
	vec4 tex = texture2D(image, gl_TexCoord[0].st);
	vec2 tc = gl_TexCoord[0].xy;
	float alpha = 1.0 * p_alpha;
	if(tc.x * p_size.x > p_total * p_size.x){
		gl_FragColor = vec4(tex.x, tex.y, tex.z, alpha);
		gl_FragColor.r = dot(tex, vec3(.272, .272, .272));
		gl_FragColor.g = dot(tex, vec3(.272, .272, .272));
		gl_FragColor.b = dot(tex, vec3(.272, .272, .272));
	}else{
		gl_FragColor = vec4(tex.rgb, alpha);
	}	
}
