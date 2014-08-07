#version 330 compatibility

in fData
{
    vec3 normal;
    vec4 color;
};

void main()
{
    gl_FragColor = vec4(1,0,0,1);
}

