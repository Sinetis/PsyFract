//#ifndef _CONSTANTS_H_
//#define _CONSTANTS_H_

#pragma once
#include "stdafx.h"

namespace Const
{
	const double E = exp(1);
	const double PI = acos(-1);
	const double PI_180 = acos(-1) / 180;
	const double PI_2 = acos(-1) / 2;
	const double sqrtPI = sqrt(acos(-1));
	const double sqrt2PI = sqrt(2 * acos(-1));
}

using namespace Const;

static class Misc
{
private:

	float σ = 100;
	float μ = 300;
	// 1/(σ√(2π))·e^(-(x-μ)²/(2σ²))
	float *norm;

public:

	Misc()
	{
		int n = 6 * σ + 1;
		norm = new float[n];
		float	a = 1 / (σ * sqrt2PI),
				b = -1 / (2 * σ * σ);
		for (int i = 0; i < n; i++)
		{		
			norm[i] = a * exp((i-μ) * (i-μ) *b);
		}
		int x = 21;	
	}

	// 1/(σ√(2π))·e^(-(x-μ)²/(2σ²))
	// s = σ
	// m = μ
	float Norm(float s, float m, int i)
	{
		if (m < 0)
			return 0;
		return 0;
	}
};


//#endif