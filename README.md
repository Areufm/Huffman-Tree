## 功能需求

设计并实现一个写一个哈夫曼码的编/译码系统，系统功能包括：

（1）I：初始化（Initialization）。

从终端读入字符集大小n，以及n个字符和n个权值，建立哈夫曼树，并将它存于文件hfmTree中；

（2）E：编码（Encoding）。

利用以建好的哈夫曼树（如不在内存，则从文件hfmTree中读入），对文件ToBeTran中的正文进行编码，然后将结果存入文件CodeFile中；

（3）D：译码（Decoding）。

利用已建好的哈夫曼树将文件CodeFile中的代码进行译码，结果存入文件TextFile中；

（4）P：打印代码文件（Print）。

将文件CodeFile以紧凑格式显示在终端上，每行50个代码。同时将此字符形式的编码文件写入文件CodePrint中；

（5）T：印哈夫曼树（Tree printing）。

将已在内存中的哈夫曼树以直观的方式（树或凹入表形式）显示在终端上，同时将此字符形式的哈夫曼树写入文件TreePrint中。

## 界面需求

打印出哈弗曼编码指令的界面，并显示程序进程与错误信息，编码解码与树的界面打印信息。

## 概要设计

![image-20240425134907433](./../../Photos/image-20240425134907433.png)

## 代码实现

### 必要库引入

```c
#include<stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
```



### 宏变量定义

```c
#define POS_X1 35	//打印菜单定位光标
#define POS_X3 50
#define POS_X4 60
#define MAX_LENGTH 300  //哈夫曼树元素编码后的字符串长度
```



### 基本数据结构定义

我们需要定义哈夫曼树节点，每个节点存储对应的元素及权值，同时存储他的左右子节点，由于后面要用到队列指针，同时也需要存储后节点

定义优先队列，后续读取时需要用到

```c
typedef char E;
typedef int T;

//哈夫曼树节点
typedef struct TreeNode {
    E element;  //元素
    T value;    //权值
    struct TreeNode * left;	//左字节点
    struct TreeNode * right;	//右字节点
    struct TreeNode * next; //队列指针
} * Node;

//优先队列
typedef struct Queue {
    Node front, rear;  //构建优先队列
} * LinkedQueue;
```



### 函数声明

后续会用到的所有函数如下

```c
//函数声明
void SetPosition(int x, int y); //定位光标位置
int Menu(void); //打印菜单
int InitQueue(LinkedQueue queue);   //初始化优先队列
int OfferQueue(LinkedQueue queue, T value, E element);  //优先队列入队
Node PollQueue(LinkedQueue queue);  //优先队列出队
Node CreateNode(E element, T value);    //创建哈夫曼树节点
int OfferNode(LinkedQueue queue, Node node);    //哈夫曼树节点入队优先队列
void CreatHfmTree(LinkedQueue queue, int n);    //构造哈夫曼树
int OfferQueueP(LinkedQueue queue, Node root);  //临时优先队列入队
int IsEmpty(LinkedQueue queue); //判断队列是否为空
void LevelOrderToFile(Node root, FILE *f);  //层序遍历哈夫曼树写入文件
Node ReadHfmTree(LinkedQueue queue);    //从文件读取哈夫曼树
char * EnCodeSingle(Node root, E e);  //编码单个字符e
void PrintEncode(Node root, E e);   //将编码字符写入文件
void EnCoding(Node root);   //译码文件
void DeCoding(Node root);   //译码文件
void PrePrint(Node root, int num);  //前序遍历打印哈夫曼树凹入表
void CodeFilePrint(Node root);  //读取编码文件译码后写入文件
```



### 定位光标函数

调用库函数，实现光标定位，打印菜单

```c
void SetPosition(int x, int y) {
    HANDLE hout;
    COORD pos;
    hout = GetStdHandle(STD_OUTPUT_HANDLE);
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(hout, pos);
}
```



### 打印主页面菜单

调用光标位置函数打印具体功能，用户键入不同数字实现不同功能

```c
int Menu(void) {
    int posy = 5;
    int option;
    int i, j;
    SetPosition(POS_X3, posy);
    printf("哈夫曼树编码\n");
    for (i = 0; i < 2; i++) {
        SetPosition(POS_X1, ++posy);
        for (j = 0; j < 55; j++) {
            printf("-");
        }
    }
    SetPosition(POS_X1, ++posy);
    printf("1.构建哈夫曼树");
    SetPosition(POS_X4, posy);
    printf("2.编码单个元素");
    SetPosition(POS_X1, posy += 2);
    printf("3.编码文件");
    SetPosition(POS_X4, posy);
    printf("4.译码文件");
    SetPosition(POS_X1, posy += 2);
    printf("5.打印哈夫曼树");
    SetPosition(POS_X4, posy);
    printf("6.读取哈夫曼树");
    SetPosition(POS_X1, posy += 2);
    printf("7.打印代码文件");
    SetPosition(POS_X4, posy);
    printf("0.退出");
    for (i = 0; i < 2; i++) {
        SetPosition(POS_X1, ++posy);
        for (j = 0; j < 55; j++) {
            printf("-");
        }
    }
    SetPosition(POS_X1, ++posy);
    printf("请选择你的操作[0~7]:[  ]\b\b");
    scanf("%d", &option);
    return option;
}
```



