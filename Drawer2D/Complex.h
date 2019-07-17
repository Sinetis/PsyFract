#pragma once

#include "stdafx.h"
#include "Constants.h"

#define PI 3.1415926535897932384626433832795f
#define twoPI 6.283185307179586476925286766559f
#define halfPI 1.5707963267948966192313216916398f

struct float2
{
public:
	float x;
	float y;
};

struct Complex
{
	float x;
		float& Re() { return x; }
	float y;
		float& Im() { return y; }

	Complex(float re = 0, float im = 0)
		:x(re), y(im)
	{	}
	
	Complex(const Complex& val)
		:x(val.x), y(val.y)
	{	}

	inline Complex operator= (const Complex& val)
	{
		this->x = val.x;
		this->y = val.y;
		return val;
	}
	
	inline Complex operator+ (const Complex& val) const
	{
		return Complex(this->x + val.x, this->y + val.y);
	}
	inline Complex operator+= (const Complex& val)
	{
		return *this = *this + val;
	}

	inline Complex operator- (const Complex& val) const
	{
		return Complex(this->x - val.x, this->y - val.y);
	}
	inline Complex operator- ()
	{
		return Complex(this->x = -x, this->y = -y);
	}
	
	inline Complex operator* (const Complex& val) const
	{
		// (x1 + iy1) * (x2 + iy2)
		Complex res;
		res.x = this->x * val.x - this->y * val.y;
		res.y = this->x * val.y + this->y * val.x;
		return res;
	}
	inline Complex operator*= (const Complex& val)
	{
		return *this = *this * val;
	}
	
	inline Complex operator/ (const Complex& val) const
	{
		// (x1 + iy1) / (x2 + iy2)
		Complex res;
		float f = val.s();
		res.x = (this->x * val.x + this->y * val.y) / f;
		res.y = (this->x * val.y - this->y * val.x) / f;
		return res;
	}

	//inline operator float () const
	//{
	//	return sqrtf(this->x * this->x + this->y * this->y);
	//}
	// Модуль числа
	inline float d() const{return sqrtf(this->x * this->x + this->y * this->y);}
	// Квадрат модуля числа
	inline float s() const
	{return this->x * this->x + this->y * this->y;}
	// Аргумент числа
	inline float arg() const
	{return atanf(y/x);}

	inline Complex operator +(float val)
	{
		return Complex(this->x+val, this->y);
	}

	inline Complex operator +=(float val)
	{
		this->x += val;
		return *this;
	}
	inline Complex operator -(float val)
	{
		return Complex(this->x-val, this->y);
	}

	inline Complex operator -=(float val)
	{
		this->x -= val;
		return *this;
	}

	inline Complex operator *(float val)
	{	
		return Complex(this->x*val, this->y*val);
	}

	inline Complex operator /(float val)
	{
		return Complex(this->x/val, this->y/val);
	}
	inline Complex operator /=(float val)
	{ return *this = *this / val; }

	inline Complex operator =(float val)
	{
		x = val;
		y = 0;
		return *this;
	}
	
	Complex operator^ (int val)
	{
		Complex c = *this;
		bool minus = val < 0;
		val = minus ? -val : val;
		for (int i = 1; i < val; i++) c *= *this;
		c = minus ? c.Inverse() : c;
		return c;
	}

	inline static Complex Zero()
	{
		return Complex(0,0);
	}

	// Поворот в радианах
	inline void Rotate(float fi)
	{
		float c = cosf(fi), s = sinf(fi);
		float xx = this->x;
		this->x = x*c - y*s;
		this->y = xx*s + y*c;
	}

	inline void Scale(float k)
	{
		this->x *= k;
		this->y *= k;
	}

	inline void Shift(Complex c)
	{
		Shift(c.x, c.y);
	}
	inline void Shift(float cx, float cy)
	{
		this->x += cx;
		this->y += cy;
	}

	inline Complex Inverse()
	{
		Complex c;
		float m = x*x + y*y;
		c.x /= m;
		c.y /= -m;
		return c;
	}

	inline Complex exp()
	{
		Complex c;
		float e;
		e = expf(x); 
		c.x = e*cosf(y);
		c.y = e*sinf(y);
		return c;
	}

	inline Complex Ln()
	{
		Complex c;
		c.x = logf(this->d());
		c.y = atanf(this->y / this->x);
		return c;
	}
	
