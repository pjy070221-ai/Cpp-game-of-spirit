// ═══════════════════════════════════════════════
// Vignette 暗角着色器
// 画面四角逐渐变暗，增强氛围
// uniform: intensity (0.0~1.0), 0=无效果, 1=完全黑角
// ═══════════════════════════════════════════════

#version 330 core

uniform sampler2D texture;
uniform float intensity = 0.5;  // 暗角强度

in vec2 texCoord;
out vec4 fragColor;

void main()
{
    vec4 color = texture2D(texture, texCoord);

    // 计算到中心的距离
    vec2 center = texCoord - vec2(0.5);
    float dist = length(center);

    // 柔和暗角
    float vignette = 1.0 - dist * intensity;
    vignette = smoothstep(0.0, 1.0, vignette);

    fragColor = vec4(color.rgb * vignette, color.a);
}
