// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct rg_elmt)
{
  struct vm_rg_struct *new_rg = malloc(sizeof(struct vm_rg_struct));
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;
  new_rg->rg_start = rg_elmt.rg_start;
  new_rg->rg_end = rg_elmt.rg_end;

  if (rg_elmt.rg_start >= rg_elmt.rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt.rg_next = rg_node;

  if (rg_node != NULL)
  {
    new_rg->rg_next = mm->mmap->vm_freerg_list;
  }
  else
  {
    new_rg->rg_next = NULL;
  }
  /* Enlist the new region */
  mm->mmap->vm_freerg_list = new_rg;
  return 0;
}

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  // mmap points to nhung vm_area
  // this instruction is create a dynamic array which points to the first index of mm->mmap
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = 0;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    vmait++;
    pvma = pvma->vm_next;
  }
  // print_list_rg(pvma->vm_freerg_list);
  return pvma;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;
  if (mm->symrgtbl[rgid].rg_start == mm->symrgtbl[rgid].rg_end)
    return NULL;
  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;

  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;

    *alloc_addr = rgnode.rg_start;

    // return 0;
  }
  // printf("khong tim duoc region \n");
  /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /*Attempt to increate limit to get space */
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  // printf("vmaid: %d", vmaid);
  if (cur_vma == NULL)
  {
    // printf("get here\n");
    return -1;
  }

  // int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  // int inc_limit_ret
  int old_sbrk;

  old_sbrk = cur_vma->sbrk;
  if (old_sbrk + size <= cur_vma->vm_end)
  {
    cur_vma->sbrk += size;
    caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
    caller->mm->symrgtbl[rgid].rg_end = old_sbrk + size;
  }
  /* TODO INCREASE THE LIMIT
   * inc_vma_limit(caller, vmaid, inc_sz)
   */
  else
  {
    inc_vma_limit(caller, vmaid, size);

    /*Successful increase limit */
    caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
    caller->mm->symrgtbl[rgid].rg_end = old_sbrk + size;
  }
  *alloc_addr = old_sbrk;
#ifdef DEBUG_MM
  print_pgtbl(caller, caller->mm->mmap->vm_start, caller->mm->mmap->vm_end);
#endif
  return 0;
}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  struct vm_rg_struct rgnode;

  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;

  /* TODO: Manage the collect freed region to freerg_list */
  rgnode = caller->mm->symrgtbl[rgid];

  /*enlist the obsoleted memory region */
  enlist_vm_freerg_list(caller->mm, rgnode);
  printf("free region: %ld %ld \n", caller->mm->mmap->vm_freerg_list->rg_start, caller->mm->mmap->vm_freerg_list->rg_end);
  return 0;
}

/*pgalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int pgalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  /* By default using vmaid = 1 */
  return __alloc(proc, 0, reg_index, size, &addr);
}

/*pgfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int pgfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  return __free(proc, 0, reg_index);
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller, int *frmnum)
{
  uint32_t tlb_pte;
  // if in TLB
  if (tlb_cache_read(caller->tlb, caller->pid, pgn, &tlb_pte) == 0)
  {
    // if not in RAM
    *frmnum = 0;
    if (!PAGING_PAGE_PRESENT(tlb_pte))
    {
      /* Page is not online, make it actively living */
      int vicpgn, swpfpn, emptyfpn;
      // int vicfpn;
      // uint32_t vicpte;
      // int tgtfpn = PAGING_SWP(pte);
      int tgtfpn = PAGING_FPN(tlb_pte); // the target frame storing our variable
      // if RAM has space
      if (MEMPHY_get_freefp(caller->mram, &emptyfpn) == 0)
      {
        __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, emptyfpn);
        MEMPHY_put_freefp(caller->active_mswp, tgtfpn);
        // update the pgd
        pte_set_fpn(&caller->mm->pgd[pgn], emptyfpn);
#ifdef CPU_TLB
        // after being pushed to RAM, update the TLB
        // because just 1 page is edited
        tlb_cache_write(caller->tlb, caller->pid, pgn, caller->mm->pgd[pgn]);
