#include <cuda_runtime_api.h>
#define PI 3.1415926535897932384626433832795f
#define twoPI 6.283185307179586476925286766559f
#define halfPI 1.5707963267948966192313216916398f

struct  cuComplex
{
	float x;
		float& Re() {return x;}
	float y;
		float& Im() {return y;}
		
	__device__ cuComplex(float re = 0, float im = 0)
		:x(re), y(im)
	{	}
	
	__device__ cuComplex(const cuComplex& val)
		:x(val.x), y(val.y)
	{	}

	__device__ inline cuComplex i()
	{return cuComplex(0.0f, 1.0f);}

	inline __device__ cuComplex operator= (cuComplex val)
	{
		this->x = val.x;
		this->y = val.y;
		return *this;
	}
	
	inline __device__ cuComplex operator+ (cuComplex val)
	{
		return cuComplex(this->x + val.x, this->y + val.y);
	}
	inline __device__ cuComplex operator+= (cuComplex val)
	{
		return *this = *this + val;
	}

	inline __device__ cuComplex operator- (cuComplex val)
	{
		return cuComplex(this->x - val.x, this->y - val.y);
	}
	inline __device__ cuComplex operator- ()
	{
		return cuComplex(-x, -y);
	}
	
	inline __device__ cuComplex operator* (cuComplex val)
	{
		// (x1 + iy1) * (x2 + iy2)
		cuComplex res;
		res.x = this->x * val.x - this->y * val.y;
		res.y = this->x * val.y + this->y * val.x;
		return res;
	}
	inline __device__ cuComplex operator*= (cuComplex val)
	{
		return *this = *this * val;
	}
	
	inline __device__ cuComplex operator/ (cuComplex val)
	{
		// (x1 + iy1) / (x2 + iy2)
		cuComplex res;
		float f = val.s();
		res.x = (this->x * val.x + this->y * val.y) / f;
		res.y = (-this->x * val.y + this->y * val.x) / f;
		return res;
	}

	inline __device__ cuComplex operator()(float re, float im = 0.f)
	{
		return cuComplex(re, im);
	}

	//inline __device__ operator float () const
	//{
	//	return sqrtf(this->x * this->x + this->y * this->y);
	//}
	// Модуль числа
	inline __device__ float d(){return sqrtf(this->x * this->x + this->y * this->y);}
	// Квадрат модуля числа
	inline __device__ float s()
	{
		
		return this->x * this->x + this->y * this->y;}
	// Аргумент числа
	inline __device__ float arg()
	{
		if (x > 0)
			return atanf(y/x);
		if (y > 0)
			return atanf(y/x)+PI;
		else
			return atanf(y/x)-PI;
	}

	inline __device__ cuComplex operator +(float val)
	{
		return cuComplex(this->x+val, this->y);
	}

	inline __device__ cuComplex operator +=(float val)
	{
		this->x += val;
		return *this;
	}
	inline __device__ cuComplex operator -(float val)
	{
		return cuComplex(this->x-val, this->y);
	}

	inline __device__ cuComplex operator -=(float val)
	{
		this->x -= val;
		return *this;
	}

	inline __device__ cuComplex operator *(float val)
	{	
		return cuComplex(this->x*val, this->y*val);
	}

	inline __device__ cuComplex operator /(float val)
	{
		return cuComplex(this->x/val, this->y/val);
	}
	inline __device__ cuComplex operator /=(float val)
	{ return *this = *this / val; }

	inline __device__ cuComplex operator =(float val)
	{
		x = val;
		y = 0;
		return *this;
	}
	inline __device__ cuComplex operator^ (int val)
	{
		cuComplex c = *this;
		bool minus = val < 0;
		val = minus ? -val : val;
		for (int i = 1; i < val; i++) c *= *this;
		c = minus ? c.Inverse() : c;
		return c;
	}

	inline __device__ static cuComplex Zero()
	{
		return cuComplex(0,0);
	}