### 哈夫曼树的实现

先放最后封装好的构建函数，接下来我们一步步看如何实现的

首先，用户输入要构建几个节点 `n` ，然后我们使用 `for` 循环持续读取 `n` 个节点信息，将节点信息存储起来。但是考虑到哈夫曼树的性质，我们夫节点绝对是大于子节点的 `value` 的，而我们用户输入的数据并不能保持有序性，可能是大小随机的，如 `7、12、1、4、6` ，所以我们需要考虑将用户输入的数据进行统一的排列，此时就需要用到优先队列，我们可以将用户输入的每个节点都存储起来，构建优先队列，保持从小到大的顺序，如 `1、4、6、7、12` ，这样我们在后续构建哈夫曼树时，可以依次取出最小的节点来构建

`for` 循环完成后我们成功构建出优先队列，此时开始构建哈夫曼树。我们将优先队列中的数值依次取出，构建节点，取出两个小节点后，将两个左右节点的数值相加得到父节点数值，再将父节点存入到优先队列，多次遍历后实现哈夫曼树的构建

```c
//创建哈夫曼树
void CreatHfmTree(LinkedQueue queue, int n) {
    T value;
    E element;
    for (int i = 0; i < n; ++i) {
        printf("请输入节点权值及名称：");
        scanf("%d %c", &value, &element);
        OfferQueue(queue, value, element);
    }
    while (queue->front != NULL && queue->front->next != queue->rear) {   //
        Node left = PollQueue(queue);
        Node right = PollQueue(queue);
        Node node = CreateNode(' ', left->value + right->value);   //
        node->left = left;
        node->right = right;
        OfferNode(queue, node);
    }
    printf("哈夫曼树创建成功！\n");
    getchar();
}
```

接下来我们依次看各个函数的实现：

#### 初始化优先队列

首先我们需要初始化优先队列，传入节点后先开辟内存空间，然后将队列的前指针和后指针都指向自身，置空节点的左右子节点及后节点

```c
//初始化优先队列
int InitQueue(LinkedQueue queue) {
    Node node = (Node)malloc(sizeof(struct TreeNode));
    if(node == NULL) return 0;
    queue->front = queue->rear = node;
    node->left = node->right = NULL;
    node->next = NULL;   //置空
    return 1;
}
```



#### 优先队列入队

然后要考虑怎么将节点插入进来，构建优先队列：

传入节点后，先将节点赋值，然后将节点的左右节点及后节点置空，然后在优先队列中寻找合适的位置，插入到合适顺序，保持优先队列由大到小排列

```c
//进优先队列
int OfferQueue(LinkedQueue queue, T value, E element){
    Node node = (Node)malloc(sizeof(struct TreeNode));
    if(node == NULL) return 0;
    node->element = element;
    node->value = value;
    node->next = NULL;
    node->left = node->right = NULL;
    Node pre = queue->front;
    while (pre->next && pre->next->value <= value)   //插入到合适的位置
        pre = pre->next;
    if(pre == queue->rear) {
        queue->rear->next = node;
        queue->rear = node;
    } else {
        node->next = pre->next;
        pre->next = node;
    }
    return 1;
}
```



#### 优先队列出队

和正常队列一样，取出头节点即可，记得判断是不是最后一个节点，如果是需要将头尾指针对齐

```c
//出队
Node PollQueue(LinkedQueue queue){
    Node node = queue->front->next;
    queue->front->next = queue->front->next->next;  //直接取出
    if(queue->rear == node) queue->rear = queue->front;   //判断是不是最后一个
    return node;
}
```



#### 创建哈夫曼树节点

创建哈夫曼树节点，存储传入的元素及权值，注意将左右节点置空即可

```c
//创建哈夫曼树节点
Node CreateNode(E element, T value){
    Node node = (Node)malloc(sizeof(struct TreeNode));
    node->element = element;
    node->value = value;
    node->left = node->right = NULL;
    return node;
}
```



#### 将哈夫曼树节点入队

将哈夫曼树节点入队，插入到优先队列中即可，有同学可能会问，这为什么不调用之前的优先队列入队函数 `offerQueue(LinkedQueue queue, T value, E element)` 呢？

```c
//控制台输入数据进优先队列
int OfferQueue(LinkedQueue queue, T value, E element){
    Node node = (Node)malloc(sizeof(struct TreeNode));
    if(node == NULL) return 0;
    node->element = element;
    node->value = value;
    node->next = NULL;
    node->left = node->right = NULL;
    
    ...
    ...
}
```

其实认真观察即可发现，在`offerQueue` 函数中，传入的`value` `element` 后，是重新新建了一个节点，如果我们调用这个方法，将无法保存我们在上一步构建的左右子节点关系，所以我们需要重新封装一个函数，传入我们的 `node` 节点进去来进入优先队列

