#define PI 3.14159265358979323846264338328f
#define twoPI 6.28318530717958647692528676656f
#define halfPI 1.57079632679489661923132169164f
#define sqrt2PI 2.5066282746310005024157652848f
#define sin native_sin
#define cos native_cos
#define exp native_exp
#define log native_log
#define length fast_length
#define distance fast_distance
//#define float half
//#define float2 half2
float2 cmul(float2 f, float2 s);
float2 cdiv(float2 this,float2 val);

float2 fx(float f)
{return (float2)(f,0);}
float2 fy(float f)
{return (float2)(0,f);}
float2 re(float2 f)
{return (float2)(f.x,0);}
float2 im(float2 f)
{return (float2)(0,f.y);}

float clen(float2 f)
{
	return sqrt(f.x*f.x + f.y*f.y);
}

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

float2 cpow(float2 this, float2 val)
{
	float sr = this.x*this.x+this.y*this.y;
	float r, fi;
	fi = carg(this);
	r = pow(sr, val.x/2.0f)/(exp(fi*val.y));
	fi = val.x*fi + val.y*log(sr)/2.0f;

	float2 res;
	res.x = r*cos(fi);
	res.y = r*sin(fi);
	return res;
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
	c.x = log(length(f));
	return c;
}

float2 csqrt(float2 f)
{
	float2 c;
	float r = sqrt(length(f));
	float a = (carg(f))/2;
	c.y = sin(a)*r;
	c.x = cos(a)*r;
	return c;
}

float2 casin(float2 f)
{
	float2 s = csqrt(fx(1)-cpow2(f));
	return cmul(fy(-1),clog(cmul(f,fy(1))+s));
}

