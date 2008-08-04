/**\file*********************************************************************
 *                                                                     \brief
 *  NT Kernel Types support library
 *
 ****************************************************************************
 */

#ifndef NTL__KM_BASEDEF
#define NTL__KM_BASEDEF

#include "../nt/basedef.hxx"
#include "../stdlib.hxx"
#include "../nt/exception.hxx"
#include "../nt/string.hxx"

namespace ntl {

  namespace intrinsic {

#if defined(_M_IX86)

#elif defined(_M_X64)
    extern "C" uint64_t __cdecl __readcr8();
    extern "C" void __cdecl __writecr8(uint64_t);

#	pragma intrinsic(__readcr8)
#	pragma intrinsic(__writecr8)

#endif

  }//namespace intrinsic


namespace km {

#ifndef SYSTEM_CACHE_ALIGNMENT_SIZE
#define SYSTEM_CACHE_ALIGNMENT_SIZE 64
#endif

using nt::slist_header;
using nt::single_list_entry;
using nt::slist_entry;
using nt::list_entry;
using nt::list_head;
using nt::status;
using nt::exception;
using nt::access_mask;
using nt::synchronize;
using nt::generic_read;

using nt::io_status_block;
using nt::io_apc_routine;

using nt::device_power_state;
using nt::system_power_state;

using nt::ansi_string;
using nt::unicode_string;
using nt::const_unicode_string;
using nt::const_ansi_string;



struct kthread;
struct mdl;
struct kevent;

//enum mode { KernelMode, UserMode };
struct kprocessor_mode { uint8_t mode; };

static const kprocessor_mode KernelMode = { 0 };
static const kprocessor_mode UserMode = { 1 };

typedef long kpriority;
typedef uintptr_t kaffinity_t;

#if 1

/// interrupt request level
typedef uint8_t kirql_t;

#if defined(_M_IX86)
NTL__EXTERNAPI
kirql_t __stdcall
  KeGetCurrentIrql();

NTL__EXTERNAPI
kirql_t __fastcall
  KfLowerIrql(kirql_t NewIrql);

NTL__EXTERNAPI
kirql_t __fastcall
  KfRaiseIrql(kirql_t NewIrql);

#elif defined(_M_X64)

__forceinline
kirql_t KeGetCurrentIrql()
{
  return (kirql_t)ntl::intrinsic::__readcr8();
}

__forceinline
kirql_t KfRaiseIrql(kirql_t NewIrql)
{
  kirql_t OldIrql = KeGetCurrentIrql();
  ntl::intrinsic::__writecr8(NewIrql);
  return OldIrql;
}

__forceinline
void KfLowerIrql(kirql_t NewIrql)
{
  ntl::intrinsic::__writecr8(NewIrql);
}

#endif

#ifdef _DEBUG
NTL__EXTERNAPI
void __stdcall
DbgBreakPointWithStatus(
                        ntstatus Status
                        );
#endif

/** Interrupt request level RAII wrapper */
struct kirql
{ 
  /// level definitions
  enum level {
		passive_level = 0,             // passive release level
		low_level = 0,                 // lowest interrupt level
		apc_level = 1,                 // apc interrupt level
		dispatch_level = 2,            // dispatcher level

#if defined(_M_IX86)
		profile_level = 27,            // timer used for profiling.
		clock1_level = 28,             // interval clock 1 level - not used on x86
		clock2_level = 28,             // interval clock 2 level
		ipi_level,                     // interprocessor interrupt level
		power_level,                   // power failure level
		high_level,                    // highest interrupt level
#elif defined(_M_X64)
    clock_level = 13,              // interval clock level
    ipi_level,                     // interprocessor interrupt level
    power_level,                   // power failure level
    profile_level,                 // timer used for profiling
    high_level,                    // highest interrupt level
#endif

#ifdef NT_UP
    // UP
    synch_level = dispatch_level  // synchronization level
#else
    // MP
    synch_level = ipi_level-2
#endif
	};

protected:
	kirql_t _;

public:
  kirql()
  {
    _ = KeGetCurrentIrql();
  }
  kirql(kirql_t irql)
  {
    _ = irql;
  }

	static kirql get_current()
	{
		return kirql(KeGetCurrentIrql());
	}

