#include <windows.h>
#include <commdlg.h>
#include <shlwapi.h> // 包含 PathRemoveFileSpec 函数的头文件
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define VECTOR_SIZE 1024
#define K 3
#define LABEL_SIZE 10 // 10数字 + 26大写字母 + 26小写字母

// 全局变量，窗口相关
HWND hResultTextBox;
HBITMAP hBackgroundBitmap = NULL; // 背景图片的句柄

// 加载背景图像

void LoadBackgroundImage(HWND hwnd)
{
    // 尝试加载图像文件
    hBackgroundBitmap = (HBITMAP)LoadImage(NULL, "./background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (hBackgroundBitmap == NULL)
    {
        DWORD dwError = GetLastError();
        char errorMsg[256];
        // snprintf(errorMsg, sizeof(errorMsg), "Failed to load background image! Error code: %lu", dwError);
        // MessageBox(hwnd, errorMsg, "Error", MB_OK | MB_ICONERROR);
    }
}

// 绘制背景图像
void DrawBackgroundImage(HWND hwnd, HDC hdc)
{
    if (hBackgroundBitmap != NULL)
    {
        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBackgroundBitmap);

        BITMAP bm;
        GetObject(hBackgroundBitmap, sizeof(bm), &bm);

        // 获取窗口客户区的大小
        RECT rect;
        GetClientRect(hwnd, &rect);

        // 将背景图像绘制到窗口客户区
        StretchBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

        // 清理
        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
    }
}

void create_labels(char labels[LABEL_SIZE][55])
{
    int idx = 0;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 55; j++)
        {
            labels[idx][j] = '0' + i;
        }
        idx++;
    }
    // for (char ch = 'A'; ch <= 'Z'; ch++)
    // {
    //     for (int j = 0; j < 55; j++)
    //     {
    //         labels[idx][j] = ch;
    //     }
    //     idx++;
    // }
    // for (char ch = 'a'; ch <= 'z'; ch++)
    // {
    //     for (int j = 0; j < 55; j++)
    //     {
    //         labels[idx][j] = ch;
    //     }
    //     idx++;
    // }
}

void read_matrix(const char *file_name, int matrix[32][32])
{
    printf("read_matrix 函数 Opening file: %s\n", file_name); // 打印文件路径用于调试
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    char line[33]; // 读取每行32个字符 + 1个空字符
    int row = 0;

    while (fgets(line, sizeof(line), file) != NULL && row < 32)
    {
        for (int col = 0; col < 32; col++)
        {
            if (line[col] == '0')
            {
                matrix[row][col] = 0;
            }
            else if (line[col] == '1')
            {
                matrix[row][col] = 1;
            }
            else
            {
                matrix[row][col] = 0; // 默认值，如果遇到非0/1字符
            }
        }
        row++;
    }

    fclose(file);
}

void matrix_to_vector(int matrix[32][32], double vector[VECTOR_SIZE])
{
    int idx = 0;
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            vector[idx++] = (double)matrix[i][j];
        }
    }
    // for (int i = 0; i < 32; i++)
    // {
    //     for (int j = 0; j < 32; j++)
    //     {
    //         printf("矩阵%d ", matrix[i][j]);
    //     }
    //     printf("\n");
    // }
}

double calculate_distance(double vector1[VECTOR_SIZE], double vector2[VECTOR_SIZE])
{
    double distance = 0.0;
    for (int i = 0; i < VECTOR_SIZE; i++)
    {
        // double diff = vector1[i] - vector2[i];
        // printf("Difference at index %d: %.2f\n", i, diff); // 打印差值
        distance += pow(vector1[i] - vector2[i], 2);
    }
    return sqrt(distance);
}

void get_distances(double distances[], double vector1[VECTOR_SIZE])
{
    char working_directory[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, working_directory) == 0)
    {
        printf("get_distances 函数 Failed to get current directory\n");
        exit(1);
    }
    // 退回到上一级目录
    PathRemoveFileSpec(working_directory); // 去掉最后的文件夹部分，回到上一级目录

    for (int i = 1; i <= LABEL_SIZE; i++)
    {
        for (int j = 1; j <= 55; j++)
        {
            char file_name[256];
            snprintf(file_name, sizeof(file_name), "%s\\bin\\Sample%03d\\img%03d-%03d.txt", working_directory, i, i, j); // 完整路径

            printf("get_distances 函数 Opening file: %s\n", file_name); // 打印完整路径调试

            int matrix[32][32];
            read_matrix(file_name, matrix); // 读取训练集文件

            double vector2[VECTOR_SIZE];
            matrix_to_vector(matrix, vector2);

            distances[(i - 1) * 55 + (j - 1)] = calculate_distance(vector1, vector2);
        }
    }
}

int compare(const void *a, const void *b)
{
    return (*(double *)a - *(double *)b);
}

