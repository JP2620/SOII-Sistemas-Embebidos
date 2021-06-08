#include <stdio.h>
#include <ulfius.h>
#include "../include/server_util.h"
#define PORT 8538
#define PREFIX "/api/server"


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
  
  ulfius_add_endpoint_by_val(&instance, "GET", PREFIX, "/goes", 0,
                             &callback_ls_goes, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", PREFIX, "/goes", 0,
                             &callback_find_goes, NULL);

  // Inicializar el framework
  retval = ulfius_start_framework(&instance); // Abre conexion http
  if (retval == U_OK)
  {
    pause();
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
