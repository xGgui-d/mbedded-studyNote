#include <stdio.h>
#include <stdlib.h>

typedef int Data_Type;
#define DATA_SIZE 10
// 设计一个顺序表结构体
typedef struct table
{
    Data_Type *data; // 堆空间的入口地址
    int size;        // 可以存放的数据量 数组最大的下标
    int last;        // 当前的数据量 数组当前使用的下标
} Table, *P_Table;


P_Table init_data()
{
    P_Table ptr = calloc(1, sizeof(Table));
    // 申请堆空间来作为顺序表的存储空间
    ptr->data = calloc(DATA_SIZE, sizeof(Data_Type));
    ptr->size = DATA_SIZE;
    ptr->last = 0;
    return ptr;
}

int ins_data(P_Table table, Data_Type new_data)
{
    int lastIndex = table->last;
    // 判断要管理的结构体是否为空或为满
    if (table == NULL || lastIndex == table->size)
    {
        printf("当前内存已满\n");
        return -1;
    }

    // 获取新数据的下标
    int index = 0;
    for (; index < lastIndex; index++)
        if (new_data < *(table->data + index))
            break;

    // 把数据插入列表
    while (index <= lastIndex)
    {
        *(table->data + lastIndex + 1) = *(table->data + lastIndex);
        lastIndex--;
    }

    // 把数据存放到顺序表的内存中
    *(table->data + index) = new_data;
    table->last++;
    return index;
}

int del_data(P_Table table, Data_Type old_data)
{
    // 判断要管理的结构体是否为空或为0
    if (table == NULL || table->last == 0)
    {
        printf("当前内存为空\n");
        return -1;
    }

    // 匹配要删除的数据
    int index = 0;
    for (index = 0; index < table->last; index++)
    {
        if (old_data == *(table->data + index))
        {
            while (index <= table->last)
            {
                // 数据向左移动
                *(table->data + index) = *(table->data + index + 1);
                index++;
            }
            table->last--;
        }
    }
    return index;
}

int main()
{
    P_Table Ctrl = init_data(); // 定义一个结构体指针
    printf("Data_Enter: %p Size: %d Last: %d\n", Ctrl->data, Ctrl->size, Ctrl->last);
    while (1)
    {

        printf("请输入要插入的数据: \n");
        Data_Type new_data;
        scanf("%d", &new_data);
        while (!getchar())
            ;

        if (new_data < 0)
            del_data(Ctrl, abs(new_data));
        else
            ins_data(Ctrl, new_data);

        for (int i = 0; i < DATA_SIZE; i++)
            printf("%d ", *(Ctrl->data + i));
    }
    return 0;
}
