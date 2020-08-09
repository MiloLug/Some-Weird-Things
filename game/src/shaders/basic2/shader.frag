#version 330 core

uniform vec4 colorIn;
in vec4 ShadowCoord;
out vec4 colorOut;

float smoothrect(in vec2 uv, in vec2 pos, in vec2 size, float s)
{
    vec2 end = pos + size;
    return smoothstep(pos.x - s, pos.x + s, uv.x) * (1.0 - smoothstep(end.x - s, end.x + s, uv.x)) *
        smoothstep(pos.y - s, pos.y + s, uv.y) * (1.0 - smoothstep(end.y - s, end.y + s, uv.y));
}


void main()
{
//	vec2 uv = gl_FragCoord.xy / 600;
//    aspect = 400 / 600;
//
//	vec3 col = vec4(0.0, 0.0, 0.0, 0.0);
//	draw_rect(col, uv, vec2(0.1, 0.1), vec2(0.4, 0.3), vec3(1.0, 0.0, 0.0));
    //draw_rect(col, uv, vec2(0.4, 0.25), vec2(0.4, 0.45), vec3(0.0, 0.0, 1.0));

    colorOut = colorIn;
	//colorOut = vec4(colorIn.rgb, 1.0 * smoothstep(gl_FragCoord.x - 0.15, gl_FragCoord.x - ShadowCoord.x + 0.15, gl_FragCoord.xy));
	//colorOut =  vec4(vec3(clamp(ShadowCoord.x/500,0.0,1.0),clamp(gl_FragCoord.y/500,0.0,1.0),0.0),1.0);
}