	inline Complex sin()
	{
		Complex c, eiz(0.f,1.f);
		eiz = (eiz * (*this)).exp(); 
		c = eiz - eiz.Inverse();
		c = c / Complex(0.f, 2.f);
		return c;
	}
	inline Complex cos()
	{
		Complex c, eiz(0.f,1.f);
		eiz = (eiz * (*this)).exp(); 
		c = eiz + eiz.Inverse();
		c = c / Complex(2.f, 0.f);
		return c;
	}

	inline Complex ch()
	{
		Complex c, ez(1.f,0.f);
		ez = (ez * (*this)).exp(); 
		c = ez + ez.Inverse();
		c = c / Complex(2.f, 0.f);
		return c;
	}
	inline Complex sh()
	{
		Complex c, ez(1.f,0.f);
		ez = (ez * (*this)).exp(); 
		c = ez - ez.Inverse();
		c = c / Complex(2.f, 0.f);
		return c;
	}

	inline Complex sqrt(bool root = false)
	{
		Complex c;
		float r = sqrtf(this->d());
		float a = (this->arg() + twoPI*root)/2;
		c.y = sinf(a)*r;
		c.x = cosf(a)*r;
		return c;
	}

	inline Complex asin(bool root = false)
	{
		Complex s = (-(*this)*(*this) + 1.f).sqrt(root);
		return Complex(0.f, -1.f)*(*this*Complex(0.f, 1.f) + s).Ln();
	}
	inline Complex acos(bool root = false, bool sign = true)
	{
		return Complex(halfPI, 0.f) - asin(root);
	}
};

class ComplexArr
{
	Complex *arr;
	int num;

public:
	ComplexArr()
	{
		arr = 0;
		num = 0;
	}

	ComplexArr(int num)
	{
		this->num = num;
		arr = new Complex[num];
	}

	ComplexArr(const ComplexArr& C)
	{
		num = C.num;
		arr = new Complex[num];
		for (int i = 0; i < num; i++)
			arr[i] = C.arr[i];
	}

	ComplexArr(float *reArr, int n)
	{
		this->num = n;
		arr = new Complex[num];
		for (int i = 0; i < n; i++)
			arr[i].x = reArr[i];
	}

	ComplexArr(int *reArr, int n)
	{
		this->num = n;
		arr = new Complex[num];
		for (int i = 0; i < n; i++)
			arr[i].x = reArr[i];
	}

	Complex& operator[](int i)
	{
		if (i < 0 && i >= num)
			throw L"Out of Range in ComplexArr";
		return arr[i];
	}

	ComplexArr operator =(const ComplexArr& C)
	{
		num = C.num;
		if (arr)
			delete []arr;
		arr = new Complex[num];
		for (int i = 0; i < num; i++)
			arr[i] = C.arr[i];
		return *this;
	}

private:
	ComplexArr FFT(ComplexArr P, bool D)
	{
		int i, N = P.num;
		if (N == 1)
			return P;
		ComplexArr y(N), y0, y1;
		float a = 2 * PI / N;
		if (!D)
			a = -a;
		ComplexArr A0(N/2), A1(N/2);
		Complex Wn(cos(a), sin(a)), W(1.0, 0.0);
		for (i = 0; i < N/2; i++)
		{
			A0[i] = P[2 * i];
			A1[i] = P[2 * i + 1];
		}
		y0 = FFT(A0, D);
		y1 = FFT(A1, D);
		for (i = 0; i < N/2; i++)
		{
			y[i] = y0[i] + W * y1[i];
			y[i + N/2] = y0[i] - W * y1[i];
			W = W * Wn;
		}

		//delete []A0.arr;
		//delete []A1.arr;
		//delete []y0.arr;
		//delete []y1.arr;
		return y;
	}

public:
	ComplexArr F_F_T(bool D)
	{
		int n = 1;
		while (n < this->num)
			n *= 2;
		ComplexArr Q(n);
		for (int i = 0; i < this->num; i++)
			Q[i] = (*this)[i];
		Q = FFT(Q, D);
		if (!D)
			for (int i = 0; i < n; i++)
				Q[i] /= n;
		return Q;
	}

