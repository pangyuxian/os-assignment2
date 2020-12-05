import numpy as np 
import matplotlib.pyplot as plt
result_list=[]
address_count=0
address_list=[]
page_list=[]

def Fibonacci_Recursion_tool(n):
    global address_count
    global address_list
    address_count+=2
    address_list.append(id(n))
    address_list.append(id(Fibonacci_Recursion_tool))
    if n <= 0:
        address_count+=1
        address_list.append(id(0))
        return 0
    elif n == 1:
        address_count+=1
        address_list.append(id(1))
        return 1
    else:
        address_count+=1
        tmp = Fibonacci_Recursion_tool(n - 1) + Fibonacci_Recursion_tool(n - 2)
        address_list.append(id(tmp))
        return tmp


def Fibonacci_Recursion(n):
    global address_count
    global address_list
    for i in range(1, n + 1): 
        address_count+=1
        result_list.append(Fibonacci_Recursion_tool(i))
        address_list.append(id(result_list[-1]))
    return result_list

def Space_array():
    global address_list
    global address_count
    tmp2=[]
    for i in range(90):
        tmp2.append([])
        Time_array()
        for j in range(50):
            tmp2[i].append(2**i*j*8)
            address_count+=1
            address_list.append(id(tmp2[i][j]))
            
def Time_array():
    global address_count
    global address_list
    tmp1=[]
    for i in range(90):
        tmp1.append([])
        for j in range(50):
            tmp1[i].append(j)
    for i in range(50):
        for j in range(90):
            tmp1[j][i]=2**j*i*3
            address_count+=1
            address_list.append(id(tmp1[j][i]))

def main():
    global address_count
    global address_list
    global page_list
    address_count+=2
    #address_list.append(id(Fibonacci_Recursion(15)))
    address_list.append(id(Time_array))
    address_list.append(id(Space_array()))
    address_list=address_list[:10000]
    
    #print(address_list)
    f = open('addresses-locality.txt','a')
    for address in address_list:
        address = address & 0xFFFF
        PageNum = address >>8
        PageNum%=256 
        offset = address & 0xFF
        offset%=256
        address = int(PageNum<<8)+offset
        page_list.append(PageNum)
        #进行简单的页码，偏移量计算，便于读取backing_store
        #print(PageNum)
        #print(offset)
        #print(address)
        f.write(str(address))
        f.write("\n")
    f.close()
    page_list=page_list[:10000]
    x = np.arange(1,10001)
    y = page_list
    fig = plt.figure()
    ax1 = fig.add_subplot(111)
    ax1.set_title('address_local')
    plt.xlabel('times')
    plt.ylabel('address')
    ax1.scatter(x,y,c='r',marker='.')
    plt.legend('x1')
    #plt.show()
    plt.savefig('1.png')
    
main()

