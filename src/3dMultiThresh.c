#include "mrilib.h"
#include "mri_threshX.c"

static int verb=1 ;

void mri_apply_mask( MRI_IMAGE *inim , MRI_IMAGE *bim ) ; /* prototype */

int main( int argc , char *argv[] )
{
   THD_3dim_dataset *mset=NULL , *iset=NULL ;
   char *prefix = "mthresh.nii" ;
   int nopt , ii , tindex=-1 , ival,nval,nvox ;
   int nnlev=0,nnsid=0,nzthr=0 ; float *zthr=NULL ;
   MRI_IMAGE *fim , *bfim ; int nhits=0 , do_nozero=0 ;
   int do_hpow0=0,do_hpow1=0,do_hpow2=0, nhpow=0,hpow=0, hstart;
   int do_maskonly=0 , do_pos=-1 ;
   MRI_IMARR *cimar0=NULL , *cimar1=NULL , *cimar2=NULL ;

   /*----- help, I'm trapped in an instance of vi and can't get out -----*/

   if( argc < 3 || strcasecmp(argv[1],"-help") == 0 ){
     printf("\n"
      "Program to apply a multi-threshold (mthresh) dataset\n"
      "to an input dataset.\n"
      "\n"
      "Usage:\n"
      "  3dMultiThresh OPTIONS\n"
      "\n"
      "OPTIONS (in any order)\n"
      "----------------------\n"
      "\n"
      " -mthresh mmm    = multi-threshold dataset from 3dXClustSim\n"
      " -input   ddd    = dataset to threshold\n"
      " -1tindex iii    = index (sub-brick) on which to threshold\n"
      " -signed  +/-    = if the .mthresh.nii file from 3dXClustSim\n"
      "                   was created using 1-sided thresholding,\n"
      "                   this option tells which sign to choose when\n"
      "                   doing voxel-wise thresholding: + or -.\n"
      "                  ++ If the .mthresh.nii file was created using\n"
      "                     2-sided thresholding, this option is ignored.\n"
      " -pos            = same as '-signed +'\n"
      " -neg            = same as '-signed -'\n"
      " -prefix  ppp    = prefix for output dataset\n"
      "                  ++ Can be 'NULL' to get no output dataset\n"
      " -maskonly       = Instead of outputing a thresholded version\n"
      "                   of the input dataset, just output a 0/1 mask\n"
      "                   dataset of voxels that survive the process.\n"
      " -nozero         = this option prevents the output of a\n"
      "                   dataset if it would be all zero\n"
      " -quiet          = turn off progress report messages\n"
      "\n"
      "The number of surviving voxels will be written to stdout.\n"
      "It can be captured in a csh script by a command such as\n"
      "   set nhits = `3dMultiThresh OPTIONS`\n"
      "\n"
      "Meant to be used in conjunction with program 3dXClustSim,\n"
      "which is in turn meant to be used with program 3dttest++ -- RWCox\n\n"
     ) ;
     exit(0) ;
   }

   /*----- scan options -----*/

   mainENTRY("3dMultiThresh"); machdep();

   nopt = 1 ;

   while( nopt < argc && argv[nopt][0] == '-' ){

     if( strcasecmp(argv[nopt],"-signed") == 0 ){  /* 26 Apr 2017 */
       if( ++nopt >= argc )
         ERROR_exit("need 1 argument after option '%s'",argv[nopt-1]) ;
            if( argv[nopt][0] == '+' ) do_pos = 1 ;
       else if( argv[nopt][0] == '-' ) do_pos = 0 ;
       else
         ERROR_exit("option '%s %s' doesn't mean anything :(",argv[nopt-1],argv[nopt]) ;
       nopt++ ; continue ;
     }

     if( strncasecmp(argv[nopt],"-pos",4) == 0 ){  /* 26 Apr 2017 */
       do_pos = 1 ; nopt++ ; continue ;
     }
     if( strncasecmp(argv[nopt],"-neg",4) == 0 ){  /* 26 Apr 2017 */
       do_pos = 0 ; nopt++ ; continue ;
     }

     if( strcasecmp(argv[nopt],"-maskonly") == 0 ){
       do_maskonly = 1 ; nopt++ ; continue ;
     }

     if( strcasecmp(argv[nopt],"-nozero") == 0 ){
       do_nozero = 1 ; nopt++ ; continue ;
     }

     if( strcasecmp(argv[nopt],"-quiet") == 0 ){
       verb = 0 ; nopt++ ; continue ;
     }

     if( strcasecmp(argv[nopt],"-prefix") == 0 ){
       if( ++nopt >= argc )
         ERROR_exit("need 1 argument after option '%s'",argv[nopt-1]) ;
       prefix = strdup(argv[nopt]) ;
       if( !THD_filename_ok(prefix) )
         ERROR_exit("prefix '%s' is illegal (and funny looking)",prefix) ;
       nopt++ ; continue ;
     }

     if( strcasecmp(argv[nopt],"-input") == 0 ){
       if( iset != NULL )
         ERROR_exit("you can't use '-input' twice!") ;
       if( ++nopt >= argc )
         ERROR_exit("need 1 argument after option '%s'",argv[nopt-1]) ;
       iset = THD_open_dataset(argv[nopt]) ;
       if( iset == NULL )
         ERROR_exit("can't open -input dataset '%s'",argv[nopt]) ;
       DSET_load(iset) ; CHECK_LOAD_ERROR(iset) ;
       nopt++ ; continue ;
     }

     if( strcasecmp(argv[nopt],"-1tindex") == 0 ){
       if( ++nopt >= argc )
         ERROR_exit("need 1 argument after option '%s'",argv[nopt-1]) ;
       tindex = (int)strtod(argv[nopt],NULL) ;
       nopt++ ; continue ;
     }

     if( strcasecmp(argv[nopt],"-mthresh") == 0 ){
       ATR_float *atr ; float *afl ; int nbad=0 ;
       if( mset != NULL )
         ERROR_exit("you can't use '-mthresh' twice!") ;
       if( ++nopt >= argc )
         ERROR_exit("need 1 argument after option '%s'",argv[nopt-1]) ;
       mset = THD_open_dataset(argv[nopt]) ;
       if( mset == NULL )
         ERROR_exit("can't open -mthresh dataset '%s'",argv[nopt]) ;
       DSET_load(mset) ; CHECK_LOAD_ERROR(mset) ;

       atr = THD_find_float_atr( mset->dblk , "MULTI_THRESHOLDS" ) ;
       if( atr == NULL )
         ERROR_exit("-mthresh dataset does not have MULTI_THRESHOLDS attribute :(") ;
       afl = atr->fl ;

       nnlev = (int)afl[0] ;
       nnsid = (int)afl[1] ;
       nzthr = (int)afl[2] ;
       hpow  = (int)afl[3] ;
       zthr  = (float *)malloc(sizeof(float)*nzthr) ;
       for( ii=0 ; ii < nzthr ; ii++ ) zthr[ii] = afl[4+ii] ;

       do_hpow0 = (hpow & 1) != 0 ;
       do_hpow1 = (hpow & 2) != 0 ;
       do_hpow2 = (hpow & 4) != 0 ;
       nhpow    = do_hpow0 + do_hpow1 + do_hpow2 ;

       if( verb ){
           INFO_message("-mthresh dataset parameters") ;
         ININFO_message("     clustering NN=%d  thresholding=%d-sided  %d thresholds  %d hpows",
                        nnlev,nnsid,nzthr,nhpow ) ;
       }
       if( nnlev < 1 || nnlev > 3 ){ ERROR_message("illegal nnlev=%d",nnlev); nbad++; }
       if( nnsid < 1 || nnsid > 2 ){ ERROR_message("illegal sided=%d",nnsid); nbad++; }
       if( nzthr < 1              ){ ERROR_message("illegal nzthr=%d",nzthr); nbad++; }
       if( nhpow < 1              ){ ERROR_message("illegal nhpow=%d",nhpow); nbad++; }
       if( nbad )                    ERROR_exit("Can't continue after the above problems") ;

       nopt++ ; continue ;
     }

     ERROR_exit("Unknown option '%s'",argv[nopt]) ;
   }

   /*-- check for errors --*/

   if( iset == NULL ) ERROR_exit("-input is a mandatory 'option'") ;
   if( mset == NULL ) ERROR_exit("-mthresh is a mandatory 'option'") ;

   /* check sign-age [26 Apr 2017] */

   if( nnsid == 1 ){
     if( do_pos < 0 ){
       INFO_message("1-sided thresholding: default is positive") ; do_pos = 1 ;
     }
   } else if( do_pos >= 0 ){
     INFO_message("2-sided thresholding: ignoring %s sign option",
                   (do_pos) ? "+" : "-" ) ;
     do_pos = 1 ;
   }

   /* change signs to threshold on the negative side? */

   if( do_pos == 0 ){
     for( ival=0 ; ival < nzthr ; ival++ ) zthr[ival] = -zthr[ival] ;
   }

   /*----- do the work (oog) -----*/

   /* find the threshold sub-brick, if not forced upon us */

   nval = DSET_NVALS(iset) ;
   nvox = DSET_NVOX(iset) ;

   /* if tindex not given, find something appropriate */

   if( tindex < 0 || tindex >= nval ){
     for( ival=0 ; ival < nval ; ival++ ){
       if( DSET_BRICK_STATCODE(iset,ival) == FUNC_ZT_TYPE ) break ;
     }
     if( ival >= nval ){
       for( ival=0 ; ival < nval ; ival++ ){
         if( DSET_BRICK_STATCODE(iset,ival) == FUNC_TT_TYPE ) break ;
       }
     }
     tindex = (ival < nval) ? ival : 0 ;
     if( verb ) INFO_message("threshold default index set to %d",ival) ;
   }


   /* get the threshold brick image */

   fim = THD_extract_float_brick( tindex , iset ) ;
   if( fim == NULL ) ERROR_exit("Can't get sub-brick %d to threshold",tindex) ;

   /* build the arrays of FOM threshold volumes */

   hstart = 0 ;
   if( do_hpow0 ){
     INIT_IMARR(cimar0) ;
     for( ival=0 ; ival < nzthr ; ival++ )
       ADDTO_IMARR(cimar0,DSET_BRICK(mset,hstart+ival*nhpow)) ;
     hstart++ ;
   }
   if( do_hpow1 ){
     INIT_IMARR(cimar1) ;
     for( ival=0 ; ival < nzthr ; ival++ )
       ADDTO_IMARR(cimar1,DSET_BRICK(mset,hstart+ival*nhpow)) ;
     hstart++ ;
   }
   if( do_hpow2 ){
     INIT_IMARR(cimar2) ;
     for( ival=0 ; ival < nzthr ; ival++ )
       ADDTO_IMARR(cimar2,DSET_BRICK(mset,hstart+ival*nhpow)) ;
     hstart++ ;
   }

   /* get the mask of values above the multi-thresholds */

   mri_multi_threshold_setup() ;

   bfim = mri_multi_threshold_Xcluster( fim ,
                                        nzthr , zthr , nnsid , nnlev ,
                                        cimar0 , cimar1 , cimar2 ,
                                        XTHRESH_OUTPUT_MASK , &nhits ) ;

   mri_multi_threshold_unsetup() ;

   /* don't need mthresh dataset no more */

   DSET_unload(mset) ;
   FREE_IMARR(cimar0) ; FREE_IMARR(cimar1) ; FREE_IMARR(cimar2) ;

   /* nothing survived? */

   if( bfim == NULL && do_nozero ){ printf("0\n") ; exit(0) ; }

   /* apply the mask and produce the output dataset */

   if( strcmp(prefix,"NULL") != 0 ){
     THD_3dim_dataset *oset ;
     if( bfim == NULL ){  /* make all-zero mask */
       bfim = mri_new_conforming( fim , MRI_byte ) ;
       memset(MRI_BYTE_PTR(bfim),0,sizeof(byte)*bfim->nvox) ;
     }

     if( !do_maskonly ){  /* output a real dataset */
       oset = EDIT_full_copy(iset,prefix) ;
       DSET_unload(iset) ;
       tross_Copy_History( iset , oset ) ;
       tross_Make_History( "3dMultiThresh" , argc,argv , oset ) ;
       THD_copy_datablock_auxdata( iset->dblk , oset->dblk ) ;
       THD_copy_labeltable_atr(oset->dblk,iset->dblk);
       for( ival=0 ; ival < nval ; ival++ ){
         mri_apply_mask( DSET_BRICK(oset,ival) , bfim ) ;
       }
       DSET_write(oset) ;
       if( verb ) WROTE_DSET(oset) ;
       DSET_delete(oset) ;
     } else {               /* maskonly output [21 Apr 2017] */
       DSET_unload(iset) ;
       oset = EDIT_empty_copy(iset) ;
       tross_Copy_History( iset , oset ) ;
       tross_Make_History( "3dMultiThresh" , argc,argv , oset ) ;
       EDIT_dset_items( oset , ADN_nvals,1 , ADN_prefix,prefix , ADN_none ) ;
       EDIT_substitute_brick( oset , 0 , MRI_byte , MRI_BYTE_PTR(bfim) ) ;
       DSET_write(oset) ;
       if( verb ) WROTE_DSET(oset) ;
     }
   }

   mri_free(bfim) ;
   printf("%d\n",nhits) ;
   exit(0) ;
}

