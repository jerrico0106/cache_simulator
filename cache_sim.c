#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>

#define CACHE_SIZE_ORIGIN 32768
#define BLOCK_SIZE_ORIGIN 64
#define WAYS_ORIGIN 4

struct block{
    uint64_t tag;
    uint64_t time;
    int if_used;
    int dir;
};

int bit_count(int a){
    a=a-1;
    int out = 0;
    while(a>0){
        a = (a >> 1);
        out++;
    }
    return out;
}

int main(int argc , char* argv[]){
    int op;
    int cache_size = CACHE_SIZE_ORIGIN;
    int block_size = BLOCK_SIZE_ORIGIN;
    int ways = WAYS_ORIGIN;
    char* tracefile_path = NULL;
    while((op = getopt(argc, argv , "c:b:w:t:")) != -1){
        if(op == 'c'){
            cache_size = atoi(optarg);
        }
        else if(op == 'b'){
            block_size = atoi(optarg);
        }
        else if(op == 'w'){
            ways = atoi(optarg);
        }
        else if(op == 't'){
            tracefile_path = optarg;
        }
    }

    int blocks_num = cache_size/block_size;
    int sets_num = blocks_num/ways;
    int tag_len;
    int index_len = bit_count(sets_num);
    int offset_len = bit_count(block_size);
    tag_len = 64-index_len-offset_len;

    struct block** cache = (struct block**)calloc(sets_num , sizeof(struct block*));
    for(int i=0;i<sets_num;i++){
        cache[i] = (struct block*)calloc(ways, sizeof(struct block));
    }

    FILE* fp = fopen(tracefile_path , "r");
    if(fp == NULL){
        perror("open error");
        return 0;
    }

    uint64_t hits_num = 0;
    uint64_t miss_num = 0;
    uint64_t times = 0;
    char c;
    uint64_t addr;
    uint64_t dram_write = 0;

    while(fscanf(fp , " %c %lx" , &c , &addr) == 2){
        times++;
        uint64_t offset;
        uint64_t index;
        uint64_t tag;
        uint64_t mask;
        mask = ((1ULL<<offset_len)-1);
        offset = (addr&mask);
        mask = ((1ULL<<index_len)-1);
        index = (addr>>offset_len)&mask;
        mask = ((1ULL<<tag_len)-1);
        tag = (addr>>(offset_len+index_len))&mask;
        int choose_way = -1;
        uint64_t choose_time = UINT64_MAX;
        int if_dwrite = 0;
        for(int i=0;i<ways;i++){
	        if(cache[index][i].tag == tag && cache[index][i].if_used == 1){
                cache[index][i].time = times;
                hits_num++;
                choose_way = -1;
                if(c == 'W'){
                    cache[index][i].dir = 1;
                }
                if_dwrite = 0;
                break;
            }
            if(cache[index][i].if_used == 0){
                choose_way = i;
                choose_time = 0;
                if_dwrite = 0;
            }
            else if(choose_way == -1 || cache[index][i].time < choose_time){
                if(choose_time != 0){
                    choose_way = i;
                    choose_time = cache[index][i].time;
                    if(cache[index][i].dir == 1){
                        if_dwrite = 1;
                    }
                    else{
                        if_dwrite = 0;
                    }
                }
            }
        }
        if(choose_way != -1){
            miss_num++;
            cache[index][choose_way].time = times;
            cache[index][choose_way].if_used = 1;
            cache[index][choose_way].tag = tag;
            if(c == 'W'){
                cache[index][choose_way].dir = 1;
            }
            else{
                cache[index][choose_way].dir = 0;
            }
        }
        if(if_dwrite){
            dram_write++;
        }
    }
    for(int i=0;i<sets_num;i++){
        for(int j=0;j<ways;j++){
            if(cache[i][j].dir == 1){
                dram_write++;
            }
        }
    }
     printf("cache size:%d block size:%d ways:%d trace file path:%s\n",cache_size,block_size,ways,tracefile_path);
     printf("blocks number:%d sets number:%d tag bit length:%d index bit length:%d offset bit length:%d\n",blocks_num,sets_num,tag_len,    index_len,offset_len);

    printf("hit number: %lu miss number: %lu dram write times: %lu\n",hits_num,miss_num,dram_write);

    for(int i=0;i<sets_num;i++){
        free(cache[i]);
    }

    fclose(fp);
    free(cache);
    return 0;
}
