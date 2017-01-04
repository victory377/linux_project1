#include <syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CLOCK_PER_SEC 1000
#define buff_size 3000000
#define times 10

struct phy_addr_list{
    unsigned long virt_addr;
    unsigned long phy_addr;
    struct phy_addr_list *next;
};
struct VMA_list{
    unsigned long start;
    unsigned long end;
    unsigned long phy_start;
    unsigned long phy_end;
    int offset;
    float percent;
    char *path_name;
    struct phy_addr_list *phy_addr;
    struct VMA_list *next;
};
struct cor_phy{
    unsigned long phy_addr;
    unsigned long virt_addr1;
    unsigned long virt_addr2;
    char *path_name1;
    char *path_name2;
    struct cor_phy *next;
};

struct cor_phy *correspond_phy;

void string_cut (char *result, struct VMA_list *vma_header, int pattern);
void memory_free(struct VMA_list *vma_header1);

int main(int argc, char** argv) {
    int i=0, pid, pattern=0,percent = 0;
    char *result;
    unsigned long *imemoryAddr2;
    struct VMA_list *vma_header1;
    struct VMA_list *vma_header2;
    struct cor_phy *cor_prev = NULL;
    struct cor_phy *cor_current = NULL;
    unsigned long Max,Min;
    clock_t start,end;
    float diff,total;
    int vma = 0,phy = 0;
    
    
    
    vma_header1 =  (struct VMA_list *) malloc(sizeof(struct VMA_list));
    vma_header2 =  (struct VMA_list *) malloc(sizeof(struct VMA_list));
    memset(vma_header1, 0, sizeof(struct VMA_list));
    memset(vma_header2, 0, sizeof(struct VMA_list));
    
    printf("Enter Process1 pid: ");
    scanf("%d", &pid);
    //----- process1------
    result = malloc(buff_size * sizeof(char));
    memset(result, 0, buff_size*sizeof(char));
    //start = clock();
    //printf("\nIn systemcall\n");
    //syscall(351,pid,result);
    //printf("systemcall finish\n");
    // string_cut(result, vma_header1, 1);  //------pid1_string_cut
    //end = clock();
    //diff = end-start;
    //printf("Proce1_systemcall_time:%.2f\n",diff/CLOCK_PER_SEC);
    
    //start = clock();
    printf("%30s%6d ---------\n", "--------- pid: ", pid);
    
    
    for(i=0;i<times;i++)
    { printf("%30s%6d ---------\n", "---------Times: ", i);
        printf("%17s %17s %5s %8s %9s\n","[virt - interval]", "[phys - interval]", "[num]", "[offset]", "[percent]");
        //memset(vma_header1, 0, sizeof(struct VMA_list));
        start = clock();
        syscall(351,pid,result);
        end = clock();
        diff = end - start;
        //printf("Syscall_run_time:%.2f\n",diff/CLOCK_PER_SEC);
        start = clock();
        string_cut(result, vma_header1, 1);
        end = clock();
        diff = end - start;
        //printf("String_cut_run_time:%.2f\n",diff/CLOCK_PER_SEC);
        vma = 0;
        for(struct VMA_list *current = vma_header1; current!=NULL; current = current->next){
            struct phy_addr_list *backup4 = current->phy_addr;
            percent =0;
            if(backup4 == NULL){
                Max = 0;
                Min = 0;
            }
            else{
                Max = backup4->phy_addr;
                Min = backup4->phy_addr;
                while(backup4 != NULL){
                    percent++;
                    if(Max < backup4->phy_addr)
                        Max = backup4->phy_addr;
                    if(Min > backup4->phy_addr)
                        Min = backup4->phy_addr;
                    
                    backup4 = backup4->next;
                }//while
                current->percent = (float) percent / current->offset *100;   //percent
                current->phy_start = Min;
                current->phy_end = Max;
                vma += current->offset;
                phy += percent;
                printf("%08lx-%08lx %08lx-%08lx %5d %7dK %8.2f%%  %s\n", current->start, current->end,current->phy_start,current->phy_end,percent,current->offset*4,current->percent,current->path_name);
            }//else
        }//for
        //sleep(20);
        total=(float)phy/vma *100;
        printf("Total [vma size]:%dK   [phy size]:%dK   [percent]:%3.2f\%\n\n",vma*4,phy*4,total);
        //printf("String_cut_run_time:%.2f\n",diff/CLOCK_PER_SEC);
        if(i==times-1)
            break;
        memory_free(vma_header1);
        vma_header1 =  (struct VMA_list *) malloc(sizeof(struct VMA_list));
        memset(vma_header1, 0, sizeof(struct VMA_list));
        //printf("String_cut_run_time:%.2f\n",diff/CLOCK_PER_SEC);
    }
    //printf("Total vma number:%dK\n",vma*4);
    //end = clock();
    //diff = end-start;
    //printf("Proce1_run_time:%.2f\n",diff/CLOCK_PER_SEC);
    
    start = clock();
    //----- process2---------
    printf("Enter Process2 pid: ");
    scanf("%d", &pid);
    //printf("\nIn systemcall\n");
    syscall(351,pid,result);
    //printf("systemcall finish\n");
    string_cut(result, vma_header2, 2);  //------ pid2_string_cut
    //----- end
    printf("%30s%6d ---------\n", "--------- pid: ", pid);
    printf("%17s %17s %5s %8s %9s\n","[virt - interval]", "[phys - interval]", "[num]", "[offset]", "[percent]");
    vma =0;
    total = 0;
    phy = 0;
    for(struct VMA_list *current = vma_header2; current!=NULL; current = current->next){
        struct phy_addr_list *backup4 = current->phy_addr;
        percent =0;
        if(backup4 == NULL){
            Max = 0;
            Min = 0;}
        else{
            Max = backup4->phy_addr;
            Min = backup4->phy_addr;
            while(backup4 != NULL){
                percent++;
                if(Max < backup4->phy_addr)
                    Max = backup4->phy_addr;
                if(Min > backup4->phy_addr)
                    Min = backup4->phy_addr;
                backup4 = backup4->next;
            }
            vma += current->offset;
            phy += percent;
            current->percent = (float) percent / current->offset *100;
            current->phy_start = Min;
            current->phy_end = Max;
            printf("%08lx-%08lx %08lx-%08lx %5d %7dK %8.2f%%  %s\n", current->start, current->end,current->phy_start,current->phy_end,percent,current->offset*4,current->percent,current->path_name);
        }//else
    }//for
    total=(float)phy/vma *100;
    printf("Total [vma size]:%dK   [phy size]:%dK   [percent]:%3.2f\%\n\n",vma*4,phy*4,total);
    end = clock();
    diff = end-start;
    //printf("Proce2_run_time:%.2f\n",diff/CLOCK_PER_SEC);
    
    start = clock();
    //---------------------last solution-------------------
    for(struct VMA_list *current = vma_header1; current!=NULL; current = current->next){
        struct phy_addr_list *backup3 = current->phy_addr;                                        //避免pid1的值被覆蓋
        while(backup3 != NULL){
            percent++;
            for(struct VMA_list *test = vma_header2; test!=NULL; test = test->next)
            {
                struct phy_addr_list *backup = test->phy_addr;                                //避免pid2的值被覆蓋
                if( (backup3->phy_addr > test->phy_start) && (backup3->phy_addr < test->phy_end))
                {
                    while(backup != NULL){
                        struct cor_phy *backup2 = correspond_phy;         //avoaid 重複碰撞的值放入串列中
                        if(backup3->phy_addr == backup->phy_addr ){
                            while(backup2!=NULL)
                            {
                                if(backup2->phy_addr == test->phy_addr->phy_addr)
                                    goto here;
                                backup2 = backup2->next;
                            }
                            if(cor_current == NULL){
                                cor_current = (struct cor_phy*) malloc(sizeof(struct cor_phy));
                                memset(cor_current, 0, sizeof(struct cor_phy));
                                correspond_phy = cor_current;
                            }
                            else{
                                cor_prev = cor_current;
                                cor_current = (struct cor_phy*) malloc(sizeof(struct cor_phy));
                                memset(cor_current, 0, sizeof(struct cor_phy));
                                cor_prev->next = cor_current;
                            }  //else
                            cor_current->phy_addr = backup3->phy_addr;
                            cor_current->virt_addr1 = backup3->virt_addr;
                            cor_current->virt_addr2 = backup->virt_addr;
                            cor_current->path_name1 = current->path_name;
                            cor_current->path_name2 = test->path_name;
                        here:
                            1;
                        }// if-2
                        backup = backup->next;
                    } //while
                }//if-1
            }//for-2
            backup3 = backup3->next;
        }//while-1
    }//for-1
    
    end = clock();
    diff = end-start;
    //printf("compare_time:%.2f\n",diff/CLOCK_PER_SEC);
    
    printf("\n[Overlap] : \n");
    printf(       "==============================\n");
    int new_line = 1;
    while(correspond_phy != NULL)
    {
        printf("corr_phys : %08lx \n", correspond_phy->phy_addr);
        printf("proc1 virt: %08lx  %s\n",correspond_phy->virt_addr1,correspond_phy->path_name1);
        printf("proc2 virt: %08lx  %s\n",correspond_phy->virt_addr2,correspond_phy->path_name2);
        puts(  "------------------------------");
        correspond_phy=  correspond_phy->next;
        if(new_line%3 == 0)
            puts("");
        new_line++;
    }
    printf("---Overlap end---\n");
    fgetc(stdin);
    return 0;
}

