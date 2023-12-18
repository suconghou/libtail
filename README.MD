## libtail

tail -f 简单实现

使用单线程实现多个文件的tail 

使用sleep轮询，无需inotify

使用 `>>` 一切正常，文件dev和inode无变化，仅仅是append文件

> 程序正确输出新行

使用`>`, 导致文件变小，文件dev和inode无变化

> 程序读取游标重新指向文件末尾,此后有半行将自动忽略，后续新行正确输出

使用`truncate` 和使用 `>` 效果一样

> 如果截取的文件末尾没有换行符，后续数据第一个换行之前数据舍去，后续新行正确输出

移动文件走，再移动回来， 文件dev和inode无变化， 还是那个文件

> 后续新行正确输出

使用一个新文件 cp 到目标文件， 目标文件 文件dev和inode无变化

> 若新文件比之前小，则效果等同于truncate，游标将重置到文件末尾，如有半行则忽略，后续新行正确输出
>
> 比之前大，则比之前文件大的那一部分字节相当于append到文件，程序在上次游标上继续分析这些字节


使用一个新文件 mv 到目标文件， 目标文件 dev 和inode变化，重新加载

> 重新打开新文件，从文件头开始读取，输出新行




### 错误码

5: 文件行数据太长



```
gcc -Wall -std=gnu17 -flto=auto -static-libstdc++ -static-libgcc --static -Wl,-Bstatic,--gc-sections -O3 -ffunction-sections -fdata-sections main.c -o
 tail
```

