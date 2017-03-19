Linux project1
===
* [x] 增加一個System Call void linux_survey_TT(int A, char * B) 到Linux Kernel 中觀察目標的程序的pid。特殊的位址空間，可以儲存所有在Kernel中的資訊，你必須自己規劃設計資訊的存放方式。
* [x] System Call 紀錄該程序的 Virtual Address 的區間。
* [x] System Call 記錄該程序執行當下相對應的 Physcial Address 區間。
* [x] 列出包含Process1的 Virtual Address Space。
* [x] 每次呼叫 linux_survey_TT，列出對應的Physical Address區間。
* [x] 基於這個執行結果，計算出在虛擬記憶體中有配置到實體記憶體的百分比。

#### 執行步驟
1.執行一個 FireFox瀏覽器 (Process1) 在你的Linux (32bit) 並使用ps取得Prosess1 Pid。
2.在開啟6個FireFox瀏覽器並播放Youtube的影片。
3.在執行的期間不可以終止上述任一瀏覽器。
4.寫一個如下程式(test.c)，搜集所有你需要的資訊，在你開啟要求的瀏覽器後執行這支程式。

~~~ c
#include <stdio.h>
#include <unistd.h>
#define  REPEAT_TIME  10
#define  MEMORY_SIZE  700000 

void main() {
  int    i,pid; 
  char   result[MEMORY_SIZE];
        // Definition of other varialbes you need to use in your program
        // Your initialization code
  printf("Input the PID of the process that you want to observe:");
  scanf("%d", &pid);
  for(i=0;i<REPEAT_TIME;i++)
  {
    linux_survey_TT(pid,result);
               // Code to process results contained in rslt[5000] 
    sleep(120);
  }
      // Code to report the final results
}
~~~

## Part2 需求
* [x] 列出Process2的Virtual Address的區間。
* [x] 呼叫 `linux_survey_TT` 列出以上虛擬位置所對應的實體記憶體位址。
* [x] 列出Process1和Process2的虛擬位址重疊的部分。
* [x] 列出Process1和Process2的實體位址重疊的部分。

### 問題與討論

__Kernel Space__

1. 程式`Pud_Index()` 輸出的entry數並未達到預期的值，如:0x08048000 ->正常entry數為32，但丟進函式去算會變成0。

>3.9.8的linux kernel使用4層架構，`Pud_Index()`是只有2個bits，所以值才會得到0，因為值的區間為0～3。

2. `Pud_offset()`並未include<pgtable.h>，卻可以編譯成功，不知道系統include哪一個header?

>經由多次測試後，因為版本(3.13.x)的問題。

3. 計算physical address 利用4層架構去算，不知道是否可以?還有其他方式可以得知嗎?

>kernel有分一層、二層、三層，因課堂的教材是3.9.x，所以最後我們的版本改成3.9.8。
![](https://i.imgur.com/n1FMk9e.png)

4. CR3暫存器的值竟然與mm_struct裡的pgd不一樣，不知道是否程式有錯誤?（Code A）

>printk(KERN_INFO "Original var_cr3 = 0x%08lx, cr3");
asm volatile(
    "movl %%cr3,%%eax\n"
    "movl %%eax %0\n"
    : "=r" (cr3)
    :
    : "%eax"
)

> 當時要抓取Cr3暫存器的值，cr3存放Directory Base address，正常必須要與pgd的值相同，因為cr3存的值是當下執行程式的Directory Base address，所以說我們並未抓取到我們想要process的值，而是抓到系統執行中程式的值。

5. 執行system call程式crash時，如何可以更有效率的擷取錯誤訊息?

> 若系統執行system_call未Crash的話，可以利用Dmesg來讀取kernel執行system_call的資訊，為避免系統Crash老師有建議使用tty的方式去執行及測試程式，較不易造成嚴重的系統損毀。

6.如何把kernel的值存入從user space的result指標?
> Kernel space可用的API並不像User space如此豐富，經由查詢kernel space的API，我們使用`sprintf()`完成做資料串接的方式，最後利用`copy_to_user()`的方式，把result的值正確傳送。

7.若VMA(virtual memory area) 的記憶體位址沒有對應到page frame，如何偵測出來?

> 當時把process中每個VMA都找出，透過`follow_page()`來當作檢驗，該VMA是否有對應的physical page frame，假若回傳的page為NULL，則表示找不到physical page frame，接續就可以做physical address的轉換，呼叫system call才不會有crash的發生。
![Uploading file..._avdiwvjy9]()

### Userspace

1. 從kernel讀出來的result 如何去做有效的字串切割，以及畫面的呈現?

> 經由小組多次的討論後，使用結構的方式來存取每個VMA的資訊串接，好處是可以很清楚地了解到每一個VMA資訊的細節，且印出來的資訊也會較完整，我們利用三個索引符號個別是~、|、:，如:Vma_Start-Vma_End ~Vma_virt-Vma_phy|: [b76b5000-b76bc000~b76ba000-82082000|b76bb000-8546f000|:]，Vma_Start紀錄Vma開始位址，Vma_End紀錄Vma結束位址，Vma_virt是每一個Vma內細項的開頭，Vma_phy是映射到Page frame的位址資訊，將資料傳入user_space後，透過我們設計的`string_cut()`做完整切割。

