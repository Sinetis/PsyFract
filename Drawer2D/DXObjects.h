#pragma once

#include <d3d10.h>
#include <d3dx10.h>
#include <omp.h>

#define V2 D3DXVECTOR2
#define V3 D3DXVECTOR3
#define V4 D3DXVECTOR4
#define Q D3DXQUATERNION
#define M D3DXMATRIX

#define fore(i, n) for(i = 0; i < n; i++)

namespace DXObjects
{
	const V4 DEFAULT_COLOR = V4(0, 0, 0, 1);
};

struct SimpleVertex
{
	V3 Pos;
	V3 Normal;
	V4 Color = DXObjects::DEFAULT_COLOR;

	SimpleVertex(V3 pos, V3 normal, V4 color = DXObjects::DEFAULT_COLOR)
	{
		this->Pos = pos;
		this->Normal = normal;
		this->Color = color;
	}

	SimpleVertex() {}
};

struct Triangle
{
	// Vertex numbers of triangle
	int A, B, C;

	Triangle(int a = 0, int b = 0, int c = 0) :A(a),B(b),C(c)	
	{}
};

///<summary>
///Description
///</summary>
///<param name="a"><b>Ashechka</b></param>
///<param name="b">Beshechka</param>
///<returns>description</returns>  
//int func(int a, int b)
//{
//	int x = func(a, b);
//}

class Object
{
protected:
	SimpleVertex* vertices;
	int numV;

	Triangle* triangles;
	int numT;

	V3 pos;
	V3 center;
	V4 color;

	Object() {}

	Object * SetCenter(V3 newCenter)
	{
		center = newCenter;
	}

public:
	~Object()
	{
		if (vertices != NULL)
			delete[] vertices;
		if (triangles != NULL)
			delete[] triangles;
	}
	

	Object* SetPosition(V3 newPosition)
	{
		pos = newPosition;
		return this;
	}

	Object* MoveTo(V3 newPosition)
	{
		int i;
#pragma omp parallel for private(i)
		fore(i, numV)
		{
			vertices[i].Pos += newPosition - pos;
		}
		pos = newPosition;
		return this;
	}



	Object* Shift(V3 value)
	{
		pos += value;
	}

	Object* RotateAroundZero(V3 axes)
	{
		return this;
	}

	Object* RotateAroundPoint(V3 axes, V3 point)
	{
		return this;
	}

	///<summary>Поворот вокруг оси X</summary>
	///<param name="radian">Угол поворота в радианах</param>
	Object* RotateAroundX(float radian)
	{
		M *rM;
		D3DXMatrixRotationX(rM, radian);

		int i;
#pragma omp parallel for private(i)
		fore(i, numV)
		{
			D3DXVec3TransformCoord(&vertices[i].Pos, &vertices[i].Pos, rM);
			D3DXVec3TransformCoord(&vertices[i].Normal, &vertices[i].Normal, rM);
		}
		return this;
	}

	Object* RotateAroundY(float radian)
	{
		M *rM;
		D3DXMatrixRotationY(rM, radian);

		int i;
#pragma omp parallel for private(i)
		fore(i, numV)
		{
			D3DXVec3TransformCoord(&vertices[i].Pos, &vertices[i].Pos, rM);
			D3DXVec3TransformCoord(&vertices[i].Normal, &vertices[i].Normal, rM);
		}
		return this;
	}

	Object* RotateAroundZ(float radian)
	{
		M *rM;
		D3DXMatrixRotationZ(rM, radian);

		int i;
#pragma omp parallel for private(i)
		fore(i, numV)
		{
			D3DXVec3TransformCoord(&vertices[i].Pos, &vertices[i].Pos, rM);
			D3DXVec3TransformCoord(&vertices[i].Normal, &vertices[i].Normal, rM);
		}
		return this;
	}

	Object* ScaleAroundZero(V3 axes)
	{
		int i;
#pragma omp parallel for private(i)
		fore(i, numV)
		{
			vertices[i].Pos.x *= axes.x;
			vertices[i].Pos.y *= axes.y;
			vertices[i].Pos.z *= axes.z;
		}
	}
	Object* ScaleAroundCenter(V3 axes)
	{
		int i;
#pragma omp parallel for private(i)
		fore(i, numV)
		{
			vertices[i].Pos -= center;
			vertices[i].Pos.x *= axes.x;
			vertices[i].Pos.y *= axes.y;
			vertices[i].Pos.z *= axes.z;
			vertices[i].Pos += center;
		}
		return this;
	}

	Object* Affect(M matrix)
	{}

	static Object* Attach(const Object& base, const Object& added)
	{
		return NULL;
	}

private:
};


class Plane: public Object
{
	//SimpleVertex* vertices;
	//int numV;
	//Triangle* triangles;
	//int numT;
	//V3 pos;
	//V4 color;

	int n, m;
	V2 size;

public:
	Plane(int n = 1, int m = 1, V3 pos = V3(0,0,0), V2 size = V2(1,1), V4 color = DXObjects::DEFAULT_COLOR)
	{
		this->m = m;
		this->n = n;
		this->pos = pos;
		this->center = V3(0, 0, 0);
		this->size = size;
		this->color = color;

		numV = (n + 1)*(m + 1);
		vertices = new SimpleVertex[numV];

		int nm = n * m;
		numT = nm * 2;
		triangles = new Triangle[numT];

		float w = size.x / m;
		float h = size.y / n;
#pragma omp parallel for
		for(int i = 0, k; i <= m; i++)
			for (int j = 0; j <= n; j++)
			{
				k = i*(n + 1) + j;
				vertices[k].Pos = V3(
					i*w - size.x/2, 
					j*h - size.y/2, 
					pos.z);
				vertices[k].Normal = V3(0, 0, 1);
				vertices[k].Color = color;

				if (i == m || j == n)
					continue;

				triangles[k].A = k;
				triangles[k].B = k + n;
				triangles[k].C = k + 1;

				k += nm;
				triangles[k].A = k + n;
				triangles[k].B = k + n + 1;
				triangles[k].C = k + 1;
			}
	}
};

class Box : public Object
{
	int n, m, k;
	V3 size;

	friend Plane;
public:
	Box(int n = 1, int m = 1, int k = 1, V3 pos = V3(0, 0, 0),
		V3 size = V3(1, 1, 1), V4 color = DXObjects::DEFAULT_COLOR)
		:n(n),m(m),k(k), size(size)
	{
		Plane top(n, m), bottom(n, m);
		Plane left(m, k), right(m, k);
		Plane back(n, k), forward(n, k);

		top.MoveTo(V3(0, 0, 0.5f));

		bottom.MoveTo(V3(0, 0, 0.5f))
			->RotateAroundX(PI);

		left.MoveTo(V3(0, 0, 0.5f))
			->RotateAroundY(PI_2);
		right.MoveTo(V3(0, 0, 0.5f))
			->RotateAroundY(-PI_2);

		back.MoveTo(V3(0, 0, 0.5f))
			->RotateAroundX(PI_2);
		forward.MoveTo(V3(0, 0, 0.5f))
			->RotateAroundX(-PI_2);
	}
};