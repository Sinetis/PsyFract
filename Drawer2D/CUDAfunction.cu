#include <cuda_runtime_api.h>
#include <cuda_runtime.h>

#include "CUDAcomplex.cu"

#define PI 3.1415926535897932384626433832795f
#define twoPI 6.283185307179586476925286766559f
#define sqrt2PI 2.506628274631000502415765284811f

__global__ void CUDAJulia(float* xPos, float* yPos, uchar4* col, int iter, int xPixels, float4 f)
{
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	//if (idx > num)
	//	return;
	cuComplex z(xPos[idx % xPixels], yPos[idx / xPixels]), c(f.x, f.y), c1(f.z, f.w);
	z = z.Inverse();
	int it = 0;
	while (it < iter && z.s() < 900.0f)
	{
		c = z*c1.exp()*c;
		//z = z*z + c1*z.Ln() + c*z.exp() +c*c1;
		//z = z*z*z.Ln()/c + c1/z;

		it++;
	}

	col[idx].z = char(127 * __cosf(it/266.0f-c1.x*10)) + 128;
	col[idx].y = char(127 * __cosf(it/15.0f-c1.y*10)) + 128;
	col[idx].x = char(127 * __cosf(it/4.0f-c1.x*10-c1.y*10)) + 128;
	col[idx].w = 122;
}

