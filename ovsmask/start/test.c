#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
 
 
int main(void)
{
    int fd;
    fd = open("/dev/maskovs",O_RDWR);
    if(fd<0)
    {
        perror("open fail \n");
        return 0;
    }
    close(fd);
    return 0;
}
