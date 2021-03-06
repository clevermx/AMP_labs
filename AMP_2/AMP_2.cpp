// AMP_2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "io.h"
using namespace std;
using namespace concurrency;
using namespace fast_math;

 double arrSum(double *a, int n) {
	 double sum = 0;
	 for (int i = 0; i < n; i++) {
		 sum += a[i];
	 }
	 return sum;
}
 template <typename T>
 void printMatrix(T* source, int len1, int len2) {
	 T result = 0;
	 for (int i = 0; i < len1; i++) {
		 for (int j = 0; j < len2; j++)
		 {
			 cout <<source[i*len1 + j] << ", ";

		 }
		 cout<<"|"<< endl;
	 }

 }
 void singleTranspon(double *b, int N) {
	 double t = 0;
	 for (int i = 0; i < N; i++) {
		 for (int j = 0; j <= i; j++) {
			 t = b[j*N + i];
			 b[j*N + i] = b[i*N + j];
			 b[i*N + j] = t;

		 }
	 }
 }
 void ampUsualTranspon(double *b, int N) {
	 array_view<double, 2> d_b_t(N, N, b);

	 parallel_for_each(d_b_t.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		 [=](index<2> idx)restrict(amp) {//kernel-функция обработчик каждого потока 
		 if (idx[1] > idx[0]) {
			 double t = d_b_t(idx);

			 d_b_t(idx) = d_b_t(idx[1], idx[0]);
			 d_b_t(idx[1], idx[0]) = t;
		 }

	 });
	 d_b_t.synchronize();
 }
 template <int tileSize>
 void ampBlockTranspon(double *b,  int N) {
	 array_view<double, 2> d_b_t(N, N, b);
	 d_b_t.discard_data();
	 array_view<const double, 2> d_a_t(N, N, b);
	 parallel_for_each(d_a_t.extent.tile<tileSize,tileSize>(), //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		 [=](tiled_index<tileSize, tileSize> tidx)restrict(amp) {//kernel-функция обработчик каждого потока
		 tile_static double localData[tileSize][tileSize];
		 localData[tidx.local[1]][tidx.local[0]] = d_a_t(tidx.global);
		 tidx.barrier.wait();
		 index<2> outIndx(index<2>(tidx.tile_origin[1], tidx.tile_origin[0]) + tidx.local);
		 d_b_t(outIndx) = localData[tidx.local[0]] [tidx.local[1]];

	 });
	 d_b_t.synchronize();
 }

 void ampUsualMult(double *a, double * b, double * c, int N) {
	
	 array_view<const double, 2> d_a_m(N, N, a);
	 array_view<const double, 2> d_b_m(N, N, b);
	 array_view<double, 2> d_c_m(N, N, c);

	 parallel_for_each(d_c_m.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		 [=](index<2> idx)restrict(amp) {//kernel-функция обработчик каждого потока 			
		 double temp = 0;
		 for (int g = 0; g < N; g++) {
			 temp += d_a_m(idx[0], g)*d_b_m(g, idx[1]);
		 }
		 d_c_m(idx) = temp;

	 });
	 d_c_m.synchronize();
 }
 template <int tileSize>
 void ampBlockMultShared(double *a, double * b, double * c, int N) {

	 array_view<const double, 2> d_a_m(N, N, a);
	 array_view<const double, 2> d_b_m(N, N, b);
	 array_view<double, 2> d_c_m(N, N, c);
	 d_c_m.discard_data();
	 parallel_for_each(d_c_m.extent.tile<tileSize, tileSize>(), //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		 [=](tiled_index<tileSize, tileSize> tidx)restrict(amp) {//kernel-функция обработчик каждого потока
		 double temp = 0;
		 int row = tidx.local[0];
		 int col = tidx.local[1];
		 tile_static double localPartA[tileSize][tileSize];
		 tile_static double localPartB[tileSize][tileSize];
		 for (int g = 0; g < N; g+=tileSize) {
			 localPartA[row][col] = d_a_m(tidx.global[0], col + g);
			 localPartB[row][col] = d_b_m(row + g, tidx.global[1]);
			 tidx.barrier.wait();
			 for (int k = 0; k < tileSize; k++) {
				 temp += localPartA[row][k] * localPartB[k][col];
			 }
			 tidx.barrier.wait();
		 }
		 d_c_m(tidx.global) = temp;

	 });
	 d_c_m.synchronize();
 }
 template <int tileSize>
 void ampBlockMult(double *a, double * b, double * c, int N) {

	 array_view<const double, 2> d_a_m(N, N, a);
	 array_view<const double, 2> d_b_m(N, N, b);
	 array_view<double, 2> d_c_m(N, N, c);
	 d_c_m.discard_data();
	 parallel_for_each(d_c_m.extent.tile<tileSize, tileSize>(), //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		 [=](tiled_index<tileSize, tileSize> tidx)restrict(amp) {//kernel-функция обработчик каждого потока
		 double temp = 0;
		 int row = tidx.global[0];
		 int col = tidx.global[1];
		 for (int k = 0; k < N; k++) {
				 temp += d_a_m[row][k] * d_b_m[k][col];
		}
		 d_c_m(tidx.global) = temp;

	 });
	 d_c_m.synchronize();
 }

 void ampEnlargedMult(double *a, double * b, double * c, int N) {

	 array_view<const double, 2> d_a_m(N, N, a);
	 array_view<const double, 2> d_b_m(N, N, b);
	 array_view <int, 1> ind_arr(N);
	 array_view<double, 2> d_c_m(N, N, c);
	 ind_arr.discard_data();
	 d_c_m.discard_data();
	 parallel_for_each(ind_arr.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		 [=](index<1> idx)restrict(amp) {//kernel-функция обработчик каждого потока 			
		 double temp = 0.0;
		 int row = idx[0];
		 for (int i= 0; i < N; i++) {
			 temp = 0.0;
			 for (int k = 0; k < N; k++) {
				 temp += d_a_m(row, k)*d_b_m(k, i);
			 }
			 d_c_m(row,i) = temp;
		 }
	 });
	 d_c_m.synchronize();
 }

 void someUselesWork(double* a, int N) {
	 //init stuf


	 array_view<double, 1> d_a_init(N, a);


	 parallel_for_each(d_a_init.extent, //информация о конфигурации вычислений. сколько элементов в указанном буфере, столько будет потоков. Число блоков нам не известно, оставляем на amp
		 [=](index<1> idx)restrict(amp) {//kernel-функция обработчик каждого потока 
		 d_a_init(idx) = d_a_init(idx)*d_a_init(idx);

	 });
	 d_a_init.synchronize();
 }
 void resetMatrix(double * a, int sz) {
	 for (int i = 0; i < sz; i++) {
		 for (int j = 0; j < sz; j++)
		 {
			 a[sz*i + j] = i;
		 }
	 }
 }
