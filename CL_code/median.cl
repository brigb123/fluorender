#define KX 3
#define KY 3
#define KZ 3
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;
__kernel void kernel_main(
	read_only image3d_t data,
	__global unsigned char* result,
	unsigned int x,
	unsigned int y,
	unsigned int z)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	int4 kc;
	float4 dvalue;
	float rvalue[KX*KY*KZ];
	int id = 0;
	int c;
	float temp;
	int i, j, k;
	for (i=0; i<KX; ++i)
	for (j=0; j<KY; ++j)
	for (k=0; k<KZ; ++k)
	{
		kc = (int4)(coord.x+(i-KX/2),
				coord.y+(j-KY/2),
				coord.z+(k-KZ/2), 1);
		dvalue = read_imagef(data, samp, kc);
		rvalue[id] = dvalue.x;
		if (id > 0)
		{
			c = id;
			while (c > 0)
			{
				if (rvalue[c] < rvalue[c-1])
				{
					temp = rvalue[c];
					rvalue[c] = rvalue[c-1];
					rvalue[c-1] = temp;
				}
				else break;
				c--;
			}
		}
		id++;
	}
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = rvalue[KX*KY*KZ/2-1]*255.0;
}