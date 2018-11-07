cmake_minimum_required (VERSION 3.2)
project(AFNI)

include(FindX11)
include(FindMotif)
set(AFNI_MAJOR_VERSION 17)
set(AFNI_MINOR_VERSION 1)
set(AFNI_PATCH_VERSION 12)
set(AFNI_VERSION ${AFNI_MAJOR_VERSION}.${AFNI_MINOR_VERSION}.${AFNI_PATCH_VERSION})

string(TIMESTAMP BUILD_DATE "%m %d %Y")
configure_file(AFNI_version.h.in "${CMAKE_CURRENT_BINARY_DIR}/AFNI_version.h" @ONLY)


# find_package(f2c REQUIRED)
find_package(Xt REQUIRED)



include_directories(
	${PROJECT_SOURCE_DIR}
	${PROJECT_BINARY_DIR}
)


# TODO add specific flag for specific file
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DAFNI_DEBUG -DIMSEQ_DEBUG -DDISPLAY_DEBUG -DTHD_DEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DREAD_WRITE_64 -DLINUX2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_TRACING") # XXX not in default cflags


# XXX remove dlcompat
# XXX remove jpeg-6b
add_subdirectory(scripts_install)
add_subdirectory(python_scripts)
add_subdirectory(R_scripts)



# bundled library (XXX to remove/package elsewhere)
macro(optional_bundle name)
	string(TOUPPER ${name} upper)

	option(USE_SYSTEM_${upper} "use system ${name} instead of bundled")
	if(USE_SYSTEM_${upper})
		find_package(${name} REQUIRED)
	else(USE_SYSTEM_${upper})
		add_subdirectory(${name})
	endif(USE_SYSTEM_${upper})
endmacro(optional_bundle)

# not sure how to resolve cyclic dependency between 3dedge and suma so add the
# include dir for 3edge here
SET(3DEDGE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/3DEdge/src)

foreach(bundled avovk coxplot crorden eispack gifti  nifti niml Audio
       ptaylor qhull rickr SUMA svm volpack XmHTML 3DEdge converted_from_fortran)
	optional_bundle(${bundled})
endforeach(bundled)

option(USE_GIFTI "use gifti")
if(USE_GIFTI)
	set_source_files_properties(gifti_choice.c PROPERTIES COMPILE_DEFINITIONS "HAVE_GIFTI")
endif(USE_GIFTI)

option(CHECK_FRAME_BUFFER "do not check the framebuffer before rendering (might be buggy)" YES)
if(NOT CHECK_FRAME_BUFFER)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGL_NO_CHECK_FRAME_BUFFER")
endif(NOT CHECK_FRAME_BUFFER)

# --
# find_package (NetCDF REQUIRED)
FIND_LIBRARY(NETCDF_LIBRARY netcdf)

file(GLOB AFOBJS "afni*.c")
list(APPEND AFOBJS pbar.c $<TARGET_OBJECTS:parser> mcw_graf.c)

file(GLOB T3OBJS  "to3d*.c")

file(GLOB MRI_OBJS "mri_*.c")
list(APPEND MRI_OBJS csfft.c mcw_glob.c mcw_malloc.c cox_render.c debugtrace.c ge4_header.c rcmat.c dmat44.c)
foreach(filename mri_affine.c mri_pc_pursuit.c mri_process_siemens.c mri_quantize.c mri_alphasim.c mri_wf.c mri_gwarp.c mri_pvmap.c)
   list(REMOVE_ITEM MRI_OBJS ${PROJECT_SOURCE_DIR}/${filename})
endforeach(filename)

file(GLOB THD_OBJS "thd_*.c")
list(APPEND THD_OBJS afni_vedit.c ktaub.c bilinear_warp3D.c)
foreach(filename thd_bstrap_corr.c thd_gifti.c thd_md5.c thd_rot3d_2byte.c
    thd_serialate.c thd_ttatlas_CA_EZ.c thd_Xdataset.c thd_base64.c
    thd_xwebber.c thd_shearwarp.c)
  list(REMOVE_ITEM THD_OBJS "${PROJECT_SOURCE_DIR}/${filename}")
