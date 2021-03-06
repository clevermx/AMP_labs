// AMP_1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;
using namespace concurrency;
using namespace fast_math;
void someUselesWork(double* a, int N) {
	//init stuf
	

	array_view<double, 1> d_a_init(N, a);
	

	parallel_for_each(d_a_init.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		[=](index<1> idx)restrict(amp) {//kernel-функция обработчик каждого потока 
		d_a_init(idx) = d_a_init(idx)*d_a_init(idx);

	});
	d_a_init.synchronize();
}
int main()
{
	int const REP_COUNT = 10;
	vector<accelerator> all = accelerator::get_all();
	for (int i = 0; i < all.size(); i++) {
		auto a = all[i];
		wcout << "Description: " << a.description << endl;
		wcout << "Path: " << a.device_path << endl;
		wcout << "Memmory : " << a.dedicated_memory << endl;
		wcout << "isEmulated: " << a.is_emulated << endl;
	}
	wcout << "==============================================================================" << endl;
	int N = 10000;

	double* a = new double[N];
	double* b = new double[N];
	double* c = new double[N];
	for (int i = 0; i < N; i++) {
		a[i] = rand()*1.0;
		b[i] = rand()*1.0;
	}
	clock_t start, stop;
	double sec = 0;


	int TestEl = rand() % (N - 1);

//
//	wcout << "++++++++++++COS+++++++" << "  N=" << N  << endl;
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();
//#pragma omp parallel for
//	for (int i = 0; i < N; i++) {
//		a[i] = cosf(a[i]);
//	}
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//
//	}
//	wcout << "omp cos " << sec << " Msec"<<"   Test_el =" << a[TestEl]<< endl;
//
//
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();
//	parallel_for(0, N,
//		[=](int i) { // = захватываем всё по значению, а & всё по ссылке можно конкретно перечислять: a,&b
//		a[i] = cosf(a[i]); });
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//
//}
//	wcout << "ppl cos " << sec << " Msec" << "   Test_el =" << a[TestEl]<<endl;
//
//	sec = 0;
//	someUselesWork(a, N);
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();
//	array_view<double, 1> d_a(N, a);
//	parallel_for_each(d_a.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
//		[=](index<1> idx)restrict(amp) {//kernel-функция обработчик каждого потока 
//		d_a(idx) =cos( d_a(idx));
//
//	});
//	d_a.synchronize();
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//	}
//	wcout << "amp cos " << sec << " Msec" << "   Test_el =" << a[TestEl] << endl;

	//--------------------------------------------------------------------------------
//сумма
//	wcout << "+++++++++++SUM++++++" << "  N=" << N << endl;
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();
//#pragma omp parallel for
//	for (int i = 0; i < N; i++) {
//		c[i] = a[i] + b[i];
//	}
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//
//	}
//	wcout << "omp sum " << sec << " Msec" << "   Test_el =" << c[TestEl] << endl;
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();
//
//
//
//
//	parallel_for(0, N,
//		[=](int i) {
//		c[i] = a[i]+b[i]; });
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//
//	}
//	wcout << "ppl sum " << sec << " Msec" << "   Test_el =" << c[TestEl] << endl;
//
//	sec = 0;
//	someUselesWork(c, N);
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();	
//	array_view< double, 1> d_b_s(N, b);
//	array_view< double, 1> d_a_s(N, a);
//	array_view<double, 1> d_c_s(N, c);
//	
//	parallel_for_each(d_c_s.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
//		[=](index<1> idx)restrict(amp) {//kernel-функция обработчик каждого потока 
//		d_c_s(idx) = d_a_s(idx)+d_b_s(idx);
//
//	});
//	d_c_s.synchronize();
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//
//	}
//	sec = sec / (REP_COUNT*1.0);
//	wcout << "amp sum " << sec << " Msec" <<"   Test_el =" << c[TestEl]<<endl;
//
//-------------------------------------------------------------------------------
	//умножение матрицы на число
//	N = 10000;
//	wcout << "++++ k*matrix +++++++++++++++++++++" << "  N=" << N << endl;
//
//	 a = new double[N*N];
//	 b = new double[N*N];
//	int k = 5;
//	for (int i = 0; i < N*N; i++) {
//		a[i] = rand()*1.0f;
//	}
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//
//	start = clock();
//#pragma omp parallel for
//	for (int i = 0; i < N; i++) {
//#pragma omp parallel for
//		for (int j = 0; j < N; j++) {
//			b[i*N + j] = a[i*N + j] * k;
//		}
//	}
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//
//	}
//	sec = sec / (REP_COUNT*1.0);
//	wcout << "omp k " << sec << " Msec" << b[TestEl] << endl;
//
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();
//	parallel_for(0, N,
//		[=](int i) {
//		for (int j = 1; j < N; j++) {
//			b[i*N + j] = a[i*N + j] * k;
//		}
//		 });
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//	
//	}
//	sec = sec / (REP_COUNT*1.0);
//	wcout << "ppl k " << sec  << " Msec" << b[TestEl] << endl;
//
//
//	someUselesWork(b, N);
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();
//	array_view<const double, 2> d_a_k(N,N, a);
//	array_view<double, 2> d_b_k(N, N, b);
//
//	parallel_for_each(d_b_k.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
//		[=](index<2> idx)restrict(amp) {//kernel-функция обработчик каждого потока 
//		d_b_k(idx) =d_a_k(idx) * 5;
//
//	});
//	d_b_k.synchronize();
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//
//	}
//	sec = sec / (REP_COUNT*1.0);
//	wcout << "amp k " << sec << " Msec" << b[TestEl] << endl;
N = 20000;
	wcout << "++++ transpon +++++++++++++++++++++" << "  N=" << N << endl;
	//транспонирование
	a = new double[N*N];
	b = new double[N*N];

	for (int i = 0; i < N*N; i++) {
		a[i] = rand()*1.0;
	}
	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
	start = clock();
#pragma omp parallel for
	for (int i = 0; i < N; i++) {
		for (int j = 0; j <=i ; j++) {
			b[j*N+i] = a[i*N + j];
		}
	}
	stop = clock();
	if (rep >= 1) {
		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
	}

	}
	sec = sec / (REP_COUNT*1.0); 

	wcout << "omp transpon " << sec << " Msec" << "  N=" << N << "   Test_el =" << b[TestEl] << endl;

	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
	start = clock();
	parallel_for(0, N,
		[=](int i) {
		for (int j = 0; j <= i; j++) {
			b[j*N + i] = a[i*N + j];
		}
		 });
	stop = clock();
	if (rep >= 1) {
		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
	}

	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "ppl transpon " << sec << " Msec" << "  N=" << N << "   Test_el =" << b[TestEl] << endl;

	someUselesWork(b, N);
	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
	start = clock();
	array_view<const double, 2> d_a_t(N, N, a);
	array_view<double, 2> d_b_t(N, N, b);

	parallel_for_each(d_b_t.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		[=](index<2> idx)restrict(amp) {//kernel-функция обработчик каждого потока 
		d_b_t(idx) = d_a_t(idx[1], idx[0]);

	});
	d_b_t.synchronize();
	stop = clock();
	if (rep >= 1) {
		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
	}

	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "amp transpon " << sec << " Msec" << "  N=" << N << "   Test_el =" << b[TestEl] << endl;

