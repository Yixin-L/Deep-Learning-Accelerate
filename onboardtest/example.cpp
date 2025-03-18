typedef int data_type;

void test(data_type *a, data_type *b, data_type *c){
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE m_axi depth=256 port=a offset=slave
#pragma HLS INTERFACE m_axi depth=256 port=b offset=slave
#pragma HLS INTERFACE m_axi depth=256 port=c offset=slave

	data_type tmp_a[3];
	data_type tmp_b[3];
	data_type tmp_c[3];

	//DRAM read
	for(int i=0;i<3;i++){
		tmp_a[i]= *(a+i);
		tmp_b[i]= *(b+i);
	}

	//Add
	for(int i=0;i<3;i++){
		tmp_c[i]=tmp_a[i]+tmp_b[i];
	}

	//DRAM Write
	for(int i=0;i<3;i++){
		*(c+i)=tmp_c[i];
	}
}
