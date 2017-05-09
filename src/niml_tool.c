/*************************************************************************
 niml_tool
 04/10/2017 Justin Rajendra
 read in 2 niml datasets, parse the element headers and print to screen
 Compare function to sort and view
 etc...


 *************************************************************************/

#include "niml.h"
#include "mrilib.h"
#include <libgen.h>

typedef struct    // only parts are used for the comparison option
{
  char name[1000], name2[1000];
  char par_name[1000], par_name2[1000];
  int elm_num, elm_num2;
  NI_element *ni_elm, *ni_elm2;
  NI_group *ni_grp, *ni_grp2;
  void *parent, *parent2;
}elm_struct;

elm_struct *n_struct1, *n_struct2, *n_struct_out;
int loop_num = 0, max_arr_len = 0, max_name_len = 1;

/*************************************************************************/
// compare function for qsort names only
int name_compare( const void *p1, const void *p2 )
{
  return strcasecmp( ((elm_struct*)p1)->name, ((elm_struct*)p2)->name);
}

// compare by number 1 then 2
int elm_compare(const void *p1, const void *p2) {
  int n1 = ((elm_struct*)p1)->elm_num;
  int n2 = ((elm_struct*)p2)->elm_num;
  int o1, o2;
  o1 = n1;
  o2 = n2;

  if(n1 == 9999) n1 = ((elm_struct*)p1)->elm_num2;
  if(n2 == 9999) n2 = ((elm_struct*)p2)->elm_num2;
  if(n1 == n2){
    if(o1 == 9999) return 1;
    if(o2 == 9999) return -1;
  }
  return n1 - n2;
}

///*************************************************************************/
//// count the number of elements and groups total (ignores first one?) NOT YET USED
//int NI_count_all( NI_group *ni_in){
//  void *nini;
//  int ii, nn=0;
//
//  if( ni_in == NULL || ni_in->type != NI_GROUP_TYPE    ) return 0 ;
//  if( ni_in->part_num == 0                            ) return 0 ;
//
//  for( ii=0 ; ii < ni_in->part_num ; ii++ ){
//    nini = ni_in->part[ii] ;
//    nn++;
//    if( NI_element_type(nini) == NI_GROUP_TYPE ){  /* recursion */
//      int nsub , jj ;
//      nsub = NI_count_all( nini ) ;
//      if( nsub > 0 ){
//        nn += nsub;
//      }
//    }
//  }
//  return nn ;
//}   // end NI_count_all

/*************************************************************************/
// function for finding first matching attribute index
// side is 1 = rhs, 2 = lhs, 3 = either

# define type_loop()                              \
for( i=0; i < ni_srch->attr_num; i++ ){   \
if(side == 1){  \
if(!ni_srch->attr_rhs[i]) return -1;                            \
if(strcmp(ni_srch->attr_rhs[i],term) == 0) return i; \
} else if(side == 2){                                            \
if(!ni_srch->attr_lhs[i]) return -1;                            \
if(strcmp(ni_srch->attr_lhs[i],term) == 0) return i; \
} else                                                             \
if(side == 3){                                                   \
if(!ni_srch->attr_rhs[i] || !ni_srch->attr_lhs[i]) return -1;  \
if(strcmp(ni_srch->attr_rhs[i],term) == 0 ||                   \
strcmp(ni_srch->attr_lhs[i],term) == 0) return i;            \
}                                                                  \
} return -1;

int NI_loop_attr( void *nini, char* term, int side){
  int i;
  // find the type
  if( NI_element_type(nini) == NI_ELEMENT_TYPE ){  // data
    NI_element *ni_srch = (NI_element *) nini;
    type_loop();
  } else if( NI_element_type(nini) == NI_PROCINS_TYPE ){  // proc instructions?
    NI_procins *ni_srch = (NI_procins *) nini;
    type_loop();
  } else if( NI_element_type(nini) == NI_GROUP_TYPE ){  // group
    NI_group *ni_srch = (NI_group *) nini;
    type_loop();
  } else { return -1; }
}   // end NI_loop_attr