endforeach(filename)
file(GLOB EDT_OBJS "edt_*.c")

file(GLOB CS_OBJS "cs_*.c")
list(APPEND CS_OBJS multivector.c afni_environ.c cl2.c machdep.c afni_logger.c cl1.c powell_int.c powell_newuoa.c rhdd.c zfun.c svdlib.c)
list(REMOVE_ITEM CS_OBJS "${PROJECT_SOURCE_DIR}/cs_color.c")
list(REMOVE_ITEM CS_OBJS "${PROJECT_SOURCE_DIR}/cs_simult.c")
list(REMOVE_ITEM CS_OBJS "${PROJECT_SOURCE_DIR}/cs_symeig23.c")

file(GLOB SUMA_OBJS "suma_*.c")
list(REMOVE_ITEM SUMA_OBJS "${PROJECT_SOURCE_DIR}/suma_gifti.c")

list(APPEND MISC_OBJS afni_suma.c vol2surf.c list_struct.c nifti_statlib.c matrix.c gifti_choice.c misc_math.c)

list(APPEND SOMEAFOBJS $<TARGET_OBJECTS:parser> afni_warp.c pbar_color_defs.c afni_ports.c)
foreach(type byte complex float rgbyte short)
	list(APPEND SOMEAFOBJS "${CMAKE_CURRENT_BINARY_DIR}/afni_slice_${type}.c")
endforeach(type)

set(LIBHEADERS 3ddata.h AFNI_label.h Amalloc.h afni_environ.h afni_suma.h
	afni_warp.h coxplot.h cs.h cs_qsort_small.h cs_sort_template.h
	debugtrace.h editvol.h eispack.h killer.h list_struct.h machdep.h
	mcw_glob.h mcw_malloc.h mri_dicom_stuff.h mri_warpfield.h mrilib.h
	multivector.h niml.h tagset.h thd_atlas.h thd_compress.h thd_iochan.h
	thd_maker.h thd_ttatlas_CA_EZ.h thd_ttatlas_query.h vecmat.h vol2surf.h
)
set(LIBHEADERS ${LIBHEADERS} "${CMAKE_CURRENT_BINARY_DIR}/AFNI_version.h")
set(IMOBJS display.c imseq.c bbox.c xim.c xutil.c LiteClue.c thd_xwebber.c)

file(GLOB AFNI_HIST_FILES "afni_history*.c")
foreach(filename afni_fimmer_compute.c afni_history.c afni_open.c
		afni_pcor_update.c afni_slice.c afni_vcheck.c ${AFNI_HIST_FILES})
	list(REMOVE_ITEM AFOBJS "${PROJECT_SOURCE_DIR}/${filename}")
endforeach(filename)

foreach(type byte float short)
	set(output_file "${CMAKE_CURRENT_BINARY_DIR}/afni_pcor_${type}.c")
	configure_file(afni_pcor_update.c "${output_file}" COPYONLY)
	set_source_files_properties("${output_file}" PROPERTIES COMPILE_DEFINITIONS "DTYPE=${type}")
	list(APPEND AFOBJS "${output_file}")
endforeach(type)

foreach(type byte complex float int rgba rgbyte short)
	set(output_file "${CMAKE_CURRENT_BINARY_DIR}/afni_slice_${type}.c")
	configure_file(afni_slice.c "${output_file}" COPYONLY)
	set_source_files_properties("${output_file}" PROPERTIES COMPILE_DEFINITIONS "DTYPE=${type}")
	list(APPEND AFOBJS "${output_file}")
endforeach(type)


