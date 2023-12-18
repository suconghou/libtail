#include "libtail.c"

void print_line(const char *line)
{
    printf("[%s]\n", line);
}

int fn_task_one()
{
    return 50000;
}

int fn_task()
{
    return 50000;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }
    tailItem *head = NULL;
    for (int i = 1; i < argc; i++)
    {
        tailItem *node = (tailItem *)malloc(sizeof(tailItem));
        const char *fname = argv[i];
        node->fname = strdup(fname);
        node->file = NULL;
        node->buf = (char *)malloc(BUFFER_SIZE);
        node->lines = -1;
        struct stat info = {};
        node->info = info;
        node->next = head;
        head = node;
    }

    tailItem *ff = head;
    while (ff != NULL)
    {
        int r = tail(ff, print_line, fn_task_one);
        if (r != 0)
        {
            return r;
        }
        fn_task();
    }
    return 0;
}