```c
//将哈夫曼树节点入队
int OfferNode(LinkedQueue queue, Node node){
    node->next = NULL;
    Node pre = queue->front;
    while (pre->next && pre->next->value <= node->value)   //
        pre = pre->next;
    if(pre == queue->rear) {
        queue->rear->next = node;
        queue->rear = node;
    } else {
        node->next = pre->next;
        pre->next = node;
    }
    return 1;
}
```



#### 构建哈夫曼树

最后将封装好的函数依次调用即可，整理后如下：

```c
//创建哈夫曼树
void CreatHfmTree(LinkedQueue queue, int n) {
    T value;
    E element;
    for (int i = 0; i < n; ++i) {
        printf("请输入节点权值及名称：");
        scanf("%d %c", &value, &element);
        OfferQueue(queue, value, element);
    }
    while (queue->front != NULL && queue->front->next != queue->rear) {   //
        Node left = PollQueue(queue);
        Node right = PollQueue(queue);
        Node node = CreateNode(' ', left->value + right->value);   //
        node->left = left;
        node->right = right;
        OfferNode(queue, node);
    }
    printf("哈夫曼树创建成功！\n");
    getchar();
}
```



#### 整体思路总结

至此，我们哈夫曼树的构建就算成功实现。我们再来整体复原一下思路：

1. 第一步，我们需要构建优先队列，将用户输入的乱序的信息由小到大存储到优先队列中
2. 第二步，我们每次用优先队列中取出最小的两个节点，将两节点的数值相加，构建出第三个父节点，并将其两节点拼接为左右字节点，再将父节点重新加入优先队列

然后重复第二步，即可完成哈夫曼树的构建



### 将哈夫曼树写入文件 

由于题目要求，如果没有构建哈夫曼树需要支持从文件中提取，所以我们在构建成功后，还需要实现将哈夫曼树写入文件，这样才能在后续实现读取哈夫曼树

考虑到我们的构建哈夫曼树函数是依次取出最小的节点，然后构建父节点的流程，所以我们存储时，可以进行层序遍历哈夫曼树，将哈夫曼树按照层序遍历写入文件。我们同样构建一个优先队列，这里传入的 `root` 节点是已经构建好的哈夫曼树的根节点，我们从根节点开始依次将节点打印到文件中，然后将左右子树分别入队，再依次出队打印到文件中，即可实现将哈夫曼树写入文件的操作

#### 队列入队操作

由于我们并不需要非常复杂的操作，仅仅使用队列节点的入队出队操作即可，所以单独封装出一个对 `queue_p` 使用的入队操作，出队操作一样所以不用更改

```c
//队列入队
int OfferQueueP(LinkedQueue queue, Node root){
    queue->rear->next = root;
    queue->rear = root;
    return 1;
}
```



#### 判断队列是否为空

```c
//判断队列是否为空
int IsEmpty(LinkedQueue queue){
    return queue->front == queue->rear;
}
```



#### 层序遍历哈夫曼树写入文件

```c
//层序遍历写入文件HfmTree
void LevelOrderToFile(Node root, FILE *f){
    struct Queue queue_p;   //层序的队列
    InitQueue(&queue_p);
    OfferQueueP(&queue_p, root);  //入队列
    while (!IsEmpty(&queue_p)) {   //判空
        Node node = PollQueue(&queue_p);   //出队
        fprintf(f, "%c, %d\n", node->element, node->value);
        if(node->left)    //先看左边
            OfferQueueP(&queue_p, node->left);  //入队
        if(node->right){	//再看右边
            OfferQueueP(&queue_p, node->right);
        }
    }
    fclose(f);
}
```



### 读取哈夫曼树

题目需要我们实现如果没有构建哈夫曼树则要能够从文件中读取，所以我们需要编写从文件中读取哈夫曼树的函数。

我们在构建哈夫曼树时已经实现了将其层序遍历存入到文件中，所以直接依次读取每行的 `element` `value` 即可，思路和构造哈夫曼树类似，分别存入优先队列，然后再依次取出两个小节点，构建父节点后再次入队，循环几次直到构建成功。

```c
//从文件HfmTree中读取哈夫曼树
Node ReadHfmTree(LinkedQueue queue) {
    // 打开文件，确保文件名正确
    FILE* file = fopen("D:\\Clion\\Projects\\HfmTree\\HfmTree.txt", "r"); 
    if (file == NULL) {
        printf("无法打开文件\n");
        return NULL;
    }

    char line[50];  // 假设每行不超过50个字符
    while (fgets(line, sizeof(line), file)) {
        // 去掉行末尾的换行符
        line[strcspn(line, "\n")] = '\0';

        // 检查行是否为空
        if (line[0] == ' ') {
            continue;  // 跳过空行
        }

        char element;
        int value;
        sscanf(line, "%c, %d", &element, &value);  // 从每行中提取字符和权重值
        OfferQueue(queue, value, element);  // 调用你的OfferQueue函数
    }

    fclose(file);  // 关闭文件

    //构建哈夫曼树
    while (queue->front != NULL && queue->front->next != queue->rear) {
        Node left = PollQueue(queue);
        Node right = PollQueue(queue);
        Node node = CreateNode(' ', left->value + right->value);   //创建父节点
        node->left = left;
        node->right = right;
        OfferNode(queue, node);
    }

    Node root = PollQueue(queue);
    printf("哈夫曼树读取成功！\n");
    return root;
}
```