	// Поворот в радианах
	inline __device__ cuComplex Rotate(float fi)
	{
		cuComplex res;
		float c = __cosf(fi), s = __sinf(fi);
		res.x = x*c - y*s;
		res.y = x*s + y*c;
		return res;
	}

	inline __device__ void Scale(float k)
	{
		this->x *= k;
		this->y *= k;
	}

	inline __device__ void Shift(cuComplex c)
	{
		Shift(c.x, c.y);
	}
	inline __device__ void Shift(float cx, float cy)
	{
		this->x += cx;
		this->y += cy;
	}

	inline __device__ cuComplex Inverse()
	{
		cuComplex c(x, y);
		float m = x*x + y*y;
		c.x /= m;
		c.y /= -m;
		return c;
	}

	inline __device__ cuComplex exp()
	{
		cuComplex c;
		float e;
		e = __expf(x); 
		c.x = e*__cosf(y);
		c.y = e*__sinf(y);
		return c;
	}

	inline __device__ cuComplex Ln()
	{
		cuComplex c;
		c.y = arg();
		c.x = __logf(this->d());
		return c;
	}
	
	inline __device__ cuComplex sin()
	{
		cuComplex c, eiz(0.f,1.f);
		eiz = (eiz * (*this)).exp(); 
		c = eiz - eiz.Inverse();
		c = c / cuComplex(0.f, 2.f);
		return c;
	}
	inline __device__ cuComplex cos()
	{
		cuComplex c, eiz(0.f,1.f);
		eiz = (eiz * (*this)).exp(); 
		c = eiz + eiz.Inverse();
		c = c / cuComplex(2.f, 0.f);
		return c;
	}

	inline __device__ cuComplex ch()
	{
		cuComplex c, ez(1.f,0.f);
		ez = (ez * (*this)).exp(); 
		c = ez + ez.Inverse();
		c = c / cuComplex(2.f, 0.f);
		return c;
	}
	inline __device__ cuComplex sh()
	{
		cuComplex c, ez(1.f,0.f);
		ez = (ez * (*this)).exp(); 
		c = ez - ez.Inverse();
		c = c / cuComplex(2.f, 0.f);
		return c;
	}

	inline __device__ cuComplex sqrt(bool root = false)
	{
		cuComplex c;
		float r = sqrtf(this->d());
		float a = (this->arg() + twoPI*root)/2;
		c.y = __sinf(a)*r;
		c.x = __cosf(a)*r;
		return c;
	}

	inline __device__ cuComplex asin(bool root = false)
	{
		cuComplex s = (-(*this)*(*this) + 1.f).sqrt(root);
		return cuComplex(0.f, -1.f)*(*this*cuComplex(0.f, 1.f) + s).Ln();
	}
	inline __device__ cuComplex acos(bool root = false, bool sign = true)
	{
		return cuComplex(halfPI, 0.f) - asin(root);
	}

	inline __device__ cuComplex atan()
	{
		cuComplex iz = (*this)*i();
		return ((-iz + 1.0f).Ln() - (iz + 1.0f).Ln())*(cuComplex(0.0f, 0.5f));
	}

	inline __device__ cuComplex pow(float a, float b = 0.0f)
	{
		float sr = x*x+y*y;
		float r, fi;
		fi = this->arg();
		r = powf(sr, a/2.0f)/(__expf(fi*b));
		fi = a*fi + b*__logf(sr)/2.0f;

		cuComplex res;
		res.x = r*__cosf(fi);
		res.y = r*__sinf(fi);
		return res;
	}
	inline __device__ cuComplex pow(cuComplex val)
	{
		return pow(val.x, val.y);
	}
	inline __device__ cuComplex pow(int val)
	{
		cuComplex c = *this;
		bool minus = val < 0;
		val = minus ? -val : val;
		for (int i = 1; i < val; i++) c *= *this;
		c = minus ? c.Inverse() : c;
		return c;
	}

	inline __device__ cuComplex сопряжение()
	{
		return cuComplex(x, -y);
	}
};