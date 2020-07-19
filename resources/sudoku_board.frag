#version 430
out vec4 fragColor;

in vec2 texCoords;
uniform sampler2D _texture;

uniform vec4 _col;
uniform vec4 _backCol;
uniform vec4 _selCol;
uniform ivec2 _viewportDims;

uniform ivec2 board_dims;
uniform int thick_line_width;
uniform int thin_line_width;
uniform int cell_width;
uniform ivec2 selected_cell;
void main()
{
  // Calculate which pixel this fragment represents
  ivec2 pixel = ivec2(texCoords*board_dims);
  // Invert y-axis
  pixel.y = board_dims.y-pixel.y-1;
  
  int big_cell_width = (thick_line_width + (2 * thin_line_width) + (3 * cell_width));
  int little_cell_width = thin_line_width + cell_width;
  
  // Workout if we are line or background  
  ivec2 big_cell_index = pixel / big_cell_width;
  ivec2 big_cell_offset = pixel - (big_cell_index * big_cell_width) - thick_line_width;
  ivec2 little_cell_index = big_cell_offset / little_cell_width;
  ivec2 little_cell_offset = big_cell_offset - (little_cell_index * little_cell_width);
  
  bvec2 lb = greaterThanEqual(little_cell_offset, ivec2(0));
  bvec2 ub = lessThan(little_cell_offset, ivec2(cell_width));
  if ((!(lb.x && ub.x ) || !(lb.y && ub.y))) {
    // We are line
    fragColor = _backCol;
  } else {
    // We are cell
    ivec2 cell_index = (big_cell_index * 3) + little_cell_index;
    if (cell_index == selected_cell) {
      fragColor = _selCol;
    } else {
      fragColor = _col;
    }
    // Apply glyph tex to cell
    //TextureLod(0) so even if mipmaps gen, we try to use best quality
    //float tex = textureLod(_texture, vec2(texCoords.x,-texCoords.y),0.0f).r;
    //Grab a solid pixel, ensure no interpolation
    ivec2 texDim = textureSize(_texture, 0);
    vec2 tex = texelFetch(_texture, ivec2(int(texCoords.x*texDim.x),texDim.y-int(texCoords.y*texDim.y)),0).rg;
    float intensity = tex.r;
    float red = tex.g;
    if (red > 0) {
      // Manual alpha blend
      // (foregroundRed * foregroundAlpha) + (backgroundRed * (1.0 - foregroundAlpha))
      fragColor.r = red * intensity + (fragColor.r * (1-intensity));
      fragColor.gb = vec2(0 * (1-intensity) + (fragColor.gb * (1-intensity)));
    } else {
      fragColor.rgb *= 1 - intensity;
    }
    fragColor.a = 1;
  }
}