#include "mem/GlobalConfig.h"
#include "mem/ConfigReader.h"
//#include "debug/GLOBALCONFIG.hh"



GlobalConfig::GlobalConfig(ConfigReader * cr){


	NANDType = cr->ReadInt32("NANDType",NAND_TLC),
	NumChannel = cr->ReadInt32("NumChannel",1),
	NumPackage = cr->ReadInt32("NumPackage",1),
	NumDie = cr->ReadInt32("NumDie",1),
	NumPlane = cr->ReadInt32("NumPlane",2),
	NumBlock = cr->ReadInt32("NumBlock",1368),
	NumPage = cr->ReadInt32("NumPage",384),
	SizePage = cr->ReadInt32("SizePage",8192),
	DMAMHz =  cr->ReadInt32("DMAMhz",50),
	EnableDMAPreemption = cr->ReadInt32("DMAPreemption", 1),

	FTLOP = cr->ReadFloat("FTLOP", 0.25);
	FTLGCThreshold = cr->ReadFloat("FTLGCThreshold", 0.07); 
	FTLMapN = cr->ReadInt32("FTLMapN", 32);
    FTLMapK = cr->ReadInt32("FTLMapK", 32);

	FTLEraseCycle = cr->ReadInt32("FTLEraseCycle", 100000); 
	SuperblockDegree = cr->ReadInt32("SuperblockDegree", NumChannel*NumPackage*NumDie);
	SuperpageDegree = cr->ReadInt32("SuperpageDegree" , 1); 
	Warmup = cr->ReadFloat("Warmup", 1.0); 

    //Note: Add duplicate check? //remove manual reallocation
//   	AddrSeq[0] = (uint8)cr->ReadInt32("AddrSeq0", ADDR_CHANNEL);
//   	AddrSeq[1] = (uint8)cr->ReadInt32("AddrSeq1", ADDR_PACKAGE);
//   	AddrSeq[2] = (uint8)cr->ReadInt32("AddrSeq2", ADDR_DIE);
//   	AddrSeq[3] = (uint8)cr->ReadInt32("AddrSeq3", ADDR_PLANE);
//   	AddrSeq[4] = (uint8)cr->ReadInt32("AddrSeq4", ADDR_BLOCK);
//   	AddrSeq[5] = (uint8)cr->ReadInt32("AddrSeq5", ADDR_PAGE);


  OriginalSizes[ADDR_CHANNEL] = NumChannel;
  OriginalSizes[ADDR_PACKAGE] = NumPackage;
  OriginalSizes[ADDR_DIE]     = NumDie;
  OriginalSizes[ADDR_PLANE]   = NumPlane;
  OriginalSizes[ADDR_BLOCK]   = NumBlock;
  OriginalSizes[ADDR_PAGE]    = NumPage;
  OriginalSizes[6]            = 0; //Add remaining bits

#if 0
	int superblock = SuperblockDegree;
	AddrSeq[0] = ADDR_PAGE;
	AddrSeq[1] = ADDR_BLOCK;
	AddrSeq[2] = ADDR_PLANE;
	AddrSeq[3] = ADDR_DIE;
	AddrSeq[4] = ADDR_PACKAGE;
	AddrSeq[5] = ADDR_CHANNEL;
	int offset = 5;
	while (superblock > 1){
		superblock = superblock / OriginalSizes[5-offset];
		offset--;
	}
	int tmp = AddrSeq[0];
	unsigned i;
	for (i = 0; i < offset; i++){
		AddrSeq[i] = AddrSeq[i+1];
	}
	AddrSeq[i] = tmp;
#else
	int superblock = SuperblockDegree;
	AddrSeq[0] = ADDR_CHANNEL;
	AddrSeq[1] = ADDR_PACKAGE;
	AddrSeq[2] = ADDR_DIE;
	AddrSeq[3] = ADDR_PLANE;
	AddrSeq[4] = ADDR_BLOCK;
	AddrSeq[5] = ADDR_PAGE;
	int offset = 0;
	while (superblock > 1){
		if (superblock / OriginalSizes[offset] == 0){
			OriginalSizes[6] = OriginalSizes[offset] / superblock;
			OriginalSizes[offset] = superblock;
			AddrSeq[6] = offset;
		}
		superblock = superblock / OriginalSizes[offset];
		offset++;
	}
	unsigned i;
	for (i = 0; i < offset; i++){
		int tmp = AddrSeq[0];
		for (unsigned j = 1; j < 6 - i; j++){
			AddrSeq[j-1] = AddrSeq[j];
		}
		AddrSeq[6-i-1] = tmp;
	}
#endif
  PrintInfo();
  //DPRINTF(GLOBALCONFIG,"\n");
}

