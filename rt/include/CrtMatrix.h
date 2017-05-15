/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#ifndef __COLLADA_RT__MATRIX_H__
#define __COLLADA_RT__MATRIX_H__

#include "CrtTypes.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'
		
static const RT::Float 
DEGREES_TO_RADIANS = (RT::Float)0.017453292519943;

//SCHEDULED FOR REMOVAL
//These are added to implement <lookat> only.
//It's made complicated by maintaining flexible matrix layouts.
inline void Normalize(RT::Float &x, RT::Float &y, RT::Float &z)
{
	RT::Float s = 1/sqrtf(x*x+y*y+z*z); x*=s; y*=s; z*=s;
}
template<int a0,int a1,int a2, int b0,int b1,int b2, 
int d0,int d1,int d2, class A, class B, class D>
inline void Cross(A &a, B &b, D &dst)
{
	//This doesn't have to be done in place or with template parameters.
	//It's just easier to describe this way.
	dst[d0] = a[a1]*b[b2]-a[a2]*b[b1];
	dst[d1] = a[a2]*b[b0]-a[a0]*b[b2]; 
	dst[d2] = a[a0]*b[b1]-a[a1]*b[b0]; 
}

inline void MatrixLoadAsset(RT::Matrix &m, RT::Up up, RT::Float meter=1)
{
	const bool x = up==RT::Up::X_UP;
	const bool z = up==RT::Up::Z_UP; const RT::Float l = meter;
	m[M00] = x?0:l;  m[M01] = x?l:0;      m[M02] = 0;      m[M03] = 0;
	m[M10] = x?-l:0; m[M11] = !x&&!z?l:0; m[M12] = z?-l:0; m[M13] = 0;
	m[M20] = 0;      m[M21] = z?l:0;      m[M22] = !z?l:0; m[M23] = 0;
	m[M30] = 0;      m[M31] = 0;          m[M32] = 0;      m[M33] = 1;
}
inline void MatrixLoadIdentity(RT::Matrix &m)
{
	MatrixLoadAsset(m,RT::Up::Y_UP);
}

inline void MatrixCopy(const RT::Matrix &src, RT::Matrix &dst)
{
	memcpy(dst,src,sizeof(RT::Matrix));
}
	
inline void Matrix3x3Transpose(RT::Float *m)
{
	std::swap(m[M01],m[M10]);
	std::swap(m[M02],m[M20]);
	std::swap(m[M12],m[M21]);
}
inline void Matrix4x4Transpose(RT::Float *m)
{
	Matrix3x3Transpose(m);
	std::swap(m[M03],m[M30]);
	std::swap(m[M13],m[M31]);
	std::swap(m[M23],m[M32]);
}

void Matrix3x4Invert(const RT::Matrix &LM, RT::Matrix &LMI);
void Matrix3x3Invert(const RT::Matrix &LM, RT::Matrix &LMI);
inline void MatrixInvertTranspose0(const RT::Matrix &m, RT::Matrix &it)
{		
	//Assuming for surface normals:
	//There's not code on hand for proper 4x4 inversions.
	RT::Matrix3x3Invert(m,it); RT::Matrix3x3Transpose(it); it[M33] = 0;
}

void MatrixToQuat(const RT::Matrix &rotMat, RT::Quaternion &RotQuat);

//MIXING double AND float MATH.
inline FX::Float3 &MatrixTransform(const RT::Matrix &Matrix, const FX::Float3 &v, FX::Float3 &tv)
{
	tv.x = float(v.x*Matrix[M00]+v.y*Matrix[M10]+v.z*Matrix[M20]+Matrix[M30]);
	tv.y = float(v.x*Matrix[M01]+v.y*Matrix[M11]+v.z*Matrix[M21]+Matrix[M31]);
	tv.z = float(v.x*Matrix[M02]+v.y*Matrix[M12]+v.z*Matrix[M22]+Matrix[M32]); return tv;
}
inline FX::Float3 &MatrixRotate(const RT::Matrix &Matrix, const FX::Float3 &v, FX::Float3 &tv)
{
	tv.x = float(v.x*Matrix[M00]+v.y*Matrix[M10]+v.z*Matrix[M20]);
	tv.y = float(v.x*Matrix[M01]+v.y*Matrix[M11]+v.z*Matrix[M21]);
	tv.z = float(v.x*Matrix[M02]+v.y*Matrix[M12]+v.z*Matrix[M22]); return tv;
}

