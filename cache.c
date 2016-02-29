/*
 * cache.c
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static Pcache ucache;
static Pcache_line free_cache_lines;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;



/************************************************************/
void set_cache_param(param, value)
  int param;
  int value;
{

  switch (param) {
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_split = FALSE;
    cache_usize = value;
    break;
  case CACHE_PARAM_ISIZE:
    cache_split = TRUE;
    cache_isize = value;
    break;
  case CACHE_PARAM_DSIZE:
    cache_split = TRUE;
    cache_dsize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  case CACHE_PARAM_WRITEBACK:
    cache_writeback = TRUE;
    break;
  case CACHE_PARAM_WRITETHROUGH:
    cache_writeback = FALSE;
    break;
  case CACHE_PARAM_WRITEALLOC:
    cache_writealloc = TRUE;
    break;
  case CACHE_PARAM_NOWRITEALLOC:
    cache_writealloc = FALSE;
    break;
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
  }

}
/************************************************************/

/************************************************************/
void init_cache()
{
  /* initialize the cache, and cache statistics data structures */


  /* Inicializar tamaño */
  /* Split == FALSE*/
  if(cache_split == 0){
    ucache = dcache = icache  = &c1;
    ucache->size          = cache_usize;
    ucache->associativity = cache_assoc;
    ucache->n_sets        = (cache_usize * cache_assoc) / cache_block_size;

    /* Mascaras */
    ucache->index_mask_offset = LOG2(cache_block_size);
    ucache->index_mask        = ~(0xFFFFFFFF << (ucache->index_mask_offset +
                                                 LOG2(ucache->n_sets)));

    /* Allocación de memoria */
    ucache->LRU_head     = (Pcache_line *)malloc(sizeof(Pcache_line) * ucache->n_sets);
    ucache->LRU_tail     = (Pcache_line *)malloc(sizeof(Pcache_line) * ucache->n_sets);
    ucache->set_contents = (int *)malloc(sizeof(int) * ucache->n_sets);

    /* Vaciar contenido cache */
    for (int i = 0; i < ucache->n_sets; i++)
      {
        ucache->LRU_head[i] = NULL;
        ucache->LRU_tail[i] = NULL;
        ucache->set_contents[i]= 0;
      }
    ucache->contents = 0;
    free_cache_lines = NULL;
    /*------------------------------------------------------------------------*/
    /* Split == TRUE*/
    /*------------------------------------------------------------------------*/
  }else{
    icache = &c1;
    dcache = &c2;

    /* Inicializar tamaño */

    /* instrucciones */
    icache->size          = cache_isize;
    icache->associativity = cache_assoc;
    icache->n_sets        = (cache_isize * cache_assoc) / cache_block_size;

    /* datos */
    dcache->size          = cache_dsize;
    dcache->associativity = cache_assoc;
    dcache->n_sets        = (cache_dsize * cache_assoc) / cache_block_size;

    /* Mascaras */
    /* instrucciones */
    icache->index_mask_offset = LOG2(cache_block_size);
    icache->index_mask        = ~(0xFFFFFFFF << (icache->index_mask_offset +
                                                 LOG2(icache->n_sets)));
    /* datos */
    dcache->index_mask_offset = LOG2(cache_block_size);
    dcache->index_mask        = ~(0xFFFFFFFF << (dcache->index_mask_offset +
                                                 LOG2(dcache->n_sets)));

    /* Allocación de memoria */
    /* instrucciones */
    icache->LRU_head     = (Pcache_line *)malloc(sizeof(Pcache_line) * icache->n_sets);
    icache->LRU_tail     = (Pcache_line *)malloc(sizeof(Pcache_line) * icache->n_sets);
    icache->set_contents = (int *)malloc(sizeof(int) * icache->n_sets);

    /* datos */
    dcache->LRU_head     = (Pcache_line *)malloc(sizeof(Pcache_line) * dcache->n_sets);
    dcache->LRU_tail     = (Pcache_line *)malloc(sizeof(Pcache_line) * dcache->n_sets);
    dcache->set_contents = (int *)malloc(sizeof(int) * dcache->n_sets);

    /* Vaciar contenido cache */
    /* instrucciones */
    for (int i = 0; i < icache->n_sets; i++)
      {
        icache->LRU_head[i] = NULL;
        icache->LRU_tail[i] = NULL;
        icache->set_contents[i]= 0;
      }
    icache->contents = 0;

    /* instrucciones */
    for (int i = 0; i < dcache->n_sets; i++)
      {
        dcache->LRU_head[i] = NULL;
        dcache->LRU_tail[i] = NULL;
        dcache->set_contents[i]= 0;
      }
    dcache->contents = 0;

    /* No hay líneas vacías*/
    free_cache_lines = NULL;
  }
  /*------------------------------------------*/
  /* Vaciar contenido stats */
  /*------------------------------------------*/
  /* Instrucciones */
  cache_stat_inst.accesses       = 0;
  cache_stat_inst.misses         = 0;
  cache_stat_inst.replacements   = 0;
  cache_stat_inst.demand_fetches = 0;
  cache_stat_inst.copies_back    = 0;
  /* Datos */
  cache_stat_data.accesses       = 0;
  cache_stat_data.misses         = 0;
  cache_stat_data.replacements   = 0;
  cache_stat_data.demand_fetches = 0;
  cache_stat_data.copies_back    = 0;
}
/************************************************************/