void GlobalConfig::PrintInfo()
{
    //Use DPRINTF here - ALL of these
    printf("PAM: [ Configuration ]\n");
    printf("PAM: DMAPreemption=%d\n", EnableDMAPreemption);
    printf("PAM: plane count = %llu planes\n", GetTotalNumPlane() ); //cout<<"plane count = "<< GetTotalNumPlane()<<" planes\n";
    printf("PAM: block count = %llu blocks\n", GetTotalNumBlock() ); //cout<<"block count = "<< GetTotalNumBlock()<<" blocks\n";
    printf("PAM: page count = %llu pages\n", GetTotalNumPage() ); //cout<<"page count = "<< GetTotalNumPage()<<" pages\n";
    printf("PAM: size = %llu Byte\n", GetTotalSizeSSD() ); //cout<<"size = "<< GetTotalSizeSSD()<<" Byte\n";
    printf("PAM: size = %llu MByte\n", GetTotalSizeSSD()/(MBYTE) ); //cout<<"size = "<< GetTotalSizeSSD()/(MBYTE)<<" MByte\n";
    printf("PAM: size = %llu GByte\n", GetTotalSizeSSD()/(GBYTE) ); //cout<<"size = "<< GetTotalSizeSSD()/(GBYTE)<<" GByte\n";
    printf("PAM: AddrSeq:\n");
    /*for (int i = 0;i<6;i++)
    {
        if (i) printf(" | ");
        DPRINTF(GLOBALCONFIG,"%8s", ADDR_STRINFO[AddrSeq[i]]);
	}*/
    printf("PAM: %8s | %8s | %8s | %8s | %8s | %8s\n", ADDR_STRINFO[AddrSeq[0]], ADDR_STRINFO[AddrSeq[1]], ADDR_STRINFO[AddrSeq[2]], ADDR_STRINFO[AddrSeq[3]], ADDR_STRINFO[AddrSeq[4]], ADDR_STRINFO[AddrSeq[5]]);
    
    //DPRINTF(GLOBALCONFIG,"\n"); //cout<<"\n";
    /*for (int i = 0;i<6;i++)
    {
        if (i) DPRINTF(GLOBALCONFIG," | ");
        DPRINTF(GLOBALCONFIG,"%8u", OriginalSizes[AddrSeq[i]]);
	}*/
    printf("PAM: %8u | %8u | %8u | %8u | %8u | %8u\n", OriginalSizes[AddrSeq[0]], OriginalSizes[AddrSeq[1]], OriginalSizes[AddrSeq[2]], OriginalSizes[AddrSeq[3]], OriginalSizes[AddrSeq[4]], OriginalSizes[AddrSeq[5]]);

    switch (NANDType)
    {
        default:
        case NAND_TLC: printf("NANDType = TLC (REAL_TLC_PAGE8K)\n"); break;
        case NAND_MLC: printf("NANDType = MLC (ASSUMED_MLC)\n");     break;
        case NAND_SLC: printf("NANDType = SLC (ASSUMED_SLC)\n");     break;
    }
	
    //DPRINTF(GLOBALCONFIG,"\n"); //cout<<"\n";
    printf("NAND DMA Speed = %u MHz, DMA Pagesize = %u Bytes\n", DMAMHz, SizePage);
}

uint64 GlobalConfig::GetTotalSizeSSD()
{
    return GetTotalNumPage() * (uint64)SizePage;
}

uint64 GlobalConfig::GetTotalNumPage()
{
    return GetTotalNumBlock() * (uint64)NumPage;
}

uint64 GlobalConfig::GetTotalNumBlock()
{
    return GetTotalNumPlane() * (uint64)NumBlock;
}

uint64 GlobalConfig::GetTotalNumDie()
{
    return (uint64)NumChannel * (uint64)NumPackage * (uint64)NumDie;
}

uint64 GlobalConfig::GetTotalNumPlane()
{
    return (uint64)NumChannel * (uint64)NumPackage * (uint64)NumDie * (uint64)NumPlane;
}