function(quotize input basename)
	set(output "${basename}.h")
	add_custom_command(OUTPUT "${output}"
		COMMAND quotize "${basename}" < "${input}" > "${output}"
		MAIN_DEPENDENCY quotize)
endfunction(quotize)
quotize("${CMAKE_CURRENT_SOURCE_DIR}/../doc/README/README.copyright" license)
quotize("${CMAKE_CURRENT_SOURCE_DIR}/../doc/README/README.environment" readme_env)
quotize("${CMAKE_CURRENT_SOURCE_DIR}/../doc/README/README.afnigui" readme_afnigui)
quotize("${CMAKE_CURRENT_SOURCE_DIR}/afni_papers.txt" afni_papers)

function(configure_pcor lib_name definitions)
	set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${libname}.c")
	configure_file(pcor.c "${output_file}" COPYONLY)
	set_source_files_properties("${output_file}" PROPERTIES COMPILE_DEFINITIONS "${definitions}")
	add_library(${lib_name} OBJECT "${output_file}")
endfunction(configure_pcor)
configure_pcor(pcor "REF_FLOAT_SINGLE")
configure_pcor(pcorsh "REF_FLOAT_SINGLE;VOX_SHORT")

# foreach(filename colfit colcalc parser)
# 	add_custom_command(OUTPUT "${filename}.c"
# 		COMMAND ${F2C_BIN} "${PROJECT_SOURCE_DIR}/${filename}.f"
# 		MAIN_DEPENDENCY "${filename}.f")
# endforeach(filename)


foreach(target model_beta model_constant model_conv_PRF model_conv_PRF_6
		model_conv_PRF_DOG model_conv_cosine4 model_conv_diffgamma
		model_convgamma model_convgamma2a model_demri_3 model_diffexp
		model_diffusion model_exp model_expMEMRI model_expMEMRI3
		model_expr2 model_gammavar model_linear model_linplusort
		model_michaelis_menton model_null model_quadratic
		model_sinewave_ap model_sinewave_apf model_squarewave_ap
		model_squarewave_apf model_trnglwave_ap model_trnglwave_apf
		model_zero plug_3Ddump_V2 plug_3ddot plug_3ddup plug_4Ddump
		plug_L1fit plug_afnihistory plug_aslA3D3 plug_betafit plug_clust
		plug_compress plug_coorder plug_copy plug_crender
		plug_deconvolve plug_delay_V2 plug_drawdset plug_edit
		plug_extract plug_fourier plug_hemisub plug_histog
		plug_histog_multi plug_imreg plug_lsqfit plug_maskave
		plug_maskcalc plug_nlfit plug_notes plug_nth_dataset plug_nudge
		plug_null plug_permtest plug_power plug_realtime plug_rename
		plug_render plug_reorder plug_roiedit plug_roiplot plug_scatplot
		plug_second_dataset plug_stats plug_stavg plug_tag
		plug_threshold plug_ttget plug_vol2surf plug_volreg
		plug_wavelets plug_zeropad
)
	add_library(${target} ${target}.c)
	target_link_libraries(${target} mri)
endforeach(target)

foreach(target Xphace afni afni_open aiv)
  add_library(${target} ${target}.c)
  target_include_directories(${target} PUBLIC ${NIFTI_INCLUDE_DIRS} ${COXPLOT_INCLUDE_DIRS})
endforeach(target)

add_library(afobj ${AFOBJS})
add_library(mri ${MRI_OBJS} ${THD_OBJS} ${EDT_OBJS} ${CS_OBJS} ${SUMA_OBJS} ${MISC_OBJS} ${SOMEAFOBJS} ${NIML_OBJS} ${AUDIO_OBJS})
add_library(mrix ${IMOBJS})
add_library(parser OBJECT parser.c parser_int.c)
add_library(plug_drealtime plug_realtime.c)
add_library(plug_maxima plug_maxima.c maxima.c)
add_library(plug_retroicor plug_retroicor.c retroicor.c)

