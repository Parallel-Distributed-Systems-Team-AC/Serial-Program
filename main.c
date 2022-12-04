
#include "mmio.h"
#include "mmio.c"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#define  BILLION 1000000000L;

typedef unsigned int uint;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Forward Declarations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void initialize_colors(uint *v, uint *v_back, uint size_of_V);
void trim_serial( uint* I , uint* J ,bool* is_in_I , bool* is_in_J , bool* is_in_SCC , bool* is_it_root, uint E , uint total_nodes  , int* total_nodes_in_SCC);
void print_array(uint *v, uint size_of_V);
void print_edges(uint *I, uint *J, uint E);
void load_file(const char *file_name, uint *total_nodes,
               uint *total_number_of_edges, uint **I, uint **J, int number_of_columns);
void load_example(uint *total_nodes, uint *total_number_of_edges, uint **I,
                  uint **J);
const char* select_file(int number_of_file);
int number_columns(int number_of_file);
double get_time(struct timespec start, struct timespec finish);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Function Implementations
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void initialize_colors(uint *v, uint *v_back, uint size_of_V) {
  for (uint i = 0; i < size_of_V; i++) {
    v[i] = i;
    v_back[i] = i;
  }
}

void print_array(uint *v, uint size_of_V) {
  // printf("Inside print_array \n");
  for (uint i = 0; i < size_of_V; i++) {
    printf("Node: %4d  v = %4d\n", i, v[i]);
  }
  // printf("End of print_array \n");
}

void print_edges(uint *I, uint *J, uint E) {
  printf("Inside print_edges \n");

  printf(" \n");
  for (uint i = 0; i < E; i++) {
    printf("Edge form: %4d --> %4d \n", I[i], J[i]);
  }

  printf(" \n");
  printf("End of print_edges \n");
}

void trim_serial( uint* I , uint* J ,bool* is_in_I , bool* is_in_J , bool* is_in_SCC , bool* is_it_root, uint E , uint total_nodes  , int* total_nodes_in_SCC){


  // is_in_I , is_in_J  must have false everywere 

  bool trimed_once = false ;
  int count = 0 ; 

  for( int i = 0 ; i < E ; i++){
    if( (! is_in_SCC[ I[i] ] ) && (!  is_in_SCC[ J[i] ]) ){

      is_in_I[ I[i] ] = true;
    }
    if( (! is_in_SCC[ I[i] ] ) && (!  is_in_SCC[ J[i] ])  ){

      is_in_J[ J[i] ] = true;
    }
  }

  for( int i = 0 ; i < total_nodes ; i++){
    if ( is_in_I[i] != is_in_J[i] ){
      // Then it is not in I and is in J
      // or it is in I and not in J 
      // This is a XOR
      is_in_SCC[i] = true ; 
      is_it_root[i] = true ; 
      (*total_nodes_in_SCC)++;
      count++ ;

      trimed_once = true ;
    }
  }

  // We can call this again to trim new nodes

  //printf("Trimed count %d", count);

  if( trimed_once ){

    for( int i = 0 ; i < total_nodes ; i++){

      is_in_I[i] = false;
      is_in_J[i] = false;
    }


    trim_serial( I , J , is_in_I , is_in_J , is_in_SCC , is_it_root, E ,  total_nodes  ,total_nodes_in_SCC);

  }

  //return NULL ;
}


