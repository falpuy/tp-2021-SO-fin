#include "./level.h"

// typedef struct nivel {
// 		char* nombre;
// 		t_list* items;
// 	} NIVEL;

// 	typedef struct item {
// 		char id;
// 		int posx;
// 		int posy;
// 		int item_type; // PERSONAJE, ENEMIGO o CAJA
// 		int quantity;
// 	} ITEM_NIVEL;

// 	/*
// 	* @NAME: nivel_gui_inicializar
// 	* @DESC: Inicializa el espacio de dibujo
// 	* @ERRORS:
// 	*     NGUI_ALREADY_INIT
// 	*/
// 	int nivel_gui_inicializar(void);

// 	/*
// 	* @NAME: nivel_gui_dibujar
// 	* @DESC: Dibuja cada entidad en la lista de items
// 	* @PARAMS:
// 	*       nivel - nivel a dibujar
// 	* @ERRORS:
// 	*     NGUI_NO_INIT
// 	*/
// 	int nivel_gui_dibujar(NIVEL* nivel);

void create_map(void *item) {
    t_log *logger = (t_log*) item;
    log_info(logger, "Iniciando la creacion del mapa..");
    nivel_gui_inicializar();

    // Drawing test
    nivel = nivel_crear("Among OS");
    nivel_gui_dibujar(nivel);

}