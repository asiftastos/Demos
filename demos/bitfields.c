#include <stdio.h>

// N = the number of bit to change, range 0-7
#define SET_BIT(BF, N) BF |= ((char)0x01 << N)
#define CLR_BIT(BF, N) BF &= ~((char)0x01 << N)
#define IS_BIT_SET(BF, N) (BF & ((char)0x01 << N))
#define TOGGLE_BIT(BF, N) BF ^= ((char)0x01 << N)

#define LOW 0xf0
#define HIGH 0x0f

void printBits(char b)
{
    printf("[");
    for(int i = 0; i < 8; i++)
    {
        if(IS_BIT_SET(b, i))
        {
            printf("+");
        }
        else
        {
            printf(".");
        }
    }
    printf("]\n");
}

int main(void)
{
    char s = 0;
    //printBits(s);

    SET_BIT(s, 0);
    SET_BIT(s, 2);
    SET_BIT(s, 3);
    SET_BIT(s, 4);
    SET_BIT(s, 7);
    printBits(s);

    printBits(LOW);
    printBits(HIGH);
    char masked = s & HIGH;
    printBits(masked);

    return 0;
}
