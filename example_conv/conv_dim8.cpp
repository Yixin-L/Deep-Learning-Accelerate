#define CHin 4
#define CHout 4
#define R 14
#define C 14
#define Rin 16
#define Cin 16
#define K 3

void test(float* In_ddr, float* W_ddr, float* Out_ddr){
#pragma HLS INTERFACE m_axi depth=32 port=In_ddr
#pragma HLS INTERFACE m_axi depth=32 port=W_ddr
#pragma HLS INTERFACE m_axi depth=32 port=Out_ddr

	float In[4][Rin][Cin];
#pragma HLS array_partition variable=In complete dim=1
	float Out[4][R][ C];
#pragma HLS array_partition variable=Out complete dim=1
	float W[4][4][K][K];
#pragma HLS array_partition variable=W complete dim=1
#pragma HLS array_partition variable=W complete dim=2

	Output_Channel_Tiling:
	for(int cho=0;cho<CHout;cho+=4){ //每次自增一块（4）
		Input_Channel_Tiling:
		for(int chi=0;chi<CHin;chi+=4){
			//Load In tile from DRAM
			for(int L_ri=0;L_ri<Rin;L_ri++){
#pragma HLS PIPELINE
				for(int L_ci=0;L_ci<Cin;L_ci++){
					for(int L_chi=0;L_chi<4;L_chi++){
						In[L_chi][L_ri][L_ci]=*In_ddr++;
					}
				}
			}
			//Load W tile from DRAM
			for(int L_cho=0;L_cho<4;L_cho++){
#pragma HLS PIPELINE
				for(int L_chi=0;L_chi<4;L_chi++){
					for(int L_kr=0;L_kr<K;L_kr++){
						for(int L_kc=0;L_kc<K;L_kc++){
							W[L_cho][L_chi][L_kr][L_kc]= *W_ddr++;
						}
					}
				}
			}
			Kernel_Row:
			for(int kr=0;kr<K;kr++){
				Kernel_Column:
				for(int kc=0;kc<K;kc++){
					Row:
					for(int r=0;r<R;r++){
						Column:
						for(int c=0;c<C;c++){
#pragma HLS PIPELINE
							Output_Channel:
							for(int cho=0;cho<CHout;cho++){
								Input_Channel:
								for(int chi=0; chi<CHin;chi++){
									Out[cho][r][c]+=In[chi][r+kr][c+kc]*W[cho][chi][kr][kc];
								}
							}
						}
					}
				}
			}

			//Offload Out tile to DRAM
			for(int L_ro=0;L_ro<R;L_ro++){
#pragma HLS PIPELINE
				for(int L_co=0;L_co<C;L_co++){
					for(int L_cho=0;L_cho<4;L_cho++){
						*Out_ddr++ = Out[L_cho][L_ro][L_co];
					}
				}
			}
		}
	}
	return;

}
