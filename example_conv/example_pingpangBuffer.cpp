#define CHin 4
#define CHout 4
#define R 14
#define C 14
#define Rin 16
#define Cin 16
#define K 3

void Load_In(float* In_ddr, float In[4][Rin][Cin], bool exe){
	if(!exe)
		return;
	for(int L_ri=0;L_ri<Rin;L_ri++){
#pragma HLS PIPELINE
		for(int L_ci=0;L_ci<Cin;L_ci++){
			for(int L_chi=0;L_chi<4;L_chi++){
				In[L_chi][L_ri][L_ci]=*In_ddr++;
			}
		}
	}
	return;
}

void Load_W(float* W_ddr, float W[4][4][K][K], bool exe){
	if(!exe)
		return;
	for(int L_cho=0;L_cho<4;L_cho++){
#pragma HLS PIPELINE
		for(int L_chi=0;L_chi<4;L_chi++){
			for(int L_kr=0;L_kr<K;L_kr++){
				for(int L_kc=0;L_kc<K;L_kc++){
					W[L_cho][L_chi][L_kr][L_kc] = *W_ddr++;
				}
			}
		}
	}
	return;
}
void Convolution(float In[4][Rin][Cin],float W[4][4][K][K],float Out[4][R][C],bool exe){
	if(!exe)
		return;
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
	return;
}

void Offload_Out(float* Out_ddr, float Out[4][R][C]){
	for(int L_ro=0;L_ro<R;L_ro++){
	#pragma HLS PIPELINE
		for(int L_co=0;L_co<C;L_co++){
			for(int L_cho=0;L_cho<4;L_cho++){
				*Out_ddr++ = Out[L_cho][L_ro][L_co];
			}
		}
	}
	return;
}

void Process(float* In_ddr, float* W_ddr,
			float In_0[4][Rin][Cin],float W_0[4][4][K][K],
			float In_1[4][Rin][Cin],float W_1[4][4][K][K],
			float Out[4][R][C], int flag,
			bool exe_load, bool exe_conv){
	if(flag==0){
		Load_In(In_ddr,In_0,exe_load);
		Load_W(W_ddr,W_0,exe_load);
		Convolution(In_1,W_1,Out,exe_conv);
	}else{
		Load_In(In_ddr,In_1,exe_load);
		Load_W(W_ddr,W_1,exe_load);
		Convolution(In_0,W_0,Out,exe_conv);
	}
	return;
}

void test(float *In_ddr, float* W_ddr, float* Out_ddr){
#pragma HLS INTERFACE m_axi depth=32 port=In_ddr
#pragma HLS INTERFACE m_axi depth=32 port=W_ddr
#pragma HLS INTERFACE m_axi depth=32 port=Out_ddr
	static float In_0[4][Rin][Cin];
#pragma HLS array_partition variable=In_0 complete dim=1
	static float In_1[4][Rin][Cin];
#pragma HLS array_partition variable=In_1 complete dim=1
	static float Out[4][R][C];
#pragma HLS array_partition variable=Out complete dim=1
	static float W_0[4][4][K][K]
#pragma HLS array_partition variable=W_0 complete dim=1
#pragma HLS array_partition variable=W_0 complete dim=2
	static float W_1[4][4][K][K]
#pragma HLS array_partition variable=W_1 complete dim=1
#pragma HLS array_partition variable=W_1 complete dim=2
	int flag=0;

	Output_Channel_Tiling:
	for(int cho=0;cho<CHout;cho+=4){ //每次自增一块（4）
		Input_Channel_Tiling:
		for(int chi=0;chi<CHin;chi+=4){
			Process(In_ddr,W_ddr,In_0,W_0,In_1,W_1,Out,flag,chi<CHin/4,chi>0);
			flag=1-flag;
		}
		Offload_Out(Out_ddr,Out);
	}
	return;
}
