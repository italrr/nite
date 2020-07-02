// uniform sampler2D image;
// uniform vec2 size;
// uniform vec3 color;
// uniform float alpha;

// void main() {
//   vec2 _size = size + vec2(0.5);
//   vec2 tc = gl_TexCoord[0].xy;
//   float vig = tc.x * tc.y * (1.0 - tc.x) * (1.0 - tc.y);
//   const float v = pow(.2, 4.5);
//   gl_FragColor = vec4(color.rgb, vig < v ? alpha * (vig / v) : alpha);
// }


uniform sampler2D image;
uniform vec2 size;
uniform vec3 color;
uniform float alpha;
uniform float thickness;

void main() {
    vec2 tc = gl_TexCoord[0].xy;
    float vig = tc.x * tc.y * (1.0 - tc.x) * (1.0 - tc.y);
    const float v = pow(.2, 4.5);
    float _alpha = alpha;
    float vOff = size.x / size.y;
    float area = ((size.x + size.y) * 0.5);
    float thresW = (area * 0.01 * thickness) / area;
    float thresH = thresW * vOff;
    if(tc.x >= (1.0 - thresW)){
        _alpha *= (1.0 - (tc.x - (1.0 - thresW)) / thresW);
    }
    if(tc.x <= thresW){
        _alpha *= tc.x / thresW;
    }
    if(tc.y >= (1.0 - thresH)){
        _alpha *= (1.0 - (tc.y - (1.0 - thresH)) / thresH);
    } 
    if(tc.y <= thresH){
        _alpha *= tc.y / thresH;
    }      
    if(tc.x <= (1.0 - thresW) && tc.x >= thresW && tc.y <= (1.0 - thresH) && tc.y >= thresH){
        _alpha = 0.0;
    }
    gl_FragColor = vec4(color.rgb, _alpha);
}


