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
		C += A_temp * B_temp;
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
	hls::stream<float> A_inter[M][N+1];
#pragma HLS STREAM variable=A_inter
	hls::stream<float> B_inter[M+1][N];
#pragma HLS STREAM variable=B_inter

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

void test(float* A_ddr, float* B_ddr, float* C_ddr){
#pragma HLS INTERFACE m_axi depth=32 port=A_ddr
#pragma HLS INTERFACE m_axi depth=32 port=B_ddr
#pragma HLS INTERFACE m_axi depth=32 port=C_ddr

	static float A[M][K];
#pragma HLS array_partition variable=A complete dim=1
	static float B[K][N];
#pragma HLS array_partition variable=B complete dim=2
	static float C[M][N];
#pragma HLS array_partition variable=C complete dim=0

	//Load A
	for(int ak=0;ak<K;ak++){
#pragma HLS PIPELINE
		for(int m=0;m<M;m++){
			A[m][ak] = *(A_ddr++);
		}
	}

	//Load B
	for(int bk=0;bk<K;bk++){
#pragma HLS PIPELINE
		for(int n=0;n<N;n++){
			B[bk][n] = *(B_ddr++);
		}
	}

	PE_array(A,B,C);

	//offload C
	for(int m=0;m<M;m++){
#pragma HLS PIPELINE
		for(int n=0;n<N;n++){
			*(C_ddr++) = C[m][n];
		}
	}

}


