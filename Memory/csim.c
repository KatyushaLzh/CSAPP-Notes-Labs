#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int E, s, S, b, B, v, t;
const int m = 64;
int curtime;
int hit, miss, evic;

struct row {
    int valid, flag, dfn;
    //有效位 标志位 上次更新的时间戳
};//一行

typedef struct row* set;//一组
typedef set* cache;//整个缓存
cache c;

void Cache_init() {
    c = (cache)malloc(sizeof(set) * S);//分配S个组的空间
    for (int i = 0; i < S; i++) {
        c[i] = (set)malloc(sizeof(struct row) * E);//每一组分配E行的空间
        for (int j = 0; j < E; j++) {
            c[i][j].valid = 0;
            c[i][j].flag = c[i][j].dfn = -1;
        }
    }
}

/*
Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>
Options:
  -h         Print this help message.
  -v         Optional verbose flag.
  -s <num>   Number of set index bits.
  -E <num>   Number of lines per set.
  -b <num>   Number of block offset bits.
  -t <file>  Trace file.

Examples:
  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace
  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace
*/

void Help() {
    printf(
"Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
"Options:\n"
"  -h         Print this help message.\n"
"  -v         Optional verbose flag.\n"
"  -s <num>   Number of set index bits.\n"
"  -E <num>   Number of lines per set.\n"
"  -b <num>   Number of block offset bits.\n"
"  -t <file>  Trace file.\n"

"Examples:\n"
"  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
"  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n"
    );
}

void Load(int ad) {
    //m = t + s + b
    // int _b = ad & ((1 << b) - 1);
    int _s = (ad >> b) & ((1 << s) - 1);
    int _t = ad >> (s + b);

    struct row* r = c[_s];
    for (int i = 0; i < E; i++) {
        if ((r + i)->valid && (r + i)->flag == _t) {//缓存命中
            if (v) puts("hit");
            ++hit;
            (r + i)->dfn = curtime;
            return;
        }
    }
    

    struct row* evicp = r;
    for (int i = 1; i < E; i++) {
        if ((r + i)->dfn == -1 || (r + i)->dfn < evicp->dfn) {//优先使用空行
            evicp = r + i;
        }
    }
    
    miss++; 
    printf("miss");
    if (evicp->dfn != -1) {
        evic++;
        if (v) printf(" eviction");
    }//发生了替换
    putchar('\n');
    evicp->dfn = curtime; evicp->valid = 1; evicp -> flag = _t;
    return;
}

void Store(int ad) {
    Load(ad);
}

void Modify(int ad) {
    //m = t + s + b
    // int _b = ad & ((1 << b) - 1);
    int _s = (ad >> b) & ((1 << s) - 1);
    int _t = ad >> (s + b);
    struct row *r = c[_s]; 
    for (int i = 0; i < E; i++) {
        if ((r + i)->valid && (r + i)->flag == _t) {
            if (v) puts("hit");
            hit += 2;
            (r + i)->dfn = curtime;
            return;
        }
    }

    struct row* evicp = r;
    for (int i = 1; i < E; i++) {
        if ((r + i)->dfn == -1 || (r + i)->dfn < evicp->dfn) {
            evicp = r + i;
        }
    }
    
    miss++;  hit++;
    if (v) puts("miss");
    if (evicp->dfn != -1) {
        evic++;
        if (v) printf(" eviction");
    }
    puts(" hit");
    evicp->dfn = curtime; evicp->valid = 1; evicp -> flag = _t;
    return;
}


int main(int argc, char *argv[]){
    int op;
    FILE *fp;

    while ((op = getopt(argc, argv, "hvs:E:b:t:")) != EOF) {
        if (op == 'h') {
            Help();
            return 0;
        }
        if (op == 'v') {
            v = 1;
            continue;
        }
        if (op == 's') {
            s = atoi(optarg);//atoi 在 stdlib.h中，传入一个字符串开头的指针，将其转换为整数
            S = (1 << s);
            continue;
        }
        if (op == 'E') {
            E = atoi(optarg);
            continue;
        }
        if (op == 'b') {
            b = atoi(optarg);
            B = (1 << b);
            continue;
        }
        if (op == 't') {
            fp = fopen(optarg, "r");//文件指针指向参数标明的文件
            continue;
        }
        Help();
        return 0;//其它异常参数符
    }

    Cache_init();//缓冲初始化，动态分配空间

    char opt[5];
    size_t ad;
    int siz;
    while (fscanf(fp, "%s %lx,%d", opt, &ad, &siz) != EOF) {
        ++curtime;
        if (v) {
            printf("%c %lx,%d\n", opt[0], ad, siz);
        }
        if (opt[0] == 'I') continue;
        if (opt[0] == 'L') Load(ad);
        if (opt[0] == 'S') Store(ad);
        if (opt[0] == 'M') Modify(ad);
    }

    printSummary(hit, miss, evic);
    return 0;
}
