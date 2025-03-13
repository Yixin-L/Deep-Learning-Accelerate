#define CHin 4
#define CHout 4
#define R 14
#define C 14
#define Rin 16
#define Cin 16
#define K 3

void test1(){
	float In[CHin][Rin][Cin];
#pragma HLS array_partition variable=In complete dim=1
	float Out[CHout][R][C];
#pragma HLS array_partition variable=Out complete dim=1
	float W[CHout][CHin][K][K];
#pragma HLS array_partition variable=W complete dim=1
#pragma HLS array_partition variable=W complete dim=2

	Row:
	for(int r=0;r<R;r++){
		Column:
		for(int c=0;c<C;c++){
			Kernel_Row:
			for(int kr=0;kr<K;kr++){
				Kernel_Column:
				for(int kc=0;kc<K;kc++){
					Output_Channel:
					for(int cho=0;cho<CHout;cho++){
#pragma HLS UNROLL
						Input_Channel:
						for(int chi=0; chi<CHin;chi++){
#pragma HLS UNROLL
							Out[cho][r][c]+=In[chi][r+kr][c+kc]*W[cho][chi][kr][kc];
						}
					}
				}
			}
		}
	}

}
