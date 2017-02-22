/*
 * ssd_sim.cc
 *
 *  Created on: Jul 14, 2016
 *      Author: zhang_000
 */
#include "ssd_sim.h"

RequestGenerator::RequestGenerator(uint32 _MAX_REQ, uint64 _start_PPN,
		uint32 _REQ_SIZE, uint32 _read_fraction, uint32 _read_denominator, uint32 _random_fraction, uint32 _random_denominator, uint32 _randSeed, int _IOGEN){
	cur_page = _MAX_REQ;
	MAX_REQ = _MAX_REQ;
	//MAX_REQ = 65536 / _REQ_SIZE; //assume the total request volume is 256MB 
	MAX_REQ = 262144 / _REQ_SIZE; //assume the request volume is 1GB 
	cur_REQ = 0;

	start_PPN = _start_PPN;
	REQ_SIZE = _REQ_SIZE;
	curRequest.PPN = 0;
	curRequest.REQ_SIZE = 0;
	curRequest.Oper = OP_READ;
	curRequest.IOGEN = 0;
	readratio.fraction = _read_fraction;
	readratio.denominator = _read_denominator;
	randomratio.fraction = _random_fraction;
	randomratio.denominator = _random_denominator;
	srand(_randSeed);
	IOGEN = _IOGEN;
}

RequestGenerator::~RequestGenerator(){}

//********** Wrong implementation
//bool RequestGenerator::generate_request(){
//	if (cur_REQ >= MAX_REQ) return false;
//	if (cur_page % randomratio.denominator < randomratio.fraction) //random access
//	{
//		curRequest.PPN = (uint64)((uint64)(1.0 * rand()/(RAND_MAX+1.0)*4*1*2*2*256*128)*(uint64)8192);
//	}
//	else{
//		start_PPN += 8192;
//		curRequest.PPN = start_PPN; //8192 is default page size
//	}
//	if (cur_page % readratio.denominator < readratio.fraction){
//		curRequest.Oper = OP_READ;
//	}
//	else{
//		curRequest.Oper = OP_WRITE;
//	}
//	curRequest.REQ_SIZE = 1;
//	curRequest.IOGEN = 0;
//	cur_page++;
//	if (cur_page == REQ_SIZE){
//		cur_page = 0; //reset
//		cur_REQ++;
//		curRequest.IOGEN = -1;
//	}
//
//	return true;
//}


