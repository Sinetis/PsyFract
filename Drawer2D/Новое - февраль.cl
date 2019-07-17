#define PI 3.14159265358979323846264338328f
#define twoPI 6.28318530717958647692528676656f
#define halfPI 1.57079632679489661923132169164f
#define sqrt2PI 2.5066282746310005024157652848f
#define sin native_sin
#define cos native_cos
#define exp native_exp
#define log native_log
//#define float half
//#define float2 half2
float2 cmul(float2 f, float2 s);
float2 cdiv(float2 this,float2 val);

float carg(float2 f)
{
	if (f.x > 0)
		return atan(f.y/f.x);
	if (f.y > 0)
		return atan(f.y/f.x)+PI;
	else
		return atan(f.y/f.x)-PI;
}

float2 cmul(float2 f, float2 s)
{
	float2 r;
	//(f.x + if.y)*(s.x + is.y)
	r.x = f.x*s.x - f.y*s.y;
	r.y = f.x*s.y + s.x*f.y;
	return r;
}

float2 cpow2(float2 f)
{
	return cmul(f,f);
}

float2 ccube(float2 f)
{
	return cmul(cmul(f,f),f);
}

float2 cdiv(float2 this,float2 val)
{
	// (x1 + iy1) / (x2 + iy2)
	float2 res;
	float f = val.x*val.x + val.y*val.y;
	res.x = (this.x * val.x + this.y * val.y) / f;
	res.y = (-this.x * val.y + this.y * val.x) / f;
	return res;
}

float2 cinv(float2 f)
{
	float2 c = (float2)(f.x, f.y);
	float m = f.x*f.x + f.y*f.y;
	c.x /= m;
	c.y /= -m;
	return c;
}

float2 cexp(float2 f)
{
	float2 r;
	float e;
	e = exp(f.x); 
	r.x = e*cos(f.y);
	r.y = e*sin(f.y);
	return r;
}

float2 csin(float2 f)
{
	float2 r, eiz = (float2)(0,1);
	eiz = cexp(cmul(eiz,f)); 
	r = eiz - cinv(eiz);
	r = cdiv(r, (float2)(0, 2));
	return r;
}
float2 ccos(float2 f)
{
	float2 c, eiz = (float2)(0.f,1.f);
	eiz = cexp(cmul(eiz,f));  
	c = eiz + cinv(eiz);
	c = cdiv(c, (float2)(2, 0));
	return c;
}

float2 cch(float2 f)
{
	float2 c, ez = (float2)(1.f,0.f);
	ez = cexp(cmul(ez, f));
	c = ez + cinv(ez);
	c = cdiv(c, (float2)(2.f, 0.f));
	return c;
}

float2 clog(float2 f)
{
	float2 c;
	c.y = carg(f);
	c.x = log(fast_length(f));
	return c;
}

// Поворот в радианах
float2 crotate(float2 f,float fi)
{
	float2 res;
	float c = cos(fi), s = sin(fi);
	res.x = f.x*c - f.y*s;
	res.y = f.x*s + f.y*c;
	return res;
}

float2 fx(float f)
{return (float2)(f,0);}
float2 fy(float f)
{return (float2)(0,f);}
float2 im(float2 f)
{return (float2)(f.x,0);}
float2 re(float2 f)
{return (float2)(0,f.y);}

struct data
{
	int iter;
	float k;
	unsigned int frac;   // bits: ----.----  ----.--33  3333.3322  2222.2210
				// where: 0 - type, 1 - two frac, 2 - first frac, 3 - second frac 
	unsigned int col;
};
bool type(struct data st)
{
	return st.frac&1;
}
bool twoFrac(struct data st)
{
	return st.frac&2;
}
int firstFrac(struct data st)
{
	return (st.frac>>2)%256;
}
int secondFrac(struct data st)
{
	return (st.frac>>10)%256;
}

