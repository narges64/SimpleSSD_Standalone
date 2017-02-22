//#include "arch/isa_traits.hh"
#include "mem/PAM2.h"
//#include "debug/PAM2.hh"

PAM2::PAM2(FTL* ftl, PAMStatistics* statistics)
{
    ftl_core = ftl;
    stats = statistics;

    uint32* OriginalSizes = gconf->OriginalSizes;
    RearrangedSizes[6] = OriginalSizes[6];
    for (int i=0; i<6; i++)
    {
        RearrangedSizes[i] = OriginalSizes[ gconf->AddrSeq[i] ];

        printf("PAM: [%d] ORI(%s): %u --> REARR(%s): %u\n", i, ADDR_STRINFO[ i ], OriginalSizes[ i ], ADDR_STRINFO[ gconf->AddrSeq[i] ], RearrangedSizes[i]); //Use DPRINTF here
        //cout<<"["<<i<<"] ORI("<<ADDR_STRINFO[ i ]<<"): "<<OriginalSizes[ i ]<<" --> REARR("<<ADDR_STRINFO[ gconf->AddrSeq[i] ]<<"): "<<RearrangedSizes[i]<<"\n";
    }

    ChTimeSlots = new TimeSlot*[gconf->NumChannel];
    std::memset( ChTimeSlots, 0, sizeof(TimeSlot*) * gconf->NumChannel );

    DieTimeSlots = new TimeSlot*[gconf->GetTotalNumDie()];
    std::memset( DieTimeSlots, 0, sizeof(TimeSlot*) * gconf->GetTotalNumDie() );

    MergedTimeSlots = new TimeSlot*[1];
    MergedTimeSlots[0] = NULL;

    ChFreeSlots = new std::map<uint64, std::map<uint64, uint64>* >[gconf->NumChannel];
    ChStartPoint = new uint64[gconf->NumChannel];
    for (unsigned i = 0; i < gconf->NumChannel; i++) ChStartPoint[i] = 0;

    DieFreeSlots = new std::map<uint64, std::map<uint64, uint64>* >[gconf->GetTotalNumDie()];
    DieStartPoint = new uint64[gconf->GetTotalNumDie()];
    for (unsigned i = 0; i < gconf->GetTotalNumDie(); i++) DieStartPoint[i] = 0;

    //Jie: currently, hard code pre-dma, mem-op and post-dma values
    for (unsigned i = 0; i < gconf->NumChannel; i++){
    	std::map<uint64, uint64> * tmp;
    	//tmp = new std::map<uint64, uint64>;
    	//ChFreeSlots[i][0] = tmp;
    	switch (gconf->NANDType) {
    	case NAND_SLC:
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][100000/lat->SPDIV + 100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][185000000/(lat->PGDIV*lat->SPDIV)] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][185000000/(lat->PGDIV*lat->SPDIV) + 100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][1500000/lat->SPDIV] = tmp;
    		break;
    	case NAND_MLC:
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][100000/lat->SPDIV + 100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][185000000/(lat->PGDIV*lat->SPDIV)] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][185000000/(lat->PGDIV*lat->SPDIV) + 100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][1500000/lat->SPDIV] = tmp;
    		break;
    	case NAND_TLC:
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][100000/lat->SPDIV + 100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][185000000/(lat->PGDIV*lat->SPDIV)] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][185000000/(lat->PGDIV*lat->SPDIV) + 100000/lat->SPDIV] = tmp;
    		tmp = new std::map<uint64, uint64>;
    		ChFreeSlots[i][1500000/lat->SPDIV] = tmp;
    		break;
    	default:
    		printf("unsupported NAND types!\n");
    		std::terminate();
    		break;
    	}
    }

    for (unsigned i = 0; i < gconf->GetTotalNumDie(); i++){
    	std::map<uint64, uint64> * tmp;
    	//std::map<uint64, uint64> * tmp = new std::map<uint64, uint64>;
    	//DieFreeSlots[i][0] = tmp;
    	switch (gconf->NANDType) {
    		case NAND_SLC:
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][25000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][300000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][2000000000 + 100000/lat->SPDIV] = tmp;
    		break;
    		case NAND_MLC:
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][40000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][90000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][500000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][1300000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][3500000000 + 100000/lat->SPDIV] = tmp;
    		break;
    		case NAND_TLC:
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][58000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][78000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][107000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][558000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][2201000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][5001000000 + 100000/lat->SPDIV] = tmp;
    			tmp = new std::map<uint64, uint64>;
    			DieFreeSlots[i][2274000000 + 100000/lat->SPDIV] = tmp;
    		break;
    		default:
    			printf("unsupported NAND types!\n");
    			std::terminate();
    		break;

    	}
    }

  #if GATHER_TIME_SERIES
    TimeSeriesLastTick = 0;
    SavedTimeSeries = 0;
  #endif
}

PAM2::~PAM2()
{
    FlushTimeSlots(MAX64);
    delete ChTimeSlots;
    delete DieTimeSlots;
    delete MergedTimeSlots;

    delete ChFreeSlots;
    delete DieFreeSlots;
}

