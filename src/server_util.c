#include "../include/server_util.h"
#include <jansson.h>

int callback_create_user(const struct _u_request *request,
                         struct _u_response *response, void *user_data)
{
  int retval;
  json_error_t error;
  json_t *body_req = ulfius_get_json_body_request(request, &error);

  const char *username = json_string_value(json_object_get(body_req,
                                                           "username"));
  const char *password = json_string_value(json_object_get(body_req,
                                                           "password"));

  /* Crea el comando con los datos de la request y lo corre */
  char cmd[MAX_CMD_LEN];
  retval = snprintf(cmd, MAX_CMD_LEN, "sudo useradd --gid %s -p %s %s\n",
                    GROUP_NAME, password, username);
  if (retval < 0)
    return U_CALLBACK_ERROR;
  retval = system(cmd);
  if (retval != 0)
    return U_CALLBACK_ERROR;

  /* Crea el json que va a ir dentro de la response */
  json_t *body_res = json_object();
  struct passwd *user = getpwnam(username); // Para obtener el user id
  if (user == NULL)
    return U_CALLBACK_ERROR;
  if (ulfius_init_response(response) != U_OK)
    return U_CALLBACK_ERROR;

  uid_t user_id = user->pw_gid; // user id del user recién creado
  json_object_set(body_res, "id", json_integer(user_id));
  json_object_set(body_res, "username", json_string(username));

  time_t now = time(NULL);
  if (now = ((time_t) -1))
    return U_CALLBACK_ERROR;
  char datetime[80];
  strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", localtime(&now));
  json_object_set(body_res, "created_at", json_string(datetime));
  if (ulfius_set_json_body_response(response, MHD_HTTP_OK, body_res) != U_OK)
    return U_CALLBACK_ERROR;

  /* Se envía la response */ 
  return U_CALLBACK_CONTINUE;
}