/*************************************************************************/
// function for testing if two niml groups/elements are the same
// based on various names
int NI_name_match( void *nini1, void *nini2){

  int i, type1, type2;
  char *name1, *name2;
  char *name_list[] = {"atr_name","name","Name","data_type","dset_type"};
  int list_n = 5;

  // get the types and make sure they match
  type1 = NI_element_type(nini1);
  type2 = NI_element_type(nini2);

  if( type1 != type2) return 0;

  if( type1 == NI_ELEMENT_TYPE ){
    NI_element *ni_elm1 = (NI_element *) nini1;
    NI_element *ni_elm2 = (NI_element *) nini2;

    if(strcasecmp(ni_elm1->name,ni_elm2->name) != 0) return 0;
    for(i=0;i<list_n;i++){
      name1 = NI_get_attribute(ni_elm1,name_list[i]);
      name2 = NI_get_attribute(ni_elm2,name_list[i]);
      if(name1 && name2){
        if(strcasecmp(name1,name2) != 0) return 0;
      }
    }
    return 1;
  }
  if( type1 == NI_GROUP_TYPE ){
    NI_group *ni_grp1 = (NI_group *) nini1;
    NI_group *ni_grp2 = (NI_group *) nini2;

    if(strcasecmp(ni_grp1->name,ni_grp2->name) != 0) return 0;
    for(i=0;i<list_n;i++){
      name1 = NI_get_attribute(ni_grp1,name_list[i]);
      name2 = NI_get_attribute(ni_grp2,name_list[i]);
      if(name1 && name2){
        if(strcasecmp(name1,name2) != 0) return 0;
      }
    }
    return 1;
  }
  if( type1 == NI_PROCINS_TYPE ){
    NI_procins *ni_pi1 = (NI_procins *) nini1;
    NI_procins *ni_pi2 = (NI_procins *) nini2;

    if(strcasecmp(ni_pi1->name,ni_pi2->name) != 0) return 0;
    for(i=0;i<list_n;i++){
      name1 = NI_get_attribute(ni_pi1,name_list[i]);
      name2 = NI_get_attribute(ni_pi2,name_list[i]);
      if(name1 && name2){
        if(strcasecmp(name1,name2) != 0) return 0;
      }
    }
    return 1;
  }
  return 0;
}   // end NI_name_match

/*************************************************************************/
// find if 1 element/group/proc_ins names match any in another group
// only finds the first one
int NI_find_in_group( void *ni_find, NI_group *ni_in, void **ni_out){

  int i;
  void *nini;
  if( ni_in == NULL || ni_in->type != NI_GROUP_TYPE ) return 0 ;

  if(NI_name_match(ni_find,ni_in)){
    *ni_out = ni_in;
    return 1;
  } else {
    for(i=0;i<ni_in->part_num;i++){
      nini = ni_in->part[i];
      if( NI_element_type(nini) == NI_ELEMENT_TYPE ){  // data
        if(NI_name_match(ni_find,nini)){
          *ni_out = ni_in;
          return 1;
        }
      } else if( NI_element_type(nini) == NI_PROCINS_TYPE ){
        if(NI_name_match(ni_find,nini)){
          *ni_out = ni_in;
          return 1;
        }
      } else if( NI_element_type(nini) == NI_GROUP_TYPE ){  /* recursion */
        NI_group *ni_grp = (NI_group *) nini;
        NI_find_in_group(ni_find, ni_grp, ni_out);
      }
    }
  }
}   // end NI_find_in_group

/*************************************************************************/
// recursive function count and return elements matching attribute names
int NI_search_attr( NI_group *ngr , char *attr , void ***nipt ){
  void **nelar=NULL, *nini;
  int i, ii, nn=0, index;

  // check stuff
  if( ngr  == NULL || ngr->type != NI_GROUP_TYPE  ) return 0 ;
  if( attr == NULL || *attr == '\0'                ) return 0 ;
  if( ngr->part_num == 0                          ) return 0 ;

  // do the group attr
  index = NI_loop_attr(ngr,attr,3);
  if(index >= 0){
    nn++;
    if( nipt ) {
      nelar = (void **) NI_realloc(nelar,void*,nn*sizeof(void *)) ;
      nelar[nn-1] = ngr;
    }
  }
  // for all parts not group  (add null check)
  for( ii=0 ; ii < ngr->part_num ; ii++ ){
    nini = ngr->part[ii];
    if( NI_element_type(nini) == NI_ELEMENT_TYPE ){  // data
      index = NI_loop_attr(nini,attr,3);
      if(index >= 0){
        nn++;
        if( nipt ) {
          nelar = (void **) NI_realloc(nelar,void*,nn*sizeof(void *)) ;
          nelar[nn-1] = nini;
        }
      }
    } else if( NI_element_type(nini) == NI_PROCINS_TYPE ){  // proc instructions?
      index = NI_loop_attr(nini,attr,3);
      if(index >= 0){
        nn++;
        if( nipt ) {
          nelar = (void **) NI_realloc(nelar,void*,nn*sizeof(void *)) ;
          nelar[nn-1] = nini;
        }
      }
    } else if( NI_element_type(nini) == NI_GROUP_TYPE ){  /* recursion */
      NI_group *ni_grp = (NI_group *) nini;
      int nsub , jj ; void **esub = NULL;

      if( nipt ){  /* if nipt == NULL, just return a count */
        nsub = NI_search_attr( ni_grp, attr, &esub);
      } else {
        nsub = NI_search_attr( ni_grp, attr, NULL) ;
      }
      // put it back together
      if( nsub > 0 ){
        if( nipt ) {
          nelar = (void **)NI_realloc(nelar,void*,(nn+nsub)*sizeof(void *));
          for( jj=0 ; jj < nsub ; jj++ ){
            nelar[nn+jj] = esub[jj];
          }
          NI_free(esub);
        }
        nn += nsub;
      }
    }
  }
  if( nn > 0 && nipt ) *nipt = nelar;
  return nn ;
}   // end NI_search_attr

