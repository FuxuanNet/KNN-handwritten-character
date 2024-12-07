# KNN-Handwritten-Digit-Recognition

> 这是一份大一的 C 语言大作业，利用 KNN 算法实现手写字符识别，同时也提供了 python 的版本

## 项目介绍

> 基于 KNN 算法实现手写字符识别
>
> 数据集包括了字母和数字
>
> 先尝试用 python 实现，然后尝试用 C 语言复现

## 如何运行

1. 将 `数据集.zip` 解压，将 `Img` 文件夹放在代码的同级目录里。
2. 选取一张手写的要预测的图片，放到 test 文件夹里。
3. 更改 `bin.py` 或 `bin.ipynb` 的倒数第二行和倒数第三行，将文件路径改为 test 里自己要预测的图片的图片文件名。
4. 运行 `bin.py` 或 `bin.ipynb`，将图片二值化。

   ```python
     # 导入判断图片并转化
   file_in = "test\\9.png"  # 自定义上传图片路径
   file_out_int = "test\\9.txt"  # 自定义保存 txt 文件路径
   handle_char(file_in, width, height, file_out_int)
   ```
5. 运行 `matrix.py` 或 `matrix.ipynb` 或 `matrix.c`，预测手写数字。

## 文件夹说明

- `bin.py` 数据预处理
- `matrix.py` KNN 算法实现
- `data` 数据集
- `test` 里面存放需要识别的图片

  - 暂时还没有测试集数据。可以考虑在这里装置测试集，找出最佳的 K 值。

## 项目进度

- python 实现

  - [X] 数据预处理
  - [X] KNN 算法实现
  - [X] 手写字符识别
- C 语言复现

  - [X] KNN 算法实现
  - [ ] 测试集实现
  - [X] 手写字符识别
