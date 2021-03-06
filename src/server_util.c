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
    json_t *res_fail = json_object();
    json_object_set(res_fail, "description", json_string("Bad syntax"));
    ulfius_set_json_body_response(response, MHD_HTTP_BAD_REQUEST, res_fail);
    return U_CALLBACK_IGNORE;
  }

  /* Crea el comando con los datos de la request y lo corre */
  char cmd[MAX_CMD_LEN];
  retval = snprintf(cmd, MAX_CMD_LEN, "sudo useradd --gid %s -p $(openssl passwd -6 %s) %s -m\n",
                    GROUP_NAME, password, username);
  if (retval < 0)
    return U_CALLBACK_ERROR;
  retval = system(cmd);
  if (retval != 0)
  {
    json_t *res_fail = json_object();
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
  if (now == ((time_t)-1))
    return U_CALLBACK_ERROR;
  char datetime[80];
  strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", localtime(&now));
  json_object_set(body_res, "created_at", json_string(datetime));
  if (ulfius_set_json_body_response(response, MHD_HTTP_OK, body_res) != U_OK)
    return U_CALLBACK_ERROR;

  /* Loggeo evento */
  FILE *f_log = fopen(PATH_LOG, "a");
  if (f_log == NULL)
  {
    perror("f_log");
    fclose(f_log);
    return U_CALLBACK_ERROR;
  }
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
  FILE *f_users = fopen("/etc/passwd", "r");
  if (f_users == NULL)
  {
    perror("fopen");
    return U_CALLBACK_ERROR;
  }
  while (fgets(line, sizeof(line), f_users) != NULL)
  {
    char username[MAX_UNAME_LEN + 1];
    char uid[10];
    strncpy(username, strtok(line, ":"), sizeof(username));
    strtok(NULL, ":");
    strncpy(uid, strtok(NULL, ":"), sizeof(uid));
    json_t *info_user = json_object();
    json_object_set(info_user, "user_id", json_integer(atoi(uid)));
    json_object_set(info_user, "username", json_string(username));
    json_array_append(user_array, info_user);
  }
  json_object_set(body_res, "data", user_array);
  fclose(f_users);
  if (ulfius_set_json_body_response(res, MHD_HTTP_OK, body_res) != U_OK)
    return U_CALLBACK_ERROR;
  /* Loggeo evento */
  FILE *f_log = fopen(PATH_LOG, "a");
  if (f_log == NULL)
  {
    perror("f_log");
    fclose(f_log);
    return U_CALLBACK_ERROR;
  }
  log_event(f_log, "| [%s] | cantidad de usuarios del SO: %zu\n", USER_SERVICE,
            json_array_size(user_array));
  fclose(f_log);
  return U_CALLBACK_CONTINUE;
}

/**
 * @brief Callback que lista archivos
 * @return {
 * "files" : [
 * {
 *    "file_id" : 1,
 *    "link"    : blablabla,
 *    "size"    : 321 kB 
 * },
 * ...
 * ]
 * }
 * */
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
        /* Consigue file size */
        struct stat st;
        if (stat(path_file, &st) == -1)
        {
          perror("stat");
          return U_CALLBACK_ERROR;
        }
        char filesize[20];
        int retval = bytes_to_human_readable(filesize, 20, st.st_size);
        if (retval == -1)
        {
          perror("bytes_to_human_readable");
          return U_CALLBACK_ERROR;
        }

        /* Arma respuesta */
        json_t *file = json_object();
        json_object_set(file, "file_id", json_integer(counter));
        char link_descarga[500];
        sprintf(link_descarga, "%s/%s", DOMAIN_NAME, dir->d_name);
        json_object_set(file, "link", json_string(link_descarga));
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

  FILE *f_log = fopen(PATH_LOG, "a");
  if (f_log == NULL)
  {
    perror("fopen al loggear\n");
    return U_CALLBACK_CONTINUE;
  }
  log_event(f_log, "| [%s] | Archivos en el servidor: %zu\n", GOES_SERVICE,
            json_array_size(file_array));
  fclose(f_log);
  return U_CALLBACK_CONTINUE;
}

