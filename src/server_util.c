#include "../include/server_util.h"
#include <jansson.h>

int callback_create_user(const struct _u_request *request,
                         struct _u_response *response, void *user_data)
{
  json_error_t error;
  json_t * body = ulfius_get_json_body_request(request, &error); 


  const char *username = json_string_value(json_object_get(body, "username"));
  const char *password = json_string_value(json_object_get(body, "password"));

  char cmd[MAX_CMD_LEN];
  snprintf(cmd, MAX_CMD_LEN, "sudo useradd --gid %s -p %s %s\n", GROUP_NAME,
      password, username);
  system(cmd);


  return 0;
}