#endif
      }
      // RAM has no spaces
      else
      {
        /* TODO: Play with your paging theory here */
        /* Find victim page */
        find_victim_page(caller->mm, &vicpgn);
        int vicfpn = PAGING_FPN(caller->mm->pgd[vicpgn]);
        /* Get free frame in MEMSWP */
        MEMPHY_get_freefp(caller->active_mswp, &swpfpn);

        /* Do swap frame from MEMRAM to MEMSWP and vice versa*/
        /* Copy victim frame to swap */
        __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
        /* Copy target frame from swap to mem */
        __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vicfpn);

        /* Update page table */
        pte_set_swap_fpn(&caller->mm->pgd[vicpgn], swpfpn);
        // &mm->pgd;

        /* Update its online status of the target page */
        pte_set_fpn(&caller->mm->pgd[pgn], vicfpn);
        // & mm->pgd[pgn];
        // pte_set_fpn(&pte, tgtfpn);
        MEMPHY_put_freefp(caller->active_mswp, tgtfpn);
#ifdef CPU_TLB
        /* Update its online status of TLB (if needed) */
        uint32_t pte_check;
        if (tlb_cache_read(caller->tlb, caller->pid, vicpgn, &pte_check) == 0)
        {
          tlb_cache_write(caller->tlb, caller->pid, vicpgn, caller->mm->pgd[vicpgn]);
        }
        tlb_cache_write(caller->tlb, caller->pid, pgn, caller->mm->pgd[pgn]);
#endif
      }
      enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
    }
  }
  // not in TLB
  else
  {
    uint32_t pte = mm->pgd[pgn];
    if (!PAGING_PAGE_PRESENT(pte))
    {
      /* Page is not online, make it actively living */
      int vicpgn, swpfpn, emptyfpn;
      // int vicfpn;
      // uint32_t vicpte;
      // int tgtfpn = PAGING_SWP(pte); // the target frame storing our variable
      int tgtfpn = PAGING_FPN(pte);
      if (MEMPHY_get_freefp(caller->mram, &emptyfpn) == 0)
      {
        __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, emptyfpn);
        MEMPHY_put_freefp(caller->active_mswp, tgtfpn);
        pte_set_fpn(&caller->mm->pgd[pgn], emptyfpn);
#ifdef CPU_TLB
        tlb_cache_write(caller->tlb, caller->pid, pgn, caller->mm->pgd[pgn]);
#endif
      }
      else
      {
        /* TODO: Play with your paging theory here */
        /* Find victim page */
        find_victim_page(caller->mm, &vicpgn);
        int vicfpn = PAGING_FPN(caller->mm->pgd[vicpgn]);
        /* Get free frame in MEMSWP */
        MEMPHY_get_freefp(caller->active_mswp, &swpfpn);
        /* Do swap frame from MEMRAM to MEMSWP and vice versa*/
        /* Copy victim frame to swap */
        __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
        /* Copy target frame from swap to mem */
        __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vicfpn);

        /* Update page table */
        pte_set_swap_fpn(&caller->mm->pgd[vicpgn], swpfpn);
        // &mm->pgd;
        /* Update its online status of the target page */
        pte_set_fpn(&caller->mm->pgd[pgn], vicfpn);
        // & mm->pgd[pgn];
        // pte_set_fpn(&pte, tgtfpn);
        MEMPHY_put_freefp(caller->active_mswp, tgtfpn);
#ifdef CPU_TLB
        /* Update its online status of TLB (if needed) */
        uint32_t pte_check;
        if (tlb_cache_read(caller->tlb, caller->pid, vicpgn, &pte_check) == 0)
        {
          tlb_cache_write(caller->tlb, caller->pid, vicpgn, caller->mm->pgd[vicpgn]);
        }
        tlb_cache_write(caller->tlb, caller->pid, pgn, caller->mm->pgd[pgn]);
#endif
      }
      enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
    }
    tlb_cache_write(caller->tlb, caller->pid, pgn, caller->mm->pgd[pgn]);
  }
  *fpn = PAGING_FPN(caller->tlb->tlb_entries[pgn].pte);
  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller, int *frmnum)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller, frmnum) != 0)
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_read(caller->mram, phyaddr, data);

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller, int *frmnum)
{
  int pgn = addr / PAGING_PAGESZ;
  int off = addr - pgn * PAGING_PAGESZ;
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller, frmnum) != 0)
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_write(caller->mram, phyaddr, value);

  return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data, int *frmnum)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller, frmnum);

  return 0;
}

