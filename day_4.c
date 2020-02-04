#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

int check_rule(int number)
{
    int dig1, dig2;
    int has_double = 0, cur_nuple = 0;
    int i;

    dig1 = number % 10;
    number /= 10;
    for (i = 1; i < 6; i++) {
        dig2 = number % 10;
        number /= 10;
        if (dig2 > dig1)
            return 0;
        if (dig2 == dig1) {
            cur_nuple++;
        } else {
            if (cur_nuple == 1)
                has_double = 1;
            cur_nuple = 0;
        }
        dig1 = dig2;
    }
    return has_double || (cur_nuple==1);
}

int main(int argc, char **argv)
{
    int start, end, i;
    int passwords = 0;

    start = atoi(argv[1]);
    end = atoi(argv[2]);

    printf("check_rule %d %d %d %d\n", check_rule(112233), check_rule(123444), check_rule(111122), check_rule(112222));

    for (i = start; i <= end; i++) {
        passwords += check_rule(i);
    }

    printf("passwords %d\n", passwords);
    
    return 0;
}