void PAM2::FetchQueue()
{
    // for each request
    Command* reqHead = ftl_core->que->getFromQueue(0);
    Command* req = reqHead;

    //Flush Old TimeSlots  --> You can't verify the full-timeline with this.
    #if (FULL_VERIFY_TIMELINE==0)
    //Jie: change from FlushTimeSlots to FlushFreeSlots
    FlushFreeSlots(sim->GetTick());
    #endif

    #if GATHER_RESOURCE_CONFLICT && GATHER_TIME_SERIES
    uint64 ts_conflict_DMA0 = 0, ts_conflict_MEM = 0, ts_conflict_DMA1 = 0;
    #endif
    while (req)
    {
        if (req->initial || req->finished_time != 0) {
			req = req->next_command; continue;
		}	
        //req->status = REQSTAT_PROC;
        //Jie: ensure we can erase multiple blocks from single request
        unsigned erase_block = 1;
        if(req->operation == OP_ERASE){
        	for (unsigned i = 5; i>= 0; i--){
        		if ((gconf->AddrSeq)[i] != 5) {
					erase_block *= RearrangedSizes[i];
					//printf("erase block size = %u\n",erase_block);
					//cout.flush();
					}
        		else{
					//printf("i = %u\n",i);
					break;
				}
        	}
        }
		//for (unsigned i = 0; i<=5; i++) printf("AddrSeq[%u] = %u\n",i,(gconf->AddrSeq)[i]);
		//for (unsigned i = 0; i<=5; i++) printf("RearrangedSize[%u] = %u\n",i,RearrangedSizes[(gconf->AddrSeq)[i]]);
			//printf("From Jie: the erase block number is %u\n",erase_block);
        /*=========== CONFLICT data gather ============*/
        for (unsigned cur_command = 0; cur_command < erase_block; cur_command++){
        #if GATHER_RESOURCE_CONFLICT
        uint8 confType = CONFLICT_NONE;
        uint64 confLength = 0;
        #endif
        int superPage = 0;
        while (superPage < gconf->SuperpageDegree){
        	if ((req->superpage_mask).get(superPage) == false && req->operation != OP_ERASE){
        		superPage++;
        		continue;
        	}
        	req->page_address = req->page_address * gconf->SuperpageDegree + superPage;
            req->page_address = req->page_address - (req->page_address & (erase_block - 1)) + cur_command;
            CPDPBP reqCPD;
            PPNtoCPDPBP((uint64*) &(req->page_address), &reqCPD );
            uint32 reqCh = reqCPD.Channel;
            uint32 reqDieIdx = CPDPBPtoDieIdx( &reqCPD );
            TimeSlot *tsDMA0 = NULL, *tsMEM = NULL, *tsDMA1 = NULL;
            uint64 tickDMA0 = 0, tickMEM = 0, tickDMA1 = 0;
            uint64 latDMA0, latMEM, latDMA1, DMA0tickFrom, MEMtickFrom, DMA1tickFrom, totalLat;
            uint64 latANTI; //anticipate time slot
            bool conflicts; //check conflict when scheduling
            latDMA0 = lat->GetLatency(reqCPD.Page, req->operation, BUSY_DMA0);
            latMEM  = lat->GetLatency(reqCPD.Page, req->operation, BUSY_MEM );
            latDMA1 = lat->GetLatency(reqCPD.Page, req->operation, BUSY_DMA1);
            latANTI = lat->GetLatency(reqCPD.Page, OP_READ, BUSY_DMA0);
            //printf("new request is coming\n");
            //Start Finding available Slot
            DMA0tickFrom = sim->GetTick(); //get Current System Time
            while (1) //LOOP0
            {
                while (1) //LOOP1
                {
                    // 1a) LOOP1 - Find DMA0 available slot in ChTimeSlots
                    if (! FindFreeTime( ChFreeSlots[reqCh], latDMA0, DMA0tickFrom, tickDMA0, conflicts)){
                    	if (DMA0tickFrom < ChStartPoint[reqCh]){
                    		DMA0tickFrom = ChStartPoint[reqCh];
                    		conflicts = true;
                    	}
                    	else conflicts = false;
                    	tickDMA0 = ChStartPoint[reqCh];

                    }
                    else{
                    	if (conflicts) DMA0tickFrom = tickDMA0;
                    }
                    //Jie_inst:
                    //printf("DMA0: DMA0tickFrom %llu tickDMA0 %llu\n",DMA0tickFrom, tickDMA0);
                    /*=========== CONFLICT check - DMA0 ============*/
					#if GATHER_RESOURCE_CONFLICT
                    if (conflicts && !(confType & CONFLICT_MEM) )
                    {
                    	confType |= CONFLICT_DMA0;
                    }
					#endif


                    // 2b) LOOP1 - Find MEM avaiable slot in DieTimeSlots
                    MEMtickFrom = DMA0tickFrom;
                    if (! FindFreeTime( DieFreeSlots[reqDieIdx], (latDMA0 + latMEM), MEMtickFrom, tickMEM, conflicts)){
                    	if (MEMtickFrom < DieStartPoint[reqDieIdx]){
                    		MEMtickFrom = DieStartPoint[reqDieIdx];
                    		conflicts = true;
                    	}
                    	else{
                    		conflicts = false;
                    	}
                    	tickMEM = DieStartPoint[reqDieIdx];

                    }
                    else{
                    	if (conflicts) MEMtickFrom = tickMEM;
                    }
                    //Jie_inst:
                    //printf("MEM: MEMtickFrom %llu tickMEM %llu\n",MEMtickFrom, tickMEM);
                    if (tickMEM == tickDMA0) break;
                    DMA0tickFrom = MEMtickFrom;
                    uint64 tickDMA0_vrfy;
                    if (!FindFreeTime( ChFreeSlots[reqCh], latDMA0, DMA0tickFrom, tickDMA0_vrfy, conflicts)){
                    	tickDMA0_vrfy = ChStartPoint[reqCh];
                    }
                    if (tickDMA0_vrfy == tickDMA0) break;
                    /*=========== CONFLICT check - MEM ============*/
					#if GATHER_RESOURCE_CONFLICT
                    if ( conflicts && !(confType & CONFLICT_DMA0) )
                    {
                    	confType |= CONFLICT_MEM;
                    }
					#endif

                }

                // 3) Find DMA1 available slot
                //printf("DBG- find DMA1\n");
                DMA1tickFrom = DMA0tickFrom + (latDMA0 + latMEM);
                if (!FindFreeTime( ChFreeSlots[reqCh], latDMA1 + latANTI, DMA1tickFrom, tickDMA1, conflicts)){
                	if(DMA1tickFrom < ChStartPoint[reqCh]){
                		DMA1tickFrom = ChStartPoint[reqCh];
                		conflicts = true;
                	}
                	else conflicts = false;
                	tickDMA1 = ChStartPoint[reqCh];

                }
                else{
                	if (conflicts) DMA1tickFrom = tickDMA1;
                }
                //Jie_inst:
                //printf("DMA1: DMA1tickFrom %llu tickDMA1 %llu\n",DMA1tickFrom, tickDMA1);

                // 4) Re-verify MEM slot with including MEM(DMA0_start ~ DMA1_end)
                totalLat = (DMA1tickFrom+latDMA1 + latANTI) - DMA0tickFrom;
                uint64 tickMEM_vrfy;
                if (!FindFreeTime( DieFreeSlots[reqDieIdx], totalLat, DMA0tickFrom, tickMEM_vrfy, conflicts)){
                	tickMEM_vrfy = DieStartPoint[reqDieIdx];
                }
                if (tickMEM_vrfy == tickMEM) break;
                /*=========== CONFLICT check - DMA1 ============*/
                #if GATHER_RESOURCE_CONFLICT
                {
                    confType |= CONFLICT_DMA1;
                }
                #endif

                DMA0tickFrom = tickMEM_vrfy; // or re-search for next available resource!
                //Jie_inst:
                //printf("DMAverify: DMA0tickFrom %llu tickMEM_vrfy %llu\n",DMA0tickFrom, tickMEM_vrfy);
            }

            // 5) Assggn dma0, dma1, mem
            {
            	InsertFreeSlot(ChFreeSlots[reqCh], latDMA0, DMA0tickFrom, tickDMA0, ChStartPoint[reqCh], 0);

                //printf("...DMA0tickFrom #4 = %llu\n", DMA0tickFrom);
                #if HARD_VERIFY_TIMELINE
                if ( VerifyTimeLines(0) )
                {
                    TimeSlot* tsDBG = DieTimeSlots[reqDieIdx];
                    printf("TimeSlot - DIE%d : ", reqDieIdx);
                    while (tsDBG)
                    {

                        printf("%llu~%llu, ", tsDBG->StartTick, tsDBG->EndTick);
                        tsDBG = tsDBG->Next;
                    }
                    printf("\n");
                    std::terminate();
                }
                #endif

                if (!FindFreeTime( ChFreeSlots[reqCh], latDMA1 + latANTI, DMA1tickFrom, tickDMA1, conflicts)){
                	if(DMA1tickFrom < ChStartPoint[reqCh]){
                		DMA1tickFrom = ChStartPoint[reqCh];
                		conflicts = true;
                	}
                	else conflicts = false;
                	tickDMA1 = ChStartPoint[reqCh];

                }
                else{
                	if (conflicts) DMA1tickFrom = tickDMA1;
                }
                //Jie_inst:
                //printf("DMA1insert: DMA1tickFrom %llu tickDMA1 %llu\n",DMA1tickFrom, tickDMA1);
                if (DMA1tickFrom > tickDMA1) InsertFreeSlot(ChFreeSlots[reqCh], latDMA1, DMA1tickFrom + latANTI, tickDMA1, ChStartPoint[reqCh], 0);
                else InsertFreeSlot(ChFreeSlots[reqCh], latDMA1, tickDMA1 + latANTI, tickDMA1, ChStartPoint[reqCh], 0);

                #if HARD_VERIFY_TIMELINE
                    if ( VerifyTimeLines(0) ) std::terminate();
                #endif

                //temporarily use previous MergedTimeSlots design
                InsertFreeSlot(DieFreeSlots[reqDieIdx], totalLat, DMA0tickFrom, tickMEM, DieStartPoint[reqDieIdx], 0);
                if (tsDMA0 != NULL) delete tsDMA0;
                if (DMA0tickFrom < tickDMA0)tsDMA0 = new TimeSlot(tickDMA0, latDMA0);
                else tsDMA0 = new TimeSlot(DMA0tickFrom, latDMA0);
                if (tsDMA1 != NULL) delete tsDMA1;
                if (DMA1tickFrom < tickDMA1)tsDMA0 = new TimeSlot(tickDMA1 + latANTI, latDMA1);
                else tsDMA1 = new TimeSlot(DMA1tickFrom + latANTI, latDMA1);
                if (tsMEM != NULL) delete tsMEM;
                if (DMA0tickFrom < tickMEM)tsMEM = new TimeSlot(tickMEM, totalLat);
                else tsMEM = new TimeSlot(DMA0tickFrom, totalLat);

                //******************************************************************//
                if (DMA0tickFrom > tickDMA0) DMA0tickFrom = DMA0tickFrom + latDMA0;
                else DMA0tickFrom = tickDMA0 + latDMA0;
                uint64 tmpTick = DMA0tickFrom;
                if (! FindFreeTime( ChFreeSlots[reqCh], latANTI * 2, DMA0tickFrom, tickDMA0, conflicts)){
                	if (DMA0tickFrom < ChStartPoint[reqCh]){
                		DMA0tickFrom = ChStartPoint[reqCh];
                		conflicts = true;
                	}
                	else conflicts = false;
                	tickDMA0 = ChStartPoint[reqCh];

                }
                else{
                	if (conflicts) DMA0tickFrom = tickDMA0;
                }
                if (DMA0tickFrom == tmpTick)
                	InsertFreeSlot(ChFreeSlots[reqCh], latANTI * 2, DMA0tickFrom, tickDMA0, ChStartPoint[reqCh], 1);
                //******************************************************************//
              #if 1
                //Manage MergedTimeSlots
                if (MergedTimeSlots[0] == NULL)
                {
                    MergedTimeSlots[0] = new TimeSlot( tsMEM->StartTick, tsMEM->EndTick-tsMEM->StartTick+1 );
                }
                else
                {
                    TimeSlot* cur = MergedTimeSlots[0];
                    uint64 s = tsMEM->StartTick;
                    uint64 e = tsMEM->EndTick;
                    TimeSlot *spos = NULL, *epos = NULL;
                    int spnt = 0, epnt = 0; //inside(0), rightside(1)

                    //find s position
                    cur = MergedTimeSlots[0];
                    while (cur)
                    {
                        if( cur->StartTick<=s && s<=cur->EndTick )
                        {
                            spos = cur;
                            spnt = 0;//inside
                            break;
                        }

                        if ( (cur->Next == NULL) ||
                             (cur->Next && (s < cur->Next->StartTick) ) )
                        {
                            spos = cur;
                            spnt = 1;//rightside
                            break;
                        }

                        cur = cur->Next;
                    }

                    //find e position
                    cur = MergedTimeSlots[0];
                    while (cur)
                    {
                        if( cur->StartTick<=e && e<=cur->EndTick )
                        {
                            epos = cur;
                            epnt = 0;//inside
                            break;
                        }

                        if ( (cur->Next == NULL) ||
                             (cur->Next && (e < cur->Next->StartTick) ) )
                        {
                            epos = cur;
                            epnt = 1;//rightside
                            break;
                        }
                        cur = cur->Next;
                    }

                    //merge
                    if( !( (spos || epos) && (spos==epos && spnt==0 && epnt==0) ) ) //if both side is in a merged slot, skip
                    {
                        //printf("DBG - merge: %X(next=%X) %X(next=%X) %d %d\n",spos,spos->Next,epos,epos->Next,spnt,epnt);
                        if (spos)
                        {
                            if (spnt == 1) //rightside
                            {
                                TimeSlot* tmp = new TimeSlot(tsMEM->StartTick, tsMEM->EndTick-tsMEM->StartTick+1); //duration will be updated later
                                tmp->Next = spos->Next;
                                if (spos == epos)
                                {
                                    epos = tmp;
                                }
                                spos->Next = tmp; //overlapping temporary now;
                                spos = tmp; //update spos
                            }
                        }
                        else
                        {
                            if (!epos) //both new
                            {
                                TimeSlot* tmp = new TimeSlot( tsMEM->StartTick, tsMEM->EndTick-tsMEM->StartTick+1 ); //copy one
                                tmp->Next = MergedTimeSlots[0];
                                MergedTimeSlots[0] = tmp;
                            }
                            else if (epos)
                            {
                                TimeSlot* tmp = new TimeSlot( tsMEM->StartTick, 999 ); //duration will be updated later
                                tmp->Next = MergedTimeSlots[0];
                                MergedTimeSlots[0] = tmp;
                                spos = tmp;
                            }
                        }

                        //printf("DBG - merge2: %X(next=%X) %X(next=%X) %d %d\n",spos,spos->Next,epos,epos->Next,spnt,epnt);
                        if (epos)
                        {
                            {
                                if (epnt == 0)
                                {
                                    spos->EndTick = epos->EndTick;
                                }
                                else if(epnt == 1)
                                {
                                    spos->EndTick = tsMEM->EndTick;
                                }
                                //remove [ spos->Next ~ epos ]
                                cur = spos->Next;
                                spos->Next = epos->Next;
                                while (cur)
                                {
                                    TimeSlot* rem = cur;
                                    cur = cur->Next;
                                    delete rem;
                                    if (rem == epos) break;
                                }
                            }
                        }
                    }
                }

                #if 0 // DBG
                TimeSlot* cur = MergedTimeSlots[0];
                while (cur)
                {
                    printf("%llu ~ %llu(next=%X), ", cur->StartTick, cur->EndTick, cur->Next);
                    //if (cur->StartTick == 2274400 && cur->EndTick == 4548799) std::terminate();
                    cur = cur->Next;
                }
                printf("\n");
                #endif
              #endif //END MERGE TIMESLOT


                #if HARD_VERIFY_TIMELINE
                    if ( VerifyTimeLines(0) ) std::terminate();
                #endif

                #if 0
                    TimeSlot* tsDBG = ChTimeSlots[reqCh];
                    printf("TimeSlot - CH%d : ", reqCh);
                    while (tsDBG)
                    {

                        printf("%llu~%llu, ", tsDBG->StartTick, tsDBG->EndTick);
                        tsDBG = tsDBG->Next;
                    }
                    printf("\n");
                #endif
            }

            //print Log
            #if 1 //DBG_PRINT_BUSY
			if (1)
            {
			#if DBG_PRINT_REQUEST
            	printf("PAM: %s PPN 0x%lX ch%02d die%05d : REQTime  %lu\n", OPER_STRINFO[req->operation], req->page_address, reqCPD.Channel, reqDieIdx, req->arrived_time); //Use DPRINTF here
                printCPDPBP(&reqCPD);
                printf("PAM: %s PPN 0x%lX ch%02d die%05d : DMA0 %llu ~ %llu (%llu) , MEM  %llu ~ %llu (%llu) , DMA1 %llu ~ %llu (%llu)\n",
                       OPER_STRINFO[req->operation], req->page_address, reqCPD.Channel, reqDieIdx,
                       tsDMA0->StartTick, tsDMA0->EndTick, (tsDMA0->EndTick - tsDMA0->StartTick + 1),
                       tsMEM->StartTick,  tsMEM->EndTick,  (tsMEM->EndTick  - tsMEM->StartTick + 1)-(tsDMA0->EndTick - tsDMA0->StartTick + 1)-(tsDMA1->EndTick - tsDMA1->StartTick + 1),
                       tsDMA1->StartTick, tsDMA1->EndTick, (tsDMA1->EndTick - tsDMA1->StartTick + 1)
                      ); //Use DPRINTF here

                /*
                printf("%s PPN 0x%llX ch%02d die%05d : DMA0 %llu ~ %llu (%llu)\n", OPER_STRINFO[req->operation], req->page_address, reqCPD.Channel, reqDieIdx, tsDMA0->StartTick, tsDMA0->EndTick, (tsDMA0->EndTick - tsDMA0->StartTick + 1) );
                printf("%s PPN 0x%llX ch%02d die%05d : MEM  %llu ~ %llu (%llu)\n", OPER_STRINFO[req->operation], req->page_address, reqCPD.Channel, reqDieIdx, tsMEM->StartTick,  tsMEM->EndTick,  (tsMEM->EndTick  - tsMEM->StartTick + 1) );
                printf("%s PPN 0x%llX ch%02d die%05d : DMA1 %llu ~ %llu (%llu)\n", OPER_STRINFO[req->operation], req->page_address, reqCPD.Channel, reqDieIdx, tsDMA1->StartTick, tsDMA1->EndTick, (tsDMA1->EndTick - tsDMA1->StartTick + 1) );
                */
                printf("PAM: %s PPN 0x%lX ch%02d die%05d : REQ~DMA0start(%llu), DMA0~DMA1end(%llu)\n", OPER_STRINFO[req->operation], req->page_address, reqCPD.Channel, reqDieIdx, (tsDMA0->StartTick-1 - req->arrived_time + 1), (tsDMA1->EndTick - tsDMA0->StartTick + 1) ); //Use DPRINTF here
			#endif
                #if GATHER_RESOURCE_CONFLICT
                if (confType != CONFLICT_NONE)
                {
                    confLength = tsDMA0->StartTick - req->arrived_time;
                    /*printf("Conflict: %s %s %s delayed: %llu ns\n",
                           confType&CONFLICT_DMA0? CONFLICT_STRINFO[1]:"",
                           confType&CONFLICT_MEM?  CONFLICT_STRINFO[2]:"",
                           confType&CONFLICT_DMA1? CONFLICT_STRINFO[3]:"",
                           confLength);*/
                }
                #endif
            }
            #endif

            // 6) Write-back latency on RequestLL
			if (tsDMA1->EndTick > req->finished_time)
				req->finished_time = tsDMA1->EndTick;
            //req->status = REQSTAT_END;

            // categorize the time spent for read/write operation
            std::map<uint64, uint64>::iterator e;
            e = OpTimeStamp[req->operation].find(tsDMA0->StartTick);
            if (e != OpTimeStamp[req->operation].end()){
            	if (e->second < tsDMA1->EndTick) e->second = tsDMA1->EndTick;
            }
            else{
            	OpTimeStamp[req->operation][tsDMA0->StartTick] = tsDMA1->EndTick;
            }
            FlushOpTimeStamp();
//            if (req->operation == OP_ERASE || req->seq_num == -1){
//            	FlushOpTimeStamp();
//            }

            //printf("OpTimeStamp length %d\n",OpTimeStamp[req->operation].size());
            // Update stats
          #if 1
            stats->UpdateLastTick(tsDMA1->EndTick);
            #if GATHER_RESOURCE_CONFLICT
            stats->AddLatency( req, &reqCPD, reqDieIdx, tsDMA0, tsMEM, tsDMA1, confType, confLength );
            #else
            stats->AddLatency( req, &reqCPD, reqDieIdx, tsDMA0, tsMEM, tsDMA1 );
            #endif
          #endif

            #if GATHER_RESOURCE_CONFLICT && GATHER_TIME_SERIES
            if (confType & CONFLICT_DMA0) ts_conflict_DMA0++;
            if (confType & CONFLICT_MEM ) ts_conflict_MEM++;
            if (confType & CONFLICT_DMA1) ts_conflict_DMA1++;
            #endif
            if (req->operation == OP_ERASE || req->seq_num == -1)
            {
            	//MergeATimeSlot(ChTimeSlots[reqCh], tsDMA0);
            	//MergeATimeSlot(DieTimeSlots[reqDieIdx]);
            	stats->MergeSnapshot();
            }
            if (req->operation == OP_ERASE) break;
            superPage++;

        }

        }
            // *) Go Next Request
            req = req->next_command;


    }

    #if GATHER_RESOURCE_CONFLICT && GATHER_TIME_SERIES
    printf("TSCF, T,%llu, D0,%llu, M,%llu, D1,%llu\n", sim->GetTick(), ts_conflict_DMA0, ts_conflict_MEM, ts_conflict_DMA1);
    #endif
}

