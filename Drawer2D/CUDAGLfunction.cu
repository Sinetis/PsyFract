#include <cuda_runtime_api.h>
#include <cuda_runtime.h>

#include "CUDAcomplex.cu"

#define PI 3.1415926535897932384626433832795f
#define twoPI 6.283185307179586476925286766559f
#define sqrt2PI 2.506628274631000502415765284811f

struct data
{
	int iter;
	float k;
	int frac;
	unsigned int col;
};

__device__ inline cuComplex Fracs(cuComplex z, cuComplex C, cuComplex*c, int frac, int k)
{
	switch (frac)
	{
	case 1:
		return z - ((-z*z).exp() - 1.0f)/C + c[1].x;
	case 2:
		return z*z*z.Ln() + C;
	case 3:
		return z*z.exp()*c[2].x + z*z.Ln()*c[1].x + C;
	case 4:
		return z - z(2.0f+c[1].x/10.f)*((z*z*z*z - 1.0f)/(z*z*z*4.0f - c[2].x)) + C;
	case 5:
		return z - z(2.0f)*((z*z*z - 1.0f)/(z*z*3.0f)) + C;
	case 6:
		return z - (z(0.7f + c[0].x*2.f)*(z*z*z - 1.0f)/(z*z*3.f)).pow(2) + C; 
	case 7:
		return (z*z*z*2.0f)/(z*z*3.0f + c[0] * c[3]) + cuComplex(0.05f,c[0].y/10.0f).Rotate(k/PI);
	case 8:
		return z - (z.exp()/z + C)/(z.exp()/z - z.exp()/(z*z));
	case 9:
		return z.exp();
	case 10:
		return z.Ln();
	case 11:
		return z.sin();
	case 12:
		return z.ch();
	case 13:
		return z.asin();
	case 14:
		return z - z.exp() + C;
	case 15:
		return c[0] + c[1]*z + c[2]*z*z + c[3]*z*z*z + C;
	case 16:
		return z - z.sin() + C;
	case 17:
		return z - z.ch() + C;
	case 18:
		return z - z.sin()*(c[1].x+c[2].x) + C;
	case 19:
		return (-z*z).exp() + C;
	case 20:
		return (z+c[0].y)*c[0].x + (z/(C - c[2].y))*c[2].x + (z*z)*c[3].x + C;
	case 21:
		return cuComplex(__cosf(z.x), -__sinf(z.y));
	case 22:
		return c[0]*(z.i()*C*z).exp() + c[1]*(-z.i()*C*z).exp() + (C*C).Inverse(); 
	case 23:
		return (z+c[2].x).sin()*(z+c[1].x).cos() + C;
	default:
		return ((z*z + C) / (z*2.0f + C - 2)).pow(2) + 1 + c[1].x;
	}
}

__device__ inline void function(cuComplex& z, cuComplex C, cuComplex* c, data st)
{
	z = Fracs(z, C, c, st.frac/1024, st.k);
	if (st.frac&1)
		z = Fracs(z, C, c, (st.frac%1024)/2, st.k);
}

__global__ void CUDAFractal(float4 posData, uchar4* pix, int xPixels, 
							cuComplex* c, data st, bool t)
{
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	cuComplex C, z, zn;
	if (t)
	{
		C = cuComplex(posData.y + posData.w*(idx / xPixels), posData.x + posData.w*(idx % xPixels));
		//z = C/c[0].x;
		z = cuComplex(c[0].x);
	}
	else
	{
		z = cuComplex(posData.y + posData.w*(idx / xPixels), posData.x + posData.w*(idx % xPixels));
		//C = z/c[0].x;
		C = cuComplex(c[0].x);
	}
	int it = 0;
	float s;
	do
	{
		zn = z;
		function(z, C, c, st);
		it++;
		s = (z-zn).s();
	}
	while (it < st.iter && s > 1.0e-5);// && s < 1.0e10);

	float color = it*2.0f + 7*c[0].x + st.k;
	int r,g,b;
	b = (char(127 * __cosf(color/266.0f+4*c[1].x)) + 128);
	g = (char(127 * __cosf(color/15.0f+4*c[2].x)) + 128);
	r = (char(127 * __cosf(color/4.0f+4*c[3].x)) + 128);
	
	if (st.col&1) r = 255 - r;
	if (st.col&2) g = 255 - g;
	if (st.col&4) b = 255 - b;

	pix[idx].x = r;
	pix[idx].y = g;
	pix[idx].z = b;
}
///<summary> 
/// ifct: Iter, Frac, Col, Type 
///</summary>
__host__ void StartCUDAFractal(float4 posData, uchar4* destcol, 
							   int xPixels, int yPixels, uint4 ifct, int* freq)
{
	float2 com[4];
	cuComplex *devCom;
	uchar4 *coldev;

	int S = 40;
	for(int j = 0; j < 4; j++)
	{
		com[j].x = 0;
		for(int i = 0; i < S + 10; i++)
			com[j].x += freq[j*S + i + 4];
	}
	for(int j = 4; j < 8; j++)
	{
		com[j/2].y = 0;
		for(int i = 0; i < S + 10; i++)
			com[j/2].y += freq[j*S + i + 4];
	}
	int max = 1, mid = 0;
	{
		if (com[0].x > max)
			max = com[0].x;
		if (com[1].x > max)
			max = com[1].x;
		if (com[2].x > max)
			max = com[2].x;
		if (com[3].x > max)
			max = com[3].x;
		if (com[0].y > max)
			max = com[0].y;
		if (com[1].y > max)
			max = com[1].y;
		if (com[2].y > max)
			max = com[2].y;
		if (com[3].y > max)
			max = com[3].y;
		
		mid += com[0].x;
		mid += com[1].x;
		mid += com[2].x;
		mid += com[3].x;
		mid += com[0].y;
		mid += com[1].y;
		mid += com[2].y;
		mid += com[3].y;
		mid >>= 3;
	}
	float x, y;
	//max -= mid;
	if (max < 512)
		com[0]=com[1]=com[2]=com[3]=float2();
	else
	for (int i = 0; i < 4; i++)
	{
		x = (com[i].x - mid) / max;
		y = (com[i].y - mid) / max;

		com[i].x = x;
		com[i].y = y;
	}

	cudaMalloc((void**)&devCom, 4 * sizeof(cuComplex));
	cudaMalloc((void**)&coldev, xPixels * yPixels * sizeof(uchar4));

	cudaMemcpy((void*)devCom, (void*)com, 4 * sizeof(float2), cudaMemcpyHostToDevice);

	data st;
	st.col = ifct.z;
	st.frac = ifct.y;
	st.iter = ifct.x%512;
	st.k = ifct.x/512;
	
	CUDAFractal<<<dim3(xPixels * yPixels / 256), dim3(256)>>>
		(posData, coldev, xPixels, devCom, st, ifct.w);

	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
	
	cudaFree(devCom);
	cudaFree(coldev);
}