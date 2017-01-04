#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/mm.h>
#include<linux/uaccess.h>
#include<linux/string.h>
#include<asm/highmem.h>
#include<linux/highmem.h>
#include<linux/vmalloc.h>
//#include<asm/pgtable_32.h>
#include <asm/page.h>
//#include <asm/pgtable.h>

#define BUFF_SIZE 3000000

//unsigned long virtophy(struct mm_struct *target_mm, unsigned long target);

asmlinkage void sys_linux_survey_TT(int pid, char __user * result)
{
    struct task_struct *target_task;
    struct mm_struct *target_mm;
    struct vm_area_struct *target_vma;
    struct page *page;
    unsigned long vm_addr, temp_phy;
    char *buff;
    unsigned long err;
    unsigned long pa,pfn;
    
    buff = (char *)vmalloc(BUFF_SIZE * sizeof(char));
    memset(buff , 0 , BUFF_SIZE * sizeof(char));
    
    for_each_process(target_task){
        if(target_task->pid == pid)
            break;
    }
    printk("\nsurvey: target- %d %s\n", target_task->pid, target_task->comm);
    
    target_mm = target_task->mm;
    
    //-----
    /*printk(
     "Code Segment start = %08lx, end = %08lx\n"
     "Data Segement start = %08lx, end = %08lx\n"
     "Heap Segement start = %08lx, end = %08lx\n"
     "Stack Segment start = %08lx\n",
     target_mm->start_code, target_mm->end_code,
     target_mm->start_data, target_mm->end_data,
     target_mm->start_brk, target_mm->brk,
     target_mm->start_stack);*/
    
    for(target_vma = target_mm->mmap; target_vma != NULL; target_vma = target_vma->vm_next){
        
        //printk("VM Area start = %08lx, end = %08lx\n", target_vma->vm_start, target_vma->vm_end);
        
        sprintf(buff + strlen(buff), "%08lx-%08lx~", target_vma->vm_start, target_vma->vm_end);
        if(target_vma->vm_file != NULL){
            printk("file path: %s\n",target_vma->vm_file->f_path.dentry->d_iname);
            sprintf(buff + strlen(buff), "%s", target_vma->vm_file->f_path.dentry->d_iname );
        }
        sprintf(buff + strlen(buff), "#");
        for(vm_addr = target_vma->vm_start ;vm_addr < target_vma->vm_end;vm_addr += 0x1000){
            // printk("Getting in sprintf...\nstart_addr: %08lx target_addr: %08lx\n Now buff_len: %d\n", target_vma->vm_start, vm_addr, strlen(buff));
            //temp_phy = virtophy(target_task->mm, vm_addr);
            page = follow_page(target_mm->mmap, vm_addr ,0);
            if(page == NULL)
                continue;
            pfn = page_to_pfn(page);
            pa = pfn << PAGE_SHIFT;
            //printk("Pa:0x%08lx\n",pa);
            sprintf(buff + strlen(buff), "%08lx-%08lx|", vm_addr, pa);
            //printk("Finish sprintf.\n");
        }
        sprintf(buff + strlen(buff), ":");
    }  //------******-------
    
    //---
    printk(KERN_DEBUG"Debug: buff length : %d\n", strlen(buff));
    //---
    
    err = copy_to_user(result, buff, strlen(buff)+1 );
    if(err)
        printk(KERN_ALERT"\nAlert: %lu bytes copy failed!\n", err);
    
    vfree(buff);
}

/*unsigned long virtophy(struct mm_struct *target_mm ,unsigned long target)
 {
 unsigned long pa,pfn;
 struct page *page;
 //printk("Start virtophy.....\n");
 page = follow_page(target_mm->mmap, target ,0);
 if(page == NULL)
 { pa = 0x00000000;
 goto end;}
 pfn = page_to_pfn(page);
 pa = pfn << PAGE_SHIFT;
 
 printk("Pfn:0x%08lx\n",pa);
 //-------------------------------------------------------------
 
 /*printk(KERN_INFO "Original var_cr3 = 0x%08lx\n", cr3);
 asm volatile(
 "movl %%cr3, %%eax\n"
 "movl %%eax, %0\n"
 : "=r" (cr3)
 :
 : "%eax"
 );
 
 printk(KERN_INFO "cr3 = 0x%08lx\n",cr3);*/


//pgd_t *pgd_tmp = NULL;
//pud_t *pud_tmp = NULL;
//pmd_t *pmd_tmp = NULL;
//pte_t *pte_tmp = NULL;

//-------------------------------------------------------------

/*
 temp = pgd_index(target_mm->start_code);
 printk("PGD_index : %lu\n", temp);
 temp = pud_index(target_mm->start_code);
 printk("PUD_index : %lu\n", temp);
 temp = pmd_index(target_mm->start_code);
 printk("PMD_index : %lu\n", temp);
 temp = pte_index(target_mm->start_code);
 printk("PTE_index : %lu\n", temp);
 */

//printk("PGD: %p \n",target_mm->pgd);

//pgd_tmp = pgd_offset(target_mm,target);
//----
//printk(KERN_INFO"pgd_tmp = 0x%p\n",pgd_tmp);

/*printk(KERN_INFO"pgd_val(*pgd_tmp) = 0x%lx\n",pgd_val(*pgd_tmp));
 if(pgd_none(*pgd_tmp)){
 printk(KERN_INFO"Not mapped in pgd.\n");
 pa = 0x00000000;
 goto end;
 }
 pud_tmp = pud_offset(pgd_tmp,target);
 printk(KERN_INFO"pud_tmp = 0x%p\n",pud_tmp);
 printk(KERN_INFO"pud_val(*pud_tmp) = 0x%lx\n",pud_val(*pud_tmp));
 if(pud_none(*pud_tmp)){
 printk(KERN_INFO"Not mapped in pud.\n");
 pa = 0x00000000;
 goto end;
 }
 pmd_tmp = pmd_offset(pud_tmp,target);
 printk(KERN_INFO"pmd_tmp = 0x%p\n",pmd_tmp);
 printk(KERN_INFO"pmd_val(*pmd_tmp) = 0x%lx\n",pmd_val(*pmd_tmp));
 if(pmd_none(*pmd_tmp)){
 printk(KERN_INFO"Not mapped in pmd.\n");
 pa = 0x00000000;
 goto end;
 }
 //----
 pte_tmp = pte_offset_map(pmd_tmp,target);
 printk(KERN_INFO"pte_tmp = 0x%p\n",pte_tmp);
 
 printk(KERN_INFO"pte_val(*pte_tmp) = 0x%lx\n",pte_val(*pte_tmp));
 
 if(pte_none(*pte_tmp)){
 printk(KERN_INFO"Not mapped in pte.\n");
 pa = 0x00000000;
 goto end;
 }
 
 if(!pte_present(*pte_tmp)){
 printk(KERN_INFO"pte not in RAM.\n");
 pa = 0x00000000;
 goto end;
 }
 
 pa = (pte_val(*pte_tmp) & PAGE_MASK) |(target & ~PAGE_MASK);
 printk(KERN_INFO"virt_addr 0x%08lx in RAM is 0x%08lx .\n",target,pa);*/
/*
 printk(KERN_INFO"contect in 0x%lx is 0x%lx\n",pa, *(unsigned long *)((char *)pa + PAGE_OFFSET));
 
 //pte_unmap(pte_tmp);
 end:
 //printk("End of the kernel program.\n");
 //printk("Finish virtophy!!\n");
 return pa;
 
 }*/