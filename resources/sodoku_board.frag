#version 430
out vec4 fragColor;

in vec2 texCoords;

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
      fragColor = vec4(1,0,0,1);
    } else {
      fragColor = _col;
    }
  }
  //fragColor = vec4(float(little_cell_index), big_cell_index.y/float(big_cell_width), 0, 1);
}
// || (lb.y && ub.y) (lb.x && ub.x )