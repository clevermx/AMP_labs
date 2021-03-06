// AMP_3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;
using namespace concurrency;
using namespace fast_math;
//Практическая работа №3.Алгоритмы редукции с помощью AMP C++
//
//Реализовать 4 варианта алгоритма редукции и сравнить их эффективность по времени вычислений.В качестве задачи редукции можно использовать : сложение элементов вектора, умножение элементов вектора, поиск максимального или минимального элемента вектора.
//
//1. Редукция без блочной декомпозиции
//
//В этой реализации не используется блочно - статическая(разделяемая) память и не выделяются блоки потоков.
//В коде host - программы выполняется цикл c уменьшением шага смещения(stride).На каждой итерации цикла 
//выполняется запуск GPU - вычислений.В ядерной функции потоки выполняют редукцию пар элементов, находящихся 
//на расстоянии stride.Результат помещается в позицию первого элемента : a[i] = a[i] + a[i + stride];


double MaxSimpleReduct(double * a, unsigned int N) {
	concurrency::array <double, 1> d_a(N, a);
	double oddElem = N & 0x1 ? a[N - 1] : -DBL_MAX;
	Concurrency::array_view <double, 1> tail(1, &oddElem);
	double * result=new double;
	
	for (unsigned int s =  (N / 2); s > 0;s= s / 2) {
		parallel_for_each(concurrency::extent<1>(s),
			[=, &d_a](index<1> idx) restrict(amp) {
			d_a[idx] = max(d_a[idx], d_a[idx + s]);

			if (idx[0] == s - 1 && s & 0x1 && s != 1 && tail[0]< d_a[s-1]) {
				tail[0] = d_a[s - 1];
			}
		});
	}
	concurrency::copy(d_a.section(0, 1), result);
	tail.synchronize();
	return max( *result,oddElem);
}




//2.  Редукция без блочной декомпозиции с окном
//
//В этой реализации каждый поток, выполняющий редукцию на данной итерации, осуществляет операцию над 
//несколькими элементами(число элементов определяет ширину окна).Например,
//
//a[i] = a[i] + a[i + stride] + a[i + 2 * stride] + a[i + 3 * stride];
//

double MaxWindowReduct(double * a,  int N,int width) {

	double tail_max = a[N-1];
	if  ((N>width)&&(N% width != 0)){
		for (int i = 1; i <= N%width; i++) {			
				tail_max = max(tail_max, a[N-i]);			
		}
	}
	Concurrency::array_view <double, 1> tail(1, &tail_max);
	concurrency::array <double, 1> d_a(N, a);	
	
	int prevS = N;
	for (unsigned int s = N/width; s > 0; s = s / width) {
		parallel_for_each(concurrency::extent<1>(s),
			[=, &d_a](index<1> idx) restrict(amp) {
			double temp = d_a[idx];
			for (int i = 1; i < width; i++) {
			temp =max(temp, d_a[idx + i*s]);
			}
			d_a[idx] = temp;
			if (idx[0] == s - 1 && s%width !=0 && s>width ){
				for (int i = 1; i < N%width; i++) {
					temp = max(temp, d_a[N - i]);
				}
			}
		});
		prevS = s;
	}
	
	double * result = new double[prevS];
	concurrency::copy(d_a.section(0, prevS), result);
	double tmp = result[0];
	for (int i = 1; i < prevS; i++) {
		tmp = max(result[i], tmp);
	}
	tail.synchronize();
	return max(tail_max,tmp);
}



//3.  Блочный алгоритм с расхождением
//
//В этой реализации выполняется блочная декомпозиция.Каждый блок выполняет редукцию части массива,
//используя разделяемую память.Внутри ядерной функции выполняется цикл с последовательным увеличением сдвига
//для редукции пар элементов.На каждой итерации этого цикла число работающих потоков сокращается вдвое, 
//а сдвиг увеличивается вдвое.В конечном итоге единственный поток(с номером 0) осуществляет редукцию 
//пар элементов и записывает результат в нулевую позицию в разделяемой памяти.
//Затем поток сохраняет результат в глобальной памяти в соответствии с абсолютным положением первого 
//потока в общем массиве.
//
//После завершения GPU - вычислений число элементов сокращается в M раз, где M – число элементов в блоке.При этом остается столько элементов, сколько блоков образуется с учетом общего числа элементов(N / M).Это число может быть еще очень большим, чтобы поместиться в одном блоке.Поэтому необходимо выполнить несколько итераций в host - программе с запуском GPU - вычислений с уменьшением числа обрабатываемых элементов и сокращением числа блоков.Цикл можно завершить, когда число элементов станет меньше размера блока.Конечную редукцию можно выполнить уже на CPU.
//

