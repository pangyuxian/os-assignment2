/*
vm.c
注释:
    本文件解决1.1(1)的问题
    分别用FIFO和LRU实现了TLB
    当使用FIFO时，须注释掉LRU部分代码
*/

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define TLB_SIZE 16

int bin2int(int*,int, int);//将二进制码start-end位转换为int
int search_TLB(int, int*, int*);
void update_TLB(int*, int* , int, int);
int load_memory(char*, char [256][256], int);

int main(int argc, char *argv[])
{
    char* back_store_name = argv[1];
    char* input_address_name = argv[2];

    //读取addresses.txt文件
    FILE *fp;
    if((fp = fopen(input_address_name,"r"))== NULL){
        printf("File read failed: %s", input_address_name);
        return -1;
    }
    int addresses[1500];
    int i = 0;
    while(fscanf(fp, "%d", &addresses[i]) != EOF) {
        i++;
    }
    fclose(fp);
    
    int page_table[256];//通过int数组模拟页表映射，存储效率低，操作方便，速度快
    char memory[256][256];//通过二维数组模拟物理内存，memory[i][j]表示第i帧中的第j个字节
    int firstFrame = 0;
    for(int j = 0; j < 256 ; j++){
        page_table[j]  = -1;//设定这个-1为我们未存取的情况
    }
    //TLB_page_number[i] -->> TLB_frame_number[i]
    int TLB_page_number[TLB_SIZE] = {
        -1,-1,-1,-1,-1, -1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1};//初始化为-1意味着所有位置都是空闲的
    int TLB_frame_number[TLB_SIZE];

    for(int i = 0; i < 1000; i++){
        //获取page和offset
        int temp = addresses[i];
        int count = 0;
        int binary[100];
        for(int l = 0; l < 100; l++){
            binary[l] = 0;
        }
        while(temp != 0){
            binary[count] = temp & 1;
            temp = (temp>>1);
            count++;
        }
        int page_number = 0;
        int off_set = 0;
        page_number = bin2int(binary,8,16);
        for (int m = 0; m < 8; m++)
        {
            off_set = off_set + pow(2,m) * binary[m];
        }

        int frame_number;
        if ((frame_number = search_TLB(page_number, TLB_page_number, TLB_frame_number)) >= 0)
        {
            //TLB hit
            int physical_address = (frame_number << 8) | off_set;
            printf("Virtual address: %u Physical address: %u Value: %d\n",
            addresses[i], physical_address, (int)memory[frame_number][off_set]);
        }
        else
        {
            //TLB miss
            if ((frame_number = page_table[page_number]) == -1)
            {
                // 缺页错误，应该从bin文件加载页面到内存帧

                //将文件加载到内存
                frame_number = load_memory(back_store_name, memory, page_number);
                
                //更新页表
                page_table[page_number] = frame_number;
            }
            //更新TLB
            update_TLB(TLB_page_number, TLB_frame_number, page_number, frame_number);

            unsigned int physical_address = (frame_number << 8) | off_set;
            printf("Virtual address: %u Physical address: %u Value: %d\n",
             addresses[i], physical_address, (int)memory[frame_number][off_set]);
        }

    }

    return 0;
}

int bin2int(int binary[],int start,int end){
    int i=0;
    int ans = 0;
    for ( i = start; i < end; i++)
    {
        ans = ans + pow(2,i-start) * binary[i];
    }
    return ans;
}

//------------------FIFO TLB start--------------------------
int search_TLB(int page_number, int* TLB_page_number, int* TLB_frame_number)
{
    // 因为采用int数组page_table[page]表示页表，可以线性查找
    for(int i = 0; i < TLB_SIZE; i++)
    if(TLB_page_number[i]==page_number)
        return TLB_frame_number[i];// 页表存在，返回frame，TLB hit
    return -1;//不存在， TLB miss
}
void update_TLB(int* TLB_page_number, int* TLB_frame_number, int page_number, int frame_number)
{
    /*使用FIFO更新TLB
    */
    static int TLB_FI = 0;
    // 搜索TLB以找到空闲space
    for(int i = 0; i < TLB_SIZE; i++)
    {
        if (TLB_page_number[i] == -1)
        {
            // TLB还没有满，直接向空闲sapce添加元素
            TLB_page_number[i] = page_number;
            TLB_frame_number[i] = frame_number;
            return;
        }
    }
    // TLB满时，元素应按FIFO方式更新
    TLB_page_number[TLB_FI] = page_number;
    TLB_frame_number[TLB_FI] = frame_number;
    TLB_FI = (TLB_FI + 1)%TLB_SIZE;
    return;
}
//------------------FIFO TLB end----------------------------

/*
//------------------LRU TLB start--------------------------
time_t TLB_LRU_time[TLB_SIZE];
int search_TLB(int page_number, int* TLB_page_number, int* TLB_frame_number)
{
    // 线性搜索TLB以匹配页码
    for(int i = 0;i < TLB_SIZE; i++)
        if(TLB_page_number[i]==page_number)
        {
            // 匹配成功, TLB hit
            time(TLB_LRU_time + i);// 更新最近使用时间
            return TLB_frame_number[i];
        }
    return -1;//匹配失败, TLB miss
}
void update_TLB(int* TLB_page_number, int* TLB_frame_number, int page_number, int frame_number)
{
    for(int i=0;i<TLB_SIZE;i++)
    {
        // 搜索TLB以找到空闲space
        if (TLB_page_number[i] == -1)
        {
            time(TLB_LRU_time + i);//记录最近使用时间
            TLB_page_number[i] = page_number;
            TLB_frame_number[i] = frame_number;
            return;
        }
    }
    // TLB满时，元素应按LRU方式更新 
    time_t least_recently_used_time = TLB_LRU_time[0];
    int least_recently_used_index = 0;
    for(int i = 1;i < TLB_SIZE; i++)
    {
        if(TLB_LRU_time[i]<least_recently_used_time)
        {
            least_recently_used_time = TLB_LRU_time[i];
            least_recently_used_index = i;
        }
    }
    //新添加的元素取代了第一个元素
    TLB_page_number[least_recently_used_index] = page_number;
    TLB_frame_number[least_recently_used_index] = frame_number;
    time(TLB_LRU_time+least_recently_used_index);//记录新添加的元素最近使用的时间
    return;
}
//------------------LRU TLB end----------------------------
*/

int load_memory(char* back_store_name, char physical_memory[256][256], int page_number)
{
    static int first_empty_frame = 0;
    //打开bin文件
    FILE *bin_file;
    if((bin_file = fopen(back_store_name, "rb"))== NULL){
        printf("File read failed: %s", back_store_name);
        return -1;
    }
    fseek(bin_file,256 * page_number,0);//读指针设置为所需的页面
    fread(physical_memory[first_empty_frame],1,256,bin_file);// 将页面加载到空闲的物理内存帧中
    fclose(bin_file);
    return first_empty_frame++;//线性保留下一个自由帧位置
}