//	//-------------------------------------------------------------------------------------------
//	//матричное умножение
//	wcout << "++++++++++++++++++++++Matrix multiplication+++++++++++++++++++++++++" << endl;
//	N = 1000;
//	a = new float[N*N];
//	b = new float[N*N];
//	c = new float[N*N];
//	
//	for (int i = 0; i < N*N; i++) {
//		a[i] = i;
//		b[i] = i;
//		c[i] = 0;
//	}
//	/*sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//	start = clock();
//#pragma omp parallel for
//	for (int i = 0; i < N; i++) {
//		for (int j = 0; j < N; j++) {
//			double temp = 0;
//			for (int g = 0; g < N; g++) {
//				temp+= a[i*N + g] * b[g*N + j];
//			}
//			c[i*N + j] = temp;
//		}
//	}
//	stop = clock();
//	if (rep >= 1) {
//		sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//	}
//
//	}
//	sec = sec / (REP_COUNT*1.0);
//	wcout << "omp matrix mult " << sec << " Msec" << "  N=" << N << endl;
//
//	*/
//
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//		start = clock();
//		parallel_for(0, N,
//			[=](int i) {
//			for (int j = 0; j < N; j++) {
//				double temp = 0;
//				for (int g = 0; g < N; g++) {
//					temp += a[i*N + g] * b[g*N + j];
//				}
//				c[i*N + j] = temp;
//			}
//		});
//		stop = clock();
//		if (rep >= 1) {
//			sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//		}
//
//	}
//	sec = sec / (REP_COUNT*1.0);
//	wcout << "ppl mult " << sec << " Msec" << "  N=" << N << endl;
//
//	
//
//	someUselesWork(a, N);
//	sec = 0;
//	for (int rep = 0; rep <= REP_COUNT; rep++) {
//		start = clock();
//		array_view<float, 2> d_a_m(N, N, a);
//		array_view<float, 2> d_b_m(N, N, b);
//		array_view<float, 2> d_c_m(N, N, c);
//
//		parallel_for_each(d_c_m.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
//			[=](index<2> idx)restrict(amp) {//kernel-функция обработчик каждого потока 			
//				double temp = 0;
//				for (int g = 0; g < N; g++) {
//					temp += d_a_m(idx[0], g)*d_b_m(g, idx[1]);					
//				}
//				d_c_m(idx) = temp;
//
//		});
//		d_c_m.synchronize();
//		stop = clock();
//		if (rep >= 1) {
//			sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
//		}
//
//	}
//	sec = sec / (REP_COUNT*1.0);
//	wcout << "amp mult " << sec << " Msec" << "  N=" << N << endl;
//	
//	/*
//	


	//parallel_for(0,N,
	//	[=](int i) { // = захватываем всё по значению, а & всё по ссылке можно конкретно перечислять: a,&b
	//	a[i] = cosf(a[i]); });
	//
	getchar();
    return 0;
}

