#include <ulfius.h>
#include <string.h>
#include <orcania.h> 
#include <pwd.h>
#include <grp.h>
#include <limits.h>

#define GROUP_ID 1234
#define GROUP_NAME "SERVER_USERS"
#define MAX_CMD_LEN 1024 // Fuente: yo
#define MAX_UNAME_LEN 32 // según useradd

int callback_create_user(const struct _u_request *request,
                         struct _u_response *response, void *user_data);
int callback_get_users(const struct _u_request *request,
                         struct _u_response *response, void *user_data);