float2 cacos(float2 f)
{
	return fx(halfPI) - asin(f);
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
		return z - cdiv(cexp(cmul(-z,z))-fx(1 + c[0].x/3.0f),C) + fx(c[0].y);
	case 2:
		return cmul(cpow2(z+fx(c[1].x)),clog(z)) + C;
	case 3:
		return cmul(z, cexp(z))*c[1].y + cmul(z, clog(z))*c[2].x + C;
	case 4:
		return z - (2+c[2].y/10)*cdiv(cpow2(cpow2(z))-fx(1+c[3].x/3.5f),4*cmul(cpow2(z),z) - fx(c[1].x)) + C; 
	case 5:
		return z - (2-c[0].y)*cdiv(ccube(z), 3*cmul(z,z)) + C;
	case 6: 
		return z - cpow2((0.7f + 2*c[0].x)*cdiv(ccube(z)-fx(1), 3*cpow2(z))) + C;
	case 7:
		return cdiv(2*ccube(z) - fx(c[0].y), 3*cpow2(z) + re(cmul(c[0],c[3]))) + 
									crotate((float2)(0.0f, c[1].x/10.0f), k/PI);
	case 8:
		return z - cdiv(cdiv(cexp(z+fx(c[1].y)),z) + C, 
						cdiv(cexp(z),z) - cdiv(cexp(z-fx(c[2].x)),cpow2(z)));
	case 9:
		return cexp(z + re(c[1])) + cmul(re(c[0]),C);
	case 10:
		return clog(z + re(c[3])) + re(c[2]);
	case 11:
		return csin(z + re(c[2])) + re(c[1]);
	case 12:
		return cch(z + re(c[0])) + re(c[3]);
	case 13:
		return cacos(z+re(c[2]))+C;
	case 14:
		return z - cexp(z+fx(c[0].x)) + C;
	case 15:
		return fx(c[0].y) + cmul(re(c[1]),z) + cmul(re(c[2]),cpow2(z)) 
		+ cmul(re(c[3]),cmul(z,cpow2(z))) + C;
	case 16:
		return z - csin(z-re(c[2]/2)) + C;
	case 17:
		return z - cch(z+fx(c[1].y*c[0].x)) + C;
	case 18:
		return z - cmul(csin(z+fx(fast_length(c[1]))), fx(c[0].y+c[2].x)) + C + fx(c[1].x);
	case 19:
		return cexp(cmul(-z,z)) + C + fx(c[3].y);
	case 20:
		return (z+fx(c[0].y))*c[0].x + cdiv(z, C - fx(c[2].y))*c[2].x + cpow2(z)*c[3].x + C;
	case 21:
		return (float2)(cos(z.x),-sin(z.y));
	case 22:
		return c[0].y*cexp(cmul((1.0f+c[2].x)*C,z)) 
				+ (cdiv(1,c[1])).x*cexp(cmul((-1.0f - (cmul(c[2],c[1])).x)*C,z)) + cinv(cpow2(C));
	case 23:
		return cmul(csin(z+fx(c[2].x)), ccos(z+fx(c[1].y))) + C;
	case 24:  
		return c[0].x*cexp(cmul(z,z)) + c[1].y*cexp(z) + C + c[2].x*z + c[3].y*cpow2(z);
	case 25:
		return cexp(cmul(-z,z+fx(c[0].y)))/sqrt2PI + C;
	case 26:
		return z - cdiv(cexp(cmul(-z,z)) + fx(c[2].x), -2*cmul(z,cexp(cmul(-z,z-fx(c[0].x)))))+C;
	case 27:
		return cpow2(z) - cdiv(cexp(cmul(-z,z))+fx(c[2].x), cmul((-2+c[1].x)*z,cexp(cmul(-z,z)))) + cpow2(C);
	case 28:
		return cexp(csin(z+re(c[0]))+ re(c[2]))+C;
	case 29:
		{
			float2 nz = cmul(z,z);
			float x = nz.x, y = nz.y;
			return (float2)(3*x*x - 6*x*y - y*y, 3*x*x + 6*x*y - 3*y*y) + C;
		}
	case 30:
		return cmul(cmul(c[0]-c[1]+c[2]-c[3],z),fx(1)-z) + C;
	case 31:
		return z + cdiv(ccos(z+fx(c[1].x)), csin(z-fx(c[1].y))) + C;
	case 32:
		return cch(z + fx(c[3].x))*c[0].y + csin(cmul(cmul(z, fx(c[1].x)), cexp(C - fx(c[1].y)))) + C;
	default:
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
//22 2222.2211 1111.11xx
struct data changeFirst(struct data d, int f)
{
	struct data nd = d;
	unsigned int F = f<<2;
	unsigned char m = 0 - 1;
	nd.frac = nd.frac & (~((int)m));
	nd.frac = nd.frac | F;
	return nd;
}
struct data changeSecond(struct data d, int f)
{
	struct data nd = d;
	unsigned int F = f<<10;
	unsigned char m = 0 - 1;
	nd.frac = nd.frac & (~((int)m));
	nd.frac = nd.frac | F;
	return nd;
}


//Зелёный
#define r1 50.0f
#define g1 150.0f
#define b1 60.0f

//Салатовый
#define r2 185.0f
#define g2 241.0f
#define b2 80.0f

//Белый
#define r3 255.0f
#define g3 255.0f
#define b3 255.0f

//Жёлтый
#define r4 254.0f
#define g4 237.0f
#define b4 80.0f

#define right 300
#define right2 600
#define down 300

__kernel void OpenCLFractal(
	__global uchar4* pix, float4 posData, int xPixels,struct data st, __global float2* c)
{
	unsigned int	idy = get_global_id(1),
					idx = get_global_id(0);
	float	x = posData.y + posData.w*(idy),
			y = posData.x + posData.w*(idx);
			
	bool mini = idy < right2 && idx < down;
	bool left = idy < right;
	
	if (mini)
	{
		if (left)
			x = ((int)idy - right/2);
		else
			x = ((int)idy + right/2 + right);
		y = ((int)idx - down/2);		
	}
	
	//if (idx > xPixels/2)
	//	return;
	float col = posData.z; 
	//c[4].y = (c[4].y-(posData.w*xPixels/2))/(posData.w*16);
	//c[4].y = c[5].y/32;
	c[4].y = sin(st.k)/2;
	c[4].x = cos(st.k/2)/3;
	//int X = c[4].x,Y = c[4].y;
	//X = X - posData.x
	
	//st.k = 0;
	//st = changeData(st);
	/*
	c[0].x = sin(st.k*2);
	c[0].y = log(cos(st.k)+1.1);
	c[1].x = (sin(st.k*2))*(sin(st.k*5));
	c[1].y = sin(st.k*2)*cos(st.k*2);
	c[2].x = sin(st.k*2);
	c[2].y = sin(st.k*1);
	c[3].x = sin(st.k*3);
	c[3].y = sin(st.k/2);*/
	//c[4].x = 0;
	
	float2 C, z, zn, zn_1;
	if (type(st))
	{
		C = (float2)(x,y);
		zn = (float2)(0,0);
		zn_1 = zn + (float2)(0.001f, 0);
	}
	else
	{
		zn = (float2)(x,y);
		C = (float2)(0,0);
		zn_1 = (float2)(x+0.001f, y);
	}

	//float2 Fracs(float2 z, float2 C, __global float2*c, int frac, int k)
	float2 fn1, fn2;
	int it = 0;
	float s;
	if (!mini)
	do
	{
		fn1 = Fracs(zn, C, c, firstFrac(st), st.k);
		fn2 = Fracs(zn, C, c, secondFrac(st), st.k);
		
		z = fn1 + fn2;
		
		it++;
		s = fast_distance(z,zn);
		zn = z;
	}
	while (it < st.iter && s > 1e-6);
	else
	{
		if (left)
			do
			{
				z = Fracs(zn, C, c, firstFrac(st), st.k);
				
				it++;
				s = fast_distance(z,zn);
				zn = z;
			}
			while (it < st.iter && s > 1e-6);
		else
			do
			{
				z = Fracs(zn, C, c, secondFrac(st), st.k);
				
				it++;
				s = fast_distance(z,zn);
				zn = z;
			}
			while (it < st.iter && s > 1e-6);
	}

	//float t = ((float)(it))/((float)(st.iter));
	
	int H,S,V;
	H = 360*sin(it/30.0f) + st.col*40 + 180*cos(it/15.0f);
	S = 90; 
	V = 90 + sin(it/5.0f)*40;
	H = H % 360;
	S = S % 200; if (S>100) S = 200 - S;
	V = V % 200; if (V>100) V = 200 - V;
	
	int hi;
	float vm,a,vi,vd;
	hi = H/60.0f;
	vm = (100-S)*V/100.0f;
	a = (V-vm)*(((int)H)%60)/60.0f;
	vi = vm+a;
	vd = V-a;
	
	float R,G,B;
	switch(hi)
	{
		case 0: R = V; G = vi; B = vm; break;
		case 1: R = vd; G = V; B = vm; break;
		case 2: R = vm; G = V; B = vi; break;
		case 3: R = vm; G = vd; B = V; break;
		case 4: R = vi; G = vm; B = V; break;
		default: R = V; G = vm; B = vd; break;
	}
	int r, g, b;
	r = (int)(255.0f*R/100.0f);
	g = (int)(255.0f*G/100.0f);
	b = (int)(255.0f*B/100.0f);
	
	
	//int r, g, b;
	r = R;
	g = G;
	b = B;
	r = r % 512; if (r>256) r = 512 - r;
	g = g % 512; if (g>256) g = 512 - g;
	b = b % 512; if (b>256) b = 512 - b;

	int pos = idx + idy*xPixels;
	//int pos2 = (xPixels - idx) + idy*xPixels;
	//pix[pos].x = st.col&1 ? 255-b : b;
	//pix[pos].y = st.col&2 ? 255-g : g;
	//pix[pos].z = st.col&4 ? 255-r : r;
	pix[pos].w = 255;
	//pix[pos2].x = st.col&1 ? 255-b : b;
	//pix[pos2].y = st.col&2 ? 255-g : g;
	//pix[pos2].z = st.col&4 ? 255-r : r;
	//pix[pos2].w = 255;
	pix[pos].x = b;
	pix[pos].y = g;
	pix[pos].z = r;
	//pix[pos].w = it%2==0 ? 255 : 0;
}