uint8 PAM2::VerifyTimeLines(uint8 print_on)
{
    uint8 ret = 0;
    if (print_on) printf("[ Verify Timelines ]\n");

    for (uint32 c = 0; c < gconf->NumChannel; c++)
    {
        TimeSlot *tsDBG = ChTimeSlots[c];
        TimeSlot *prev = tsDBG;
        uint64 ioCnt = 0;
        uint64 cntVrfy = 0;
        uint64 utilTime = 0;
        uint64 idleTime = 0;
        if (!tsDBG) {
            printf("WARN: no entry in CH%02d\n", c);
            continue;
        }
        ioCnt++;
        utilTime += (tsDBG->EndTick - tsDBG->StartTick + 1);
        tsDBG = tsDBG->Next;
        //printf("TimeSlot - CH%02d Vrfy : ", c);
        while (tsDBG)
        {
            #if 0
            printf("%llu~%llu, ", tsDBG->StartTick, tsDBG->EndTick);
            #endif

            #if 1
            if(!( (prev->EndTick    < tsDBG->StartTick) &&
                  (tsDBG->StartTick < tsDBG->EndTick) ))
            {
                if(print_on) printf( "CH%02d VERIFY FAILED: %llu~%llu, %llu~%llu\n", c, prev->StartTick, prev->EndTick, tsDBG->StartTick, tsDBG->EndTick );
                cntVrfy++;
            }
            ioCnt++;
            utilTime += (tsDBG->EndTick - tsDBG->StartTick + 1);
            idleTime += (tsDBG->StartTick - prev->EndTick - 1);
            //printf("if %llu ~ %llu => %llu\n", tsDBG->StartTick, tickVrfy , (tsDBG->StartTick - tickVrfy - 1) );
            //printf("uf %llu\n", (tsDBG->EndTick - tsDBG->StartTick + 1) );
            #endif

            prev = tsDBG;
            tsDBG = tsDBG->Next;
        }
        if (cntVrfy)
        {
            if(print_on) 
            {
                printf("TimeSlot - CH%02d Vrfy : FAIL %llu\n", c, cntVrfy);

                TimeSlot* tsDBG = ChTimeSlots[c];
                printf("TimeSlot - CH%d : ", c);
                while (tsDBG)
                {
                    
                    printf("%llu~%llu, ", tsDBG->StartTick, tsDBG->EndTick);
                    tsDBG = tsDBG->Next;
                }
                printf("\n");
            }
            ret|=1;
        }
        if(print_on) printf("TimeSlot - CH%02d UtilTime : %llu , IdleTime : %llu , Count: %llu\n", c, utilTime, idleTime, ioCnt);
    }

    for (uint32 d=0;d<gconf->GetTotalNumDie();d++)
    {
        TimeSlot *tsDBG = DieTimeSlots[d];
        TimeSlot *prev = tsDBG;
        uint64 ioCnt = 0;
        uint64 cntVrfy = 0;
        uint64 idleTime = 0;
        uint64 utilTime = 0;
        if (!tsDBG){
            printf("WARN: no entry in DIE%05d\n", d);
            continue;
        }
        ioCnt++;
        utilTime += (tsDBG->EndTick - tsDBG->StartTick + 1);
        tsDBG = tsDBG->Next;
        //printf("TimeSlot - DIE%02d Vrfy : ", d);
        while (tsDBG)
        {
            #if 0
            printf("TimeSlot - DIE%05d : %llu~%llu, ", d, tsDBG->StartTick, tsDBG->EndTick);
            #else
            if(!( (prev->EndTick    < tsDBG->StartTick) &&
                  (tsDBG->StartTick < tsDBG->EndTick) ))
            {
                if(print_on) printf( "DIE%05d VERIFY FAILED: %llu~%llu, %llu~%llu\n", d, prev->StartTick, prev->EndTick, tsDBG->StartTick, tsDBG->EndTick );
                cntVrfy++;
            }
            ioCnt++;
            utilTime += (tsDBG->EndTick - tsDBG->StartTick + 1);
            idleTime += (tsDBG->StartTick - prev->EndTick - 1);
            #endif

            prev = tsDBG;
            tsDBG = tsDBG->Next;
        }
        if (cntVrfy)
        {
            if(print_on)
            {
                printf("TimeSlot - DIE%05d Vrfy : FAIL %llu\n", d, cntVrfy);

                TimeSlot* tsDBG = DieTimeSlots[d];
                printf("TimeSlot - DIE%d : ", d);
                while (tsDBG)
                {
                    
                    printf("%llu~%llu, ", tsDBG->StartTick, tsDBG->EndTick);
                    tsDBG = tsDBG->Next;
                }
                printf("\n");
            }
            ret|=2;
        }
        if(print_on) printf("TimeSlot - DIE%05d UtilTime : %llu , IdleTime : %llu , Count: %llu\n", d, utilTime, idleTime, ioCnt);
    }

    return ret;
}


