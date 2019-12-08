#version 330 core

out vec4 color;

uniform vec3 uniLightPos;
uniform vec3 uniLightColor;

void main()
{
    // Make point to be cycle
    vec3 normal;
    normal.xy = gl_PointCoord.xy * vec2(2.0, -2.0) + vec2(-1.0,1.0);
    float mag = dot(normal.xy, normal.xy);
    if(mag > 1.0) discard;
    
    color = vec4(0.5, 0.7, 0.9, 0.2f);
}