  void raise(const level NewIrql)
  {
#ifdef _DEBUG
    if(KeGetCurrentIrql() > static_cast<kirql_t>(NewIrql))
      // lower irql is impossible
      // IRQL_NOT_GREATER_OR_EQUAL
      DbgBreakPointWithStatus((ntstatus)0x00000009);
#endif
    _ = KfRaiseIrql(static_cast<kirql_t>(NewIrql));
  }
  void raisetodpc()
  {
#ifdef _DEBUG
    if(KeGetCurrentIrql() > static_cast<kirql_t>(dispatch_level))
      // lower irql is impossible
      // IRQL_NOT_GREATER_OR_EQUAL
      DbgBreakPointWithStatus((ntstatus)0x00000009);
#endif
    _ = KfRaiseIrql(dispatch_level);
  }
  void raisetosynch()
  {
#ifdef _DEBUG
    if(KeGetCurrentIrql() > static_cast<kirql_t>(synch_level))
      // lower irql is impossible
      // IRQL_NOT_GREATER_OR_EQUAL
      DbgBreakPointWithStatus((ntstatus)0x00000009);
#endif
    _ = KfRaiseIrql(synch_level);
  }
  void lower()
  {
    KfLowerIrql(_);
  }

  operator kirql_t() const
  {
    return _;
  }

	friend bool operator == (const kirql irql, level l)
	{
		return irql._ == static_cast<kirql_t>(l);
	}
	friend bool operator < (const kirql irql, level l)
	{
		return irql._ < static_cast<kirql_t>(l);
	}
	friend bool operator > (const kirql irql, level l)
	{
		return irql._ > static_cast<kirql_t>(l);
	}
};

#else
typedef uint8_t kirql;
#endif

struct kspin_lock
{ 
    kspin_lock() : _() {} // no need for KeInitializeSpinLock call

    __forceinline kirql acquire();
    __forceinline void release(const kirql);

