#version 440
layout( location = 0 ) uniform sampler2D Texture0;
layout( location = 3 ) uniform vec2 offset; // tells us L/R render

out vec4 FragColor;

vec2 screenSize = vec2(textureSize(Texture0, 0));

// PASSTHROUGH
// void main(){
//     FragColor = texture2D(Texture0, gl_FragCoord.xy / screenSize);
// }

void main(){

    vec2 renderSize = screenSize * vec2(0.5, 1); // size of drawing region

    vec2 pixel_pos = gl_FragCoord.xy;
    if (offset.x == 0) pixel_pos.x -= 48; // hacky image shift to align ...
    if (offset.x != 0) pixel_pos.x += 48; // ... image with lens position

    vec2 viewport_position = (pixel_pos.xy - offset) / renderSize;

    vec2 scale           = vec2(1);                // TODO: use scale
    vec4 warp_params     = vec4(1, 0.22, 0.24, 0); // TODO: get from Rift

    vec2  relative_pos = 2.0 * viewport_position - 1.0; // now in range -1:1
    float r            = length( relative_pos * vec2(1,1.25)); // aspect ratio 1.25 
    float r_sq         = r * r; 
    vec2  warped_pos   = relative_pos * (
	warp_params.x +
	warp_params.y * r_sq +
	warp_params.z * r_sq * r_sq +
	warp_params.w * r_sq * r_sq * r_sq);

    vec2 coord  = (warped_pos + 1.0) / 2.0; // relative to image

    // scale to screen, if right eye then also x += 0.5
    coord = coord / vec2(2, 1) + (offset.x == 0 ? vec2(0) : vec2(0.5, 0));
    // clamp(coord, vec2(0), vec2(1)); // caused artefacts

    FragColor = vec4(0);

    // guard against 'out of scope'
    if ((coord.x < 0) || (coord.x > 1) || (coord.y < 0) || (coord.y > 1)) return;
    if ((offset.x == 0) && (coord.x > 0.5)) return;
    if ((offset.x != 0) && (coord.x < 0.5)) return;
    
    FragColor += texture2D(Texture0, coord);
}

// Debugging snippets:

// // Highlight 'out of texture' regions
// if ((coord.x < 0) || (coord.x > 1) || (coord.y < 0) || (coord.y > 1))
//     FragColor += vec4(1,0,0,.5);//coord = vec2(0);
// if ((offset.x == 0) && (coord.x > 0.5))
//     FragColor += vec4(0,1,0,.5);//coord = vec2(0);
// if ((offset.x != 0) && (coord.x < 0.5))
//     FragColor += vec4(0,0,1,.5);//coord = vec2(0);

// // Highlight the 'viewport_position' midpoint
// if (viewport_position.x > 0) FragColor = vec4(1, 0, 0, 0.5);

// // Highlight the 'relative_position' midpoint (x, y)
// if(relative_pos.x > 0 ) FragColor = vec4(1,0,0,1);
// if(relative_pos.y > 0 ) FragColor = vec4(0,1,0,1);

// // Check that the spherical deform is round-ish
// if (r > 0.3) FragColor = vec4(0,0,1,1);