TimeSlot* PAM2::InsertAfter(TimeSlot* tgtTimeSlot, uint64 tickLen, uint64 startTick)
{
    TimeSlot* curTS;
    TimeSlot* newTS;

    curTS = tgtTimeSlot;

    /*if (curTS->Next == NULL && curTS->EndTick>=startTick )
    {
        startTick = curTS->EndTick+1;
    }*/

    newTS = new TimeSlot(startTick, tickLen);
    newTS->Next = curTS->Next;

    curTS->Next = newTS;

    return newTS;
}

TimeSlot* PAM2::FlushATimeSlot(TimeSlot* tgtTimeSlot, uint64 currentTick)
{
    //uint32 cnt = 0;
    TimeSlot* cur = tgtTimeSlot;
    while (cur)
    {
        if (cur->EndTick < currentTick)
        {
            //cnt++;
            TimeSlot* rem = cur;
            cur = cur->Next;
            delete rem;
            continue;
        }
        break;
    }
    /*
    TimeSlot* test = cur;
    int counter = 0;
    while (test)
    {
        counter++;
        test = test->Next;
    }
    printf("TimeSlot length: %d\n",counter);
    */

//    if (counter > 0){
//    	test = cur;
//    	printf("************detailed statistics**********\n");
//    	while (test){
//    		printf("%llu\t%llu\n", test->StartTick, test->EndTick);
//    		test = test -> Next;
//    	}
//    }

    return cur;
}

