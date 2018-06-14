#version 150

uniform sampler2D source[];

in Vertex {
  vec2 texCoord;
};

out vec4 fragColor;

void main() {
  vec4 rgbs = texture(source[0], texCoord);
  if(rgbs[3] < 1.0) { // Game display
    fragColor = rgbs;
  } else {            // Menu/instructions display
    fragColor = vec4(rgbs[0], rgbs[1], rgbs[2], 0.0);
  }
}