  private:
    /*volatile*/ uintptr_t _;
};

typedef uint64_t kspin_lock_queue_number_t;
enum kspin_lock_queue_number 
{
  LockQueueDispatcherLock,
  LockQueueExpansionLock,
  LockQueuePfnLock,
  LockQueueSystemSpaceLock,
  LockQueueVacbLock,
  LockQueueMasterLock,
  LockQueueNonPagedPoolLock,
  LockQueueIoCancelLock,
  LockQueueWorkQueueLock,
  LockQueueIoVpbLock,
  LockQueueIoDatabaseLock,
  LockQueueIoCompletionLock,
  LockQueueNtfsStructLock,
  LockQueueAfdWorkQueueLock,
  LockQueueBcbLock,
  LockQueueMmNonPagedPoolLock,
  LockQueueUnusedSpare16,
  LockQueueTimerTableLock,
  LockQueueMaximumLock = LockQueueTimerTableLock + (1 << (8-4))
};

struct kspin_lock_queue
{
  kspin_lock_queue* volatile Next;
  kspin_lock* volatile Lock;
};

#define MAXIMUM_LEADBYTES   12

struct cptableinfo {
  uint16_t CodePage;                    // code page number
  uint16_t MaximumCharacterSize;        // max length (bytes) of a char
  uint16_t DefaultChar;                 // default character (MB)
  uint16_t UniDefaultChar;              // default character (Unicode)
  uint16_t TransDefaultChar;            // translation of default char (Unicode)
  uint16_t TransUniDefaultChar;         // translation of Unic default char (MB)
  uint16_t DBCSCodePage;                // Non 0 for DBCS code pages
  uint8_t  LeadByte[MAXIMUM_LEADBYTES]; // lead byte ranges
  uint16_t*MultiByteTable;             // pointer to MB translation table
  void*    WideCharTable;              // pointer to WC translation table
  uint16_t*DBCSRanges;                 // pointer to DBCS ranges
  uint16_t*DBCSOffsets;                // pointer to DBCS offsets
};

struct nlstableinfo {
  cptableinfo OemTableInfo;
  cptableinfo AnsiTableInfo;
  uint16_t*   UpperCaseTable;             // 844 format upcase table
  uint16_t*   LowerCaseTable;             // 844 format lower case table
};

extern "C" {
  extern uint32_t* NtBuildNumber;
  extern uint32_t* InitSafeBootMode;
#if 0
  extern unicode_string* NtSystemRoot;
  extern const uint32_t* NtMajorVersion;
  extern const uint32_t* NtMinorVersion;
  extern uint32_t* CmNtCSDVersion;
  extern uint32_t* CmNtCSDReleaseType;
  extern uint32_t* CmNtSpBuildNumber;
  extern unicode_string* CmVersionString;
  extern unicode_string* CmCSDVersionString;

  extern const char NtBuildLab[];

  extern nlstableinfo* InitTableInfo;
  extern uint32_t* InitNlsTableSize;
  extern void** InitNlsTableBase;
  extern uint32_t* InitAnsiCodePageDataOffset;
  extern uint32_t* InitOemCodePageDataOffset;
  extern uint32_t* InitUnicodeCaseTableDataOffset;
  extern void** InitNlsSectionPointer;
  extern bool* InitSafeModeOptionPresent;

  extern bool* InitIsWinPEMode;
  extern uint32_t* InitWinPEModeType;
#endif
}

NTL__EXTERNAPI
bool __stdcall
  PsGetVersion(
    uint32_t *        MajorVersion,
    uint32_t *        MinorVersion,
    uint32_t *        BuildNumber,
    unicode_string *  CSDVersion
    );

static inline
bool get_version(uint32_t& major_version, uint32_t& minor_version)
{
  return PsGetVersion(&major_version, &minor_version, 0, 0);
}

NTL__EXTERNAPI
ntstatus __stdcall
  ZwYieldExecution();

static inline
ntstatus yield_execution()
{
  return ZwYieldExecution();
}


NTL__EXTERNAPI
ntstatus __stdcall
  KeDelayExecutionThread(
    kprocessor_mode WaitMode,
    bool            Alertable,
    const int64_t * Interval
    );


template<times TimeResolution>
static inline
ntstatus sleep(
  uint32_t        time_resolution,
  bool            alertable = false,
  kprocessor_mode wait_mode = KernelMode)
{
  const int64_t interval = int64_t(-1) * TimeResolution * time_resolution;
  return KeDelayExecutionThread(wait_mode, alertable, &interval);
}

/// default milliseconds
static inline
ntstatus sleep(
  uint32_t        ms,
  bool            alertable = false,
  kprocessor_mode wait_mode = KernelMode)
{
  const int64_t interval = int64_t(-1) * milliseconds * ms;
  return KeDelayExecutionThread(wait_mode, alertable, &interval);
}


NTL__EXTERNAPI
kirql_t __fastcall
  KfAcquireSpinLock(kspin_lock * SpinLock);

NTL__EXTERNAPI
void __fastcall
  KfReleaseSpinLock(
    kspin_lock *  SpinLock,
    const kirql_t   NewIrql
    );


kirql kspin_lock::acquire()
{
  return KfAcquireSpinLock(this);
}

void kspin_lock::release(const kirql new_irql)
{
  KfReleaseSpinLock(this, new_irql);
}


struct kspin_lock_at_dpc : public kspin_lock
{
    __forceinline void acquire();
    __forceinline void release();
};

NTL__EXTERNAPI
void __fastcall
  KefAcquireSpinLockAtDpcLevel(kspin_lock * SpinLock);

NTL__EXTERNAPI
void __fastcall
  KefReleaseSpinLockFromDpcLevel(kspin_lock * SpinLock);


void kspin_lock_at_dpc::acquire()
{
  KefAcquireSpinLockAtDpcLevel(this);
}

void kspin_lock_at_dpc::release()
{
  KefReleaseSpinLockFromDpcLevel(this);
}


typedef ntl::atomic_exec<kspin_lock, kirql> synchronize_access;
typedef ntl::atomic_exec<kspin_lock_at_dpc, void> synchronize_access_at_dpc_level;


struct dispatcher_header
{
  uint8_t     Type;
  uint8_t     Absolute;
  uint8_t     Size;
  uint8_t     Inserted;
  long        SignalState;
  list_entry  WaitListHead;
};

struct kgate { dispatcher_header Header; };


enum kwait_reason
{
  Executive,
  FreePage,
  PageIn,
  PoolAllocation,
  DelayExecution,
  Suspended,
  UserRequest,
  WrExecutive,
  WrFreePage,
  WrPageIn,
  WrPoolAllocation,
  WrDelayExecution,
  WrSuspended,
  WrUserRequest,
  WrEventPair,
  WrQueue,
  WrLpcReceive,
  WrLpcReply,
  WrVirtualMemory,
  WrPageOut,
  WrRendezvous,
  Spare2,
  Spare3,
  Spare4,
  Spare5,
  Spare6,
  WrKernel,
  WrResource,
  WrPushLock,
  WrMutex,
  WrQuantumEnd,
  WrDispatchInt,
  WrPreempted,
  WrYieldExecution,
  WrFastMutex,
  WrGuardedMutex,
  WrRundown,
  MaximumWaitReason
}; // enum kwait_reason

NTL__EXTERNAPI
ntstatus __stdcall
  KeWaitForSingleObject(
    void *          Object,
    kwait_reason    WaitReason,
    kprocessor_mode WaitMode,
    bool            Alertable,
    int64_t *       Timeout  __optional
    );

static inline
ntstatus
  wait_for_single_object(
    void *          Object,
    kwait_reason    WaitReason  = Executive,
    kprocessor_mode WaitMode    = KernelMode,
    bool            Alertable   = false,
    int64_t *       Timeout     = 0
    )
{
  return KeWaitForSingleObject(Object, WaitReason, WaitMode, Alertable, Timeout);
}

#if 0

struct kdpc;

typedef
void __stdcall
  kdeferred_routine_t(
    const kdpc *  Dpc,
    const void *  DeferredContext,
    const void *  SystemArgument1,
    const void *  SystemArgument2
    );

struct kdpc
{
  uint8_t               Type;
  uint8_t               Importance;
  uint8_t               Number;
  uint8_t               Expedite;
  list_entry            DpcListEntry;
  kdeferred_routine_t * DeferredRoutine;
  void *                DeferredContext;
  void *                SystemArgument1;
  void *                SystemArgument2;
  void *                DpcData;
};

#else

struct kdpc_data 
{
  list_entry DpcListHead;
  kspin_lock DpcLock;
#if defined(_M_IX86)
  volatile int32_t DpcQueueDepth;
#elif defined(_M_X64)
  volatile int32_t DpcQueueDepth;
#endif
  uint32_t DpcCount;
};

struct kdpc 
{
  enum DpcType {
    Normal,
    Threaded
  };
  enum DpcImportance {
    LowImportance,
    MediumImportance,
    HighImportance
  };

