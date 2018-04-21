/* Copyright (C) 2018, Project Pluto

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA. */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "mpc_func.h"

#define MAX_DEPTH 20

typedef struct
{
   int depth, tags[MAX_DEPTH];
   char line[83];    /* allow possible CR, LF,  & null */
   char line2[83];
   char rms_ra[20], rms_dec[20], corr[20];
   char rms_mag[20], rms_time[20];
   int id_set, getting_lines;
   int prev_line_passed_through;
} ades2mpc_t;

void *init_ades2mpc( void)
{
   ades2mpc_t *rval = (ades2mpc_t *)calloc( 1, sizeof( ades2mpc_t));

   return( rval);
}

/* In theory,  when we reach the end of the file,  any 'state' flags that
got set (because of encountering ADES tags) should be cleared (because of
encountering closing tags).  */

int free_ades2mpc_context( void *context)
{
   ades2mpc_t *tptr = (ades2mpc_t *)context;
   const int64_t rval = tptr->depth;

   free( context);
   return( rval);
}

   /* see 'adestags.c' for code that generates these #defines.  Might use
   it to generate a sorted list of tags,  so that a binary search could
   be done,  to speed things up (just barely). */
#define ADES_permID                        1
#define ADES_provID                        2
#define ADES_artSat                        3
#define ADES_trkSub                        4
#define ADES_obsID                         5
#define ADES_trkID                         6
#define ADES_mode                          7
#define ADES_stn                           8
#define ADES_trx                           9
#define ADES_rcv                          10
#define ADES_sys                          11
#define ADES_ctr                          12
#define ADES_pos1                         13
#define ADES_pos2                         14
#define ADES_pos3                         15
#define ADES_posCov11                     16
#define ADES_posCov12                     17
#define ADES_posCov13                     18
#define ADES_posCov22                     19
#define ADES_posCov23                     20
#define ADES_posCov33                     21
#define ADES_prog                         22
#define ADES_obsTime                      23
#define ADES_ra                           24
#define ADES_dec                          25
#define ADES_raStar                       26
#define ADES_decStar                      27
#define ADES_obsCenter                    28
#define ADES_deltaRA                      29
#define ADES_deltaDec                     30
#define ADES_dist                         31
#define ADES_pa                           32
#define ADES_rmsRA                        33
#define ADES_rmsDec                       34
#define ADES_rmsDist                      35
#define ADES_rmsPA                        36
#define ADES_rmsCorr                      37
#define ADES_delay                        38
#define ADES_rmsDelay                     39
#define ADES_doppler                      40
#define ADES_rmsDoppler                   41
#define ADES_astCat                       42
#define ADES_mag                          43
#define ADES_rmsMag                       44
#define ADES_band                         45
#define ADES_photCat                      46
#define ADES_photAp                       47
#define ADES_nucMag                       48
#define ADES_logSNR                       49
#define ADES_seeing                       50
#define ADES_exp                          51
#define ADES_rmsFit                       52
#define ADES_nStars                       53
#define ADES_com                          54
#define ADES_frq                          55
#define ADES_ref                          56
#define ADES_disc                         57
#define ADES_subFmt                       58
#define ADES_subFrm                       59
#define ADES_precTime                     60
#define ADES_precRA                       61
#define ADES_precDec                      62
#define ADES_uncTime                      63
#define ADES_notes                        64
#define ADES_remarks                      65
#define ADES_deprecated                   66
#define ADES_localUse                     67
#define ADES_orbProd                      68
#define ADES_orbID                        69
#define ADES_resRA                        70
#define ADES_resDec                       71
#define ADES_selAst                       72
#define ADES_sigRA                        73
#define ADES_sigDec                       74
#define ADES_sigCorr                      75
#define ADES_sigTime                      76
#define ADES_biasRA                       77
#define ADES_biasDec                      78
#define ADES_biasTime                     79
#define ADES_photProd                     80
#define ADES_resMag                       81
#define ADES_selPhot                      82
#define ADES_sigMag                       83
#define ADES_biasMag                      84
#define ADES_photMod                      85
#define ADES_resDelay                     86
#define ADES_selDelay                     87
#define ADES_sigDelay                     88
#define ADES_resDoppler                   89
#define ADES_selDoppler                   90
#define ADES_sigDoppler                   91
#define ADES_observatory                  92
#define ADES_submitter                    93
#define ADES_observers                    94
#define ADES_measurers                    95
#define ADES_telescope                    96
#define ADES_software                     97
#define ADES_coinvestigators              98
#define ADES_collaborators                99
#define ADES_fundingSource               100
#define ADES_comment                     101
#define ADES_optical                     102
#define ADES_offset                      103
#define ADES_occultation                 104
#define ADES_radar                       105
#define ADES_obsContext                  106
#define ADES_obsData                     107
#define ADES_obsBlock                    108
#define ADES_opticalResidual             109
#define ADES_radarResidual               110
#define ADES_ades                        111
#define ADES_MPCID                       112
#define ADES_OpticalID                   113
#define ADES_RadarID                     114
#define ADES_RadarValue                  115
#define ADES_Precision                   116
#define ADES_Location                    117
#define ADES_Photometry                  118
#define ADES_OffsetVal                   119
#define ADES_OpticalRes                  120
#define ADES_OpticalResMag               121
#define ADES_OpticalResiduals            122
#define ADES_RadarResiduals              123
#define ADES_mpcCode                     124
#define ADES_institution                 125
#define ADES_design                      126
#define ADES_aperture                    127
#define ADES_detector                    128
#define ADES_fRatio                      129
#define ADES_filter                      130
#define ADES_arrayScale                  131
#define ADES_pixelScale                  132
#define ADES_astrometry                  133
#define ADES_fitOrder                    134
#define ADES_photometry                  135
#define ADES_objectDetection             136
#define ADES_line                        137
#define ADES_name                        138