void load_file(const char *file_name, uint *total_nodes,
               uint *total_number_of_edges, uint **I, uint **J, int number_of_columns) {

  uint ret_code;
  MM_typecode matcode;
  FILE *f;

  uint M, N, nz;

  printf("Inside load file \n");

  if ((f = fopen(file_name, "r")) == NULL) {
    printf("Could not open file it returned a NULL pointer.\n");
    exit(1);
  }

  if (mm_read_banner(f, &matcode) != 0) {
    printf("Could not process Matrix Market banner.\n");
    exit(1);
  }

  /*  This is how one can screen matrix types if their application */
  /*  only supports a subset of the Matrix Market data types.      */

  if (mm_is_complex(matcode) && mm_is_matrix(matcode) &&
      mm_is_sparse(matcode)) {
    printf("Sorry, this application does not support ");
    printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
    exit(1);
  }

  /* find out size of sparse matrix .... */

  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) != 0)
    exit(1);

  /* reseve memory for matrices */

  *I = (uint *)malloc(nz * sizeof(int));
  *J = (uint *)malloc(nz * sizeof(int));
  double val;
  *total_number_of_edges = nz;

  /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
  /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
  /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

    if (number_of_columns==2){
      for (uint i = 0; i < nz; i++) {
          fscanf(f, "%u %u \n", &(*I)[i], &(*J)[i]);
        }
    }
    else {
        for (uint i = 0; i < nz; i++) {
          fscanf(f, "%u %u %lg\n", &(*I)[i], &(*J)[i], &val);
        }
    
    }

  if (f != stdin) {
    fclose(f);
  }

  /************************/
  /* now write out matrix */
  /************************/

  // mm_write_banner(stdout, matcode);
  mm_write_mtx_crd_size(stdout, M, N, nz);

  *total_nodes = M + 1;

  // for (i = 0; i < nz; i++)
  //  fprintf(stdout, "%d %d %20.19g\n", I[i], J[i], val[i]);
}