  uint8_t Type;
#if defined(_M_IX86)
  uint8_t Number;
  uint8_t Importance;
#elif defined(_M_X64)
  uint8_t Importance;
  uint8_t Number;
  uint8_t Expedite;
#endif
  list_entry DpcListEntry;

  void (__stdcall *DeferredRoutine)(
    const kdpc* Dpc, 
    const void* DeferredContext, 
    const void* SystemArgument1, 
    const void* SystemArgument2);

  void* DeferredContext;
  void* SystemArgument1;
  void* SystemArgument2;

  kdpc_data* DpcData;
};

#if defined(_M_IX86)
STATIC_ASSERT(sizeof(kdpc) == 0x20);
#elif  _M_X64
STATIC_ASSERT(sizeof(kdpc) == 0x40);
#endif

#endif


struct ksemaphore
{
  dispatcher_header Header;
  long  Limit;
};


struct work_queue_item 
{
  list_entry  List;
  void     (* WorkerRoutine)(void*);
  void      * Parameter;
};

//
//  executive resource data structures.
//

typedef uintptr_t eresource_thread_t;

struct owner_entry {
	eresource_thread_t OwnerThread;
	union {
		int32_t OwnerCount;
		uint32_t TableSize;
	};

};

struct eresource {
	list_entry SystemResourcesList;
	owner_entry* OwnerTable;

	int16_t ActiveCount;
	enum Flags {
		ResourceNeverExclusive		= 0x10,
		ResourceReleaseByOtherThread= 0x20,
		ResourceOwnedExclusive		= 0x80
	};
	uint16_t Flag;

	volatile ksemaphore* SharedWaiters;
	volatile kevent* ExclusiveWaiters;

	owner_entry OwnerEntry;
	uint32_t ActiveEntries;
	uint32_t ContentionCount;
	uint32_t NumberOfSharedWaiters;
	uint32_t NumberOfExclusiveWaiters;
#if defined(_M_X64)
	void* Reserved2;
#endif
	union {
		void* Address;
		uintptr_t CreatorBackTraceIndex;
	};

	kspin_lock SpinLock;
};


#define RESOURCE_HASH_TABLE_SIZE 64

struct resource_hash_entry {
	list_entry ListEntry;
	void* Address;
	uint32_t ContentionCount;
	uint32_t Number;
};

struct resource_performance_data {
	uint32_t ActiveResourceCount;
	uint32_t TotalResourceCount;
	uint32_t ExclusiveAcquire;
	uint32_t SharedFirstLevel;
	uint32_t SharedSecondLevel;
	uint32_t StarveFirstLevel;
	uint32_t StarveSecondLevel;
	uint32_t WaitForExclusive;
	uint32_t OwnerTableExpands;
	uint32_t MaximumTableExpand;
	list_entry HashTable[RESOURCE_HASH_TABLE_SIZE];
};

struct ex_rundown_ref
{
  union
  {
    /*<thisrel this+0x0>*/ /*|0x4|*/ uint32_t Count;
    /*<thisrel this+0x0>*/ /*|0x4|*/ void* Ptr;
  };
};

struct ex_fast_ref
{
  union {
    /*<thisrel this+0x0>*/ /*|0x4|*/ void* Object;
    /*<bitfield this+0x0>*/ /*|0x4|*/ uint32_t RefCnt:3;
    /*<thisrel this+0x0>*/ /*|0x4|*/ uint32_t Value;
  };
};// <size 0x4>

struct ex_push_lock
{
  union {
    /*<bitfield this+0x0>*/ /*|0x4|*/ uint32_t Waiting:1;
    /*<bitfield this+0x0>*/ /*|0x4|*/ uint32_t Exclusive:1;
    /*<bitfield this+0x0>*/ /*|0x4|*/ uint32_t Shared:0x1E;
    /*<thisrel this+0x0>*/ /*|0x4|*/ uint32_t Value;
    /*<thisrel this+0x0>*/ /*|0x4|*/ void* Ptr;
  };
};// <size 0x4>



struct pp_lookaside_list {
  struct _general_lookaside *P;
  struct _general_lookaside *L;
};

static const size_t pool_small_lists = 32;

