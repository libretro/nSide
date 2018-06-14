#version 150

uniform sampler2D source[];

in Vertex {
  vec2 texCoord;
};

out vec4 fragColor;

void main() {
  vec4 rgba = texture(source[0], texCoord);
  fragColor = rgba;
}
