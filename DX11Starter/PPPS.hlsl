
cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	int blurAmount;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D Pixels		: register(t0);
SamplerState Sampler	: register(s0);


// Simple Bloom
float4 main(VertexToPixel input) : SV_TARGET
{
	// Total color for the overall average
	float4 totalColor = float4(0,0,0,0);
	uint numSamples = 0;

	for (int y = -blurAmount; y <= blurAmount; y++)
	{
		for (int x = -blurAmount; x <= blurAmount; x++)
		{
			// Get the uv for the current sample
			float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);

			// Add to the total color
			float4 color = Pixels.Sample(Sampler, uv);
			float cutoff = 0.8;
			color = float4(saturate(color.r - cutoff), saturate(color.g - cutoff), saturate(color.b - cutoff), color.w);
			totalColor += color;
			numSamples++;
		}
	}

	// Average and return
	return (totalColor / numSamples) + 0.95 * Pixels.Sample(Sampler, input.uv);
}