cbuffer Data : register(b0)
{
	float Distance;
	float Range;
	float Near;
	float Far;
}


// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
};

Texture2D Unblurred		: register(t0);
Texture2D Blurred		: register(t1);
Texture2D DepthBuffer	: register(t2);
SamplerState Sampler	: register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 base = Unblurred.Sample(Sampler, input.uv);
	float4 blur = Blurred.Sample(Sampler, input.uv);
	float depth = DepthBuffer.Sample(Sampler, input.uv).r;

	depth = 1 - depth;

	float fSceneZ = (-Near * Far) / (depth - Far);
	float blurFactor = saturate(abs(fSceneZ - Distance) / Range);

	return lerp(base, blur, blurFactor);
}