bool RequestGenerator::generate_request(){
	if (cur_REQ >= MAX_REQ) return false;
	if (cur_REQ % randomratio.denominator < randomratio.fraction) //random access
	{
        int cmd = rand() % 100; //for testing
		//curRequest.PPN = (uint64)((uint64)(rand()% (MAX_REQ))* REQ_SIZE *(uint64)4096);
		//curRequest.PPN = (uint64)((uint64)(rand()% (MAX_REQ * REQ_SIZE))*(uint64)4096);
		curRequest.PPN = (uint64)((uint64)(rand()% (MAX_REQ * REQ_SIZE / 2))*(uint64)8192);
	}
	else{
		curRequest.PPN = start_PPN; //8192 is default page size
		start_PPN += 4096*REQ_SIZE;
	}
	if (cur_REQ % readratio.denominator < readratio.fraction){
		curRequest.Oper = OP_READ;
	}
	else{
		curRequest.Oper = OP_WRITE;
	}
	curRequest.REQ_SIZE = REQ_SIZE;
	curRequest.IOGEN = IOGEN;
	cur_REQ++;
	//if (cur_REQ % cur_page == 0) curRequest.IOGEN = -1;
	return true;
}
/*==============================
    Main
==============================*/
int main(int argc, char* argv[])
{
	//Initialization configuration
	unsigned SSD_enable = 1;
	string SSD_config;
	if (argc > 2){
		SSD_config = argv[1];
		cout<<"Config File:"<<SSD_config<<"\n";
		cout<<flush;
		HIL* m_hil = new HIL(0, SSD_enable, SSD_config);
		m_hil->setSSD(1);

		//*********initialize trace generator*****************
		curTick = 0;
		ConfigReader cr(argv[2]);
		uint32 _MAX_REQ = cr.ReadInt32("_MAX_REQ", 16);
		uint64 _start_PPN = cr.ReadInt32("_start_PPN", 0);
		uint32 _REQ_SIZE = cr.ReadInt32("_REQ_SIZE", 16);
		uint32 _read_fraction = cr.ReadInt32("_read_fraction", 0);
		uint32 _read_denominator = cr.ReadInt32("_read_denominator", 1);
		uint32 _random_fraction = cr.ReadInt32("_random_fraction", 0);
		uint32 _random_denominator = cr.ReadInt32("_random_denominator", 1);
		uint32 _randSeed = cr.ReadInt32("_randSeed", 131313);
		int _IOGEN = cr.ReadInt32("_IOGEN",0);
		int queue_depth = cr.ReadInt32("_QUEUE_DEPTH", 1); //assume default queue depth is 1
		std::list<Tick> request_queue;
		RequestGenerator* RG = new RequestGenerator(
				_MAX_REQ, _start_PPN, _REQ_SIZE, _read_fraction, _read_denominator, _random_fraction, _random_denominator, _randSeed, _IOGEN
		);
		sampled_period = EPOCH_INTERVAL;
		//*****************************************************
        for (unsigned i = 0; i < queue_depth; i++){
			if (!RG->generate_request()) break;
			m_hil->SSDoperation(RG->curRequest.PPN, RG->curRequest.REQ_SIZE, curTick, RG->curRequest.Oper);
			m_hil->updateFinishTick(RG->curRequest.PPN);
			m_hil->print_sample(sampled_period);
			request_queue.push_front(finishTick);
		}
		request_queue.sort();

		printf("From the head to the end:\t");
		for (std::list<Tick>::iterator e = request_queue.begin(); e != request_queue.end(); e++){
			printf("%llu\t", *e);
		}
		printf("\n");
		while(1){
			if (!RG->generate_request()) break;
				//sync operation
				curTick = request_queue.front();
				m_hil->SSDoperation(RG->curRequest.PPN, RG->curRequest.REQ_SIZE, curTick, RG->curRequest.Oper);
				Tick scheduledTick = m_hil->updateDelay(RG->curRequest.PPN) + curTick;
                m_hil->updateFinishTick(RG->curRequest.PPN);
				m_hil->print_sample(sampled_period);
				request_queue.pop_front();
				request_queue.push_front(scheduledTick);
				request_queue.sort();
//				if (RG->curRequest.IOGEN == -1){
//					curTick = finishTick;
//					//m_hil->print_sample(sampled_period);
//				}
//				else curTick += RG->curRequest.IOGEN;

		}
		//*****************************************************
//		while(1){
//			if (!RG->generate_request()) break;
//				printf("PPN: %llu\n", RG->curRequest.PPN);
//				//sync operation
//				m_hil->SSDoperation(RG->curRequest.PPN, RG->curRequest.REQ_SIZE, curTick, RG->curRequest.Oper);
//				m_hil->updateFinishTick(RG->curRequest.PPN);
//				if (RG->curRequest.IOGEN == -1) curTick = finishTick;
//				else curTick += RG->curRequest.IOGEN;
//				m_hil->print_sample(sampled_period);
//		}


		//sync operation
//		while(1){
//			if (!RG->generate_request()) break;
//			m_hil->SSDoperation(RG->curRequest.PPN, RG->curRequest.REQ_SIZE, curTick, RG->curRequest.Oper); //initial request
//			m_hil->updateFinishTick(RG->curRequest.PPN);
//			printf("finishTick = %llu\n", finishTick);
//			curTick = finishTick;
//		    m_hil->print_sample(sampled_period);
//		}

//		//async operation
//		while(addr < 4096 * 4096){
//			m_hil->AsyncOperation(addr, 1, curTick, OP_READ);
//			addr += 4096;
//			curTick += 1000000000;
//		}

		//print final results:
		struct output_result output;
		printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		m_hil->get_parameter(PAM_LAYER, output);
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		printf("PAM: read count = %lf\n",output.statistics[RD][CAPACITY][COUNT]);
		printf("PAM: write count = %lf\n",output.statistics[WR][CAPACITY][COUNT]);
		printf("PAM: device idle time = %lf\n",output.statistics[TOT][DEVICE_IDLE][TOT]);
		printf("PAM: device busy time = %lf\n",output.statistics[TOT][DEVICE_BUSY][TOT]);
		printf("PAM: average read bandwidth = %lf\n", output.statistics[RD][BANDWIDTH][AVG]);
		printf("PAM: average write bandwidth = %lf\n", output.statistics[WR][BANDWIDTH][AVG]);
		printf("PAM: average read IOPS = %lf\n", output.statistics[RD][IOPS][AVG] );
		printf("PAM: average write IOPS = %lf\n", output.statistics[WR][IOPS][AVG] );
		printf("PAM: average read latency (ms) = %lf\n", output.statistics[RD][LATENCY][AVG] / 1000 /1000 / 1000);
		printf("PAM: average write latency (ms) = %lf\n", output.statistics[WR][LATENCY][AVG] / 1000 /1000 / 1000);
		printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
		m_hil->get_parameter(FTL_HOST_LAYER, output);
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		printf("FTL_HOST: read count = %lf\n",output.statistics[RD][CAPACITY][COUNT]);
		printf("FTL_HOST: write count = %lf\n",output.statistics[WR][CAPACITY][COUNT]);
		printf("FTL_HOST: average read bandwidth = %lf\n", output.statistics[RD][BANDWIDTH][AVG]);
		printf("FTL_HOST: average write bandwidth = %lf\n", output.statistics[WR][BANDWIDTH][AVG]);
		printf("FTL_HOST: average read IOPS = %lf\n", output.statistics[RD][IOPS][AVG]);
		printf("FTL_HOST: average write IOPS = %lf\n", output.statistics[WR][IOPS][AVG]);
		printf("FTL_HOST: average read latency = %lf\n", output.statistics[RD][LATENCY][AVG]);
		printf("FTL_HOST: average write LATENCY = %lf\n", output.statistics[WR][LATENCY][AVG]);

		return 0;
	}
	else{
		printf("Usage:\n\t%s [SSD-ConfigFile] [Trace-generator]\n",argv[0]);
		return 0;
	}

}