foreach(target 1dAstrip 1dBandpass 1dBport 1dCorrelate 1dFlagMotion 1dMarry
		1dTsort 1dUpsample 1dcat 1ddot 1dfft 1dgenARMA11 1dnorm 1dsum
		1dsvd 1dtranspose 24swap 2dImReg 2perm 2swap 3dABoverlap
		3dAFNIto3D 3dAFNItoANALYZE 3dAFNItoMINC 3dAFNItoNIFTI
		3dAFNItoNIML 3dAFNItoRaw 3dANALYZEtoAFNI 3dAcost 3dAnatNudge
		3dAnhist 3dAttribute 3dAutoTcorrelate 3dAutobox 3dAutomask
		3dBandpass 3dBlurInMask 3dBlurToFWHM 3dBrickStat 3dCM
		3dClipLevel 3dClustCount 3dClustSim 3dConformist 3dConvolve
		3dCountSpikes 3dDFT 3dDTeig 3dDTtoDWI 3dDWItoDT
		3dDegreeCentrality 3dDespike 3dEmpty 3dEntropy 3dErrtsCormat
		3dExtractGroupInCorr 3dExtrema 3dFDR 3dFFT 3dFWHM 3dFWHMx
		3dFourier 3dFriedman 3dGetrow 3dGroupInCorr 3dIntracranial
		3dInvFMRI 3dKruskalWallis 3dLFCD 3dLRflip 3dLSS 3dLocalACF
		3dLocalBistat 3dLocalHistog 3dLocalPV 3dLocalSVD 3dLocalstat
		3dMINCtoAFNI 3dMannWhitney 3dMean 3dMedianFilter 3dMultiThresh
		3dNLfim 3dNormalityTest 3dNotes 3dNwarpAdjust 3dNwarpApply
		3dNwarpCalc 3dNwarpCat 3dNwarpFuncs 3dNwarpXYZ 3dOverlap
		3dPeriodogram 3dPolyfit 3dPval 3dQwarp 3dREMLfit 3dROIstats
		3dRank 3dRankizer 3dRegAna 3dRetinoPhase 3dRowFillin
		3dSetupGroupInCorr 3dSharpen 3dSpatNorm 3dStatClust 3dSynthesize
		3dTRfix 3dTSgen 3dTagalign 3dTcat 3dTcorr1D 3dTcorrMap
		3dTcorrelate 3dTfilter 3dThreetoRGB 3dTnorm 3dToutcount
		3dToyProg 3dTproject 3dTqual 3dTshift 3dTsmooth 3dTsort
		3dTsplit4D 3dTwotoComplex 3dUndump 3dUnifize 3dUniformize
		3dUpsample 3dWarp 3dWarpDrive 3dWavelets 3dWilcoxon 3dWinsor
		3dXClustSim 3dXYZcat 3dZcat 3dZcutup 3dZeropad 3dZregrid
		3daxialize 3dbuc2fim 3dbucket 3dcopy 3ddelay 3ddot 3ddup 3dfim+
		3dfractionize 3dinfo 3dmaskSVD 3dmask_tool 3dmaskave 3dmatmult
		3dnewid 3dnoise 3dnvals 3dpc 3drefit 3drename 3drotate
		3dsvm_linpredict 3dtoXdataset 3dttest++ 3dvolreg 4swap AlphaSim
		DTIStudioFibertoSegments FIRdesign GLTsymtest Ifile RSFgen
		Vecwarp abut adwarp afni_vcheck apsearch bitvec byteorder
		cat_matvec cdf count dicom_hdr dicom_hinfo dicom_to_raw ent16
		epsim exx fdrval fftest float_scan from3d ftosh ge_header
		help_format ibinom im2niml images_equal imand imaver imcalc
		imcat imcutup imdump immask imrotate imstack imstat imupsam
		mayo_analyze mritopgm mycat myget nicat niccc nidset nimel
		niml_feedme nimltest nsize plugout_drive plugout_ijk plugout_tt
		plugout_tta quotize rmz rotcom rtfeedme sfim siemens_vision
		sqwave stimband strblast test_powell testcox tfim uncomment
		uniq_images unu warping_test whereami whirlgif
)
	add_executable(${target} ${target}.c)
	target_link_libraries(${target} mri mrix)
