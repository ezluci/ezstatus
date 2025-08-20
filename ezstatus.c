#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include "cjson/cJSON.h"
#include "services.h"

const int SECONDS_DELAY = 30;
#define BUFFER_LEN 10000000

char *status_file_path;
char buffer[BUFFER_LEN];

cJSON* get_status_json() {
   int fd = open(status_file_path, O_RDONLY);
   if (fd == -1) {
      perror("open O_RDONLY status.json error");
      if (errno == ENOENT) {
         printf("initializing default status.json...\n");
         return cJSON_CreateObject();
      }
      exit(100);
   }
   
   int read_count;
   if (-1 == (read_count = read(fd, buffer, BUFFER_LEN))) {
      perror("read status.json error");
      close(fd);
      exit(101);
   }

   cJSON *json = cJSON_ParseWithLength(buffer, read_count);
   if (json == NULL) {
      printf("error while parsing status.json - go fix it lol\n");
      close(fd);
      exit(102);
   }

   close(fd);
   return json;
}

void write_status_json(cJSON *json) {
   int fd = open("status.tmp", O_CREAT | O_WRONLY | O_TRUNC, 0644);
   if (fd == -1) {
      perror("open O_WRONLY status.tmp error");
      exit(103);
   }

   char *json_string = cJSON_PrintUnformatted(json);
   write(fd, json_string, strlen(json_string));
   free(json_string);

   close(fd);

   if (-1 == rename("status.tmp", status_file_path)) {
      perror("can't rename status.tmp");
      exit(104);
   }
}

void add_now_to_service(cJSON *json, const char *service_name, time_t time, int status) {
   cJSON *service = cJSON_GetObjectItemCaseSensitive(json, service_name);
   if (service == NULL) {
      service = cJSON_AddArrayToObject(json, service_name);
   }

   cJSON *now = cJSON_CreateObject();
   cJSON_AddNumberToObject(now, "time", time);
   cJSON_AddNumberToObject(now, "status", status);
   cJSON_AddItemToArray(service, now);
}


void update_status(int signum) {
   cJSON *json = get_status_json();
   time_t current_time = time(NULL);

   add_now_to_service(json, "nginx", current_time, get_status_nginx());
   add_now_to_service(json, "ezlucicom", current_time, get_status_ezlucicom());
   add_now_to_service(json, "blog", current_time, get_status_blog());
   add_now_to_service(json, "files", current_time, get_status_files());
   add_now_to_service(json, "ezpaste", current_time, get_status_ezpaste());
   add_now_to_service(json, "pyro_web", current_time, get_status_pyro_web());
   add_now_to_service(json, "pyro_socket", current_time, get_status_pyro_socket());
   add_now_to_service(json, "mc_server", current_time, get_status_mc_server());

   write_status_json(json);
   cJSON_Delete(json);

   alarm(SECONDS_DELAY);
}


int main(int argc, char *argv[]) {
   
   if (argc != 2) {
      printf("usage: ./ezstatus /path/to/status.json\n");
      printf("make sure this location is in the same filesystem as this program!\n");
      return 1;
   }
   status_file_path = argv[1];
   

   // set alarm clock
   
   struct sigaction act;
   act.sa_handler = update_status;
   
   if (-1 == sigaction(SIGALRM, &act, NULL)) {
      perror("sigaction error");
      return 3;
   }
   
   printf("ready!\n");
   
   alarm(1);
   
   // keep the program running
   
   while (1) {}
}