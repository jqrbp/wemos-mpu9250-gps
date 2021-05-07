////////////////////////////////////////////////////////////////////////////
//
//  This file is part of linux-mpu9150
//
//  Copyright (c) 2013 Pansenti, LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of 
//  this software and associated documentation files (the "Software"), to deal in 
//  the Software without restriction, including without limitation the rights to use, 
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
//  Software, and to permit persons to whom the Software is furnished to do so, 
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all 
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "quaternion.h"

void quaternionNorm(quaternion_t q, double *n)
{
	*n = sqrtf(q[QUAT_W] * q[QUAT_W] + q[QUAT_X] * q[QUAT_X] +  
				q[QUAT_Y] * q[QUAT_Y] + q[QUAT_Z] * q[QUAT_Z]);
}

void quaternionNormalize(quaternion_t q)
{
	double length;

	quaternionNorm(q, &length);

	if (length == 0)
		return;

	q[QUAT_W] /= length;
	q[QUAT_X] /= length;
	q[QUAT_Y] /= length;
	q[QUAT_Z] /= length;
}

void quaternionToEuler(quaternion_t q, vector3d_t v)
{
	// fix roll near poles with this tolerance
	double pole = (double)M_PI / 2.0 - 0.05;

	double qysqr = q[QUAT_Y] * q[QUAT_Y];

	double sinr_cosp = 2.0 * (q[QUAT_W] * q[QUAT_X] + q[QUAT_Y] * q[QUAT_Z]);
    double cosr_cosp = 1.0 - 2.0 * (q[QUAT_X] * q[QUAT_X] + qysqr);
	double sinp = 2.0 * (q[QUAT_W] * q[QUAT_Y] - q[QUAT_Z] * q[QUAT_X]);

	double siny_cosp = 2.0 * (q[QUAT_W] * q[QUAT_Z] + q[QUAT_X] * q[QUAT_Y]);
    double cosy_cosp = 1.0 - 2.0 * (qysqr + q[QUAT_Z] * q[QUAT_Z]);

	// Keep sinp within range of asin (-1, 1)
    sinp = sinp > 1.0 ? 1.0 : sinp;
    sinp = sinp < -1.0 ? -1.0 : sinp;

	v[VEC3_Y] = asin(sinp);

	if ((v[VEC3_Y] < pole) && (v[VEC3_Y] > -pole)) {
		v[VEC3_X] = atan2(sinr_cosp, cosr_cosp);
	}

	v[VEC3_Z] = atan2(siny_cosp, cosy_cosp);
}

void eulerToQuaternion(vector3d_t v, quaternion_t q)
{
	double cosX2 = cos(v[VEC3_X] / 2.0);
	double sinX2 = sin(v[VEC3_X] / 2.0);
	double cosY2 = cos(v[VEC3_Y] / 2.0);
	double sinY2 = sin(v[VEC3_Y] / 2.0);
	double cosZ2 = cos(v[VEC3_Z] / 2.0);
	double sinZ2 = sin(v[VEC3_Z] / 2.0);

	q[QUAT_W] = cosX2 * cosY2 * cosZ2 + sinX2 * sinY2 * sinZ2;
	q[QUAT_X] = sinX2 * cosY2 * cosZ2 - cosX2 * sinY2 * sinZ2;
	q[QUAT_Y] = cosX2 * sinY2 * cosZ2 + sinX2 * cosY2 * sinZ2;
	q[QUAT_Z] = cosX2 * cosY2 * sinZ2 - sinX2 * sinY2 * cosZ2;

	quaternionNormalize(q);
}

void quaternionConjugate(quaternion_t s, quaternion_t d) 
{
	d[QUAT_W] = s[QUAT_W];
	d[QUAT_X] = -s[QUAT_X];
	d[QUAT_Y] = -s[QUAT_Y];
	d[QUAT_Z] = -s[QUAT_Z];
}
	
void quaternionMultiply(quaternion_t qa, quaternion_t qb, quaternion_t qd) 
{
	vector3d_t va;
	vector3d_t vb;
	double dotAB;
	vector3d_t crossAB;
	
	va[VEC3_X] = qa[QUAT_X];
	va[VEC3_Y] = qa[QUAT_Y];
	va[VEC3_Z] = qa[QUAT_Z];

	vb[VEC3_X] = qb[QUAT_X];
	vb[VEC3_Y] = qb[QUAT_Y];
	vb[VEC3_Z] = qb[QUAT_Z];

	vector3DotProduct(va, vb, &dotAB);
	vector3CrossProduct(va, vb, crossAB);
	
	qd[QUAT_W] = qa[QUAT_W] * qb[QUAT_W] - dotAB;
	qd[QUAT_X] = qa[QUAT_W] * vb[VEC3_X] + qb[QUAT_W] * va[VEC3_X] + crossAB[VEC3_X];
	qd[QUAT_Y] = qa[QUAT_W] * vb[VEC3_Y] + qb[QUAT_W] * va[VEC3_Y] + crossAB[VEC3_Y];
	qd[QUAT_Z] = qa[QUAT_W] * vb[VEC3_Z] + qb[QUAT_W] * va[VEC3_Z] + crossAB[VEC3_Z];
}

void tiltCompensate(quaternion_t magQ, quaternion_t unfusedQ)
{
	quaternion_t unfusedConjugateQ;
	quaternion_t tempQ;

	quaternionConjugate(unfusedQ, unfusedConjugateQ);
	quaternionMultiply(magQ, unfusedConjugateQ, tempQ);
	quaternionMultiply(unfusedQ, tempQ, magQ);
}

void quaternionLERP(quaternion_t a, const quaternion_t b, const double t)
{
	double t_ = 1 - t;
	a[QUAT_X] = t_*a[QUAT_X] + t*b[QUAT_X];
	a[QUAT_Y] = t_*a[QUAT_Y] + t*b[QUAT_Y];
	a[QUAT_Z] = t_*a[QUAT_Z] + t*b[QUAT_Z];
	a[QUAT_W] = t_*a[QUAT_W] + t*b[QUAT_W];
	quaternionNormalize(a);
}

void quaternionSLERP(quaternion_t a, const quaternion_t b, const double t)
{
	double t_ = 1 - t;
	double Wa, Wb;
	double theta = acos(a[QUAT_X]*b[QUAT_X] + a[QUAT_Y]*b[QUAT_Y] + a[QUAT_Z]*b[QUAT_Z] + a[QUAT_W]*b[QUAT_W]);
	double sn = sin(theta);
	Wa = sin(t_*theta) / sn;
	Wb = sin(t*theta) / sn;
	a[QUAT_X] = Wa*a[QUAT_X] + Wb*b[QUAT_X];
	a[QUAT_Y] = Wa*a[QUAT_Y] + Wb*b[QUAT_Y];
	a[QUAT_Z] = Wa*a[QUAT_Z] + Wb*b[QUAT_Z];
	a[QUAT_W] = Wa*a[QUAT_W] + Wb*b[QUAT_W];
	quaternionNormalize(a);
}