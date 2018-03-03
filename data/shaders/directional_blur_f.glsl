uniform sampler2D image;
uniform vec2 dir;
void main() {
	//vec4 tex = texture2D(image, gl_TexCoord[0].st);
	//gl_FragColor = vec4(tex.rgb/tex.a, tex.a);

	vec4 sum = vec4(0.0);
	vec2 tc = gl_TexCoord[0].xy;
	// float vx_offset = 0.5;
	// float blur = 1.0;
	// float hstep = 0.0006;
	// float vstep = 0.0006;
	// if (gl_TexCoord[0].x<(vx_offset))
	// {
	// 	hdir = 1.0;
	// }
	// else if (gl_TexCoord[0].x>=(vx_offset))
  // {
  //   hdir = 1.0;
  // }
	// sum += texture2D(image, vec2(tc.x - 4.0*blur*hstep, tc.y - 4.0*blur*vstep)) * 0.0162162162;
	// sum += texture2D(image, vec2(tc.x - 3.0*blur*hstep, tc.y - 3.0*blur*vstep)) * 0.0540540541;
	// sum += texture2D(image, vec2(tc.x - 2.0*blur*hstep, tc.y - 2.0*blur*vstep)) * 0.1216216216;
	// sum += texture2D(image, vec2(tc.x - 1.0*blur*hstep, tc.y - 1.0*blur*vstep)) * 0.1945945946;
	//
	// sum += texture2D(image, vec2(tc.x, tc.y)) * 0.2270270270;
	//
	// sum += texture2D(image, vec2(tc.x + 1.0*blur*hstep, tc.y + 1.0*blur*vstep)) * 0.1945945946;
	// sum += texture2D(image, vec2(tc.x + 2.0*blur*hstep, tc.y + 2.0*blur*vstep)) * 0.1216216216;
	// sum += texture2D(image, vec2(tc.x + 3.0*blur*hstep, tc.y + 3.0*blur*vstep)) * 0.0540540541;
	// sum += texture2D(image, vec2(tc.x + 4.0*blur*hstep, tc.y + 4.0*blur*vstep)) * 0.0162162162;

	//sum += texture2D(image, vec2(tc.x, tc.y)) * 0.1;

	float hdir = dir.x;
	float vdir = dir.y;
	float minoffset = 0.01;
	float q = 10.0;
	float minfrag = 1.0 / q;
	// if (gl_TexCoord[0].x<(vx_offset)){
	// 	hdir = minoffset * -1.0f;
	// }else
	// if (gl_TexCoord[0].x>=(vx_offset)){
  //   hdir = minoffset * 1.0f;
  // }

	for(float i = 0.0; i < q; ++i){
		float di = (i / q);
		sum += texture2D(image, vec2(tc.x + hdir * di, tc.y + vdir * di)) * minfrag;
	}
	vec4 original = texture2D(image, gl_TexCoord[0].st);
	gl_FragColor = vec4(sum.rgba);
}