static int find_tag( const char *buff, size_t len)
{
   static const char *tags[] = {
         "permID", "provID", "artSat", "trkSub", "obsID", "trkID",
         "mode", "stn", "trx", "rcv", "sys", "ctr", "pos1", "pos2", "pos3",
         "posCov11", "posCov12", "posCov13",
         "posCov22", "posCov23", "posCov33",
         "prog", "obsTime", "ra", "dec", "raStar", "decStar", "obsCenter",
         "deltaRA", "deltaDec", "dist", "pa", "rmsRA", "rmsDec", "rmsDist",
         "rmsPA", "rmsCorr", "delay", "rmsDelay", "doppler", "rmsDoppler",
         "astCat", "mag", "rmsMag", "band", "photCat", "photAp",
         "nucMag", "logSNR", "seeing", "exp",
                     /* end p6, start p7 */
         "rmsFit", "nStars", "com",
         "frq", "ref", "disc", "subFmt", "subFrm",
                     /* end p7/start p8 */
         "precTime", "precRA", "precDec", "uncTime", "notes", "remarks",
         "deprecated", "localUse", "orbProd", "orbID", "resRA", "resDec",
         "selAst", "sigRA", "sigDec", "sigCorr", "sigTime", "biasRA",
         "biasDec", "biasTime", "photProd", "resMag", "selPhot",
                     /* end p9/start p10 */
         "sigMag", "biasMag", "photMod", "resDelay", "selDelay", "sigDelay",
         "resDoppler", "selDoppler", "sigDoppler", "observatory", "submitter",
         "observers", "measurers", "telescope", "software", "coinvestigators",
         "collaborators", "fundingSource",
                     /* end p10/start p11 */
         "comment", "optical", "offset", "occultation", "radar", "obsContext",
         "obsData", "obsBlock", "opticalResidual", "radarResidual", "ades",
                     /* start p 18: complex types */
         "MPCID", "OpticalID", "RadarID", "RadarValue", "Precision", "Location",
         "Photometry", "OffsetVal", "OpticalRes", "OpticalResMag",
         "OpticalResiduals", "RadarResiduals",
                     /* start p 25 */
         "mpcCode", "institution", "design", "aperture", "detector", "fRatio",
         "filter", "arrayScale", "pixelScale", "astrometry", "fitOrder",
         "photometry", "objectDetection", "line",
         "name",
         NULL };

   int i, rval = -1;

   if( *buff == '/')       /* closing tag */
      {
      buff++;
      len--;
      }
   if( !memcmp( buff, "ades", 4))
      rval = 0;
   for( i = 0; tags[i] && rval == -1; i++)
      if( !memcmp( tags[i], buff, len) && !tags[i][len])
         rval = i + 1;
   return( rval);
}