/************************************************************/
Pcache_line get_free_line(void)
    {
    Pcache_line head = free_cache_lines;
    
    if(head != NULL){
        free_cache_lines = head->LRU_next;
        }else{
        head = (Pcache_line)malloc(sizeof(cache_line));

        if(head == NULL)
            return NULL;
        }
    
    memset((void*)head, 0, sizeof(cache_line));

    return head;
    }

/************************************************************/

void insert_head(Pcache_line * head, Pcache_line * tail, Pcache_line item)
    {
    if(item->LRU_prev){
        item->LRU_prev->LRU_next = item->LRU_next;
        }else{
        /* item en cabeza */
        return;
        }

    if(item->LRU_next){
        item->LRU_next->LRU_prev = item->LRU_prev;
        }else{
        /* item en cola */
        *tail = item->LRU_prev;
        }

    item->LRU_next = *head;
    item->LRU_prev = (Pcache_line)NULL;

    if(item->LRU_next){
        item->LRU_next->LRU_prev = item;
    }else{
        *tail = item;
    }

    *head = item;
    }

/************************************************************/


/************************************************************/
void perform_access(addr, access_type)
     unsigned addr, access_type;
{
  Pcache_line LRU_aux;
  unsigned tag, index;
  /* Maneja accesos a cache */

  /*
    Verifica si el acceso es carga de datos o instrucciones
    e incrementa los contadores respectivamente
   */
  if(access_type == TRACE_INST_LOAD){
      cache_stat_inst.accesses++;
  }else{
    cache_stat_data.accesses++;
  }

  /* Verifica si el cache está unificado */
  if(cache_split == FALSE){

    /* Calcula índice y tag */
    index = (addr & ucache->index_mask)  >> ucache->index_mask_offset;
    tag   = (addr & ~ucache->index_mask) >> (ucache->index_mask_offset +
                                             LOG2(ucache->n_sets));


    /* Verifica si el índice está en el rango de direcciones */
    if((int)index <= ucache->n_sets){

      /* Accede a la línea de interés  e incrementa número de accesos */
      LRU_aux = ucache->LRU_head[index];

      /*
        Recorrer todos los ways de la línea
        en búsqueda del tag.
      */
      while(LRU_aux != NULL){

        /* Verificar si los tags coinciden*/
        if(LRU_aux->tag == tag){

          /* Verificar si el acceso es para almacenar datos*/
          if(access_type == TRACE_DATA_STORE){

            /* Marca el bit como sucio*/
            LRU_aux->dirty = TRUE;
          }

          /* inserta valor*/
          insert_head(&ucache->LRU_head[index],
                 &ucache->LRU_tail[index],
                 LRU_aux);

          /* Sale de rutina */
          return;
        }

        /* Obtiene siguiente elemento en la línea*/
        LRU_aux = LRU_aux->LRU_next;
      }

      /*
        Verifica tipo de acceso fallido e incrementa
        estadísticas
      */
      if(access_type == TRACE_INST_LOAD){
        cache_stat_inst.misses++;
        cache_stat_inst.demand_fetches += words_per_block;
      }else{
        cache_stat_data.misses++;
        cache_stat_data.demand_fetches += words_per_block;
      }

      /*
        En caso de fallo, verifica si la línea está llena y
        realiza reemplazo LRU (en la cola)
      */
      if(ucache->set_contents[index] == ucache->associativity){

        /* En caso de que esté sucia la escribe en memoria*/
        if(ucache->LRU_tail[index]->dirty == TRUE){
          cache_stat_data.copies_back += words_per_block;
        }

        /* Reemplazar tag de la cola */
        ucache->LRU_tail[index]->tag = tag;

        /* Si fue escritura lo ponemos como sucio */
        if(access_type == TRACE_DATA_STORE){
          ucache->LRU_tail[index]->dirty = TRUE;
        }

        /* Incrementa número de reemplazos */
        if(access_type == TRACE_INST_LOAD){
          cache_stat_inst.replacements++;
        }else{
          cache_stat_data.replacements++;
        }
        return;
      }

      /*
        En caso de fallo, si la línea no está
        llena obtiene un way y realiza una inserción.
      */
      LRU_aux = get_free_line();

      if(LRU_aux != NULL){
        insert(&ucache->LRU_head[index],
               &ucache->LRU_tail[index],
               LRU_aux);

        ucache->set_contents[index]++;
        LRU_aux->tag = tag;

        if(access_type == TRACE_DATA_STORE){
          LRU_aux->dirty = TRUE;
        }
      }else{
        exit(-1);
        }
    }else{
      printf("El índice: %d, es mayor al número de localidades %d",
             index, ucache->n_sets);
      return;
    }
    /*------------------------------------------------------------------------*/
    /* Split == TRUE*/
    /*------------------------------------------------------------------------*/
  }else{

    /* Instrucciones */
    if(access_type == TRACE_INST_LOAD){ 

      /* Calcula índice y tag */
      index = (addr & icache->index_mask)  >> icache->index_mask_offset;
      tag   = (addr & ~icache->index_mask) >> (icache->index_mask_offset +
                                             LOG2(icache->n_sets));
      
      /* Verifica si el índice está en el rango de direcciones */
      if((int)index <= icache->n_sets){

        /* Accede a la línea de interés  e incrementa número de accesos */
        LRU_aux = icache->LRU_head[index];

        /*
          Recorrer todos los ways de la línea
          en búsqueda del tag.
        */
        while(LRU_aux != NULL){

          /* Verificar si los tags coinciden*/
          if(LRU_aux->tag == tag){

            /* Si encuentra, sale de rutina */
            return;
          }

          /* Obtiene siguiente elemento en la línea*/
          LRU_aux = LRU_aux->LRU_next;
        }

        /* En caso de no obtener un match*/
        cache_stat_inst.misses++;
        cache_stat_inst.demand_fetches += words_per_block;
        
        /*
          En caso de fallo, verifica si la línea está llena y
          realiza reemplazo LRU (en la cola)
        */
        if(icache->set_contents[index] == icache->associativity){

          /* Reemplazar tag de la cola */
          icache->LRU_tail[index]->tag = tag;

          /* Incrementa número de reemplazos */
          cache_stat_inst.replacements++;

          return;
        }

        /*
          En caso de fallo, si la línea no está
          llena obtiene un way y realiza una inserción.
        */
        LRU_aux = get_free_line();
        
        if(LRU_aux != NULL){
          insert(&icache->LRU_head[index],
                 &icache->LRU_tail[index],
                 LRU_aux);
          
          icache->set_contents[index]++;
          LRU_aux->tag = tag;
        }else{
          exit(-1);
        }
      }
      /* Datos */
      }else{

    }
  }
}
/************************************************************/