__host__ void StartCUDAJulia(float* xpos, float* ypos, uchar4* destcol, 
							 int xPixels, int yPixels, int iter, float4 f)
{
	float *xdev, *ydev;
	uchar4 *coldev;
	cudaMalloc((void**)&xdev, xPixels * sizeof(float));
	cudaMalloc((void**)&ydev, yPixels * sizeof(float));
	cudaMalloc((void**)&coldev, xPixels * yPixels * sizeof(uchar4));
	cudaMemcpy((void*)xdev, (void*)xpos, xPixels * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy((void*)ydev, (void*)ypos, yPixels * sizeof(float), cudaMemcpyHostToDevice);

	CUDAJulia<<<dim3(xPixels * yPixels / 256), dim3(256)>>>(xdev, ydev, coldev, iter, xPixels, f);

	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
		
	cudaFree(xdev);
	cudaFree(ydev);
	cudaFree(coldev);
}


__global__ void CUDAJuliaMusic(float* xPos, float* yPos, uchar4* col, int iter, int xPixels, 
							   int* freq1, int* freq2, int anim)
{
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;

	cuComplex z(xPos[idx % xPixels], yPos[idx / xPixels]), c(0, 0);
	int it = 0;
	//z.Inverse();
		while (it < iter && z.s() < 900.0f)
	{
		c.x = (freq1[it]);
		c.y = (freq2[it]);
		
		z = (z*z*z).exp() + cuComplex(-0.621f, 0.f);
		it++;
	}

	col[idx].z = 255-(char(127 * __cosf(it/266.0f)) + 128);
	col[idx].y = 255-(char(127 * __cosf(it/15.0f)) + 128);
	col[idx].x = 255-(char(127 * __cosf(it/4.0f)) + 128);
	col[idx].w = 122;
}

__host__ void StartCUDAJuliaMusic(float* xpos, float* ypos, uchar4* destcol, 
							 int xPixels, int yPixels, int iter,
							 int* freq1, int* freq2, int anim)
{
	float *xdev, *ydev;
	int *devfreq1, *devfreq2;
	uchar4 *coldev;

	cudaMalloc((void**)&xdev, xPixels * sizeof(float));
	cudaMalloc((void**)&ydev, yPixels * sizeof(float));
	cudaMalloc((void**)&devfreq1, iter * sizeof(int));
	cudaMalloc((void**)&devfreq2, iter * sizeof(int));
	cudaMalloc((void**)&coldev, xPixels * yPixels * sizeof(uchar4));

	cudaMemcpy((void*)xdev, (void*)xpos, xPixels * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy((void*)ydev, (void*)ypos, yPixels * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy((void*)devfreq1, (void*)freq1, iter * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy((void*)devfreq2, (void*)freq2, iter * sizeof(int), cudaMemcpyHostToDevice);

	CUDAJuliaMusic<<<dim3(xPixels * yPixels / 256), dim3(256)>>>
		(xdev, ydev, coldev, iter, xPixels, devfreq1, devfreq2, anim);

	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
		
	cudaFree(xdev);
	cudaFree(ydev);
	cudaFree(coldev);
	cudaFree(devfreq1);
	cudaFree(devfreq2);
}


// —писок классных формул
/*
z = z*z*z.Ln() + c;
z = z*z.Ln() + c;
z = z*z.exp() + z*z.Ln() + c;
z = z*z*z.Ln()/c + c;
z = z*z*z.Ln()/c + c/z; Invert colors
*/
//


__global__ void CUDAJuliaFFT(float* xPos, float* yPos, uchar4* col, int iter, int xPixels, 
							 cuComplex* c)
{
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;


	cuComplex z(xPos[idx % xPixels], yPos[idx / xPixels]);
	int it = 0;
	//z = z.Inverse();
	while (it < iter && z.s() < 900.0f)
	{
		z = z*z + c[1];//c[0]*z.asin().exp() + c[1]*z.acos() + c[0]*z.asin(1) + c[1]*z.acos(1);
		it++;
	}

	col[idx].z = 255-(char(127 * __cosf(it/266.0f)) + 128);
	col[idx].y = 255-(char(127 * __cosf(it/15.0f)) + 128);
	col[idx].x = 255-(char(127 * __cosf(it/4.0f)) + 128);
	//col[idx].w = 122;
}

__host__ void StartCUDAJuliaFFT(float* xpos, float* ypos, uchar4* destcol, 
							 int xPixels, int yPixels, int iter,
							 int* freq)
{
	float *xdev, *ydev;
	float2 com[4];
	cuComplex *devCom;
	uchar4 *coldev;

	int S = 512/16;
	for(int j = 0; j < 4; j++)
	{
		com[j].x = 0;
		for(int i = 0; i < S; i++)
			com[j].x += freq[j*S + i + 8];
	}
	for(int j = 4; j < 8; j++)
	{
		com[j/2].y = 0;
		for(int i = 0; i < S; i++)
			com[j/2].y += freq[j*S + i + 8];
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
	max -= mid;
	for (int i = 0; i < 4; i++)
	{
		x = (com[i].x - mid) / max;
		y = (com[i].y - mid) / max;

		com[i].x = x;
		com[i].y = y;
	}

	cudaMalloc((void**)&devCom, 4 * sizeof(cuComplex));
	cudaMalloc((void**)&xdev, xPixels * sizeof(float));
	cudaMalloc((void**)&ydev, yPixels * sizeof(float));
	cudaMalloc((void**)&coldev, xPixels * yPixels * sizeof(uchar4));

	cudaMemcpy((void*)xdev, (void*)xpos, xPixels * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy((void*)ydev, (void*)ypos, yPixels * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy((void*)devCom, (void*)com, 4 * sizeof(float2), cudaMemcpyHostToDevice);

	//cudaMemcpy((void*)devfreq1, (void*)freq1, iter * sizeof(int), cudaMemcpyHostToDevice);
	//cudaMemcpy((void*)devfreq2, (void*)freq2, iter * sizeof(int), cudaMemcpyHostToDevice);

	CUDAJuliaFFT<<<dim3(xPixels * yPixels / 256), dim3(256)>>>
		(xdev, ydev, coldev, iter, xPixels, devCom);

	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
		
	cudaFree(xdev);
	cudaFree(ydev);
	cudaFree(coldev);
	cudaFree(devCom);
}

__device__ cuComplex func(cuComplex z, cuComplex* c)
{
	return z*z*z - 1.0f;
}

__device__ cuComplex der(cuComplex z, cuComplex* c, int k)
{
	//return z*z*3.0f;
	//return (func(zn, c) - func(zn_1, c))/(zn - zn_1);
	cuComplex dz(0.f,0.0001f);
	//dz.x = __cosf(k/50.0f) / 1000.0f;
	//dz.y = __sinf(k/50.0f) / 1000.0f;
	return (func(z+dz, c) - func(z, c))/dz;
}

/*
z = z*z*z.Ln() + c;
z = z*z.Ln() + c;
z = z*z.exp() + z*z.Ln() + c;
z = z*z*z.Ln()/c + c;
z = z*z*z.Ln()/c + c/z; Invert colors
*/

	__global__ void CUDANewton(float4 posData, uchar4* col, int iter, int xPixels, 
							 cuComplex* c, float k) // Nova
{
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	cuComplex C(posData.y + posData.w*(idx / xPixels), posData.x + posData.w*(idx % xPixels)),
		z(c[0].x), zn;
	int it = 0;
	float s;
	do
	{
		zn = z;
		//z = z - ((-z*z).exp() - 1.0f)/C + c[1].x;
		//z = z*z*z.Ln() + C;
		//z = z*z.exp()*c[2].x + z*z.Ln()*c[1].x + C;
		//z = z - z(2.0f+c[1].x/10.f)*((z*z*z*z - 1.0f)/(z*z*z*4.0f - c[2].x)) + C;
		//z = z - z(2.0f)*((z*z*z - 1.0f)/(z*z*3.0f)) + C;
		//z = z - (z(0.7f + c[0].x*2.f)*(z*z*z - 1.0f)/(z*z*3.f)).pow(2) + C; 
		z = ((z*z + C) / (z*2.0f + C - 2)).pow(2) + 1 + c[1].x;
		//z = z - (z.exp()/z + C)/(z.exp()/z - z.exp()/(z*z));
		it++;
		s = (z-zn).s();
	}
	while (it < iter && s > 1.0e-5);// && s < 1.0e10);

	float color = it*2.0f + 10*c[0].x;
	col[idx].z = 255-(char(127 * __cosf(color/266.0f+3*c[1].x)) + 128);
	col[idx].y = 255-(char(127 * __cosf(color/15.0f+3*c[2].x)) + 128);
	col[idx].x = (char(127 * __cosf(color/4.0f+3*c[3].x)) + 128);
}

__global__ void CUDANewton_(float4 posData, uchar4* col, int iter, int xPixels, 
							 cuComplex* c, float k) // Magnetic
{
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	cuComplex C(-posData.y - posData.w*(idx / xPixels), posData.x + posData.w*(idx % xPixels)),
		z(3*cosf(k/101)), zn;

	int it = 0;
	float s;
	do
	{
		zn = z;
		z = z - z(2.f)*((z*z*z - 1.f + 3*cosf(k/31))/(z*z*3.f + 3*cosf(k/61))) + C;
		it++;
		s = (z-zn).s();
	}
	while (it < iter && s > 0.00001f );

	float color = it*2.0f;
	col[idx].z = 255-(char(127 * __cosf(color/266.0f+3*c[1].x)) + 128);
	col[idx].y = (char(127 * __cosf(color/15.0f+3*c[2].x)) + 128);
	col[idx].x = (char(127 * __cosf(color/4.0f+3*c[3].x)) + 128);
}

__host__ void StartCUDANewton(float4 posData, uchar4* destcol, 
							 int xPixels, int yPixels, int iter,
							 int* freq)
{
	cudaEvent_t start, startkernel;
	cudaEvent_t stop, stopkernel;
	cudaEventCreate(&start);
	cudaEventCreate(&startkernel);
	cudaEventCreate(&stop);
	cudaEventCreate(&stopkernel);
	cudaEventRecord(start, 0);

	float2 com[4];
	cuComplex *devCom;
	uchar4 *coldev;

	int S = 512/16;
	for(int j = 0; j < 4; j++)
	{
		com[j].x = 0;
		for(int i = 0; i < S; i++)
			com[j].x += freq[j*S + i + 6];
	}
	for(int j = 4; j < 8; j++)
	{
		com[j/2].y = 0;
		for(int i = 0; i < S; i++)
			com[j/2].y += freq[j*S + i + 6];
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
	
	cudaEventRecord(startkernel, 0);
	CUDANewton<<<dim3(xPixels * yPixels / 256), dim3(256)>>>
		(posData, coldev, iter%256, xPixels, devCom, iter/256);
	cudaEventRecord(stopkernel, 0);
	//cudaEventSynchronize(stopkernel);

	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
		
	cudaFree(coldev);
	cudaFree(devCom);
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);

	float time, timekernel, without;
	cudaEventElapsedTime(&time, start, stop);
	cudaEventElapsedTime(&timekernel, startkernel, stopkernel);
	without = time - timekernel;
	cudaEventDestroy(start);
	cudaEventDestroy(startkernel);
	cudaEventDestroy(stop);
	cudaEventDestroy(stopkernel);
}

//__global__ void CUDANewton(float* xPos, float* yPos, uchar4* col, int iter, int xPixels, 
//							 cuComplex* c, float k)
//{
//	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
//	//cuComplex z(xPos[idx % xPixels], yPos[idx / xPixels]),zn(0.f,0.f);
//	cuComplex C(xPos[idx % xPixels], yPos[idx / xPixels]),z(1.f,0.f), zn;
//	//z = z*z*z - 1.0f;
//	//cuComplex /*f = func (z, c),*/ d = der(z, c, k);
//	//z = z.Inverse();
//	//z = z.Rotate(halfPI);
//	int it = 0;
//	float s;
//	do
//	{
//		//d = der(z, zn, c, k);
//		zn = z;
//		//z = z - (func (zn, c) / d)*2.0f;//er(z, c, k);
//		//z = func(z, c);
//		z = z - z(3.0f)*((z*z*z*z - 1.0f)/(z*z*z*4.0f)) + C;
//		//z = (z*z*z*2.0f + 1.0f)/(z*z*3.0f); 
//		//z = (z*z*z*2.0f)/(z*z*3.0f + c[0] * c[3]) + cuComplex(0.05f,c[0].y/10.0f).Rotate(k/PI);
//		//z = z*z + C;
//		//z = z - (z.ch() + c[it%4]);
//		it++;
//		//f = func (z, c);
//		//d = der (z, c, k);
//		s = (z-zn).s();
//	}
//	while (it < iter && s > 0.00001f && s < 10000.f);
//	
//	
//	//;// && z.s() < 16000.0f)
//
//	//idx*=4;
//	float color = it*2.0f;// + k/9.0f;
//	col[idx].z = (char(127 * __cosf(color/266.0f+3*c[1].x)) + 128);
//	col[idx].y = (char(127 * __cosf(color/15.0f+3*c[2].x)) + 128);
//	col[idx].x = (char(127 * __cosf(color/4.0f+3*c[3].x)) + 128);
//	//col[idx+3] = 122;
//}
//
//__host__ void StartCUDANewton(float* xpos, float* ypos, uchar4* destcol, 
//							 int xPixels, int yPixels, int iter,
//							 int* freq)
//{
//	cudaEvent_t start, startkernel;
//	cudaEvent_t stop, stopkernel;
//	cudaEventCreate(&start);
//	cudaEventCreate(&startkernel);
//	cudaEventCreate(&stop);
//	cudaEventCreate(&stopkernel);
//	cudaEventRecord(start, 0);
//
//
//	float *xdev, *ydev;
//	float2 com[4];
//	cuComplex *devCom;
//	uchar4 *coldev;
//
//	int S = 512/16;
//	for(int j = 0; j < 4; j++)
//	{
//		com[j].x = 0;
//		for(int i = 0; i < S; i++)
//			com[j].x += freq[j*S + i + 8];
//	}
//	for(int j = 4; j < 8; j++)
//	{
//		com[j/2].y = 0;
//		for(int i = 0; i < S; i++)
//			com[j/2].y += freq[j*S + i + 8];
//	}
//	int max = 1, mid = 0;
//	{
//		if (com[0].x > max)
//			max = com[0].x;
//		if (com[1].x > max)
//			max = com[1].x;
//		if (com[2].x > max)
//			max = com[2].x;
//		if (com[3].x > max)
//			max = com[3].x;
//		if (com[0].y > max)
//			max = com[0].y;
//		if (com[1].y > max)
//			max = com[1].y;
//		if (com[2].y > max)
//			max = com[2].y;
//		if (com[3].y > max)
//			max = com[3].y;
//		
//		mid += com[0].x;
//		mid += com[1].x;
//		mid += com[2].x;
//		mid += com[3].x;
//		mid += com[0].y;
//		mid += com[1].y;
//		mid += com[2].y;
//		mid += com[3].y;
//		mid >>= 3;
//	}
//	float x, y;
//	//max -= mid;
//	for (int i = 0; i < 4; i++)
//	{
//		x = (com[i].x - mid) / max;
//		y = (com[i].y - mid) / max;
//
//		com[i].x = x;
//		com[i].y = y;
//	}
//
//	cudaMalloc((void**)&devCom, 4 * sizeof(cuComplex));
//	cudaMalloc((void**)&xdev, xPixels * sizeof(float));
//	cudaMalloc((void**)&ydev, yPixels * sizeof(float));
//	cudaMalloc((void**)&coldev, xPixels * yPixels * sizeof(uchar4));
//
//	cudaMemcpy((void*)xdev, (void*)xpos, xPixels * sizeof(float), cudaMemcpyHostToDevice);
//	cudaMemcpy((void*)ydev, (void*)ypos, yPixels * sizeof(float), cudaMemcpyHostToDevice);
//	cudaMemcpy((void*)devCom, (void*)com, 4 * sizeof(float2), cudaMemcpyHostToDevice);
//	
//	cudaEventRecord(startkernel, 0);
//	CUDANewton<<<dim3(xPixels * yPixels / 256), dim3(256)>>>
//		(xdev, ydev, coldev, iter%256, xPixels, devCom, iter/256);
//	cudaEventRecord(stopkernel, 0);
//	//cudaEventSynchronize(stopkernel);
//
//	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
//		
//	cudaFree(xdev);
//	cudaFree(ydev);
//	cudaFree(coldev);
//	cudaFree(devCom);
//	cudaEventRecord(stop, 0);
//	cudaEventSynchronize(stop);
//
//	float time, timekernel, without;
//	cudaEventElapsedTime(&time, start, stop);
//	cudaEventElapsedTime(&timekernel, startkernel, stopkernel);
//	without = time - timekernel;
//	cudaEventDestroy(start);
//	cudaEventDestroy(startkernel);
//	cudaEventDestroy(stop);
//	cudaEventDestroy(stopkernel);
//}
/*
__global__ void CUDASeveral(float* xPos, float* yPos, uchar4* col, int iter, int xPixels, 
							 cuComplex* c, float k)
{
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	cuComplex z, zn; float s;

	z = cuComplex(xPos[idx % xPixels], yPos[idx / xPixels]);
	zn = cuComplex(0.f,0.f);
	int it1 = 0;
	do
	{
		zn = z;
		z = (z*z*z)*c[1] - 1.0f;
		it1++;
		s = (z-zn).s();
	}
	while (it1 < iter && s > 0.000001f && s < 1000000.f);

	z = cuComplex(xPos[idx % xPixels], yPos[idx / xPixels]);
	zn = cuComplex(0.f,0.f);
	int it2 = 0;
	do
	{
		zn = z;
		z = (z*z)*c[2]*c[0] - 1.0f;
		it2++;
		s = (z-zn).s();
	}
	while (it2 < iter && s > 0.000001f && s < 1000000.f);

	z = cuComplex(xPos[idx % xPixels], yPos[idx / xPixels]);
	zn = cuComplex(0.f,0.f);
	int it3 = 0;
	do
	{
		zn = z;
		z = (z*z*z*z)*c[3] - 1.0f;
		it3++;
		s = (z-zn).s();
	}
	while (it3 < iter && s > 0.000001f && s < 1000000.f);
	
	col[idx].z = 255-(char(127 * __cosf((it1+it2)/11.0f+c[0].y)) + 128);
	col[idx].y = 255-(char(127 * __cosf((it1+it3)/11.0f+c[1].y)) + 128);
	col[idx].x = 255-(char(127 * __cosf((it2+it3)/11.0f+c[2].y)) + 128);
}

__host__ void StartCUDASeveral(float* xpos, float* ypos, uchar4* destcol, 
							 int xPixels, int yPixels, int iter,
							 int* freq)
{
	cudaEvent_t start, startkernel;
	cudaEvent_t stop, stopkernel;
	cudaEventCreate(&start);
	cudaEventCreate(&startkernel);
	cudaEventCreate(&stop);
	cudaEventCreate(&stopkernel);
	cudaEventRecord(start, 0);


	float *xdev, *ydev;
	float2 com[4];
	cuComplex *devCom;
	uchar4 *coldev;

	int S = 512/16;
	for(int j = 0; j < 4; j++)
	{
		com[j].x = 0;
		for(int i = 0; i < S; i++)
			com[j].x += freq[j*S + i + 8];
	}
	for(int j = 4; j < 8; j++)
	{
		com[j/2].y = 0;
		for(int i = 0; i < S; i++)
			com[j/2].y += freq[j*S + i + 8];
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
	max -= mid;
	for (int i = 0; i < 4; i++)
	{
		x = (com[i].x - mid) / max;
		y = (com[i].y - mid) / max;

		com[i].x = x;
		com[i].y = y;
	}

	cudaMalloc((void**)&devCom, 4 * sizeof(cuComplex));
	cudaMalloc((void**)&xdev, xPixels * sizeof(float));
	cudaMalloc((void**)&ydev, yPixels * sizeof(float));
	cudaMalloc((void**)&coldev, xPixels * yPixels * sizeof(uchar4));

	cudaMemcpy((void*)xdev, (void*)xpos, xPixels * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy((void*)ydev, (void*)ypos, yPixels * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy((void*)devCom, (void*)com, 4 * sizeof(float2), cudaMemcpyHostToDevice);
	
	cudaEventRecord(startkernel, 0);
	CUDASeveral<<<dim3(xPixels * yPixels / 256), dim3(256)>>>
		(xdev, ydev, coldev, iter%256, xPixels, devCom, iter/256);
	cudaEventRecord(stopkernel, 0);
	cudaEventSynchronize(stopkernel);

	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
		
	cudaFree(xdev);
	cudaFree(ydev);
	cudaFree(coldev);
	cudaFree(devCom);
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);

	float time, timekernel, without;
	cudaEventElapsedTime(&time, start, stop);
	cudaEventElapsedTime(&timekernel, startkernel, stopkernel);
	without = time - timekernel;
	cudaEventDestroy(start);
	cudaEventDestroy(startkernel);
	cudaEventDestroy(stop);
	cudaEventDestroy(stopkernel);
}
*/
struct data
{
	int iter;
	float k;
	int frac;
	unsigned int col;
};

//__device__ inline cuComplex Fracs(cuComplex z, cuComplex C, cuComplex*c, int frac, int k)
//{
//	switch (frac)
//	{
//	case 1:
//		return z - ((-z*z).exp() - 1.0f)/C + c[1].x;
//	case 2:
//		return z*z*z.Ln() + C;
//	case 3:
//		return z*z.exp()*c[2].x + z*z.Ln()*c[1].x + C;
//	case 4:
//		return z - z(2.0f+c[1].x/10.f)*((z*z*z*z - 1.0f)/(z*z*z*4.0f - c[2].x)) + C;
//	case 5:
//		return z - z(2.0f)*((z*z*z - 1.0f)/(z*z*3.0f)) + C;
//	case 6:
//		return z - (z(0.7f + c[0].x*2.f)*(z*z*z - 1.0f)/(z*z*3.f)).pow(2) + C; 
//	case 7:
//		return (z*z*z*2.0f)/(z*z*3.0f + c[0] * c[3]) + cuComplex(0.05f,c[0].y/10.0f).Rotate(k/PI);
//	case 8:
//		return z - (z.exp()/z + C)/(z.exp()/z - z.exp()/(z*z));
//	case 9:
//		return z.exp();
//	case 10:
//		return z.Ln();
//	case 11:
//		return z.sin();
//	case 12:
//		return z.ch();
//	case 13:
//		return z.asin();
//	case 14:
//		return z - z.exp() + C;
//	case 15:
//		return c[0] + c[1]*z + c[2]*z*z + c[3]*z*z*z + C;
//	case 16:
//		return z - z.sin() + C;
//	case 17:
//		return z - z.ch() + C;
//	case 18:
//		return z - z.sin()*(c[1].x+c[2].x) + C;
//	case 19:
//		return (-z*z).exp() + C;
//	case 20:
//		return (z+c[0].y)*c[0].x + (z/(C - c[2].y))*c[2].x + (z*z)*c[3].x + C;
//	case 21:
//		return cuComplex(__cosf(z.x), -__sinf(z.y));
//	case 22:
//		return c[0]*(z.i()*C*z).exp() + c[1]*(-z.i()*C*z).exp() + (C*C).Inverse(); 
//	case 23:
//		return (z+c[2].x).sin()*(z+c[1].x).cos() + C;
//	case 24:
//		return c[0]*((z*z).exp()) + c[1]*(z.exp()) + C + c[2]*z + c[3]*z*z;  
//	case 25:
//		return (-z*z).exp()*(1/(sqrt2PI)) + C;
//	default:
//		return ((z*z + C) / (z*2.0f + C - 2)).pow(2) + 1 + c[1].x;
//	}
//}

__device__ inline cuComplex Fracs(cuComplex z, cuComplex C, cuComplex*c, int frac, int k)
{
	switch (frac)
	{
	case 1:
		return z - ((-z*z).exp() - 1.0f)/C + c[1].x;
	case 2:
		return z*z*z.Ln() + C;
	case 3:
		return z*z.exp()*c[2].x + z*z.Ln()*c[3].x + C;
	case 4:
		return z - z(2.0f+c[0].y/10.f)*((z*z*z*z - 1.0f)/(z*z*z*4.0f - c[1].y)) + C;
	case 5:
		return z - z(2.0f)*((z*z*z - 1.0f)/(z*z*3.0f)) + C;
	case 6:
		return z - (z(0.7f + c[2].y*2.f)*(z*z*z - 1.0f)/(z*z*3.f)).pow(2) + C; 
	case 7:
		return (z*z*z*2.0f)/(z*z*3.0f + c[0] * c[3]) + cuComplex(0.05f,c[3].y/10.0f).Rotate(k/PI);
	case 8:
		return z - (z.exp()/z + C)/(z.exp()/z - z.exp()/(z*z));
	case 9:
		return (z+c[1].x).exp() + c[0];
	case 10:
		return (z+c[3].x).Ln() + c[2];
	case 11:
		return (z+c[2].x).sin() + c[1];
	case 12:
		return (z+c[0].x).ch() + c[3];
	case 13:
		return (z+c[2].x).acos() + c[1];
	case 14:
		return z - z.exp() + C;
	case 15:
		return c[0] + c[1]*z + c[2]*z*z + c[3]*z*z*z + C;
	case 16:
		return z - z.sin() + C;
	case 17:
		return z - z.ch() + C;
	case 18:
		return z - (z+c[1].d()).sin()*(c[0].y+c[2].x) + C + c[1].x;
	case 19:
		return (-z*z).exp() + C;
	case 20:
		return (z+c[0].y)*c[0].x + (z/(C - c[2].y))*c[2].x + (z*z)*c[3].x + C;
	case 21:
		return cuComplex(__cosf(z.x), -__sinf(z.y));
	case 22:
		return c[0]*(z.i()*C*z).exp() + c[1]*(-z.i()*C*z).exp() + (C*C).Inverse(); 
	case 23:
		return (z+c[2].x).sin()*(z+c[1].y).cos() + C;
	case 24:
		return c[0]*((z*z).exp()) + c[1]*(z.exp()) + C + c[2]*z + c[3]*z*z;  
	case 25:
		return (-z*z).exp()*(1/(sqrt2PI)) + C;
	case 26:
		return z - ((-z*z).exp() + c[2].x)/(-z(2)*z*(-z*z).exp()) + C;
	case 27:
		return z*z - ((-z*z).exp() + c[2].x)/(-z(2)*z*(-z*z).exp()) + C*C;
	case 28:
		return z*z + C;
	case 29:
		{
			cuComplex nz = z*z;
			float x = nz.x, y = nz.y;
			return z(3*x*x - 6*x*y - y*y, 3*x*x + 6*x*y - 3*y*y) + C;
		}
	case 30:
		return (c[0]-c[1]+c[2]-c[3])*z*(z(1)-z) + C;
	case 31:
		return z + z.cos()/z.sin() + C;
	case 32:
		{
			//cuComplex a1(c[0]), a2(-c[1]);
			//cuComplex	f1 = (z - a1)/(z(1) - z*a1.сопр€жение()),
			//			f2 = (z - a2)/(z(1) - z*a2.сопр€жение());
			//cuComplex f = (C*z.i()).exp()*f1*f2;
			return z - ((z-1)*(z-2)*(z-3))/((z+z.i())*(z+z.i()*2)*(z+z.i()*3)) + C;
		}
	default:
		return ((z*z + C + c[0].y) / (z*2.0f + C - 2 + c[2].y)).pow(2) + 1 + c[1].x;
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
	unsigned int idy = blockDim.y * blockIdx.y + threadIdx.y;
	float	x = posData.y + posData.w*(idy),
			y = posData.x + posData.w*(idx);
	cuComplex C, z, zn;
	if (t)
	{
		C = cuComplex(x, y);
		//float s = C.s();
		//z = C/s;
		z = cuComplex(c[0].x);
	}
	else
	{
		z = cuComplex(x, y);
		//float s = z.s();
		//C = z/s;
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
	while (it < st.iter && s > 1.0e-6 && s < 1.0e9);
	float color = it*2.0f + 7*c[0].x + st.k;
	int r,g,b;
	//b = (char(127 * __cosf(color/166.0f+4*c[1].x)) + 128);
	//g = (char(127 * __cosf(color/15.0f+4*c[2].x)) + 128);
	//r = (char(127 * __cosf(color/4.0f+4*c[3].x)) + 128);
	b = (char(127 * __cosf(color/17.0f+4*c[1].x)) + 128);
	g = (char(127 * __cosf(color/11.0f+4*c[2].x)) + 128);
	r = (char(127 * __cosf(color/7.0f+4*c[3].x)) + 128);
	
	if (st.col&1) r = 255 - r;
	if (st.col&2) g = 255 - g;
	if (st.col&4) b = 255 - b;
	
	int pos = idx + idy*xPixels;
	pix[pos].x = r;
	pix[pos].y = g;
	pix[pos].z = b;/*
	pos = xPixels - idx + idy*xPixels;
	pix[pos].x = r;
	pix[pos].y = g;
	pix[pos].z = b;*/
}

__global__ void CUDAFractalOld(float4 posData, uchar4* pix, int xPixels, 
							cuComplex* c, data st, bool t)
{
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned int idy = blockDim.y * blockIdx.y + threadIdx.y;
	float	x = posData.y + posData.w*(idy),
			y = posData.x + posData.w*(idx);
	cuComplex C, z1, z2, z1n, z2n;
	
	if (t)
	{
		C = cuComplex(x, y);
		//float s = C.s();
		//z = C/s;
		z1 = cuComplex(c[0].x);
		z2 = cuComplex(c[1].x);
	}
	else
	{
		z1 = cuComplex(x, y);
		z2 = cuComplex(x, y);
		//float s = z.s();
		//C = z/s;
		C = cuComplex(c[0].x);
	}
	int it = 0;
	float s;
	do
	{
		z1n = z1;
		z2n = z2;
		z1 = z1 - (z2.pow(3) - 1)/(z1(3)*z1.pow(2)) + C;
		z2 = (z2 - z1.Inverse())/2 + C;
		
		it++;
		s = (z2-z2n).s();
	}
	while (it < st.iter && s > 1.0e-6);//&& s < 1.0e9);
	float color = it*2.0f + 7*c[0].x + st.k;
	int r,g,b;
	b = (char(127 * __cosf(color/166.0f+4*c[1].x)) + 128);
	g = (char(127 * __cosf(color/15.0f+4*c[2].x)) + 128);
	r = (char(127 * __cosf(color/4.0f+4*c[3].x)) + 128);
	
	if (st.col&1) r = 255 - r;
	if (st.col&2) g = 255 - g;
	if (st.col&4) b = 255 - b;
	
	int pos = idx + idy*xPixels;
	pix[pos].x = r;
	pix[pos].y = g;
	pix[pos].z = b;/*
	pos = xPixels - idx + idy*xPixels;
	pix[pos].x = r;
	pix[pos].y = g;
	pix[pos].z = b;*/
}

//__global__ void CUDAFractal(float4 posData, uchar4* pix, int xPixels, 
//							cuComplex* c, data st, bool t)
//{
//	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
//	float	x = posData.y + posData.w*(idx / xPixels),
//		y = posData.x + posData.w*(idx % xPixels);
//	cuComplex C, z, zn;
//	if (t)
//	{
//		C = cuComplex(x, y);
//		//z = C/c[0].x;
//		z = cuComplex(c[0].x);
//	}
//	else
//	{
//		z = cuComplex(x, y);
//		//C = z/c[0].x;
//		C = cuComplex(c[0].x);
//	}
//	int it = 0;
//	float s;
//	do
//	{
//		zn = z;
//		function(z, C, c, st);
//		it++;
//		s = (z-zn).s();
//	}
//	while (it < st.iter && s > 1.0e-5);// && s < 1.0e10);
//
//	float color = it*2.0f + 7*c[0].x + st.k;
//	int r,g,b;
//	b = (char(127 * __cosf(color/266.0f+4*c[1].x)) + 128);
//	g = (char(127 * __cosf(color/15.0f+4*c[2].x)) + 128);
//	r = (char(127 * __cosf(color/4.0f+4*c[3].x)) + 128);
//	
//	if (st.col&1) r = 255 - r;
//	if (st.col&2) g = 255 - g;
//	if (st.col&4) b = 255 - b;
//	
//	pix[idx].x = r;
//	pix[idx].y = g;
//	pix[idx].z = b;
//}
///<summary> 
/// ifct: Iter, Frac, Col, Type 
///</summary>
//__host__ void StartCUDAFractal(float4 posData, uchar4* destcol, 
//							   int xPixels, int yPixels, uint4 ifct, int* freq)
//{
//	float2 com[4];
//	cuComplex *devCom;
//	uchar4 *coldev;
//
//	int S = 40;
//	for(int j = 0; j < 4; j++)
//	{
//		com[j].x = 0;
//		for(int i = 0; i < S + 10; i++)
//			com[j].x += freq[j*S + i + 4];
//	}
//	for(int j = 4; j < 8; j++)
//	{
//		com[j/2].y = 0;
//		for(int i = 0; i < S + 10; i++)
//			com[j/2].y += freq[j*S + i + 4];
//	}
//	int max = 1, mid = 0;
//	{
//		if (com[0].x > max)
//			max = com[0].x;
//		if (com[1].x > max)
//			max = com[1].x;
//		if (com[2].x > max)
//			max = com[2].x;
//		if (com[3].x > max)
//			max = com[3].x;
//		if (com[0].y > max)
//			max = com[0].y;
//		if (com[1].y > max)
//			max = com[1].y;
//		if (com[2].y > max)
//			max = com[2].y;
//		if (com[3].y > max)
//			max = com[3].y;
//		
//		mid += com[0].x;
//		mid += com[1].x;
//		mid += com[2].x;
//		mid += com[3].x;
//		mid += com[0].y;
//		mid += com[1].y;
//		mid += com[2].y;
//		mid += com[3].y;
//		mid >>= 3;
//	}
//	float x, y;
//	//max -= mid;
//	if (max < 512)
//		com[0]=com[1]=com[2]=com[3]=float2();
//	else
//	for (int i = 0; i < 4; i++)
//	{
//		x = (com[i].x - mid) / max;
//		y = (com[i].y - mid) / max;
//
//		com[i].x = x;
//		com[i].y = y;
//	}
//
//	cudaMalloc((void**)&devCom, 4 * sizeof(cuComplex));
//	cudaMalloc((void**)&coldev, xPixels * yPixels * sizeof(uchar4));
//
//	cudaMemcpy((void*)devCom, (void*)com, 4 * sizeof(float2), cudaMemcpyHostToDevice);
//
//	data st;
//	st.col = ifct.z;
//	st.frac = ifct.y;
//	st.iter = ifct.x%512;
//	st.k = ifct.x/512;
//	
//	CUDAFractal<<<dim3(xPixels * yPixels / 256), dim3(256)>>>
//		(posData, coldev, xPixels, devCom, st, ifct.w);
//
//	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
//	
//	cudaFree(devCom);
//	cudaFree(coldev);
//}

__host__ void StartCUDAFractal(float4 posData, uchar4* destcol, 
							   int xPixels, int yPixels, uint4 ifct, int* freq)
{
	float2 com[4];
	cuComplex *devCom;
	uchar4 *coldev;

	int xx = 200;

	int S = 40;
	int i;
	com[0].x=com[1].x=com[2].x=com[3].x=
	com[0].y=com[1].y=com[2].y=com[3].y=0;
	for (i = 0; i < 7; i++)//8; i++)11
		if (freq[i]>xx)
		com[0].x += freq[i];//-xx;
	for (; i < 18+1; i++)//21; i++)29
		if (freq[i]>xx)
		com[1].x += freq[i];//-xx;
	i--;
	for (; i < 36+1; i++)//40; i++)58
		if (freq[i]>xx)
		com[2].x += freq[i];//-xx;
	i--;
	for (; i < 64+2; i++)//71; i++)103
		if (freq[i]>xx)
		com[3].x += freq[i];//-xx;
	i-=2;
	for (; i < 110+3; i++)//119; i++)178
		if (freq[i]>xx)
		com[0].y += freq[i];//-xx;
	i-=3;
	for (; i < 185+5; i++)//194; i++)299
		if (freq[i]>xx)
		com[1].y += freq[i];//-xx;
	i-=5;
	for (; i < 306+8; i++)//313; i++)495
		if (freq[i]>xx)
		com[2].y += freq[i];//-xx;
	i-=8;
	for (; i < 502+10; i++)//499; i++)812
		if (freq[i]>xx)
		com[3].y += freq[i];//-xx;
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
	max*=2;
	for (int i = 0; i < 4; i++)
	{
		x = (com[i].x - mid) / max;
		y = (com[i].y - mid) / max;

		com[i].x = x;
		com[i].y = y;
	}
	//float n = 10240;
	//if (max < n)
	//	for (int i = 0; i < 4; i++)
	//	{
	//		com[i].x *= (1-(n-max)/n);
	//		com[i].y *= (1-(n-max)/n);
	//	}
	//float n = (float)mid/(float)max;
	//if (n > 0.25)
	//	for (int i = 0; i < 4; i++)
	//	{
	//		com[i].x *= (1-n);
	//		com[i].y *= (1-n);
	//	}

	cudaMalloc((void**)&devCom, 4 * sizeof(cuComplex));
	cudaMalloc((void**)&coldev, xPixels * yPixels * sizeof(uchar4));

	cudaMemcpy((void*)devCom, (void*)com, 4 * sizeof(float2), cudaMemcpyHostToDevice);

	data st;
	st.col = ifct.z;
	st.frac = ifct.y;
	st.iter = ifct.x%512;
	st.k = ifct.x/512;
	
	//posData.x *= com[3].x;
	//posData.y *= com[3].x;
	//posData.w *= com[3].x;
	
	//CUDAFractal<<<dim3(xPixels*yPixels/256), dim3(256)>>>	
	CUDAFractal<<<dim3(xPixels/16, yPixels/16), dim3(16,16)>>>
		(posData, coldev, xPixels, devCom, st, ifct.w);

	cudaMemcpy((void*)destcol, (void*)coldev, xPixels * yPixels * sizeof(uchar4), cudaMemcpyDeviceToHost);
	
	cudaFree(devCom);
	cudaFree(coldev);
}