/*************************************************************************/
// recursive function to read elements and print to screen
int elm_print( void *ni_in, int indent, int verb )
{
  int elm_type, i, NumParts, spaces, index;
  // what type
  elm_type = NI_element_type( ni_in );
  if( elm_type == NI_ELEMENT_TYPE ){   // it is data
    NI_element *ni_elm = (NI_element *) ni_in;
    spaces=indent*4;
    printf("\n%*sData element:\n",spaces,"");
    printf("%*s name       = ",spaces,"");
    printf("%s\n",ni_elm->name);
    if(verb){
      printf("%*s vec_num    = ",spaces,"");
      printf("%d\n",ni_elm->vec_num);
      printf("%*s vec_len    = ",spaces,"");
      printf("%d\n",ni_elm->vec_len);
      printf("%*s vec_filled = ",spaces,"");
      printf("%d\n",ni_elm->vec_filled);
      printf("%*s vec_rank   = ",spaces,"");
      printf("%d\n",ni_elm->vec_rank);
      printf("%*s attr_num   = ",spaces,"");
      printf("%d\n",ni_elm->attr_num);
      for( i=0 ; i < ni_elm->attr_num ; i++ ){
        printf("%*s ",spaces,"");
        printf("%d: %s = %s\n",i,ni_elm->attr_lhs[i],ni_elm->attr_rhs[i]);
      }
    } else {
      index = NI_loop_attr(ni_elm,"atr_name",2);
      if(index >= 0){
        printf("%*s ",spaces,"");
        printf("%s   = %s\n",ni_elm->attr_lhs[index],ni_elm->attr_rhs[index]);
      }
    }
  } else if( elm_type == NI_GROUP_TYPE ){   // it is a group
    NI_group *ni_grp = (NI_group *) ni_in;
    spaces=indent*4;
    printf("\n%*sGroup element:\n",spaces,"");
    printf("%*s name = ",spaces,"");
    printf("%s\n",ni_grp->name);
    printf("%*s part_num = ",spaces,"");
    printf("%d\n",ni_grp->part_num);
    if(verb){
      printf("%*s attr_num = ",spaces,"");
      printf("%d\n",ni_grp->attr_num);
      for( i=0 ; i < ni_grp->attr_num ; i++ ){
        printf("%*s ",spaces,"");
        printf("%d: %s = %s\n",i,ni_grp->attr_lhs[i],ni_grp->attr_rhs[i]);
      }
    } else {
      index = NI_loop_attr(ni_grp,"name",2);
      if(index >= 0){
        printf("%*s ",spaces,"");
        printf("%s   = %s\n",ni_grp->attr_lhs[index],ni_grp->attr_rhs[index]);
      }
    }
    indent++;
    // recursive through the number of parts
    for( NumParts=0; NumParts < ni_grp->part_num; NumParts++ ){
      elm_print(ni_grp->part[NumParts],indent,verb);
    }
  } else if( elm_type == NI_PROCINS_TYPE ){   // it is processing instructions?
    NI_procins *ni_pi = (NI_procins *) ni_in;
    printf("\n%*sProcessing instruction:\n",spaces,"");
    printf("%*s target = ",spaces,"");
    printf("%s\n",ni_pi->name);
    if(verb){
      for( i=0 ; i < ni_pi->attr_num ; i++ ){
        printf("%*s ",spaces,"");
        printf("%d: %s = %s\n",i,ni_pi->attr_lhs[i],ni_pi->attr_rhs[i]);
      }
    } else {
      index = NI_loop_attr(ni_pi,"name",2);
      if(index >= 0){
        printf("%*s ",spaces,"");
        printf("%s   = %s\n",ni_pi->attr_lhs[index],ni_pi->attr_rhs[index]);
      }
    }
  }
  return 0;
}  // end recursive print to screen