/************************************************************/
void flush()
{
    Pcache_line LRU_aux;

    /* flush the cache */

    /* Split == FALSE*/
    if(cache_split == FALSE) {
      for(int i = 0; i < ucache->n_sets; i++){
        LRU_aux = ucache->LRU_head[i];
        while(LRU_aux != NULL){
          if(LRU_aux->dirty == TRUE){
            cache_stat_data.copies_back += words_per_block;
            LRU_aux->dirty = FALSE;
          }
          LRU_aux = LRU_aux->LRU_next;
        }
      }
      /*------------------------------------------------------------------------*/
      /* Split == TRUE*/
      /*------------------------------------------------------------------------*/
    }else{
      for(int i = 0; i < dcache->n_sets; i++){
        LRU_aux = dcache->LRU_head[i];
        while(LRU_aux != NULL){
          if(LRU_aux->dirty == TRUE){
            cache_stat_data.copies_back += words_per_block;
            LRU_aux->dirty = FALSE;
          }
          LRU_aux = LRU_aux->LRU_next;
        }
      }
    }
}
/************************************************************/

/************************************************************/
void delete(head, tail, item)
  Pcache_line *head, *tail;
  Pcache_line item;
{
  if (item->LRU_prev) {
    item->LRU_prev->LRU_next = item->LRU_next;
  } else {
    /* item at head */
    *head = item->LRU_next;
  }

  if (item->LRU_next) {
    item->LRU_next->LRU_prev = item->LRU_prev;
  } else {
    /* item at tail */
    *tail = item->LRU_prev;
  }
}
/************************************************************/

