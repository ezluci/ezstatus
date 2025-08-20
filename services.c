#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/wait.h>

int systemctl_isactive(const char *service) {
   int fork_pid = fork();
   if (fork_pid == -1) {
      perror("fork failed");
      exit(10);
   }

   if (fork_pid == 0) {
      // child process
      execlp("systemctl", "systemctl", "is-active", service, "--quiet", NULL);
      perror("execlp failed");
      exit(11);
   } else {
      // parent process
      int status;
      waitpid(fork_pid, &status, 0);

      if (WIFEXITED(status)) {
         int exit_code = WEXITSTATUS(status);

         if (exit_code == 0) {
            return 1;
         } else {
            return 0;
         }
      } else {
         printf("forked child did not exit normally, i'll say it's down (%s)\n", service);
         return 0;
      }
   }
}

int get_status_nginx() {
   return systemctl_isactive("nginx");
}

int get_status_ezlucicom() {
   return get_status_nginx();
}

int get_status_blog() {
   return get_status_nginx();
}

int get_status_files() {
   return systemctl_isactive("filebrowser");
}

int get_status_ezpaste() {
   return systemctl_isactive("ezpaste");
}

int get_status_pyro_web() {
   return systemctl_isactive("pyromasters-mp-web");
}

int get_status_pyro_socket() {
   return systemctl_isactive("pyromasters-mp-socket");
}

int get_status_mc_server() {
   return systemctl_isactive("mc-server");
}