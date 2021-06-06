#include "../include/server_util.h"
#include <jansson.h>

/**
  * @brief Callback que crea usuario y devuelve
  * @return {
  *   "id": user_id,
  *   "username": "myusername",
  *   created_at: "YYYY-MM-DD hh:mm:ss"
  *}
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

  uid_t user_id = user->pw_uid; // user id del user recién creado
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
  
  /* Loggeo evento */
  FILE * f_log = fopen(PATH_LOG, "a");
  log_event(f_log, "| [%s] | Usuario %d\n", USER_SERVICE, user_id);
  fclose(f_log);

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
  
  /* Loggeo evento */
  FILE * f_log = fopen(PATH_LOG, "a");
  log_event(f_log, "| [%s] | cantidad de usuarios del SO: %zu\n", USER_SERVICE,
           json_array_size(user_array));
  fclose(f_log);
  return U_CALLBACK_CONTINUE;
}


int callback_ls_goes(__attribute__((unused)) const struct _u_request *request,
                         struct _u_response *response, 
                         __attribute__((unused)) void *user_data)
{
  char path_file[PATH_MAX];
  json_t *body_res = json_object();
  json_t *file_array = json_array();
  int counter = 1;
  DIR *d;
  struct dirent *dir;
  d = opendir(PATH_GOES); // Abre directorio con archivos de GOES
  if (d)
  {
    while ((dir = readdir(d)) != NULL) // Lee archivo a archivo
    {
      // Si es un archivo regular (no directorio ni simlink ni nada raro)
      if (dir->d_type == DT_REG) 
      {

        memset(path_file, 0, sizeof(path_file));
        /* Arma el path para hacer el path del archivo y
         usarlo para conseguir el filesize */
        strncpy(path_file, PATH_GOES, strlen(PATH_GOES) + 1);
        strncat(path_file, dir->d_name, NAME_MAX);
	      struct stat st;
	      if (stat(path_file, &st) == -1)
	      {
	      	perror("stat");
	      	return U_CALLBACK_ERROR;
	      }

        char unidades[5][5] = {"B", "kB", "MB", "GB", "TB"};
        int pos = 0;
        long double resultado;
        long double div = 1024;
        while ( (resultado = st.st_size / div) > 1)
        {
          pos++;
          div *= div;
        }
        printf("%3.1Lf %s\n", resultado * 1000, unidades[pos]);
        printf("file_id: %d, path: %s, size: %ld\n",
              counter, path_file, st.st_size);
        char filesize[20];
        sprintf(filesize, "%3.1Lf %s", resultado * 1000, unidades[pos]);

        /* Arma JSON para el body de response */
        json_t * file = json_object();
        json_object_set(file, "file_id", json_integer(counter));
        json_object_set(file, "link", json_string(path_file));
        json_object_set(file, "filesize", json_string(filesize));
        json_array_append(file_array, file);
        counter++;
      }
    }
    closedir(d);
  }
  json_object_set(body_res, "files", file_array);
  if (ulfius_set_json_body_response(response, MHD_HTTP_OK, body_res) != U_OK)
    return U_CALLBACK_ERROR;
  return U_CALLBACK_CONTINUE;
}

/**
 * @brief Función para loggear eventos
 * @param log stream a donde loggear
 * @param fmt_str format string para armar el mensaje a loggear
 * @return 0 -> OK, -1 -> ERROR
 * */
int log_event(FILE* log, char* fmt_str, ...)
{
	char msg_log[200];
	if (get_datetime(msg_log, 50) == -1) // Consigue datetime
		return -1;

	va_list format_specs;
	va_start(format_specs, fmt_str);

	char string_to_print[128];
	vsnprintf(string_to_print, sizeof(string_to_print), fmt_str, format_specs);
  // Concatena la cadena que quiero printear al datetime
  strncat(msg_log, string_to_print, 200); 
	fputs(msg_log, log);
	va_end(format_specs);
	return 0;
	
}


/**
 * @brief Escribe timestamp en formato DD/MM/YY hh:mm:ss
 * @param buf el buffer sobre donde se va a escribir la timestamp
 * @param max_len tamaño de buf
 * @return 0 -> OK, -1 -> ERROR
 * */
int get_datetime(char* buf, size_t max_len)
{
	time_t now = time(NULL);
	if (now == ((time_t) -1))
		return -1;
	strftime(buf, max_len, "%D %H:%M:%S ", localtime(&now));
	return 0;
}
