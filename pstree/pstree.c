#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <glib.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

const wchar_t opening = L'┬';
const wchar_t middle = L'├';
const wchar_t ending = L'└';
const wchar_t dash = L'─';
const wchar_t vdash = L'│';

struct proc {
    GNode n;
    int pid;
    wchar_t* comm;
    int ppid;
};

struct proc* rootp;
bool show_pid;
bool pidsort;

GHashTable* proc_table;

wchar_t linebuf[512], *pbuf = linebuf;

bool isdigits(char* s)
{
    while (*s) {
        if (!isdigit(*s++))
            return false;
    }
    return true;
}

int collect_proc()
{
    FILE* f;
    char* buf;
    char path[512];
    struct proc* p;
    int len, pid, ppid;
    struct dirent* dirent;
    DIR* dir = opendir("/proc");

    proc_table = g_hash_table_new(g_int_hash, g_int_equal);
    while ((dirent = readdir(dir))) {
        if (!isdigits(dirent->d_name))
            continue;
        sprintf(path, "/proc/%s/stat", dirent->d_name);
        f = fopen(path, "r");

        p = (struct proc*)calloc(1, sizeof(struct proc));
        fscanf(f, "%d %ms %*c %d", &p->pid, &buf, &p->ppid);
        len = strlen(buf);

        p->comm = (wchar_t*)malloc((len + 1 - 2) * sizeof(wchar_t));
        for (int i = 0; i < len - 2; i++)
            p->comm[i] = buf[i + 1];
        p->comm[len - 2] = 0;

        free(buf);

        g_hash_table_insert(proc_table, &p->pid, p);
        fclose(f);
    }
    closedir(dir);
    return 0;
}

void die(char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
    exit(-1);
}

wchar_t* trim(wchar_t* s)
{
    wchar_t* p;

    wchar_t pat[] = L"└─ ";
    while ((p = wcsstr(linebuf, pat)))
        p[0] = p[1] = L' ';

    wchar_t pat2[] = L"├─ ";
    while ((p = wcsstr(linebuf, pat2)))
        p[1] = L' ';

    wchar_t pat3[] = L"├ ";
    while ((p = wcsstr(linebuf, pat3)))
        p[0] = vdash;

    wchar_t pat4[] = L"─┬─ ";
    while ((p = wcsstr(linebuf, pat4))) {
        p[0] = p[2] = L' ';
        p[1] = vdash;
        while (*p != middle) {
            *p = L' ';
            p--;
        }
        *p = vdash;
    }
    return s;
}

int print_tree(struct proc* p)
{
    int length;
    wchar_t* pstart = pbuf;
    if (show_pid)
        pbuf += length = swprintf(pbuf, 256, L"%ls(%d)", p->comm, p->pid);
    else
        pbuf += length = swprintf(pbuf, 256, L"%ls", p->comm);

    GNode* c = g_node_first_child(&p->n);
    if (c) {
        *pbuf++ = dash;
        if (g_node_next_sibling(c))
            *pbuf++ = opening;
        else
            *pbuf++ = dash;
        *pbuf++ = dash;
        print_tree((struct proc*)c);

        for (int i = 0; i < length + 2; i++)
            pstart[i] = ' ';

        while ((c = g_node_next_sibling(c))) {
            pbuf -= 2;
            if (c == g_node_last_child(&p->n))
                *pbuf++ = ending;
            else
                *pbuf++ = middle;
            *pbuf++ = dash;
            print_tree((struct proc*)c);
        }
    } else {
        trim(linebuf);
        wprintf(L"%ls\n", linebuf);
    }
    pbuf = pstart;
    return 0;
}

int build_tree()
{
    GHashTableIter iter;
    int* pid;
    struct proc* p;

    g_hash_table_iter_init(&iter, proc_table);
    while (g_hash_table_iter_next(&iter, (void**)&pid, (void**)&p)) {
        if (*pid == 1) {
            rootp = p;
            continue;
        }
        int ppid = p->ppid;
        GNode* pn = g_hash_table_lookup(proc_table, &ppid);
        if (!pn)
            continue;
        g_node_append(pn, &p->n);
    }
    return 0;
}

int pidinc(const void* a, const void* b)
{
    return (*(struct proc**)a)->pid - (*(struct proc**)b)->pid;
}

int compinc(const void* a, const void* b)
{
    return wcscmp((*(struct proc**)a)->comm, (*(struct proc**)b)->comm);
}

int sort_tree(GNode* n, int (*comp)(const void*, const void*))
{
    GNode** arr;
    int i = 0, child = g_node_n_children(n);
    if (child) {
        arr = (GNode**)malloc(child * sizeof(GNode*));
        GNode* c = g_node_first_child(n);
        do {
            sort_tree(c, comp);
            arr[i++] = c;
        } while ((c = g_node_next_sibling(c)));

        qsort(arr, child, sizeof(GNode*), comp);
        arr[0]->prev = NULL;
        for (i = 0; i < child - 1; i++) {
            arr[i]->next = arr[i + 1];
            arr[i + 1]->prev = arr[i];
        }
        arr[child - 1]->next = NULL;
        n->children = arr[0];
        free(arr);
    }
    return 0;
}

int main(int argc, const char* argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    argc--;
    argv++;
    while (argc) {
        if (!strcmp(*argv, "-p") || !strcmp(*argv, "--show-pids"))
            show_pid = true;
        else if (!strcmp(*argv, "-n") || !strcmp(*argv, "--numeric-sort"))
            pidsort = true;
        else if (!strcmp(*argv, "-V") || !strcmp(*argv, "--version")) {
            printf("pstree by frezcirno, v1.0\n");
            exit(0);
        } else
            die("Unexpected arguments: %s", *argv);
        argc--;
        argv++;
    }

    collect_proc();
    build_tree();
    sort_tree((GNode*)rootp, pidsort ? pidinc : compinc);
    print_tree(rootp);
    return 0;
}