float2 Fracs(float2 z, float2 C, __global float2*c, int frac, int k)
{
	switch (frac)
	{
	case 1:
		//return z - ((-z*z).exp() - 1.0f)/C + c[1].x;
		return z - cdiv(cexp(cmul(-z,z))-fx(1),C) + fx(c[1].x);
	case 2:
	//	return z*z*z.Ln() + C;
		return cmul(cpow2(z),clog(z)) + C;
	case 3:
	//	return z*z.exp()*c[2].x + z*z.Ln()*c[3].x + C;
		return cmul(z, cexp(z))*c[2].x + cmul(z, clog(z))*c[3].x + C;
	case 4:
	//	return z - z(2.0f+c[0].y/10.f)*((z*z*z*z - 1.0f)/(z*z*z*4.0f - c[1].y)) + C;
		return z - (2+c[0].y/10)*cdiv(cpow2(cpow2(z))-fx(1),4*cmul(cpow2(z),z) - fx(c[1].y)) + C; 
	case 5:
	//	return z - z(2.0f)*((z*z*z - 1.0f)/(z*z*3.0f)) + C;
		return z - 2*cdiv(ccube(z), 3*cmul(z,z)) + C;
	case 6:
	//	return z - (z(0.7f + c[2].y*2.f)*(z*z*z - 1.0f)/(z*z*3.f)).pow(2) + C; 
		return z - cpow2((0.7f + 2*c[2].y)*cdiv(ccube(z)-fx(1), 3*cpow2(z))) + C;
	case 7:
	//	return (z*z*z*2.0f-fx(c[2]))/(z*z*3.0f + im(c[0] * c[3])) + cuComplex(0.0f,c[3].y/10.0f).Rotate(k/PI);
		return cdiv(2*ccube(z) - fx(c[2].x), 3*cpow2(z) + re(cmul(c[0],c[3]))) + 
									crotate((float2)(0.0f, c[3].y/10.0f), k/PI);
	case 8:
	//	return z - (z.exp()/z + C)/(z.exp()/z - z.exp()/(z*z));
		return z - cdiv(cdiv(cexp(z),z) + C, cdiv(cexp(z),z) - cdiv(cexp(z),cpow2(z)));
	case 9:
	//	return (z+c[1].x).exp() + c[0];
		return cexp(z + re(c[1])) + cmul(re(c[0]),C);
	case 10:
	//	return (z+c[3].x).Ln() + c[2];
		return clog(z + re(c[3])) + re(c[2]);
	case 11:
	//	return (z+c[2].x).sin() + c[1];
		return csin(z + re(c[2])) + re(c[1]);
	case 12:
	//	return (z+c[0].x).ch() + c[3];
		return cch(z + re(c[0])) + re(c[3]);
	case 13:
	//	return (z+c[2].x).acos() + c[1];
	case 14:
	//	return z - z.exp() + C;
		return z - cexp(z) + C;
	case 15:
	//	return c[0] + c[1]*z + c[2]*z*z + c[3]*z*z*z + C;
		return c[0] + cmul(c[1],z) + cmul(c[2],cpow2(z)) + cmul(c[3],cmul(z,cpow2(z))) + C;
	case 16:
	//	return z - z.sin() + C;
		return z - csin(z) + C;
	case 17:
	//	return z - z.ch() + C;
		return z - cch(z) + C;
	case 18:
	//	return z - (z+c[1].d()).sin()*(c[0].y+c[2].x) + C + c[1].x;
		return z - cmul(csin(z+fx(fast_length(c[1]))), fx(c[0].y+c[2].x)) + C + fx(c[1].x);
	case 19:
	//	return (-z*z).exp() + C;
		return cexp(cmul(-z,z)) + C;
	case 20:
	//	return (z+c[0].y)*c[0].x + (z/(C - c[2].y))*c[2].x + (z*z)*c[3].x + C;
		return (z+fx(c[0].y))*c[0].x + cdiv(z, C - fx(c[2].y))*c[2].x + cpow2(z)*c[3].x + C;
	case 21:
	//	return cuComplex(__cosf(z.x), -__sinf(z.y));
		return (float2)(cos(z.x),-sin(z.y));
	case 22:
	//	return c[0]*(z.i()*C*z).exp() + c[1]*(-z.i()*C*z).exp() + (C*C).Inverse(); 
		return cmul(c[0], cexp(cmul(cmul(fy(1.0f),C),z))) 
				+ cmul(c[1], cexp(cmul(cmul(fy(-1.0f),C),z))) + cinv(cpow2(C));
	case 23:
	//	return (z+c[2].x).sin()*(z+c[1].y).cos() + C;
		return cmul(csin(z+fx(c[2].x)), ccos(z+fx(c[1].y))) + C;
	case 24:
	//	return c[0]*((z*z).exp()) + c[1]*(z.exp()) + C + c[2]*z + c[3]*z*z;  
		return cmul(c[0],cexp(cmul(z,z))) + cmul(c[1], cexp(z)) + C + cmul(c[2],z) + cmul(c[3],cpow2(z));
	case 25:
	//	return (-z*z).exp()*(1/(sqrt2PI)) + C;
		return cexp(cmul(-z,z))/sqrt2PI + C;
	case 26:
	//	return z - ((-z*z).exp() + c[2].x)/(-z(2)*z*(-z*z).exp()) + C;
		return z - cdiv(cexp(cmul(-z,z)) + fx(c[2].x), -2*cmul(z,cexp(cmul(-z,z))))+C;
	case 27:
	//	return z*z - ((-z*z).exp() + c[2].x)/(-z(2)*z*(-z*z).exp()) + C*C;
		return cpow2(z) - cdiv(cexp(cmul(-z,z))+fx(c[2].x), cmul(-2*z,cexp(cmul(-z,z)))) + cpow2(C);
	case 28:
	//	return z*z + C;
		return cmul(z,z) + C;
	case 29:
		{
			float2 nz = cmul(z,z);
			float x = nz.x, y = nz.y;
			return (float2)(3*x*x - 6*x*y - y*y, 3*x*x + 6*x*y - 3*y*y) + C;
		}
	case 30:
	//	return (c[0]-c[1]+c[2]-c[3])*z*(z(1)-z) + C;
		return cmul(cmul(c[0]-c[1]+c[2]-c[3],z),fx(1)-z) + C;
	case 31:
	//	return z + z.cos()/z.sin() + C;
		return z + cdiv(ccos(z), csin(z)) + C;
	case 32:
		return cch(z + fx(c[3].x))*c[0].y + csin(cmul(cmul(z, fx(c[1].x)), cexp(C - fx(c[1].y)))) + C;
	default:
	//	return ((z*z + C + c[0].y) / (z*2.0f + C - 2 + c[2].y)).pow(2) + 1 + c[1].x;
		return cpow2(cdiv(cpow2(z) + C + fx(c[0].y), 2*z + C - fx(2 + c[2].y))) + fx(1 + c[1].x);
	}
}

