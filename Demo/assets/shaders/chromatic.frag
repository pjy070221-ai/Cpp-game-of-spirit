// ═══════════════════════════════════════════════
// Chromatic Aberration 色散着色器
// RGB 通道向不同方向偏移，产生故障/撕裂效果
// uniform: amount (0.0~1.0), 0=无效果, 1=最大偏移
// ═══════════════════════════════════════════════

#version 330 core

uniform sampler2D texture;
uniform vec2  texSize;
uniform float amount = 0.0;     // 色散强度

in vec2 texCoord;
out vec4 fragColor;

void main()
{
    vec2 pixelSize = 1.0 / texSize;

    // 从中心向外的偏移方向
    vec2 dir = texCoord - vec2(0.5);
    float dist = length(dir);
    dir = normalize(dir);

    // RGB 偏移量：红蓝通道向相反方向偏移
    float shift = amount * 10.0 * pixelSize.x;
    vec2 rOffset = dir * shift * 1.0;
    vec2 bOffset = dir * shift * -1.0;

    float r = texture2D(texture, texCoord + rOffset).r;
    float g = texture2D(texture, texCoord).g;
    float b = texture2D(texture, texCoord + bOffset).b;

    // 边缘区域效果更强
    float edgeFactor = smoothstep(0.2, 0.8, dist);
    r = mix(texture2D(texture, texCoord).r, r, edgeFactor * amount);
    b = mix(texture2D(texture, texCoord).b, b, edgeFactor * amount);

    fragColor = vec4(r, g, b, texture2D(texture, texCoord).a);
}
