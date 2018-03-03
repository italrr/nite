uniform sampler2D image;
uniform vec2 p_size;
uniform float p_alpha;
uniform vec3 p_bcolor;
uniform vec3 p_lbcolor;
uniform float p_salpha;
uniform vec2 p_hborder;
uniform vec2 p_vborder;
uniform float p_blurthickness;

float rand(vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
  vec2 tc = gl_TexCoord[0].xy;
  vec3 texel = texture2D(image, gl_TexCoord[0].xy).rgb;
  vec3 color = p_bcolor;
  float alpha = 1.0;
  float _alpha = p_salpha;
  float vOff = p_size.x / p_size.y;
  float area = ((p_size.x + p_size.y) * 0.5);
  float thresW = (area * 0.01 * p_blurthickness) / area;
  float thresH = thresW * vOff;

  //|| tc.x * p_size.x > p_size.x * 0.98
  if(tc.y * p_size.y < p_vborder.x || tc.y * p_size.y > p_size.y - p_vborder.y || tc.x * p_size.x < p_hborder.x || tc.x * p_size.x > p_size.x - p_hborder.y){
    color = p_lbcolor;
  }else{
    alpha *= p_salpha;
  }

  // if(tc.x >= (1.0 - thresW)){
  //   _alpha = 0.0;
  // }
  // if(tc.x <= thresW){
  //   _alpha *= 0.0;
  // } 
  // if(tc.y >= (1.0 - thresH)){
  //   _alpha *= 0.0;
  // } 
  // if(tc.y <= thresH){
  //   _alpha *= 0.0;
  // }   

  gl_FragColor = vec4(color.xyz, alpha * p_alpha * _alpha);
}