int main()
{
	vector<accelerator> all = accelerator::get_all();
	for (int i = 0; i < all.size(); i++) {
		auto a = all[i];
		wcout << "Description: " << a.description << endl;
		wcout << "Path: " << a.device_path << endl;
		wcout << "Memmory : " << a.dedicated_memory << endl;
		wcout << "isEmulated: " << a.is_emulated << endl;
	}
	wcout << "==============================================================================" << endl;
	int N;
	int const REP_COUNT=2;
	double *a;
	double *b;
	double *c;
	double start,stop,sec;

	//транспонирование в один поток
	N = 6400;
	a = new double[N*N];
	b = new double[N*N];

	for (int i = 0; i < N*N; i++) {
		a[i] = i;
		b[i] = a[i];
	}
	sec = 0;
	
	for (int rep = 0; rep <= REP_COUNT; rep++) {
		start = clock();
		singleTranspon(b, N);
		stop = clock();
		if (rep >= 1) {
			sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
		}
	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "single transpon " << sec << " Msec" << "  N=" << N << "   a_sum =" <<arrSum(a,N) << "   b_sum =" << arrSum(b, N) << endl;
	//транспонирование в амп обычное	
	someUselesWork(b, N);
	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
		start = clock();		
		ampUsualTranspon(a, N);
		stop = clock();
		if (rep >= 1) {
			sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
		}

	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "amp usual transpon " << sec << " Msec" << "  N=" << N << "   res_sum =" << arrSum(a, N) << endl;
	ampUsualTranspon(a, N);
	//транспонирование в амп блочное
	const int  tileSize = 16;
	someUselesWork(b, N);
	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
		start = clock();
		ampBlockTranspon<tileSize>(a,N);
		stop = clock();
		if (rep >= 1) {
			sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
		}

	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "amp block transpon "<<"block size "<< tileSize<< ":  " << sec << " Msec" << "  N=" << N << "   res_sum =" << arrSum(a, N) << endl;
	ampUsualTranspon(a, N);
	
	wcout << "=====================================Multiplication========================================" << endl;
	
	//Умножение amp обычное 
	N = 640;
	a = new double[N*N];
	resetMatrix(a, N);
	someUselesWork(a, N);
	resetMatrix(a, N);
	b = new double[N*N];
	c = new double[N*N];
	resetMatrix(b, N);	

	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
			start = clock();
			ampBlockMultShared<tileSize>(a, b, c, N);
			stop = clock();
			if (rep >= 1) {
				sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
			}
	
	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "amp usual mult " << "block size " << tileSize << ":  " << sec << " Msec" << "  N=" << N << "   res_sum =" << arrSum(c, N*N) << endl;
	
	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
		start = clock();
		ampUsualMult(a, b, c, N);
		stop = clock();
		if (rep >= 1) {
			sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
		}

	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "amp block mult  " << "block size " << tileSize << ":  " << sec << " Msec" << "  N=" << N << "   res_sum =" << arrSum(c, N*N) << endl;
	
	

	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
		start = clock();
		ampBlockMult<tileSize>(a, b, c, N);
		stop = clock();
		if (rep >= 1) {
			sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
		}

	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "amp block mult with shared " << "block size " << tileSize << ":  " << sec << " Msec" << "  N=" << N << "   res_sum =" << arrSum(c, N*N) << endl;

	sec = 0;
	for (int rep = 0; rep <= REP_COUNT; rep++) {
		start = clock();
		ampEnlargedMult(a, b, c, N);
		stop = clock();
		if (rep >= 1) {
			sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
		}

	}
	sec = sec / (REP_COUNT*1.0);
	wcout << "amp enlarged mult " << sec << " Msec" << "  N=" << N << "   res_sum =" << arrSum(c, N*N) << endl;

	system("pause");
	return 0;
}

