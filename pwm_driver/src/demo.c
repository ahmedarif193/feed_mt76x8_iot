#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sooall_pwm.h"

#define PWM_DEV "/dev/mt7628-pwm"

int main(int argc, char **argv) {
    int pwm_fd;
    int cnt = 0;
    struct pwm_cfg cfg;

    pwm_fd = open(PWM_DEV, O_RDWR);
    if (pwm_fd < 0) {
        perror("Failed to open pwm device");
        return 1;
    }

    cfg.no = 0;                // pwm0
    cfg.clksrc = PWM_CLK_40KHZ; 
    cfg.clkdiv = PWM_CLK_DIV2;
    cfg.idelval = 0;  
    cfg.guardval = 0;
    cfg.guarddur = 0; 
    cfg.wavenum = 0;           // forever loop
    cfg.datawidth = 1000;
    cfg.threshold = 500; 

    ioctl(pwm_fd, PWM_CONFIGURE, &cfg);   
    ioctl(pwm_fd, PWM_ENABLE, &cfg);

    while (cnt < 10) {
        sleep(5);
        ioctl(pwm_fd, PWM_GETSNDNUM, &cfg);
        printf("send wave num = %d\n", cfg.wavenum);
        cnt++;
    }

    ioctl(pwm_fd, PWM_DISABLE, &cfg);
    close(pwm_fd);
    
    return 0;
}
