#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define VECTOR_SIZE 1024
#define K 3
#define LABEL_SIZE 10 // 10数字 + 26大写字母 + 26小写字母

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
    for (int i = 1; i <= LABEL_SIZE; i++)
    {
        for (int j = 1; j <= 55; j++)
        {
            char file_name[256];
            snprintf(file_name, sizeof(file_name), "bin\\Sample%03d\\img%03d-%03d.txt", i, i, j);

            int matrix[32][32];
            read_matrix(file_name, matrix);

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

int main()
{
    // system("chcp 65001"); // 设置控制台输出编码为UTF-8
    char labels[LABEL_SIZE][55];
    create_labels(labels);

    const char *test_file = "./test/10.txt"; // 请根据实际文件路径修改
    int test_matrix[32][32];
    read_matrix(test_file, test_matrix);

    double vector1[VECTOR_SIZE];
    matrix_to_vector(test_matrix, vector1);

    double distances[LABEL_SIZE * 55];
    get_distances(distances, vector1);

    char result[K];
    get_top_k_labels(distances, labels, result);

    char final_result = most_common_label(result);
    printf("预测结果: %c\n", final_result);

    return 0;
}
