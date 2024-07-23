#version 330 core

in vec2 TexCoords;

uniform sampler2D image;
uniform vec3 spriteColor;

void main(){
    gl_FragColor = vec4(spriteColor, 1.0) * texture(image, TexCoords);
}