/*----------------------------------------------------------------------------*/

void mri_apply_mask( MRI_IMAGE *inim , MRI_IMAGE *bim )
{
   byte *msk ; int ii,nvox ;

ENTRY("mri_apply_mask") ;

   if( inim == NULL || bim == NULL || bim->kind != MRI_byte ) EXRETURN ;

   msk = MRI_BYTE_PTR(bim) ;
   nvox = inim->nvox ; if( bim->nvox != nvox ) EXRETURN ;

   switch( inim->kind ){

     default: ERROR_message("Can't handle data of type %s",MRI_TYPE_name[inim->kind]) ;
              break ;

     case MRI_byte:{
       byte *iar = MRI_BYTE_PTR(inim) ;
       for( ii=0 ; ii < nvox ; ii++ ) if( msk[ii] == 0 ) iar[ii] = 0 ;
     }
     break ;

     case MRI_short:{
       short *iar = MRI_SHORT_PTR(inim) ;
       for( ii=0 ; ii < nvox ; ii++ ) if( msk[ii] == 0 ) iar[ii] = 0 ;
     }
     break ;

     case MRI_float:{
       float *iar = MRI_FLOAT_PTR(inim) ;
       for( ii=0 ; ii < nvox ; ii++ ) if( msk[ii] == 0 ) iar[ii] = 0.0f ;
     }
     break ;

   }

   EXRETURN ;
}
