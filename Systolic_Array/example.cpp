#include "hls_stream.h"

#define M 3
#define K 3
#define N 3

void PE(hls::stream<float> &A_pre, hls::stream<float> &B_pre,
		hls::stream<float> &A_nxt,hls::stream<float> &B_nxt, float C){
	float A_temp, B_temp;

	K_loop:
	for(int k=0;k<K;k++){
#pragma HLS PIPELINE
		A_pre >> A_temp;
		B_pre >> B_temp;
		//MAC
		C+= A_temp * B_temp;
		A_nxt << A_temp;
		B_nxt << B_temp;
	}
	return;
}

void Load_A(int m, float A[M][K], hls::stream<float> &A_nxt){
	for(int k=0;k<K;k++){
		A_nxt << A[m][k];
	}
	return;
}

void Load_B(int n, float B[K][N], hls::stream<float> &B_nxt){
	for(int k=0;k<K;k++){
		B_nxt << B[k][n];
	}
	return;
}

void Drain_A (hls::stream<float> &A_pre){
	for(int k=0;k<K;k++){
		float drain;
		A_pre >> drain;
	}
	return;
}

void Drain_B (hls::stream<float> &B_pre){
	for(int k=0;k<K;k++){
		float drain;
		B_pre >> drain;
	}
	return;
}

void PE_array(float A[M][K], float B[K][N], float C[M][N]){
	hls::stream<float> A_inter[M][N+1]; //横向传递A元素
#pragma HLS STREAM variable=A_inter;
	hls::stream<float> B_inter[M+1][N];//纵向传递B元素
#pragma HLS STREAM variable=B_inter;

#pragma HLS DATAFLOW
	for(int m=0;m<M;m++){
#pragma HLS UNROLL
		Load_A(m, A, A_inter[m][0]);
	}

	for(int n=0;n<N;n++){
#pragma HLS UNROLL
		Load_B(n, B, B_inter[0][n]);
	}

	C_Row:
	for(int m=0;m<M;m++){
#pragma HLS UNROLL
		C_Col:
		for(int n=0;n<N;n++){
#pragma HLS UNROLL
			PE(A_inter[m][n],B_inter[m][n],A_inter[m][n+1],B_inter[m+1][n],C[m][n]);
		}
	}

	//Drain
	for(int m=0;m<M;m++){
#pragma HLS UNROLL
		Drain_A(A_inter[m][N]);
	}

	for(int n=0;n<N;n++){
#pragma HLS UNROLL
		Drain_B(B_inter[M][n]);
	}
	return;
}

void test(){}