### 编码单个元素

哈夫曼树的编码，我们定义向左为0，向右为1，所以每个节点都可以被编码。我们从根节点开始依次递归遍历，如果遍历到要找到的元素，则返回 `""` ，在遍历过程中，向左后拼接0， 向右后拼接1

```c
//编码单个元素e
char * EnCodeSingle(Node root, E e){
    if(root == NULL) return NULL;   //如果节点没0则返回NULL
    if(root->element == e) return "";   //如果找到要求元素则返回""
    char *str = EnCodeSingle(root->left, e);
    char *s = malloc(sizeof (char));	//开辟内存
    if(str != NULL) {
        s[0] = '0';
        str = strcat(s, str);   //向后拼接
    } else {    //如果不是左子树
        str = EnCodeSingle(root->right, e);	//向右遍历
        if(str != NULL) {
            s[0] = '1';
            str = strcat(s, str);   //向后拼接
        }
    }
    return str;   //返回编码
}

//将编码元素写入文件CodeFile
void PrintEncode(Node root, E e){
    printf("%c 的编码为：%s", e, EnCodeSingle(root, e));	//打印到控制台
    FILE *CodeFile = fopen("D:\\Clion\\Projects\\HfmTree\\CodeFile.txt", "a+");
    fprintf(CodeFile, "%s", EnCodeSingle(root, e));
    fclose(CodeFile);
    putchar('\n');
}
```



### 从文件中编码

我们从文件中读取到数据，首先检测文件是否为空，依次读取文件中的字符，然后依次编译元素。将编译好的元素写入到文件 `CodeFile.txt` 后，再将文件内容打印到控制台中，便于直观显示

```c
//从文件ToBeTran中编码元素
void EnCoding(Node root) {
    FILE *ToBeTran = fopen("D:\\Clion\\Projects\\HfmTree\\ToBeTran.txt", "r+");
    FILE *CodeFile = fopen("D:\\Clion\\Projects\\HfmTree\\CodeFile.txt", "w+");
    if (feof(ToBeTran)){
        printf("ToBeTran文件为空!");
        return ;
    }

    char c;
    while (!feof(ToBeTran)) {
        c = fgetc(ToBeTran);
        if (feof(ToBeTran))
            break;
        PrintEncode(root, c);
    }
    fclose(ToBeTran);

    //读取编码后的文件内容
    char buffer[256];
    fgets(buffer, sizeof(buffer), CodeFile);
    printf("编码后文件内容为: %s\n", buffer);
    fclose(CodeFile);
}
```

### 译码文件

由于哈夫曼树的编码在文件中是以连续的长串字符存储的，如100101010011这样，所以我们需要根据一长串字符来译码存储的字符，思路为将字符存储到数组中，我们从根节点开始，如果字符串的字符为0，则将根节点向左走，若为1则向右走，直到没有子树说明单个字符译码完成，读取该字符即可

```c
//译码文件CodeFile字符
void DeCoding(Node root) {
    Node node = root;
    char line[100];
    FILE *CodeFile = fopen("D:\\Clion\\Projects\\HfmTree\\CodeFile.txt", "r");
    fgets(line, 100, CodeFile);
    int len = strlen(line);
    printf("文件CodeFile为：%s\n", line);
    for (int i = 0; i < len; ++i) {
        if(line[i] != '0' && line[i] !='1') {
            printf("文件有误！");
            return;
        }
    }
    printf("从文件CodeFile中编译出字符为：");
    for (int i = 0; i < len; ++i) {
        if(line[i] == '0'){
            node = node->left;
            if(!node->left) {
                printf("%c", node->element);
                node = root;
            }
        }else if (line[i] == '1') {
            node = node->right;
            if(!node->right) {
                printf("%c", node->element);
                node = root;
            }
        }
    }
    printf("\n");
}
```



### 打印哈夫曼树

我们使用凹入表形式来打印哈夫曼树，可以直观的看到哈夫曼树的层级关系

使用前序遍历很容易实现，在进入左右子节点前，根据层级关系打印出-------，层级越低显示的---也就越少，表示为高层的子树

由于我们需要将哈夫曼树以凹入表形式写入文件中，但是我们又利用了递归调用，所以不能使用w+模式来打开文件，因为每次调用函数时都会清空原先内容再重新写入，但是我们使用a模式追写的话，每次运行一遍函数都会在文件内追写一次，我们只需要文件存入最后一次运行的哈夫曼树，所以我们就要考虑在每次运行前清空一次文件来实现功能

我们可以在外部调用此函数时，以w模式打开一次文件，再关闭，就可以实现清空文件的功能，然后再调用 `PrePrint` 功能，即可实现打印哈夫曼树到文件