void PAM2::MergeATimeSlot(TimeSlot* tgtTimeSlot)
{
	TimeSlot* cur = tgtTimeSlot;
	while (cur)
	{
		if (cur->Next)
		{
			TimeSlot* rem = cur->Next;
			cur->EndTick = rem->EndTick;
			cur->Next = rem->Next;
			delete rem;
		}
		else{
			cur = cur->Next;
		}
	}
}

void PAM2::MergeATimeSlot(TimeSlot* startTimeSlot, TimeSlot* endTimeSlot){
	TimeSlot* cur = startTimeSlot;
	while (cur)
	{
		if (cur->Next && cur->Next == endTimeSlot){
			TimeSlot* rem = cur->Next;
			cur->EndTick = rem->EndTick;
			cur->Next = rem->Next;
			delete rem;
			break;
		}
		else if (cur->Next)
		{
			TimeSlot* rem = cur->Next;
			cur->EndTick = rem->EndTick;
			cur->Next = rem->Next;
			delete rem;
		}
		else{
			cur = cur->Next;
		}
	}
//    TimeSlot* test = cur;
//    int counter = 0;
//    while (test)
//    {
//        counter++;
//        test = test->Next;
//    }
//    printf("TimeSlot length: %d\n",counter);
}

void PAM2::MergeATimeSlotCH(TimeSlot* tgtTimeSlot)
{
	TimeSlot* cur = tgtTimeSlot;
	while (cur)
	{
		while (cur->Next && (cur->Next)->StartTick - cur->EndTick == 1)
		{
			TimeSlot* rem = cur->Next;
			cur->EndTick = rem->EndTick;
			cur->Next = rem->Next;
			delete rem;
		}
		cur = cur->Next;
	}
    TimeSlot* test = tgtTimeSlot;
    int counter = 0;
    while (test)
    {
        counter++;
        test = test->Next;
    }

    printf("TimeAfterSlot length: %d\n",counter);

}

void PAM2::MergeATimeSlotDIE(TimeSlot* tgtTimeSlot)
{
	TimeSlot* cur = tgtTimeSlot;
	while (cur)
	{
		while (cur->Next && (cur->Next)->StartTick - cur->EndTick == 1)
		{
			TimeSlot* rem = cur->Next;
			cur->EndTick = rem->EndTick;
			cur->Next = rem->Next;
			delete rem;
		}
		cur = cur->Next;
	}
    TimeSlot* test = tgtTimeSlot;
    int counter = 0;
    while (test)
    {
        counter++;
        test = test->Next;
    }
    printf("TimeAfterSlot length: %d\n",counter);

}
TimeSlot* PAM2::FlushATimeSlotBusyTime(TimeSlot* tgtTimeSlot, uint64 currentTick, uint64* TimeSum)
{
    //uint32 cnt = 0;
    TimeSlot* cur = tgtTimeSlot;
    while (cur)
    {
        if (cur->EndTick < currentTick)
        {
            //cnt++;
            TimeSlot* rem = cur;
            cur = cur->Next;
            *TimeSum += ( rem->EndTick - rem->StartTick + 1 );
            delete rem;
            continue;
        }
        break;
    }
    //printf("%llu: dropped %u TimeSlots\n", currentTick, cnt);
    return cur;
}

void PAM2::FlushOpTimeStamp() //currently only used during garbage collection
{
	//flush OpTimeStamp
	std::map<uint64, uint64>::iterator e;
	for (unsigned Oper = 0; Oper < 3; Oper++){
		uint64 start_tick = (uint64)-1, end_tick = (uint64)-1;
		for (e = OpTimeStamp[Oper].begin(); e != OpTimeStamp[Oper].end(); ){
			if (start_tick == (uint64)-1 && end_tick == (uint64)-1){
				start_tick = e->first; end_tick = e->second;
			}
			else if (e->first < end_tick && e->second <= end_tick){
			}
			else if (e->first < end_tick && e->second > end_tick){
				end_tick = e->second;

			}
			else if (e->first >= end_tick){
				stats->OpBusyTime[Oper] += end_tick - start_tick + 1;
				start_tick = e->first; end_tick = e->second;
			}
			OpTimeStamp[Oper].erase(e);
			e = OpTimeStamp[Oper].begin();
		}
		stats->OpBusyTime[Oper] += end_tick - start_tick + 1;
	}
}
void PAM2::InquireBusyTime(uint64 currentTick)
{
	stats->SampledExactBusyTime = stats->ExactBusyTime;
	//flush OpTimeStamp
	std::map<uint64, uint64>::iterator e;
	for (unsigned Oper = 0; Oper < 3; Oper++){
		uint64 start_tick = (uint64)-1, end_tick = (uint64)-1;
		for (e = OpTimeStamp[Oper].begin(); e != OpTimeStamp[Oper].end(); ){
			if (e->second > currentTick) break;
			if (start_tick == (uint64)-1 && end_tick == (uint64)-1){
				start_tick = e->first; end_tick = e->second;
			}
			else if (e->first < end_tick && e->second <= end_tick){
			}
			else if (e->first < end_tick && e->second > end_tick){
				end_tick = e->second;

			}
			else if (e->first >= end_tick){
				stats->OpBusyTime[Oper] += end_tick - start_tick + 1;
				start_tick = e->first; end_tick = e->second;
			}
			OpTimeStamp[Oper].erase(e);
			e = OpTimeStamp[Oper].begin();
		}
		stats->OpBusyTime[Oper] += end_tick - start_tick + 1;
	}
	//printf("ExactBusyTime=%llu\n",stats->ExactBusyTime);
	TimeSlot* cur = MergedTimeSlots[0];

    while (cur)
    {
        if (cur->EndTick < currentTick)
        {
            //cnt++;
            TimeSlot* rem = cur;
            cur = cur->Next;
            stats->SampledExactBusyTime += ( rem->EndTick - rem->StartTick + 1 );
            continue;
        }
        else if (cur->EndTick >= currentTick && cur->StartTick < currentTick)
        {
        	TimeSlot* rem = cur;
        	stats->SampledExactBusyTime += ( currentTick - rem->StartTick + 1 );
        	break;
        }
        break;
    }
    //printf("SampledExactBusyTime=%llu\n",stats->SampledExactBusyTime);
}

