#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>

struct proc
{
  GNode n;
  int pid;
  char *comm;
  int ppid;
};

struct proc *rootp;
bool show_pid;
bool sort;

GHashTable *proc_table;

char linebuf[512], *pbuf = linebuf;

int listproc()
{
  struct DIR *dir = opendir("/proc");
  struct dirent *dirent;

  while (dirent = readdir(dir))
  {
    char path[512] = "/proc/";
    strcat(path, dirent->d_name);
    strcat(path, "/stat");
    int fd = open(path, O_RDONLY);
    struct FILE *f = fdopen(fd, "r");
    int pid, ppid;

    struct proc *p = (struct proc *)malloc(sizeof(struct proc));

    fscanf(f, "%d %ms %*c %d", &p->pid, &p->comm, &p->ppid);
    g_hash_table_insert(proc_table, p->pid, p);
    fclose(f);
    close(fd);
  }
  closedir(dir);
  return 0;
}

void die(char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  vprintf(msg, ap);
  va_end(ap);
  exit(-1);
}

int dfs(struct proc *p)
{
  int length = strlen(p->comm);
  int child = g_node_n_children(&p->n);
  char *pstart = pbuf;
  pbuf += sprintf(pbuf, "%s", p->comm);
  if (child)
  {
    struct GNode *c = g_node_first_child(&p->n);

    pbuf += sprintf(pbuf, "-");
    if (child > 1)
      pbuf += sprintf(pbuf, "+");
    pbuf += sprintf(pbuf, "-");
    dfs((struct proc *)c);

    for (int i = 0; i < length; i++)
      pstart[i] = ' ';

    while (c = g_node_next_sibling(c))
    {
      pbuf -= 2;
      if (c == g_node_last_child(&p->n))
        pbuf += sprintf(pbuf, "\\");
      else
        pbuf += sprintf(pbuf, "-");
      pbuf += sprintf(pbuf, "-");
      dfs((struct proc *)c);
    }
  }
  else
  {
    *pbuf = '\0';
    printf("%s", linebuf);
  }
  return 0;
}

int main(int argc, const char *argv[])
{
  while (argc)
  {
    if (!strcmp(*argv, "-p") || !strcmp(*argv, "--show-pids"))
    {
      show_pid = true;
    }
    else if (!strcmp(*argv, "-n") || !strcmp(*argv, "--numeric-sort"))
    {
      sort = true;
    }
    else if (!strcmp(*argv, "-V") || !strcmp(*argv, "--version"))
    {
      printf("pstree by frezcirno, v1.0\n");
      exit(0);
    }
    else
    {
      die("Unexpected arguments: %s", argv);
    }
    argc--;
    argv++;
  }

  proc_table = g_hash_table_new(g_int_hash, g_int_equal);

  listproc();

  GHashTableIter iter;
  int pid;
  struct proc *p;

  g_hash_table_iter_init(&iter, proc_table);
  while (g_hash_table_iter_next(&iter, &pid, &p))
  {
    int ppid = p->ppid;
    if (ppid == 0)
    {
      rootp = p;
      continue;
    }
    GNode *pn = g_hash_table_lookup(proc_table, ppid);
    g_node_append(pn, (GNode *)p);
  }
  dfs(rootp);
  return 0;
}
