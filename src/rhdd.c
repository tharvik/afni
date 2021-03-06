#include "mrilib.h"

/*---------------------------------------------------------------------------*/
/* C0 and C2 box splines with rhombic dodecahedron (RHDD) support.  From
    A Entezari, D van de Ville, T Moeller.
    Practical Box Splines for Reconstruction on the Body Centered Cubic Lattice.
    IEEE Transactions on Visualization and Computer Graphics  14:313-328, 2008.
    http://doi.ieeecomputersociety.org/10.1109/TVCG.2007.70429
   Each function is 1 at (x,y,z)=(0,0,0) and goes to 0 at the edge of its RHDD.
   RHDD(a) is defined as the 12 sided polyhedron set
     { (x,y,z) such that max(|x|+|y|,|x|+|z|,|y|+|z|) < a }
   It has volume 2*a^3.  The 14 vertices are given by the points
     (a/2,a/2,a/2) {8 points = each coordinate independently can be + or -}
     (a,0,0) (0,a,0) (0,0,a) {6 more points, with + or -}
   To fill space with overlapping RHDD(a) objects, sprinkle them on the
   lattice of vertices; i.e., put an RHDD center at each point
     0.5*i * (-a,a,a) + 0.5*j * (a,-a,a) + 0.5*k * (a,a,-a)
   with (i,j,k) being integers.
   For example, (x,y,z)=(a/2,a/2,a/2) is given by (i,j,k)=(1,1,1);
                (x,y,z)=(a,0,0)       is given by (i,j,k)=(0,1,1).
   The 14 immediate neighbors of (0,0,0) are given by the (i,j,k) triples
     (1,0,0)  (-1, 0, 0)  (0,1,0)  (0 ,-1, 0)  (0,0,1)  (0, 0,-1)
     (1,1,0)  (-1,-1, 0)  (1,0,1)  (-1, 0,-1)  (0,1,1)  (0,-1,-1)
     (1,1,1)  (-1,-1,-1)

   This lattice is generated by the matrix L, applied to integer vectors
   [i j k]':

                [ -1  1  1 ]                        [  0  1  1 ]
      L = (a/2) [  1 -1  1 ]    with inv(L) = (1/a) [  1  0  1 ]
                [  1  1 -1 ]                        [  1  1  0 ]

   Note that this is NOT the space-filling lattice of RHDDs; the RHDDs
   generated by the above description overlap so that multiple RHDD basis
   functions contribute to each location.
*//*-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* If needed, swap so that first argument ends up as the smaller of the pair */

#undef  ISWAP
#define ISWAP(a,b) if( (b) < (a) ) ( tt=(a), (a)=(b), (b)=tt )

/*---------------------------------------------------------------------------*/
/*! C0 basis function with RHDD(1) support (piecewise linear).
    These should be spaced on the RHDD lattice with a=1.
*//*-------------------------------------------------------------------------*/

float rhddc0( float x, float y, float z )
{
   register float xx, yy, zz, tt ;

   xx = fabsf(x) ; if( xx >= 1.0f ) return 0.0f ;  /* way too big */
   yy = fabsf(y) ; if( yy >= 1.0f ) return 0.0f ;
   zz = fabsf(z) ; if( zz >= 1.0f ) return 0.0f ;
   ISWAP(zz,yy) ;  /* sort so xx >= yy >= zz */
   ISWAP(zz,xx) ;  /* ISWAP(yy,xx) ; */

   tt = xx+yy ;
   if( tt >= 1.0f ) return ( 0.0f ) ;     /* outside RHDD(1) */
                    return ( 1.0f-tt ) ;  /* linear inside  */
}

/*---------------------------------------------------------------------------*/

   /* constants for rhddc2() function */

#undef  ALPHA
#undef  BETA
#undef  GAMMA
#define ALPHA  0.0026041667f  /* 1/384 */
#define BETA   0.0052083333f  /* 1/192 */
#define GAMMA  0.0104166667f  /* 1/96  */

/*---------------------------------------------------------------------------*/
/*! C2 basis function with RHDD(2) support (piecewise quintic).
    These should be spaced on the RHDD lattice with a=1 to make a basis
    set for 3D functions (i.e., they overlap with their nearest neighbors).
*//*-------------------------------------------------------------------------*/

float rhddc2( float x, float y, float z )
{
   register float xx, yy, zz, tt, xz2,yz2,xy2 ;

   xx = fabsf(x) ; if( xx >= 2.0f ) return 0.0f ;  /* way too big */
   yy = fabsf(y) ; if( yy >= 2.0f ) return 0.0f ;
   zz = fabsf(z) ; if( zz >= 2.0f ) return 0.0f ;
   ISWAP(zz,yy) ;  /* sort so that xx >= yy >= zz */
   ISWAP(zz,xx) ; ISWAP(yy,xx) ;

   /* Entezari paper is in terms of RHDD(4) support, so scale coords by 2 */

   xx *= 2.0f ;  yy *= 2.0f ; zz *= 2.0f ;
   tt = xx+yy-4.0f ;
   if( tt >= 0.0f ) return 0.0f ;  /* outside RHDD(4) */

   xz2 = xx+zz-2.0f ; yz2 = yy+zz-2.0f ; xy2 = tt+2.0f ;

#undef  PA
#define PA ALPHA * tt*tt*tt                                           \
           * ( -3.0f*xx*yy - 5.0f*zz*zz + 2.0f*(xx+yy) + 20.0f*zz     \
              + xx*xx + yy*yy - 24.0f )
#undef  PB1
#define PB1 BETA * xz2*xz2*xz2                                        \
            * (  xx*xx - 9.0f*xx - 3.0f*xx*zz + 10.0f*yy - 5.0f*yy*yy \
               + 14.0f + 11.0f*zz + zz*zz )
#undef  PB2
#define PB2 BETA * yz2*yz2*yz2                                        \
            * (  46.0f - 30.0f*xx - zz - yy + 3.0f*zz*yy + 5.0f*xx*xx \
               - yy*yy - zz*zz )

   if( xy2 <= 0.0f ){             /** Region 1 **/
     return (  PA + PB1 + PB2
             - GAMMA * xy2*xy2*xy2
              * ( xx*xx + xx - 3.0f*xx*yy - 5.0f*zz*zz + yy*yy + yy - 6.0f ) ) ;
   }

   if( xz2 <= 0.0f ){             /** Region 2 **/
     return ( PA + PB1 + PB2 ) ;
   }

   if( yz2 <= 0.0f ){             /** Region 3 **/

     if( xx-zz >= 2.0f ){         /* Region 3A */

       return ( ALPHA * tt*tt*tt
               * ( -xx*xx + 8.0f*xx + 3.0f*xx*yy - yy*yy + 5.0f*zz*zz
                   -16.0f - 12.0f*yy ) ) ;

     } else {                     /* Region 3B */

       return( PA + PB2 ) ;

     }

   }

   /* Region 4 */

   return ( PA ) ;                /** Region 4 **/
}