void string_cut (char *result, struct VMA_list *vma_header, int pattern)
{
    char *test2;
    test2 = result;
    char memoryAddress[20];
    unsigned long imemoryAddr1;
    const char *delim = ":";
    const char *tag = "-" ;
    const char *rol = "|";
    const char *s_e = "~";
    const char *sign = "#";
    char *pch,*copy ,*pch1,*code;
    int i;
    int isFirst = 1;
    int position_delim = -1;
    int position_rol = -1;
    int position_s_e = -1;
    int position_sign = -1;
    
    struct VMA_list *vma_prev;
    struct VMA_list *vma_current;
    struct phy_addr_list *phy_prev;
    struct phy_addr_list *phy_current;
    struct cor_phy *cor_prev = NULL;
    struct cor_phy *cor_current = NULL;
    //------pointer define
    correspond_phy =(struct cor_phy*) malloc(sizeof(struct cor_phy));
    memset(correspond_phy,0,sizeof(struct cor_phy));
    position_delim = strcspn(test2, delim);
    position_rol = strcspn(test2, rol);
    position_s_e = strcspn(test2, s_e);
    position_sign = strcspn(test2, sign);
    
    while(position_delim !=0) {
        if(position_s_e == 17){
            if(isFirst){
                vma_current = vma_header;
                isFirst = 0;
            }
            else{
                vma_prev = vma_current;
                vma_current = (struct VMA_list *) malloc(sizeof(struct VMA_list));
                memset(vma_current, 0, sizeof(struct VMA_list));
                vma_prev->next = vma_current;
            }
            phy_current = NULL;
            phy_prev = NULL;
            
            memcpy(memoryAddress, test2, position_s_e);
            memoryAddress[position_s_e] = '\0';
            if((position_delim - position_s_e) == 2)
                test2 = test2 + position_s_e +3 ;
            else if((position_sign-position_s_e) == 1){
                test2 = test2 + position_s_e +2 ;
                vma_current->path_name = "[ anon ]";
            }
            else
                test2 = test2 + position_s_e +1 ;
            
            pch = test2;         //avoid pch point to NULL
            pch = strtok(memoryAddress, "-");
            sscanf(pch,"%lx",&imemoryAddr1);
            vma_current->start = imemoryAddr1;         //vma_start
            pch = strtok(NULL, "-");
            sscanf(pch,"%lx",&imemoryAddr1);
            vma_current->end = imemoryAddr1;         //vma_end
            position_s_e = strcspn(test2, s_e);
            position_delim = strcspn(test2, delim);
            position_rol = strcspn(test2, rol);
            position_sign = strcspn(test2, sign);
            vma_current->offset = (vma_current->end-vma_current->start)/0x1000;
        }
        else{
            //printf("position_delim:%d position_s_e:%d position_rol:%d position_sign:%d\n",position_delim,position_s_e,position_rol,position_sign);
            
            if(position_sign > 0 && position_sign < position_delim)
            {
                vma_current->path_name =(char *)malloc(position_sign * sizeof(char)+1);
                memset(vma_current->path_name,0,position_sign * sizeof(char)+1);
                memcpy(vma_current->path_name,test2,position_sign);
                
                //printf("%s\n",vma_current->path_name);
                if((position_delim-position_sign) == 1)
                    test2 = test2 + position_sign + 2;
                else
                    test2 = test2 + position_sign + 1;
                
                position_s_e = strcspn(test2, s_e);
                position_delim = strcspn(test2, delim);
                position_rol = strcspn(test2, rol);
                position_sign = strcspn(test2, sign);
            }
            
            while(position_s_e != 17){
                memcpy(memoryAddress, test2, position_rol);
                memoryAddress[position_rol] = '\0';
                pch = test2;                             //avoid pch point to NULL
                pch = strtok(memoryAddress, "-");
                sscanf(pch,"%lx",&imemoryAddr1);
                if(phy_current == NULL){
                    phy_current = (struct phy_addr_list *) malloc(sizeof(struct phy_addr_list));
                    memset(phy_current, 0, sizeof(struct phy_addr_list));
                    vma_current->phy_addr = phy_current;
                }
                else{
                    phy_prev = phy_current;
                    phy_current = (struct phy_addr_list *) malloc(sizeof(struct phy_addr_list));
                    memset(phy_current, 0, sizeof(struct phy_addr_list));
                    phy_prev->next = phy_current;
                }
                phy_current->virt_addr = imemoryAddr1;                //vma_virt
                pch = strtok(NULL, "-");
                sscanf(pch,"%lx",&imemoryAddr1);
                phy_current->phy_addr = imemoryAddr1;              //vma_phy
                
                if((position_delim-position_rol) == 1)
                {
                    test2 = test2 + position_rol + 2;
                    // printf("Test:%s\n");
                    position_s_e = strcspn(test2, s_e); 
                    position_delim = strcspn(test2, delim);
                    position_rol = strcspn(test2, rol);
                    position_sign = strcspn(test2, sign);
                    break;
                }
                else{        
                    test2 = test2 + position_rol + 1;
                    if(position_s_e > 0)
                        position_s_e-=18;
                    if(position_sign > 0)
                        position_sign-=18;
                    //position_s_e = strcspn(test2, s_e); 
                    //position_delim = strcspn(test2, delim);
                    position_delim-=18;
                    //position_rol = strcspn(test2, rol);
                }
            }//while
        }                //first_lawyer_else_end
    }   //while_end            
}

void memory_free(struct VMA_list *vma_header1)
{
    struct VMA_list *tmp;
    
    for(struct VMA_list *current = vma_header1; current!=NULL; current = tmp)
    {   
        tmp =current-> next;
        free(current);    
    }
}