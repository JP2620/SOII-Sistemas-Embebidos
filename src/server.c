#include <stdio.h>
#include <ulfius.h>
#include "../include/u_example.h"
#include "../include/server_util.h"
#define PORT 8537
#define PREFIX "/api"


int main()
{
  int retval; // Para guardar valores de retorno

  /* Seteo de numero de puerto para el framework */
  struct _u_instance instance; // Para inicializar el framework ulfius
  // y_init_logs("simple_example", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL,
              // "Starting simple_example");
  if (ulfius_init_instance(&instance, PORT, NULL, NULL))
  {
    // y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
  }
  instance.max_post_body_size = 1024; // Maximo size del body: 1kB

  // Endpoints
  ulfius_add_endpoint_by_val(&instance, "POST", PREFIX, "/users", 0,
                             &callback_create_user, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/users", 0,
                             &callback_get_users, NULL);

  // Inicializar el framework
  retval = ulfius_start_framework(&instance); // Abre conexion http
  if (retval == U_OK)
  {
    getchar();
    // y_log_message(Y_LOG_LEVEL_DEBUG, "Start %s framework on port %d", "", instance.port);
  }
  else
  {
    // y_log_message(Y_LOG_LEVEL_DEBUG, "Error starting framework");
  }

  // Cleanup
  // y_close_logs();
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  return 0;
}