endforeach(target)

foreach(i "" 2 3)
	set(target 3dANOVA${i}) 
	add_executable(${target} ${target}.c 3dANOVA.lib)
	target_link_libraries(${target} mri mrix)
endforeach(i)

foreach(target 1dNLfit 1deval 1dmatcalc 3dDeconvolve 3dDetrend 3dTstat 3dcalc
		3dclust 3dhistog 3dmaskdump 3dmatcalc 3dmerge 3dproject 3dttest
		ccalc waver
)
	add_executable(${target} ${target}.c $<TARGET_OBJECTS:parser>)
	target_link_libraries(${target} mri mrix)
endforeach(target)
target_link_libraries(3dproject volpack)


add_executable(3dTfitter 3dTfitter.c thd_fitter.c thd_lasso.c)
add_executable(3dfim 3dfim.c afni_pcor_float.c ts.c)
add_executable(FD2 FD2.c ts.c mcw.c overfim.c $<TARGET_OBJECTS:pcorsh> csfft_AJ.c)
add_executable(fim2 fim2.c ts.c $<TARGET_OBJECTS:pcor>)
add_executable(imreg imreg.c $<TARGET_OBJECTS:pcor>)
add_executable(to3d ${T3OBJS})
add_executable(3danisosmooth 3danisosmooth.c DWIstructtensor.c)
add_executable(1dSEM 1dSEM.c sqrmat.c matrix.c)
add_executable(3dNLfim_n 3dNLfim.c)
add_executable(gwarp mri_gwarp.c)
add_executable(3dmaxima 3dmaxima.c maxima.c)
add_executable(3dDeconvolve_n 3dDeconvolve.c $<TARGET_OBJECTS:parser>)
add_executable(3dDeconvolve_f 3dDeconvolve.c $<TARGET_OBJECTS:parser>)
add_executable(3dAllineate 3dAllineate.c mri_genalign.c mri_genalign_util.c thd_correlate.c)
add_executable(3dECM 3dECM.c sparse_array.c)
add_executable(3dMSE 3dMSE.c sparse_array.c)
add_executable(1dgrayplot 1dgrayplot.c display.c)
add_executable(1dplot 1dplot.c display.c)
add_executable(3dretroicor 3dretroicor.c retroicor.c)
add_executable(3dedge3 ${3DEDGE_INCLUDE_DIRS}/3dedge3.c)


set_source_files_properties(3dDeconvolve_f PROPERTIES COMPILE_DEFINITIONS "FLOATIZE")
set_source_files_properties(gifti_choice.c PROPERTIES COMPILE_DEFINITIONS "DTYPE=${type}")

set_target_properties(afni PROPERTIES PUBLIC_HEADER "${AFNI_HEADERS}")
set_target_properties(mri PROPERTIES PUBLIC_HEADER ${LIBHEADERS})