```c
//前序遍历打印哈夫曼树的凹入表
void PrePrint(Node root, int num){
    if(root == NULL) return;
    //将凹入表形式哈夫曼树写入文件TreePrint
    FILE *TreePrint = fopen("D:\\Clion\\Projects\\HfmTree\\TreePrint.txt", "a");
    for (int i = 0; i < num; ++i) {
        printf("-");
        fprintf(TreePrint, "-");
    }
    printf("%3d %c", root->value, root->element);
    fprintf(TreePrint,"%3d %c", root->value, root->element);
    printf("\n");
    fprintf(TreePrint,"\n");
    PrePrint(root->left, num-8);
    PrePrint(root->right, num-8);
    fclose(TreePrint);
}
```



### 读取文件打印文件

```c
void CodeFilePrint(Node root) {
    char lines[MAX_LENGTH];	//定义存储编码后的字符串
    FILE *CodeFile = fopen("D:\\Clion\\Projects\\HfmTree\\CodeFile.txt", "r");
    FILE *CodePrint = fopen("D:\\Clion\\Projects\\HfmTree\\CodePrint.txt", "w+");

    fgets(lines, MAX_LENGTH, CodeFile);

    printf("文件CodeFile内容为：\n");
    for (int i = 0; i < strlen(lines); ++i) {
        //每行显示50个字符
        if(i % 50 == 0) {
            printf("\n");
        }
        printf("%c", lines[i]);
    }
    printf("\n");

    Node node = root;
    int len = strlen(lines);
    printf("CodeFile文件译码后内容如下：\n");
    for (int i = 0; i < len; ++i) {
        if(lines[i] == '0'){
            node = node->left;
            if(!node->left) {
                printf("%c", node->element);
                fprintf(CodePrint, "%c", node->element);
                node = root;
            }
        }else if (lines[i] == '1') {
            node = node->right;
            if(!node->right) {
                printf("%c", node->element);
                fprintf(CodePrint, "%c", node->element);
                node = root;
            }
        }
    }

    printf("\nCodeFile文件译码后内容已成功写入CodePrint!\n");
    fclose(CodeFile);
    fclose(CodePrint);
}
```



### 主函数

首先初始化队列，定义根节点，根据用户键入不同数字实现不同功能，如果没有构建哈夫曼树则提示选择是否从文件中提取。

```c
int main() {
    int n, ch;
    char element;
    int first = 1;
    struct Queue queue;
    InitQueue(&queue);
    Node root;

    system("mode con cols=130 lines=60");
    system("color 0E");
    while (1) {
        system("cls");
        ch = Menu();
        switch (ch) {
            case 1:
                system("cls");
                printf("请输入节点个数： ");
                scanf("%d", &n);
                CreatHfmTree(&queue, n);
                first = 0;
                root = PollQueue(&queue);
                FILE* f = fopen("D:\\Clion\\Projects\\HfmTree\\HfmTree.txt", "w");
                LevelOrderToFile(root, f);
                system("pause");
                break;
            case 2:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                printf("请输入要编码的元素：");
                getchar();
                scanf("%c", &element);
                PrintEncode(root, element);
                system("pause");
                break;
            case 3:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                EnCoding(root);
                system("pause");
                break;
            case 4:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                DeCoding(root);
                system("pause");
                break;
            case 5:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                FILE *temp = fopen("D:\\Clion\\Projects\\HfmTree\\TreePrint.txt", "w");
                if (temp) fclose(temp);
                printf("哈夫曼树凹入表如下：\n");
                PrePrint(root, 50);
                system("pause");
                break;
            case 6:
                system("cls");
                root = ReadHfmTree(&queue);
                printf("读取哈夫曼树根节点值为: %d\n",root->value);
                first = 0;
                system("pause");
                break;
            case 7:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                CodeFilePrint(root);
                system("pause");
                break;
            case 0:
                system("cls");
                printf("系统已成功退出！\n");
                exit(0);
            default:
                system("cls");
                printf("输入有误，请重新输入！\n");
                system("pause");
        }
    }
    return 0;
}
```



## 源码