	int* Mod()
	{
		//srand(time(NULL));
		ComplexArr q = F_F_T(1);
		int* a = new int[num];
		for (int i = 0; i < num; i++)
			a[i] = (int)(q.arr[i].d());

		return a;
	}

	//void main()
	//{
	//	srand(time(NULL));
	//	int i, j, n = 100, m = 10000, k, l;
	//	float s1, s2;
	//	Polynom<complex> A(n), B(n), C(n);
	//	for (i = 0; i < n; i++)
	//		A[i] = rand() % 100;

	//	C = F_F_T(A, 1);


	//	s1 = s2 = 0;
	//	for (i = 0; i < A.degree(); i++)
	//		s1 = s1 + fabs(A[i]) * fabs(A[i]);
	//	for (i = 0; i < C.degree(); i++)
	//		s2 = s2 + fabs(C[i]) * fabs(C[i]);
	//	//cout << s1 << " " << s2/C.degree() << endl;

	//	getch();

	~ComplexArr()
	{
		if (arr)
			delete []arr;
	}
};

static class FFT
{

private:
	static void Rearrange(Complex *samples, int length) 
	{
	static int rearrangeSize=0; // Размер таблицы перестановки.
	static int *rearrange=0;
		if (rearrangeSize != length) 
		{
			// Производим перерасчет каждый раз при изменении размера.
			if (rearrange) 
				delete [] rearrange;
			rearrange = new int[length];
			// Заполняем назначение каждой величины.
			rearrange[0] = 0;
			for(int limit=1, bit=length/2; limit<length; limit <<= 1, bit>>=1 )
				for(int i=0;i<limit;i++)
					rearrange[i+limit] = rearrange[i] + bit;
			// Помещаем 0 в те элементы, которые остаются
			// без изменений.
			// Также обнуляем один из элементов так,
			// чтобы каждый обмен происходил только один раз.
			for(int i=0; i<length; i++) 
			{
				if (rearrange[i] == i) rearrange[i] = 0;
				else rearrange[ rearrange[i] ] = 0;
			}
			rearrangeSize = length;
		}
		// Используем таблицу перестановки для перестановки элементов.
		// Нулевые индексы просто пропускаются.
		Complex t;
		for (int i=0; i<length; i++)
			if (rearrange[i]) 
			{ // Этот элемент надо переставлять?
				t = samples[i]; // Да, переставляем.
				samples[i] = samples[ rearrange[i] ];
				samples[ rearrange[i] ] = t;
			}
	}

	static void ForwardFft(Complex *samples, int length ) 
	{
		Rearrange(samples,length);
		for(int halfSize=1; halfSize < length; halfSize *= 2) 
		{
			Complex phaseShiftStep = Complex(0.f,-PI/halfSize).exp();//polar(1.0,-PI/halfSize);
			Complex currentPhaseShift(1,0);
			for(int fftStep = 0; fftStep < halfSize; fftStep++) 
			{
				for(int i=fftStep; i < length; i += 2*halfSize) 
				{
					Complex t = currentPhaseShift *
						samples[i+halfSize];
					samples[i+halfSize] = samples[i] - t;
					samples[i] += t;
				}
				currentPhaseShift *= phaseShiftStep;
			}
		}
	}

public:
	static int* Return(int* samples, int length)
	{
		int i;
		Complex *ComSamples = new Complex[length];
		for (i = 0; i < length; i++)
			ComSamples[i].x = samples[i];

		ForwardFft(ComSamples, length);

		int *NewSamples = new int[length];
		for (i = 0; i < length; i++)
			NewSamples[i] = (int)ComSamples[i].d();

		delete[] ComSamples;
		return NewSamples;
	}

	static Complex* ReturnFull(int* samples, int length)
	{
		Complex *ComSamples = new Complex[length];
		for (int i = 0; i < length; i++)
			ComSamples[i].x = samples[i];

		ForwardFft(ComSamples, length);

		return ComSamples;
	}

	static float* ReturnFloat(int* samples, int length)
	{
		int i;
		Complex *ComSamples = new Complex[length];
		for (i = 0; i < length; i++)
			ComSamples[i].x = samples[i];

		ForwardFft(ComSamples, length);

		float *NewSamples = new float[length];
		for (i = 0; i < length; i++)
			NewSamples[i] = ComSamples[i].d();

		delete[] ComSamples;
		return NewSamples;
	}
};
