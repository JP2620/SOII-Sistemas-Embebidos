#include <stdio.h>
#include <ulfius.h>
#include "../include/server_util.h"
#define PORT 8537
#define PREFIX "/api"


int main()
{
  int retval; // Para guardar valores de retorno

  /* Seteo de numero de puerto para el framework */
  struct _u_instance instance; // Para inicializar el framework ulfius
  if (ulfius_init_instance(&instance, PORT, NULL, NULL))
  {
    exit(EXIT_FAILURE);
  }
  instance.max_post_body_size = 1024;
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
  }
  else
  {
    fprintf(stderr, "ulfius_start_framework\n");
    exit(EXIT_FAILURE);
  }

  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  return 0;
}