void load_example(uint *total_nodes, uint *total_number_of_edges, uint **I,
                  uint **J) {

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ OUR TEST ARRAY
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  printf("Get in main and Define variable \n");

  /* SCC are :
        3                          // from  trim
        6                          // from  trim
        2                          // from Re - trim
        5                          // from Re - trim
        26                         // from Re - trim
        27                         // from Re - trim
        20                         // points to itself
        [13,12]                    // max 12
        [7,4,1]                    // max 7
        [11,10,9,8]                // max 11
        [17,16,15,14]              // max 17
        [25,23,24,21,22,19,18]     // max 25
    */

  *total_nodes =
      28; // plus one for the node ZERO , for the numbers to be the  same

  printf("Define test array\n");
  // Temp array to test and load values
  uint array[] = {5,  1,  7,  1,  26, 2,  1,  4,  2,  5,  4,  7,  11,
                  8,  8,  9,  9,  10, 10, 11, 13, 12, 12, 13, 17, 14,
                  27, 14, 14, 15, 16, 15, 15, 16, 16, 17, 22, 18, 18,
                  19, 20, 20, 18, 21, 19, 22, 25, 22, 21, 23, 22, 23,
                  19, 24, 23, 25, 24, 25, 10, 26, 26, 27};

  *total_number_of_edges = sizeof(array) / 8; //  = len(I) = len(J)
  printf("number_of_edges = %d\n", *total_number_of_edges);
  printf("number_of_nodes = %d\n", *total_nodes);

  printf("Start malloc \n");

  *I = (uint *)malloc(*total_number_of_edges * sizeof(int));
  *J = (uint *)malloc(*total_number_of_edges * sizeof(int));

  printf("Fill I and J from the example array\n");

  for (uint i = 0; i < *total_number_of_edges; i++) {
    // printf("Passed i= %d\n" ,i);
    (*I)[i] = array[2 * i];
    (*J)[i] = array[2 * i + 1];
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of OUR ARRAY
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

const char* select_file(int number_of_file){
  const char *file_name;

  switch (number_of_file){
    case 0:
      file_name = "files/celegansneural.mtx";
      break;
    case 1:
      file_name = "files/foldoc.mtx";
      break;
    case 2:
      file_name = "files/language.mtx";
      break;
    case 3:
      file_name = "files/eu-2005.mtx";
      break;
    case 4:
      file_name = "files/wiki-topcats.mtx";
      break;
    case 5:
      file_name = "files/sx-stackoverflow.mtx";
      break;
    case 6:
      file_name = "files/wikipedia-20060925.mtx";
      break;
    case 7:
      file_name = "files/wikipedia-20061104.mtx";
      break;
    case 8:
      file_name = "files/wikipedia-20070206.mtx";
      break;
    case 9:
      file_name = "files/wb-edu.mtx";
      break;
    case 10:
      file_name = "files/indochina-2004.mtx";
      break;
    case 11:
      file_name = "files/uk-2002.mtx";
      break;
    case 12:
      file_name = "files/arabic-2005.mtx";
      break;
    case 13:
      file_name = "files/uk-2005.mtx";
      break;
    case 14:
      file_name = "files/twitter7.mtx";
      break;
  }

  return file_name;
}

int number_columns(int number_of_file){
  int number_of_columns;
  switch (number_of_file){
    case 0:
      number_of_columns=3;
      break;
    case 1:
      number_of_columns=3;
      break;
    case 2:
      number_of_columns=3;
      break;
    case 3:
      number_of_columns=2;
      break;
    case 4:
      number_of_columns=2;
      break;
    case 5:
      number_of_columns=3;
      break;
    case 6:
      number_of_columns=2;
      break;
    case 7:
      number_of_columns=2;
      break;
    case 8:
      number_of_columns=2;
      break;
    case 9:
      number_of_columns=2;
      break;
    case 10:
      number_of_columns=2;
      break;
    case 11:
      number_of_columns=2;
      break;
    case 12:
      number_of_columns=2;
      break;
    case 13:
      number_of_columns=2;
      break;
    case 14:
      number_of_columns=2;
      break;
  }

  return number_of_columns;
}

double get_time(struct timespec start, struct timespec finish){
  double time;
  time =(finish.tv_sec - start.tv_sec)+(double)(finish.tv_nsec - start.tv_nsec)/(double)BILLION;
  return time;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   Main
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int main() {

  struct timespec start_load, finish_load, start_trim, finish_trim, start_find, finish_find;


  printf(" ~~~~~~~~~~~~~~~~~~  Code Version 1.8  ~~~~~~~~~~~~~~~~~~ \n\n");
  //~~~~~~~~~~~~~~~~~~~~~  Opening file ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  uint *I, *J;
  uint total_nodes;
  uint total_number_of_edges;
  bool load_from_file = true;
  int a;

  printf("0) For 'celegansneural.mtx'\n");
  printf("1) For 'foldoc.mtx' \n");
  printf("2) For 'language.mtx' \n");
  printf("3) For 'eu-2005.mtx'\n");
  printf("4) For 'wiki-topcats.mtx'\n");
  printf("5) For 'sx-stackoverflow.mtx'\n");
  printf("6) For 'wikipedia-20060925.mtx'\n");
  printf("7) For 'wikipedia-20061104.mtx'\n");
  printf("8) For 'wikipedia-20070206.mtx'\n");
  printf("9) For 'wb-edu.mtx'\n");
  printf("10) For 'indochina-2004.mtx' \n");
  printf("11) For 'uk-2002.mtx' \n");
  printf("12) For 'arabic-2005.mtx' \n");
  printf("13) For 'uk-2005.mtx'\n");
  printf("14) For 'twitter7.mtx' \n");

  printf("Enter the file you want to test: \n");
  scanf("  %d", &a);

  int number_of_file=a;

  const char* file_name = select_file(number_of_file);
  int number_of_columns=number_columns(number_of_file);
  struct timeval stop, start;


  clock_gettime(CLOCK_MONOTONIC, &start_load);

  // Load data from a file
  if (load_from_file) {
    //const char *file_name = "files/language.mtx";
    printf("                     Opened File: %s\n", file_name);
    load_file(file_name, &total_nodes, &total_number_of_edges, &I, &J,number_of_columns);

  } else {

    printf("Get in main and Define variable \n");
    load_example(&total_nodes, &total_number_of_edges, &I, &J);
  }

  clock_gettime(CLOCK_MONOTONIC, &finish_load);

  double load_time=get_time(start_load, finish_load);
  printf("Loading the file took: %lf\n",load_time);

  uint *v, *v_back, *SCC_colors;
  uint total_different_colors_in_SCC = 0;
  printf("number_of_edges = %d\n", total_number_of_edges);
  printf("number_of_nodes = %d\n", total_nodes);

  v = (uint *)malloc(total_nodes * sizeof(uint));
  v_back = (uint *)malloc(total_nodes * sizeof(uint));
  SCC_colors = (uint *)malloc(total_nodes * sizeof(uint));
  bool *is_it_root = (bool *)calloc(sizeof(bool), total_nodes);
  bool *is_in_SCC = (bool *)calloc(sizeof(bool), total_nodes);
  bool *is_in_I = (bool*) calloc(sizeof(bool) , total_nodes);
  bool *is_in_J = (bool*) calloc(sizeof(bool) , total_nodes);

  uint E = total_number_of_edges; 


  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  End of Set-up
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  gettimeofday(&start, NULL);
  bool has_changed_colors_forward, has_changed_colors_backward;
  bool is_G_not_empty = true;
  uint total_nodes_in_SCC = 0;

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Trim
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  clock_gettime(CLOCK_MONOTONIC, &start_trim);

  // Inputs is_in_I and is_in_J must have false everywhere !
  trim_serial( I , J , is_in_I , is_in_J , is_in_SCC , is_it_root, E ,  total_nodes ,&total_nodes_in_SCC);

  clock_gettime(CLOCK_MONOTONIC, &finish_trim);

  double trim_time=get_time(start_trim, finish_trim);
  printf("Trimming took: %lf\n",trim_time);


  printf("\n Start finding the SCC \n");

  clock_gettime(CLOCK_MONOTONIC, &start_find);
  
  while (is_G_not_empty) {

    initialize_colors(v, v_back, total_nodes);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  FORWWARD & BACKWARD
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    has_changed_colors_forward = true;
    has_changed_colors_backward = true;

    while (has_changed_colors_forward && has_changed_colors_backward) {
      has_changed_colors_forward = false;
      has_changed_colors_backward = false;

      for (uint k = 0; k < E; k++) {
        if (is_in_SCC[I[k]] || is_in_SCC[J[k]]) {

          continue;
        }

        if (v[I[k]] > v[J[k]]) {
          v[J[k]] = v[I[k]];
          has_changed_colors_forward = true;
        }
        if (v_back[J[k]] > v_back[I[k]]) {
          v_back[I[k]] = v_back[J[k]];
          has_changed_colors_backward = true;
        }
      }
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  FORWWARD
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    while (has_changed_colors_forward) {
      has_changed_colors_forward = false;
      for (uint k = 0; k < E; k++) {
        if (is_in_SCC[I[k]] || is_in_SCC[J[k]]) {
          continue;
        }
        if (v[I[k]] > v[J[k]]) {
          v[J[k]] = v[I[k]];
          has_changed_colors_forward = true;
        }
      }
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  BACKWARD
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    while (has_changed_colors_backward) {
      has_changed_colors_backward = false;
      for (uint k = 0; k < E; k++) {
        if (is_in_SCC[I[k]] || is_in_SCC[J[k]]) {
          continue;
        }
        if (v_back[J[k]] > v_back[I[k]]) {
          v_back[I[k]] = v_back[J[k]];
          has_changed_colors_backward = true;
        }
      }
    }

    for (uint node = 0; node < total_nodes; node++) {
      if (is_in_SCC[node]) {
        continue;
      }
      if (v[node] == v_back[node]) {
        // Then the node 'node' is part of an SCC with color v[node]
        is_in_SCC[node] = true;
        SCC_colors[node] = v[node];
        total_nodes_in_SCC++;

        if (v[node] == node) {
          // then 'node' is the root of an SCC
          // add it in a list ?  or bool_array
          is_it_root[node] = true;
        }
        if (total_nodes_in_SCC == total_nodes) {
          is_G_not_empty = false;
          break;
        }
      }
    }

    // printf("The number of total trimmed nodes is:%d",total_trimmed_elements);
  }

  uint total_number_SCC = -1; // because 0 is included

  for (uint i = 0; i < total_nodes; i++) {

    if (!is_in_SCC[i]) {
      // This means i is not yet in an SCC and does not have any arrows / edges
      // so it is a trivial SCC
      is_it_root[i] = true;
      total_number_SCC++;
      // Store i in an SCC with color i
    } else if (is_it_root[i]) {
      total_number_SCC++;
    }
  }

  printf("\n Total total_number_SCC = %d \n\n", total_number_SCC);

  clock_gettime(CLOCK_MONOTONIC, &finish_find);
  gettimeofday(&stop, NULL);
  double find_time=get_time(start_find, finish_find);
  double find_and_trim_time=get_time(start_trim, finish_find);
  printf("Finding the number of SCCs took: %lf\n\n",find_time);
  printf("Trimming and finding the number of SCCs took: %lf\n\n",find_and_trim_time);
  printf("took %lu us\n",
         (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

  free(I);
  free(J);
  free(v);
  free(v_back);
  free(is_it_root);
  free(is_in_SCC);
  free(SCC_colors);
}