```c
#include<stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>

#define POS_X1 35	//打印菜单定位光标
#define POS_X3 50
#define POS_X4 60
#define MAX_LENGTH 300  //哈夫曼树元素编码后的字符串长度

typedef char E;
typedef int T;

//哈夫曼树节点
typedef struct TreeNode {
    E element;  //元素
    T value;    //权值
    struct TreeNode * left;	//左字节点
    struct TreeNode * right;	//右字节点
    struct TreeNode * next; //队列指针
} * Node;

//优先队列
typedef struct Queue {
    Node front, rear;  //构建优先队列
} * LinkedQueue;

//函数声明
void SetPosition(int x, int y); //定位光标位置
int Menu(void); //打印菜单
int InitQueue(LinkedQueue queue);   //初始化优先队列
int OfferQueue(LinkedQueue queue, T value, E element);  //优先队列入队
Node PollQueue(LinkedQueue queue);  //优先队列出队
Node CreateNode(E element, T value);    //创建哈夫曼树节点
int OfferNode(LinkedQueue queue, Node node);    //哈夫曼树节点入队优先队列
void CreatHfmTree(LinkedQueue queue, int n);    //构造哈夫曼树
int OfferQueueP(LinkedQueue queue, Node root);  //临时优先队列入队
int IsEmpty(LinkedQueue queue); //判断队列是否为空
void LevelOrderToFile(Node root);  //层序遍历哈夫曼树写入文件
Node ReadHfmTree(LinkedQueue queue);    //从文件读取哈夫曼树
char * EnCodeSingle(Node root, E e);  //编码单个字符e
void PrintEncode(Node root, E e);   //将编码字符写入文件
void EnCoding(Node root);   //译码文件
void DeCoding(Node root);   //译码文件
void PrePrint(Node root, int num);  //前序遍历打印哈夫曼树凹入表
void CodeFilePrint(Node root);  //读取编码文件译码后写入文件

void SetPosition(int x, int y) {
    HANDLE hout;
    COORD pos;
    hout = GetStdHandle(STD_OUTPUT_HANDLE);
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(hout, pos);
}

int Menu(void) {
    int posy = 5;
    int option;
    int i, j;
    SetPosition(POS_X3, posy);
    printf("哈夫曼树编码\n");
    for (i = 0; i < 2; i++) {
        SetPosition(POS_X1, ++posy);
        for (j = 0; j < 55; j++) {
            printf("-");
        }
    }
    SetPosition(POS_X1, ++posy);
    printf("1.构建哈夫曼树");
    SetPosition(POS_X4, posy);
    printf("2.编码单个元素");
    SetPosition(POS_X1, posy += 2);
    printf("3.编码文件");
    SetPosition(POS_X4, posy);
    printf("4.译码文件");
    SetPosition(POS_X1, posy += 2);
    printf("5.打印哈夫曼树");
    SetPosition(POS_X4, posy);
    printf("6.读取哈夫曼树");
    SetPosition(POS_X1, posy += 2);
    printf("7.打印代码文件");
    SetPosition(POS_X4, posy);
    printf("0.退出");
    for (i = 0; i < 2; i++) {
        SetPosition(POS_X1, ++posy);
        for (j = 0; j < 55; j++) {
            printf("-");
        }
    }
    SetPosition(POS_X1, ++posy);
    printf("请选择你的操作[0~7]:[  ]\b\b");
    scanf("%d", &option);
    return option;
}

//初始化优先队列
int InitQueue(LinkedQueue queue) {
    Node node = (Node)malloc(sizeof(struct TreeNode));
    if(node == NULL) return 0;
    queue->front = queue->rear = node;
    node->left = node->right = NULL;
    node->next = NULL;   //置空
    return 1;
}

//进优先队列
int OfferQueue(LinkedQueue queue, T value, E element){
    Node node = (Node)malloc(sizeof(struct TreeNode));
    if(node == NULL) return 0;
    node->element = element;
    node->value = value;
    node->next = NULL;
    node->left = node->right = NULL;
    Node pre = queue->front;
    while (pre->next && pre->next->value <= value)   //插入到合适的位置
        pre = pre->next;
    if(pre == queue->rear) {
        queue->rear->next = node;
        queue->rear = node;
    } else {
        node->next = pre->next;
        pre->next = node;
    }
    return 1;
}

//出队
Node PollQueue(LinkedQueue queue){
    Node node = queue->front->next;
    queue->front->next = queue->front->next->next;  //直接取出
    if(queue->rear == node) queue->rear = queue->front;   //判断是不是最后一个
    return node;
}

//创建哈夫曼树节点
Node CreateNode(E element, T value){
    Node node = (Node)malloc(sizeof(struct TreeNode));
    node->element = element;
    node->value = value;
    node->left = node->right = NULL;
    return node;
}

//将哈夫曼树节点入队
int OfferNode(LinkedQueue queue, Node node){
    node->next = NULL;
    Node pre = queue->front;
    while (pre->next && pre->next->value <= node->value)   //
        pre = pre->next;
    if(pre == queue->rear) {
        queue->rear->next = node;
        queue->rear = node;
    } else {
        node->next = pre->next;
        pre->next = node;
    }
    return 1;
}

//创建哈夫曼树
void CreatHfmTree(LinkedQueue queue, int n) {
    T value;
    E element;
    for (int i = 0; i < n; ++i) {
        printf("请输入节点权值及名称：");
        scanf("%d %c", &value, &element);
        OfferQueue(queue, value, element);
    }
    while (queue->front != NULL && queue->front->next != queue->rear) {   //
        Node left = PollQueue(queue);
        Node right = PollQueue(queue);
        Node node = CreateNode(' ', left->value + right->value);   //
        node->left = left;
        node->right = right;
        OfferNode(queue, node);
    }
    printf("哈夫曼树创建成功！\n");
    getchar();
}

//队列入队
int OfferQueueP(LinkedQueue queue, Node root){
    queue->rear->next = root;
    queue->rear = root;
    return 1;
}

//判断队列是否为空
int IsEmpty(LinkedQueue queue){
    return queue->front == queue->rear;
}

//层序遍历写入文件HfmTree
void LevelOrderToFile(Node root){
    FILE* f = fopen("D:\\Clion\\Projects\\HfmTree\\HfmTree.txt", "w");
    struct Queue queue_p;   //层序的队列
    InitQueue(&queue_p);
    OfferQueueP(&queue_p, root);  //入队列
    while (!IsEmpty(&queue_p)) {   //判空
        Node node = PollQueue(&queue_p);   //出队
        fprintf(f, "%c, %d\n", node->element, node->value);
        if(node->left)    //先看左边
            OfferQueueP(&queue_p, node->left);  //入队
        if(node->right){	//再看右边
            OfferQueueP(&queue_p, node->right);
        }
    }
    fclose(f);
}

//从文件HfmTree中读取哈夫曼树
Node ReadHfmTree(LinkedQueue queue) {
    // 打开文件，确保文件名正确
    FILE* file = fopen("D:\\Clion\\Projects\\HfmTree\\HfmTree.txt", "r");
    if (file == NULL) {
        printf("无法打开文件\n");
        return NULL;
    }

    char line[50];  // 假设每行不超过50个字符
    while (fgets(line, sizeof(line), file)) {
        // 去掉行末尾的换行符
        line[strcspn(line, "\n")] = '\0';

        // 检查行是否为空
        if (line[0] == ' ') {
            continue;  // 跳过空行
        }

        char element;
        int value;
        sscanf(line, "%c, %d", &element, &value);  // 从每行中提取字符和权重值
        OfferQueue(queue, value, element);  // 调用你的OfferQueue函数
    }

    fclose(file);  // 关闭文件

    //构建哈夫曼树
    while (queue->front != NULL && queue->front->next != queue->rear) {
        Node left = PollQueue(queue);
        Node right = PollQueue(queue);
        Node node = CreateNode(' ', left->value + right->value);   //创建父节点
        node->left = left;
        node->right = right;
        OfferNode(queue, node);
    }

    Node root = PollQueue(queue);
    printf("哈夫曼树读取成功！\n");
    return root;
}

//编码单个元素e
char * EnCodeSingle(Node root, E e){
    if(root == NULL) return NULL;   //如果节点没0则返回NULL
    if(root->element == e) return "";   //如果找到要求元素则返回""
    char *str = EnCodeSingle(root->left, e);
    char *s = malloc(sizeof (char));	//开辟内存
    if(str != NULL) {
        s[0] = '0';
        str = strcat(s, str);   //向后拼接
    } else {    //如果不是左子树
        str = EnCodeSingle(root->right, e);	//向右遍历
        if(str != NULL) {
            s[0] = '1';
            str = strcat(s, str);   //向后拼接
        }
    }
    return str;   //返回编码
}

//将编码元素写入文件CodeFile
void PrintEncode(Node root, E e){
    printf("%c 的编码为：%s", e, EnCodeSingle(root, e));	//打印到控制台
    FILE *CodeFile = fopen("D:\\Clion\\Projects\\HfmTree\\CodeFile.txt", "a+");
    fprintf(CodeFile, "%s", EnCodeSingle(root, e));
    fclose(CodeFile);
    putchar('\n');
}

//从文件ToBeTran中编码元素
void EnCoding(Node root) {
    FILE *ToBeTran = fopen("D:\\Clion\\Projects\\HfmTree\\ToBeTran.txt", "r+");
    FILE *CodeFile = fopen("D:\\Clion\\Projects\\HfmTree\\CodeFile.txt", "w+");
    if (feof(ToBeTran)){
        printf("ToBeTran文件为空!");
        return ;
    }

    char c;
    while (!feof(ToBeTran)) {
        c = fgetc(ToBeTran);
        if (feof(ToBeTran))
            break;
        PrintEncode(root, c);
    }
    fclose(ToBeTran);

    //读取编码后的文件内容
    char buffer[256];
    fgets(buffer, sizeof(buffer), CodeFile);
    printf("编码后文件内容为: %s\n", buffer);
    fclose(CodeFile);
}

//译码文件CodeFile字符
void DeCoding(Node root) {
    Node node = root;
    char line[100];
    FILE *CodeFile = fopen("D:\\Clion\\Projects\\HfmTree\\CodeFile.txt", "r");
    fgets(line, 100, CodeFile);
    int len = strlen(line);
    printf("文件CodeFile为：%s\n", line);
    for (int i = 0; i < len; ++i) {
        if(line[i] != '0' && line[i] !='1') {
            printf("文件有误！");
            return;
        }
    }
    printf("从文件CodeFile中编译出字符为：");
    for (int i = 0; i < len; ++i) {
        if(line[i] == '0'){
            node = node->left;
            if(!node->left) {
                printf("%c", node->element);
                node = root;
            }
        }else if (line[i] == '1') {
            node = node->right;
            if(!node->right) {
                printf("%c", node->element);
                node = root;
            }
        }
    }
    printf("\n");
}

//前序遍历打印哈夫曼树的凹入表
void PrePrint(Node root, int num){
    if(root == NULL) return;
    FILE *TreePrint = fopen("D:\\Clion\\Projects\\HfmTree\\TreePrint.txt", "a");
    for (int i = 0; i < num; ++i) {
        printf("-");
        fprintf(TreePrint, "-");
    }
    printf("%3d %c", root->value, root->element);
    fprintf(TreePrint,"%3d %c", root->value, root->element);
    printf("\n");
    fprintf(TreePrint,"\n");
    PrePrint(root->left, num-8);
    PrePrint(root->right, num-8);
    fclose(TreePrint);
}

void CodeFilePrint(Node root) {
    char lines[MAX_LENGTH];	//定义存储编码后的字符串
    FILE *CodeFile = fopen("D:\\Clion\\Projects\\HfmTree\\CodeFile.txt", "r");
    FILE *CodePrint = fopen("D:\\Clion\\Projects\\HfmTree\\CodePrint.txt", "w+");

    fgets(lines, MAX_LENGTH, CodeFile);

    printf("文件CodeFile内容为：\n");
    for (int i = 0; i < strlen(lines); ++i) {
        //每行显示50个字符
        if(i % 50 == 0) {
            printf("\n");
        }
        printf("%c", lines[i]);
    }
    printf("\n");

    Node node = root;
    int len = strlen(lines);
    printf("CodeFile文件译码后内容如下：\n");
    for (int i = 0; i < len; ++i) {
        if(lines[i] == '0'){
            node = node->left;
            if(!node->left) {
                printf("%c", node->element);
                fprintf(CodePrint, "%c", node->element);
                node = root;
            }
        }else if (lines[i] == '1') {
            node = node->right;
            if(!node->right) {
                printf("%c", node->element);
                fprintf(CodePrint, "%c", node->element);
                node = root;
            }
        }
    }

    printf("\nCodeFile文件译码后内容已成功写入CodePrint!\n");
    fclose(CodeFile);
    fclose(CodePrint);
}

int main() {
    int n, ch;
    char element;
    int first = 1;
    struct Queue queue; //定义优先队列
    InitQueue(&queue);
    Node root;

    system("mode con cols=130 lines=60");
    system("color 0E");

    while (1) {
        system("cls");
        ch = Menu();
        printf("请选择你的操作[0~7]:[  ]\b\b");
        switch (ch) {
            case 1:
                system("cls");
                printf("请输入节点个数： ");
                scanf("%d", &n);
                CreatHfmTree(&queue, n);
                first = 0;
                root = PollQueue(&queue);
                LevelOrderToFile(root);
                system("pause");
                break;
            case 2:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                printf("请输入要编码的元素：");
                getchar();
                scanf("%c", &element);
                PrintEncode(root, element);
                system("pause");
                break;
            case 3:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                EnCoding(root);
                system("pause");
                break;
            case 4:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                DeCoding(root);
                system("pause");
                break;
            case 5:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                FILE *temp = fopen("D:\\Clion\\Projects\\HfmTree\\TreePrint.txt", "w");
                if (temp) fclose(temp);
                printf("哈夫曼树凹入表如下：\n");
                PrePrint(root, 50);
                system("pause");
                break;
            case 6:
                system("cls");
                root = ReadHfmTree(&queue);
                printf("读取哈夫曼树根节点值为: %d\n",root->value);
                first = 0;
                system("pause");
                break;
            case 7:
                system("cls");
                if (first) {
                    printf("哈夫曼树尚未构建！\n");
                    printf("是否从文件中读取？（y/n）\n");
                    getchar();
                    char a;
                    scanf("%c", &a);
                    if(a == 'y') {
                        root = ReadHfmTree(&queue);
                        first = 0;
                    }
                    system("pause");
                    break;
                }
                CodeFilePrint(root);
                system("pause");
                break;
            case 0:
                system("cls");
                printf("系统已成功退出！\n");
                exit(0);
            default:
                system("cls");
                printf("输入有误，请重新输入！\n");
                system("pause");
        }
    }
    return 0;
}
```