/************************************************************/
/* inserts at the head of the list */
void insert(head, tail, item)
  Pcache_line *head, *tail;
  Pcache_line item;
{
  item->LRU_next = *head;
  item->LRU_prev = (Pcache_line)NULL;

  if (item->LRU_next)
    item->LRU_next->LRU_prev = item;
  else
    *tail = item;

  *head = item;
}
/************************************************************/

/************************************************************/
void dump_settings()
{
  printf("*** CACHE SETTINGS ***\n");
  if (cache_split) {
    printf("  Split I- D-cache\n");
    printf("  I-cache size: \t%d\n", cache_isize);
    printf("  D-cache size: \t%d\n", cache_dsize);
  } else {
    printf("  Unified I- D-cache\n");
    printf("  Size: \t%d\n", cache_usize);
  }
  printf("  Associativity: \t%d\n", cache_assoc);
  printf("  Block size: \t%d\n", cache_block_size);
  printf("  Write policy: \t%s\n", 
	 cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
  printf("  Allocation policy: \t%s\n",
	 cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}
/************************************************************/

/************************************************************/
void print_stats()
{
  printf("\n*** CACHE STATISTICS ***\n");

  printf(" INSTRUCTIONS\n");
  printf("  accesses:  %d\n", cache_stat_inst.accesses);
  printf("  misses:    %d\n", cache_stat_inst.misses);
  if (!cache_stat_inst.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses,
	 1.0 - (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
  printf("  replace:   %d\n", cache_stat_inst.replacements);

  printf(" DATA\n");
  printf("  accesses:  %d\n", cache_stat_data.accesses);
  printf("  misses:    %d\n", cache_stat_data.misses);
  if (!cache_stat_data.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses,
	 1.0 - (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
  printf("  replace:   %d\n", cache_stat_data.replacements);

  printf(" TRAFFIC (in words)\n");
  printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches);
  printf("  copies back:   %d\n", cache_stat_inst.copies_back +
	 cache_stat_data.copies_back);
}
/************************************************************/
