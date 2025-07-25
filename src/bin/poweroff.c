#include <unistd.h>
#include <sys/reboot.h>
#include <stdio.h>
int main() {
  if (reboot(RB_POWER_OFF) < 0) {
    perror("poweroff failed");
    return 1;
  }
  return 0;
}
