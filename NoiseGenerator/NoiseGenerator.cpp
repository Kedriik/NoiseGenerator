// NoiseGenerator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define GLM_SWIZZLE
#include "glm\glm\gtx\transform.hpp"
#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\quaternion.hpp"
#include "glm\glm\gtx\quaternion.hpp"
#include <iostream>
#include "glm\glm\glm.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <random>
#include <time.h>
#include <stdlib.h>   
#include <conio.h>
#include <vector>
#include <chrono>
#include "windows.h"
#include "glm\glm\glm.hpp"
using namespace glm;
using namespace std;
double  theMin = 100000;
class NoiseGenerator
{
public:
	static double permute(double x) { return floor(mod(((x*34.0) + 1.0)*x, 289.0)); }
	static dvec4 fade(dvec4 t) { return t * t*t*(t*(t*6.0 - 15.0) + 10.0); }
	static double taylorInvSqrt(double r) { return 1.79284291400159 - 0.85373472095314 * r; }
	static dvec4 permute(dvec4 x) { return mod(((x*34.0) + 1.0)*x, 289.0); }
	static dvec4 taylorInvSqrt(dvec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
	static double mod289(double x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
	static dvec4 mod289(dvec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
	static dvec4 perm(dvec4 x) { return mod289(((x * 34.0) + 1.0) * x); }
	static double _rand(double n) { return fract(sin(n) * 43758.5453123); }
	static dvec4 grad4(double j, dvec4 ip) {
		const dvec4 ones = dvec4(1.0, 1.0, 1.0, -1.0);
		dvec4 p, s;

		p.xyz = floor(fract(dvec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
		double temp = dot(abs(p.xyz()), ones.xyz());
		p.w = 1.5 - dot(abs(p.xyz()), ones.xyz());
		s = dvec4(lessThan(p, dvec4(0.0)));
		p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www;

		return p;
	}
	static double snoise(dvec4 v) {
		const dvec2  C = dvec2(0.138196601125010504, 0.309016994374947451); // (sqrt(5) - 1)/4   F4
																			// First corner
		dvec4 i = floor(v + dot(v, C.yyyy()));
		dvec4 x0 = v - i + dot(i, C.xxxx());

		// Other corners

		// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
		dvec4 i0;

		dvec3 isX = step(x0.yzw(), x0.xxx());
		dvec3 isYZ = step(x0.zww(), x0.yyz());
		//  i0.x = dot( isX, vec3( 1.0 ) );
		i0.x = isX.x + isX.y + isX.z;
		i0.yzw = 1.0 - isX;

		//  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
		i0.y += isYZ.x + isYZ.y;
		i0.zw += 1.0 - isYZ.xy;

		i0.z += isYZ.z;
		i0.w += 1.0 - isYZ.z;

		// i0 now contains the unique values 0,1,2,3 in each channel
		dvec4 i3 = clamp(i0, 0.0, 1.0);
		dvec4 i2 = clamp(i0 - 1.0, 0.0, 1.0);
		dvec4 i1 = clamp(i0 - 2.0, 0.0, 1.0);

		//  x0 = x0 - 0.0 + 0.0 * C 
		dvec4 x1 = x0 - i1 + 1.0 * C.xxxx;
		dvec4 x2 = x0 - i2 + 2.0 * C.xxxx;
		dvec4 x3 = x0 - i3 + 3.0 * C.xxxx;
		dvec4 x4 = x0 - 1.0 + 4.0 * C.xxxx;

		// Permutations
		i = mod(i, 289.0);
		double j0 = permute(permute(permute(permute(i.w) + i.z) + i.y) + i.x);
		dvec4 j1 = permute(permute(permute(permute(
			i.w + dvec4(i1.w, i2.w, i3.w, 1.0))
			+ i.z + dvec4(i1.z, i2.z, i3.z, 1.0))
			+ i.y + dvec4(i1.y, i2.y, i3.y, 1.0))
			+ i.x + dvec4(i1.x, i2.x, i3.x, 1.0));
		// Gradients
		// ( 7*7*6 points uniformly over a cube, mapped onto a 4-octahedron.)
		// 7*7*6 = 294, which is close to the ring size 17*17 = 289.

		dvec4 ip = dvec4(1.0 / 294.0, 1.0 / 49.0, 1.0 / 7.0, 0.0);

		dvec4 p0 = grad4(j0, ip);
		dvec4 p1 = grad4(j1.x, ip);
		dvec4 p2 = grad4(j1.y, ip);
		dvec4 p3 = grad4(j1.z, ip);
		dvec4 p4 = grad4(j1.w, ip);

		// Normalise gradients
		dvec4 norm = taylorInvSqrt(dvec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
		p0 *= norm.x;
		p1 *= norm.y;
		p2 *= norm.z;
		p3 *= norm.w;
		p4 *= taylorInvSqrt(dot(p4, p4));

		// Mix contributions from the five corners

		dvec3 m0 = dvec3(0.0);// max(0.6 - vec3(dot(x0, x0), dot(x1, x1), dot(x2, x2)), 0.0);
		dvec2 m1 = dvec2(0.0);// max(0.6 - vec2(dot(x3, x3), dot(x4, x4)), 0.0);
		m0 = m0 * m0;
		m1 = m1 * m1;
		return 49.0 * (dot(m0*m0, dvec3(dot(p0, x0), dot(p1, x1), dot(p2, x2)))
			+ dot(m1*m1, dvec2(dot(p3, x3), dot(p4, x4))));

	}
	
	//Classic 4D Perlin Noise
	static	double cnoise(dvec4 P, double radius = 0, double _fade = 0.1, double frequency = 1.0) {
		double fadeFactor = 1.0;
		double test = glm::length(P.xyz());
		if (test<theMin) {
			theMin = test;
		}
		if (radius != 0 && _fade < 1.0 && _fade > 0.0) {
			double temp = glm::length(P.xyz()) / radius;
			if (temp > 1.0 - _fade) {
				fadeFactor = -glm::length(P.xyz()) / (radius*_fade) +1.0 / _fade;
			}
			if (fadeFactor < 0) fadeFactor = 0;
		}
		P *= frequency;
		dvec4 Pi0 = floor(P); // Integer part for indexing
		dvec4 Pi1 = Pi0 + 1.0; // Integer part + 1
		Pi0 = mod(Pi0, 289.0);
		Pi1 = mod(Pi1, 289.0);
		dvec4 Pf0 = fract(P); // Fractional part for interpolation
		dvec4 Pf1 = Pf0 - 1.0; // Fractional part - 1.0
		dvec4 ix = dvec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
		dvec4 iy = dvec4(Pi0.yy, Pi1.yy);
		dvec4 iz0 = dvec4(Pi0.zzzz);
		dvec4 iz1 = dvec4(Pi1.zzzz);
		dvec4 iw0 = dvec4(Pi0.wwww);
		dvec4 iw1 = dvec4(Pi1.wwww);

		dvec4 ixy = permute(permute(ix) + iy);
		dvec4 ixy0 = permute(ixy + iz0);
		dvec4 ixy1 = permute(ixy + iz1);
		dvec4 ixy00 = permute(ixy0 + iw0);
		dvec4 ixy01 = permute(ixy0 + iw1);
		dvec4 ixy10 = permute(ixy1 + iw0);
		dvec4 ixy11 = permute(ixy1 + iw1);

		dvec4 gx00 = ixy00 / 7.0;
		dvec4 gy00 = floor(gx00) / 7.0;
		dvec4 gz00 = floor(gy00) / 6.0;
		gx00 = fract(gx00) - 0.5;
		gy00 = fract(gy00) - 0.5;
		gz00 = fract(gz00) - 0.5;
		dvec4 gw00 = dvec4(0.75) - abs(gx00) - abs(gy00) - abs(gz00);
		dvec4 sw00 = step(gw00, dvec4(0.0));
		gx00 -= sw00 * (step(0.0, gx00) - 0.5);
		gy00 -= sw00 * (step(0.0, gy00) - 0.5);

		dvec4 gx01 = ixy01 / 7.0;
		dvec4 gy01 = floor(gx01) / 7.0;
		dvec4 gz01 = floor(gy01) / 6.0;
		gx01 = fract(gx01) - 0.5;
		gy01 = fract(gy01) - 0.5;
		gz01 = fract(gz01) - 0.5;
		dvec4 gw01 = dvec4(0.75) - abs(gx01) - abs(gy01) - abs(gz01);
		dvec4 sw01 = step(gw01, dvec4(0.0));
		gx01 -= sw01 * (step(0.0, gx01) - 0.5);
		gy01 -= sw01 * (step(0.0, gy01) - 0.5);

		dvec4 gx10 = ixy10 / 7.0;
		dvec4 gy10 = floor(gx10) / 7.0;
		dvec4 gz10 = floor(gy10) / 6.0;
		gx10 = fract(gx10) - 0.5;
		gy10 = fract(gy10) - 0.5;
		gz10 = fract(gz10) - 0.5;
		dvec4 gw10 = dvec4(0.75) - abs(gx10) - abs(gy10) - abs(gz10);
		dvec4 sw10 = step(gw10, dvec4(0.0));
		gx10 -= sw10 * (step(0.0, gx10) - 0.5);
		gy10 -= sw10 * (step(0.0, gy10) - 0.5);

		dvec4 gx11 = ixy11 / 7.0;
		dvec4 gy11 = floor(gx11) / 7.0;
		dvec4 gz11 = floor(gy11) / 6.0;
		gx11 = fract(gx11) - 0.5;
		gy11 = fract(gy11) - 0.5;
		gz11 = fract(gz11) - 0.5;
		dvec4 gw11 = dvec4(0.75) - abs(gx11) - abs(gy11) - abs(gz11);
		dvec4 sw11 = step(gw11, dvec4(0.0));
		gx11 -= sw11 * (step(0.0, gx11) - 0.5);
		gy11 -= sw11 * (step(0.0, gy11) - 0.5);

		dvec4 g0000 = dvec4(gx00.x, gy00.x, gz00.x, gw00.x);
		dvec4 g1000 = dvec4(gx00.y, gy00.y, gz00.y, gw00.y);
		dvec4 g0100 = dvec4(gx00.z, gy00.z, gz00.z, gw00.z);
		dvec4 g1100 = dvec4(gx00.w, gy00.w, gz00.w, gw00.w);
		dvec4 g0010 = dvec4(gx10.x, gy10.x, gz10.x, gw10.x);
		dvec4 g1010 = dvec4(gx10.y, gy10.y, gz10.y, gw10.y);
		dvec4 g0110 = dvec4(gx10.z, gy10.z, gz10.z, gw10.z);
		dvec4 g1110 = dvec4(gx10.w, gy10.w, gz10.w, gw10.w);
		dvec4 g0001 = dvec4(gx01.x, gy01.x, gz01.x, gw01.x);
		dvec4 g1001 = dvec4(gx01.y, gy01.y, gz01.y, gw01.y);
		dvec4 g0101 = dvec4(gx01.z, gy01.z, gz01.z, gw01.z);
		dvec4 g1101 = dvec4(gx01.w, gy01.w, gz01.w, gw01.w);
		dvec4 g0011 = dvec4(gx11.x, gy11.x, gz11.x, gw11.x);
		dvec4 g1011 = dvec4(gx11.y, gy11.y, gz11.y, gw11.y);
		dvec4 g0111 = dvec4(gx11.z, gy11.z, gz11.z, gw11.z);
		dvec4 g1111 = dvec4(gx11.w, gy11.w, gz11.w, gw11.w);

		dvec4 norm00 = taylorInvSqrt(dvec4(dot(g0000, g0000), dot(g0100, g0100), dot(g1000, g1000), dot(g1100, g1100)))*fadeFactor;
		g0000 *= norm00.x;
		g0100 *= norm00.y;
		g1000 *= norm00.z;
		g1100 *= norm00.w;

		dvec4 norm01 = taylorInvSqrt(dvec4(dot(g0001, g0001), dot(g0101, g0101), dot(g1001, g1001), dot(g1101, g1101)))*fadeFactor;
		g0001 *= norm01.x;
		g0101 *= norm01.y;
		g1001 *= norm01.z;
		g1101 *= norm01.w;

		dvec4 norm10 = taylorInvSqrt(dvec4(dot(g0010, g0010), dot(g0110, g0110), dot(g1010, g1010), dot(g1110, g1110)))*fadeFactor;
		g0010 *= norm10.x;
		g0110 *= norm10.y;
		g1010 *= norm10.z;
		g1110 *= norm10.w;

		dvec4 norm11 = taylorInvSqrt(dvec4(dot(g0011, g0011), dot(g0111, g0111), dot(g1011, g1011), dot(g1111, g1111)))*fadeFactor;
		g0011 *= norm11.x;
		g0111 *= norm11.y;
		g1011 *= norm11.z;
		g1111 *= norm11.w;

		double n0000 = dot(g0000, Pf0);
		double n1000 = dot(g1000, dvec4(Pf1.x, Pf0.yzw));
		double n0100 = dot(g0100, dvec4(Pf0.x, Pf1.y, Pf0.zw));
		double n1100 = dot(g1100, dvec4(Pf1.xy, Pf0.zw));
		double n0010 = dot(g0010, dvec4(Pf0.xy, Pf1.z, Pf0.w));
		double n1010 = dot(g1010, dvec4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
		double n0110 = dot(g0110, dvec4(Pf0.x, Pf1.yz, Pf0.w));
		double n1110 = dot(g1110, dvec4(Pf1.xyz, Pf0.w));
		double n0001 = dot(g0001, dvec4(Pf0.xyz, Pf1.w));
		double n1001 = dot(g1001, dvec4(Pf1.x, Pf0.yz, Pf1.w));
		double n0101 = dot(g0101, dvec4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
		double n1101 = dot(g1101, dvec4(Pf1.xy, Pf0.z, Pf1.w));
		double n0011 = dot(g0011, dvec4(Pf0.xy, Pf1.zw));
		double n1011 = dot(g1011, dvec4(Pf1.x, Pf0.y, Pf1.zw));
		double n0111 = dot(g0111, dvec4(Pf0.x, Pf1.yzw));
		double n1111 = dot(g1111, Pf1);
		dvec4 fade_xyzw = fade(Pf0);
		dvec4 n_0w = mix(dvec4(n0000, n1000, n0100, n1100), dvec4(n0001, n1001, n0101, n1101), fade_xyzw.w);
		dvec4 n_1w = mix(dvec4(n0010, n1010, n0110, n1110), dvec4(n0011, n1011, n0111, n1111), fade_xyzw.w);
		dvec4 n_zw = mix(n_0w, n_1w, fade_xyzw.z);
		vec2 n_yzw = mix(n_zw.xy(), n_zw.zw(), fade_xyzw.y);
		double n_xyzw = mix(n_yzw.x, n_yzw.y, fade_xyzw.x);
		return 2.2 * n_xyzw;
	}
	static void generateHeightMap(vector<dvec4> *heightMap, int heightMapSize, double time, int octaves = 8)
	{
		for (int i = 0; i < heightMapSize; i++)
		{
			for (int j = 0; j < heightMapSize; j++)
			{
				int  WGidY = i;
				int  WGidX = j;
				double X = double((WGidX - (heightMapSize - 1) / 2.0f) / double(heightMapSize - 1));
				double Z = double((WGidY - (heightMapSize - 1) / 2.0f) / double(heightMapSize - 1));
				dvec4 vectorToStore = dvec4(X, 0, Z, 1);
				double f = 2;
				double A = 0.1;
				double h = 0;
				for (int i = 0; i<octaves; i++)
				{
					h += A * cnoise(dvec4(f*vectorToStore.xyz, time));
					A /= 2.0;
					f *= 2.0;
				}
				cout << "\rGenerowanie ukonczone w " << 100.0f*double(heightMapSize*i + j) / (heightMapSize*heightMapSize) << "%";
				vectorToStore.y = h;
				heightMap->push_back(vectorToStore);
			}
		}
	}
	static void generate3DToFile(vec3 size = vec3(100, 100, 100)) {
		ofstream file;
		file.open("generated3D.data");
		if (!file.is_open()) {
			cout << "Count not open file" << endl;
		}
	
		
		dvec4 randomOffset = dvec4(double(rand() % 1000) / 10000, double(rand() % 1000) / 10000, double(rand() % 1000) / 10000, double(rand() % 1000) / 10000);
		double frequency = 10.1,
			amplitude = 0.1,
			octaves = 6,
			lacunarity = 2.0,
			time = 0.0;
		dvec4 randomSeed = dvec4(double(rand() % 1000) / 1000, double(rand() % 1000) / 1000, double(rand() % 1000) / 1000, double(rand() % 1000) / 1000);
		int counter = 0;
		for (int i = 0; i < size.x; i++) {
			for (int j = 0; j < size.y; j++) {
				for (int k = 0; k < size.z; k++) {
					double X = double((double(i) - (size.x - 1) / 2.0f) / double(size.x - 1));
					double Y = double((double(j) - (size.y - 1) / 2.0f) / double(size.y - 1));
					double Z =  double((double(k) - (size.z - 1) / 2.0f) / double(size.z - 1));
					dvec4 currentPos = dvec4(X, Y, Z, 11.1);// +randomSeed;
					//cout << "(" << X << "," << Y << "," << Z << ")" << endl;
					double A = amplitude, f = frequency;
					double sum = 0;
					for (int l = 0; l<octaves; l++){
						//sum += A * cnoise(dvec4(f*currentPos.xyz, time));
						double noise = A * cnoise(currentPos,0.5,0.1,f);
						sum += noise;
						A /= lacunarity;
						f *= lacunarity;
					}
					//sum = double(rand() % 1000) / 1000;
					if(i+j+k == 0)
						file << sum;
					else
						file << " " << sum;

					int counter = i * size.x*size.y + j * size.y + k;
					if (counter % 1000 == 0) {
						system("cls");
						cout << counter;
					}
				}
			}
		}
		
		file.close();
	}
};



int main()
{
	srand(time(NULL));
	vector<dvec4> v;
//	NoiseGenerator::generateHeightMap(&v, 10, 0);
	NoiseGenerator::generate3DToFile();
    return 0;
}

