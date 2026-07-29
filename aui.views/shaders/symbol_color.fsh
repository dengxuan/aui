// 彩色 emoji 字形着色器：字形本身是 RGBA 彩色位图（FT_LOAD_COLOR），直接输出纹理颜色，
// 不用 TextColor 染色——只乘 color.a 做整体透明度。配合独立 RGBA 字形图集 + NORMAL 混合。
uniform {
  vec4 color
  2D albedo
}

inter {
  vec2 uv
}

output {
  [0] vec4 albedo
}

entry {
    vec4 s = uniform.albedo[inter.uv]
    output.albedo = vec4(s.rgb, s.a) * uniform.color.a
}
