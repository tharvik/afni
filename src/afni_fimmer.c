#undef MAIN

#include "afni.h"
#include "editvol.h"

#ifdef AFNI_DEBUG
#  define USE_TRACING
#  define PRINT_TRACING
#endif
#include "dbtrace.h"

/*--------------------------------------------------------------------
  Find out if a timeseries is in the library;
  if it is, return its index, otherwise return -1.
----------------------------------------------------------------------*/

int AFNI_ts_in_library( MRI_IMAGE * tsim )
{
   int its ;

   if( GLOBAL_library.timeseries != NULL         &&
      IMARR_COUNT(GLOBAL_library.timeseries) > 0 && tsim != NULL ){

      for( its=0 ; its < IMARR_COUNT(GLOBAL_library.timeseries) ; its++ )
         if( tsim == IMARR_SUBIMAGE(GLOBAL_library.timeseries,its) )
            return its ;
   }

   return -1 ;
}

/*--------------------------------------------------------------------*/

void AFNI_fimmer_pickref_CB( Widget wcall ,
                             XtPointer cd , MCW_choose_cbs * cbs )
{
   Three_D_View * im3d = (Three_D_View *) cd ;
   int its ;
   MRI_IMAGE * tsim ;

ENTRY("AFNI_fimmer_pickref_CB") ;

   if( ! IM3D_VALID(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;
   if( cbs->reason != mcwCR_timeseries ) EXRETURN ;  /* error */

   its = cbs->ival ;
   if( its >= 0 && its < IMARR_COUNT(GLOBAL_library.timeseries) ){

      tsim = IMARR_SUBIMAGE(GLOBAL_library.timeseries,its) ;
      AFNI_fimmer_setref( im3d , tsim ) ;
      im3d->fimdata->refadd_count = 1 ;
   }

   EXRETURN ;
}

/*--------------------------------------------------------------------*/

void AFNI_fimmer_pickort_CB( Widget wcall ,
                             XtPointer cd , MCW_choose_cbs * cbs )
{
   Three_D_View * im3d = (Three_D_View *) cd ;
   int its ;
   MRI_IMAGE * tsim ;

ENTRY("AFNI_fimmer_pickort_CB") ;

   if( ! IM3D_VALID(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;
   if( cbs->reason != mcwCR_timeseries ) EXRETURN ;  /* error */

   its = cbs->ival ;
   if( its >= 0 && its < IMARR_COUNT(GLOBAL_library.timeseries) ){

      tsim = IMARR_SUBIMAGE(GLOBAL_library.timeseries,its) ;
      AFNI_fimmer_setort( im3d , tsim ) ;
   }

   EXRETURN ;
}

/*--------------------------------------------------------------------*/

void AFNI_fimmer_setupdate_CB( Widget wcall ,
                               XtPointer cd , MCW_choose_cbs * cbs )
{
   Three_D_View * im3d = (Three_D_View *) cd ;

ENTRY("AFNI_fimmer_setupdate_CB") ;

   if( ! IM3D_VALID(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;
   if( cbs->reason != mcwCR_integer ) EXRETURN ;

   im3d->vinfo->fimmer_update_frequency = cbs->ival ;

   if( DSET_GRAPHABLE(im3d->anat_now) )
      im3d->fimdata->fimdset = im3d->anat_now ;

   ALLOW_COMPUTE_FIM(im3d) ;
   EXRETURN ;
}

/*-------------------------------------------------------------------
   Pass this a timeseries that will not be deleted!
---------------------------------------------------------------------*/

void AFNI_fimmer_setref( Three_D_View * im3d , MRI_IMAGE * tsim )
{
   int ii ;

ENTRY("AFNI_fimmer_setref") ;

   if( ! IM3D_VALID(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;

#ifdef AFNI_DEBUG
{ char str[256] ;
  sprintf(str,"setting fimref to %s",
     (tsim==NULL) ? "Nothing" : (tsim->name==NULL) ? "NoName" : tsim->name) ;
  STATUS(str) ; }
#endif

   if( im3d->g123 != NULL )
      drive_MCW_grapher( im3d->g123 , graDR_addref_ts , (XtPointer) tsim ) ;

   if( im3d->g231 != NULL )
      drive_MCW_grapher( im3d->g231 , graDR_addref_ts , (XtPointer) tsim ) ;

   if( im3d->g312 != NULL )
      drive_MCW_grapher( im3d->g312 , graDR_addref_ts , (XtPointer) tsim ) ;

   ii = AFNI_ts_in_library( tsim ) ;

#ifdef AFNI_DEBUG
{ char str[256] ; sprintf(str,"found new ref in library at ii=%d",ii) ;
  STATUS(str) ; }
#endif

   ii = AFNI_ts_in_library( im3d->fimdata->fimref ) ;

   /* 12 Nov 1996: fix problem with freeing old
                   ref that might be in the library */

#ifdef AFNI_DEBUG
{ char str[256] ; sprintf(str,"found old ref in library at ii=%d",ii) ;
  STATUS(str) ; }
#endif

   if( ii < 0 && im3d->fimdata->fimref != NULL ){
      mri_free(im3d->fimdata->fimref) ;
STATUS("freed old ref since wasn't in library") ;
   }

   im3d->fimdata->fimref = tsim ;

   if( DSET_GRAPHABLE(im3d->anat_now) )
      im3d->fimdata->fimdset = im3d->anat_now ;

   ALLOW_COMPUTE_FIM(im3d) ;
   EXRETURN ;
}

/*----------------------------------------------------------------------------*/

void AFNI_fimmer_setort( Three_D_View * im3d , MRI_IMAGE * tsim )
{
   int ii ;

ENTRY("AFNI_fimmer_setort") ;

   if( ! IM3D_VALID(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;

#ifdef AFNI_DEBUG
{ char str[256] ;
  sprintf(str,"setting fimort to %s",
     (tsim==NULL) ? "Nothing" : (tsim->name==NULL) ? "NoName" : tsim->name) ;
  STATUS(str) ; }
#endif

   if( im3d->g123 != NULL )
      drive_MCW_grapher( im3d->g123 , graDR_addort_ts , (XtPointer) tsim ) ;

   if( im3d->g231 != NULL )
      drive_MCW_grapher( im3d->g231 , graDR_addort_ts , (XtPointer) tsim ) ;

   if( im3d->g312 != NULL )
      drive_MCW_grapher( im3d->g312 , graDR_addort_ts , (XtPointer) tsim ) ;

   ii = AFNI_ts_in_library( tsim ) ;

#ifdef AFNI_DEBUG
{ char str[256] ; sprintf(str,"found new ort in library at ii=%d",ii) ;
  STATUS(str) ; }
#endif

   ii = AFNI_ts_in_library( im3d->fimdata->fimort ) ;

   /* 12 Nov 1996: fix problem with freeing old
                   ort that might be in the library */

#ifdef AFNI_DEBUG
{ char str[256] ; sprintf(str,"found old ort in library at ii=%d",ii) ;
  STATUS(str) ; }
#endif

   if( ii < 0 && im3d->fimdata->fimort != NULL ){
      mri_free(im3d->fimdata->fimort) ;
STATUS("freed old ort since wasn't in library") ;
   }

   im3d->fimdata->fimort = tsim ;

   if( DSET_GRAPHABLE(im3d->anat_now) )
      im3d->fimdata->fimdset = im3d->anat_now ;

   ALLOW_COMPUTE_FIM(im3d) ;
   EXRETURN ;
}

/*-------------------------------------------------------------------
  Set the number of points to ignore at the beginning of time
---------------------------------------------------------------------*/

void AFNI_fimmer_setignore( Three_D_View * im3d , int new_ignore )
{
   int ii ;

ENTRY("AFNI_fimmer_setignore") ;

   if( ! IM3D_VALID(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;

   if( im3d->g123 != NULL )
      drive_MCW_grapher( im3d->g123 , graDR_setignore , (XtPointer) new_ignore ) ;

   if( im3d->g231 != NULL )
      drive_MCW_grapher( im3d->g231 , graDR_setignore , (XtPointer) new_ignore ) ;

   if( im3d->g312 != NULL )
      drive_MCW_grapher( im3d->g312 , graDR_setignore , (XtPointer) new_ignore ) ;

   im3d->fimdata->init_ignore = new_ignore ;

   if( DSET_GRAPHABLE(im3d->anat_now) )
      im3d->fimdata->fimdset = im3d->anat_now ;

   ALLOW_COMPUTE_FIM(im3d) ;
   EXRETURN ;
}


/*-------------------------------------------------------------------
   Lots of CPU work in here!
---------------------------------------------------------------------*/

#define FIM_THR  0.0999

THD_3dim_dataset * AFNI_fimmer_compute( Three_D_View * im3d ,
                                        THD_3dim_dataset * dset_time ,
                                        MRI_IMAGE * ref_ts , MRI_IMAGE * ort_ts ,
                                        THD_session * sess ,
                                        int update_frequency )
{
   THD_3dim_dataset * new_dset ;
   char new_prefix[THD_MAX_PREFIX] ;
   char old_prefix[THD_MAX_PREFIX] ;
   THD_slist_find fff ;
   int ifim , it,iv , nvox , ngood_ref , ntime , it1 , dtyp , nxyz , itbot ;
   float * vval , * tsar , * aval , * rbest , * abest ;
   int   * indx ;
   short * bar ;
   void  * ptr ;
   float stataux[MAX_STAT_AUX] ;
   float fthr , topval ;
   int nx_ref , ny_ref , ivec , nnow ;
   PCOR_references ** pc_ref ;
   PCOR_voxel_corr ** pc_vc ;
   Boolean save_xhairs , save_marks ;
   int save_resam ;

   int fim_nref , nx_ort , ny_ort , internal_ort ;
   float * ortar ;
   static float * ref_vec = NULL ;
   static int    nref_vec = -666 ;

#ifndef DONT_USE_METER
   Widget meter = NULL ;
   int meter_perc , meter_pold ;
#endif

   int nupdt      = 0 ,  /* number of updates done yet */
       min_updt   = 5 ,  /* min number needed for display */
       first_updt = 1 ;  /* flag to indicate that first update is yet to be displayed */

ENTRY("AFNI_fimmer_compute") ;

   /*--- check for legal inputs ---*/

   if( ! DSET_GRAPHABLE(dset_time)    ||
       ref_ts == NULL                 ||
       ref_ts->kind != MRI_float      ||
       ! IM3D_OPEN(im3d)              ||
       im3d->type != AFNI_3DDATA_VIEW ||
       ref_ts->nx < DSET_NUM_TIMES(dset_time) ){

#ifdef AFNI_DEBUG
{ char str[256] ;
  sprintf(str,"illegal inputs: ntime=%d num_ts=%d",
          DSET_NUM_TIMES(dset_time), (ref_ts==NULL) ? (0) : (ref_ts->nx) ) ;
  STATUS(str) ; }
#endif

      RETURN(NULL) ;
   }

   /** 13 Nov 1996: allow for orts **/

   if( ort_ts != NULL ){
      nx_ort = ort_ts->nx ;
      ny_ort = ort_ts->ny ;
      ortar  = MRI_FLOAT_PTR(ort_ts) ;

      internal_ort = (nx_ort < DSET_NUM_TIMES(dset_time)) ;
   } else {
      internal_ort = 1 ;
   }
   fim_nref = (internal_ort) ? 3 : (ny_ort+3) ;

   if( nref_vec < fim_nref ){
       ref_vec = (float *) XtRealloc( (char *)ref_vec, sizeof(float)*fim_nref ) ;
      nref_vec = fim_nref ;
   }

   itbot     = im3d->fimdata->init_ignore ;
   nx_ref    = ref_ts->nx ;
   ny_ref    = ref_ts->ny ;
   ntime     = DSET_NUM_TIMES(dset_time) ;
   ngood_ref = 0 ;
   it1       = -1 ;
   for( ivec=0 ; ivec < ny_ref ; ivec++ ){
      tsar = MRI_FLOAT_PTR(ref_ts) + (ivec*nx_ref) ;
      ifim = 0 ;
      for( it=itbot ; it < ntime ; it++ ){
         if( tsar[it] < WAY_BIG ){ ifim++ ; if( it1 < 0 ) it1 = it ; }
      }

      if( ifim < min_updt ){
         STATUS("ref_ts has too few good entries!") ;
         RETURN(NULL) ;
      }

      ngood_ref = MAX( ifim , ngood_ref ) ;
   }

   /** at this point, ngood_ref = max number of good reference points,
       and                  it1 = index of first point used in first reference **/

   dtyp = DSET_BRICK_TYPE(dset_time,it1) ;
   if( ! AFNI_GOOD_FUNC_DTYPE(dtyp) ){
      STATUS("illegal input data type!") ;
      RETURN(NULL) ;
   }

   /*--- Create a new prefix ---*/

   MCW_strncpy( old_prefix , DSET_PREFIX(dset_time) , THD_MAX_PREFIX-3 ) ;

   if( ! ISVALID_SESSION(sess) ){
      sprintf( new_prefix , "%s@%d" , old_prefix , 1 ) ;
      update_frequency = 0 ;  /* can't update without session */
   } else {
      for( ifim=1 ; ifim < 99 ; ifim++ ){
         sprintf( new_prefix , "%s@%d" , old_prefix , ifim ) ;
         fff = THD_dset_in_session( FIND_PREFIX , new_prefix , sess ) ;
         if( fff.dset == NULL ) break ;
      }
      if( ifim == 99 ){
         STATUS("can't create new prefix!") ;
         RETURN(NULL) ;  /* can't make a new prefix! */
      }
   }

#ifdef AFNI_DEBUG
{ char str[256] ;
  sprintf(str,"new prefix = %s",new_prefix) ; STATUS(str) ; }
#endif

   /*--- FIM: find values above threshold to fim ---*/

   THD_load_datablock( dset_time->dblk , AFNI_purge_unused_dsets ) ;

   nxyz =  dset_time->dblk->diskptr->dimsizes[0]
         * dset_time->dblk->diskptr->dimsizes[1]
         * dset_time->dblk->diskptr->dimsizes[2] ;

   /** find the mean of the first array,
       compute the threshold (fthr) from it,
       make indx[i] be the 3D index of the i-th voxel above threshold **/

   switch( dtyp ){

      case MRI_short:{
         short * dar = (short *) DSET_ARRAY(dset_time,it1) ;
         for( iv=0,fthr=0.0 ; iv < nxyz ; iv++ ) fthr += abs(dar[iv]) ;
         fthr = FIM_THR * fthr / nxyz ;
         for( iv=0,nvox=0 ; iv < nxyz ; iv++ )
            if( abs(dar[iv]) > fthr ) nvox++ ;
         indx = (int *) malloc( sizeof(int) * nvox ) ;
         if( indx == NULL ){
            fprintf(stderr,"\n*** indx malloc failure in AFNI_fimmer_compute\n") ;
            RETURN(NULL) ;
         }
         for( iv=0,nvox=0 ; iv < nxyz ; iv++ )
            if( abs(dar[iv]) > fthr ) indx[nvox++] = iv ;
      }
      break ;

      case MRI_float:{
         float * dar = (float *) DSET_ARRAY(dset_time,it1) ;
         for( iv=0,fthr=0.0 ; iv < nxyz ; iv++ ) fthr += fabs(dar[iv]) ;
         fthr = FIM_THR * fthr / nxyz ;
         for( iv=0,nvox=0 ; iv < nxyz ; iv++ )
            if( fabs(dar[iv]) > fthr ) nvox++ ;
         indx = (int *) malloc( sizeof(int) * nvox ) ;
         if( indx == NULL ){
            fprintf(stderr,"\n*** indx malloc failure in AFNI_fimmer_compute\n") ;
            RETURN(NULL) ;
         }
         for( iv=0,nvox=0 ; iv < nxyz ; iv++ )
            if( fabs(dar[iv]) > fthr ) indx[nvox++] = iv ;
      }
      break ;

      case MRI_byte:{
         byte * dar = (byte *) DSET_ARRAY(dset_time,it1) ;
         for( iv=0,fthr=0.0 ; iv < nxyz ; iv++ ) fthr += dar[iv] ;
         fthr = FIM_THR * fthr / nxyz ;
         for( iv=0,nvox=0 ; iv < nxyz ; iv++ )
            if( dar[iv] > fthr ) nvox++ ;
         indx = (int *) malloc( sizeof(int) * nvox ) ;
         if( indx == NULL ){
            fprintf(stderr,"\n*** indx malloc failure in AFNI_fimmer_compute\n") ;
            RETURN(NULL) ;
         }
         for( iv=0,nvox=0 ; iv < nxyz ; iv++ )
            if( dar[iv] > fthr ) indx[nvox++] = iv ;
      }
      break ;
   }

   /** allocate space for voxel values **/

   vval = (float *) malloc( sizeof(float) * nvox) ;
   if( vval == NULL ){
      fprintf(stderr,"\n*** vval malloc failure in AFNI_fimmer_compute\n") ;
      free(indx) ; RETURN(NULL) ;
   }

   /** allocate extra space for comparing results from multiple ref vectors **/

   if( ny_ref > 1 ){
      aval  = (float *) malloc( sizeof(float) * nvox) ;
      rbest = (float *) malloc( sizeof(float) * nvox) ;
      abest = (float *) malloc( sizeof(float) * nvox) ;
      if( aval==NULL || rbest==NULL || abest==NULL ){
         fprintf(stderr,"\n*** abest malloc failure in AFNI_fimmer_compute\n") ;
         free(vval) ; free(indx) ;
         if( aval  != NULL ) free(aval) ;
         if( rbest != NULL ) free(rbest) ;
         if( abest != NULL ) free(abest) ;
         RETURN(NULL) ;
      }
   } else {
      aval = rbest = abest = NULL ;
   }

#ifdef AFNI_DEBUG
{ char str[256] ;
  sprintf(str,"nxyz = %d  nvox = %d",nxyz,nvox) ; STATUS(str) ; }
#endif

   /*--- FIM: initialize recursive updates ---*/

   pc_ref = (PCOR_references **) malloc( sizeof(PCOR_references *) * ny_ref ) ;
   pc_vc  = (PCOR_voxel_corr **) malloc( sizeof(PCOR_voxel_corr *) * ny_ref ) ;

   if( pc_ref == NULL || pc_vc == NULL ){
      free(vval) ; free(indx) ; free(pc_ref) ; free(pc_vc) ;
      if( aval  != NULL ) free(aval) ;
      if( rbest != NULL ) free(rbest) ;
      if( abest != NULL ) free(abest) ;
      fprintf(stderr,"\n*** FIM initialization fails in AFNI_fimmer_compute\n") ;
      RETURN(NULL) ;
   }

   ifim = 0 ;
   for( ivec=0 ; ivec < ny_ref ; ivec++ ){
      pc_ref[ivec] = new_PCOR_references( fim_nref ) ;
      pc_vc[ivec]  = new_PCOR_voxel_corr( nvox , fim_nref ) ;
      if( pc_ref[ivec] == NULL || pc_vc[ivec] == NULL ) ifim++ ;
   }

   if( ifim > 0 ){
      for( ivec=0 ; ivec < ny_ref ; ivec++ ){
         free_PCOR_references(pc_ref[ivec]) ;
         free_PCOR_voxel_corr(pc_vc[ivec]) ;
      }
      free(vval) ; free(indx) ; free(pc_ref) ; free(pc_vc) ;
      if( aval  != NULL ) free(aval) ;
      if( rbest != NULL ) free(rbest) ;
      if( abest != NULL ) free(abest) ;
      fprintf(stderr,"\n*** FIM initialization fails in AFNI_fimmer_compute\n") ;
      RETURN(NULL) ;
   }

   /*--- Make a new dataset to hold the output ---*/

   new_dset = EDIT_empty_copy( dset_time ) ;

   it = EDIT_dset_items( new_dset ,
                            ADN_prefix      , new_prefix ,
                            ADN_malloc_type , DATABLOCK_MEM_MALLOC ,
                            ADN_type        , ISHEAD(dset_time)
                                              ? HEAD_FUNC_TYPE : GEN_FUNC_TYPE ,
                            ADN_func_type   , FUNC_COR_TYPE ,
                            ADN_nvals       , FUNC_nvals[FUNC_COR_TYPE] ,
                            ADN_datum_all   , MRI_short ,
                            ADN_ntt         , 0 ,
                         ADN_none ) ;

   if( it > 0 ){
      fprintf(stderr,
              "\n*** EDIT_dset_items error %d in AFNI_fimmer_compute\n",it) ;
      THD_delete_3dim_dataset( new_dset , False ) ;
      for( ivec=0 ; ivec < ny_ref ; ivec++ ){
         free_PCOR_references(pc_ref[ivec]) ;
         free_PCOR_voxel_corr(pc_vc[ivec]) ;
      }
      free(vval) ; free(indx) ; free(pc_ref) ; free(pc_vc) ;
      if( aval  != NULL ) free(aval) ;
      if( rbest != NULL ) free(rbest) ;
      if( abest != NULL ) free(abest) ;
      RETURN(NULL) ;
   }

   for( iv=0 ; iv < new_dset->dblk->nvals ; iv++ ){
      ptr = malloc( DSET_BRICK_BYTES(new_dset,iv) ) ;
      mri_fix_data_pointer( ptr ,  DSET_BRICK(new_dset,iv) ) ;
   }

   if( THD_count_databricks(new_dset->dblk) < new_dset->dblk->nvals ){
      fprintf(stderr,
              "\n*** failure to malloc new bricks in AFNI_fimmer_compute\n") ;
      THD_delete_3dim_dataset( new_dset , False ) ;
      for( ivec=0 ; ivec < ny_ref ; ivec++ ){
         free_PCOR_references(pc_ref[ivec]) ;
         free_PCOR_voxel_corr(pc_vc[ivec]) ;
      }
      free(vval) ; free(indx) ; free(pc_ref) ; free(pc_vc) ;
      if( aval  != NULL ) free(aval) ;
      if( rbest != NULL ) free(rbest) ;
      if( abest != NULL ) free(abest) ;
      RETURN(NULL) ;
   }

   /*--- FIM: do recursive updates ---*/

#ifndef DONT_USE_METER
   meter = MCW_popup_meter( im3d->vwid->top_shell , METER_TOP_WIDE ) ;
   meter_pold = 0 ;
#endif

   if( update_frequency > 0 ){
      AFNI_add_interruptable( NULL ) ;                        /* clear */
      AFNI_add_interruptable( im3d->vwid->func->thr_scale ) ; /* setup */
   }

   /** save crosshairs and marks visibility states **/

   save_xhairs = im3d->vinfo->crosshair_visible ;
   save_marks  = im3d->vwid->marks->ov_visible ;
   save_resam  = im3d->vinfo->func_resam_mode ;

   for( it=itbot ; it < ntime ; it++ ){

      nnow = 0 ;
      for( ivec=0 ; ivec < ny_ref ; ivec++ ){
         tsar = MRI_FLOAT_PTR(ref_ts) + (ivec*nx_ref) ;
         if( tsar[it] >= WAY_BIG ) continue ;  /* skip this */

         ref_vec[0] = 1.0 ;         /* we always supply orts */
         ref_vec[1] = (float) it ;  /* for mean and linear trend */

         if( internal_ort ){
            ref_vec[2] = tsar[it] ;
         } else {
            for( iv=0 ; iv < ny_ort ; iv++ )
               ref_vec[iv+2] = ortar[it + iv*nx_ort] ;

            ref_vec[ny_ort+2] = tsar[it] ;
         }

#ifdef AFNI_DEBUG
{ char str[256] ;
  sprintf(str,"time index=%d  ideal[%d]=%f" , it,ivec,tsar[it] ) ;
  STATUS(str) ; }
#endif

         update_PCOR_references( ref_vec , pc_ref[ivec] ) ;

         if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

         switch( dtyp ){
            case MRI_short:{
               short * dar = (short *) DSET_ARRAY(dset_time,it) ;
               for( iv=0 ; iv < nvox ; iv++ ) vval[iv] = (float) dar[indx[iv]] ;
            }
            break ;

            case MRI_float:{
               float * dar = (float *) DSET_ARRAY(dset_time,it) ;
               for( iv=0 ; iv < nvox ; iv++ ) vval[iv] = (float) dar[indx[iv]] ;
            }
            break ;

            case MRI_byte:{
               byte * dar = (byte *) DSET_ARRAY(dset_time,it) ;
               for( iv=0 ; iv < nvox ; iv++ ) vval[iv] = (float) dar[indx[iv]] ;
            }
            break ;
         }

         if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

         PCOR_update_float( vval , pc_ref[ivec] , pc_vc[ivec] ) ;
         nnow++ ;

         if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;
      }
      if( nnow > 0 ) nupdt++ ;

#ifndef DONT_USE_METER
      meter_perc = (int) ( 100.0 * nupdt / ngood_ref ) ;
      if( meter_perc != meter_pold ){
         MCW_set_meter( meter , meter_perc ) ;
         meter_pold = meter_perc ;
      }
#endif

      /*--- Load results into the dataset and redisplay it ---*/

      if( nupdt == ngood_ref ||
          (update_frequency>0 && nupdt>=min_updt && nupdt%update_frequency==0) ){

         /*--- set the statistical parameters ---*/

         stataux[0] = nupdt ;               /* number of points used */
         stataux[1] = (ny_ref==1) ? 1 : 2 ; /* number of references  */
         stataux[2] = fim_nref - 1 ;        /* number of orts        */
         for( iv=3 ; iv < MAX_STAT_AUX ; iv++ ) stataux[iv] = 0.0 ;

STATUS("setting statistical parameters") ;

         (void) EDIT_dset_items( new_dset ,
                                    ADN_stat_aux , stataux ,
                                 ADN_none ) ;

         if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

         /*** Compute brick arrays for new dataset ***/

         if( ny_ref == 1 ){

         /*** Just 1 ref vector --> load values directly into dataset ***/

            /*--- get alpha (coef) into vval,
                  find max value, scale into brick array ---*/

STATUS("getting 1 ref alpha") ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            PCOR_get_coef( pc_ref[0] , pc_vc[0] , vval ) ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            topval = 0.0 ;
            for( iv=0 ; iv < nvox ; iv++ )
               if( fabs(vval[iv]) > topval ) topval = fabs(vval[iv]) ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            bar = DSET_ARRAY( new_dset , FUNC_ival_fim[FUNC_COR_TYPE] ) ;

#ifdef DONT_USE_MEMCPY
            for( iv=0 ; iv < nxyz ; iv++ ) bar[iv] = 0 ;
#else
            memset( bar , 0 , sizeof(short)*nxyz ) ;
#endif

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            if( topval > 0.0 ){
               topval = MRI_TYPE_maxval[MRI_short] / topval ;
               for( iv=0 ; iv < nvox ; iv++ )
                  bar[indx[iv]] = (short)(topval * vval[iv] + 0.499) ;

               stataux[0] = 1.0/topval ;
            } else {
               stataux[0] = 0.0 ;
            }

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            /*--- get correlation coefficient (pcor) into vval,
                  scale into brick array (with fixed scaling factor) ---*/

STATUS("getting 1 ref pcor") ;

            PCOR_get_pcor( pc_ref[0] , pc_vc[0] , vval ) ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            bar = DSET_ARRAY( new_dset , FUNC_ival_thr[FUNC_COR_TYPE] ) ;

#ifdef DONT_USE_MEMCPY
            for( iv=0 ; iv < nxyz ; iv++ ) bar[iv] = 0 ;
#else
            memset( bar , 0 , sizeof(short)*nxyz ) ;
#endif

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            for( iv=0 ; iv < nvox ; iv++ )
               bar[indx[iv]] = (short)(FUNC_COR_SCALE_SHORT * vval[iv] + 0.499) ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            stataux[1] = 1.0 / FUNC_COR_SCALE_SHORT ;

         } else {

         /*** Multiple references --> find best correlation at each voxel ***/

            /*--- get first ref results into abest and rbest (best so far) ---*/

            PCOR_get_coef( pc_ref[0] , pc_vc[0] , abest ) ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            PCOR_get_pcor( pc_ref[0] , pc_vc[0] , rbest ) ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            /*--- for each succeeding ref vector,
                  get results into aval and vval,
                  if |vval| > |rbest|, then use that result instead ---*/

            for( ivec=1 ; ivec < ny_ref ; ivec++ ){

               PCOR_get_coef( pc_ref[ivec] , pc_vc[ivec] , aval ) ;

               if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

               PCOR_get_pcor( pc_ref[ivec] , pc_vc[ivec] , vval ) ;

               if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

               for( iv=0 ; iv < nvox ; iv++ ){
                  if( fabs(vval[iv]) > fabs(rbest[iv]) ){
                     rbest[iv] = vval[iv] ;
                     abest[iv] = aval[iv] ;
                  }
               }

               if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;
            }

            /*--- at this point, abest and rbest are the best
                  results, so scale them into the dataset bricks ---*/

            topval = 0.0 ;
            for( iv=0 ; iv < nvox ; iv++ )
               if( fabs(abest[iv]) > topval ) topval = fabs(abest[iv]) ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            bar = DSET_ARRAY( new_dset , FUNC_ival_fim[FUNC_COR_TYPE] ) ;

#ifdef DONT_USE_MEMCPY
            for( iv=0 ; iv < nxyz ; iv++ ) bar[iv] = 0 ;
#else
            memset( bar , 0 , sizeof(short)*nxyz ) ;
#endif

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            if( topval > 0.0 ){
               topval = MRI_TYPE_maxval[MRI_short] / topval ;
               for( iv=0 ; iv < nvox ; iv++ )
                  bar[indx[iv]] = (short)(topval * abest[iv] + 0.499) ;

               stataux[0] = 1.0/topval ;
            } else {
               stataux[0] = 0.0 ;
            }

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            bar = DSET_ARRAY( new_dset , FUNC_ival_thr[FUNC_COR_TYPE] ) ;

#ifdef DONT_USE_MEMCPY
            for( iv=0 ; iv < nxyz ; iv++ ) bar[iv] = 0 ;
#else
            memset( bar , 0 , sizeof(short)*nxyz ) ;
#endif

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            for( iv=0 ; iv < nvox ; iv++ )
               bar[indx[iv]] = (short)(FUNC_COR_SCALE_SHORT * rbest[iv] + 0.499) ;

            if( update_frequency > 0 ) AFNI_process_interrupts(im3d->vwid->top_shell) ;

            stataux[1] = 1.0 / FUNC_COR_SCALE_SHORT ;

         }

STATUS("setting brick_fac") ;

         (void) EDIT_dset_items( new_dset ,
                                    ADN_brick_fac , stataux ,
                                 ADN_none ) ;

         /*--- redisplay ---*/

         if( update_frequency > 0 ){

STATUS("real-time update") ;

            /*-- Bastille Day 1996:
                 do most efficient type of display, except when trying the last --*/

            if( nupdt < ngood_ref ) {
               im3d->vinfo->crosshair_visible = False ;
               im3d->vwid->marks->ov_visible  = False ;

               im3d->vinfo->func_resam_mode   = RESAM_NN_TYPE ;
               if( im3d->b123_fim != NULL )
                  im3d->b123_fim->resam_code  =
                   im3d->b231_fim->resam_code  =
                    im3d->b312_fim->resam_code  = RESAM_NN_TYPE ;

            } else {
               im3d->vinfo->crosshair_visible = save_xhairs ;
               im3d->vwid->marks->ov_visible  = save_marks ;

               im3d->vinfo->func_resam_mode   = save_resam ;
               if( im3d->b123_fim != NULL )
                  im3d->b123_fim->resam_code  =
                   im3d->b231_fim->resam_code  =
                    im3d->b312_fim->resam_code  = save_resam ;
            }

STATUS("AFNI_SEE_FUNC_ON") ;

            AFNI_SEE_FUNC_ON(im3d) ;

STATUS("about to redisplay") ;

            AFNI_fimmer_redisplay( first_updt , im3d , new_dset ) ;
            first_updt = 0 ;
         }
      }
   }

#ifndef DONT_USE_METER
   MCW_set_meter( meter , 100 ) ;
#endif

   /*--- End of recursive updates; now free temporary workspaces ---*/

   if( update_frequency > 0 ) AFNI_add_interruptable( NULL ) ;  /* clear */

   for( ivec=0 ; ivec < ny_ref ; ivec++ ){
      free_PCOR_references(pc_ref[ivec]) ;
      free_PCOR_voxel_corr(pc_vc[ivec]) ;
   }
   free(vval) ; free(indx) ; free(pc_ref) ; free(pc_vc) ;
   if( aval  != NULL ) free(aval) ;
   if( rbest != NULL ) free(rbest) ;
   if( abest != NULL ) free(abest) ;

   /*--- Return new dataset ---*/

#ifndef DONT_USE_METER
   MCW_popdown_meter(meter) ;
#endif

   RETURN(new_dset) ;
}

/*--------------------------------------------------------------------------*/

#ifdef USE_FUNC_FIM
void AFNI_fimmer_menu_CB( Widget w , XtPointer cd , XtPointer cbs )
{
   FIM_menu * fmenu = (FIM_menu *) cd ;
   Three_D_View * im3d = (Three_D_View *) fmenu->parent ;

ENTRY("AFNI_fimmer_menu_CB") ;

   if( ! IM3D_OPEN(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;

   /*** Pick reference ***/

   if( w == fmenu->fim_pickref_pb ){
      if( IMARR_COUNT(GLOBAL_library.timeseries) > 0 ){
         int init_ts = AFNI_ts_in_library( im3d->fimdata->fimref ) ;

         MCW_choose_timeseries( fmenu->fim_cbut , "FIM Reference Vector" ,
                                GLOBAL_library.timeseries , init_ts ,
                                AFNI_fimmer_pickref_CB , (XtPointer) im3d ) ;
      } else {
         (void) MCW_popup_message(
                   fmenu->fim_cbut ,
                   "No timeseries library\nexists to pick from!" ,
                   MCW_USER_KILL | MCW_TIMER_KILL ) ;
      }
   }

   /*** Pick ort ***/

   else if( w == fmenu->fim_pickort_pb ){
      if( IMARR_COUNT(GLOBAL_library.timeseries) > 0 ){
         int init_ts = AFNI_ts_in_library( im3d->fimdata->fimort ) ;

         MCW_choose_timeseries( fmenu->fim_cbut , "FIM Ort Vector" ,
                                GLOBAL_library.timeseries , init_ts ,
                                AFNI_fimmer_pickort_CB , (XtPointer) im3d ) ;
      } else {
         (void) MCW_popup_message(
                   fmenu->fim_cbut ,
                   "No timeseries library\nexists to pick from!" ,
                   MCW_USER_KILL | MCW_TIMER_KILL ) ;
      }
   }


   /*** Refresh Frequency ***/

   else if( w == fmenu->fim_setupdate_pb ){
      MCW_choose_integer( fmenu->fim_cbut , "FIM Refresh Frequency" ,
                          0 , 99 , im3d->vinfo->fimmer_update_frequency ,
                          AFNI_fimmer_setupdate_CB , (XtPointer) im3d ) ;

   }

   /*** execute FIM ***/

   else if( w == fmenu->fim_execute_pb ){
      AFNI_fimmer_execute( im3d ) ;
   }

   /*** Ignore stuff ***/

   else if( w == fmenu->fim_ignore_down_pb ){
      if( im3d->fimdata->init_ignore > 0 )
         AFNI_fimmer_setignore( im3d , im3d->fimdata->init_ignore - 1 ) ;
   }

   else if( w == fmenu->fim_ignore_up_pb ){
      AFNI_fimmer_setignore( im3d , im3d->fimdata->init_ignore + 1 ) ;
   }

   else if( w == fmenu->fim_ignore_choose_pb ){
#ifdef USE_OPTMENUS
      AFNI_fimmer_ignore_choose_CB( fmenu->fim_ignore_choose_av , im3d ) ;
#else
      MCW_choose_integer( fmenu->fim_cbut , "Initial Ignore" ,
                          0 , 999 , im3d->fimdata->init_ignore ,
                          AFNI_fimmer_ignore_choose_CB , (XtPointer) im3d ) ;
#endif
   }

   /*** Pick a time dependent dataset.
        This code is mostly taken from AFNI_choose_dataset_CB ***/

#define STRLIST_SIZE (THD_MAX_PREFIX+12)

   else if( w == fmenu->fim_pickdset_pb ){
      static char * strlist[THD_MAX_SESSION_ANAT] ;  /* strings to choose between */
      static int first_call = 1 ;                    /* initialization flag */

      int num_str , ii , init_str=-1 , vv , jj ;

      /** initialize string array **/

      if( GLOBAL_library.have_dummy_dataset ){ BEEPIT ; EXRETURN ; }

      if( first_call ){
         for( ii=0 ; ii < THD_MAX_SESSION_ANAT ; ii++ )
            strlist[ii] = XtMalloc( sizeof(char) * (STRLIST_SIZE+1) ) ;
         first_call = 0 ;
      }

      /** scan through anats and find 3D+t datasets **/

      num_str = 0 ;
      vv      = im3d->vinfo->view_type ;  /* current view */
      for( ii=0 ; ii < im3d->ss_now->num_anat ; ii++ ){

         if( DSET_GRAPHABLE(im3d->ss_now->anat[ii][vv]) ){  /** have one! **/
            MCW_strncpy( strlist[num_str] ,
                         im3d->ss_now->anat[ii][vv]->dblk->diskptr->prefix ,
                         THD_MAX_PREFIX ) ;

            jj = ii ;  /* most recent */

            if( im3d->fimdata->fimdset == im3d->ss_now->anat[ii][vv] )  /* same? */
               init_str = num_str ;

            num_str ++ ;
         }
      }

      /** make the choice **/

      if( num_str <= 0 ){                    /* nothing to choose from */

         (void) MCW_popup_message(
                   fmenu->fim_cbut ,
                      "No time dependent (3D+time)\n"
                      "datasets exist to choose from!" ,
                   MCW_USER_KILL | MCW_TIMER_KILL ) ;

      } else if( num_str == 1 ){             /* Hobson's choice */

         im3d->fimdata->fimdset = im3d->ss_now->anat[jj][vv] ;
         ALLOW_COMPUTE_FIM(im3d) ;

      } else {                               /* an actual choice to make! */

         MCW_choose_strlist( fmenu->fim_cbut , "3D+time Dataset" ,
                             num_str , init_str , strlist ,
                             AFNI_fimmer_dset_choose_CB , (XtPointer) im3d ) ;
      }
   }

   /*** Unimplemented Button ***/

   else {
      XBell( im3d->dc->display , 100 ) ;
   }

   /*--- Done!!! ---*/

   EXRETURN ;
}
#endif

/*---------------------------------------------------------------------------*/

#ifdef USE_FUNC_FIM

#ifdef USE_OPTMENUS
void AFNI_fimmer_ignore_choose_CB( MCW_arrowval * cbs , XtPointer cd )
#else
void AFNI_fimmer_ignore_choose_CB( Widget wcaller, XtPointer cd, MCW_choose_cbs * cbs )
#endif
{
   Three_D_View * im3d = (Three_D_View *) cd ;

   if( ! IM3D_VALID(im3d) ) return ;

   AFNI_fimmer_setignore( im3d , cbs->ival ) ;
   return ;
}
#endif

#ifdef USE_FUNC_FIM
void AFNI_fimmer_dset_choose_CB( Widget wcaller , XtPointer cd , MCW_choose_cbs * cbs )
{
   Three_D_View * im3d = (Three_D_View *) cd ;
   int ii , vv , num_str ;

   if( ! IM3D_VALID(im3d) ) return ;
   if( GLOBAL_library.have_dummy_dataset ){ BEEPIT ; EXRETURN ; }

   num_str = 0 ;
   vv      = im3d->vinfo->view_type ;
   for( ii=0 ; ii < im3d->ss_now->num_anat ; ii++ ){
      if( DSET_GRAPHABLE(im3d->ss_now->anat[ii][vv]) ){
         if( num_str == cbs->ival ) break ;
         num_str ++ ;
      }
   }

   if( ii < im3d->ss_now->num_anat ){
      im3d->fimdata->fimdset = im3d->ss_now->anat[ii][vv] ;
      ALLOW_COMPUTE_FIM(im3d) ;
   } else {
      fprintf(stderr,"\n*** Illegal choice in AFNI_fimmer_dset_choose_CB:"
                     "\n*** cbs->ival = %d  but num_str = %d\a\n",
                     cbs->ival , num_str ) ;
   }

   return ;
}
#endif

/*---------------------------------------------------------------------------*/

void AFNI_fimmer_execute( Three_D_View * im3d )
{
   THD_3dim_dataset * new_dset , * dset_time ;
   MRI_IMAGE * ref_ts , * ort_ts ;
   THD_session * sess ;
   int ifunc ;

ENTRY("AFNI_fimmer_execute") ;

   if( ! IM3D_OPEN(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;
   if( GLOBAL_library.have_dummy_dataset ){ BEEPIT ; EXRETURN ; }

#if 0
   dset_time = im3d->anat_now ;
#else
   dset_time = im3d->fimdata->fimdset ;
#endif

   if( ! DSET_GRAPHABLE(dset_time) ){ XBell(im3d->dc->display,100) ; EXRETURN ; }

   ref_ts = im3d->fimdata->fimref ;
   ort_ts = im3d->fimdata->fimort ;
   if( ref_ts == NULL ){ XBell(im3d->dc->display,100) ; EXRETURN ; }

   sess = im3d->ss_now ;
   if( ! ISVALID_SESSION(sess) || sess->num_func >= THD_MAX_SESSION_FUNC ){
      XBell(im3d->dc->display,100) ; EXRETURN ;
   }

   SHOW_AFNI_PAUSE ;

   /*--- Start lots of CPU time ---*/

   new_dset = AFNI_fimmer_compute( im3d , dset_time , ref_ts , ort_ts , sess ,
                                   im3d->vinfo->fimmer_update_frequency ) ;

   /*--- End lots of CPU time ---*/

   if( new_dset == NULL ){
       SHOW_AFNI_READY ;
       XBell(im3d->dc->display,100) ; EXRETURN ;
   }

   if( im3d->vinfo->fimmer_update_frequency <= 0 ){
      AFNI_SEE_FUNC_ON(im3d) ;
      AFNI_fimmer_redisplay( 1 , im3d , new_dset ) ;
   }

   (void) THD_write_3dim_dataset( NULL,NULL , new_dset , True ) ;

   /*** At this point, FIM is computed and written to disk ***/

   AFNI_force_adoption( sess , False ) ;
   AFNI_make_descendants( GLOBAL_library.sslist ) ;

   SHOW_AFNI_READY ;

   EXRETURN ;
}

/*-----------------------------------------------------------------------------*/

void AFNI_fimmer_redisplay( int first_call ,
                            Three_D_View * im3d , THD_3dim_dataset * new_dset )
{
   int ifunc ;
   THD_session * sess = im3d->ss_now ;

ENTRY("AFNI_fimmer_redisplay") ;

   if( first_call ){

      /*** Fit the new dataset into its place in the session ***/

      ifunc = sess->num_func ;
      sess->func[ifunc][new_dset->view_type] = new_dset ;
      (sess->num_func)++ ;
      im3d->vinfo->func_num = ifunc ;

      THD_load_statistics( new_dset ) ;
      AFNI_initialize_view( im3d->anat_now , im3d ) ;

   } else {

      /*** Just redisplay the new dataset ***/

      THD_load_statistics( new_dset ) ;
      AFNI_reset_func_range( im3d ) ;
      AFNI_set_thr_pval( im3d ) ;

      AFNI_set_viewpoint( im3d , -1,-1,-1 , REDISPLAY_OVERLAY ) ;
   }

   EXRETURN ;
}

/*--------------------------------------------------------------------
  Routines to deal with widgets that are allowed to be active
  during 'real-time' operations.
----------------------------------------------------------------------*/

void AFNI_add_interruptable( Widget w )
{
   Window wwin , wroot , wpar ;
   int    ii ;
   Window *     wchild ;
   unsigned int nchild ;

   /* input Widget is NULL is the signal to reset everything */

   if( w == NULL ){
      if( GLOBAL_library.interruptables.windows != NULL ){ /* just toss this list */
         DESTROY_XTARR( GLOBAL_library.interruptables.windows ) ;
         GLOBAL_library.interruptables.windows = NULL ;
      }

      if( GLOBAL_library.interruptables.widgets != NULL ){ /* must reset cursors */
         for( ii=0 ; ii < GLOBAL_library.interruptables.widgets->num ; ii++ )
            MCW_set_widget_cursor( GLOBAL_library.interruptables.widgets->ar[ii] , 0 );

         DESTROY_XTARR( GLOBAL_library.interruptables.widgets ) ;
         GLOBAL_library.interruptables.widgets = NULL ;
      }
      return ;
   }

   /* non-NULL widget --> add it and its sub-windows to the list */

   if( ! XtIsRealized(w) ) return ;
   wwin = XtWindowOfObject(w) ; if( wwin == (Window) NULL ) return ;

   /* get list of all sub-windows into wchild */

   nchild = 0 ;
   (void) XQueryTree( XtDisplay(w) , wwin , &wroot , &wpar , &wchild , &nchild ) ;

   if( GLOBAL_library.interruptables.windows == NULL )       /* initialize lists */
      INIT_XTARR( GLOBAL_library.interruptables.windows ) ;

   if( GLOBAL_library.interruptables.widgets == NULL )
      INIT_XTARR( GLOBAL_library.interruptables.widgets ) ;

   ADDTO_XTARR( GLOBAL_library.interruptables.windows , wwin ) ;  /* add to lists */
   ADDTO_XTARR( GLOBAL_library.interruptables.widgets , w    ) ;

   for( ii=0 ; ii < nchild ; ii++ )
      ADDTO_XTARR( GLOBAL_library.interruptables.windows , wchild[ii] ) ;

   XFree( wchild ) ;  /* return this to Xlib */

   MCW_alter_widget_cursor( w , -XC_left_ptr , "yellow","blue" ) ; /* set cursor */

   return ;
}

void AFNI_process_interrupts( Widget w )
{
   Display * dis = XtDisplay(w) ;
   XEvent ev ;
   int ii , nwin ;;

   /* make sure server has everything from us, then make sure display is OK */

   XFlush( dis ) ; XmUpdateDisplay( w ) ;

   /* if no windows are set to allow interrupts, then go home to mother */

   if( GLOBAL_library.interruptables.windows      == NULL ||
       GLOBAL_library.interruptables.windows->num == 0      ) return ;

   nwin = GLOBAL_library.interruptables.windows->num ;

   /* check for events;
      if they are in a window on our list, let Xt handle them;
      loop until all pending events have been handled or discarded */

   while( XCheckMaskEvent( dis ,
                           ButtonPressMask  | ButtonReleaseMask |
                           ButtonMotionMask | PointerMotionMask |
                           KeyPressMask     | KeyReleaseMask     , &ev ) ){

      for( ii=0 ; ii < nwin ; ii++ ){
         if( ev.xany.window ==
             (Window) GLOBAL_library.interruptables.windows->ar[ii] ){

            XtDispatchEvent( &ev ) ;  /* was on our list! */
            break ;
         }
      }

      if( ii == nwin ) XBell( dis , 100 ) ;  /* beep for non-allowed event */
   }

   return ;
}

#ifdef USE_FUNC_FIM
void AFNI_fimmer_fix_optmenu( Three_D_View * im3d )
{
   int igtop , ntime ;

ENTRY("AFNI_fimmer_fix_optmenu") ;

   if( ! IM3D_VALID(im3d) || im3d->type != AFNI_3DDATA_VIEW ) EXRETURN ;

   /** fim ignoration **/

   if( im3d->fimdata->fimdset == NULL ) EXRETURN ;
   ntime = DSET_NUM_TIMES(im3d->fimdata->fimdset) ; if( ntime < 2 ) EXRETURN ;

   igtop = MIN( ntime-3 , 99 ) ;
   igtop = MAX( igtop , 1 ) ;

   if( im3d->vwid->func->fim_menu->fim_ignore_choose_av->imax != igtop ){

#ifdef AFNI_DEBUG
{ char str[256] ;
  sprintf(str,"refit 0..%d, init %d",igtop,im3d->fimdata->init_ignore) ;
  STATUS(str) ; }
#endif

      refit_MCW_optmenu( im3d->vwid->func->fim_menu->fim_ignore_choose_av ,
                         0 , igtop , im3d->fimdata->init_ignore, 0,
                         NULL , NULL ) ;
   } else {

#ifdef AFNI_DEBUG
{ char str[256] ;
  sprintf(str,"assign init %d",im3d->fimdata->init_ignore) ;
  STATUS(str) ; }
#endif

      AV_assign_ival( im3d->vwid->func->fim_menu->fim_ignore_choose_av ,
                      im3d->fimdata->init_ignore ) ;
   }

   EXRETURN ;
}
#endif
