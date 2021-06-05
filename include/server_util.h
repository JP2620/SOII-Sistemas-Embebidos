#include <ulfius.h>
#include <string.h>
#include <orcania.h> 
#include <pwd.h>

#define GROUP_ID 1234
#define GROUP_NAME "SERVER_USERS"
#define MAX_CMD_LEN 1024 // Fuente: yo

int callback_create_user(const struct _u_request *request,
                         struct _u_response *response, void *user_data);


