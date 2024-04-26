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