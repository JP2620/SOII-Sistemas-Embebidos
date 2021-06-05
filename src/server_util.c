#include "../include/server_util.h"
#include <jansson.h>

/*
Callback que crea usuario y devuelve
@return {
   "id": user_id,
   "username": "myusername",
   created_at: "YYYY-MM-DD hh:mm:ss"
}
*/
int callback_create_user(const struct _u_request *request,
                         struct _u_response *response, 
                         __attribute__((unused)) void *user_data)
{
  int retval;
  json_error_t error;
  json_t *body_res = json_object();
  json_t *body_req = ulfius_get_json_body_request(request, &error);

  const char *username = json_string_value(json_object_get(body_req,
                                                           "username"));
  const char *password = json_string_value(json_object_get(body_req,
                                                           "password"));
  if (username == NULL || password == NULL)
  {
    json_t * res_fail = json_object();
    json_object_set(res_fail, "description", json_string("Bad syntax"));
    ulfius_set_json_body_response(response, MHD_HTTP_BAD_REQUEST, res_fail);
    return U_CALLBACK_IGNORE;
  }

  /* Crea el comando con los datos de la request y lo corre */
  char cmd[MAX_CMD_LEN];
  retval = snprintf(cmd, MAX_CMD_LEN, "sudo useradd --gid %s -p %s %s\n",
                    GROUP_NAME, password, username);
  if (retval < 0)
    return U_CALLBACK_ERROR;
  retval = system(cmd);
  if (retval != 0)
  {
    json_t * res_fail = json_object();
    json_object_set(res_fail, "description",
                                  json_string("Combinación de username y "
                                  "password no válida")); 
    ulfius_set_json_body_response(response, MHD_HTTP_CONFLICT, res_fail);            
    return U_CALLBACK_IGNORE;
  }
  /* Crea el json que va a ir dentro de la response */
  struct passwd *user = getpwnam(username); // Para obtener el user id
  if (user == NULL)
    return U_CALLBACK_ERROR;
  if (ulfius_init_response(response) != U_OK)
    return U_CALLBACK_ERROR;

  uid_t user_id = user->pw_gid; // user id del user recién creado
  json_object_set(body_res, "id", json_integer(user_id));
  json_object_set(body_res, "username", json_string(username));

  time_t now = time(NULL);
  if (now == ((time_t) -1))
    return U_CALLBACK_ERROR;
  char datetime[80];
  strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", localtime(&now));
  json_object_set(body_res, "created_at", json_string(datetime));
  if (ulfius_set_json_body_response(response, MHD_HTTP_OK, body_res) != U_OK)
    return U_CALLBACK_ERROR;

  /* Se envía la response */ 
  return U_CALLBACK_CONTINUE;
}

/* 
Callback que envía usuarios del sistema operativo
@return {
  "data" : [
    {
      "user_id" : 1,
      "username" : "username1"
    }, 
    ...
  ]
}
*/
int callback_get_users(__attribute__((unused)) const struct _u_request *req,
                       struct _u_response *res, 
                       __attribute__((unused)) void *user_data)
{
  json_t *body_res = json_object();
  json_t *user_array = json_array();

  /* Forma array de usuarios */
  char line[1024];
  FILE * f_users = fopen("/etc/passwd", "r");
  if (f_users == NULL)
    return U_CALLBACK_ERROR;
  
  while(fgets(line, sizeof(line), f_users) != NULL)
  {
    char username[MAX_UNAME_LEN + 1];
    char uid[10];
    strncpy(username, strtok(line, ":"), sizeof(username));
    strtok(NULL, ":");
    strncpy(uid, strtok(NULL, ":"), sizeof(uid));
    json_t * info_user = json_object();
    json_object_set(info_user, "user_id", json_integer(atoi(uid)) );
    json_object_set(info_user, "username", json_string(username));
    json_array_append(user_array, info_user);
  }

  json_object_set(body_res, "data", user_array);
  fclose(f_users);
  if (ulfius_set_json_body_response(res, MHD_HTTP_OK, body_res) != U_OK)
    return U_CALLBACK_ERROR;
  return U_CALLBACK_CONTINUE;
}