static inline void move_fits_time( char *optr, const char *iptr)
{
   if( iptr[4] == '-' && iptr[7] == '-' && iptr[10] == 'T'
               && iptr[13] == ':')
      {
      *optr++ = (char)((iptr[0] * 10 + iptr[1] - '0' * 11) + 'A' - 10);
      iptr += 2;
      while( *iptr && *iptr != 'Z')
         {
         if( *iptr >= '0' && *iptr <= '9')
            *optr++ = *iptr;
         else if( *iptr == 'T')
            *optr++ = ':';
         iptr++;
         }
      }
}

static inline void place_value( char *optr, const char *iptr, const size_t ilen,
             size_t leading_places)
{
   size_t i, point_loc = ilen;

   for( i = 0; i < ilen; i++)
      if( iptr[i] == '.')
         point_loc = i;
   assert( leading_places >= point_loc);
   while( leading_places > point_loc)
      {
      *optr++ = '0';
      leading_places--;
      }
   memcpy( optr, iptr, ilen);
}

static const char *skip_whitespace( const char *tptr)
{
   while( *tptr && isspace( *tptr))
      tptr++;
   return( tptr);
}

static int get_a_line( char *obuff, ades2mpc_t *cptr)
{
   if (cptr->rms_ra[0])
      {
      sprintf( obuff, "COM Sigmas %s", cptr->rms_ra);
      if( strcmp( cptr->rms_ra, cptr->rms_dec))
         {
         sprintf( obuff + strlen( obuff), "x%s", cptr->rms_dec);
         if( atof( cptr->corr))
            sprintf( obuff + strlen( obuff), ",%s", cptr->corr);
         }
      if( cptr->rms_mag[0])
         {
         strcat( obuff, " m:");
         strcat( obuff, cptr->rms_mag);
         cptr->rms_mag[0] = '\0';
         }
      if( cptr->rms_time[0])
         {
         strcat( obuff, " t:");
         strcat( obuff, cptr->rms_time);
         cptr->rms_mag[0] = '\0';
         }
      cptr->rms_ra[0] = '\0';
      strcat( obuff, "\n");
      }
   else if( cptr->line[0])
      {
      strcpy( obuff, cptr->line);
      cptr->line[0] = '\0';
      }
   else if( cptr->line2[0])
      {
      strcpy( obuff, cptr->line2);
      cptr->line2[0] = '\0';
      }
   else                         /* got all the lines we'll get */
      cptr->getting_lines = 0;
   return( cptr->getting_lines);
}

#define ADES_UNHANDLED_TAG                (-1)
#define ADES_CLOSING_UNOPENED_TAG         (-2)
#define ADES_DEPTH_MAX                    (-3)