/*pgwrite - PAGING-based read a region memory */
int pgread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t destination)
{
  BYTE data;
  int frmnum;
  int val = __read(proc, 0, source, offset, &data, &frmnum);

  destination = (uint32_t)data;
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); // print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value, int *frmnum)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller, frmnum);

  return 0;
}

/*pgwrite - PAGING-based write a region memory */
int pgwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); // print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  int frmnum;
  return __write(proc, 0, destination, offset, data, &frmnum);
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;

  for (pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte = caller->mm->pgd[pagenum];

    if (!PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    }
    else
    {
      fpn = PAGING_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);
    }
  }

  return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct *
get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
  struct vm_rg_struct *newrg;
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  newrg = malloc(sizeof(struct vm_rg_struct));

  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = newrg->rg_start + size;

  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
  // struct vm_area_struct *vma = caller->mm->mmap;

  /* TODO validate the planned memory area is not overlapped */

  return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
{
  struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz - (cur_vma->vm_end - cur_vma->sbrk));

  int incnumpage = inc_amt / PAGING_PAGESZ;
  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);

  int old_end = cur_vma->vm_end;

  /*Validate overlap of obtained region */
  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
    return -1; /*Overlap and failed allocation */

  /* The obtained vm area (only)
   * now will be alloc real ram region */
  cur_vma->vm_end += inc_amt;
  // printf("vm_end", cur_vma->vm_end);
  cur_vma->sbrk += inc_sz;
  if (vm_map_ram(caller, area->rg_start, area->rg_end,
                 old_end, incnumpage, newrg) < 0)
    return -1; /* Map the memory to MEMRAM */

  return 0;
}

/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  /* TODO: Implement the theorical mechanism to find the victim page */
  if (pg == NULL)
  {
    *retpgn = -1;
    free(pg);
    return -1;
  }
  *retpgn = pg->pgn;
  mm->fifo_pgn = pg->pg_next;
  free(pg);

  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  // printf("found\n");
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL)
  {
    return -1;
  }

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
  if (rgit == NULL)
  {
    return -1;
  }
  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;
  // printf("%ld %ld \n",rgit->rg_start ,rgit->rg_end);
  /* Traverse on list of free vm region to find a fit space */
  while (rgit != NULL)
  {
    if (rgit->rg_start + size <= rgit->rg_end)
    { /* Current region has enough space */

      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;
      // printf("rg_start %lu, rg_end %lu", newrg->rg_start ,newrg->rg_end);
      /* Update left space in chosen region */
      if (rgit->rg_start + size < rgit->rg_end)
      {
        rgit->rg_start = rgit->rg_start + size;
      }
      else
      { /*Use up all space, remove current node */
        /*Clone next rg node */
        struct vm_rg_struct *nextrg = rgit->rg_next;

        /*Cloning */
        if (nextrg != NULL)
        {
          rgit->rg_start = nextrg->rg_start;
          rgit->rg_end = nextrg->rg_end;

          rgit->rg_next = nextrg->rg_next;

          free(nextrg);
        }
        else
        {                                /*End of free list */
          rgit->rg_start = rgit->rg_end; // dummy, size 0 region
          rgit->rg_next = NULL;
        }
      }
      break;
    }
    else
    {
      rgit = rgit->rg_next; // Traverse next rg
    }
  }
  printf("found\n");
  if (newrg->rg_start == -1) // new region not found
    return -1;

  return 0;
}

// #endif