message(Nifti existence ${NIFTI_INCLUDE_DIRS})
# target_include_directories(afni_warp PUBLIC ${COXPLOT_INCLUDE_DIRS})
target_include_directories(3dDeconvolve PUBLIC ${COXPLOT_INCLUDE_DIRS})
target_include_directories(3dDeconvolve_f PUBLIC ${COXPLOT_INCLUDE_DIRS})
target_include_directories(3dMSE PUBLIC ${COXPLOT_INCLUDE_DIRS})
target_include_directories(3dretroicor PUBLIC ${COXPLOT_INCLUDE_DIRS} ${CONVERTED_FROM_FORTRAN_INCLUDE_DIRS} ${NIFTI_INCLUDE_DIRS})
target_include_directories(3DEdge PUBLIC ${CONVERTED_FROM_FORTRAN_INCLUDE_DIRS} ${COXPLOT_INCLUDE_DIRS})
target_include_directories(SUMA PUBLIC ${CONVERTED_FROM_FORTRAN_INCLUDE_DIRS} ${COXPLOT_INCLUDE_DIRS})
target_include_directories(rickr PUBLIC ${CONVERTED_FROM_FORTRAN_INCLUDE_DIRS} ${COXPLOT_INCLUDE_DIRS})
target_include_directories(coxplot PUBLIC ${CONVERTED_FROM_FORTRAN_INCLUDE_DIRS})
target_include_directories(eispack PUBLIC ${CONVERTED_FROM_FORTRAN_INCLUDE_DIRS})
target_include_directories(mri  PUBLIC ${NIFTI_INCLUDE_DIRS} ${RICKR_INCLUDE_DIRS}
  ${3DEDGE_INCLUDE_DIRS} ${AUDIO_INCLUDE_DIRS} ${COXPLOT_INCLUDE_DIRS}
  )
target_include_directories(mrix PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${XMHTML_INCLUDE_DIRS} ${GIFTI_INCLUDE_DIRS} ${NIFTI_INCLUDE_DIRS} ${COXPLOT_INCLUDE_DIRS})
target_include_directories(afobj PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${NIFTI_INCLUDE_DIRS} ${XMHTML_INCLUDE_DIRS} ${GIFTI_INCLUDE_DIRS} ${COXPLOT_INCLUDE_DIRS})
target_include_directories(parser PRIVATE ${NIFTI_INCLUDE_DIRS}
${NIML_INCLUDE_DIRS} ${RICKR_INCLUDE_DIRS} ${CONVERTED_FROM_FORTRAN_INCLUDE_DIRS})
target_include_directories(to3d PRIVATE ${MOTIF_INCLUDE_DIR} )
target_include_directories(1dplot PRIVATE  ${XT_INCLUDE_DIR} ${X11_INCLUDE_DIRS} )

foreach(target 3dfim 3dTfitter FD2 fim2 imreg gwarp 3danisosmooth 3dmaxima
		3dNLfim_n 3dAllineate 3dECM 3dMSE 1dSEM 
		plug_drealtime plug_maxima plug_retroicor
)

   target_link_libraries(${target} mri rickr mrix)
endforeach(target)
target_link_libraries(3dretroicor coxplot mri converted_from_fortran mrix)

# target_link_libraries(parser coxplot)
target_link_libraries(1dgrayplot mri coxplot ${XT_LIBRARIES})
target_link_libraries(1dplot mrix mri nifti2_io ${X11_Xt_LIB} ${X11_LIBRARIES})
target_link_libraries(3dDeconvolve mri coxplot)
target_link_libraries(3dDeconvolve_f mri coxplot)
target_link_libraries(3dDeconvolve_n mri coxplot)
target_link_libraries(Xphace mrix nifti2_io)
target_link_libraries(afni afobj)
target_link_libraries(afni_open afobj)
target_link_libraries(afobj ${XT_LIBRARIES} mrix coxplot)
target_link_libraries(aiv mrix nifti2_io)
# target_include_directories(aiv ${NIFTI_INCLUDE_DIRS})

target_link_libraries(mri mrix rickr nifti2_io eispack netcdf m znz nifticdf giftiio 3DEdge volpack
   ${XMHTML_LIBRARIES} ${CONVERTED_FROM_FORTRAN_LIBRARIES} ${XT_LIBRARIES})
target_link_libraries(mrix coxplot)
target_link_libraries(to3d mri mrix nifti2_io ${MOTIF_LIBRARIES} ${XT_LIBRARIES})

target_link_libraries(3dedge3 nifti2_io mri)

install(TARGETS afni
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
