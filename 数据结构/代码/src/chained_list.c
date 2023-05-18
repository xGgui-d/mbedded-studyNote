#include <stdio.h>
#include <stdlib.h>

typedef int Data_Type;

typedef struct node
{
    Data_Type data;    // 数据域
    struct node *next; // 后继指针
    /* data */
} Node, *P_Node;

// 初始化
P_Node init_data(Data_Type data)
{
    // 申请头节点
    P_Node head = calloc(1, sizeof(Node));
    // 设置头节点的数据域为0
    head->data = data;
    // next指针指向为空
    head->next = NULL;
    return head;
}

// 头插入数据（在某个节点的后面插入数据）
int add_head_data(P_Node head, P_Node new_node)
{
    // 判断头指针是否为空
    if (head == NULL)
    {
        printf("链表头异常\n");
        return -1;
    }

    new_node->next = head->next;
    head->next = new_node;

    return 0;
}

// 尾插入数据（在某个节点前面插入数据）
int add_tail_data(P_Node head, P_Node new_node)
{
    // 判断头指针是否为空
    if (head == NULL)
    {
        printf("链表头异常\n");
        return -1;
    }
    P_Node tmp = head;
    while (tmp->next != NULL)
    {
        tmp = tmp->next;
    }
    tmp->next = new_node;
    return 0;
}

// 删除指定数据
P_Node del_data(P_Node head, Data_Type data)
{
    // 判断头指针是否为空
    if (head == NULL || head->next == NULL)
    {
        printf("链表头异常\n");
        return NULL;
    }
    // 匹配数据
    P_Node tmp = head;       // 要删除的节点的上一个节点
    P_Node del = head->next; // 要删除的节点
    while (del != NULL)
    {
        if (del->data == data)
        {
            // 返回要删除的节点
            tmp->next = tmp->next->next;
            return del;
        }
        tmp = tmp->next;
        del = del->next;
    }
    return NULL;
}

// 显示链表
int show_data(P_Node head)
{
    // 判断头指针是否为空
    if (head == NULL)
    {
        printf("链表头异常\n");
        return -1;
    }
    P_Node tmp = head->next;

    while (tmp != NULL)
    {
        printf("%d ", tmp->data);
        tmp = tmp->next;
    }
    printf("\n");
    return 0;
}

// 查找链表节点
P_Node search_node(P_Node head, Data_Type data)
{
    // 判断头指针是否为空
    if (head == NULL || head->next == NULL)
    {
        printf("链表头异常");
        return NULL;
    }
    P_Node tmp = head->next;
    while (tmp != NULL)
    {
        if (tmp->data == data)
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

// 把src_data放到tag_data后面
void move_data(P_Node head, Data_Type src_data, Data_Type tag_data)
{
    // 找到需要移动的数据并移除出来
    P_Node src_node = del_data(head, src_data);
    //找到目标位置
    P_Node tag_node = search_node(head, tag_data);
    if(head == NULL || src_node == NULL || tag_node == NULL)
    {
        printf("找不到数据\n");
        return ;
    }
    //把数据插入到目标位置后面
    add_head_data(tag_node, src_node);

}

int main()
{
    P_Node Ctrl = init_data(0); // 定义一个结构体指针
    int i = 0;
    while (i++ != 5)
    {

        printf("请输入要插入的数据: \n");
        Data_Type new_data;
        scanf("%d", &new_data);
        while (!getchar())
            ;

        if (new_data < 0)
        {
            del_data(Ctrl, abs(new_data));
        }
        else
        {
            // 创建一个节点
            P_Node new_node = init_data(new_data);
            add_head_data(Ctrl, new_node); // 在头节点后面插入数据
        }

        show_data(Ctrl);
    }
    //3移到4后面
    move_data(Ctrl,3,4);
    show_data(Ctrl);

    return 0;
}