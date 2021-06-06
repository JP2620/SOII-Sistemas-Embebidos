#include <ulfius.h>
#include <string.h>
#include <orcania.h> 
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>

#define GROUP_ID 1234
#define GROUP_NAME "SERVER_USERS"
#define USER_SERVICE "User service"
#define PATH_LOG "/var/log/tp3/log.txt"
#define PATH_GOES "/var/www/goes/"
#define MAX_CMD_LEN 1024 // Fuente: yo
#define MAX_UNAME_LEN 32 // según useradd

/* CALLBACKS */
int callback_create_user(const struct _u_request *request,
                         struct _u_response *response, void *user_data);
int callback_get_users(const struct _u_request *request,
                         struct _u_response *response, void *user_data);
int callback_ls_goes(const struct _u_request *request,
                         struct _u_response *response, void *user_data);

/* UTILIDADES */
int log_event(FILE* log, char* fmt_str, ...);
int get_datetime(char* buf, size_t max_len);

