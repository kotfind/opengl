#version 330 core

out vec4 fragColor;

void main() {
    vec3 color = vec3(1., 0., 0.);
    color = pow(color, vec3(0.4545));

    fragColor = vec4(color, 1.);
}