void PAM2::FlushTimeSlots(uint64 currentTick)
{
    #if GATHER_TIME_SERIES
    //VerifyTimeLines(1);
    GetTimeSeries(currentTick);
    #endif

    for(uint32 i=0;i<gconf->NumChannel;i++)
    {
        ChTimeSlots[i] = FlushATimeSlot(ChTimeSlots[i], currentTick);
        //MergeATimeSlotCH(ChTimeSlots[i]);
    }

    for(uint32 i=0;i<gconf->GetTotalNumDie();i++)
    {
        DieTimeSlots[i] = FlushATimeSlot(DieTimeSlots[i], currentTick);
        //MergeATimeSlotDIE(DieTimeSlots[i]);
    }

    //Jie: add this for test
    //DPRINTF(PAM2, "Before Update, ExactBusyTime is %llu\n",stats->ExactBusyTime);

    MergedTimeSlots[0] = FlushATimeSlotBusyTime( MergedTimeSlots[0], currentTick, &(stats->ExactBusyTime) );

    //Jie: add this for test
    //DPRINTF(PAM2, "After Update, ExactBusyTime is %llu\n",stats->ExactBusyTime);

    stats->Access_Capacity.update();
    //Jie: 7.2
    stats->Ticks_Total.update();

}

void PAM2::FlushFreeSlots(uint64 currentTick){
    for(uint32 i=0;i<gconf->NumChannel;i++)
    {
        FlushAFreeSlot(ChFreeSlots[i], currentTick);
    }
    for(uint32 i=0;i<gconf->GetTotalNumDie();i++)
    {
        FlushAFreeSlot(DieFreeSlots[i], currentTick);
    }
    MergedTimeSlots[0] = FlushATimeSlotBusyTime( MergedTimeSlots[0], currentTick, &(stats->ExactBusyTime) );
    stats->Access_Capacity.update();
    stats->Ticks_Total.update();
}

void PAM2::FlushAFreeSlot(std::map<uint64, std::map<uint64, uint64>* >& tgtFreeSlot, uint64 currentTick){
	//Jie_inst
	//int count = 0;
	for (std::map<uint64, std::map<uint64, uint64>* >::iterator e = tgtFreeSlot.begin(); e != tgtFreeSlot.end(); e++){
		std::map<uint64, uint64>::iterator f = (e->second)->begin();
		std::map<uint64, uint64>::iterator g = (e->second)->end();

		for (; f != g;){
			//count++;
			if (f->second < currentTick)(e->second)->erase(f++);
			else break;
		}
	}
	//printf("Flush FreeSlots %d\n", count);
}

#if GATHER_TIME_SERIES
//should be called just before Flush.
void PAM2::GetTimeSeries(uint64 currentTick)
{
    uint64 MinTick = MAX64, nextMinTick = MAX64;
    uint64 numActiveCh, numActiveDie;

    //Find First MinTick
    {
        for(uint32 i=0;i<gconf->NumChannel;i++)
        {
            if ( ChTimeSlots[i] && ChTimeSlots[i]->StartTick < MinTick)
            {
                MinTick = ChTimeSlots[i]->StartTick;
            }
        }

        for(uint32 i=0;i<gconf->GetTotalNumDie();i++)
        {
            if ( DieTimeSlots[i] && DieTimeSlots[i]->StartTick < MinTick)
            {
                MinTick = DieTimeSlots[i]->StartTick;
            }
        }
    }

    //prevent go back to old time point which already have flushed.
    if (MinTick < TimeSeriesLastTick)
    {
        MinTick = TimeSeriesLastTick+1;
    }

    uint8 runPlusOne = 0;
    while (1)
    {
        //Check Saved last time tick is same as current & print if different
        if (SavedTimeSeries)
        {
            SavedTimeSeries = 0;
            if (SavedMinTick < MinTick)
            {
                printf("TS, T,%llu, C,%llu, D,%llu\n", SavedMinTick, SavedNumActiveCh, SavedNumActiveDie);
            }
        }

        //Count Active Resource & Print
        if (MinTick < currentTick && MinTick != MAX64)
        {
            CountActive(MinTick, &numActiveCh, &numActiveDie);
            printf("TS, T,%llu, C,%llu, D,%llu\n", MinTick, numActiveCh, numActiveDie);
            TimeSeriesLastTick = MinTick;
        }
        else 
        {
            if (currentTick == MAX64)
                printf("TS, T,%llu, C,0, D,0, END\n", TimeSeriesLastTick+1);
            break;
        }

        //Check of End+1 time
        if (runPlusOne)
        {
            uint64 tmpNextMinTick = MAX64;
            uint8 tmpRunPlusOne = 0;
            GetNextMinTick(MinTick+1, &tmpNextMinTick, &tmpRunPlusOne); // if nextMinTick is last one, don't print, save for later!

            runPlusOne = 0;
            if (tmpNextMinTick != MAX64)
            {
                MinTick++;
                continue;
            }
            else //save for later
            {
                SavedTimeSeries = 1;
                SavedMinTick = MinTick+1;
                CountActive(MinTick+1, &SavedNumActiveCh, & SavedNumActiveDie);
            }
        }

        //Find next MinTick
        {
            GetNextMinTick(MinTick, &nextMinTick, &runPlusOne);
            MinTick = nextMinTick;
        }
    }
}

void PAM2::GetNextMinTick(uint64 currentMinTick, uint64* p_nextMinTick, uint8* p_runOneMore)
{
    uint64 nextMinTick = MAX64;
    uint8 runPlusOne = 0;
    for(uint32 i=0;i<gconf->NumChannel;i++)
    {
        TimeSlot* curSlot = ChTimeSlots[i];
        while ( curSlot )
        {
            uint64 tick1 = curSlot->StartTick;
            uint64 tick2 = curSlot->EndTick;
            if ( currentMinTick<tick1 && (tick1<nextMinTick) )
            {
                nextMinTick = tick1;
            }

            if ( currentMinTick<tick2 && (tick2<nextMinTick) )
            {
                nextMinTick = tick2;
                runPlusOne = 1;
            }

            curSlot = curSlot->Next;
        }
    }

    for(uint32 i=0;i<gconf->GetTotalNumDie();i++)
    {
        TimeSlot* curSlot = DieTimeSlots[i];
        while ( curSlot )
        {
            uint64 tick1 = curSlot->StartTick;
            uint64 tick2 = curSlot->EndTick;
            if ( currentMinTick<tick1 && (tick1<nextMinTick) )
            {
                nextMinTick = tick1;
            }

            if ( currentMinTick<tick2 && (tick2<nextMinTick) )
            {
                nextMinTick = tick2;
                runPlusOne = 1;
            }
            curSlot = curSlot->Next;
        }
    }

    *p_nextMinTick = nextMinTick;
    *p_runOneMore = runPlusOne;
}

