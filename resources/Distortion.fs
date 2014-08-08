// // PASSTHROUGH
// //////////////
// #version 440
// uniform sampler2D Texture0;
// layout( location = 0 ) out vec4 FragColor;

// void main()
// {
//    vec2 texelSize = 1.0 / vec2(textureSize(Texture0, 0));
//    vec2 screenCoords = gl_FragCoord.xy * texelSize;

//    FragColor = texture2D(Texture0, screenCoords);
// }

#version 330


// //uniform vec2 EyeToSourceUVScale = vec2(1);
// //uniform vec2 EyeToSourceUVOffset = vec2(0);

// uniform sampler2D Texture0;
// out vec4 FragColor;

// //uniform bool samples = true;
// //in vec4 oColor;
// //in vec2 oTanEyeAngle;
// //in vec2 oTexCoord0;

// vec2 texelSize = 1.0 / vec2(textureSize(Texture0, 0));
// vec2 screenCoords = gl_FragCoord.xy * texelSize;
// vec2 oTanEyeAngle = (screenCoords - vec2(0.5))*vec2(2.0); // goes from -1 to 1
// vec2 oTexCoord0 = screenCoords;

// vec2 delta = fwidth(oTanEyeAngle); // doesn't seem right ... not true tan!

// vec2 tanEyeAngleToTexture(vec2 v) {
// //   vec2 EyeToSourceUVScale = vec2(1);
// //   vec2 EyeToSourceUVOffset = vec2(1);
// //   vec2 result =  v * EyeToSourceUVScale + EyeToSourceUVOffset;
// //   result.y = 1.0 - result.y;
// //   return result;
//     return (v / vec2(2.0)) + vec2(0.5);
// }

// void main()
// {

//     bool samples = true;

//     vec3 color = textureLod(Texture0, oTexCoord0, 0.0).rgb;
//     float count = 1;
//     if (samples && any(greaterThan(delta, vec2(0.005)))) {
//         vec2 texSample1 = tanEyeAngleToTexture(oTanEyeAngle * 0.998);
//         color += textureLod(Texture0, texSample1, 0.0).rgb;
//         vec2 texSample2 = tanEyeAngleToTexture(oTanEyeAngle * 1.002); 
//         color += textureLod(Texture0, texSample2, 0.0).rgb;
//         count = count + 2;
// 	color += vec3(1,0,0);
//     } 
//     if (samples && any(greaterThan(delta, vec2(0.010)))) {
//         vec2 texSample1 = oTanEyeAngle;
//         texSample1.x *= 0.996; 
//         texSample1.y *= 1.004; 
//         texSample1 = tanEyeAngleToTexture(texSample1);
//         color += textureLod(Texture0, texSample1, 0.0).rgb;
//         vec2 texSample2 = oTanEyeAngle; 
//         texSample2.x *= 1.004; 
//         texSample2.y *= 0.996; 
//         texSample2 = tanEyeAngleToTexture(texSample2);
//         color += textureLod(Texture0, texSample2, 0.0).rgb;
//         count = count + 2;
//     } 
//     if (samples && any(greaterThan(delta, vec2(0.015)))) {
//         vec2 texSample1 = oTanEyeAngle;
//         texSample1.x *= 0.994; 
//         texSample1.y *= 1.006; 
//         texSample1 = tanEyeAngleToTexture(texSample1);
//         color += textureLod(Texture0, texSample1, 0.0).rgb;
//         vec2 texSample2 = oTanEyeAngle; 
//         texSample2.x *= 1.006; 
//         texSample2.y *= 0.994; 
//         texSample2 = tanEyeAngleToTexture(texSample2);
//         color += textureLod(Texture0, texSample2, 0.0).rgb;
//         count = count + 2;
//     }
//     FragColor = vec4(color / count, 1);
// }


vec2 LensCenter = vec2(0.5);
vec2 ScreenCenter = vec2(0.5);
vec2 Scale = vec2(0.5);
vec2 ScaleIn = vec2(1.2);
vec4 HmdWarpParam = vec4(1, 0.4, 0.2, 0);
uniform sampler2D Texture0;

vec2 texelSize = 1.0 / vec2(textureSize(Texture0, 0));
vec2 oTexCoord = gl_FragCoord.xy * texelSize;

vec2 HmdWarp(vec2 in01)
{
    vec2  theta = (in01 - LensCenter) * ScaleIn; // Scales to [-1, 1]
    float rSq = theta.x * theta.x + theta.y * theta.y;
    vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +
                            HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
    return LensCenter + Scale * theta1;
}
 
void main()
{
    vec2 tc = HmdWarp(oTexCoord);
    if (!any( equal( clamp( tc, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tc)))
        gl_FragColor = vec4(0);
    else
        gl_FragColor = texture2D(Texture0, tc);
}