/**
 * Acepta
 * {
 *  "product": {{PRODUCT}},
 *  "datetime": YYYY/DDD/HH 
 * }
 * Con DD siendo el día juliano
 * */
int callback_find_goes(const struct _u_request *request,
                       struct _u_response *response,
                       __attribute__((unused)) void *user_data)
{
  json_error_t error;
  int retval;
  json_t *body_res = json_object();
  json_t *body_req = ulfius_get_json_body_request(request, &error);
  const char *product = json_string_value(json_object_get(body_req, "product"));
  const char *datetime = json_string_value(json_object_get(body_req, "datetime"));
  if (product == NULL || datetime == NULL) // bad request
    goto fail;

  char datetime_to_tok[30];
  strncpy(datetime_to_tok, datetime, sizeof(datetime_to_tok));
  char *year = strtok(datetime_to_tok, "/");
  char *day = strtok(NULL, "/");
  char *hour = strtok(NULL, "/");

  if (year == NULL || day == NULL || hour == NULL)
    goto fail;

  /* datetime abreviado de comienzo de la medición */
  char goesdatetime[90];
  if (snprintf(goesdatetime, 90, "s%s%s%s", year, day, hour) < 0)
  {
    perror("snprintf\n");
    goto fail;
  }

  DIR *d;
  struct dirent *dir;
  d = opendir(PATH_GOES); // Abre directorio con archivos de GOES
  if (d == NULL)
  {
    perror("opendir");
    return U_CALLBACK_ERROR;
  }

  while ((dir = readdir(d)) != NULL) // Lee archivo a archivo
  {
    /* Si en el nombre del archivo esta el producto y fecha, lo tengo */
    if (strstr(dir->d_name, product) != NULL 
        && strstr(dir->d_name, goesdatetime) != NULL)
    {
      if (dir->d_type != DT_REG)
        continue;

      char path_file[PATH_MAX];
      strcpy(path_file, PATH_GOES);
      strcat(path_file, dir->d_name);
      struct stat st;
      if (stat(path_file, &st) == -1)
      {
        perror("stat");
        return U_CALLBACK_ERROR;
      }
      char filesize[20];
      if (bytes_to_human_readable(filesize, 20, st.st_size) == -1)
      {
        fprintf(stderr, "bytes_to_human_readable");
        return U_CALLBACK_ERROR;
      }
      
      char link_descarga[1000];
      sprintf(link_descarga, "%s/%s", DOMAIN_NAME, dir->d_name);
      json_object_set(body_res, "link", json_string(link_descarga));
      json_object_set(body_res, "filesize", json_string(filesize));
      if (ulfius_set_json_body_response(response, MHD_HTTP_OK, body_res) != U_OK)
        return U_CALLBACK_ERROR;

      /* loggeo del evento */
      FILE *f_log = fopen(PATH_LOG, "a");
      if (f_log == NULL)
      {
        perror("fopen al loggear\n");
        return U_CALLBACK_CONTINUE;
      }
      log_event(f_log, "| [%s] | Archivo solicitado: %s\n",
       GOES_SERVICE, dir->d_name);
      fclose(f_log);
      return U_CALLBACK_CONTINUE;
    }
  }

  /* Si no se encuentra el archivo, se notifica al usuario 
  y en segundo plano se descarga el archivo */

  pid_t pid = fork();
  if (pid != 0) // El padre notifica
  {
    json_object_set(body_res, "description", json_string("File not found,"
                              " try again later"));
    ulfius_set_json_body_response(response, MHD_HTTP_NOT_FOUND, body_res);
    return U_CALLBACK_IGNORE;
  }
  else if (pid == 0) // El hijo descarga
  {
    /* Arma el comando para listar 
    product es por ejemplo OR_ABI-L2-MCMIPF y la carpeta en aws
    es ABI-L2-MCMIPF
    */
    char aws_ls_cmd[MAX_CMD_LEN];
    char product_to_tok[50];
    char product_folder[50];
    char filename[255];
    strncpy(product_to_tok, product, sizeof(product_to_tok));
    strtok(product_to_tok, "_"); // Chau OR_
    strcpy(product_folder, strtok(NULL, "_"));

    sprintf(aws_ls_cmd, "aws s3 --no-sign-request ls "
      "noaa-goes16/%s/%s/%s/%s/ 2>/dev/null", product_folder, year, day, hour);
    FILE * ls_out = popen(aws_ls_cmd, "r");
    if (ls_out == NULL)
    {
      fprintf(stderr, "Memoria insuficiente, popen\n");
      exit(EXIT_FAILURE);
    }
    /* extraemos el nombre del primer archivo de la lista */
    retval = fscanf(ls_out, "%*s %*s %*s %s\n", filename);
    if (retval < 0)
    {
      perror("fscanf ");
      pclose(ls_out);
      goto fail;
    }
    retval = pclose(ls_out); 
    if (retval == -1)
    {
      perror("pclose: ");
      return U_CALLBACK_CONTINUE;
    }


    /* Descarga en paralelo el primer archivo de la lista*/
    char aws_cp_cmd[MAX_CMD_LEN];
    sprintf(aws_cp_cmd, "aws s3 --no-sign-request cp "
            "s3://noaa-goes16/%s/%s/%s/%s/%s %s",
            product_folder, year, day, hour, filename, PATH_GOES);
    system(aws_cp_cmd);
    FILE * f_log = fopen(PATH_LOG, "a");
    log_event(f_log, "| [%s] | Archivo descargado: %s\n",
              GOES_SERVICE, filename);
    //json_object_set(body_res, "description", json_string("File ready"));
    //ulfius_set_json_body_response(response, MHD_HTTP_OK, body_res);
    fclose(f_log);
    exit(EXIT_SUCCESS);
    //return U_CALLBACK_COMPLETE;
  }





  if (false) // Solo se entra aca con goto
  {
  fail:;
    json_t *res_fail = json_object();
    json_object_set(res_fail, "description", json_string("Bad request, expected"
                    " \"product\": {{PRODUCT}} and \"datetime\": {{DATETIME}}"));
    ulfius_set_json_body_response(response, MHD_HTTP_BAD_REQUEST, res_fail);
    return U_CALLBACK_IGNORE;
  }
  return U_CALLBACK_ERROR;
}

/**
 * @brief Función para loggear eventos
 * @param log stream a donde loggear
 * @param fmt_str format string para armar el mensaje a loggear
 * @return 0 -> OK, -1 -> ERROR
 * */
int log_event(FILE *log, char *fmt_str, ...)
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
int get_datetime(char *buf, size_t max_len)
{
  time_t now = time(NULL);
  if (now == ((time_t)-1))
    return -1;
  strftime(buf, max_len, "%D %H:%M:%S ", localtime(&now));
  return 0;
}

/**
 * @param hu_readable buffer donde va a escribir bytes human readable
 * @param buf_len size de hu_readable
 * @param bytes bytes a convertir en human readable
 * @return -1 -> error, 0 -> OK
 * */
int bytes_to_human_readable(char *hu_readable, size_t buf_len, long double bytes)
{
  char unidades[5][5] = {"B", "kB", "MB", "GB", "TB"};
  int pos = 0;
  long double resultado;
  long double div = 1024;
  while ((resultado = bytes / div) > 1)
  {
    pos++;
    div *= 1024;
  }
  resultado *= 1000;
  if (snprintf(hu_readable, buf_len,
               "%3.1Lf %s", resultado, unidades[pos]) < 0)
    return -1;
  return 0;
}
