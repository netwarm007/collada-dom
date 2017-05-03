/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include <RT.pch.h> //PCH

#include "CrtMatrix.h"

COLLADA_(namespace)
{
	namespace RT
	{//-.
//<-----'

void RT::MatrixMult(const RT::Matrix &LMtx1, const RT::Matrix &LMtx2, RT::Matrix &LDstMtx)
{
	RT::Float LM1_00,LM1_01,LM1_02,LM1_03,
	LM1_10,LM1_11,LM1_12,LM1_13,
	LM1_20,LM1_21,LM1_22,LM1_23,
	LM1_30,LM1_31,LM1_32,LM1_33,
	LM2_00,LM2_01,LM2_02,LM2_03,
	LM2_10,LM2_11,LM2_12,LM2_13,
	LM2_20,LM2_21,LM2_22,LM2_23,
	LM2_30,LM2_31,LM2_32,LM2_33; 

	LM1_00 = LMtx1[M00];
	LM1_01 = LMtx1[M01];
	LM1_02 = LMtx1[M02];
	LM1_03 = LMtx1[M03];
	LM1_10 = LMtx1[M10];
	LM1_11 = LMtx1[M11];
	LM1_12 = LMtx1[M12];
	LM1_13 = LMtx1[M13];
	LM1_20 = LMtx1[M20];
	LM1_21 = LMtx1[M21];
	LM1_22 = LMtx1[M22];
	LM1_23 = LMtx1[M23];
	LM1_30 = LMtx1[M30];
	LM1_31 = LMtx1[M31];
	LM1_32 = LMtx1[M32];
	LM1_33 = LMtx1[M33];

	LM2_00 = LMtx2[M00];
	LM2_01 = LMtx2[M01];
	LM2_02 = LMtx2[M02];
	LM2_03 = LMtx2[M03];
	LM2_10 = LMtx2[M10];
	LM2_11 = LMtx2[M11];
	LM2_12 = LMtx2[M12];
	LM2_13 = LMtx2[M13];
	LM2_20 = LMtx2[M20];
	LM2_21 = LMtx2[M21];
	LM2_22 = LMtx2[M22];
	LM2_23 = LMtx2[M23];
	LM2_30 = LMtx2[M30];
	LM2_31 = LMtx2[M31];
	LM2_32 = LMtx2[M32];
	LM2_33 = LMtx2[M33];

	LDstMtx[M00] = LM1_00*LM2_00+LM1_01*LM2_10+LM1_02*LM2_20+LM1_03*LM2_30;
	LDstMtx[M01] = LM1_00*LM2_01+LM1_01*LM2_11+LM1_02*LM2_21+LM1_03*LM2_31;
	LDstMtx[M02] = LM1_00*LM2_02+LM1_01*LM2_12+LM1_02*LM2_22+LM1_03*LM2_32;
	LDstMtx[M03] = LM1_00*LM2_03+LM1_01*LM2_13+LM1_02*LM2_23+LM1_03*LM2_33;

	LDstMtx[M10] = LM1_10*LM2_00+LM1_11*LM2_10+LM1_12*LM2_20+LM1_13*LM2_30;
	LDstMtx[M11] = LM1_10*LM2_01+LM1_11*LM2_11+LM1_12*LM2_21+LM1_13*LM2_31;
	LDstMtx[M12] = LM1_10*LM2_02+LM1_11*LM2_12+LM1_12*LM2_22+LM1_13*LM2_32;
	LDstMtx[M13] = LM1_10*LM2_03+LM1_11*LM2_13+LM1_12*LM2_23+LM1_13*LM2_33;

	LDstMtx[M20] = LM1_20*LM2_00+LM1_21*LM2_10+LM1_22*LM2_20+LM1_23*LM2_30;
	LDstMtx[M21] = LM1_20*LM2_01+LM1_21*LM2_11+LM1_22*LM2_21+LM1_23*LM2_31;
	LDstMtx[M22] = LM1_20*LM2_02+LM1_21*LM2_12+LM1_22*LM2_22+LM1_23*LM2_32;
	LDstMtx[M23] = LM1_20*LM2_03+LM1_21*LM2_13+LM1_22*LM2_23+LM1_23*LM2_33;

	LDstMtx[M30] = LM1_30*LM2_00+LM1_31*LM2_10+LM1_32*LM2_20+LM1_33*LM2_30;
	LDstMtx[M31] = LM1_30*LM2_01+LM1_31*LM2_11+LM1_32*LM2_21+LM1_33*LM2_31;
	LDstMtx[M32] = LM1_30*LM2_02+LM1_31*LM2_12+LM1_32*LM2_22+LM1_33*LM2_32;
	LDstMtx[M33] = LM1_30*LM2_03+LM1_31*LM2_13+LM1_32*LM2_23+LM1_33*LM2_33;
}

void RT::MatrixMult(const RT::Matrix &LSrcMtx, RT::Matrix &LDestMtx)
{
	RT::Float L00,L01,L02,L03,L10,L11,L12,L13,
	L20,L21,L22,L23,L30,L31,L32,L33;

	//!!!GAC as an experiment, reorder the operations so the lifetime of the temps is limited
	//!!!GAC the compiler should do this, but I want to make sure it's doing its job.

	L00 = LDestMtx[M00];
	L10 = LDestMtx[M10];
	L20 = LDestMtx[M20];
	L30 = LDestMtx[M30];
	LDestMtx[M00] = LSrcMtx[M00]*L00+LSrcMtx[M01]*L10+LSrcMtx[M02]*L20+LSrcMtx[M03]*L30;
	LDestMtx[M10] = LSrcMtx[M10]*L00+LSrcMtx[M11]*L10+LSrcMtx[M12]*L20+LSrcMtx[M13]*L30;
	LDestMtx[M20] = LSrcMtx[M20]*L00+LSrcMtx[M21]*L10+LSrcMtx[M22]*L20+LSrcMtx[M23]*L30;
	LDestMtx[M30] = LSrcMtx[M30]*L00+LSrcMtx[M31]*L10+LSrcMtx[M32]*L20+LSrcMtx[M33]*L30;

	L01 = LDestMtx[M01];
	L11 = LDestMtx[M11];
	L21 = LDestMtx[M21];
	L31 = LDestMtx[M31];
	LDestMtx[M01] = LSrcMtx[M00]*L01+LSrcMtx[M01]*L11+LSrcMtx[M02]*L21+LSrcMtx[M03]*L31;
	LDestMtx[M11] = LSrcMtx[M10]*L01+LSrcMtx[M11]*L11+LSrcMtx[M12]*L21+LSrcMtx[M13]*L31;
	LDestMtx[M21] = LSrcMtx[M20]*L01+LSrcMtx[M21]*L11+LSrcMtx[M22]*L21+LSrcMtx[M23]*L31;
	LDestMtx[M31] = LSrcMtx[M30]*L01+LSrcMtx[M31]*L11+LSrcMtx[M32]*L21+LSrcMtx[M33]*L31;

	L02 = LDestMtx[M02];
	L12 = LDestMtx[M12];
	L22 = LDestMtx[M22];
	L32 = LDestMtx[M32];
	LDestMtx[M02] = LSrcMtx[M00]*L02+LSrcMtx[M01]*L12+LSrcMtx[M02]*L22+LSrcMtx[M03]*L32;
	LDestMtx[M12] = LSrcMtx[M10]*L02+LSrcMtx[M11]*L12+LSrcMtx[M12]*L22+LSrcMtx[M13]*L32;
	LDestMtx[M22] = LSrcMtx[M20]*L02+LSrcMtx[M21]*L12+LSrcMtx[M22]*L22+LSrcMtx[M23]*L32;
	LDestMtx[M32] = LSrcMtx[M30]*L02+LSrcMtx[M31]*L12+LSrcMtx[M32]*L22+LSrcMtx[M33]*L32;

	L03 = LDestMtx[M03];
	L13 = LDestMtx[M13];
	L23 = LDestMtx[M23];
	L33 = LDestMtx[M33];
	LDestMtx[M03] = LSrcMtx[M00]*L03+LSrcMtx[M01]*L13+LSrcMtx[M02]*L23+LSrcMtx[M03]*L33;
	LDestMtx[M13] = LSrcMtx[M10]*L03+LSrcMtx[M11]*L13+LSrcMtx[M12]*L23+LSrcMtx[M13]*L33;
	LDestMtx[M23] = LSrcMtx[M20]*L03+LSrcMtx[M21]*L13+LSrcMtx[M22]*L23+LSrcMtx[M23]*L33;
	LDestMtx[M33] = LSrcMtx[M30]*L03+LSrcMtx[M31]*L13+LSrcMtx[M32]*L23+LSrcMtx[M33]*L33;
}

void RT::QuaternionToMatrix(const RT::Quaternion &LQ, RT::Matrix &LMatrix)
{
	RT::Float LS,LXS,LYS,LZS,LWX,LWY,LWZ,LXX,LXY,LXZ,LYY,LYZ,LZZ;

	LS = 2/(LQ[0]*LQ[0]+LQ[1]*LQ[1]+LQ[2]*LQ[2]+LQ[3]*LQ[3]);

	LXS = LQ[0]*LS;
	LYS = LQ[1]*LS;
	LZS = LQ[2]*LS;
	LWX = LQ[3]*LXS;
	LWY = LQ[3]*LYS;
	LWZ = LQ[3]*LZS;
	LXX = LQ[0]*LXS;
	LXY = LQ[0]*LYS;
	LXZ = LQ[0]*LZS;
	LYY = LQ[1]*LYS;
	LYZ = LQ[1]*LZS;
	LZZ = LQ[2]*LZS;

	LMatrix[M00] = 1-(LYY+LZZ);
	LMatrix[M01] = LXY+LWZ;
	LMatrix[M02] = LXZ-LWY;

	LMatrix[M10] = LXY-LWZ;
	LMatrix[M11] = 1-(LXX+LZZ);
	LMatrix[M12] = LYZ+LWX;

	LMatrix[M20] = LXZ+LWY;
	LMatrix[M21] = LYZ-LWX;
	LMatrix[M22] = 1-(LXX+LYY);

	LMatrix[M03] = 0;
	LMatrix[M13] = 0;
	LMatrix[M23] = 0;
	LMatrix[M30] = 0;
	LMatrix[M31] = 0;
	LMatrix[M32] = 0;
	LMatrix[M33] = 1;
}

void RT::MatrixRotateAngleAxis(RT::Matrix &dst, RT::Float x, RT::Float y, RT::Float z, RT::Float a)
{	
	RT::Float aSin = sin(a/2*RT::DEGREES_TO_RADIANS);
	RT::Float aCos = cos(a/2*RT::DEGREES_TO_RADIANS);

	RT::Quaternion q; q[0] = x*aSin; q[1] = y*aSin; q[2] = z*aSin; q[3] = aCos;

	RT::Matrix qM;	
	RT::QuaternionToMatrix(q,qM); RT::MatrixMult(qM,dst);
}

void RT::MatrixScale(RT::Matrix &dst, RT::Float x, RT::Float y, RT::Float z)
{
	RT::Matrix m;

	//Build the translation matrix 
	RT::MatrixLoadIdentity(m);
	m[M00] = x;
	m[M11] = y;
	m[M22] = z;

	//concatinate to previously passed in matrix 
	RT::MatrixMult(m,dst);
}

void RT::MatrixTranslate(RT::Matrix &LMatrix, RT::Float LX, RT::Float LY, RT::Float LZ)
{
	LMatrix[M30]+=LX*LMatrix[M00]+LY*LMatrix[M10]+LZ*LMatrix[M20];
	LMatrix[M31]+=LX*LMatrix[M01]+LY*LMatrix[M11]+LZ*LMatrix[M21];
	LMatrix[M32]+=LX*LMatrix[M02]+LY*LMatrix[M12]+LZ*LMatrix[M22];
}
	
void RT::Matrix3x3Invert(const RT::Matrix &LM, RT::Matrix &LMI)
{
	//Use temporary storage, in case LM == LMI
	RT::Float LM00 = LM[M00],LM01 = LM[M01],LM02 = LM[M02], 
	LM10 = LM[M10],LM11 = LM[M11],LM12 = LM[M12],
	LM20 = LM[M20],LM21 = LM[M21],LM22 = LM[M22],
	LInvD = 1/(LM22*LM11*LM00-LM22*LM10*LM01-LM21*LM12*LM00+LM21*LM10*LM02+LM20*LM12*LM01-LM20*LM11*LM02);

	LMI[M00] = (LM22*LM11-LM21*LM12)*LInvD;
	LMI[M01] = (-LM22*LM01+LM21*LM02)*LInvD;
	LMI[M02] = (LM12*LM01-LM11*LM02)*LInvD;
	LMI[M10] = (-LM22*LM10+LM20*LM12)*LInvD;
	LMI[M11] = (LM22*LM00-LM20*LM02)*LInvD;
	LMI[M12] = (-LM12*LM00+LM10*LM02)*LInvD;
	LMI[M20] = (LM21*LM10-LM20*LM11)*LInvD;
	LMI[M21] = (-LM21*LM00+LM20*LM01)*LInvD;
	LMI[M22] = (LM11*LM00-LM10*LM01)*LInvD;
}
void RT::Matrix3x4Invert(const RT::Matrix &LM, RT::Matrix &LMI)
{
	Matrix3x3Invert(LM,LMI);
	//NOTE: THE TRANSLATION VECTOR IS INVERTED IN PLACE.
	RT::Float X = -LM[M30], Y = -LM[M31], Z = -LM[M32];
	LMI[M30] = X*LMI[M00]+Y*LMI[M10]+Z*LMI[M20];
	LMI[M31] = X*LMI[M01]+Y*LMI[M11]+Z*LMI[M21];
	LMI[M32] = X*LMI[M02]+Y*LMI[M12]+Z*LMI[M22];
}
		 
void RT::MatrixToQuat(const RT::Matrix &rotMat, RT::Quaternion &RotQuat)
{
	RT::Float fRoot;
	RT::Float fTrace = rotMat[M00]+rotMat[M11]+rotMat[M22];
	
	if(fTrace<=0) //|w| <= 1/2 
	{
		//Not worth fixing. This file is scheduled for removal.
		#define M(r,c) (0==Matrix_is_COLLADA_order?c*4+r:r*4+c)
				
		int i = 0;
		if(rotMat[M11]>rotMat[M00])
		i = 1;
		if(rotMat[M22]>rotMat[M(i,i)])
		i = 2;

		const int Next[3] = {1,2,0};
		const int j = Next[i];
		const int k = Next[j];

		fRoot = sqrt(rotMat[M(i,i)]-rotMat[M(j,j)]-rotMat[M(k,k)]+1);		
		RotQuat[i] = fRoot/2;
		fRoot = 0.5f/fRoot;
		RotQuat[3] = (rotMat[M(k,j)]-rotMat[M(j,k)])*fRoot;
		RotQuat[j] = (rotMat[M(j,i)]+rotMat[M(i,j)])*fRoot;
		RotQuat[k] = (rotMat[M(k,i)]+rotMat[M(i,k)])*fRoot;

		#undef M
	}
	else //|w| > 1/2, may as well choose w > 1/2
	{
		fRoot = (RT::Float)sqrt(fTrace+1); //2w
		RotQuat[3] = fRoot/2;
		fRoot = 0.5f/fRoot; //1/(4w)
		RotQuat[0] = (rotMat[M12]-rotMat[M21])*fRoot;
		RotQuat[1] = (rotMat[M20]-rotMat[M02])*fRoot;
		RotQuat[2] = (rotMat[M01]-rotMat[M10])*fRoot;
	}
};

//-------.
	}//<-'
}

/*C1071*/