#define SEM_ID 0x10101011

typedef struct 
{
	pid_t planta;
	int cantidad_pieza;
}info_mjs;

typedef struct 
{
	long tipo_pieza;
	info_mjs contenido;
}data_queue;