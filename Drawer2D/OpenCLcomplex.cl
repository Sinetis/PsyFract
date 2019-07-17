
#define PI 3.1415926535897932384626433832795f
#define twoPI 6.283185307179586476925286766559f
#define halfPI 1.5707963267948966192313216916398f

struct clComplex
{
	float x;
		float& Re() {return x;}
	float y;
		float& Im() {return y;}
		
	clComplex(float re = 0, float im = 0)
		:x(re), y(im)
	{	}
	
	clComplex(const clComplex& val)
		:x(val.x), y(val.y)
	{	}

	inline clComplex operator= (const clComplex& val)
	{
		this->x = val.x;
		this->y = val.y;
		return val;
	}
	
	inline clComplex operator+ (const clComplex& val) const
	{
		return clComplex(this->x + val.x, this->y + val.y);
	}
	inline clComplex operator+= (const clComplex& val)
	{
		return *this = *this + val;
	}

	inline clComplex operator- (const clComplex& val) const
	{
		return clComplex(this->x - val.x, this->y - val.y);
	}
	inline clComplex operator- ()
	{
		return clComplex(this->x = -x, this->y = -y);
	}
	
	inline clComplex operator* (const clComplex& val) const
	{
		// (x1 + iy1) * (x2 + iy2)
		clComplex res;
		res.x = this->x * val.x - this->y * val.y;
		res.y = this->x * val.y + this->y * val.x;
		return res;
	}
	inline clComplex operator*= (const clComplex& val)
	{
		return *this = *this * val;
	}
	
	inline clComplex operator/ (const clComplex& val) const
	{
		// (x1 + iy1) / (x2 + iy2)
		clComplex res;
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

	inline clComplex operator +(float val)
	{
		return clComplex(this->x+val, this->y);
	}

	inline clComplex operator +=(float val)
	{
		this->x += val;
		return *this;
	}
	inline clComplex operator -(float val)
	{
		return clComplex(this->x-val, this->y);
	}

	inline clComplex operator -=(float val)
	{
		this->x -= val;
		return *this;
	}

	inline clComplex operator *(float val)
	{	
		return clComplex(this->x*val, this->y*val);
	}

	inline clComplex operator /(float val)
	{
		return clComplex(this->x/val, this->y/val);
	}
	inline clComplex operator /=(float val)
	{ return *this = *this / val; }

	inline clComplex operator =(float val)
	{
		x = val;
		y = 0;
		return *this;
	}
	inline clComplex operator^ (int val)
	{
		clComplex c = *this;
		bool minus = val < 0;
		val = minus ? -val : val;
		for (int i = 1; i < val; i++) c *= *this;
		c = minus ? c.Inverse() : c;
		return c;
	}

	inline static clComplex Zero()
	{
		return clComplex(0,0);
	}

	// Поворот в радианах
	inline void Rotate(float fi)
	{
		float c = __cosf(fi), s = __sinf(fi);
		float xx = this->x;
		this->x = x*c - y*s;
		this->y = xx*s + y*c;
	}

	inline void Scale(float k)
	{
		this->x *= k;
		this->y *= k;
	}

	inline void Shift(clComplex c)
	{
		Shift(c.x, c.y);
	}
	inline void Shift(float cx, float cy)
	{
		this->x += cx;
		this->y += cy;
	}

	inline clComplex Inverse()
	{
		clComplex c;
		float m = x*x + y*y;
		c.x /= m;
		c.y /= -m;
		return c;
	}

	inline clComplex exp()
	{
		clComplex c;
		float e;
		e = __expf(x); 
		c.x = e*__cosf(y);
		c.y = e*__sinf(y);
		return c;
	}

	inline clComplex Ln()
	{
		clComplex c;
		c.x = __logf(this->d());
		c.y = atanf(this->y / this->x);
		return c;
	}
	
	inline clComplex sin()
	{
		clComplex c, eiz(0.f,1.f);
		eiz = (eiz * (*this)).exp(); 
		c = eiz - eiz.Inverse();
		c = c / clComplex(0.f, 2.f);
		return c;
	}
	inline clComplex cos()
	{
		clComplex c, eiz(0.f,1.f);
		eiz = (eiz * (*this)).exp(); 
		c = eiz + eiz.Inverse();
		c = c / clComplex(2.f, 0.f);
		return c;
	}

	inline clComplex ch()
	{
		clComplex c, ez(1.f,0.f);
		ez = (ez * (*this)).exp(); 
		c = ez + ez.Inverse();
		c = c / clComplex(2.f, 0.f);
		return c;
	}
	inline clComplex sh()
	{
		clComplex c, ez(1.f,0.f);
		ez = (ez * (*this)).exp(); 
		c = ez - ez.Inverse();
		c = c / clComplex(2.f, 0.f);
		return c;
	}

	inline clComplex sqrt(bool root = false)
	{
		clComplex c;
		float r = sqrtf(this->d());
		float a = (this->arg() + twoPI*root)/2;
		c.y = __sinf(a)*r;
		c.x = __cosf(a)*r;
		return ;
	}

	inline clComplex asin(bool root = false)
	{
		clComplex s = (-(*this)*(*this) + 1.f).sqrt(root);
		return clComplex(0.f, -1.f)*(*this*clComplex(0.f, 1.f) + s).Ln();
	}
	inline clComplex acos(bool root = false, bool sign = true)
	{
		return clComplex(halfPI, 0.f) - asin(root);
	}
};