void get_top_k_labels(double distances[], char labels[LABEL_SIZE][55], char result[K])
{
    typedef struct
    {
        double distance;
        int index;
    } DistanceIndex;

    DistanceIndex distance_indices[LABEL_SIZE * 55];

    for (int i = 0; i < LABEL_SIZE * 55; i++)
    {
        distance_indices[i].distance = distances[i];
        distance_indices[i].index = i;
    }

    qsort(distance_indices, LABEL_SIZE * 55, sizeof(DistanceIndex), compare);

    // 输出前 10 个最小距离的数字和标签
    printf("距离从近到远的前10个数字及标签:\n");
    for (int i = 0; i < 10; i++)
    {
        int idx = distance_indices[i].index;
        int label_idx = idx / 55; // 获取标签的索引
        if (label_idx >= 0 && label_idx < LABEL_SIZE)
        {
            printf("距离: %.2f, 标签: %c\n", distance_indices[i].distance, labels[label_idx][0]);
        }
    }

    // 获取 top K 个标签
    for (int i = 0; i < K; i++)
    {
        int idx = distance_indices[i].index;
        int label_idx = idx / 55;
        if (label_idx >= 0 && label_idx < LABEL_SIZE)
        {
            result[i] = labels[label_idx][0]; // 取标签的第一个字符
        }
        else
        {
            printf("Invalid label index: %d\n", label_idx);
            result[i] = '?'; // 如果出现非法索引，使用 '?' 标记
        }
    }
}

char most_common_label(char top_K_labels[K])
{
    int count[256] = {0};
    for (int i = 0; i < K; i++)
    {
        count[(int)top_K_labels[i]]++;
    }

    int max_count = 0;
    char most_common = top_K_labels[0];
    for (int i = 0; i < K; i++)
    {
        if (count[(int)top_K_labels[i]] > max_count)
        {
            max_count = count[(int)top_K_labels[i]];
            most_common = top_K_labels[i];
        }
    }
    return most_common;
}

// 文件选择对话框
void OpenFileDialog(HWND hwnd)
{
    OPENFILENAME ofn;
    char szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        SetWindowText(hResultTextBox, "Processing...");
        // 调用主要预测代码
        printf("OpenFileDialog 函数 Selected file: %s\n", szFile); // 打印选中的测试集文件路径调试
        int test_matrix[32][32];
        read_matrix(szFile, test_matrix); // 这里传递的是用户选择的绝对路径

        double vector1[VECTOR_SIZE];
        matrix_to_vector(test_matrix, vector1);

        double distances[LABEL_SIZE * 55];
        get_distances(distances, vector1);

        char result[K];
        char labels[LABEL_SIZE][55];
        create_labels(labels);
        get_top_k_labels(distances, labels, result);

        char final_result = most_common_label(result);
        char result_str[100];
        sprintf(result_str, "Prediction: %c", final_result);

        SetWindowText(hResultTextBox, result_str);
    }
}

// 窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
    {
        if (LOWORD(wParam) == 1)
        {
            OpenFileDialog(hwnd);
        }
        break;
    }
    case WM_CREATE:
    {
        CreateWindow("BUTTON", "Select File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 50, 50, 200, 40, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        hResultTextBox = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY, 50, 100, 400, 200, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        LoadBackgroundImage(hwnd); // 加载背景图像
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 绘制背景图像
        DrawBackgroundImage(hwnd, hdc);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
// int main()
// {
//     // system("chcp 65001"); // 设置控制台输出编码为UTF-8
//     char labels[LABEL_SIZE][55];
//     create_labels(labels);

//     const char *test_file = "./test/10.txt"; // 请根据实际文件路径修改
//     int test_matrix[32][32];
//     read_matrix(test_file, test_matrix);

//     double vector1[VECTOR_SIZE];
//     matrix_to_vector(test_matrix, vector1);

//     double distances[LABEL_SIZE * 55];
//     get_distances(distances, vector1);

//     char result[K];
//     get_top_k_labels(distances, labels, result);

//     char final_result = most_common_label(result);
//     printf("预测结果: %c\n", final_result);

//     return 0;
// }

// 主程序
int main()
{
    system("chcp 65001"); // 设置控制台输出编码为UTF-8
    // 获取当前程序实例
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // 初始化WNDCLASS结构体
    WNDCLASS wc = {0};

    wc.lpfnWndProc = WindowProc;      // 窗口处理函数
    wc.hInstance = hInstance;         // 当前实例
    wc.lpszClassName = "KNNAppClass"; // 窗口类名

    // 注册窗口类
    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    // 创建窗口
    HWND hwnd = CreateWindowEx(
        0,                                // 扩展样式
        "KNNAppClass",                    // 类名
        "KNN Prediction App",             // 窗口标题
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // 窗口样式
        CW_USEDEFAULT, CW_USEDEFAULT,     // 默认位置
        500, 400,                         // 窗口大小
        NULL,                             // 父窗口
        NULL,                             // 菜单
        hInstance,                        // 实例句柄
        NULL                              // 附加数据
    );

    // 如果窗口创建失败
    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    // 进入消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // 翻译消息
        TranslateMessage(&msg);
        // 分发消息
        DispatchMessage(&msg);
    }

    // 返回退出代码
    return (int)msg.wParam;
}
