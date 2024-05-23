/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */


#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_read(struct memphy_struct * mp, int pid, int pgnum, uint32_t * pte)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   uint32_t tlb_entries_num = mp->tlb_entries_num;
   int index = pgnum % tlb_entries_num;
   int tag = pgnum / tlb_entries_num;
   if(mp->tlb_entries[index].pid != pid){
      return -1;
   }
   if(mp->tlb_entries[index].tag != tag){
      return -1;
   }
   uint32_t mp_pte = mp->tlb_entries[index].pte;
   *pte = mp_pte;
   return 0;
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, uint32_t pte)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    * 
    * 
    */
   uint32_t tlb_entries_num = mp->tlb_entries_num;
   int index = pgnum % tlb_entries_num;
   int tag = pgnum / tlb_entries_num;
   mp->tlb_entries[index].pid = pid;
   mp->tlb_entries[index].tag = tag;
   mp->tlb_entries[index].pte = pte;

   return 0;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */


int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   /*TODO dump memphy contnt mp->storage 
    *     for tracing the memory content
    */

   struct tlb_entry *tlb = mp->tlb_entries;
   uint32_t i;
   for(i = 0; i < mp->tlb_entries_num; i++){
      if(tlb[i].pid == 0){
         printf("0 0 0");
      }
      else{
         printf("%u %u %u", tlb[i].pid, tlb[i].tag, tlb[i].pte);
      }
   }
   return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->tlb_entries = (struct tlb_entry *)malloc(max_size / 12 * sizeof(struct tlb_entry));
   mp->maxsz = max_size;
   mp->tlb_entries_num = max_size / 12;
   mp->rdmflg = 1;

   return 0;
}

//#endif
