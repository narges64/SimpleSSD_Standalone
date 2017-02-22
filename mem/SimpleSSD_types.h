#ifndef __ssdsim_types_h__
#define __ssdsim_types_h__

/*==============================
    Switches
==============================*/

// Option Switch
#define DMA_PREEMPTION 1  // For distingushing before-implementation. On/off is available at config file.
#define ENABLE_FTL 1   //FTL integration


// DBG Print Switch
#define DBG_PRINT 1
#if DBG_PRINT
   #define DBG_PRINT_PPN 0
   #define DBG_PRINT_TICK 0
   #define DBG_PRINT_CHANNEL 0
   #define DBG_PRINT_BUSY 0
   #define DBG_PRINT_REQSTART 1
   #define DBG_PRINT_REQDONE 1
   #define DBG_PRINT_CONFLICT 0
   #define DBG_PRINT_CONFIGPARSER 0
   #define DBG_PRINT_REQUEST 0
#endif

#define LOG_PRINT_ABSOLUTE_TIME 0
#define LOG_PRINT_CONSUMED_TIME 0

#define LOG_PRINT_OCCUPY_EACH 0

#define GATHER_TIME_SERIES 0
#define GATHER_RESOURCE_CONFLICT 1
#define FULL_VERIFY_TIMELINE 0 // don't flush timeline for verification at the end
#define HARD_VERIFY_TIMELINE 0 // Verify PAM Timeline on every update! ... CAUCTION: takes VERY LONG time


/*==============================
    Strings
==============================*/ 
extern char ADDR_STRINFO[][10];
extern char ADDR_STRINFO2[][15];
extern char OPER_STRINFO[][10];
extern char OPER_STRINFO2[][10];
extern char BUSY_STRINFO[][10];
extern char PAGE_STRINFO[][10];
extern char NAND_STRINFO[][10];
#if GATHER_RESOURCE_CONFLICT
extern char CONFLICT_STRINFO[][10];
#endif


/*==============================
    Macros
==============================*/ 
//#define GET_ACCESSOR(propName, propType) propType get##propName() const { return m_##propName; }
//#define SET_ACCESSOR(propName, propType) void set##propName(const propType &newVal) { m_##propName = newVal; }

#define CREATE_ACCESSOR(propType, propName) \
    private: \
        propType propName; \
    public: \
        void set##propName(const propType newVal) { propName = newVal; }; \
        propType get##propName() const { return propName; };

#define printa(fmt...) do {  printf(fmt);   if(outfp)fprintf(outfp,fmt);} while(0);
#define printo(fmt...) do {  if(!outfp)printf(fmt); if(outfp)fprintf(outfp,fmt);} while(0);
#define printft(fmt...) do { printf("    "); printf(fmt); } while(0);

//#define ERR_EXIT(fmt...) {printf(fmt); std::terminate();}
#define SAFEDIV(left,right) ((right)==0?0:(left)/(right))


/*==============================
    Type & Struct
==============================*/ 
#define MAX64 0xFFFFFFFFFFFFFFFF
#define MAX32 0xFFFFFFFF

#define  BYTE (1)
#define KBYTE (1024)
#define MBYTE (1024*KBYTE)
#define GBYTE (1024*MBYTE)
#define TBYTE (1024*GBYTE)

#define SEC  (1)
#define MSEC (1000)      // 1 000
#define USEC (1000*MSEC) // 1 000 000
#define NSEC ((uint64)1000*USEC) // 1 000 000 000
#define PSEC ((uint64)1000*NSEC) // 1 000 000 000 000

typedef   signed char      sint8 ;
typedef   signed short     sint16;
typedef   signed int       sint32;
typedef   signed long long sint64;

typedef unsigned char      uint8 ;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

//===== Address Sequence =====
enum {
    ADDR_CHANNEL = 0,
    ADDR_PACKAGE = 1,
    ADDR_DIE     = 2,
    ADDR_PLANE   = 3,
    ADDR_BLOCK   = 4,
    ADDR_PAGE    = 5,
    ADDR_NUM
};

//===== Operation Types =====
enum {
    OPER_READ  = 0,
    OPER_WRITE = 1,
    OPER_ERASE = 2,
    OPER_NUM
};

//===== Busy Types =====
enum {
    BUSY_DMA0WAIT,
    BUSY_DMA0,
    BUSY_MEM,
    BUSY_DMA1WAIT,
    BUSY_DMA1,
    BUSY_END,
    BUSY_NUM
};


//===== Log-purpose: Tick types =====
enum {
    TICK_DMA0WAIT = 0, // == TICK_IOREQUESTED
    TICK_DMA0,
    TICK_MEM,
    TICK_DMA1WAIT,
    TICK_DMA1,
    TICK_IOEND,
    TICK_NUM
};

//===== NAND Flash - Page address kind =====
enum {
    PAGE_LSB = 0,
    PAGE_CSB = 1,
    PAGE_MSB = 2,
    PAGE_NUM
};

//===== NAND Flash type =====
enum {
    NAND_SLC,
    NAND_MLC,
    NAND_TLC,
    NAND_NUM
};

//===== Request2 Status type =====
enum
{
    REQSTAT_NEW,
    REQSTAT_PROC,
    REQSTAT_END
};


#if GATHER_RESOURCE_CONFLICT
enum
{
    CONFLICT_NONE = 0,
    CONFLICT_DMA0 = 1<<0, //DMA0 couldn't start due to CH  BUSY --- exclusive to CONFLICT_MEM
    CONFLICT_MEM  = 1<<1, //DMA0 couldn't start due to MEM BUSY --- exclusive to CONFLICT_DMA0
    CONFLICT_DMA1 = 1<<2, //DMA1 couldn't start due to CH  BUSY
    CONFLICT_NUM  = 4 //0~3 = 4
};
#endif

/*
    Todo: Make those structs to class?, to check address limitation & warn.
*/

//===== Divided Address =====
typedef struct _CPDPBP
{
    uint32 Channel;
    uint32 Package;
    uint32 Die;
    uint32 Plane;
    uint32 Block;
    uint32 Page;
}CPDPBP;

//===== PPN Request Info (would be in the queue) =====
class RequestFTL
{
    public:
        uint64 PPN;
        uint8  Oper;
        uint64 TickRequested;
};

//===== Task which can be assign to Channel or Memory =====
class Task
{
    public:
        uint64 PPN;
        CPDPBP CPD;
        uint32 PlaneIdx;
        uint8  Oper;
        uint8  Busy;
      #if DMA_PREEMPTION
        uint64 DMASuspend;   // 0 = no suspend, true = suspended & left time
      #endif
        uint64 TickStart[TICK_NUM]; // End Time of each BUSY status, [BUSY_NUM] = END_TIME, actually not needed for simulation itself.
        uint64 TickNext;
};

//===== PPN Request Info (would be in the queue) =====
class RequestLL
{
    public:
        uint64 PPN;
        uint8  Oper;
        uint64 TickRequested;
        uint64 TickFinished;

        CPDPBP CPD;
        uint8 status; // 0-New, 1-Fetched, 2-Finished
        RequestLL* LLprev; //Dual-Linked-List
        RequestLL* LLnext; //Dual-Linked-List
};

//===== Task which can be assign to Channel or Memory =====
class TaskLL : Task
{
    public:
        RequestLL* SrcRequest; //Dual-Linked-List
};

#endif //__ssdsim_types_h__
