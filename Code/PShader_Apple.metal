#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;


struct _CustomVertex {
	packed_float2 xy;
	packed_uchar4 rgba;
	packed_float2 uv;
};


struct _VertexOutput {
	float4 xyzw [[position]];
	float4 rgba;
	float2 uv;
};


vertex _VertexOutput _VertexShader (constant _CustomVertex* vertexArray [[buffer(0)]], 
									constant float4x4* matrix  [[buffer(1)]], 
									ushort vertexID [[vertex_id]]) {
	_VertexOutput out;
	out.xyzw = *matrix * float4(vertexArray[vertexID].xy, 0.0, 1.0);
	out.rgba = float4(vertexArray[vertexID].rgba) / 255.0;
	out.uv = float2(vertexArray[vertexID].uv);
	return out;
}


fragment float4 _FragmentShader (_VertexOutput in [[stage_in]], 
								 texture2d<float> texture [[texture(0)]]) {
	return in.rgba * float4(texture.sample(sampler(mag_filter::linear, min_filter::linear), in.uv));
}