void PAM2::CountActive(uint64 currentTick, uint64* activeCh, uint64* activeDie)
{
    *activeCh = 0;
    *activeDie = 0;

    for(uint32 i=0;i<gconf->NumChannel;i++)
    {
        TimeSlot* cur = ChTimeSlots[i];
        while (cur)
        {
            if (cur->StartTick <= currentTick && currentTick <= cur->EndTick)
            {
                (*activeCh)++;
                break;
            }
            else if ( currentTick < cur->StartTick )
            {
                break;
            }

            cur = cur->Next;
        }
    }

    for(uint32 i=0;i<gconf->GetTotalNumDie();i++)
    {
        TimeSlot* cur = DieTimeSlots[i];
        while (cur)
        {
            if(cur->StartTick <= currentTick && currentTick <= cur->EndTick)
            {
                (*activeDie)++;
                break;
            }
            else if ( currentTick < cur->StartTick )
            {
                break;
            }

            cur = cur->Next;
        }
    }

}
#endif


TimeSlot* PAM2::FindFreeTime(TimeSlot* tgtTimeSlot, uint64 tickLen, uint64 fromTick)
{
    TimeSlot* cur = tgtTimeSlot;
    TimeSlot* next = NULL;
    while(cur)
    {
        //printf("FindFreeTime.FIND : cur->ST=%llu, cur->ET=%llu, next=0x%X\n", cur->StartTick, cur->EndTick, (unsigned int)next); //DBG
        if(cur->Next)
            next = cur->Next;
        else
            break;

        if (cur->EndTick < fromTick && fromTick < next->StartTick)
        {
            if( ( next->StartTick - fromTick ) >= tickLen )
            {
                //printf("FindFreeTime.RET A: cur->ET=%llu, next->ST=%llu, ft=%llu, tickLen=%llu\n", cur->EndTick, next->StartTick, fromTick, tickLen); //DBG
                break;
            }
        }
        else if( fromTick <= cur->EndTick )
        {
            if( (next->StartTick - (cur->EndTick + 1) ) >= tickLen )
            {
                //printf("FindFreeTime.RET A: cur->ET=%llu, next->ST=%llu, ft=%llu, tickLen=%llu\n", cur->EndTick, next->StartTick, fromTick, tickLen); //DBG
                break;
            }
        }
        cur = cur->Next;
    }

    //                   v-- this condition looks strange? but [minus of uint64 values] do not work correctly without this!
    if ( tgtTimeSlot && (tgtTimeSlot->StartTick > fromTick) && ( (tgtTimeSlot->StartTick - fromTick) >= tickLen ) )
    {
        //printf("FindFreeTime.RET C: tgtTimeSlot->ST=%llu, ft=%llu, tickLen=%llu\n", tgtTimeSlot->StartTick, fromTick, tickLen); //DBG
        cur = NULL; // if there is an available time slot before first cmd, return NULL!
    }
    //else { printf("FindFreeTime.RET D: cur=0x%X\n", (unsigned int)cur); } //DBG

    return cur;
}

bool PAM2::FindFreeTime(std::map<uint64, std::map<uint64, uint64>* > & tgtFreeSlot, uint64 tickLen, uint64 & tickFrom, uint64 & startTick,  bool & conflicts){
	//for (std::map<uint64, std::map<uint64, uint64>* >::iterator m = tgtFreeSlot.begin(); m != tgtFreeSlot.end(); m++)
	//{
	//	printf("BOUND %llu LENGTH %d\n", m->first, (m->second)->size());
	//}
	std::map<uint64, std::map<uint64, uint64>* >::iterator e = tgtFreeSlot.upper_bound(tickLen);
	if (e == tgtFreeSlot.end()){
		e--; //Jie: tgtFreeSlot is not empty
		std::map<uint64, uint64>::iterator f = (e->second)->upper_bound(tickFrom);
		if (f != (e->second)->begin()){
			f--;
			if (f->second >= tickLen + tickFrom - (uint64)1){
				startTick = f->first;
				conflicts = false;
				return true;
			}
			f++;
		}
		while(f != (e->second)->end()){
			if (f->second >= tickLen + f->first - 1){
				startTick = f->first;
				conflicts = true;
				return true;
			}
			f++;
		}
		//Jie: reach this means no FreeSlot satisfy the requirement; allocate unused slots
		//Jie: startTick will be updated in upper function
		conflicts = false;
		return false;
	}
	if (e != tgtFreeSlot.begin()) e--;
	uint64 minTick = (uint64)-1;
	while (e != tgtFreeSlot.end()){

		std::map<uint64, uint64>::iterator f = (e->second)->upper_bound(tickFrom);
		if (f != (e->second)->begin()){
			f--;
			if (f->second >= tickLen + tickFrom - (uint64)1){ //this free slot is best fit one, skip checking others
				startTick = f->first;
				conflicts = false;
				return true;
			}
			f++;
		}
			while (f != (e->second)->end()){
				if (f->second >= tickLen + f->first - (uint64)1){
					if (minTick == (uint64)-1 || minTick > f->first){
						conflicts = true;
						minTick = f->first;
					}
					break;
				}
				f++;
			}
		e++;
	}
	if (minTick == (uint64)-1){
		//Jie: startTick will be updated in upper function
		conflicts = false;
		return false;
	}
	else{
		startTick = minTick;
		return true;
	}

}

void PAM2::InsertFreeSlot(std::map<uint64, std::map<uint64, uint64>* >& tgtFreeSlot, uint64 tickLen, uint64 tickFrom, uint64 startTick, uint64 & startPoint, bool split){
	//printf("PreStartPoint %llu\n",startPoint);
	if (startTick == startPoint){
		if (tickFrom == startTick){
			if (split) AddFreeSlot(tgtFreeSlot, tickLen, startPoint);
			startPoint = startPoint + tickLen; //Jie: just need to shift startPoint
			//printf("erase nothing!\n");
			//printf("add nothing!\n");
		}
		else{
			assert(tickFrom > startTick);
			if (split) AddFreeSlot(tgtFreeSlot, tickLen, tickFrom);
			startPoint = tickFrom + tickLen;
			AddFreeSlot(tgtFreeSlot, tickFrom - startTick, startTick);
			//printf("erase nothing!\n");
			//printf("add FreeSlot %llu ~ %llu\n", startTick, tickFrom);
		}
	}
	else {
		std::pair<std::map<uint64, std::map<uint64, uint64>* >::iterator, std::map<uint64, std::map<uint64, uint64>* >::iterator> ePair;
		ePair = tgtFreeSlot.equal_range(tickLen);
		std::map<uint64, uint64>::iterator f;
		std::map<uint64, std::map<uint64, uint64>* >::iterator e;
		if (ePair.first != ePair.second) e = ePair.first;
		else{
			assert(ePair.first != tgtFreeSlot.begin());
			e = ePair.first--;
		}
		while (e != tgtFreeSlot.end()){
			f = e->second->find(startTick);
			if (f != e->second->end()){
				uint64 tmpStartTick = f->first;
				uint64 tmpEndTick = f->second;
				e->second->erase(f);
				//printf("erase FreeSlot %llu ~ %llu\n", tmpStartTick, tmpEndTick);
				if (tmpStartTick < tickFrom){
					AddFreeSlot(tgtFreeSlot, tickFrom - tmpStartTick, tmpStartTick);
					if (split) AddFreeSlot(tgtFreeSlot, tickLen, tickFrom);
					//printf("add FreeSlot %llu ~ %llu\n", tmpStartTick, tickFrom-1);
					assert(tmpEndTick - tickFrom + 1 >= tickLen);
					if (tmpEndTick > tickLen + tickFrom - (uint64)1){

						AddFreeSlot(tgtFreeSlot, tmpEndTick - (tickFrom + tickLen -1), tickFrom + tickLen);
						//printf("add FreeSlot %llu ~ %llu\n", tickFrom + tickLen, tmpEndTick);
					}
				}
				else{
					assert(tmpStartTick == tickFrom);
					assert(tmpEndTick - tickFrom + 1 >= tickLen);
					if (split) AddFreeSlot(tgtFreeSlot, tickLen, tmpStartTick);
					if (tmpEndTick > tickLen + tickFrom - (uint64)1){

						AddFreeSlot(tgtFreeSlot, tmpEndTick - (tickFrom + tickLen -1), tmpStartTick + tickLen);
						//printf("add FreeSlot %llu ~ %llu\n", tickFrom + tickLen, tmpEndTick);
					}
				}
				break;
			}
			e++;

		}
	}
	//printf("PostStartPoint %llu\n",startPoint);
}