## 功能演示

#### 0. 菜单

![image-20240426125223672](./../../Photos/image-20240426125223672.png)

#### 1. 构建哈夫曼树

![image-20240426125358356](./../../Photos/image-20240426125358356.png)

#### 2. 编码单个字符

![image-20240426125529526](./../../Photos/image-20240426125529526.png)

![image-20240426125546644](./../../Photos/image-20240426125546644.png)



#### 3. 编码文件

文件ToBeTran如下：

![image-20240426125637838](./../../Photos/image-20240426125637838.png)

![image-20240426125726600](./../../Photos/image-20240426125726600.png)

已成功写入文件CodeFile：

![image-20240426125806562](./../../Photos/image-20240426125806562.png)

#### 4. 译码文件

![image-20240426125846065](./../../Photos/image-20240426125846065.png)



#### 5. 打印哈夫曼树

![image-20240426125923813](./../../Photos/image-20240426125923813.png)

![image-20240426125958532](./../../Photos/image-20240426125958532.png)



#### 6. 读取哈夫曼树

![image-20240426125506952](./../../Photos/image-20240426125506952.png)

#### 7. 打印代码文件

![image-20240426130032554](./../../Photos/image-20240426130032554.png)

![image-20240426130129064](./../../Photos/image-20240426130129064.png)
