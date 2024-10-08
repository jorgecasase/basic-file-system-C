#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
int BuscaInodLibre(EXT_ENTRADA_DIR *directorio);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	char comando[LONGITUD_COMANDO];
	char orden[LONGITUD_COMANDO];
	char argumento1[LONGITUD_COMANDO];
	char argumento2[LONGITUD_COMANDO];
	 
	int i,j;
	unsigned long int m;
  EXT_SIMPLE_SUPERBLOCK ext_superblock;
  EXT_BYTE_MAPS ext_bytemaps;
  EXT_BLQ_INODOS ext_blq_inodos;
  EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
  EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
  EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
  int entradadir;
  int grabardatos;
  FILE *fent;
     
  // Lectura del fichero completo de una sola vez
     
  fent = fopen("particion.bin","r+b");
  fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
  memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
  memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
  memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
  memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
  memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
  // Buce de tratamiento de comandos
  for (;;){
    grabardatos = 1;
	  do {
		  printf (">> ");
		  fflush(stdin);
		  fgets(comando, LONGITUD_COMANDO, stdin);
		} while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
    if(strcmp(orden, "info") == 0){
      LeeSuperBloque(&ext_superblock);
    }
	  else if (strcmp(orden,"dir")==0) {
      Directorio(directorio,&ext_blq_inodos);
    }
    else if(strcmp(orden, "bytemaps") == 0){
      Printbytemaps(&ext_bytemaps);
    }
    else if(strcmp(orden, "rename") == 0){
      Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);

      Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
      GrabarByteMaps(&ext_bytemaps, fent);
      GrabarSuperBloque(&ext_superblock, fent);
      if(grabardatos)
        GrabarDatos(memdatos, fent);
      grabardatos = 0;
    }
    else if(strcmp(orden, "imprimir") == 0){
      Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
    }
    else if(strcmp(orden, "remove") == 0){
      Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);

      Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
      GrabarByteMaps(&ext_bytemaps, fent);
      GrabarSuperBloque(&ext_superblock, fent);
      if(grabardatos)
        GrabarDatos(memdatos, fent);
      grabardatos = 0;
    }
    else if(strcmp(orden, "copy") == 0){
      Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);

      Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
      GrabarByteMaps(&ext_bytemaps, fent);
      GrabarSuperBloque(&ext_superblock, fent);
      if(grabardatos)
        GrabarDatos(memdatos, fent);
      grabardatos = 0;
    }
    else if(strcmp(orden, "salir") == 0){
      GrabarDatos(memdatos, fent);
      fclose(fent);
      printf("Saliendo con exito...\n");
      return 0;
    }
    else{
      printf("ERROR: comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir\n");
    }
  }
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
  // grabar inodos
  fseek(fich, 2 * sizeof(EXT_BLQ_INODOS), SEEK_SET);
  fwrite(inodos, sizeof(EXT_BLQ_INODOS), 1, fich);

  // grabar directorio
  fseek(fich, 3 * sizeof(EXT_BLQ_INODOS), SEEK_SET);
  fwrite(directorio, sizeof(EXT_ENTRADA_DIR) * MAX_FICHEROS, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
  fseek(fich, sizeof(EXT_BLQ_INODOS), SEEK_SET);
  fwrite(ext_bytemaps, sizeof(EXT_BYTE_MAPS), 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
  fseek(fich, 0, SEEK_SET);
  fwrite(ext_superblock, sizeof(EXT_SIMPLE_SUPERBLOCK), 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
  fseek(fich, 4 * sizeof(EXT_BLQ_INODOS), SEEK_SET);
  fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}


void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
   int i;
   printf("%-30s%-10s%-10s%-15s\n", "Nombre de archivo", "Tamano", "Inodo", "Bloques");
   for (i = 0; i < MAX_INODOS; i++) {
        if (directorio[i].dir_inodo != 0xFFFF && strcmp(directorio[i].dir_nfich, ".") != 0) {
            printf("%-30s%-10u%-10u", directorio[i].dir_nfich, inodos->blq_inodos[directorio[i].dir_inodo].size_fichero, directorio[i].dir_inodo);

            // Mostrar bloques ocupados
            printf("[");
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_INODO) {
                    printf("%d ", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
                }
            }
            printf("]\n");
        }
    }
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
  char *token;
  int numTokens = 0;

  token = strtok(strcomando, " ");
  while (token != NULL && numTokens < 3) {
    switch (numTokens) {
      case 0:
        strcpy(orden, token);
        break;
      case 1:
        strcpy(argumento1, token);
        break;
      case 2:
        strcpy(argumento2, token);
        break;
      }
      token = strtok(NULL, " ");
      numTokens++;
  }

  if(numTokens == 1){
    size_t len = strlen(orden);
    if (len > 0 && orden[len - 1] == '\n') {
      orden[len - 1] = '\0';
    }
    return 0;
  }
  else if(numTokens == 2){
  size_t len = strlen(argumento1);
    if(len > 0 && argumento1[len - 1] == '\n'){
      argumento1[len - 1] = '\0';
    }
    return 0;
  }
  else if(numTokens == 3){
    size_t len = strlen(argumento2);
    if(len > 0 && argumento2[len - 1] == '\n'){
      argumento2[len - 1] = '\0';
    }
    return 0;
  }
  else{
    printf("Error: numero de argumentos no válido.\n");
    return -1; 
  }
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
  printf("Bloque %d Bytes\n", psup->s_block_size);
  printf("Inodos particion = %d\n", psup->s_inodes_count);
  printf("Inodos libres = %d\n", psup->s_free_inodes_count);
  printf("Bloques particion = %d\n", psup->s_blocks_count);
  printf("Bloques libres = %d\n", psup->s_free_blocks_count);
  printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){
  printf("Inodos: ");
  for (int i = 0; i < MAX_INODOS; ++i) {
    printf("%u ", ext_bytemaps->bmap_inodos[i]);
  }
  printf("\n");

  printf("Bloques [0-25] : ");
  for (int i = 0; i < 25 && i < MAX_BLOQUES_PARTICION; ++i) {
    printf("%u ", ext_bytemaps->bmap_bloques[i]);
  }
  printf("\n");
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
  int indice = BuscaFich(directorio, inodos, nombreantiguo);
  if (indice == -1) {
    printf("ERROR: No se encuentra el archivo que quieres renombrar\n");
    return -1; 
  }
  if (BuscaFich(directorio, inodos, nombrenuevo) != -1) {
    printf("ERROR: Ya existe un fichero con ese nombre\n");
    return -1; 
  }
  strcpy(directorio[indice].dir_nfich, nombrenuevo);

  return 0;
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
  int i;
  for (i = 0; i < MAX_INODOS; i++) {
    if (strlen(directorio[i].dir_nfich) > 0) {
      if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
        return i;
      }
    }
  }
  return -1; 
}