void MatrixRotateAngleAxis(RT::Matrix &LMatrix, RT::Float LAxisX, RT::Float LAxisY, RT::Float LAxisZ, RT::Float LAngle);
																
void MatrixMult(const RT::Matrix &LMtx1, const RT::Matrix &LMtx2, RT::Matrix &LDstMtx);

void MatrixMult(const RT::Matrix &LSrcMtx, RT::Matrix &LDestMtx);

void MatrixTranslate(RT::Matrix &dst, RT::Float x, RT::Float y, RT::Float z);

void MatrixScale(RT::Matrix &dst, RT::Float x, RT::Float y, RT::Float z);

void QuaternionToMatrix(const RT::Quaternion &LQ, RT::Matrix &LMatrix);

////2017: These routines are old code, refactored to merge the many series classes
////into FX::Float1,2,3&4.

//UNUSED/WAS CrtQuat operator*(CrtQuat,CrtQuat).
//2017: Rewriting this UNUSED code so CrtQuat can be retired in favor of FX::Float4.
inline void QuaternionMult(const RT::Quaternion &q1, const RT::Quaternion &q2, RT::Quaternion &q1_dst)
{
	RT::Float A,B,C,D,E,F,G,H;
	A = (q1[3]+q1[0])*(q2[3]+q2[0]);
	B = (q1[2]-q1[1])*(q2[1]-q2[2]);
	C = (q1[3]-q1[0])*(q2[1]+q2[2]);
	D = (q1[1]+q1[2])*(q2[3]-q2[0]);
	E = (q1[0]+q1[2])*(q2[0]+q2[1]);
	F = (q1[0]-q1[2])*(q2[0]-q2[1]);
	G = (q1[3]+q1[1])*(q2[3]-q2[2]);
	H = (q1[3]-q1[1])*(q2[3]+q2[2]);

	q1_dst[0] = A-(E+F+G+H)*0.5f;
	q1_dst[1] = C+(E-F+G-H)*0.5f;
	q1_dst[2] = D+(E-F-G+H)*0.5f;
	q1_dst[3] = B+(-E-F+G+H)*0.5f; 
}
//UNUSED/WAS CrtQuat::operator*=(). 
//NOTE the sense has been reversed to match MatrixMult().
//2017: Rewriting this UNUSED code so CrtQuat can be retired in favor of FX::Float4.
inline void QuaternionMult(const RT::Quaternion &q1, RT::Quaternion &q2_and_q1_dst)
{
	//This is admittedly confusing, and untested. Basically it chains.
	RT::QuaternionMult(q1,q2_and_q1_dst,q2_and_q1_dst);
}
//UNUSED/UNTESTED/ORIGINAL CODE
//void CrtQuat::Rotate(const FX::Float3 &axis, float theta);
inline void QuaternionRotateAngleAxis(RT::Quaternion &dst, const FX::Float3 &axis, RT::Float a)
{
	//If the axis of rotation is (ax, ay, az)- must be a unit vector
	//and the angle is theta (radians)
	a*=DEGREES_TO_RADIANS/2; 
	dst[3] = cosf(a); a = sin(a); 
	dst[0] = axis.x*a; dst[1] = axis.y*a; dst[2] = axis.z*a; 
}
//UNUSED/REMOVED CODE/PRESERVING ANYWAY
inline void QuaternionRotateEulerAngles(RT::Quaternion &dst, const FX::Float3 &v)
{
	//reading a rotation or computing a rotation from the animation channels:
	//v is a vector which contains the X, Y and Z euler angles
	RT::Quaternion qX,qY,qZ; 	
	RT::QuaternionRotateAngleAxis(qX,FX::Float3(1,0,0),v.x);
	RT::QuaternionRotateAngleAxis(qY,FX::Float3(0,1,0),v.y);	
	RT::QuaternionRotateAngleAxis(qZ,FX::Float3(0,0,1),v.z);
	RT::QuaternionMult(qY,qX,dst); RT::QuaternionMult(qZ,dst);
}

//-------.
	}//<-'
}

#endif //__COLLADA_RT__MATRIX_H__
/*C1071*/