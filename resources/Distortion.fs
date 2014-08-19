#version 440
layout( location = 0 ) uniform sampler2D Texture0;
layout( location = 3 ) uniform vec2 offset;

out vec4 FragColor;

vec2 screenSize = vec2(textureSize(Texture0, 0));
vec2 vanilla_pos= gl_FragCoord.xy / screenSize;

// PASSTHROUGH
// void main(){
//     FragColor = texture2D(Texture0, gl_FragCoord.xy / screenSize);
// }

//vec2 lens_centre = (0 == offset.x) ? vec2(0.425, 0.5) : vec2(0.575, 0.5);
//vec2 frag_offset = gl_FragCoord.xy 
// + ( (0 == offset.x) ? vec2(48, 0) : vec2(-48,0));

 
void main(){

    vec2 renderSize = screenSize * vec2(0.5, 1); // size of drawing region

    vec2 pixel_pos = gl_FragCoord.xy;

    if (offset.x == 0) pixel_pos.x -= 48; //hacky image shift
    if (offset.x != 0) pixel_pos.x += 48; //hacky image shift

    vec2 viewport_position = (pixel_pos.xy - offset) / renderSize;
    viewport_position = 2.0 * viewport_position - 1.0; // range now -1:1

    vec2 lens_centre     = vec2(0); //centre of viewport (to start with)
    vec2 screen_centre   = vec2(0);
    vec2 scale           = vec2(1);
    vec4 warp_params     = vec4(1, 0.22, 0.24, 0);

    vec2  relative_pos = viewport_position - lens_centre;
    float r            = length( relative_pos * vec2(1,1.25)); 
    float r_sq         = r * r; 
	relative_pos.x * relative_pos.x + relative_pos.y * relative_pos.y;
    //	length( relative_pos * vec2(1,1.25)); 
    // adjust for aspect ratio 800:640
    vec2  warped_pos   = relative_pos * (
	warp_params.x +
	warp_params.y * r_sq +
	warp_params.z * r_sq * r_sq +
	warp_params.w * r_sq * r_sq * r_sq);

    vec2 coord  = warped_pos + lens_centre;
    coord = (coord + 1.0) / 2.0;
    // scale, and also if right eye then x += 0.5
    coord = coord / vec2(2, 1) + (offset.x == 0 ? vec2(0) : vec2(0.5, 0));
    // clamp(coord, vec2(0), vec2(1)); // caused artefacts

    FragColor = vec4(0);

    if ((coord.x < 0) || (coord.x > 1) || (coord.y < 0) || (coord.y > 1)){
	return;
//	FragColor += vec4(1,0,0,.5);//coord = vec2(0);
    }
    // special cases
    if ((offset.x == 0) && (coord.x > 0.5)){
//    	FragColor += vec4(0,1,0,.5);//coord = vec2(0);
	return;
    }
    if ((offset.x != 0) && (coord.x < 0.5)){
//    	FragColor += vec4(0,0,1,.5);//coord = vec2(0);
	return;
    }

    // if (viewport_position.x > 0)
    // 	FragColor = vec4(1, 0, 0, 0.5);
    // else
    // 	FragColor = vec4(0, 1, 0, 0.5);
    
    // if(relative_pos.x > 0 ) FragColor = vec4(1,0,0,1);
    // if(relative_pos.y > 0 ) FragColor = vec4(0,1,0,1);
    // if (r > 0.3) FragColor = vec4(0,0,1,1);
    
    FragColor += texture2D(Texture0, coord);

}


// #version 110

// uniform sampler2D source;

// uniform vec2 LensCenter;
// uniform vec2 ScreenCenter;
// uniform vec2 Scale;
// uniform vec2 ScaleIn;
// uniform vec4 HmdWarpParam;

// vec2 HmdWarp(vec2 texIn)
// {
//   vec2 theta = (texIn - LensCenter) * ScaleIn;
//   float  rSq= theta.x * theta.x + theta.y * theta.y;
//   vec2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +
//        HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
//   return LensCenter + Scale * theta1; //theta1;
// }

// void main()
// {
//   vec2 tc = HmdWarp(gl_TexCoord[0].st);
//   if (any(notEqual(clamp(tc, ScreenCenter-vec2(0.5,0.5), ScreenCenter+vec2(0.5, 0.5)) - tc, vec2(0.0, 0.0))))
//     gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
//   else
//     gl_FragColor = texture2D(source, tc);
// };