int xlate_ades2mpc( void *context, char *obuff, const char *buff)
{
   int rval = 0;
   const char *tptr;
   ades2mpc_t *cptr = (ades2mpc_t *)context;

   if( cptr->prev_line_passed_through)
      {
      cptr->prev_line_passed_through = 0;
      return( 0);
      }
   if( !cptr->depth && !strstr( buff, "<ades version"))
      {
      strcpy( obuff, buff);
      cptr->prev_line_passed_through = 1;
      return( 1);
      }
   if( cptr->getting_lines)
      return( get_a_line( obuff, cptr));
   tptr = skip_whitespace( buff);
   while( rval >= 0 && *tptr)
      {
      size_t len = 0;

      if( *tptr == '<')        /* start of tag,  we assume */
         {
         while( tptr[len] && tptr[len] != '>')
            len++;
         if( tptr[len] != '>')
            printf( "Tag didn't close: '%s'\n", tptr);
         else
            {
            const int tag_idx = find_tag( tptr + 1, len - 1);

            if( tag_idx < 0)
               {
               if( memcmp( tptr, "?xml version=", 13))
                  rval = ADES_UNHANDLED_TAG;
               }
            else
               {
               if( tptr[1] == '/')
                  {
                  cptr->depth--;
                  if( cptr->depth < 0 || cptr->tags[cptr->depth] != tag_idx)
                     rval = ADES_CLOSING_UNOPENED_TAG;
                  }
               else
                  {
                  cptr->tags[cptr->depth++] = tag_idx;
                  if( cptr->depth == MAX_DEPTH)
                     rval = ADES_DEPTH_MAX;
                  }
               if( tag_idx == ADES_optical)
                  {
                  if( tptr[1] == '/')
                     cptr->getting_lines = rval = 1;
                  else
                     {
                     memset( cptr->line, ' ', 80);
                     strcpy( cptr->line + 80, "\n");
                     memset( cptr->line2, ' ', 80);
                     strcpy( cptr->line2 + 80, "\n");
                     cptr->line2[0] = '\0';
                     cptr->id_set = 0;
                     }
                  }
               }
            }
         tptr += len + 1;
         }
      else if( cptr->depth)
         {
         const int itag = cptr->tags[cptr->depth - 1];
         char name[20];

         while( tptr[len] && tptr[len] != '<')
            len++;
         while( len && tptr[len - 1] == ' ')
            len--;
         if( len < sizeof( name))
            {
            memcpy( name, tptr, len);
            name[len] = '\0';
            }
         switch( itag)
            {
            case ADES_mpcCode:
               sprintf( obuff, "COD %s\n", name);
               rval = 1;
               break;
            case ADES_name:
               {
               const char *format = NULL;

               if( cptr->depth > 1)
                  switch( cptr->tags[cptr->depth - 2])
                     {
                     case ADES_mpcCode:
                        format = "COM %.*s\n";
                        break;
                     case ADES_observatory:
                        format = "COM Observatory name %.*s\n";
                        break;
                     case ADES_observers:
                        format = "OBS %.*s\n";
                        break;
                     case ADES_measurers:
                        format = "MEA %.*s\n";
                        break;
                     case ADES_submitter:
                        format = "CON %.*s\n";
                        break;
                     }
               if( format)
                  sprintf( obuff, format, (int)len, tptr);
               else
                  strcpy( obuff, "COM Mangled name data\n");
               rval = 1;
               }
               break;
            case ADES_stn:
               memcpy( cptr->line + 77, tptr, 3);
               break;
            case ADES_obsTime:
               move_fits_time( cptr->line + 15, tptr);
               break;
            case ADES_band:
               cptr->line[70] = *tptr;
               break;
            case ADES_mode:
               if( len == 3 && !memcmp( tptr, "CCD", 3))
                  cptr->line[14] = 'C';
               break;
            case ADES_prog:
               {
               const char *programs = "0123456789!\"#$%&'()*+,-./[\\]^_`{|}~";
               const int idx = atoi( tptr);

               if( idx >=0 && idx <= 34)
                  cptr->line[13] = programs[idx];
               }
               break;
            case ADES_sys:
               cptr->line2[0] = ' ';
               if( !strcmp( name, "ICRF_KM"))
                  cptr->line2[32] = '1';
               else if( !strcmp( name, "ICRF_AU"))
                  cptr->line2[32] = '2';
               else
                  {
                  strcpy( obuff, "Bad <sys> tag\n");
                  assert( 1);
                  }
               cptr->line2[14] = 's';
               cptr->line[14] = 'S';
               break;
            case ADES_pos1:
            case ADES_pos2:
            case ADES_pos3:
               {
               int dec_loc;
               char *tptr2 = strchr( name, '.');

               assert( tptr2);
               dec_loc = 40 + (itag - ADES_pos1) * 12;
               if( *name == '-')
                  cptr->line2[dec_loc - 6] = '-';
               else
                  cptr->line2[dec_loc - 6] = '+';
               if( *name == '-' || *name == '+')
                  *name = ' ';
               dec_loc -= (tptr2 - name);
               if( cptr->line2[32] == '2')
                  dec_loc -= 4;
               else if( cptr->line2[32] != '1')
                  {
                  strcpy( obuff, "Bad posn data\n");
                  rval = 1;
                  }
               memcpy( &cptr->line2[dec_loc], name, len);
               }
               break;
            case ADES_ra:
               if( *tptr == '+')
                  {
                  tptr++;
                  len--;
                  }
               place_value( cptr->line + 32, tptr, len, 3);
               break;
            case ADES_dec:
               if( *tptr == '-' || *tptr == '+')
                  {
                  cptr->line[44] = *tptr++;
                  len--;
                  }
               else
                  cptr->line[44] = '+';
               place_value( cptr->line + 45, tptr, len, 2);
               break;
            case ADES_provID:
               if( cptr->id_set == ADES_permID)
                  break;                  /* FALLTHRU */
            case ADES_astCat:
               assert( len < sizeof( name));
               cptr->line[71] = net_name_to_byte_code( name);
               break;
            case ADES_rmsRA:
               assert( len < 20);
               strcpy( cptr->rms_ra, name);
               break;
            case ADES_rmsDec:
               assert( len < 20);
               strcpy( cptr->rms_dec, name);
               break;
            case ADES_rmsCorr:
               assert( len < 20);
               strcpy( cptr->corr, name);
               break;
            case ADES_uncTime:
               assert( len < 20);
               strcpy( cptr->rms_time, name);
               break;
            case ADES_rmsMag:
               assert( len < 20);
               strcpy( cptr->rms_mag, name);
               break;
            case ADES_permID:
               {
               char tbuff[20];

               assert( len < sizeof( name));
               create_mpc_packed_desig( tbuff, name);
               memcpy( cptr->line, tbuff, 12);
               cptr->id_set = itag;
               }
               break;
            case ADES_artSat:
               assert( len < 13);
               if( !cptr->id_set)
                  {
                  cptr->id_set = ADES_artSat;
                  memcpy( cptr->line, tptr, len);
                  }
               break;
            case ADES_trkSub:
               assert( len < 8);
               if( !cptr->id_set)
                  {
                  cptr->id_set = ADES_trkSub;
                  memcpy( cptr->line + 5, tptr, len);
                  }
               break;
            case ADES_mag:
               memcpy( cptr->line + 65, tptr, (len < 5) ? len : 5);
               break;
            }
         tptr += len;
         }
      tptr = skip_whitespace( tptr);
      }
   if( rval)
      {
      if( cptr->line2[0])     /* yes,  we've a valid line 2 */
         {
         memcpy( cptr->line2, cptr->line, 12);
         memcpy( cptr->line2 + 15, cptr->line + 15, 17);
         memcpy( cptr->line2 + 77, cptr->line + 77, 3);
         }
      get_a_line( obuff, cptr);
      }
   return( rval);
}

int xlate_ades2mpc_in_place( void *context, char *buff)
{
   char temp_obuff[100];
   const int rval = xlate_ades2mpc( context, temp_obuff, buff);

   if( rval)
      strcpy( buff, temp_obuff);
   return( rval);
}

#ifdef TEST_CODE

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( argv[1], "rb");
   char buff[200];
   void *ades_context = init_ades2mpc( );
   int rval;

   while( fgets( buff, sizeof( buff), ifile))
      {
      if( argc > 2)        /* verbose mode */
         printf( "Input line %s", buff);
      while( xlate_ades2mpc_in_place( ades_context, buff) > 0)
         printf( "%s", buff);
      }
   rval = free_ades2mpc_context( ades_context);
   printf( "rval = %d\n", rval);
   return( 0);
}
#endif
