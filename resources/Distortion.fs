#version 440
layout( location = 0 ) uniform sampler2D Texture0;
layout( location = 3 ) uniform vec2 offset;

out vec4 FragColor;

vec2 texelSize = 1.0 / vec2(textureSize(Texture0, 0));
vec2 screen_coords = (gl_FragCoord.xy - offset) * texelSize; 
// correct for viewport offset
// Texture0 does not have any offset!

// PASSTHROUGH
// void main(){
//    FragColor = texture2D(Texture0, screen_coords);
// }

vec2 lens_centre     = vec2(0.5);
vec2 screen_centre   = vec2(0.5);
vec2 scale           = vec2(0.5);
vec4 warp_params = vec4(1, 0.4, 0.2, 0);


vec2 barrel_warp(vec2 screen_coord){
    vec2  relative_pos = screen_coord - lens_centre;
    float r            = length( relative_pos );
    vec2  warped_pos  = 
	relative_pos * warp_params.x +
	relative_pos * warp_params.y * r +
	relative_pos * warp_params.z * r * r +
	relative_pos * warp_params.z * r * r * r;
	
    return lens_centre + (warped_pos * scale);
}
 
void main(){
    vec2 coord = barrel_warp(screen_coords);

    clamp(coord, vec2(0), vec2(1));

    if (screen_coords.x > 0.5)
	FragColor = vec4(0.4, 0, 0, 0.5);
    else
	FragColor = vec4(0, 0.4, 0, 0.5);

    FragColor += texture2D(Texture0, coord);

}
