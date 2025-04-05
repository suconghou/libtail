#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define BUFFER_SIZE 4096

typedef void (*on_read_line)(const char *line);
typedef int (*on_task_one)(void);
typedef int (*on_task)(void);

typedef struct tailItem
{
    const char *fname;
    FILE *file;
    char *buf;
    long lines;
    struct stat info;
    struct tailItem *next;
} tailItem;

int is_same_file(struct stat info1, struct stat info2)
{
    if (info1.st_dev == info2.st_dev && info1.st_ino == info2.st_ino)
    {
        return 1;
    }
    return 0;
}

int tail(tailItem *item, on_read_line on_read_line, on_task_one on_task_one)
{
    if (item->file == NULL)
    {
        item->file = fopen(item->fname, "r");
        if (item->file != NULL)
        {
            if (item->lines == -2)
            {
                // 程序后续文件变化，重新打开文件，此时需要从文件头开始
                fseek(item->file, 0, SEEK_SET);
                item->lines = 0;
            }
            else
            {
                // 程序首次启动，跳转到文件末尾差一字节，如果文件末尾有换行，则正好我们忽略这一换行
                // 如果没有换行符，则后续新增时，我们舍去这个半行
                // 如果文件不足1字节，则操作失败，没有影响
                fseek(item->file, -1, SEEK_END);
                item->lines = -1;
            }
            fstat(fileno(item->file), &item->info);
            item->buf[0] = '\0';
        }
    }
    if (item->file == NULL)
    {
        usleep(on_task_one());
        if (item->next != NULL)
        {
            return tail(item->next, on_read_line, on_task_one);
        }
        return 0;
    }
    int is_read_line = 0;
    // 如果之前打开的文件，则上次必然已读取到文件末尾了，只有再次执行fseek或者clearerr，我们才能读取后续append的数据
    // clearerr的作用是使文件错误标志和文件结束标志置为0
    size_t l = strlen(item->buf);
    clearerr(item->file);
    while (fgets(item->buf + l, BUFFER_SIZE - l, item->file) != NULL)
    {
        is_read_line = 1;
        l = strlen(item->buf);
        if (l < 1)
        {
            continue;
        }
        if (item->buf[l - 1] == '\n')
        {
            item->lines++;
            // 因为我们是从SEEK_END -1 开始计算的,item->lines初始值为-1,所以舍去第一行
            if (item->lines > 0)
            {
                if (l > 1)
                {
                    on_read_line(item->buf);
                }
            }
            item->buf[0] = '\0';
            l = 0;
        }
        else
        {
            // 如果未读取到完整行，但是缓存区已满，则只能中断程序
            if (l >= BUFFER_SIZE - 1)
            {
                return 5;
            }
        }
    }
    if (!is_read_line)
    {
        // printf("没有读取到新数据，执行stat\n");
        // 没有读取到新数据，需要判断是否已轮转
        int size = item->info.st_size; // 记录当前文件大小，然后再测查询文件大小，比较是否变化
        struct stat info = {};
        if (stat(item->fname, &info) == 0)
        {
            if (!is_same_file(info, item->info))
            {
                fclose(item->file);
                item->file = NULL;
                item->lines = -2; // 标记后续文件重新打开时，从文件头开始
                item->buf[0] = '\0';
            }
            else
            {
                // printf("还是那个文件\n");
                item->info = info; // 更新文件的信息
                // 仍然是原先的文件,之所以没有读取到新行，可能：文件没有append,或还未遇到换行,或者文件变小了
                if (info.st_size < size)
                {
                    // 文件反而变小了：使用truncate或>操作符，或者使用一个小的文件cp到目标文件
                    fseek(item->file, -1, SEEK_END);
                    item->buf[0] = '\0';
                    item->lines = -1;
                }
            }
        }
        else
        {
            // 文件可能已被删除或移走, 需要看是否释放文件句柄，避免磁盘空间仍被占用
            int cleanup = 0;
            if (fstat(fileno(item->file), &info) == 0)
            {
                // 文件句柄仍然有效，我们需要比较st_nlink，如果文件被删除 st_nlink 可能比之前值小
                cleanup = info.st_nlink < item->info.st_nlink || info.st_nlink == 0;
            }
            else
            {
                // 文件句柄无效，可能已被删除
                cleanup = 1;
            }
            if (cleanup)
            {
                fclose(item->file);
                item->file = NULL;
                item->lines = -2; // 标记后续文件重新打开时，从文件头开始
                item->buf[0] = '\0';
            }
            // 未关闭句柄时，对方如果仍在此句柄写入数据，我们仍能读取到
        }
    }
    else
    {
        usleep(1000);
        // 有读取到新数据，都读取到文件末尾了
    }
    usleep(on_task_one());
    if (item->next != NULL)
    {
        return tail(item->next, on_read_line, on_task_one);
    }
    return 0;
}