    //
    // Define executive resource function prototypes.
    //
    NTL__EXTERNAPI
      ntstatus __stdcall
      ExInitializeResourceLite (
      eresource* Resource
      );

    NTL__EXTERNAPI
      ntstatus __stdcall
      ExReinitializeResourceLite (
      eresource* Resource
      );

    NTL__EXTERNAPI
      bool
      ExAcquireResourceSharedLite (
      eresource* Resource,
      bool Wait
      );

#if (NTDDI_VERSION >= NTDDI_LONGHORN || NTDDI_VERSION >= NTDDI_WS03SP1)
    NTL__EXTERNAPI
      void*
      ExEnterCriticalRegionAndAcquireResourceShared (
      eresource* Resource
      );
#endif

    NTL__EXTERNAPI
      bool
      ExAcquireResourceExclusiveLite (
      eresource* Resource,
      bool Wait
      );

#if (NTDDI_VERSION >= NTDDI_LONGHORN || NTDDI_VERSION >= NTDDI_WS03SP1)
    NTL__EXTERNAPI
      void*
      ExEnterCriticalRegionAndAcquireResourceExclusive (
      eresource* Resource
      );
#endif

    NTL__EXTERNAPI
      bool
      ExAcquireSharedStarveExclusive(
      eresource* Resource,
      bool Wait
      );

    NTL__EXTERNAPI
      bool
      ExAcquireSharedWaitForExclusive(
      eresource* Resource,
      bool Wait
      );

#if (NTDDI_VERSION >= NTDDI_LONGHORN || NTDDI_VERSION >= NTDDI_WS03SP1)
    NTL__EXTERNAPI
      void*
      ExEnterCriticalRegionAndAcquireSharedWaitForExclusive (
      eresource* Resource
      );
#endif

    //
    //  void
    //  ExReleaseResource(
    //      IN eresource* Resource
    //      );
    //

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExReleaseResource)       // Use ExReleaseResourceLite
#endif
#define ExReleaseResource(R) (ExReleaseResourceLite(R))

    NTL__EXTERNAPI
      void
      __fastcall
      ExReleaseResourceLite(
      eresource* Resource
      );

#if (NTDDI_VERSION >= NTDDI_LONGHORN || NTDDI_VERSION >= NTDDI_WS03SP1)
    NTL__EXTERNAPI
      void
      __fastcall
      ExReleaseResourceAndLeaveCriticalRegion(
      eresource* Resource
      );
#endif

    NTL__EXTERNAPI
      void
      ExReleaseResourceForThreadLite(
      eresource* Resource,
      eresource_thread_t ResourceThreadId
      );

    NTL__EXTERNAPI
      void
      ExSetResourceOwnerPointer(
      eresource* Resource,
      void* OwnerPointer
      );

    NTL__EXTERNAPI
      void
      ExConvertExclusiveToSharedLite(
      eresource* Resource
      );

    NTL__EXTERNAPI
      ntstatus __stdcall
      ExDeleteResourceLite (
      eresource* Resource
      );

    NTL__EXTERNAPI
      uint32_t
      ExGetExclusiveWaiterCount (
      eresource* Resource
      );

    NTL__EXTERNAPI
      uint32_t
      ExGetSharedWaiterCount (
      eresource* Resource
      );

    NTL__EXTERNAPI
      bool
      ExIsResourceAcquiredExclusiveLite (
      eresource* Resource
      );

    NTL__EXTERNAPI
      uint32_t
      ExIsResourceAcquiredSharedLite (
      eresource* Resource
      );

#define ExIsResourceAcquiredLite ExIsResourceAcquiredSharedLite

}//namespace km
}//namespace ntl

#endif//#ifndef NTL__KM_BASEDEF