template <int tileSize>
double maxBlockRash(double * a, unsigned int N) {
	concurrency::array <double, 1>arr_1 (N, a);
	concurrency::array <double, 1>arr_2(N / tileSize == 0 ? 1 : N / tileSize);
	array_view <double, 1> source(arr_1);
	array_view <double, 1> dst(arr_2);
	dst.discard_data();
	while (N % tileSize== 0) {
		parallel_for_each(concurrency::extent<1>(N).tile<tileSize>(),
			[=](tiled_index<tileSize> tidx) restrict(amp)
		{
			tile_static double tileData[tileSize];
			int local_idx = tidx.local[0];
			tileData[local_idx] = source[tidx.global];
			tidx.barrier.wait();
			for (unsigned s = 1; s < tileSize; s *= 2)
			{
				if (local_idx % (2 * s) == 0)
				{
					tileData[local_idx]= max(tileData[local_idx], tileData[local_idx + s]);
				}
				tidx.barrier.wait();
			}
			if (local_idx == 0)
			{
				dst[tidx.tile] = tileData[0];
			}
		});
		N = N / tileSize;
		std::swap(source, dst);
		dst.discard_data();
	}
	double * result = new double[N];
	concurrency::copy(source.section(0, N), result);
	double tmp = result[0];
	for (int i = 1; i < N; i++)
	{
		tmp = max(tmp, result[i]);
	}
	return tmp;
}


//4. Блочный каскадный алгоритм
//
//В этой реализации используется «окно обработки» в ядерной функции.Если в предыдущей версии, каждый поток блока
//загружал единственный элемент из глобальной памяти в разделяемую память, то в этой реализации каждый поток
//сохраняет в разделяемой памяти результат редукции нескольких элементов(например,восьми).
//
//Кроме этого, в этой реализации потоки выполняют редукцию элементов разделяемой памяти, таким образом, 
//чтобы на первой итерации были заняты все первые M / 2 - потоков; на второй итерации заняты все первые 
//M / 4 - потоков.Для этого шаг смещения(stride) последовательно уменьшается, начиная с M / 2 и заканчивая 1.
//
//В этой реализации за счет окна редукции можно ограничиться единственным запуском GPU - вычислений.


template <int tileSize, int tileCount,int batch_size>
double maxBlockCascade(double * a, unsigned int N) {
	int stride = tileCount * batch_size*tileSize;// каждый блок обрабатывает batch_size за один шаг
	int ostN = N % stride;
	double tail_max=-DBL_MAX;
	if (ostN > 0) {
		tail_max = a[ostN - 1];
		for (int i = 1; i < ostN; i++)
		{
			if (tail_max < a[N - i]){
				tail_max = a[N - i];
			}
		}
		N = N - ostN;
	}
	if (N > 0) {


		concurrency::array <double, 1>arr_1(N, a);
		concurrency::array <double, 1>partRes(tileCount);

		parallel_for_each(concurrency::extent<1>(tileCount*tileSize).tile<tileSize>(),
			[=, &arr_1, &partRes](tiled_index<tileSize> tidx) restrict(amp) {

			tile_static double tile_data[tileSize];
			unsigned local_idx = tidx.local[0];
			double temp = arr_1[tidx.global];
			for (int batch_num = 0; batch_num <= N / stride; batch_num++) {
				double batch_start = batch_num * stride + tidx.tile[0] * tileSize*batch_size;
				for (int i = 0; i < batch_size; i++) {
					temp = max(temp, arr_1[batch_start + i * tileSize + local_idx]);
				}
			}
			tile_data[local_idx] = temp;
			tidx.barrier.wait();
			double odd;
			for (int tile_stride = tileSize / 2; tile_stride > 0; tile_stride /= 2) {
				if (local_idx < tile_stride) {
					tile_data[local_idx] = max(tile_data[local_idx], tile_data[local_idx + tile_stride]);
				}
				tidx.barrier.wait();
			}
			if (local_idx == 0) {
				partRes[tidx.tile[0]] = tile_data[0];
			}

		});
		double result[tileCount];
		concurrency::copy(partRes, result);
		for (int i = 1; i < tileCount; i++)
		{
			tail_max = max(tail_max, result[i]);

		}
	}
	return tail_max;
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
void resetArr(double * a, int sz) {
	for (int i = 0; i < sz; i++) {
			a[i] = i;
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
	double *a;
	double start, stop, sec;
	double res;
	N = 32000;
	a = new double[N];
	resetArr(a, N);
	someUselesWork(a, N);
	resetArr(a, N);
	sec = 0;
	start = clock();
	res = MaxSimpleReduct(a, N);

	stop = clock();
	sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
	wcout << "SimleBinRed " << sec << " Msec" << "  N=" << N <<" result = "<< res  <<endl;
	someUselesWork(a, N);
	resetArr(a, N);
	sec = 0;
	start = clock();
	res = MaxWindowReduct(a, N,5);

	stop = clock();
	sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
	wcout << "WindowReduct " << sec << " Msec" << "  N=" << N << " result = " << res << endl;


	//someUselesWork(a, N);
	//resetArr(a, N);
	//sec = 0;
	//start = clock();
	//res =maxBlockRash<16>(a,N);

	//stop = clock();
	//sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
	//wcout << "BlockR " << sec << " Msec" << "  N=" << N << " result = " << res << endl;
	//
	someUselesWork(a, N);
	resetArr(a, N);
	sec = 0;
	start = clock();
	res = maxBlockCascade<16,4,8>(a, N);

	stop = clock();
	sec += (double)(stop - start)*1000.0 / (CLK_TCK*1.0);
	wcout << "BlockCascade " << sec << " Msec" << "  N=" << N << " result = " << res << endl;

	system("pause");
    return 0;
}