void PAM2::AddFreeSlot(std::map<uint64, std::map<uint64, uint64>* >& tgtFreeSlot, uint64 tickLen, uint64 tickFrom){
	std::map<uint64, std::map<uint64, uint64>* >::iterator e;
	e = tgtFreeSlot.upper_bound(tickLen);
	if (e != tgtFreeSlot.begin()){
		e--;
		(e->second)->insert(std::pair<uint64,uint64>(tickFrom, tickFrom + tickLen - (uint64)1));
		//Jie_inst
		//printf("AddFreeSlot: %llu ~ %llu\n",tickFrom, tickFrom + tickLen -1);
		//printf("Bound %d length %d\n", e->first, (e->second)->size());
	}
}
//PPN number conversion
uint32 PAM2::CPDPBPtoDieIdx(CPDPBP* pCPDPBP)
{
    //[Channel][Package][Die];
    uint32 ret = 0;
    ret += pCPDPBP->Die;
    ret += pCPDPBP->Package * (gconf->NumDie);
    ret += pCPDPBP->Channel * (gconf->NumDie * gconf->NumPackage);

    return ret;
}

void PAM2::printCPDPBP(CPDPBP* pCPDPBP)
{
    uint32* pCPDPBP_IDX = ((uint32*)pCPDPBP);

    printf("PAM:    %7s | %7s | %7s | %7s | %7s | %7s\n", ADDR_STRINFO[ gconf->AddrSeq[0] ], ADDR_STRINFO[ gconf->AddrSeq[1] ], ADDR_STRINFO[ gconf->AddrSeq[2] ], ADDR_STRINFO[ gconf->AddrSeq[3] ], ADDR_STRINFO[ gconf->AddrSeq[4] ], ADDR_STRINFO[ gconf->AddrSeq[5] ]); //Use DPRINTF here
    /*for (int i = 0;i<6;i++) {
        if (i) printf(" | "); //Use DPRINTF here
        DPRINTF(PAM2,"%7s", ADDR_STRINFO[ gconf->AddrSeq[i] ]); //Use DPRINTF here
    }
    DPRINTF(PAM2,"\n"); //Use DPRINTF here*/


    printf("PAM:    %7u | %7u | %7u | %7u | %7u | %7u\n", pCPDPBP_IDX[ gconf->AddrSeq[0] ], pCPDPBP_IDX[ gconf->AddrSeq[1] ], pCPDPBP_IDX[ gconf->AddrSeq[2] ], pCPDPBP_IDX[ gconf->AddrSeq[3] ], pCPDPBP_IDX[ gconf->AddrSeq[4] ], pCPDPBP_IDX[ gconf->AddrSeq[5] ]); //Use DPRINTF here
    /*for (int i = 0;i<6;i++) {
        if (i) printf(" | "); //Use DPRINTF here
        DPRINTF(PAM2,"%7u", pCPDPBP_IDX[ gconf->AddrSeq[i] ]); //Use DPRINTF here
    }
    DPRINTF(PAM2,"\n"); //Use DPRINTF here*/
}

void PAM2::PPNtoCPDPBP(uint64* pPPN, CPDPBP* pCPDPBP)
{
    uint32* pCPDPBP_IDX = ((uint32*)pCPDPBP);
    uint64 tmp_MOD = *pPPN;
    uint8*  AS = gconf->AddrSeq;
    uint32* RS = RearrangedSizes;
    for (uint32 i = 0; i < 6; i++){
        pCPDPBP_IDX[i] = 0;
    }
    uint32 tmp = 0;
    if (RearrangedSizes[6] != 0){
    	tmp = tmp_MOD / (RS[5] *RS[4] * RS[3] * RS[2] * RS[1] * RS[0]);
    	tmp_MOD = tmp_MOD % (RS[5] * RS[4] * RS[3] * RS[2] * RS[1] * RS[0]);
    }
    pCPDPBP_IDX[ AS[0] ] = tmp_MOD / (RS[5] *RS[4] * RS[3] * RS[2] * RS[1]);
    tmp_MOD = tmp_MOD % (RS[5] * RS[4] * RS[3] * RS[2] * RS[1]);

    pCPDPBP_IDX[ AS[1] ] = tmp_MOD / (RS[5] *RS[4] * RS[3] * RS[2]);
    tmp_MOD = tmp_MOD % (RS[5] * RS[4] * RS[3] * RS[2]);

    pCPDPBP_IDX[ AS[2] ] = tmp_MOD / (RS[5] *RS[4] * RS[3]);
    tmp_MOD = tmp_MOD % (RS[5] * RS[4] * RS[3]);

    pCPDPBP_IDX[ AS[3] ] = tmp_MOD / (RS[5] *RS[4]);
    tmp_MOD = tmp_MOD % (RS[5] * RS[4]);

    pCPDPBP_IDX[ AS[4] ] = tmp_MOD / (RS[5]);
    tmp_MOD = tmp_MOD % (RS[5]);

    pCPDPBP_IDX[ AS[5] ] = tmp_MOD;
    if (RearrangedSizes[6] != 0){
    	pCPDPBP_IDX[ AS[6] ] = pCPDPBP_IDX[ AS[6] ] * RS[6] + tmp;
    }


#if DBG_PRINT_PPN
    DPRINTF(PAM2,"PAM:    0x%llX (%llu) ==>\n",*pPPN,*pPPN); //Use DPRINTF here
    printCPDPBP(pCPDPBP);
#endif
}

void PAM2::CPDPBPtoPPN(CPDPBP* pCPDPBP, uint64* pPPN)
{
    uint64 AddrPPN = 0;

#if 1
    //with re-arrange ... I hate current structure! too dirty even made with FOR-LOOP! (less-readability)
    uint32* pCPDPBP_IDX = ((uint32*)pCPDPBP);
    uint8*  AS = gconf->AddrSeq;
    uint32* RS = RearrangedSizes;
    AddrPPN += pCPDPBP_IDX[ AS[5] ];
    AddrPPN += pCPDPBP_IDX[ AS[4] ] * (RS[5] );
    AddrPPN += pCPDPBP_IDX[ AS[3] ] * (RS[5] * RS[4]);
    AddrPPN += pCPDPBP_IDX[ AS[2] ] * (RS[5] * RS[4] * RS[3]);
    AddrPPN += pCPDPBP_IDX[ AS[1] ] * (RS[5] * RS[4] * RS[3] * RS[2]);
    AddrPPN += pCPDPBP_IDX[ AS[0] ] * (RS[5] * RS[4] * RS[3] * RS[2] * RS[1]);
#else
    //without re-arrange
    AddrPPN += pCPDPBP->Page;
    AddrPPN += pCPDPBP->Block   * (gconf->NumPage);
    AddrPPN += pCPDPBP->Plane   * (gconf->NumPage * gconf->NumBlock);
    AddrPPN += pCPDPBP->Die     * (gconf->NumPage * gconf->NumBlock * gconf->NumPlane);
    AddrPPN += pCPDPBP->Package * (gconf->NumPage * gconf->NumBlock * gconf->NumPlane * gconf->NumDie);
    AddrPPN += pCPDPBP->Channel * (gconf->NumPage * gconf->NumBlock * gconf->NumPlane * gconf->NumDie * gconf->NumPackage);
#endif

    *pPPN = AddrPPN;

#if DBG_PRINT_PPN
    printCPDPBP(pCPDPBP);
    DPRINTF(PAM2,"PAM    ==> 0x%llx (%llu)\n",*pPPN,*pPPN); //Use DPRINTF here
#endif
}
