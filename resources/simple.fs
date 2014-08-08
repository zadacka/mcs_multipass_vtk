// PASSTHROUGH
//////////////
#version 440
uniform sampler2D Texture0;
layout( location = 0 ) out vec4 FragColor;

void main()
{
    // Some basic tests...
    // FragColor = texture2D(Texture0, gl_PointCoord);    
    // FragColor = vec4(1,1,1,1);
    // FragColor = texture2D(Texture0, vec2(0.5,0.5));
    // FragColor = texture2D(Texture0, gl_FragCoord.xy);

    // Showing that FragCoord gets pixel location...
    // if (gl_FragCoord.x > 640){
    // 	FragColor = vec4(1,0,0,1);
    // }
    // else{
    // 	FragColor = vec4(0,0,1,1);
    // }
    vec2 texelSize = 1.0 / vec2(textureSize(Texture0, 0));
    vec2 screenCoords = gl_FragCoord.xy * texelSize;

    FragColor = texture2D(Texture0, screenCoords);
}
