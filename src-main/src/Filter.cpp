#include "stdafx.h"
#include "Nintendulator.h"
#include <xmmintrin.h>
#include <pmmintrin.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Filter.h"

namespace Filter {
LPF_RC::LPF_RC():
	a0(1.0),
	b1(0.0),
	z1(0.0) {
}

LPF_RC::LPF_RC(double Fc):
	z1(0.0) {
	setFc(Fc);
}

void LPF_RC::setFc(double Fc) {
	b1 = exp(-2.0 * M_PI * Fc);
	a0 = 1.0 - b1;
}

float LPF_RC::process(float in) {
	return z1 = in * a0 + z1 * b1;
}

HPF_RC::HPF_RC():
	LPF_RC() {
}

HPF_RC::HPF_RC(double Fc):
	LPF_RC(Fc) {
}

float HPF_RC::process(float in) {
	return in -LPF_RC::process(in);
}

Butterworth::Butterworth(int _n, double s, double f):
	w2(nullptr),
	w1(nullptr),
	w0(nullptr),
	d2(nullptr),
	d1(nullptr),
	A(nullptr) {
	recalc(_n, s, f);
}

Butterworth::~Butterworth() {
	flush();
}

void Butterworth::flush() {
	if (w2) free(w2);
	if (w1) free(w1);
	if (w0) free(w0);
	if (d2) free(d2);
	if (d1) free(d1);
	if (A) free(A);
	w2 = w1 = w0 = d2 = d1 = A = nullptr;
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}

double Butterworth::output(double x) {
	for(int i =0; i <n; ++i) {
		w0[i] = d1[i]*w1[i] + d2[i]*w2[i] + x;
		x = A[i]*(w0[i] + 2.0*w1[i] + w2[i]);
		w2[i] = w1[i];
		w1[i] = w0[i];
	}
	return x;
}

void Butterworth::recalc(int _n, double s, double f) {
	flush();
	n = _n/2;
	A = (double *)malloc(n*sizeof(double));
	d1 = (double *)malloc(n*sizeof(double));
	d2 = (double *)malloc(n*sizeof(double));
	w0 = (double *)calloc(n, sizeof(double));
	w1 = (double *)calloc(n, sizeof(double));
	w2 = (double *)calloc(n, sizeof(double));
	double a = tan(M_PI*f/s);
	double a2 = a*a;
	double r;
	for(int i =0; i <n; ++i) {
		r = sin(M_PI*(2.0*i+1.0)/(4.0*n));
		s = a2 + 2.0*a*r + 1.0;
		A[i] = a2/s;
		d1[i] = 2.0*(1-a2)/s;
		d2[i] = -(a2 - 2.0*a*r + 1.0)/s;
	}
}

} // namespace Filter