/*************************************************************************/
// make the type_name_name string
int name_cat(void *ni_in,char *str_in, int indent, int order){

  int elm_type, i, NumParts;
  char *name_temp;
  char indent_char[10], order_char[10];
  sprintf(order_char, "%d", order);

  // get the type
  elm_type = NI_element_type( ni_in );

  if( elm_type == NI_ELEMENT_TYPE ){   // it is data
    sprintf(indent_char, "%d", indent-1);
    NI_element *ni_part = (NI_element *) ni_in;
    strcat(strcat(str_in,"e"),indent_char);
    strcat(strcat(str_in,":"),order_char);
    strcat(strcat(str_in,":"),ni_part->name);
    if ((name_temp = NI_get_attribute(ni_in,"atr_name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
    if ((name_temp = NI_get_attribute(ni_in,"name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
    if ((name_temp = NI_get_attribute(ni_in,"Name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
    if ((name_temp = NI_get_attribute(ni_in,"data_type"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
  } else if(elm_type == NI_GROUP_TYPE){
    sprintf(indent_char, "%d", indent);
    NI_group *ni_part = (NI_group *) ni_in;
    strcat(strcat(str_in,"g"),indent_char);
    strcat(strcat(str_in,":"),order_char);
    strcat(strcat(str_in,":"),ni_part->name);
    if ((name_temp = NI_get_attribute(ni_in,"atr_name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
    if ((name_temp = NI_get_attribute(ni_in,"name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
    if ((name_temp = NI_get_attribute(ni_in,"Name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
    if ((name_temp = NI_get_attribute(ni_in,"dset_type"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
  } else if(elm_type == NI_PROCINS_TYPE){
    sprintf(indent_char, "%d", indent-1);
    NI_procins *ni_part = (NI_procins *) ni_in;
    strcat(strcat(str_in,"p"),indent_char);
    strcat(strcat(str_in,":"),order_char);
    strcat(strcat(str_in,":"),ni_part->name);
    if ((name_temp = NI_get_attribute(ni_in,"atr_name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
    if ((name_temp = NI_get_attribute(ni_in,"name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
    if ((name_temp = NI_get_attribute(ni_in,"Name"))) {
      strcat(strcat(str_in,":"),name_temp);
    }
  }
  return 0;
}

/*************************************************************************/
// get group "name" for matching parents (not really used)
int NI_group_name(NI_group *ngr,char *name_out){
  char *name_temp;
  if( ngr  == NULL || ngr->type != NI_GROUP_TYPE  ) return 1 ;
  strcat(name_out,ngr->name);
  if ((name_temp = NI_get_attribute(ngr,"atr_name"))) {
    strcat(strcat(name_out,":"),name_temp);
  }
  if ((name_temp = NI_get_attribute(ngr,"name"))) {
    strcat(strcat(name_out,":"),name_temp);
  }
  if ((name_temp = NI_get_attribute(ngr,"Name"))) {
    strcat(strcat(name_out,":"),name_temp);
  }
  if ((name_temp = NI_get_attribute(ngr,"dset_type"))) {
    strcat(strcat(name_out,":"),name_temp);
  }
  return 0;
}

/*************************************************************************/
// add to array
int elm_arr( void *ni_in, elm_struct **arr_in, int indent, void *parent, int arr_size)
{
  char elm_name[1000] = "";
  char par_name[1000] = "";
  int elm_type, NumParts, i;

  // what type
  elm_type = NI_element_type( ni_in );
  if(elm_type == -1){ return loop_num; }
  if(loop_num > arr_size){
    arr_size = arr_size + 20;
    *arr_in = realloc(*arr_in, sizeof(elm_struct) * arr_size );
  }
  if( elm_type == NI_ELEMENT_TYPE ){   // it is data
    NI_element *ni_elm = (NI_element *) ni_in;
    name_cat(ni_elm,elm_name,indent,loop_num);
    strcpy((*arr_in)[loop_num].name,elm_name);
    (*arr_in)[loop_num].elm_num = loop_num;
    (*arr_in)[loop_num].ni_elm = ni_in;
    (*arr_in)[loop_num].ni_grp = NULL;
    (*arr_in)[loop_num].parent = parent;
    NI_group_name(parent,par_name);
    strcpy((*arr_in)[loop_num].par_name,par_name);

    // check for binary and set to output binary
    if(NI_get_attribute(ni_in,"ni_form")){
      (*arr_in)[loop_num].ni_elm->outmode = 1;
    }
    // get the max name length for spacing
    if(max_name_len < (int)strlen(elm_name)){
      max_name_len = (int)strlen(elm_name);
    }
    loop_num++;
  } else  if( elm_type == NI_PROCINS_TYPE ){   // it is proc instructions?
    NI_procins *ni_pi = (NI_procins *) ni_in;
    name_cat(ni_pi,elm_name,indent,loop_num);
    strcpy((*arr_in)[loop_num].name,elm_name);
    (*arr_in)[loop_num].elm_num = loop_num;
    (*arr_in)[loop_num].ni_elm = NULL;
    (*arr_in)[loop_num].ni_grp = NULL;
    (*arr_in)[loop_num].parent = parent;
    NI_group_name(parent,par_name);
    strcpy((*arr_in)[loop_num].par_name,par_name);

    if(max_name_len < (int)strlen(elm_name)){
      max_name_len = (int)strlen(elm_name);
    }
    loop_num++;
  } else if( elm_type == NI_GROUP_TYPE ){   // it is a group
    NI_group *ni_grp = (NI_group *) ni_in;
    name_cat(ni_grp,elm_name,indent,loop_num);
    strcpy((*arr_in)[loop_num].name,elm_name);
    (*arr_in)[loop_num].elm_num = loop_num;
    (*arr_in)[loop_num].ni_elm = NULL;
    (*arr_in)[loop_num].ni_grp = ni_in;
    (*arr_in)[loop_num].parent = parent;
    NI_group_name(parent,par_name);
    strcpy((*arr_in)[loop_num].par_name,par_name);

    loop_num++;
    indent++;
    if(max_name_len < (int)strlen(elm_name)){
      max_name_len = (int)strlen(elm_name);
    }
    // recursive through the number of parts
    for( NumParts=0; NumParts < ni_grp->part_num; NumParts++ ){
      elm_arr(ni_grp->part[NumParts],arr_in,indent,ni_in,arr_size);
    }
  }
  return loop_num;
}   // end add to array

/*************************************************************************/
// main drag
int main( int argc , char *argv[] )
{
  set_ni_globs_from_env();   /* 3 Aug 2006 [rickr] */

  //NI's
  void *nini1, *nini2, *ni_in_grp, *ni_out_grp, **elm_list;
  NI_group *ni_out;

  // various variables
  int nn, loop_max, nopt=1, NotDone=1;
  int verb=0, pr_int=0, comp=0, del_ete=0, co_py=0, help=0;
  int i, i1, i2, j, val, es_size=20;
  int loop_num1=0, loop_num2=0;
  char input1[1000]="", input2[1000]="", prefix[1000]="", 
      copy_elm[1000]="", del_elm[1000]="";

  // allocate 20
  n_struct1 = malloc(sizeof(elm_struct) * 20);
  n_struct2 = malloc(sizeof(elm_struct) * 20);
  n_struct_out = malloc(sizeof(elm_struct) * 20);

  /*******************************************************/
  /* parse aruguments (and read in files)*/
  if( argc < 2 ){
    help = 1;
  }
  while( nopt < argc ){
    if( strcmp(argv[nopt],"-print" ) == 0 ){
      pr_int=1; nopt++;
      if(comp || co_py || del_ete || help){
        fprintf(stderr,"\nError: too many options on command line!");
        return 1;
      }
      continue;
    }
    if( strcmp(argv[nopt],"-compare" ) == 0 ){
      comp=1; nopt++;
      if(pr_int || co_py || del_ete || help){
        fprintf(stderr,"\nError: too many options on command line!\n\n");
        return 1;
      }
      continue;
    }
    if( strcmp(argv[nopt],"-verb" ) == 0 ){
      verb=1; nopt++;
      if(pr_int == 0){
        fprintf(stderr,"\nError: -print missing!\n\n");
        return 1;
      }
      continue;
    }
    if( strcmp(argv[nopt],"-source" ) == 0 ){
      if(!THD_is_file(argv[nopt+1])){
        fprintf(stderr,"\nError: source does not exist!\n\n");
        return 1;
      }
      strcpy(input1,basename(argv[nopt+1]));
      nini1 = read_niml_file(argv[++nopt],1);
      nopt++; continue;
    }
    if( strcmp(argv[nopt],"-target" ) == 0 ){
      if(!THD_is_file(argv[nopt+1])){
        fprintf(stderr,"\nError: target does not exist!\n\n");
        return 1;
      }
      strcpy(input2,basename(argv[nopt+1]));
      nini2 = read_niml_file(argv[++nopt],1);
      nopt++; continue;
    }
    if( strcmp(argv[nopt],"-copy" ) == 0 ){
      co_py = 1;
      if(pr_int || comp || del_ete || help){
        fprintf(stderr,"\nError: too many options on command line!\n\n");
        return 1;
      }
      strcpy(copy_elm,argv[nopt+1]);
      nopt++; continue;
    }
    if( strcmp(argv[nopt],"-prefix" ) == 0 ){
      strcpy(prefix,argv[nopt+1]);
      nopt++; continue;
    }
    if( strcmp(argv[nopt],"-delete" ) == 0 ){
      del_ete = 1;
      if(pr_int || comp || co_py || help){
        fprintf(stderr,"\nError: too many options on command line!\n\n");
        return 1;
      }
      strcpy(del_elm,argv[nopt+1]);
      nopt++; continue;
    }
    if( strcmp(argv[nopt],"-help" ) == 0 ){
      help = 1;
      if(pr_int || comp || co_py || del_ete){
        fprintf(stderr,"\nError: too many options on command line!\n\n");
        return 1;
      }
      nopt++; continue;
    }
    nopt++;
  }

  // check to see if something was selected to do
  if(!(pr_int || comp || co_py || del_ete || help)) help = 1;
  if(strcmp(input1,"") == 0){
    fprintf(stderr,"\nError: missing -source!\n\n");
    return 1;
  }

  /*******************************************************/
  // print (if only 1 input, skip 2)
  if(pr_int){
    printf("\n");
    printf("------------%s-----------\n",input1);
    elm_print(nini1,0,verb);
    printf("\n");
    if(strcmp(input2,"") != 0){
      printf("------------%s-----------\n",input2);
      elm_print(nini2,0,verb);
    }
    printf("\n");
    return 0;
  }

  /*******************************************************/
  // delete  a named element from input1 to a copy of input2
  if(del_ete){
    // check if all needed arguments are present
    if(strcmp(input1,"") == 0){
      fprintf(stderr,"\nError: missing -source!\n\n");
      return 1;
    }
    if(strcmp(input2,"") == 0){
      fprintf(stderr,"\nError: no -target needed!\n\n");
      return 1;
    }
    if(strcmp(prefix,"") == 0){
      fprintf(stderr,"\nError: missing -prefix!\n\n");
      return 1;
    }

    // duplicate the source just to be safe
    ni_out = NI_duplicate(nini1,1);
    NI_set_attribute(ni_out, "self_idcode", UNIQ_idcode());

    // get the list of matching elements and how many
    val = NI_search_attr(ni_out,del_elm,&elm_list);

    // give error for more than 1 or none
    if(val > 1){
      fprintf(stderr,"\nError: multiple elements found!\n\n");
      for(i=0;i<val;i++){
        printf("%d. name %s\n",i,NI_element_name( elm_list[i]));
        if( NI_element_type(elm_list[i]) == NI_ELEMENT_TYPE){  // data
          NI_element *ni_err = (NI_element *) elm_list[i];
          for( j=0 ; j < ni_err->attr_num ; j++ ){
            printf("    %s = %s\n",ni_err->attr_lhs[j],ni_err->attr_rhs[j]);
          }
        }
      }
      return 1;
    } else if(val == 0){
      fprintf(stderr,"\nError: %s not found!\n\n",del_elm);
      return 1;
    } else if(val == 1){ // copy to output
      // find its parent, remove, write out, free
      NI_find_in_group(elm_list[0], ni_out, &ni_out_grp);
      NI_remove_from_group(ni_out_grp,elm_list[0]);
      printf("\nremoving %s from %s\n\n",del_elm,input1);
      write_niml_file(prefix,ni_out);
      NI_free_element(elm_list[0]);
      return 0;
    }
  }   // end delete element

  /*******************************************************/
  // copy a named element from input1 to a copy of input2

  if(co_py){
    // check if all needed arguments are present
    if(strcmp(input2,"") == 0){
      fprintf(stderr,"\nError: need -target to copy element!\n\n");
      return 1;
    }
    if(strcmp(prefix,"") == 0){
      fprintf(stderr,"\nError: missing -prefix!\n\n");
      return 1;
    }
    // get the list of matching elements and how many
    val = NI_search_attr(nini1,copy_elm,&elm_list);

    // give error for more than 1 or 0
    if(val > 1){
      fprintf(stderr,"\nError: multiple elements found!\n\n");
      for(i=0;i<val;i++){
        printf("%d. name %s\n",i,NI_element_name(elm_list[i]));
        if( NI_element_type(elm_list[i]) == NI_ELEMENT_TYPE ){  // data
          NI_element *ni_err = (NI_element *) elm_list[i];
          for( j=0 ; j < ni_err->attr_num ; j++ ){
            printf("    %s = %s\n",ni_err->attr_lhs[j],ni_err->attr_rhs[j]);
          }
        }
      }
      return 1;
    } else if(val == 0){
      fprintf(stderr,"\nError: %s not found!\n\n",copy_elm);
      return 1;
    } else if(val == 1){ // copy to output
      // duplicate the target
      ni_out = NI_duplicate(nini2,1);
      NI_set_attribute(ni_out, "self_idcode", UNIQ_idcode());

      // find the group for the element in the source and see if it is in target
      NI_find_in_group(elm_list[0], nini1, &ni_in_grp);

      if(NI_find_in_group(ni_in_grp, ni_out, &ni_out_grp)){
        // add to the output
        NI_add_to_group(ni_out_grp,elm_list[0]);
        printf("\ncopying %s to %s\n",NI_element_name(elm_list[0]),prefix);
        write_niml_file(prefix,ni_out);
        printf("copy complete\n\n");
      } else {
        fprintf(stderr,"\nError: matching group/parent not found in target!\n\n");
        return 1;
      }
      return 0;
    }
  }   // end copy

  /*******************************************************/
  // sorting and matching for comparison

  if(comp){

    // add to arrays, reseting global counts
    loop_num = 0;
    loop_num1 = elm_arr(nini1,&n_struct1,0,nini1,20);
    loop_num = 0;
    loop_num2 = elm_arr(nini2,&n_struct2,0,nini2,20);

    if (loop_num1 >= loop_num2){
      loop_max = loop_num1;
    } else {
      loop_max = loop_num2;
    }

    // sort both alphabetically
    qsort( n_struct1,loop_num1, sizeof( elm_struct ), name_compare );
    qsort( n_struct2,loop_num2, sizeof( elm_struct ), name_compare );

    /*******************************************************/
    // diff/match loop

# define nini1_match() \
strcpy(n_struct_out[i].name,n_struct1[i1].name);  \
n_struct_out[i].elm_num = n_struct1[i1].elm_num;  \
n_struct_out[i].ni_elm = n_struct1[i1].ni_elm;  \
n_struct_out[i].ni_grp = n_struct1[i1].ni_grp;  \
n_struct_out[i].parent = n_struct1[i1].parent;  \
strcpy(n_struct_out[i].par_name,n_struct1[i1].par_name);

# define nini2_match() \
strcpy(n_struct_out[i].name2,n_struct2[i2].name); \
n_struct_out[i].elm_num2 = n_struct2[i2].elm_num; \
n_struct_out[i].ni_elm2 = n_struct2[i2].ni_elm; \
n_struct_out[i].ni_grp2 = n_struct2[i2].ni_grp; \
n_struct_out[i].parent2 = n_struct2[i2].parent; \
strcpy(n_struct_out[i].par_name2,n_struct2[i2].par_name);

# define nini1_empty() \
strcpy(n_struct_out[i].name,"");  \
n_struct_out[i].elm_num = 9999; \
n_struct_out[i].ni_elm = NULL;  \
n_struct_out[i].ni_grp = NULL;  \
n_struct_out[i].parent = NULL;  \
strcpy(n_struct_out[i].par_name,"");

# define nini2_empty() \
strcpy(n_struct_out[i].name2,""); \
n_struct_out[i].elm_num2 = 9999;  \
n_struct_out[i].ni_elm2 = NULL; \
n_struct_out[i].ni_grp2 = NULL; \
n_struct_out[i].parent2 = NULL; \
strcpy(n_struct_out[i].par_name2,"");

    i1 = 0; i2 = 0; i = 0;
    while (NotDone) {
      // they match
      while (i2 < loop_num2 && i1 < loop_num1
          && strcmp(n_struct1[i1].name, n_struct2[i2].name) == 0) {
        if( i > es_size ) {
          es_size = es_size + 20;
          n_struct_out = realloc(n_struct_out, sizeof(elm_struct) * es_size);
        }
        nini1_match(); nini2_match();
//        printf("%d %d == %d\n", i, n_struct_out[i].elm_num,
//            n_struct_out[i].elm_num2);
//        printf("%s == %s\n", n_struct1[i1].name, n_struct2[i2].name);
//        printf("%d %d  %d %d %d\n",i,i1,i2,loop_num1, loop_num2);
        max_arr_len = i;
        i++; i1++; i2++;
      }  // end while equal

      // input1 < input2
      while(i2 < loop_num2 && i1 < loop_num1 && 
          strcmp(n_struct1[i1].name, n_struct2[i2].name) < 0){
        if(i > es_size){
          es_size = es_size + 20;
          n_struct_out = realloc(n_struct_out, sizeof(elm_struct) * es_size );
        }
        nini1_match(); nini2_empty();
//        printf("%d %d < %d\n",i,n_struct_out[i].elm_num,n_struct_out[i].elm_num2);
//        printf("%s < %s\n",n_struct1[i1].name, n_struct2[i2].name);
//        printf("%d %d  %d %d %d\n",i,i1,i2,loop_num1, loop_num2);
        max_arr_len = i;
        i++; i1++;       
      }  // end while <

      // input1 > input2
      while(i2 < loop_num2 && i1 < loop_num1 &&
            strcmp(n_struct1[i1].name, n_struct2[i2].name) > 0){
        if(i > es_size){
          es_size = es_size + 20;
          n_struct_out = realloc(n_struct_out, sizeof(elm_struct) * es_size );
        }
        nini2_match(); nini1_empty();
//                printf("%d %d > %d\n",i,n_struct_out[i].elm_num,n_struct_out[i].elm_num2);
//                printf("%s > %s\n",n_struct1[i1].name, n_struct2[i2].name);
//                printf("%d %d  %d %d %d\n",i,i1,i2,loop_num1, loop_num2);
        max_arr_len = i;
        i++; i2++;
      }  // end while >

      // if both are at the end (they are all equal)
      if(i2 >= loop_num2 && i1 >= loop_num1){
        NotDone = 0;
      } else {
        // if there are any remaining at the end, fill in both
        if(i1 < loop_num1 && i2 >= loop_num2) {
          while(i1 < loop_num1){
            if(i > es_size){
              es_size = es_size + 20;
              n_struct_out = realloc(n_struct_out, sizeof(elm_struct) * es_size );
            }
            nini1_match(); nini2_empty();
//            printf("%d %d :1 %d\n",i,n_struct_out[i].elm_num,n_struct_out[i].elm_num2);
//            printf("%s :1 %s\n",n_struct1[i1].name, n_struct2[i2].name);
//            printf("%d %d  %d %d %d\n",i,i1,i2,loop_num1, loop_num2);
            max_arr_len = i;
            i++; i1++;
          }
          NotDone = 0;
        }
        if(i2 < loop_num2 && i1 >= loop_num1) {
          while(i2 < loop_num2){
            if(i > es_size){
              es_size = es_size + 20;
              n_struct_out = realloc(n_struct_out, sizeof(elm_struct) * es_size );
            }
            nini2_match(); nini1_empty();
//            printf("%d %d :2 %d\n",i,n_struct_out[i].elm_num,n_struct_out[i].elm_num2);
//            printf("%s :2 %s\n",n_struct1[i1].name, n_struct2[i2].name);
//            printf("%d %d  %d %d %d\n",i,i1,i2,loop_num1, loop_num2);
            max_arr_len = i;
            i++; i2++;
          }
          NotDone = 0;
        }
      }  // end if stuff is left
    } // end diff while loop

    // sort back to original order of input1 then input2
    qsort( n_struct_out,max_arr_len+1, sizeof( elm_struct ), elm_compare );

    // get the spacing
    if(max_name_len <= (int)strlen(input1)) max_name_len = (int)strlen(input1);

    /*******************************************************/
    // print out the matched files (or just one)

    printf("\n%s:",input1);
    if(strcmp(input2,"") != 0){
      printf("%*s",max_name_len+3-(int)strlen(input1),"");
      printf("%s:\n\n",input2);
    } else {
      printf("\n\n");
    }
    for(i=0;i<max_arr_len+1;i++) {
      printf("%s",n_struct_out[i].name);
      if(strcmp(input2,"") != 0){
        printf("%*s",max_name_len+4-(int)strlen(n_struct_out[i].name),"");
        printf("%s\n",n_struct_out[i].name2);
      } else {
        printf("\n");
      }
    }
    printf("\n");

//    // save for checking ordering
//    printf("\n");
//    printf("----------------\n\n");
//    printf("%s","input1:");
//    printf("%*s",max_name_len+4-(int)strlen("input1:"),"");
//    printf("%s\n","input2:");
//
//    for(i=0;i<=max_arr_len;i++) {
//      printf("%d",n_struct_out[i].elm_num);
//      printf("%*s",max_name_len+4-4,"");
//      printf("%d\n",n_struct_out[i].elm_num2);
//
//    }
    return 0;
  }   // end compare

  if(help){
    printf("\n\n"
           "usage: niml_tool -source ss [options]\n");
    printf("       Reads in niml file(s) and does various actions.\n"
           "       Copy and delete only work if there is ONE element that matches\n"
           "       the search term.\n");
    printf("\n"
           "Options:\n"
           "  -source ss  = Specify input niml file. Required for all options\n"
           "  -target tt  = Specify second niml file. Required for -copy.\n"
           "                It is optional for -compare and -print.\n"
           "                It must not be used for -delete.\n"
           "  -prefix pp  = Specify the output file name. Required for -copy and -delete.\n"
           "  -print      = Prints out names of the elements and the structure.\n"
           "                If there is a -target, it will print for both.\n"
           "  -verb       = Prints out all attributes. Must have -print as well.\n"
           "  -compare    = Prints out a list of groups and elements within -source.\n"
           "                If -target is specified, both will be printed and sorted by\n"
           "                matchined elements. The structure is x0:0:name:name2:etc..\n"
           "                'x' can be 'g' for group, 'e' for element, or 'p' for\n"
           "                processing instructions. The first digit is the group level.\n"
           "                The second digit is the absolute order in the file.\n"
           "  -copy elm   = Copies 'elm' from the source to a copy of the target.\n"
           "                'elm' is a string that matches some attribute of an element\n"
           "                within the source file. Requires both -source and -target.\n"
           "                The group to which the element belongs must also be in \n"
           "                the -target. If more than one element is found matching\n"
           "                'elm', this will fail.\n"
           "  -delete elm = Removes 'elm' from a duplicate of the source.\n");
    printf("------------------------------\n"
           "examples:\n"
           "  niml_tool -source test1.niml.dset -target test2.niml.dset -compare\n"
           "  niml_tool -source test1.niml.dset -target test2.niml.dset -print\n"
           "  niml_tool -source test1.niml.dset -target test2.niml.dset \\ \n"
           "            -copy LabelTableObject_data -prefix out.niml.dset\n"
           "  niml_tool -source test1.niml.dset -delete LabelTableObject_data \\ \n"
           "            -prefix out.niml.dset\n");
    printf("------------------------------\n"
           "DiscoRaj May 2017\n\n");

    return 0;
  }   // end help

}   // end main
