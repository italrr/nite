uniform sampler2D image;
uniform vec2 p_size;
uniform float p_alpha;
uniform vec3 p_cshade;
uniform vec3 p_ctarget;

void main() {
	vec4 tex = texture2D(image, gl_TexCoord[0].st);
	vec2 tc = gl_TexCoord[0].xy;
	float alpha = 1.0 * p_alpha;


	gl_FragColor = vec4(p_cshade.rgb, tex.a * alpha);

}