int BuscaInodLibre(EXT_ENTRADA_DIR *directorio){
  int i;
  for(i = 0; i < MAX_INODOS; i++){
		if (directorio[i].dir_inodo == NULL_INODO)
		{
			return i;
		}
  }
  return -1;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
  int indice = directorio[BuscaFich(directorio,inodos, nombre)].dir_inodo;
  if (indice == -1) {
    return -1;
  }
  while(indice != NULL_INODO){
    for(int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++){
      if(inodos->blq_inodos[indice].i_nbloque[i] != NULL_BLOQUE){
          printf("%s\n", memdatos[inodos->blq_inodos[indice].i_nbloque[i] - 4].dato);
      }
    }
      indice = inodos->blq_inodos[indice].i_nbloque[MAX_NUMS_BLOQUE_INODO - 1];
  }
  return 1;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
  if(BuscaFich(directorio, inodos, nombre) == -1){
    printf("ERROR: el fichero no existe\n");
    return -1;
  }
  for(int i = 0; i < MAX_INODOS; i++) {
    if(directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre) == 0) {
      int inodo = directorio[i].dir_inodo;
      for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
        int bloque = inodos->blq_inodos[inodo].i_nbloque[j];
        if (bloque != NULL_BLOQUE) {
          ext_bytemaps->bmap_bloques[bloque] = 0;
          ext_superblock->s_free_blocks_count++; 
        }
      }
      ext_bytemaps->bmap_inodos[inodo] = 0;
      ext_superblock->s_free_inodes_count++; 

      memset(&directorio[i], 0, sizeof(EXT_ENTRADA_DIR));
      directorio[i].dir_inodo = NULL_INODO;
      return 0;
    }
  }
  return -1;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich)
{
	int found = 0; 

  int ficheroOrigen = BuscaFich(directorio, inodos, nombreorigen);
  if (ficheroOrigen == -1) {
    printf("ERROR: el fichero de origen no existe\n");
    return -1;
  }

	int inodoLibre = BuscaInodLibre(directorio);
  if(inodoLibre == -1){
    printf("No hay inodos libres para realizar esa operacion\n");
    return -1;
  }
	memcpy(directorio[inodoLibre].dir_nfich, nombredestino, LEN_NFICH);
	
	found = 0;
	for(int i = 0; i < MAX_INODOS; i++){
		if(ext_bytemaps->bmap_inodos[i] == 0){
      found = 1;
			ext_bytemaps->bmap_inodos[i] = 1;
			directorio[inodoLibre].dir_inodo = i;
			break;
		}
	}
	if(found == 0){
		return -1;
	}
	int numBloques = 0;
  for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
    if (inodos->blq_inodos[directorio[ficheroOrigen].dir_inodo].i_nbloque[i] != NULL_BLOQUE) {
      for(int j = 0; j < MAX_BLOQUES_DATOS; j++){
				if(ext_bytemaps->bmap_bloques[j] == 0){
					ext_bytemaps->bmap_bloques[j] = 1;
					inodos->blq_inodos[directorio[inodoLibre].dir_inodo].i_nbloque[numBloques] = j;
					memdatos[j - 4] = memdatos[inodos->blq_inodos[directorio[ficheroOrigen].dir_inodo].i_nbloque[i] - 4];
					numBloques++;
					break;
				}
			}
    }
  }
	inodos->blq_inodos[directorio[inodoLibre].dir_inodo].size_fichero = inodos->blq_inodos[directorio[ficheroOrigen].dir_inodo].size_fichero;
}