float2 func(float2 z, float2 C, __global float2* c, struct data st)
{
	float2 r;
	r = Fracs(z, C, c, firstFrac(st), st.k);
	if (twoFrac(st))
		r = Fracs(r, C, c, secondFrac(st), st.k);
	return r;
}

struct data changeJul(struct data d, int f1, int f2)
{
	struct data nd = d;
	nd.frac = 0 + 2 + (f1<<2) + (f2<<10);
	return nd;
}
struct data changeMand(struct data d, int f1, int f2)
{
	struct data nd = d;
	nd.frac = 1 + 2 + (f1<<2) + (f2<<10);
	return nd;
}
struct data changeOne(struct data d, int t, int f)
{
	struct data nd = d;
	nd.frac = t + 0 + (f<<2) + 0;
	return nd;
}

struct data changeData(struct data d)
{
	if (twoFrac(d)==0)
		switch(firstFrac(d))
		{//changeOne
			case 0: return changeOne(d,0,7);
		}
	else
		if (type(d))
			switch(firstFrac(d))
			{//changeMand
				case 0: return changeMand(d,0,0);
			}
		else
		switch(firstFrac(d))
			{//changeJul
				case 0: return changeJul(d,0,5);
				case 1: return changeJul(d,6,0);
				//case 2: return changeJul(d,0,6);
				case 3: return changeJul(d,7,0);
				case 4: return changeJul(d,8,0);
				case 5: return changeJul(d,0,8);
				case 6: return changeJul(d,17,18);
				case 7: return changeJul(d,18,17);
				case 8: return changeJul(d,18,27);
				case 9: return changeJul(d,27,18);
				case 10: return changeJul(d,28,18);
				case 11: return changeJul(d,18,28);
				case 12: return changeJul(d,18,8);
				case 13: return changeJul(d,8,18);
				case 14: return changeJul(d,18,6);
				case 15: return changeJul(d,6,13);
				case 16: return changeJul(d,13,6);
				case 17: return changeJul(d,6,8);
				case 18: return changeJul(d,6,16);
				case 19: return changeJul(d,6,17);
				case 20: return changeJul(d,6,26);
				case 21: return changeJul(d,6,30);
				case 22: return changeJul(d,6,31);
				case 23: return changeJul(d,17,20);
				case 24: return changeJul(d,17,8);
				case 2: return changeJul(d,8,17);
				case 26: return changeJul(d,15,8);
				case 25: return changeJul(d,4,11);
				case 27: return changeJul(d,26,18);
				case 28: return changeJul(d,17,4);
				case 29: return changeJul(d,26,4);
				case 30: return changeJul(d,8,4);
			}
}

__kernel void OpenCLFractal(
	__global uchar4* pix, float4 posData, int xPixels,struct data st, __global float2* c)
{
	unsigned int	idx = get_global_id(0),
					idy = get_global_id(1);
	float	x = posData.y + posData.w*(idy),
			y = posData.x + posData.w*(idx);
	//st.k = 0;
	st = changeData(st);
	
	float2 C, z, zn;
	if (type(st))
	{
		C = (float2)(x,y);
		z = (float2)(c[0].x,0);
	}
	else
	{
		z = (float2)(x,y);
		C = (float2)(c[0].x,0);
	}

	int it = 0;
	float s;
	do
	{
		zn = z;	
		z = z - func(z, C, c, st);
		it++;
		s = fast_distance(z,zn);
	}
	while (it < st.iter && s > 1e-6);//  && s < 1e+6);
	
	int pos = idx + idy*xPixels;
	float color = it*2.0f + 7*c[0].x + st.k;
	uchar r,g,b;
	r = (uchar)((127 * cos(color/17.0f+4*c[1].x)) + 128);
	g = (uchar)((127 * cos(color/11.0f+4*c[2].x)) + 128);
	b = (uchar)((127 * cos(color/7.0f+4*c[3].x)) + 128);
	pix[pos].x = st.col&1 ? r : 255 - r;
	pix[pos].y = st.col&2 ? g : 255 - g;
	pix[pos].z = st.col&4 ? b : 255 - b;
	pix[pos].w = (r+g+b)/3;
}