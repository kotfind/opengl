#version 330 core

out vec4 fragColor;

in vec3 myColor;
in vec2 texCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    fragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.5) * vec4(myColor, 1.);
}
