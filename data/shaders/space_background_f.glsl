uniform sampler2D image;

uniform vec3 baseColor;
uniform vec3 borderColor;
uniform vec3 titleColor;
uniform vec2 size;
uniform float border;
uniform int useTitle;
uniform float titleHeight;

void main() {
    gl_FragColor = texture2D(image, gl_TexCoord[0].xy);
    vec2 pp = gl_TexCoord[0].xy; // pixel position

    if( pp.x * size.x > size.x-border ||
        pp.x * size.x < border ||
        pp.y * size.y < border ||
        pp.y * size.y > size.y-border){
        gl_FragColor = vec4(borderColor, 1.0);
    }else{
        if(useTitle == 1){
            if(pp.y * size.y < titleHeight){
                gl_FragColor = vec4(titleColor, 1.0);
            }else
            if(pp.y * size.y < titleHeight + border){
                gl_FragColor = vec4(borderColor, 1.0);
            }else{
                gl_FragColor = vec4(baseColor, 1.0);    
            }
        }else{
            gl_FragColor = vec4(baseColor, 1.0);
        }                    
    }    
}