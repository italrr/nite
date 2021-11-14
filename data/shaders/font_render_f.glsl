#version 120
//in vec2 TexCoords;
//out vec4 color;

uniform sampler2D text;
uniform vec3 outline_col;
uniform vec3 fill_col;

void main(){  
    vec2 tc = gl_TexCoord[0].xy;
    // vec3 outline_col    = vec3(0.0, 0.0, 0.0); // e.g blue
    // vec3 fill_col = vec3(1.0, 0.0, 0.0); // e.g red

    vec2 tex = texture2D(text, tc).rg;
    float fill    = tex.g;
    float outline = tex.r;

    float alpha    = max( fill, outline );
    vec3 mix_color = mix( mix(vec3(0.0), outline_col, fill), fill_col, outline );

    gl_FragColor = vec4(mix_color, alpha);
}