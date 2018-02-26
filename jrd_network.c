/*=================================================================================================
***************************************************************************************************
***************************************************************************************************
** FILE        :
**                                 jrd_oem_network.c
** DESCRIPTION :
**                            JRD OEM NETWORK process file
**
***************************************************************************************************
*******************Copyright (c) 2015 by JRDCOM Incorporated.  All Rights Reserved.****************
***************************************************************************************************
=================================================================================================*/
/*==========================Details=================================
  This file is internal processor of network module.
  Contains:
  1. api of webui act.
  2. network parameters init.
  3. indication api
*=================================================================/
/*=========================================EditHistory=============================================
This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

    when		      who			     what, where, why
------------- 	-------------	----------------------------------------------------------------------

10/24/2015	   Ace.ye	                  initial version
=================================================================================================*/
/*==============================HeadFile===============================*/
#include "jrd_oem.h"
#include "jrd_oem_common.h"
#include "comdef.h"
#include "jrd_oem_indication.h"
#include "jrd_network_al.h"
#include "jrd_network_ext.h"
#include "jrd_network.h"
#include "jrd_usim.h"
#include "jrd_diag.h"
#include "jrd_sys_al.h"
#include "jrd_connection.h"
/*==============================MACRO DEFINE===========================*/

/*==============================Local Var===============================*/
static jrd_oem_nw_cache_t jrd_oem_nw_param_cache;
static jrd_oem_nw_cache_t* p_jrd_oem_nw_param_cache = &jrd_oem_nw_param_cache;
/*==============================Data Structure===========================*/
/*==============================Function===============================*/
/*===========================================================================

FUNCTION jrd_oem_nw_get_cache

DESCRIPTION
  Get local cache pointer

DEPENDENCIES
  None.

RETURN VALUE
  memory address of local cache pointer.

SIDE EFFECTS
  None.

===========================================================================*/
 jrd_oem_nw_cache_t* jrd_oem_nw_get_cache(void)
{
  return p_jrd_oem_nw_param_cache;
}
/*===========================================================================

FUNCTION jrd_oem_airtel_name_feature

DESCRIPTION
  Fill the network name Airtel 1/Airtel 2/Airtel 3 in Vodafone/Idea/Aircel network.

DEPENDENCIES
  None.

RETURN VALUE
  0 -- fail    -1 -- vodafone    -2 -- idea    -3 -- aircel

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_oem_airtel_name_feature(uint32 cur_nw_plmn)
{
	uint32 vodafone_plmn[] = {404011,
							404030,
							404005,
							404046,
							404027,
							404043,
							404020,
							404013,
							404084,
							404086,
							405067,
							404088,
							405066,
							404015,
							404060,
							404001,
							405756,
							405751,
							405754,
							405753,
							405755,
							405752,
							405750};
	uint32 idea_plmn[] = {404022,
						404024,
						404007,
						404078,
						404004,
						404012,
						404056,
						404019,
						404089,
						404087,
						404082,
						405799,
						405070,
						405845,
						405846,
						404044,
						405848,
						405849,
						405850,
						404014,
						405852,
						405853};
	uint32 aircel_plmn[] = {404028,
						404037};
	uint8 plmn_count = 0;
	plmn_count = sizeof(vodafone_plmn)/sizeof(vodafone_plmn[0]);
	while(plmn_count--)
	{
		if(cur_nw_plmn == vodafone_plmn[plmn_count])
			return 1;
	}
	plmn_count = sizeof(idea_plmn)/sizeof(idea_plmn[0]);
	while(plmn_count--)
	{
		if(cur_nw_plmn == idea_plmn[plmn_count])
			return 2;
	}
	plmn_count = sizeof(aircel_plmn)/sizeof(aircel_plmn[0]);
	while(plmn_count--)
	{
		if(cur_nw_plmn == aircel_plmn[plmn_count])
			return 3;
	}
	return 0;
}
/*===========================================================================

FUNCTION jrd_network_compare_simplmn_netplmn

DESCRIPTION
  compare the simcard plmn with network plmn,if they are equal,return true,otherwise false.

DEPENDENCIES
  None.

RETURN VALUE
  0 -- fail    -1 -- success

SIDE EFFECTS
  None.

===========================================================================*/
static boolean jrd_network_compare_simplmn_netplmn(uint32 simplmn,uint32 netplmn)
{
  unsigned int sim_count=0;
  uint32 hplmn_plmn_table[][2] = {
  						     {404004,404011},   // 3G
                                                 {405848,404030},
                                                 {405852,404043},
                                                 {405852,404084},
                                                 {405799,404092},
                                                 {405070,405052},
                                                 {405849,404016},
                                                 {405845,405056},
                                                 {404087,404070},
						     {404044,404045},
						     {405853,405051}
  							  };
#if 0
{404096,404001},// 2G-3G
{404096,404012},
{404002,404014},
{404097,405066},
{404031,404030},
{404090,404027},
{404090,404022},
{404098,404005},
{404098,404024},
{404093,404078},
{404095,404019},
{405054,404015},
{405054,404089},
{405055,405750}
#endif
  sim_count = sizeof(hplmn_plmn_table)/sizeof(hplmn_plmn_table[0]);
  while(sim_count--)
  {
  	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_network_compare_simplmn_netplmn,SIMplmn_table[%d]=%d\n",\
															sim_count,hplmn_plmn_table[sim_count][0]);
  	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_network_compare_simplmn_netplmn,NETplmn_table[%d]=%d\n",\
															sim_count,hplmn_plmn_table[sim_count][1]);	
      if((hplmn_plmn_table[sim_count][0]==simplmn)&&(hplmn_plmn_table[sim_count][1]==netplmn))
      {
            return TRUE;
      }
  }
  return FALSE;
}

/*===========================================================================

FUNCTION jrd_nw_run_nw_name_rule

DESCRIPTION
  Fill the network name buffer accordding to the network name show rule.
  This rule comes from the data base.

DEPENDENCIES
  None.

RETURN VALUE
  0 -- success    -1 -- fail

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_network_split_plmn_name_code(char *src_str, char dest_str[][32], int *str_num)
{
      int index = 0;
      char *ph = src_str;
      char *p  = src_str;
    
      if(!src_str)
        return JRD_FAIL;
    
      while(*p )
      {
        if(index > 32)
          return JRD_FAIL;
    
        if(*(++p) == ',' || *p == '\0' )
        {
          JRD_MEMCPY(dest_str[index], ph,(p - ph));
          index++;
          ph = p+1;
          (*str_num)++;
    
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"cyan,jrd_usim_split_simlock_code dest_str[%d]=%s.\n",index-1,dest_str[index-1]);
        }
      }
      return JRD_NO_ERR;
    
 }
static char jrd_oem_show_roam_specify_net_plmn[256] = {0};
static char jrd_oem_show_roam_specify_sim_plmn[256] = {0};
static char jrd_oem_show_roam_specify_net_plmn_code[20][32] = {0};
static char jrd_oem_show_roam_specify_sim_plmn_code[20][32] = {0};



static char jrd_oem_specify_plmn[256] = {0};
static char jrd_oem_specify_plmn_name[256] = {0};
static char jrd_oem_specify_plmn_code[20][32] = {0};
static char jrd_oem_specify_plmn_name_code[20][32] = {0};
static int jrd_oem_specify_plmn_num = 0;
static int jrd__oem_specify_plmn_name_num = 0;
static char jrd_oem_sim_plmn[256] = {0};
static char jrd_oem_sim_plmn_code[20][32] = {0};
static int jrd_oem_sim_plmn_num = 0;
static uint32 jrd_oem_search_list_same_longnetworkname = 0;
static char jrd_oem_sim_plmn_name[256] = {0};
static char jrd_oem_sim_plmn_name_code[20][32] = {0};
static int jrd_oem_sim_plmn_name_num = 0;

static int jrd_oem_run_tmo_ons_flag = 0;
e_jrd_oem_full_service_t jrd_oem_full_service_requested = 0;

int jrd_show_roam_specify_handle(int sim_plmn,int net_plmn)
{    
    int sim_plmn_num = 0;
    int net_plmn_num = 0;
    int rc = -1;
    int i = 0;
    char sim_plmn_str[32] = {0};
    char net_plmn_str[32] = {0};
    char sim_plmn_all[32]= "***";

/*  把参数中的 sim_plmn  输出到sim_plmn_str  */
/*  把参数中的 net_plmn  输出到net_plmn_str  */
    JRD_SPRINTF(sim_plmn_str, "%d",sim_plmn);
    JRD_SPRINTF(net_plmn_str, "%d",net_plmn);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "current net_plmn_str %s \n",net_plmn_str);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "current sim_plmn_str %s \n",sim_plmn_str);
    jrd_network_split_plmn_name_code(jrd_oem_show_roam_specify_net_plmn,jrd_oem_show_roam_specify_net_plmn_code,&net_plmn_num);
    jrd_network_split_plmn_name_code(jrd_oem_show_roam_specify_sim_plmn,jrd_oem_show_roam_specify_sim_plmn_code,&sim_plmn_num);
	if(net_plmn_num > 0 && sim_plmn_num > 0)
	{
		for(i = 0;i < net_plmn_num; i ++)
		{
		    if(0 == JRD_STRCMP(net_plmn_str, jrd_oem_show_roam_specify_net_plmn_code[i]))
		    {
		      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "net_plmn_str %s \n",net_plmn_str);
		      if(0 == JRD_STRCMP(sim_plmn_str, jrd_oem_show_roam_specify_sim_plmn_code[i])|| 0 == JRD_STRCMP(sim_plmn_all, jrd_oem_show_roam_specify_sim_plmn_code[i]))
		      {
		         JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "sim_plmn_str %s \n",sim_plmn_str);
		         rc = 0;
		      }

		    }
		}
	}
	return rc;
}
int jrd_nw_custom_nw_name(char *spn_name,int plmn,char *nw_name)
{
    jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
    uint16 copy_len = 0;
    uint16 long_name_len = 0;
    uint16 short_name_len = 0;
    uint16 spn_name_len = 0;
    uint16 pnn_name_len = 0;
    uint32 current_plmn = 0;
    char current_plmn_str[32] = {0};
    char sim_plmn_str[32] = {0};
    int i = 0;
    //LZY
    uint16 nitz_name_len = 0;
    uint16 show_pnn_switch = p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_show_pnn_name;
    short_name_len = JRD_STRLEN(p_jrd_nw_local_cache->sys_info.short_name);
    long_name_len = JRD_STRLEN(p_jrd_nw_local_cache->sys_info.long_name);
    spn_name_len = JRD_STRLEN(spn_name);
    pnn_name_len = JRD_STRLEN(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn);
 
      JRD_SPRINTF(current_plmn_str, "%d",plmn);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "subin test current_plmn %s \n",current_plmn_str);
    jrd_oem_specify_plmn_num = 0;
    jrd__oem_specify_plmn_name_num = 0;
      jrd_network_split_plmn_name_code(jrd_oem_specify_plmn,jrd_oem_specify_plmn_code,&jrd_oem_specify_plmn_num);
      jrd_network_split_plmn_name_code(jrd_oem_specify_plmn_name,jrd_oem_specify_plmn_name_code,&jrd__oem_specify_plmn_name_num);
    
    
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_specify_plmn_code %s \n",jrd_oem_specify_plmn_code[0]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_specify_plmn_code %s \n",jrd_oem_specify_plmn_code[1]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_specify_plmn_code %s \n",jrd_oem_specify_plmn_code[2]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_specify_plmn_code  %s \n",jrd_oem_specify_plmn_code[3]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_oem_specify_plmn_num  %d \n",jrd_oem_specify_plmn_num);
      JRD_SPRINTF(sim_plmn_str, "%d",p_jrd_nw_local_cache->nw_usim_ind_cache.usim_plmn);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "sim_plmn_str %s \n",sim_plmn_str);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "show_pnn_switch=%d pnn_name_len=%d pnn_name=%s\n",show_pnn_switch,pnn_name_len,p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn);
      if (show_pnn_switch > 0 && pnn_name_len > 0)
     //if (show_pnn_switch > 0 )
      {		
		if(0 == JRD_STRCMP(current_plmn_str, sim_plmn_str))
		{
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "plmn %s use nw_name %s \n",sim_plmn_str,p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn);
            JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
            
            JRD_STRNCPY(nw_name,p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn, pnn_name_len);
            return 0;
		}
      }
      if(spn_name_len <= 0 )   
      {
          if(jrd__oem_specify_plmn_name_num > 0)
          {
              for(i = 0;i < jrd_oem_specify_plmn_num; i ++)
              {
                    if(0 == JRD_STRCMP(current_plmn_str, jrd_oem_specify_plmn_code[i]))
                    {
                      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_specify_plmn_code i %d \n",i);
                      JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
                      JRD_STRNCPY(nw_name,jrd_oem_specify_plmn_name_code[i], JRD_STRLEN(jrd_oem_specify_plmn_name_code[i]));
                      break;
                    }
              }
          }
      }
      else 
      {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " test current_plmn with spn %s \n",current_plmn_str);
          
          if(jrd__oem_specify_plmn_name_num > 0)
          {
                  if( spn_name_len > 0 
                  && p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.force_to_show_spn_name)
                  {
                    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "force to use spn as network name (spn is %s)\n",spn_name);
                    JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
                    JRD_STRNCPY(nw_name,spn_name, JRD_STRLEN(spn_name));
                  }
		     else
     		     {
                   for(i = 0;i < jrd_oem_specify_plmn_num; i ++)
                   {
                         if(0 == JRD_STRCMP(current_plmn_str, jrd_oem_specify_plmn_code[i]))
                         {
                           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_specify_plmn_code i %d %s\n",i,jrd_oem_specify_plmn_name_code[i]);
                           JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
                           JRD_STRNCPY(nw_name,jrd_oem_specify_plmn_name_code[i], JRD_STRLEN(jrd_oem_specify_plmn_name_code[i]));
                           break;
                         }
                   }
     	           }
          }
          else if( ( E_SHOW_SPN_IF_EXIST == p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_rule)
             &&(0 == JRD_STRCMP(sim_plmn_str, current_plmn_str)))
          {
                    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "force to use spn as network name (spn is %s)\n",spn_name);
                    JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
                    JRD_STRNCPY(nw_name,spn_name, JRD_STRLEN(spn_name));
          }
      
      }
      
      jrd_oem_sim_plmn_num = 0;
      jrd_oem_sim_plmn_name_num = 0;
      jrd_network_split_plmn_name_code(jrd_oem_sim_plmn,jrd_oem_sim_plmn_code,&jrd_oem_sim_plmn_num);
      jrd_network_split_plmn_name_code(jrd_oem_sim_plmn_name,jrd_oem_sim_plmn_name_code,&jrd_oem_sim_plmn_name_num);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_name_code%s \n",jrd_oem_sim_plmn_name_code[0]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_name_code%s \n",jrd_oem_sim_plmn_name_code[1]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_name_code%s \n",jrd_oem_sim_plmn_name_code[2]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_name_code%s \n",jrd_oem_sim_plmn_name_code[3]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_code %s \n",jrd_oem_sim_plmn_code[0]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_code %s \n",jrd_oem_sim_plmn_code[1]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_code%s \n",jrd_oem_sim_plmn_code[2]);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_code %s \n",jrd_oem_sim_plmn_code[3]); 
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_specify_plmn_num  %d \n",jrd_oem_sim_plmn_num);
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_specify_plmn_num  %d \n",jrd_oem_sim_plmn_name_num);
    
      if(jrd_oem_sim_plmn_name_num > 0)
      {
          for(i = 0;i < jrd_oem_sim_plmn_name_num; i ++)
          { 
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "current_plmn_str  %s sim_plmn_str%s i=%d jrd_oem_specify_plmn_code[i]%sjrd_oem_sim_plmn_code[i]%s\n",current_plmn_str,sim_plmn_str,i,jrd_oem_specify_plmn_code[i],jrd_oem_sim_plmn_code[i]);
                if((0 == JRD_STRCMP(current_plmn_str, jrd_oem_specify_plmn_code[i]))
                  &&(0 == JRD_STRCMP(sim_plmn_str, jrd_oem_sim_plmn_code[i]))
                 )
                {
                  if( spn_name_len > 0 
                  && p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.force_to_show_spn_name)
                  {
                    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "force to use spn as network name (spn is %s)\n",spn_name);
                    JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
                    JRD_STRNCPY(nw_name,spn_name, JRD_STRLEN(spn_name));
                  }
                  else
                  {
                    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_sim_plmn_code i %d \n",i);
                    JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
                    JRD_STRNCPY(nw_name,jrd_oem_sim_plmn_name_code[i], JRD_STRLEN(jrd_oem_sim_plmn_name_code[i]));
                  }

                  break;
                }
          }
      }
      else  if( spn_name_len > 0 
      && ( E_SHOW_SPN_IF_EXIST == p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_rule)
      &&(0 == JRD_STRCMP(sim_plmn_str, current_plmn_str)))
      {
                    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "force to use spn as network name (spn is %s)\n",spn_name);
                    JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
                    JRD_STRNCPY(nw_name,spn_name, JRD_STRLEN(spn_name));
      }
return 0;
}

int jrd_nw_run_tmo_nos_rule(void)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  uint32 plmn = 0;
  uint16 lac;

  if (p_jrd_nw_local_cache->sys_info.mnc > 99)
  {
     plmn =p_jrd_nw_local_cache->sys_info.mnc + p_jrd_nw_local_cache->sys_info.mcc*1000;
  }
  else
  {
     plmn =p_jrd_nw_local_cache->sys_info.mnc + p_jrd_nw_local_cache->sys_info.mcc*100;
  }

  lac = p_jrd_nw_local_cache->sys_info.lac;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "register mcc: %d\n",p_jrd_nw_local_cache->sys_info.mcc);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "register mnc: %d\n",p_jrd_nw_local_cache->sys_info.mnc);

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "register PLMN: %d, Lac: %d\n",plmn, lac);

  if(jrd_oem_run_tmo_ons_flag && p_jrd_nw_local_cache->sys_info.regist_status == E_NW_REGISTED)
  {
     if(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_GPRS || p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EDGE || p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_GSM)
     {
         //run 2G roaming nos
         if(plmn == 302370 ||plmn == 310150 || plmn == 310410 || plmn == 23430 || plmn == 33420)
         {
            p_jrd_nw_local_cache->sys_info.roam = E_NW_ROAM_OFF;
         }
         //run 2G nos rlues
         if(plmn == 23430)
         {           
            JRD_STRNCPY(p_jrd_nw_local_cache->sys_info.nw_name, \
                                   "T-Mobile UK", \
                                   JRD_STRLEN("T-Mobile UK"));

         }
         if(plmn == 33420)
         {
            JRD_STRNCPY(p_jrd_nw_local_cache->sys_info.nw_name, \
                                   "Mexican", \
                                   JRD_STRLEN("Mexican"));
         }
         //run NOS/ENOS rule
         if(plmn == 310170 || plmn == 31017)
         {
            if(lac == 400) {}
            else if(p_jrd_nw_local_cache->operator_name_info.eons_sim_flag == TRUE)
            {
               JRD_STRNCPY(p_jrd_nw_local_cache->sys_info.nw_name, \
                                      "T Mobile EONS", \
                                      JRD_STRLEN("T Mobile EONS"));
            }
            else if(p_jrd_nw_local_cache->operator_name_info.eons_sim_flag == FALSE)
            {
               JRD_STRNCPY(p_jrd_nw_local_cache->sys_info.nw_name, \
                                      "T Mobile", \
                                      JRD_STRLEN("T Mobile"));
            }
         }
         JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "tmo nos roaming indicator, roaming: %d\n",p_jrd_nw_local_cache->sys_info.roam);
         JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "tmo nos network name indicator, name: %s\n",p_jrd_nw_local_cache->sys_info.nw_name);
     }//run 2G nos feature.
  }

  return 0;
}


int jrd_nw_run_nw_name_rule(char* nw_name)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  uint16 copy_len = 0;
  uint16 long_name_len = 0;
  uint16 short_name_len = 0;
  uint16 spn_name_len = 0;
  uint32 current_plmn = 0;
  char current_plmn_str[32] = {0};
  char sim_plmn_str[32] = {0};
  int i = 0;
  //LZY
  uint16 nitz_name_len = 0;
  uint32 plmn = p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_plmn_name.plmn;
  short_name_len = JRD_STRLEN(p_jrd_nw_local_cache->sys_info.short_name);
  long_name_len = JRD_STRLEN(p_jrd_nw_local_cache->sys_info.long_name);
  spn_name_len = JRD_STRLEN(p_jrd_nw_local_cache->sys_info.spn_name);


   /*for NITZ name, if it has long name, display long name, else if has short name, display short name*/
   if(p_jrd_nw_local_cache->operator_name_info.long_nitz_len > 0)
   {
      nitz_name_len = JRD_STRLEN(p_jrd_nw_local_cache->operator_name_info.long_nitz_name);
   }
   else if(p_jrd_nw_local_cache->operator_name_info.short_nitz_len > 0)
   {
      nitz_name_len = JRD_STRLEN(p_jrd_nw_local_cache->operator_name_info.short_nitz_name);
   }

 
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "############short_name_len:%d,long_name_len:%d,spn_name_len:%d,nitz_name_len:%d p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_rule=%d!!!\n",
  										short_name_len,long_name_len,spn_name_len,nitz_name_len,p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_rule);   
  if(NULL == nw_name)
  {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid nw name buffer\n");  
     return -1;
  }
  if (p_jrd_nw_local_cache->sys_info.mnc > 99)
   {
       current_plmn =p_jrd_nw_local_cache->sys_info.mnc + p_jrd_nw_local_cache->sys_info.mcc*1000;
   }
   else
   {
       current_plmn =p_jrd_nw_local_cache->sys_info.mnc + p_jrd_nw_local_cache->sys_info.mcc*100;
   }

  if(p_jrd_nw_local_cache->sys_info.regist_status != E_NW_REGISTED)
  {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Not registed\n");
     JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
     JRD_STRNCPY(nw_name, \
                            "N/A", \
                            JRD_STRLEN("N/A"));
     return 0;
  }

  if(p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_roaming_spn \
     && p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_ON)
  {
     if(spn_name_len != 0)
     {
       copy_len = spn_name_len;
	JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
       JRD_STRNCPY(nw_name, \
                              p_jrd_nw_local_cache->sys_info.spn_name, \
                              MIN(copy_len, MAX_NW_NAME_LEN));
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Set network name to SPN while roaming\n");  
       return 0;
     }
  }

  if(short_name_len == 0 && long_name_len ==0)
  {
     if(spn_name_len != 0)
     {
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "No long name or short name, set nw name to spn\n");  
       copy_len = spn_name_len;
        JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
       JRD_STRNCPY(nw_name, \
                              p_jrd_nw_local_cache->sys_info.spn_name, \
                              MIN(copy_len, MAX_NW_NAME_LEN));
       return 0;
     }
     else
     {
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "No any network name, set to plmn\n");  
       if(p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.unknown_plmn_rule == E_SHOW_PLMN_ONLY)
       {
         JRD_SPRINTF(nw_name, "%3d%2d", p_jrd_nw_local_cache->sys_info.mcc, p_jrd_nw_local_cache->sys_info.mnc);
         return 0;
       }
       else
       {
         JRD_SPRINTF(nw_name, "%3d%2d_unknown", p_jrd_nw_local_cache->sys_info.mcc, p_jrd_nw_local_cache->sys_info.mnc);
         return 0;
       }
     }
  }
  else if(short_name_len == 0 && long_name_len !=0)
  {
       copy_len = long_name_len;
       JRD_STRNCPY(p_jrd_nw_local_cache->sys_info.short_name, \
                              p_jrd_nw_local_cache->sys_info.long_name, \
                              MIN(copy_len, MAX_NW_NAME_LEN));
  }
  copy_len = short_name_len;
  JRD_STRNCPY(nw_name, \
                         p_jrd_nw_local_cache->sys_info.short_name, \
                         MIN(copy_len, MAX_NW_NAME_LEN));

  switch(p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_rule)
  {
     case E_SHOW_SHORT_NAME_ONLY:
       JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
       copy_len = short_name_len;
       JRD_STRNCPY(nw_name, \
                              p_jrd_nw_local_cache->sys_info.short_name, \
                              MIN(copy_len, MAX_NW_NAME_LEN));
       jrd_nw_custom_nw_name("",current_plmn,nw_name);                       
     break;
     case E_SHOW_SHORT_NAME_SPN:
       copy_len = short_name_len + spn_name_len;
       JRD_SNPRINTF(nw_name, \
                               MIN(copy_len, MAX_NW_NAME_LEN), \
                               "%s%s", \
                               p_jrd_nw_local_cache->sys_info.short_name, \
                               p_jrd_nw_local_cache->sys_info.spn_name); 
     break;
     case E_SHOW_SHORT_NAME_SP_SPN:
       copy_len = short_name_len + spn_name_len + 1;
       if(spn_name_len != 0)
       {
         JRD_SNPRINTF(nw_name, \
                                 MIN(copy_len, MAX_NW_NAME_LEN), \
                                 "%s|%s", \
                                 p_jrd_nw_local_cache->sys_info.short_name, \
                                 p_jrd_nw_local_cache->sys_info.spn_name); 
       }
       else
       {
         copy_len = short_name_len;
         JRD_STRNCPY(nw_name, \
                                p_jrd_nw_local_cache->sys_info.short_name, \
                                MIN(copy_len, MAX_NW_NAME_LEN));
       }
     break;
     case E_SHOW_LONG_NAME_ONLY:
       copy_len = long_name_len;
       JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
       JRD_STRNCPY(nw_name, \
                              p_jrd_nw_local_cache->sys_info.long_name, \
                              MIN(copy_len, MAX_NW_NAME_LEN));
     break;
     case E_SHOW_LONG_NAME_SPN:
       copy_len = long_name_len + spn_name_len;
       JRD_SNPRINTF(nw_name, \
                               MIN(copy_len, MAX_NW_NAME_LEN), \
                               "%s%s", \
                               p_jrd_nw_local_cache->sys_info.long_name, \
                               p_jrd_nw_local_cache->sys_info.spn_name); 
     break;
     case E_SHOW_LONG_NAME_SP_SPN:
       if(spn_name_len != 0)
       {
         copy_len = long_name_len + spn_name_len + 1;
         JRD_SNPRINTF(nw_name, \
                                 MIN(copy_len, MAX_NW_NAME_LEN), \
                                 "%s|%s", \
                                 p_jrd_nw_local_cache->sys_info.long_name, \
                                 p_jrd_nw_local_cache->sys_info.spn_name); 
       }
       else
       {
         copy_len = long_name_len;
         JRD_STRNCPY(nw_name, \
                                p_jrd_nw_local_cache->sys_info.long_name, \
                                MIN(copy_len, MAX_NW_NAME_LEN));
       }
     break;
	case E_SHOW_SPN_RU:
	if(spn_name_len != 0)
	{
		if (0 != JRD_STRCMP(nw_name,p_jrd_nw_local_cache->sys_info.spn_name))
		{
		   copy_len = short_name_len + spn_name_len + 1;
		   JRD_SNPRINTF(nw_name, \
		                                 MIN(copy_len, MAX_NW_NAME_LEN), \
		                                 "%s|%s", \
		                                 p_jrd_nw_local_cache->sys_info.short_name, \
		                                 p_jrd_nw_local_cache->sys_info.spn_name);
		}
	 }
	 break;
	case E_SHOW_NITZ_NAME_SPN:
	JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	if((p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_LTE) \
	   ||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_LTE_CA))
	 { 
	       copy_len = short_name_len;
	       JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.short_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
		 JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "4G network,display short name!!!\n");  
		 return 0;
	 }
 
       if(spn_name_len != 0)
       {
	  if(((p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_GPRS) \
	  	||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EDGE) \
	  	||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_CDMA) \
	  	||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EVDO_RO) \
	  	||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EVDO_RA) \
	  	||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_GSM) \
	  	||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EVDO_RB)) \
	  	&&(p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_OFF))
	  {
	  	copy_len = spn_name_len;
	  	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "#################2G and HOME NETWORK,Display SPN only:%s,LEN:%d!!!\n", \
												p_jrd_nw_local_cache->sys_info.spn_name,spn_name_len);  
		JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.spn_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "E_SHOW_NITZ_NAME_SPN,nw_name = %s\n",nw_name);  
	  }
	  else if(nitz_name_len != 0)
	  {
	      copy_len = nitz_name_len + spn_name_len + 2;

            if(p_jrd_nw_local_cache->operator_name_info.long_nitz_len > 0)
            {
               JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "#################NO 2G  NETWORK,Display NITZ/VPLMN-SPN:%s-%s!!!\n", \
                                                                   p_jrd_nw_local_cache->operator_name_info.long_nitz_name, \
                                                                                    p_jrd_nw_local_cache->sys_info.spn_name); 
                   JRD_SNPRINTF(nw_name, \
                                       MIN(copy_len, MAX_NW_NAME_LEN), \
                                       "%s-%s", \
                                       p_jrd_nw_local_cache->operator_name_info.long_nitz_name, \
                                       p_jrd_nw_local_cache->sys_info.spn_name); 
            }
            else if(p_jrd_nw_local_cache->operator_name_info.short_nitz_len > 0)
            {
               JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "#################NO 2G  NETWORK,Display NITZ/VPLMN-SPN:%s-%s!!!\n", \
                                                                   p_jrd_nw_local_cache->operator_name_info.short_nitz_name, \
                                                                                    p_jrd_nw_local_cache->sys_info.spn_name); 
                   JRD_SNPRINTF(nw_name, \
                                       MIN(copy_len, MAX_NW_NAME_LEN), \
                                       "%s-%s", \
                                       p_jrd_nw_local_cache->operator_name_info.short_nitz_name, \
                                       p_jrd_nw_local_cache->sys_info.spn_name); 
            }

	  }
	  else
	  {
	      copy_len = spn_name_len + spn_name_len + 2;
	      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "#################NO NITZ and NO 2G NETWORK,Display SPN-SPN:%s-%s!!!\n", \
		  																       p_jrd_nw_local_cache->sys_info.spn_name, \
                                 															p_jrd_nw_local_cache->sys_info.spn_name); 
             JRD_SNPRINTF(nw_name, \
                                 MIN(copy_len, MAX_NW_NAME_LEN), \
                                 "%s-%s", \
                                 p_jrd_nw_local_cache->sys_info.spn_name, \
                                 p_jrd_nw_local_cache->sys_info.spn_name); 
	  }
       }
       else
       {
         copy_len = short_name_len;
         JRD_STRNCPY(nw_name, \
                                p_jrd_nw_local_cache->sys_info.short_name, \
                                MIN(copy_len, MAX_NW_NAME_LEN));
       }
     break;
     case E_SHOW_NITZ_NAME_SPN_OTHER:
	JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
       if(spn_name_len != 0)
       {
	  if(p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_OFF)
	  {
	  	copy_len = spn_name_len;
	  	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "#################HOME NETWORK,Display SPN only:%s,LEN:%d!!!\n", \
												p_jrd_nw_local_cache->sys_info.spn_name,spn_name_len);  
		JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.spn_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "E_SHOW_NITZ_NAME_SPN_OTHER,nw_name = %s\n",nw_name);  
	  }
	  else if((nitz_name_len != 0)&&(p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_ON))
	  {
	      copy_len = nitz_name_len + spn_name_len + 6;
            if(p_jrd_nw_local_cache->operator_name_info.long_nitz_len > 0)
            {
               JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "#################NO 2G  NETWORK,Display NITZ/VPLMN-SPN:%s-%s!!!\n", \
                                                                   p_jrd_nw_local_cache->operator_name_info.long_nitz_name, \
                                                                                    p_jrd_nw_local_cache->sys_info.spn_name); 
                   JRD_SNPRINTF(nw_name, \
                                       MIN(copy_len, MAX_NW_NAME_LEN), \
                                       "%s %s (R)", \
                                       p_jrd_nw_local_cache->operator_name_info.long_nitz_name, \
                                       p_jrd_nw_local_cache->sys_info.spn_name); 
            }
            else if(p_jrd_nw_local_cache->operator_name_info.short_nitz_len > 0)
            {
               JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "#################NO 2G  NETWORK,Display NITZ/VPLMN-SPN:%s-%s!!!\n", \
                                                                   p_jrd_nw_local_cache->operator_name_info.short_nitz_name, \
                                                                                    p_jrd_nw_local_cache->sys_info.spn_name); 
                   JRD_SNPRINTF(nw_name, \
                                       MIN(copy_len, MAX_NW_NAME_LEN), \
                                       "%s-%s", \
                                       p_jrd_nw_local_cache->operator_name_info.short_nitz_name, \
                                       p_jrd_nw_local_cache->sys_info.spn_name); 
            }
	  }
	  else
	  {
	      copy_len = spn_name_len + spn_name_len + 6;
	      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "#################ROAM NETWORK,Display SPN SPN:%s %s (R),LEN:%d!!!\n", \
		  																       p_jrd_nw_local_cache->sys_info.spn_name, \
                                 															p_jrd_nw_local_cache->sys_info.spn_name,copy_len); 
             JRD_SNPRINTF(nw_name, \
                                 MIN(copy_len, MAX_NW_NAME_LEN), \
                                 "%s %s (R)", \
                                 p_jrd_nw_local_cache->sys_info.spn_name, \
                                 p_jrd_nw_local_cache->sys_info.spn_name);
	  }
       }
       else
       {
         copy_len = short_name_len;
         JRD_STRNCPY(nw_name, \
                                p_jrd_nw_local_cache->sys_info.short_name, \
                                MIN(copy_len, MAX_NW_NAME_LEN));
       }
     break;
    case E_SHOW_AIRTEL_X_NAME:
	{
		uint32 cur_nw_plmn = 0;
		cur_nw_plmn = (uint32)p_jrd_nw_local_cache->sys_info.mcc*1000+(uint32)p_jrd_nw_local_cache->sys_info.mnc;
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Current network plmn :%d!\n",cur_nw_plmn);
		switch(jrd_oem_airtel_name_feature(cur_nw_plmn))
		{
		     case 1:
		     JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
		     JRD_STRNCPY(nw_name, "airtel 1", JRD_STRLEN("airtel 1"));
		     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ICR mode name :%s!\n",nw_name);
		     break;
		     case 2:
		     JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
		     JRD_STRNCPY(nw_name, "airtel 2", JRD_STRLEN("airtel 2"));
		     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ICR mode name :%s!\n",nw_name);			 
		     break;
		     case 3:
		     JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
		     JRD_STRNCPY(nw_name, "airtel 3", JRD_STRLEN("airtel 3"));
		     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ICR mode name :%s!\n",nw_name);
		     break;
		     default:
	            copy_len = short_name_len;
		     JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);	   
	            JRD_STRNCPY(nw_name, p_jrd_nw_local_cache->sys_info.short_name, MIN(copy_len, MAX_NW_NAME_LEN));
		     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Not in  ICR circle :%s!\n",nw_name);
		     break;
		}
    	}
	break;
    case E_SHOW_IK40_DT_GROUP:
	{
	 uint32 	simcard_plmn = 0;
	 simcard_plmn = (uint32)p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mcc*1000+(uint32)p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mnc;
       JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
       if((705041 == simcard_plmn)&&(spn_name_len != 0))
       { 
	       copy_len = spn_name_len;
	       JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.spn_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "DT Simcard(70541) show SPN name: %s !\n",nw_name);  
       }
       else
       {
	       copy_len = short_name_len;
	       JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.short_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "DT network name: %s !\n",nw_name);  
      }
      }
     break;
     case E_SHOW_SPN_OR_SHORTNAME:
	   
       if(spn_name_len != 0)
       {
	       if(p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_OFF)
	       {
	           JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	           copy_len = spn_name_len;
	           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "#################HOME NETWORK,Display SPN only:%s,LEN:%d!!!\n", \
	  	       									p_jrd_nw_local_cache->sys_info.spn_name,spn_name_len);  
	           JRD_STRNCPY(nw_name, \
	                                      p_jrd_nw_local_cache->sys_info.spn_name, \
	                                       MIN(copy_len, MAX_NW_NAME_LEN));
	           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "nw_name=SPN = %s\n",nw_name);  
	       }
	       else
	       {
	            JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	            copy_len = short_name_len;
	            JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.short_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
	           jrd_nw_custom_nw_name("",current_plmn,nw_name);                  
		   }

       }
       else
       {

	      JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	      copy_len = short_name_len;
	      JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.short_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
         jrd_nw_custom_nw_name("",current_plmn,nw_name);
       }

     break;

    case E_SHOW_SPN_IF_EXIST:
      JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
      
      if(spn_name_len != 0)
      {
        copy_len = spn_name_len;
        JRD_STRNCPY(nw_name, \
                    p_jrd_nw_local_cache->sys_info.spn_name, \
                    MIN(copy_len, MAX_NW_NAME_LEN));

        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "nw_name=SPN = %s\n",nw_name);  
      }
      else
      {
        copy_len = short_name_len;
        JRD_STRNCPY(nw_name, \
                    p_jrd_nw_local_cache->sys_info.short_name, \
                    MIN(copy_len, MAX_NW_NAME_LEN));
      }
	  break;
	case E_SHOW_LONG_NAME_COND:
	{
	 uint8 	display_cond = 0;
	 
	 if(0 == spn_name_len)
	 {
		copy_len = long_name_len;
	    JRD_STRNCPY(nw_name, \
	                      p_jrd_nw_local_cache->sys_info.long_name, \
	                      MIN(copy_len, MAX_NW_NAME_LEN));

		break;
	 }
	 display_cond = p_jrd_nw_local_cache->operator_name_info.display_cond;
	 switch (display_cond & JRD_OEM_DIAPLAY_COND_MARK)
	 {
		case 0:
		 JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
		 if(p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_ON)
		 {
			 copy_len = spn_name_len + long_name_len + 2;
             JRD_SNPRINTF(nw_name, \
                                   MIN(copy_len, MAX_NW_NAME_LEN), \
                                   "%s-%s", \
                                   p_jrd_nw_local_cache->sys_info.long_name, \
                                   p_jrd_nw_local_cache->sys_info.spn_name); 			
		 }
		 else 
		 {
			 copy_len = spn_name_len;
	         JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.spn_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN)); 	
		 }
		break;
		case 1:
		 JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
		 copy_len = spn_name_len + long_name_len + 2;
		 JRD_SNPRINTF(nw_name, \
                                   MIN(copy_len, MAX_NW_NAME_LEN), \
								   "%s-%s", \
								   p_jrd_nw_local_cache->sys_info.long_name, \
								   p_jrd_nw_local_cache->sys_info.spn_name); 
		break;
		case 2:
		 JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
		 if(p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_ON)
		 {
			 copy_len =  long_name_len;
			 JRD_STRNCPY(nw_name, \
									p_jrd_nw_local_cache->sys_info.long_name, \
									MIN(copy_len, MAX_NW_NAME_LEN));

		 }
		 else 
		 {
			 copy_len = spn_name_len;
	       JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.spn_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN)); 	
		 }
		break;
		case 3:
		 JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
		 if(p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_ON)
		 {
			 copy_len =  long_name_len;
			 JRD_STRNCPY(nw_name, \
									p_jrd_nw_local_cache->sys_info.long_name, \
									MIN(copy_len, MAX_NW_NAME_LEN));


		 }
		 else 
		 {
		     copy_len = spn_name_len + long_name_len + 2;
			 JRD_SNPRINTF(nw_name, \
                                   MIN(copy_len, MAX_NW_NAME_LEN), \
                                   "%s-%s", \
                                   p_jrd_nw_local_cache->sys_info.long_name, \
                                   p_jrd_nw_local_cache->sys_info.spn_name);
		 }				
		 break;
	 }
	   JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "E_SHOW_LONG_NAME_COND name: %s !\n",nw_name);
	 }
	 break;
     case E_SHOW_SPN_OR_LONG_NAME:
	   
       if(spn_name_len != 0)
       {

	           JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	           copy_len = spn_name_len;
	           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "#################HOME NETWORK,Display SPN only:%s,LEN:%d!!!\n", \
	  	       									p_jrd_nw_local_cache->sys_info.spn_name,spn_name_len);  
	           JRD_STRNCPY(nw_name, \
	                                      p_jrd_nw_local_cache->sys_info.spn_name, \
	                                       MIN(copy_len, MAX_NW_NAME_LEN));
	           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "nw_name=SPN = %s\n",nw_name);  

       }
       else
       {

	      JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	      copy_len = long_name_len;
	      JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.long_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
             jrd_nw_custom_nw_name("",current_plmn,nw_name);
       }

     break;
     case E_SHOW_SPN_ROMING_OR_LONG_NAME:
       if(spn_name_len != 0)
       {
	       if(p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_OFF)
	       {
	           JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	           copy_len = spn_name_len;
	           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "#################HOME NETWORK,Display SPN only:%s,LEN:%d!!!\n", \
	  	       									p_jrd_nw_local_cache->sys_info.spn_name,spn_name_len);  
	           JRD_STRNCPY(nw_name, \
	                                      p_jrd_nw_local_cache->sys_info.spn_name, \
	                                       MIN(copy_len, MAX_NW_NAME_LEN));
	           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "nw_name=SPN = %s\n",nw_name);  
	       }
	       else
	       {
	            JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	            copy_len = long_name_len;
	            JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.long_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
	           jrd_nw_custom_nw_name("",current_plmn,nw_name);                  
	       }

       }
       else
       {

	      JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	      copy_len = long_name_len;
	      JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.long_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
            jrd_nw_custom_nw_name("",current_plmn,nw_name);
       }
     break;
     case E_SHOW_SPN_ROMING_OR_COMBINE_NAME:
       if(spn_name_len != 0 &&p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_ON)
       {

           JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
           copy_len = MAX_NW_NAME_LEN;
           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "#################roam NETWORK,Display short_name|SPN only:%s|%s,LEN:%d!!!\n", \
  	       									p_jrd_nw_local_cache->sys_info.short_name,p_jrd_nw_local_cache->sys_info.spn_name,spn_name_len);  
           JRD_SNPRINTF(nw_name, MIN(copy_len, MAX_NW_NAME_LEN),\
                                  "%s|%s",\
                                  p_jrd_nw_local_cache->sys_info.short_name,\
                                      p_jrd_nw_local_cache->sys_info.spn_name \
                                       );
           //JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "nw_name=SPN = %s\n",nw_name);  


       }
       else
       {

	      JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	      copy_len = short_name_len;
	      JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.short_name, \
	                              MIN(copy_len, MAX_NW_NAME_LEN));
       }
     break;
         case E_SHOW_SPN_ROMING_OR_COMBINE_NAME2:
       if(spn_name_len != 0 &&p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_ON)
       {

           JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
           copy_len = MAX_NW_NAME_LEN;
           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "#################roam NETWORK,Display short_name|SPN only:%s|%s,LEN:%d!!!\n", \
  	       									p_jrd_nw_local_cache->sys_info.short_name,p_jrd_nw_local_cache->sys_info.spn_name,spn_name_len);  
           JRD_SNPRINTF(nw_name, MIN(copy_len, MAX_NW_NAME_LEN),\
                                  "%s-%s",\
                                  p_jrd_nw_local_cache->sys_info.long_name,\
                                      p_jrd_nw_local_cache->sys_info.spn_name \
                                       );
           //JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "nw_name=SPN = %s\n",nw_name);  


       }
       else if (spn_name_len != 0 &&p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_OFF)
       {

	      JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	      copy_len = MAX_NW_NAME_LEN;
	      JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.spn_name, \
	                              MAX_NW_NAME_LEN);
       }
       else
       {

	      JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
	      copy_len = MAX_NW_NAME_LEN;
	      JRD_STRNCPY(nw_name, \
	                              p_jrd_nw_local_cache->sys_info.long_name, \
	                              MAX_NW_NAME_LEN);
       }
     break;
     case E_SHOW_SPN_OR_SHORT_NAME_ALDK:         /*added by yuegui.he for aldk*/
        JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
        if (spn_name_len == 0)
        {
            copy_len = short_name_len;
            JRD_STRNCPY(nw_name, \
                                p_jrd_nw_local_cache->sys_info.short_name, \
                                MIN(copy_len, MAX_NW_NAME_LEN));
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"yuegui.he spn_name_len is 0, nw_name is %s\n", p_jrd_nw_local_cache->sys_info.short_name);
        } else {
            if (p_jrd_nw_local_cache->sys_info.roam == E_NW_ROAM_ON)
            {
                copy_len = short_name_len + spn_name_len + 1 + 1;
                JRD_SNPRINTF(nw_name, \
                                      MIN(copy_len, MAX_NW_NAME_LEN), \
                                      "%s|%s", \
                                      p_jrd_nw_local_cache->sys_info.short_name, \
                                      p_jrd_nw_local_cache->sys_info.spn_name); 
                 JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"yuegui.he spn_name_len is 1, nw_name is %s, spn is %s\n", p_jrd_nw_local_cache->sys_info.short_name, p_jrd_nw_local_cache->sys_info.spn_name);

            }else {
                copy_len = spn_name_len; 
                JRD_STRNCPY(nw_name, \
                                      p_jrd_nw_local_cache->sys_info.spn_name, \
                                      MIN(copy_len, MAX_NW_NAME_LEN));   
                 JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"yuegui.he spn_name_len is 2, spn name is %s\n", p_jrd_nw_local_cache->sys_info.spn_name);

            }
        }
        break;
     default:
     JRD_MEMSET(nw_name, 0, MAX_NW_NAME_LEN);
     JRD_STRNCPY(nw_name, \
                            p_jrd_nw_local_cache->sys_info.long_name, \
                            MIN(copy_len, MAX_NW_NAME_LEN));
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Unknown network name rule:%d set nw name to long name\n", \
                                      p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule);  
     return -1;
  }
  

   jrd_nw_custom_nw_name(p_jrd_nw_local_cache->sys_info.spn_name,current_plmn,nw_name);
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_info_chg_ind

DESCRIPTION
  Send network information changed indication to registed modules.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int  jrd_nw_info_chg_ind(e_nw_ind_from_module_t ind)
{
  jrd_ind_data_type data;
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  jrd_oem_nw_ind_t nw_ind;
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Notify NW ind=%d\n", ind);  
  data.module_id = MODULE_NETWORK;
  if ((E_NW_IND_MOD_MAX == ind) || (E_JRD_NETWORK_MIN_IND == ind))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "INVALID ind:%d\n", ind);  
    return -1;
  }
  JRD_MEMSET(&nw_ind, 0, sizeof(nw_ind));
  data.ind_id = ind;

  nw_ind.regist_status = p_jrd_nw_local_cache->sys_info.regist_status;
  nw_ind.nw_type = p_jrd_nw_local_cache->sys_info.nw_type;
  nw_ind.ps_attach = p_jrd_nw_local_cache->sys_info.ps_attach;
  nw_ind.roam = p_jrd_nw_local_cache->sys_info.roam;
  nw_ind.mcc = p_jrd_nw_local_cache->sys_info.mcc;
  /*add new match profile by wei.huang.sz*/
  nw_ind.mnc = p_jrd_nw_local_cache->sys_info.mnc;
  /*add new match profile by wei.huang.sz*/
  if(p_jrd_nw_local_cache->sys_info.regist_status != E_NW_REGISTED)
  {
     p_jrd_nw_local_cache->rf_info.sig_level = 0;
  }
   else
   {
      if(E_NW_TYPE_LTE == p_jrd_nw_local_cache->sys_info.nw_type)
      {
        if(-1 > p_jrd_nw_local_cache->rf_info.rsrp)
          {
            p_jrd_nw_local_cache->rf_info.sig_level = jrd_nw_convert_nw_sig_level(p_jrd_nw_local_cache->rf_info.rsrp, TRUE);
          }
      }
      else
      {
        if(-1 > p_jrd_nw_local_cache->rf_info.rssi)
          {
            p_jrd_nw_local_cache->rf_info.sig_level = jrd_nw_convert_nw_sig_level(p_jrd_nw_local_cache->rf_info.rssi, FALSE);
          }
      }

   }

  nw_ind.sig_level = p_jrd_nw_local_cache->rf_info.sig_level;

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sig_level =%d\n", p_jrd_nw_local_cache->rf_info.sig_level);  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sig_level =%d\n", nw_ind.sig_level);  

  switch(ind)
  {
    case E_JRD_NETWORK_INFO_CHG_IND:
      jrd_get_cell_info(&p_jrd_nw_local_cache->cell_info);
    case E_JRD_NETWORK_SIGNAL_IND:
    {
      data.ind_data = (void *) &nw_ind;
      data.data_size = sizeof(jrd_oem_nw_ind_t);
    }
    break;
	case E_JRD_NETWORK_GET_MCC_IND:
	{
	  data.ind_data = (void *) &nw_ind;
      data.data_size = sizeof(jrd_oem_nw_ind_t);
	}
	break;
    default:
      data.ind_data = NULL;
      data.data_size = 0;
      return -1;;      
  }

  return jrd_oem_ind(&data);//0
}

/*===========================================================================

FUNCTION jrd_nw_handle_serving_sys_ind

DESCRIPTION
  Judge if serving system information has changed, if yes, send indication.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_serving_sys_ind(jrd_ind_data_type *ind)
{
  jrd_oem_sys_info_t* sys_info;
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  e_jrd_oem_nw_regist_status_t old_regist_state = p_jrd_nw_local_cache->sys_info.regist_status;
  uint8 old_sig_level = p_jrd_nw_local_cache->rf_info.sig_level;
  e_jrd_nw_roam_t old_roaming = p_jrd_nw_local_cache->sys_info.roam;
  e_jrd_oem_nw_type_t old_nw_type = p_jrd_nw_local_cache->sys_info.nw_type;
  e_nw_ps_attach_status_t old_ps_attach = p_jrd_nw_local_cache->sys_info.ps_attach;
  int regist_id = -1;

  if((NULL == ind) ||(sizeof(jrd_oem_sys_info_t) != ind->data_size) || (NULL == ind->ind_data))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind!!!\n");
    return -1;
  }
  
  sys_info = (jrd_oem_sys_info_t*)ind->ind_data;
  
  p_jrd_nw_local_cache->sys_info.ps_attach = sys_info->ps_attach;
  p_jrd_nw_local_cache->sys_info.regist_status = sys_info->regist_status;
  if((uint32)sys_info->cell_id != NW_INVALID_VALUE_U32)
  {
    p_jrd_nw_local_cache->sys_info.cell_id = sys_info->cell_id;
    p_jrd_nw_local_cache->sys_info.enbid = sys_info->enbid;
  }

  if(sys_info->lac != NW_INVALID_VALUE_U16)
  {
     if(p_jrd_nw_local_cache->sys_info.radio_if != E_NW_RADIO_IF_LTE)
     {
       p_jrd_nw_local_cache->sys_info.lac = sys_info->lac;
     }
     else
     {
       p_jrd_nw_local_cache->sys_info.lac = 0;
     }
  }
  if(sys_info->roam != NW_INVALID_VALUE)
  {
     p_jrd_nw_local_cache->sys_info.roam = sys_info->roam;
  }
  if(sys_info->nw_type!= NW_INVALID_VALUE_U16)
  {
     p_jrd_nw_local_cache->sys_info.nw_type = sys_info->nw_type;
  }
  if(sys_info->radio_if != NW_INVALID_VALUE_U16)
  {
     p_jrd_nw_local_cache->sys_info.radio_if = sys_info->radio_if;
  }
  jrd_nw_run_tmo_nos_rule();
  /*Not regist on any network, reset cache.*/
  p_jrd_nw_local_cache->sys_info.roam =jrd_get_check_roam_state(p_jrd_nw_local_cache->sys_info.roam,\
                                       p_jrd_nw_local_cache->sys_info.nw_type,\
                                       p_jrd_nw_local_cache->sys_info.mcc,\
                                       p_jrd_nw_local_cache->sys_info.mnc);
  if((uint16)sys_info->mcc != NW_INVALID_VALUE_U16)
  {
     p_jrd_nw_local_cache->sys_info.mcc = sys_info->mcc;
     p_jrd_nw_local_cache->sys_info.mnc = sys_info->mnc;
     JRD_STRCPY(p_jrd_nw_local_cache->sys_info.short_name, sys_info->short_name);
     JRD_STRCPY(p_jrd_nw_local_cache->sys_info.long_name, sys_info->long_name);
     JRD_STRCPY(p_jrd_nw_local_cache->sys_info.spn_name, sys_info->spn_name);
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "p_jrd_nw_local_cache->sys_info.spn_name = %s,p_jrd_nw_local_cache->sys_info.short_name = %s!!!\n",
	 	                              p_jrd_nw_local_cache->sys_info.spn_name,p_jrd_nw_local_cache->sys_info.short_name);
     jrd_nw_run_nw_name_rule(p_jrd_nw_local_cache->sys_info.nw_name);
	 jrd_nw_info_chg_ind(E_JRD_NETWORK_GET_MCC_IND);
  }									   
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "registration_state:%d\n", p_jrd_nw_local_cache->sys_info.regist_status);
  if((old_regist_state != p_jrd_nw_local_cache->sys_info.regist_status) \
      ||(old_nw_type != p_jrd_nw_local_cache->sys_info.nw_type) \
      || (old_roaming != p_jrd_nw_local_cache->sys_info.roam) \
      || old_ps_attach != p_jrd_nw_local_cache->sys_info.ps_attach)
  {
     jrd_nw_info_chg_ind(E_JRD_NETWORK_INFO_CHG_IND);
  }
  if(old_sig_level != p_jrd_nw_local_cache->rf_info.sig_level)
  {
     jrd_nw_info_chg_ind(E_JRD_NETWORK_SIGNAL_IND);
  }
  if ((E_NW_START_T == p_jrd_nw_local_cache->manual_regist_info.regist_timer_status)
     && (E_NW_REGISTED == p_jrd_nw_local_cache->sys_info.regist_status))
  {
    regist_id = p_jrd_nw_local_cache->manual_regist_info.regist_id;
    if (regist_id >= 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "list rat:%d cache rat:%d\n", \
                                       p_jrd_nw_local_cache->search_info.search_list[regist_id].rat, \
                                       p_jrd_nw_local_cache->sys_info.radio_if);
      if ((p_jrd_nw_local_cache->search_info.search_list[regist_id].rat == p_jrd_nw_local_cache->sys_info.radio_if)
        && (p_jrd_nw_local_cache->search_info.search_list[regist_id].nw_mcc == p_jrd_nw_local_cache->sys_info.mcc)
        && (p_jrd_nw_local_cache->search_info.search_list[regist_id].nw_mnc == p_jrd_nw_local_cache->sys_info.mnc))
        {   
           if(p_jrd_nw_local_cache->selection_change_mode)
           {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Regist success, recovery network type, and change Selection mode to Auto type: %d\n",
                             p_jrd_nw_local_cache->selection_change_mode);
              jrd_nw_recover_network_mode();
           }
        
          p_jrd_nw_local_cache->manual_regist_info.regist_timer_status = E_NW_TIMER_STOP_T;
          //if(p_jrd_nw_local_cache->manual_regist_info.network_regist_timer != INVALID_TIMER_HANDLE)
          //{
          //  webs_killtimer(&p_jrd_nw_local_cache->manual_regist_info.network_regist_timer);
          jrd_timer_stop(&(p_jrd_nw_local_cache->manual_regist_info.network_regist_timer));
          //}
        }
    }
  }  
  
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_operat_name_ind

DESCRIPTION
  Judge if operator name data information has changed, if yes, send indication.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_operat_name_ind(jrd_ind_data_type *ind)
{
  jrd_oem_operator_name_info_t* operator_name_info;
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if((NULL == ind) ||(sizeof(jrd_oem_operator_name_info_t) != ind->data_size) || (NULL == ind->ind_data))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind!!!\n");
    return -1;
  }
  
  operator_name_info = (jrd_oem_operator_name_info_t*)ind->ind_data;
  
  if((uint16)p_jrd_nw_local_cache->sys_info.mnc != NW_INVALID_VALUE_U16)
  {
      p_jrd_nw_local_cache->operator_name_info =  *operator_name_info;
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "nitz_long_name = %s,nitz_short_name = %s, display_cond =%d\n",\
	 	                                p_jrd_nw_local_cache->operator_name_info.long_nitz_name,p_jrd_nw_local_cache->operator_name_info.short_nitz_name, \
	 	                                p_jrd_nw_local_cache->operator_name_info.display_cond);
     jrd_nw_run_nw_name_rule(p_jrd_nw_local_cache->sys_info.nw_name);
     jrd_nw_run_tmo_nos_rule();
  }  
  
  return 0;
}


/*===========================================================================

FUNCTION jrd_nw_handle_lte_ca_ind

DESCRIPTION
  Judge if lte ca status has changed, if yes, send indication.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_lte_ca_ind(jrd_ind_data_type *ind)
{
  e_jrd_oem_scell_state_t* scell_state;
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  e_jrd_oem_scell_state_t old_scell_state = p_jrd_nw_local_cache->cell_info.lte_info.scell_state;

  if((NULL == ind) ||(sizeof(e_jrd_oem_scell_state_t) != ind->data_size) || (NULL == ind->ind_data))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind!!!\n");
    return -1;
  }
  
  scell_state = (e_jrd_oem_scell_state_t*)ind->ind_data;
  if(scell_state != NULL)
  {
    p_jrd_nw_local_cache->cell_info.lte_info.scell_state = *scell_state;
  }
  if(old_scell_state != p_jrd_nw_local_cache->cell_info.lte_info.scell_state)
  {
     if(p_jrd_nw_local_cache->cell_info.lte_info.scell_state == E_NAS_LTE_CPHY_SCELL_STATE_CONFIGURED_ACTIVATED)
     {
       p_jrd_nw_local_cache->sys_info.nw_type = E_NW_TYPE_LTE_CA;
     }
     jrd_nw_info_chg_ind(E_JRD_NETWORK_INFO_CHG_IND);
  }
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_event_report_ind

DESCRIPTION
  Judge if signal has changed, if yes, send indication.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_event_report_ind(jrd_ind_data_type *ind)
{
  jrd_oem_rf_info_t* rf_info;
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  uint8 old_sig_level = p_jrd_nw_local_cache->rf_info.sig_level;
  
  if((NULL == ind) || (sizeof(jrd_oem_rf_info_t) != ind->data_size) || (NULL == ind->ind_data))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind!!!\n");
    return -1;
  }
  
  rf_info = (jrd_oem_rf_info_t*)ind->ind_data;
  
  if(rf_info->rsrq != NW_INVALID_VALUE)
  {
     p_jrd_nw_local_cache->rf_info.rsrq = rf_info->rsrq;
  }
  if(rf_info->rssi != NW_INVALID_VALUE)
  {
     p_jrd_nw_local_cache->rf_info.rssi = rf_info->rssi;
  }
  if(rf_info->rsrp != NW_INVALID_VALUE)
  {
     if(p_jrd_nw_local_cache->sys_info.radio_if == E_NW_RADIO_IF_LTE)
     {
       p_jrd_nw_local_cache->rf_info.rsrp = rf_info->rsrp;
     }
     else
     {
        p_jrd_nw_local_cache->rf_info.rsrp = NW_INVALID_VALUE;
     }
  }
  if(rf_info->sinr != NW_INVALID_VALUE_U8)
  {
     if(p_jrd_nw_local_cache->sys_info.radio_if == E_NW_RADIO_IF_LTE)
     {
       p_jrd_nw_local_cache->rf_info.sinr = rf_info->sinr;
     }
     else
     {
       p_jrd_nw_local_cache->rf_info.sinr = NW_INVALID_VALUE_U8;
     }
  }
  if(rf_info->tx_pwr != NW_INVALID_VALUE)
  {
     p_jrd_nw_local_cache->rf_info.tx_pwr = rf_info->tx_pwr;
  }
  if(rf_info->sig_level != NW_INVALID_VALUE)
  {
     p_jrd_nw_local_cache->rf_info.sig_level = rf_info->sig_level;
  }
  if(rf_info->ecio != 0)
  {
     p_jrd_nw_local_cache->rf_info.ecio = (float)rf_info->ecio*(-0.5);
     if(p_jrd_nw_local_cache->sys_info.radio_if == E_NW_RADIO_IF_UMTS)
     {
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Under UMTS network!!!\n");
       jrd_get_cell_info(&p_jrd_nw_local_cache->cell_info);
     }
     else
     {
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Not in UMTS network, current is :%d!!!\n", p_jrd_nw_local_cache->sys_info.radio_if);
       p_jrd_nw_local_cache->cell_info.umts_info.rscp = NW_INVALID_VALUE;
     }
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "p_jrd_nw_local_cache->rf_info.rssi:%d ind rssi:%d!!!\n" \
                                  , p_jrd_nw_local_cache->rf_info.rssi, rf_info->rssi);
  if(old_sig_level != p_jrd_nw_local_cache->rf_info.sig_level)
  {
     jrd_nw_info_chg_ind(E_JRD_NETWORK_SIGNAL_IND);
  }
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_band_info_ind

DESCRIPTION
  Judge if band reference information changed, if yes, send indication.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_band_info_ind(jrd_ind_data_type *ind)
{
  jrd_oem_band_info_t* p_band_info_ind;
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if((NULL == ind) || (sizeof(jrd_oem_band_info_t) != ind->data_size) || (NULL == ind->ind_data))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind!!!\n");
    return -1;
  }
  p_band_info_ind = (jrd_oem_band_info_t*)ind->ind_data;
  if(p_band_info_ind->active_band != NW_INVALID_VALUE)
  {
    p_jrd_nw_local_cache->band_info.active_band = p_band_info_ind->active_band;
  }
  if(p_band_info_ind->active_channel != NW_INVALID_VALUE)
  {
    p_jrd_nw_local_cache->band_info.active_channel = p_band_info_ind->active_channel;
  }
  return 0;
}

/*===========================================================================

FUNCTION jrd_oem_nw_searching_timeout_cb

DESCRIPTION
  Seach network timer call back function. If search network timeout, send indication.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_oem_nw_searching_timeout_cb(void *usr_data)
{
  jrd_ind_data_type ind_data;

  ind_data.module_id = MODULE_NETWORK;
  ind_data.ind_id = E_JRD_NETWORK_SEARCH_TIMEOUT_IND;
  ind_data.ind_data = NULL;
  ind_data.data_size = 0;
  jrd_oem_ind(&ind_data);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "search timout\n");
  return;
}

/*===========================================================================

FUNCTION jrd_nw_handle_begin_search_ind

DESCRIPTION
  Handle begin search indication. Start search timer, set current search status to searching.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_begin_search_ind(jrd_ind_data_type *ind)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  int rc;
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, " : Begin searching\n");
  p_jrd_nw_local_cache->search_info.search_state = E_NETWORK_SEARCH_SEARCHING;
  p_jrd_nw_local_cache->manual_regist_info.regist_id = -1;
  //Connie modify start, 2016/02/24
  //if(p_jrd_nw_local_cache->search_info.network_searching_timer != INVALID_TIMER_HANDLE)
  //{
  //   webs_killtimer(&p_jrd_nw_local_cache->search_info.network_searching_timer);
  //}
  
  //p_jrd_nw_local_cache->search_info.network_searching_timer = webs_regtimer(jrd_oem_nw_searching_timeout_cb, NW_SEARCHING_TIMEOUT, 0, 0);
  //if(p_jrd_nw_local_cache->search_info.network_searching_timer == INVALID_TIMER_HANDLE)
  //{
  //  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " Start network scan timer fail\n");
  //  p_jrd_nw_local_cache->search_info.search_state = E_NETWORK_SEARCH_FAILED;
  //  return E_NW_RESULT_ERR_QMI_FAIL;
  //}
  p_jrd_nw_local_cache->search_info.network_searching_timer.ts.tv_sec = NW_SEARCHING_TIMEOUT;
  p_jrd_nw_local_cache->search_info.network_searching_timer.ts_interval.tv_sec = 0;
  p_jrd_nw_local_cache->search_info.network_searching_timer.cb.handler = jrd_oem_nw_searching_timeout_cb;
  p_jrd_nw_local_cache->search_info.network_searching_timer.cb.params = NULL;
  rc = jrd_timer_start(&(p_jrd_nw_local_cache->search_info.network_searching_timer));
  if(rc) {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " Start network scan timer fail\n");
    p_jrd_nw_local_cache->search_info.search_state = E_NETWORK_SEARCH_FAILED;
    return E_NW_RESULT_ERR_QMI_FAIL;
  }
  //Connie modify end, 2016/02/24
  
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_search_result_ind

DESCRIPTION
  Handle the search result.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_search_result_ind(jrd_ind_data_type *ind)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  jrd_oem_nw_search_result_t* search_info;

  if((NULL == ind) || (sizeof(jrd_oem_nw_search_result_t) != ind->data_size) || (NULL == ind->ind_data))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind!!!\n");
    return -1;
  }
  search_info = (jrd_oem_nw_search_result_t*)ind->ind_data;
  
/*start for idea scan network name customization*/
uint32 sim_plmn = 0;
uint32 net_plmn = 0;
uint8 idx = 0;
for(idx=0;idx<search_info->list_len;idx++)
{
sim_plmn = (uint32)p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mcc*1000+(uint32)p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mnc;


net_plmn = (uint32)search_info->search_list[idx].nw_mcc*1000+(uint32)search_info->search_list[idx].nw_mnc;
JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "idx:%d, SIM_plmn: %d, NET_plmn: %d, nw_type_rat: %d!!!\n",idx,sim_plmn,net_plmn,search_info->search_list[idx].rat);
	if(jrd_network_compare_simplmn_netplmn(sim_plmn,net_plmn) \
	  &&((search_info->search_list[idx].rat == 1) \
	  ||(search_info->search_list[idx].rat == 2) \
	  ||(search_info->search_list[idx].rat == 5) \
	  ||(search_info->search_list[idx].rat == 9)))
	{
	JRD_MEMSET(search_info->search_list[idx].nw_short_name, 0, MAX_NW_NAME_LEN);
	JRD_MEMCPY(search_info->search_list[idx].nw_short_name,"!dea",JRD_STRLEN("!dea"));
	JRD_MEMSET(search_info->search_list[idx].nw_name, 0, MAX_NW_NAME_LEN);
	JRD_MEMCPY(search_info->search_list[idx].nw_name,"!dea",JRD_STRLEN("!dea"));
	search_info->search_list[idx].nw_name_len = JRD_STRLEN("!dea")*2 + 1;
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "special circles display '!dea' when scaning special network! idx:%d,total len:%d,nw_short_name:%s,nw_name:%s\n",\
					   idx,search_info->search_list[idx].nw_name_len,search_info->search_list[idx].nw_short_name,search_info->search_list[idx].nw_name);
	}
}
/*end for idea scan network name customization*/

  p_jrd_nw_local_cache->search_info.list_len = search_info->list_len;
  p_jrd_nw_local_cache->search_info.search_state = search_info->search_state;
  JRD_MEMCPY(p_jrd_nw_local_cache->search_info.search_list, 
                        search_info->search_list, 
                        search_info->list_len*sizeof(jrd_nw_search_list_t));
  //if(p_jrd_nw_local_cache->search_info.network_searching_timer != INVALID_TIMER_HANDLE)
  //{
    //webs_killtimer(&p_jrd_nw_local_cache->search_info.network_searching_timer);
    jrd_timer_stop(&(p_jrd_nw_local_cache->search_info.network_searching_timer)); //Connie modified, 2016/2/24
  //}
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_set_auto

DESCRIPTION
  Set current network mode & selection mode to AUTO.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
int jrd_nw_set_auto()
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  jrd_oem_nw_settings_t nw_settings;

  JRD_MEMSET(&nw_settings, 0, sizeof(jrd_oem_nw_settings_t));
  p_jrd_nw_local_cache->sys_info.regist_status = E_NW_REGIST_FAILED;
  nw_settings.nw_mode = E_NETWORK_MODE_AUTO;
  nw_settings.sel_mode = E_NW_SELECT_MODE_AUTO;
  nw_settings.band_pref = E_NW_BAND_INVALID;
  if(E_NW_RESULT_ERR_SUCCESS == jrd_set_nw_sys_sel_pref(nw_settings))
  {
     p_jrd_nw_local_cache->nw_settings_info.nw_mode = nw_settings.nw_mode;
     p_jrd_nw_local_cache->nw_settings_info.sel_mode = nw_settings.sel_mode;
     p_jrd_nw_local_cache->nw_settings_info.band_pref = nw_settings.band_pref;
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Regist failed, set auto failed!!!\n");
  }
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_recover_network_mode

DESCRIPTION
  Set current network mode & selection mode to AUTO.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
int jrd_nw_recover_network_mode(void)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  jrd_oem_nw_settings_t nw_settings;

  JRD_MEMSET(&nw_settings, 0, sizeof(jrd_oem_nw_settings_t));
  p_jrd_nw_local_cache->sys_info.regist_status = E_NW_REGISTED;
  nw_settings.nw_mode = p_jrd_nw_local_cache->nw_settings_info.nw_mode;
  if(p_jrd_nw_local_cache->nw_settings_info.sel_mode == E_NW_SELECT_MODE_MANUAL)
  {
     nw_settings.sel_mode = E_NW_SELECT_MODE_AUTO;
  }
  //nw_settings.sel_mode = p_jrd_nw_local_cache->nw_settings_info.sel_mode;
  nw_settings.band_pref = E_NW_BAND_INVALID;
  if(E_NW_RESULT_ERR_SUCCESS == jrd_set_nw_sys_sel_pref(nw_settings))
  {
      if(p_jrd_nw_local_cache->selection_change_mode == E_SELECTION_CHANGE_MODEM_ONLY)    //UI is still Manual but Modem is Auto
      {
          p_jrd_nw_local_cache->nw_settings_info.nw_mode = nw_settings.nw_mode;
         // p_jrd_nw_local_cache->nw_settings_info.sel_mode = nw_settings.sel_mode;
      }
      else  //UI and Modem change to Auto Selection mode 
      {
         p_jrd_nw_local_cache->nw_settings_info.nw_mode = nw_settings.nw_mode;
         p_jrd_nw_local_cache->nw_settings_info.sel_mode = nw_settings.sel_mode;
      }
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Regist success, recover network type failed!!!\n");
  }
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_regist_result_ind

DESCRIPTION
  Handle manual regist result.
  Kill regist timer and update the regist status.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_regist_result_ind(jrd_ind_data_type *ind)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  e_jrd_oem_nw_regist_status_t* regist_status;
  
  if((NULL == ind) || (sizeof(e_jrd_oem_nw_regist_status_t) != ind->data_size) || (NULL == ind->ind_data))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind!!!\n");
    p_jrd_nw_local_cache->sys_info.regist_status = E_NW_REGIST_FAILED;
    //if(p_jrd_nw_local_cache->search_info.network_searching_timer != INVALID_TIMER_HANDLE)
    //{
    //  webs_killtimer(&p_jrd_nw_local_cache->manual_regist_info.network_regist_timer);
    jrd_timer_stop(&(p_jrd_nw_local_cache->manual_regist_info.network_regist_timer)); //Connie modified, 2016/2/24
    //}
    return -1;
  }
  regist_status = (e_jrd_oem_nw_regist_status_t*) ind->ind_data;
  p_jrd_nw_local_cache->sys_info.regist_status = *regist_status;

  if(p_jrd_nw_local_cache->sys_info.regist_status == E_NW_REGIST_FAILED)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Regist failed, set auto %d\n",
	                p_jrd_nw_local_cache->regist_failed_to_restore);
    if(p_jrd_nw_local_cache->regist_failed_to_restore)
    {
        //jrd_nw_restore_nw_status();
    }
    else
    {
        jrd_nw_set_auto();
    }


    //if(p_jrd_nw_local_cache->search_info.network_searching_timer != INVALID_TIMER_HANDLE)
    //{
      //webs_killtimer(&p_jrd_nw_local_cache->manual_regist_info.network_regist_timer);
    jrd_timer_stop(&(p_jrd_nw_local_cache->manual_regist_info.network_regist_timer)); //Connie modified, 2016/2/24
    //}
  }
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_search_timout_ind

DESCRIPTION
  Handle search time out indication.Set search status to failed.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_search_timout_ind(jrd_ind_data_type *ind)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  if (p_jrd_nw_local_cache->search_info.search_state == E_NETWORK_SEARCH_SEARCHING)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Search timeout!!!\n");
    p_jrd_nw_local_cache->search_info.search_state = E_NETWORK_SEARCH_FAILED;
  }
  //if(p_jrd_nw_local_cache->search_info.network_searching_timer != INVALID_TIMER_HANDLE)
  //{
  //  webs_killtimer(&p_jrd_nw_local_cache->search_info.network_searching_timer);
  jrd_timer_stop(&(p_jrd_nw_local_cache->search_info.network_searching_timer)); //Connie modified, 2016/2/24
  //}
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_search_timout_ind

DESCRIPTION
  Handle regist time out indication.Set regist status to failed.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_regist_timout_ind(jrd_ind_data_type *ind)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if (p_jrd_nw_local_cache->sys_info.regist_status != E_NW_REGISTED)
  {
    p_jrd_nw_local_cache->manual_regist_info.regist_timer_status = E_NW_TIMEOUT_T;
    //jrd_nw_set_auto();
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Regist failed, set auto %d\n",
	                p_jrd_nw_local_cache->regist_failed_to_restore);
    if(p_jrd_nw_local_cache->regist_failed_to_restore)
    {
        //jrd_nw_restore_nw_status();
    }
    else
    {
        jrd_nw_set_auto();
    }
  }
  else
  {
    p_jrd_nw_local_cache->manual_regist_info.regist_timer_status = E_NW_TIMER_STOP_T;
  }
  //if(p_jrd_nw_local_cache->manual_regist_info.network_regist_timer != INVALID_TIMER_HANDLE)
  //{
  //  webs_killtimer(&p_jrd_nw_local_cache->manual_regist_info.network_regist_timer);
  jrd_timer_stop(&(p_jrd_nw_local_cache->manual_regist_info.network_regist_timer));
  //}
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_usim_ind

DESCRIPTION
  Handle indication of USIM module.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_usim_ind(jrd_ind_data_type *indication)
{
  jrd_usim_ind_data_t *usim_ind_data = (jrd_usim_ind_data_t *)indication->ind_data;
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char mcc[4] = "";
  char mnc[4] = "";
  char plmn[7] = "";

  if (!indication || !usim_ind_data) {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error, Invalid param, pointer is NULL.\n");
    return JRD_FAIL;
  }

  if (indication->module_id != MODULE_USIM) {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error, Invaild moudle id(%d) of indication.\n", indication->module_id);
    return JRD_FAIL;
  }

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Info, usim card state: %d.\n", usim_ind_data->card_state);
  if (usim_ind_data->card_state == E_JRD_USIM_CARD_STATE_READY) {

    /* MCC */
    JRD_MEMCPY(mcc, usim_ind_data->plmn, 3);
    p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mcc = JRD_ATOI(mcc);

    /* MNC */
    JRD_MEMCPY(mnc, (usim_ind_data->plmn + 3), 3);
    p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mnc = JRD_ATOI(mnc);

    /* PLMN */
    JRD_MEMCPY(plmn, usim_ind_data->plmn, (sizeof(plmn) - 1));
    p_jrd_nw_local_cache->nw_usim_ind_cache.usim_plmn = JRD_ATOI(plmn);

    /* SPN */
    JRD_MEMSET(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_spn, 0, sizeof(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_spn));
    JRD_STRNCPY(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_spn, usim_ind_data->spn, (sizeof(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_spn) - 1));
    JRD_MEMSET(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn, 0, sizeof(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn));
    JRD_STRNCPY(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn, usim_ind_data->pnn, (sizeof(p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn) - 1));
    

    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "mcc=%d,mnc=%d,plmn=%d spn=%s pnn=%s\n",
      p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mcc,
      p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mnc,
      p_jrd_nw_local_cache->nw_usim_ind_cache.usim_plmn,
      p_jrd_nw_local_cache->nw_usim_ind_cache.usim_spn,
      p_jrd_nw_local_cache->nw_usim_ind_cache.usim_pnn);
  } else {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Info, Unneeded handle card state: %d.\n", usim_ind_data->card_state);
  }
  p_jrd_nw_local_cache->sys_info.roam =jrd_get_check_roam_state(p_jrd_nw_local_cache->sys_info.roam,\
                                       p_jrd_nw_local_cache->sys_info.nw_type,\
                                       p_jrd_nw_local_cache->sys_info.mcc,\
                                       p_jrd_nw_local_cache->sys_info.mnc);
  jrd_nw_run_nw_name_rule(p_jrd_nw_local_cache->sys_info.nw_name);
  jrd_nw_info_chg_ind(E_JRD_NETWORK_INFO_CHG_IND);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Info, Unneeded handle roam state: %d.\n", p_jrd_nw_local_cache->sys_info.roam);
  return JRD_NO_ERR;
}

/*===========================================================================

FUNCTION jrd_nw_handle_connection_ind

DESCRIPTION
  Judge if lte ca status has changed, if yes, send indication.

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_connect_status_ind(jrd_ind_data_type *ind)
{
   jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
   jrd_connect_info_ind_type* connect_info_ind;
   char cmd[255] = "";
   uint32 mss = 0;
   static int flag = 0;

   JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"jrd_oen_nw_connect_status_ind: Enter\n");
   if(!ind)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"jrd_oen_nw_connect_status_ind: invalid ind_info\n");
     return JRD_FAIL; 
   }
   connect_info_ind = (jrd_connect_info_ind_type*)ind->ind_data;
   JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"p_jrd_nw_sys_para_s->nw_type: %d\n", p_jrd_nw_local_cache->sys_info.nw_type);
   if(connect_info_ind->conn_status == E_WWAN_CONNECTED && flag == 0)
   {   
     if(p_jrd_nw_local_cache->sys_info.nw_type != E_NW_TYPE_EVDO_RO
        && p_jrd_nw_local_cache->sys_info.nw_type != E_NW_TYPE_EVDO_RA
        && p_jrd_nw_local_cache->sys_info.nw_type != E_NW_TYPE_CDMA
        && p_jrd_nw_local_cache->sys_info.nw_type != E_NW_TYPE_EVDO_RB)
       {
	   JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"p_jrd_nw_local_cache->mtu_value.lte_mtu: %d\n", p_jrd_nw_local_cache->mtu_value.lte_mtu); 
       if(p_jrd_nw_local_cache->mtu_value.lte_mtu == 0)
       {            
          jrd_sys_al_get_modem_mtu_value(&p_jrd_nw_local_cache->mtu_value.lte_mtu, &p_jrd_nw_local_cache->mtu_value.hrpd_mtu, &p_jrd_nw_local_cache->mtu_value.ehrpd_mtu);
       }
		memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "echo %d > /sys/kernel/oem_mtu/mtu", p_jrd_nw_local_cache->mtu_value.lte_mtu);
		system(cmd);
     }
     /*else, resaved for HRPD/eHRPD MTU value*/
	mss = p_jrd_nw_local_cache->mtu_value.lte_mtu - 40;
	//system("iptables -F");
     flag = 1;
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss %u", mss);
	system(cmd);
   }

return 0;
}

/*===========================================================================

FUNCTION jrd_nw_handle_usr_connect_ind

DESCRIPTION

DEPENDENCIES
  None.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  None.

===========================================================================*/
static int jrd_nw_handle_usr_connect_ind(jrd_ind_data_type *ind)
{
   jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
   int rc = 0;

   JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"jrd_nw_handle_usr_connect_ind: Enter\n");
   if(!ind)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"jrd_nw_handle_usr_connect_ind: invalid ind_info\n");
     return JRD_FAIL; 
   }

	/*
	change the network mode to before.
	*/
	
	rc = jrd_sys_al_set_modem_mode(E_JRD_SYS_MODEM_MODE_LOW_POWER);
	if (rc != JRD_NO_ERR) {
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, set modem to low power mode failed.\n");
		return JRD_FAIL;
	}

	/* Reboot device. */
	JRD_SLEEP(1);

	rc = jrd_sys_al_clear_mru();
	if (rc != JRD_NO_ERR) {
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, set modem to low power mode failed.\n");
		return JRD_FAIL;
	}

	//JRD_SLEEP(1);

	rc = jrd_sys_al_set_modem_mode(E_JRD_SYS_MODEM_MODE_ONLINE);
	if (rc != JRD_NO_ERR) {
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, set modem to online mode failed.\n");
		return JRD_FAIL;
	}

	return 0;
}


/*Indication table, data struct as below*/
jrd_ind_proc_info_type jrd_oem_nw_inds_tbl[] = 
{
  /*  ind provider                       ind id                                 ind handler*/
  {MODULE_NETWORK, E_JRD_QMI_NAS_SERV_SYS_IND, jrd_nw_handle_serving_sys_ind},   
  {MODULE_NETWORK, E_JRD_QMI_NAS_OPERAT_NAME_IND, jrd_nw_handle_operat_name_ind},   
  {MODULE_NETWORK, E_JRD_QMI_NAS_LTE_CA_IND, jrd_nw_handle_lte_ca_ind},   
  {MODULE_NETWORK, E_JRD_QMI_NAS_EVENT_REPORT_IND, jrd_nw_handle_event_report_ind},   
  {MODULE_NETWORK, E_JRD_QMI_NAS_BAND_INFO_IND, jrd_nw_handle_band_info_ind},  
  {MODULE_NETWORK, E_JRD_QMI_NAS_BEGIN_SEARCH_IND, jrd_nw_handle_begin_search_ind},
  {MODULE_NETWORK, E_JRD_QMI_NAS_SEARCH_RESULT_IND, jrd_nw_handle_search_result_ind},
  {MODULE_NETWORK, E_JRD_QMI_NAS_REGIST_IND, jrd_nw_handle_regist_result_ind},
  {MODULE_NETWORK, E_JRD_NETWORK_SEARCH_TIMEOUT_IND, jrd_nw_handle_search_timout_ind},
  {MODULE_NETWORK, E_JRD_NETWORK_REGIST_TIMEOUT_IND, jrd_nw_handle_regist_timout_ind},
  {MODULE_USIM,    E_JRD_USIM_IND_CARD_STATUS_CHANGE, jrd_nw_handle_usim_ind},
  {MODULE_CONNECTION,   JRD_CONN_CONN_STATUS_IND, jrd_nw_handle_connect_status_ind },
  {MODULE_CONNECTION,   JRD_CONN_USER_CONNECT_IND, jrd_nw_handle_usr_connect_ind },  
};

/***********************************************************************/
/*************************5.4.1	GetNetworkInfo****************************/
/***********************************************************************/
static int jrd_get_param_nw_plmn(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_plmn[MAX_PLMN_LEN] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  
  JRD_SPRINTF(str_plmn, "%03d%02d", \
              p_jrd_nw_local_cache->sys_info.mcc, \
              p_jrd_nw_local_cache->sys_info.mnc);
  
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_PLMN,
                                str_plmn);
  return 0;
}

static int jrd_get_param_nw_type(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_NW_TYPE,
                                &p_jrd_nw_local_cache->sys_info.nw_type);
  return 0;
}

static int jrd_get_param_nw_name(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_NW_NAME,
                                p_jrd_nw_local_cache->sys_info.nw_name);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "p_jrd_nw_local_cache->sys_info.nw_name:%s\n",p_jrd_nw_local_cache->sys_info.nw_name);  
  return 0;
}

static int jrd_get_param_spn_name(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_SPN_NAME,
                                p_jrd_nw_local_cache->sys_info.spn_name);
  return 0;
}

static int jrd_get_param_lac(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_lac[MAX_CELLINFO_LEN] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  if(p_jrd_nw_local_cache->sys_info.radio_if != E_NW_RADIO_IF_LTE)
  {
    if(p_jrd_nw_local_cache->sys_info.lac == 0)
    {
       jrd_get_nw_serving_system(&p_jrd_nw_local_cache->sys_info);
    }
    JRD_SPRINTF(str_lac, "%d", p_jrd_nw_local_cache->sys_info.lac);
  }
  else
  {
    JRD_SPRINTF(str_lac, "0");
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_LAC,
                                str_lac);
  return 0;
}

static int jrd_get_param_cellid(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_cellid[MAX_CELLINFO_LEN] = "";
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  JRD_SPRINTF(str_cellid, "%d", p_jrd_nw_local_cache->sys_info.cell_id);
  
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_CELLID,
                                str_cellid);
  return 0;
}

static int jrd_get_param_rncid(jrd_cmd_data_type* data_ptr)  //reserved
{
  char rncid[] = "reserved";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_RNCID,
                                rncid);
  
  return 0;
}

//Judgment should be al layer,Need to send true state to other modules  .......use jrd_get_check_roam_state by wei.huang.sz
e_jrd_nw_roam_t jrd_get_check_roam_state(e_jrd_nw_roam_t roam,  e_jrd_oem_nw_type_t nw_type, uint16 mcc, uint16 mnc)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  //e_jrd_nw_roam_t roam = E_NW_ROAM_OFF;
  e_jrd_nw_roam_t domestic_roam = E_NW_ROAM_OFF;
  uint32 sim_plmn = 0;
  uint32 net_plmn = 0;

  //roam = p_jrd_nw_local_cache->sys_info.roam;

	sim_plmn = (uint32)p_jrd_nw_local_cache->nw_usim_ind_cache.usim_plmn;
	if (mnc > 99)
	{
		net_plmn =mnc + mcc*1000;
	}
	else
	{
		net_plmn =mnc + mcc*100;
	}
	//net_plmn = (uint32)p_jrd_nw_local_cache->sys_info.mcc*1000+(uint32)p_jrd_nw_local_cache->sys_info.mnc;
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "SIM_plmn: %d, NET_plmn: %d!!!\n",sim_plmn,net_plmn);
	if(E_NW_TYPE_NO_SERVER != nw_type)
	{
	    if(roam == E_NW_ROAM_ON)
	    {
		if (0 == jrd_show_roam_specify_handle(sim_plmn,net_plmn))
		{
			roam = E_NW_ROAM_OFF;
		}
		if(p_jrd_nw_local_cache->jrd_network_db_config.nw_roam_rule == E_SHOW_ROAM_INTERNATIONAL_ONLY)
		{
			if(mcc == p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mcc) /*Not international roaming*/
			{
				JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Not international roaming.\n");
				roam = E_NW_ROAM_OFF;
			}
		}
		/*Start for ICR ROAM,zengyong,2016.4.11*/	 
		else if(((p_jrd_nw_local_cache->jrd_network_db_config.nw_roam_rule == E_SHOW_NO_ROAM_3G_ICR)\
		&&((nw_type == E_NW_TYPE_HSDPA)\
		||(nw_type == E_NW_TYPE_HSUPA)\
		||(nw_type == E_NW_TYPE_WCDMA)\
		||(nw_type == E_NW_TYPE_HSDPA_PLUS)\
		||(nw_type == E_NW_TYPE_DC_HSDPA_PLUS)))\
		||((p_jrd_nw_local_cache->jrd_network_db_config.nw_roam_rule == E_SHOW_NO_ROAM_2G_3G_ICR)\
		&&((nw_type == E_NW_TYPE_HSDPA)\
		||(nw_type == E_NW_TYPE_HSUPA)\
		||(nw_type == E_NW_TYPE_WCDMA)\
		||(nw_type == E_NW_TYPE_HSDPA_PLUS)\
		||(nw_type == E_NW_TYPE_DC_HSDPA_PLUS)\
		||(nw_type == E_NW_TYPE_GPRS) \
		||(nw_type == E_NW_TYPE_EDGE) \
		||(nw_type == E_NW_TYPE_CDMA) \
		||(nw_type == E_NW_TYPE_EVDO_RO) \
		||(nw_type == E_NW_TYPE_EVDO_RA) \
		||(nw_type == E_NW_TYPE_GSM) \
		||(nw_type == E_NW_TYPE_EVDO_RB))))
		{
			if(jrd_network_compare_simplmn_netplmn(sim_plmn,net_plmn))
			{
				JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ICR ROAM State,display no roaming!!!\n");
				roam = E_NW_ROAM_OFF;
			}
		}
		else
		{
			JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Show ROAM and not distinguish  internationnal & domestic roaming!!!\n");
		}
	    /*End for 3G ICR,zengyong,2016.4.11*/	
	    }
	    else if(roam == E_NW_ROAM_OFF)
	    {
   		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "NO ROAM!!!\n");
	    }
	    else
	    {
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ERROR, ROAM state is Invalid!!!\n");
	    }
	}
	else
	{
	    roam = E_NW_ROAM_OFF;
	    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid nw_type\n"); 
	}
  return roam;
}

static int jrd_get_param_roam_state(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  e_jrd_nw_roam_t roam = E_NW_ROAM_OFF;
  e_jrd_nw_roam_t domestic_roam = E_NW_ROAM_OFF;
  uint32 sim_plmn = 0;
  uint32 net_plmn = 0;

  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  roam = p_jrd_nw_local_cache->sys_info.roam;

	sim_plmn = (uint32)p_jrd_nw_local_cache->nw_usim_ind_cache.usim_plmn;
	if (p_jrd_nw_local_cache->sys_info.mnc > 99)
	{
		net_plmn =p_jrd_nw_local_cache->sys_info.mnc + p_jrd_nw_local_cache->sys_info.mcc*1000;
	}
	else
	{
		net_plmn =p_jrd_nw_local_cache->sys_info.mnc + p_jrd_nw_local_cache->sys_info.mcc*100;
	}
	//net_plmn = (uint32)p_jrd_nw_local_cache->sys_info.mcc*1000+(uint32)p_jrd_nw_local_cache->sys_info.mnc;
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "SIM_plmn: %d, NET_plmn: %d!!!\n",sim_plmn,net_plmn);
	if(E_NW_TYPE_NO_SERVER != p_jrd_nw_local_cache->sys_info.nw_type)
  {
   if(roam == E_NW_ROAM_ON)
   {
		if (0 == jrd_show_roam_specify_handle(sim_plmn,net_plmn))
		{
			roam = E_NW_ROAM_OFF;
		}
		if(p_jrd_nw_local_cache->jrd_network_db_config.nw_roam_rule == E_SHOW_ROAM_INTERNATIONAL_ONLY)
		{
			if(p_jrd_nw_local_cache->sys_info.mcc == p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mcc) /*Not international roaming*/
			{
				JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Not international roaming.\n");
				roam = E_NW_ROAM_OFF;
			}
		}
		/*Start for ICR ROAM,zengyong,2016.4.11*/	 
		else if(((p_jrd_nw_local_cache->jrd_network_db_config.nw_roam_rule == E_SHOW_NO_ROAM_3G_ICR)\
		&&((p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_HSDPA)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_HSUPA)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_WCDMA)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_HSDPA_PLUS)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_DC_HSDPA_PLUS)))\
		||((p_jrd_nw_local_cache->jrd_network_db_config.nw_roam_rule == E_SHOW_NO_ROAM_2G_3G_ICR)\
		&&((p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_HSDPA)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_HSUPA)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_WCDMA)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_HSDPA_PLUS)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_DC_HSDPA_PLUS)\
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_GPRS) \
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EDGE) \
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_CDMA) \
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EVDO_RO) \
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EVDO_RA) \
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_GSM) \
		||(p_jrd_nw_local_cache->sys_info.nw_type == E_NW_TYPE_EVDO_RB))))
		{
			if(jrd_network_compare_simplmn_netplmn(sim_plmn,net_plmn))
			{
				JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ICR ROAM State,display no roaming!!!\n");
				roam = E_NW_ROAM_OFF;
			}
	    }
		else
		{
			JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Show ROAM and not distinguish  internationnal & domestic roaming!!!\n");
		}
    /*End for 3G ICR,zengyong,2016.4.11*/	
   }
   else if(roam == E_NW_ROAM_OFF)
   {
   		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "NO ROAM!!!\n");
   }
   else
   {
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ERROR, ROAM state is Invalid!!!\n");
   }
  }
  else
  {
     roam = E_NW_ROAM_OFF;
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid nw_type\n"); 
  }

  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_ROAM_STATE,
                                &roam);
  /////////////////////add begin by wei.huang.sz/////////////////
  if(E_NW_ROAM_ON == roam)
  {
       if(p_jrd_nw_local_cache->sys_info.mcc == p_jrd_nw_local_cache->nw_usim_ind_cache.usim_mcc) /*Not international roaming*/
       {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Not international roaming.\n");
          domestic_roam = 0;

       }
  }
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_DOMESTIC_ROAM_STATE,
                                &domestic_roam);
  /////////////////////add end by wei.huang.sz/////////////////
  return 0;
}

static int jrd_get_param_sig_level(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  
  if(p_jrd_nw_local_cache->sys_info.regist_status != E_NW_REGISTED)
  {
     p_jrd_nw_local_cache->rf_info.sig_level = 0;
  }
  
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_SIG_LEVEL,
                                &p_jrd_nw_local_cache->rf_info.sig_level);
  return 0;
}

static int jrd_get_param_mcc(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_mcc[MAX_PLMN_LEN] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  JRD_SPRINTF(str_mcc, "%d", p_jrd_nw_local_cache->sys_info.mcc);
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_MCC,
                                str_mcc);
  return 0;
}

static int jrd_get_param_mnc(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_mnc[MAX_PLMN_LEN] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  JRD_SPRINTF(str_mnc, "%d", p_jrd_nw_local_cache->sys_info.mnc);
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_MNC,
                                str_mnc);
  return 0;
}

static int jrd_get_param_sinr(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_sinr[MAX_SIGNAL_INFO] = "";
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  if(p_jrd_nw_local_cache->sys_info.radio_if == E_NW_RADIO_IF_LTE)
  {
    JRD_SPRINTF(str_sinr, "%d", p_jrd_nw_local_cache->rf_info.sinr);
  }
  else
  {
    JRD_SPRINTF(str_sinr, "FF");
  }
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_SINR,
                                str_sinr);
  return 0;
}

static int jrd_get_param_rsrp(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_rsrp[MAX_SIGNAL_INFO] = "";
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  
  if(p_jrd_nw_local_cache->sys_info.radio_if == E_NW_RADIO_IF_LTE)
  {
    JRD_SPRINTF(str_rsrp, "%d", p_jrd_nw_local_cache->rf_info.rsrp);
  }
  else
  {
    JRD_SPRINTF(str_rsrp, "-1");
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_RSRP,
                                str_rsrp);
  return 0;
}

static int jrd_get_param_rssi(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_rssi[MAX_SIGNAL_INFO] = "";
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "p_jrd_nw_local_cache->rf_info.rssi:%d\n",p_jrd_nw_local_cache->rf_info.rssi);  
  JRD_SPRINTF(str_rssi, "%d", p_jrd_nw_local_cache->rf_info.rssi);
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_RSSI,
                                str_rssi);
  return 0;
}

static int jrd_get_param_enbid(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_enbid[MAX_CELLINFO_LEN] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  JRD_SPRINTF(str_enbid, "%d", p_jrd_nw_local_cache->sys_info.enbid);
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_ENBID,
                                str_enbid);  
  return 0;
}

static int jrd_get_param_cgi(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_cgi[MAX_CELLINFO_LEN] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }

  JRD_SPRINTF(str_cgi, "%d%d%x", p_jrd_nw_local_cache->sys_info.mcc,\
  	                                   p_jrd_nw_local_cache->sys_info.mnc,\
  	                                   p_jrd_nw_local_cache->sys_info.cell_id);
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_CGI,
                                str_cgi);  
  return 0;
}

static int jrd_get_param_center_freq(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_center_freq[MAX_SIGNAL_INFO] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }

  JRD_SPRINTF(str_center_freq, "%2f", p_jrd_nw_local_cache->band_info.center_freq);
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_CENTER_FREQ,
                                str_center_freq);  
  return 0;
}

static int jrd_get_param_tx_pwr(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_tx_pwr[MAX_SIGNAL_INFO] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  
  JRD_SPRINTF(str_tx_pwr, "%d", p_jrd_nw_local_cache->rf_info.tx_pwr);
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_TX_PWR,
                                str_tx_pwr);  
  return 0;
}

static int jrd_get_param_lte_state(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_LTE_STATE,
                                &p_jrd_nw_local_cache->cell_info.lte_info.ue_in_idle);  
  return 0;
}

static int jrd_get_param_plmn_name(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_PLMN_NAME,
                                p_jrd_nw_local_cache->sys_info.plmn_name);  
  
  return 0;
}

static int jrd_get_param_active_band(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_BAND,
                                &p_jrd_nw_local_cache->band_info.active_band);  
  return 0;
}

static int jrd_get_param_dl_channel(jrd_cmd_data_type* data_ptr)
{
  char str_dl_chann[MAX_CELLINFO_LEN] = "reserved";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_DL_CHANNEL,
                                str_dl_chann);  
  
  return 0;
}

static int jrd_get_param_ul_channel(jrd_cmd_data_type* data_ptr)
{
  char str_ul_chann[MAX_CELLINFO_LEN] = "reserved";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_UL_CHANNEL,
                                str_ul_chann);  
  
  return 0;
}

static int jrd_get_param_rsrq(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  char str_rsrq[MAX_SIGNAL_INFO] = "";
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }

  JRD_SPRINTF(str_rsrq, "%d", p_jrd_nw_local_cache->rf_info.rsrq);
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_RSRQ,
                                str_rsrq);  
  return 0;
}

/***********************************************************************/
/**************************5.4.2	SearchNetwork****************************/
/***********************************************************************/

int jrd_nw_manual_scan_nw(void)
{
  e_jrd_oem_nw_mode_t nw_mode;
  static e_jrd_oem_nw_mode_t nw_mode_bak = -1;
  boolean  result = FALSE;
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  
  nw_mode = p_jrd_nw_local_cache->nw_settings_info.nw_mode;
  if(nw_mode == nw_mode_bak && p_jrd_nw_local_cache->search_info.search_state == E_NETWORK_SEARCH_SEARCHING)
  {
    return E_NW_RESULT_ERR_NW_IS_SEARCHING;
  }
  if( E_NETWORK_MODE_INVALID == nw_mode )
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " Invalid network mode : return false \n");    
    return E_NW_RESULT_ERR_NW_MODE_INVALIDE;
  }
  nw_mode_bak = nw_mode;
  JRD_MEMSET(&p_jrd_nw_local_cache->search_info.search_list, 0, sizeof(p_jrd_nw_local_cache->search_info.search_list));
  p_jrd_nw_local_cache->search_info.list_len = 0;
  
  if(E_NW_RESULT_ERR_SUCCESS != jrd_search_network(nw_mode))
  {
    p_jrd_nw_local_cache->search_info.search_state = E_NETWORK_SEARCH_FAILED;
  }
  return E_NW_RESULT_ERR_SUCCESS;
}

/***********************************************************************/
/************************5.4.3	SearchNetworkResult*************************/
/***********************************************************************/

static int jrd_get_param_search_data_list(jrd_cmd_data_type* data_ptr)
{
  int idx;
  int list_len = 0;
  int data_list;
  uint32 plmn_val = 0;
  char cPlmn[MAX_PLMN_LEN] = {0};
  char nw_mcc[10] = {0};
  char nw_mnc[10] = {0};

  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  e_jrd_oem_nw_search_state_t scan_status = p_jrd_nw_local_cache->search_info.search_state;
  
  pointer_list_type *pointer_list= NULL;
  json_array_info_type target_array = {0};
  json_object_info_type *object_info = NULL;
  json_name_value_pairs_info_type *name_value_pairs_list;

  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }

  list_len = p_jrd_nw_local_cache->search_info.list_len;

  /* T("{\"nw_sel_stat\":%d,\"nw_list_len\":%d,\"data\":[") */
  data_list = (7 << 16) | list_len;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "network data_list:%d,rat:%d\n", \
  	                            data_list,sizeof(p_jrd_nw_local_cache->search_info.search_list[idx].rat));

  target_array.member_count = 0;
  target_array.member_type = E_PARAM_OBJECT_DATA;
  JRD_MALLOC(list_len*sizeof(json_object_info_type), object_info);
  jrd_oem_add_pointer(&pointer_list, object_info);
  target_array.member_data = object_info;

  if( list_len > 0 )
  {  
    /* T("{\"id\":%d,\"mcc\":%d,\"mnc\":%d,\\"rat\":%d,\"state\":%d,\"fullname\":\"%s\",\"shortname\":\"%s\"}") */
    for( idx = 0; idx < list_len; idx ++ )
    {
       plmn_val = p_jrd_nw_local_cache->search_info.search_list[idx].nw_mcc * 100 + p_jrd_nw_local_cache->search_info.search_list[idx].nw_mnc;
       JRD_SPRINTF(cPlmn, "%d", plmn_val);
		  	  
       JRD_MALLOC(10*sizeof(json_name_value_pairs_info_type), name_value_pairs_list);
       jrd_oem_add_pointer(&pointer_list, name_value_pairs_list);
       object_info[target_array.member_count].NameValuePairs_count = 8;
       object_info[target_array.member_count].NameValuePairs_data = name_value_pairs_list;
       target_array.member_count +=1;
       name_value_pairs_list[0].param_id = E_NW_PARAM_SEARCH_ITEM_ID;
       name_value_pairs_list[0].param_data = JRD_INTDUP(idx);
       name_value_pairs_list[1].param_id = E_NW_PARAM_SEARCH_ITEM_STATE;
       name_value_pairs_list[1].param_data = JRD_INTDUP(p_jrd_nw_local_cache->search_info.search_list[idx].nw_status);
       jrd_oem_add_pointer(&pointer_list, name_value_pairs_list[1].param_data);
	   
       name_value_pairs_list[2].param_id = E_NW_PARAM_SEARCH_ITEM_PLMN;
       name_value_pairs_list[2].param_data = JRD_STRDUP(cPlmn);	   
       jrd_oem_add_pointer(&pointer_list, name_value_pairs_list[2].param_data);
	   
       name_value_pairs_list[3].param_id = E_NW_PARAM_SEARCH_ITEM_RAT;
       name_value_pairs_list[3].param_data = JRD_INTDUP(p_jrd_nw_local_cache->search_info.search_list[idx].rat);	   
       jrd_oem_add_pointer(&pointer_list, name_value_pairs_list[3].param_data);
	   
       name_value_pairs_list[4].param_id = E_NW_PARAM_SEARCH_ITEM_FULL_NAME;
       name_value_pairs_list[4].param_data = JRD_STRDUP(p_jrd_nw_local_cache->search_info.search_list[idx].nw_name);
       jrd_oem_add_pointer(&pointer_list, name_value_pairs_list[4].param_data);
       if(p_jrd_nw_local_cache->jrd_oem_search_list_same_longnetworkname)
       {
		name_value_pairs_list[5].param_id = E_NW_PARAM_SEARCH_ITEM_SHORT_NAME;
		name_value_pairs_list[5].param_data = JRD_STRDUP(p_jrd_nw_local_cache->search_info.search_list[idx].nw_name);
		jrd_oem_add_pointer(&pointer_list, name_value_pairs_list[5].param_data);
       }
       else
       {
		name_value_pairs_list[5].param_id = E_NW_PARAM_SEARCH_ITEM_SHORT_NAME;
		name_value_pairs_list[5].param_data = JRD_STRDUP(p_jrd_nw_local_cache->search_info.search_list[idx].nw_short_name);
		jrd_oem_add_pointer(&pointer_list, name_value_pairs_list[5].param_data);
       }
       JRD_SPRINTF(nw_mcc,"%d",p_jrd_nw_local_cache->search_info.search_list[idx].nw_mcc);
       JRD_SPRINTF(nw_mnc,"%d",p_jrd_nw_local_cache->search_info.search_list[idx].nw_mnc);
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "####################### %s \n",nw_mcc);
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "####################### %s \n",nw_mnc);
       name_value_pairs_list[6].param_id = E_NW_PARAM_MCC;
       name_value_pairs_list[6].param_data = JRD_STRDUP(nw_mcc);
       //name_value_pairs_list[6].param_data = JRD_INTDUP(p_jrd_nw_local_cache->search_info.search_list[idx].nw_mcc);
       jrd_oem_add_pointer(&pointer_list, name_value_pairs_list[6].param_data);

       name_value_pairs_list[7].param_id = E_NW_PARAM_MNC;
       name_value_pairs_list[7].param_data = JRD_STRDUP(nw_mnc);
       jrd_oem_add_pointer(&pointer_list, name_value_pairs_list[7].param_data);		
    }
    data_ptr->error_code = 0;
    jrd_oem_add_param_to_resp_object(data_ptr, 
                      			     MODULE_NETWORK,
                      			     E_NW_PARAM_SEARCH_DATA_LIST, 
                      			     &target_array);
    jrd_oem_free_pointer_list(&pointer_list);
    return 0;
  }  
  return -1;
}

  static int jrd_get_param_search_state(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_SEARCH_STATE,
                                &p_jrd_nw_local_cache->search_info.search_state);  
  
  if( E_NETWORK_SEARCH_SEARCHING != p_jrd_nw_local_cache->search_info.search_state )
  {
    p_jrd_nw_local_cache->search_info.search_state = E_NETWORK_SEARCH_NO_NETWORK;
  }
  
  return 0;
}

  static int jrd_get_param_search_item_state(jrd_cmd_data_type* data_ptr)
{
  return 0;
}

  static int jrd_get_param_search_item_rat(jrd_cmd_data_type* data_ptr)
{
	return 0;
}

  static int jrd_get_param_search_item_plmn(jrd_cmd_data_type* data_ptr)
{
  return 0;
}

  static int jrd_get_param_search_item_id(jrd_cmd_data_type* data_ptr)
{
 	return 0;
}

  static int jrd_get_param_search_item_full_name(jrd_cmd_data_type* data_ptr)
{
  return 0;
}

  static int jrd_get_param_search_item_short_name(jrd_cmd_data_type* data_ptr)
{
  return 0;
}

  static int jrd_get_param_search_item_len(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  
    data_ptr->error_code = 0;
    jrd_oem_add_param_to_resp_object(data_ptr, 
                                  MODULE_NETWORK,
                                  E_NW_PARAM_SEARCH_LIST_LEN,
                                  &p_jrd_nw_local_cache->search_info.list_len);  
  return 0;
}

/***********************************************************************/
/*************************5.4.4	RegisterNetwork****************************/
/***********************************************************************/

static int jrd_nw_regist_timeout_cb(void *usr_data)
{
  jrd_ind_data_type ind_data;

  ind_data.module_id = MODULE_NETWORK;
  ind_data.ind_id = E_JRD_NETWORK_REGIST_TIMEOUT_IND;
  ind_data.ind_data = NULL;
  ind_data.data_size = 0;
  jrd_oem_ind(&ind_data);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "search timout\n");
  return;
}

int jrd_nw_register(int regist_index)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  int rc;

  if (regist_index >= p_jrd_nw_local_cache->search_info.list_len)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " invalid regist index:%d \n", regist_index);
    return -1;
  }
  
  rc = jrd_regist_nw(p_jrd_nw_local_cache->search_info.search_list[regist_index]);
  if(rc != E_NW_RESULT_ERR_SUCCESS)
  {
    p_jrd_nw_local_cache->manual_regist_info.regist_id = -1;
    rc = WEBS_NW_REG_FAILED;
  }
  else
  {
    int rc;
    /*Start regist timer cuz we can't wait forever*/
    p_jrd_nw_local_cache->sys_info.regist_status = E_NW_REGISTING;
    p_jrd_nw_local_cache->manual_regist_info.regist_timer_status = E_NW_START_T;
    p_jrd_nw_local_cache->manual_regist_info.regist_id = regist_index;
    rc = WEBS_NW_NO_ERROR;
    //Connie modify start, 2016/02/24
    //p_jrd_nw_local_cache->manual_regist_info.network_regist_timer = webs_regtimer(jrd_nw_regist_timeout_cb, NW_REGIST_TIMEOUT, 0, 0);
    p_jrd_nw_local_cache->manual_regist_info.network_regist_timer.ts.tv_sec = NW_REGIST_TIMEOUT;
    p_jrd_nw_local_cache->manual_regist_info.network_regist_timer.ts_interval.tv_sec = 0;
    p_jrd_nw_local_cache->manual_regist_info.network_regist_timer.cb.handler = jrd_nw_regist_timeout_cb;
    p_jrd_nw_local_cache->manual_regist_info.network_regist_timer.cb.params = NULL;
    rc = jrd_timer_start(&(p_jrd_nw_local_cache->manual_regist_info.network_regist_timer));
    if(rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Failed to start nw regist timer(%dS, %dnS), rc=%d\n", 
            p_jrd_nw_local_cache->manual_regist_info.network_regist_timer.ts.tv_sec, p_jrd_nw_local_cache->manual_regist_info.network_regist_timer.ts.tv_nsec, rc);
    }
    //Connie modify end, 2016/02/24
  }
  return  rc;
}

/***********************************************************************/
/*********************5.4.5	GetNetworkRegisterState*************************/
/***********************************************************************/

static int jrd_get_param_regist_state(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  e_jrd_oem_nw_regist_status_t reg_status = E_NW_REGIST_INVALID;
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }

  switch(p_jrd_nw_local_cache->manual_regist_info.regist_timer_status)
  {
    case E_NW_TIMEOUT_T:
      p_jrd_nw_local_cache->manual_regist_info.regist_timer_status = E_NW_NOT_START_T;
      reg_status = E_NW_REGIST_FAILED;  
    break;
    case E_NW_START_T:
      reg_status = E_NW_REGISTING;    
    break;
    case E_NW_TIMER_STOP_T:
    case E_NW_NOT_START_T:
      p_jrd_nw_local_cache->manual_regist_info.regist_timer_status = E_NW_NOT_START_T;
      if (E_NW_REGISTED == p_jrd_nw_local_cache->sys_info.regist_status)
      {
        reg_status = E_NW_REGISTED;
      }
      else
      {
        reg_status = E_NW_NOT_REGISTED;      
      }
    break;
    default:
      reg_status = E_NW_REGIST_INVALID;    
    break;
  }

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, " regist_state(%d),timeout(%d)\n", \
                                   reg_status,p_jrd_nw_local_cache->manual_regist_info.regist_timer_status); 
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_REGIST_STATE,
                                &reg_status);  
  return 0;
}

static int jrd_get_param_regist_id(jrd_cmd_data_type* data_ptr)
{
  return 0;
}

/***********************************************************************/
/*************************5.4.6	GetNetworkSettings*************************/
/***********************************************************************/

static int jrd_get_param_nw_mode(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_NW_MODE,
                                &p_jrd_nw_local_cache->nw_settings_info.nw_mode);  
  return 0;
}

static int jrd_get_param_sel_mode(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_NW_SEL_MODE,
                                &p_jrd_nw_local_cache->nw_settings_info.sel_mode);  
  return 0;
}

static int jrd_get_param_band_pref(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_NW_BAND_PREF,
                                &p_jrd_nw_local_cache->nw_settings_info.band_pref);  
  return 0;
}

static int jrd_get_param_domatic_roam(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_DOMESTIC_ROAM,
                                &p_jrd_nw_local_cache->nw_settings_info.domestic_roam);  
    
  return 0;
}

static int jrd_get_param_domatic_roam_guard(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_DOMESTIC_ROAM_GUARD,
                                &p_jrd_nw_local_cache->nw_settings_info.domestic_roam_guard);  
  return 0;
}

/***********************************************************************/
/************************5.4.7	SetNetworkSettings**************************/
/***********************************************************************/

static boolean jrd_nw_allow_set_band_para(jrd_oem_nw_settings_t  nw_settings_info)
{
  boolean allow_val = FALSE;

  if((nw_settings_info.band_pref == E_NW_BAND_INVALID)||
     (E_NW_BAND_ALL==nw_settings_info.band_pref &&
      nw_settings_info.nw_mode !=E_NETWORK_MODE_AUTO))
  {
     /*We can set network pref without band pref*/
     allow_val = TRUE;
     return allow_val;
  }
  switch (nw_settings_info.nw_mode)
  {
    case E_NETWORK_MODE_AUTO:
      if (E_NW_BAND_ALL == nw_settings_info.band_pref)
      {
        allow_val = TRUE;
      }
      break;
    case E_NETWORK_MODE_2G_ONLY:
      if ((E_NW_BAND_GSM_ALL == nw_settings_info.band_pref)
          || (E_NW_BAND_GSM_850 == nw_settings_info.band_pref)
          || (E_NW_BAND_GSM_900 == nw_settings_info.band_pref)
          || (E_NW_BAND_GSM_1800 == nw_settings_info.band_pref)
          || (E_NW_BAND_GSM_1900 == nw_settings_info.band_pref))          
      {
        allow_val = TRUE;
      }
      break;
    case E_NETWORK_MODE_3G_ONLY:
      if ((E_NW_BAND_WCDMA_ALL == nw_settings_info.band_pref)
          || (E_NW_BAND_WCDMA_900 == nw_settings_info.band_pref)
          || (E_NW_BAND_WCDMA_1800 == nw_settings_info.band_pref)
          || (E_NW_BAND_WCDMA_850 == nw_settings_info.band_pref)
          || (E_NW_BAND_WCDMA_1900 == nw_settings_info.band_pref)
          || (E_NW_BAND_WCDMA_2100 == nw_settings_info.band_pref))          
      {
        allow_val = TRUE;
      }
      break;
    case E_NETWORK_MODE_LTE_ONLY:
      if ((E_NW_BAND_LTE_ALL == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_800 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_700_BAND17 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_700_BAND28 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_1700_BAND4 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_850 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_1900 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_900 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_1800 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_2100 == nw_settings_info.band_pref)
          || (E_NW_BAND_LTE_2600 == nw_settings_info.band_pref))          
      {
        allow_val = TRUE;
      }
      break;      
    default:
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " : invalid nw_mode(%d)\n", nw_settings_info.nw_mode);
      return FALSE;      
    
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, " : nw_mode(%d), nw_band_pref(%d) allowed\n",\
                   nw_settings_info.nw_mode, nw_settings_info.band_pref);
 
  return allow_val;
}

int jrd_nw_set_nw_sys_sel_pref(jrd_oem_nw_settings_t nw_setting_param)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  int rc=E_NW_RESULT_ERR_SUCCESS;
  
  if (jrd_nw_allow_set_band_para(nw_setting_param))
  {
    rc = jrd_set_nw_sys_sel_pref(nw_setting_param);	
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Band settings not allowed\n");
    rc = E_NW_RESULT_ERR_QMI_FAIL;
  }  
  if( E_NW_RESULT_ERR_SUCCESS == rc )
  {
    p_jrd_nw_local_cache->nw_settings_info.nw_mode = nw_setting_param.nw_mode;
    p_jrd_nw_local_cache->nw_settings_info.sel_mode = nw_setting_param.sel_mode;
    p_jrd_nw_local_cache->nw_settings_info.band_pref = nw_setting_param.band_pref;
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, " set nw_band_conf(%d)\n", nw_setting_param.band_pref);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, " set json_nw_mode(%d), nw_sel_mode(%d) successfully\n", \
		                        nw_setting_param.nw_mode, nw_setting_param.sel_mode);
  }
  else
  {
    rc = WEBS_NW_SET_SETTING_FAILED;
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " set json_nw_mode(%d), nw_sel_mode(%d) fail\n", nw_setting_param.nw_mode, nw_setting_param.sel_mode);
  }

  return rc;
}

/***********************************************************************/
/*********************5.4.8	GetNetworkModeListType*************************/
/***********************************************************************/

static int jrd_get_param_nw_mode_list_type(jrd_cmd_data_type* data_ptr)
{
  return 0;
}

/***********************************************************************/
/*******************5.4.9	GetDomRoamGuardForced*************************/
/***********************************************************************/

static int jrd_get_param_domatic_roam_guard_foced(jrd_cmd_data_type* data_ptr)
{
  return 0;
}

static int jrd_get_param_ecio(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  float tmp_ecio = 0;
    
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
   if(p_jrd_nw_local_cache->sys_info.radio_if == E_NW_RADIO_IF_UMTS)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Under UMTS network!!!\n");
     tmp_ecio = p_jrd_nw_local_cache->rf_info.ecio;
   }
   else
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Not in UMTS network, current is :%d!!!\n", p_jrd_nw_local_cache->sys_info.radio_if);
     tmp_ecio = 0;
   }
  
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_ECIO,
                                &tmp_ecio);  
  return 0;
}

static int jrd_get_param_rscp(jrd_cmd_data_type* data_ptr)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  if(data_ptr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid buffer\n");  
    return -1;
  }
   if(p_jrd_nw_local_cache->sys_info.radio_if == E_NW_RADIO_IF_UMTS)
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Under UMTS network!!!\n");
     jrd_get_cell_info(&p_jrd_nw_local_cache->cell_info);
   }
   else
   {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Not in UMTS network, current is :%d!!!\n", p_jrd_nw_local_cache->sys_info.radio_if);
     p_jrd_nw_local_cache->cell_info.umts_info.rscp = NW_INVALID_VALUE;
   }
  data_ptr->error_code = 0;
  jrd_oem_add_param_to_resp_object(data_ptr, 
                                MODULE_NETWORK,
                                E_NW_PARAM_RSCP,
                                &p_jrd_nw_local_cache->cell_info.umts_info.rscp);  
  return 0;
}

/*NOTICE: The index of table is the id of the below apis!!!!!!!!!*/
jrd_nw_get_func_type jrd_oem_nw_get_tbl[] = 
{
  jrd_get_param_nw_plmn,        /*0*/
  jrd_get_param_nw_type,
  jrd_get_param_nw_name,
  jrd_get_param_spn_name,
  jrd_get_param_lac,
  jrd_get_param_cellid,
  jrd_get_param_rncid,
  jrd_get_param_roam_state,
  jrd_get_param_sig_level,
  jrd_get_param_mcc,
  jrd_get_param_mnc,        /*10*/
  jrd_get_param_sinr,
  jrd_get_param_rsrp,
  jrd_get_param_rssi,
  jrd_get_param_enbid,
  jrd_get_param_cgi,
  jrd_get_param_center_freq,
  jrd_get_param_tx_pwr,
  jrd_get_param_lte_state,
  jrd_get_param_plmn_name,
  jrd_get_param_active_band, /*20*/
  jrd_get_param_dl_channel,
  jrd_get_param_ul_channel,
  jrd_get_param_rsrq,
  jrd_get_param_search_data_list, /*include item state, item id, item rat, item plmn, item full name, item short name*/
  jrd_get_param_search_state,
  jrd_get_param_search_item_state,
  jrd_get_param_search_item_rat,
  jrd_get_param_search_item_plmn,
  jrd_get_param_search_item_id,
  jrd_get_param_search_item_full_name, /*30*/
  jrd_get_param_search_item_short_name,
  jrd_get_param_search_item_len,
  jrd_get_param_regist_state,	/*1*/
  jrd_get_param_regist_id,//regist_id
  jrd_get_param_nw_mode,
  jrd_get_param_sel_mode,
  jrd_get_param_band_pref,
  jrd_get_param_domatic_roam,
  jrd_get_param_domatic_roam_guard,
  jrd_get_param_nw_mode_list_type,//list type
  jrd_get_param_domatic_roam_guard_foced,
  jrd_get_param_ecio,
  jrd_get_param_rscp,
};

e_jrd_oem_nw_regist_status_t jrd_nw_get_regist_status(void)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  if(p_jrd_nw_local_cache != NULL)
  {
     return p_jrd_nw_local_cache->sys_info.regist_status;
  }
  return E_NW_REGIST_INVALID;
}


/***********************************************************************/
/*******************init network parameter cache****************************/
/***********************************************************************/
/*===========================================================================

FUNCTION jrd_oem_nw_get_all_params

DESCRIPTION
  Get all the parameters we need.

DEPENDENCIES
  Need network module thread start first.
  Call the api at the beginning of network thread.

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  This api should be called by jrd_nw_init_param, do not call it at other place.
===========================================================================*/
static int jrd_oem_nw_get_all_params(void)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();
  e_jrd_oem_nw_regist_status_t old_regist_state = p_jrd_nw_local_cache->sys_info.regist_status;
  uint8 old_sig_level = p_jrd_nw_local_cache->rf_info.sig_level;
  e_jrd_nw_roam_t old_roaming = p_jrd_nw_local_cache->sys_info.roam;
  e_jrd_oem_nw_type_t old_nw_type = p_jrd_nw_local_cache->sys_info.nw_type;
  e_nw_ps_attach_status_t old_ps_attach = p_jrd_nw_local_cache->sys_info.ps_attach;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "############jrd_oem_nw_get_all_params!!!\n");
 //LZY
  jrd_get_operator_name_info(&p_jrd_nw_local_cache->operator_name_info);
  jrd_get_cell_info(&p_jrd_nw_local_cache->cell_info);
  jrd_get_nw_serving_system(&p_jrd_nw_local_cache->sys_info);
  jrd_get_signal_strength(&p_jrd_nw_local_cache->rf_info);
  jrd_get_rx_tx_info(&p_jrd_nw_local_cache->rf_info, p_jrd_nw_local_cache->sys_info.radio_if);
  jrd_get_nw_sys_sel_pref(&p_jrd_nw_local_cache->nw_settings_info);
  jrd_get_active_band_info(&p_jrd_nw_local_cache->band_info);

  jrd_nw_info_chg_ind(E_JRD_NETWORK_GET_MCC_IND);
  if((old_regist_state != p_jrd_nw_local_cache->sys_info.regist_status)\
      ||(old_nw_type != p_jrd_nw_local_cache->sys_info.nw_type)\
      || (old_roaming != p_jrd_nw_local_cache->sys_info.roam)\
      || old_ps_attach != p_jrd_nw_local_cache->sys_info.ps_attach)
  {
     jrd_nw_info_chg_ind(E_JRD_NETWORK_INFO_CHG_IND);
  }
  if(old_sig_level != p_jrd_nw_local_cache->rf_info.sig_level)
  {
     jrd_nw_info_chg_ind(E_JRD_NETWORK_SIGNAL_IND);
  }
  
  return 0;
}

/*===========================================================================

FUNCTION jrd_nw_init_param

DESCRIPTION
  Init local cache.

DEPENDENCIES
  Call the api at the beginning of network thread..

RETURN VALUE
    0 -- success
  -1 -- failed

SIDE EFFECTS
  This api will send many qmi msgs, do not call it for too much times.
===========================================================================*/
int jrd_nw_init_param(void)
{
  jrd_oem_nw_cache_t* p_jrd_nw_local_cache = jrd_oem_nw_get_cache();

  JRD_MEMSET(p_jrd_nw_local_cache, 0, sizeof(jrd_oem_nw_cache_t));
  p_jrd_nw_local_cache->band_info.active_band = E_NW_NAS_ACTIVE_BAND_INVALID;
  p_jrd_nw_local_cache->nw_settings_info.band_pref = E_NW_BAND_INVALID;
  p_jrd_nw_local_cache->nw_settings_info.nw_mode = E_NETWORK_MODE_INVALID;
  p_jrd_nw_local_cache->nw_settings_info.sel_mode = E_NW_SELECT_MODE_INVALID;
  p_jrd_nw_local_cache->rf_info.sig_level = E_SIG_LEVEL_INVALID;
  p_jrd_nw_local_cache->sys_info.nw_type = E_NW_TYPE_INVALID;
  p_jrd_nw_local_cache->sys_info.roam = E_NW_ROAM_OFF;
  p_jrd_nw_local_cache->sys_info.regist_status = E_NW_REGIST_INVALID;
  p_jrd_nw_local_cache->manual_regist_info.regist_id = -1;
  p_jrd_nw_local_cache->manual_regist_info.regist_timer_status = E_NW_NOT_START_T;
  p_jrd_nw_local_cache->rf_info.ecio = 0;
  p_jrd_nw_local_cache->rf_info.sinr = NW_INVALID_VALUE_U8;
  p_jrd_nw_local_cache->rf_info.rsrp = NW_INVALID_VALUE;
  p_jrd_nw_local_cache->cell_info.umts_info.rscp = NW_INVALID_VALUE;
  p_jrd_nw_local_cache->mtu_value.ehrpd_mtu = 0;
  p_jrd_nw_local_cache->mtu_value.lte_mtu= 0;
  p_jrd_nw_local_cache->mtu_value.hrpd_mtu= 0;
  p_jrd_nw_local_cache->regist_failed_to_restore = 0;
  p_jrd_nw_local_cache->selection_change_mode = 0;

  
  jrd_db_get_value("network", "show_roam_specify_net_plmn", \
                                              &jrd_oem_show_roam_specify_net_plmn, \
                                              E_PARAM_STR, sizeof(jrd_oem_show_roam_specify_net_plmn));
  jrd_db_get_value("network", "show_roam_specify_sim_plmn", \
                                              &jrd_oem_show_roam_specify_sim_plmn, \
                                              E_PARAM_STR, sizeof(jrd_oem_show_roam_specify_sim_plmn));


  jrd_db_get_value("network", "custom_specify_plmn_name", \
                                        &jrd_oem_specify_plmn_name, \
                                        E_PARAM_STR, sizeof(jrd_oem_specify_plmn_name));
  jrd_db_get_value("network", "custom_specify_plmn", \
                                        &jrd_oem_specify_plmn, \
                                        E_PARAM_STR, sizeof(jrd_oem_specify_plmn));
  jrd_db_get_value("network", "custom_sim_plmn", \
                                        &jrd_oem_sim_plmn, \
                                        E_PARAM_STR, sizeof(jrd_oem_sim_plmn));
  jrd_db_get_value("network", "custom_sim_plmn_name", \
                                        &jrd_oem_sim_plmn_name, \
                                        E_PARAM_STR, sizeof(jrd_oem_sim_plmn_name));
  
  jrd_db_get_value("network", "search_list_long_networkname", \
                                    &p_jrd_nw_local_cache->jrd_oem_search_list_same_longnetworkname, \
                                    E_PARAM_U8, sizeof(uint8));

  jrd_db_get_value("network", "run_tmo_ons_flag", \
                                        &jrd_oem_run_tmo_ons_flag, \
                                        E_PARAM_INT, sizeof(jrd_oem_run_tmo_ons_flag));

  jrd_db_get_value("network", "full_service_requested", \
                                        &jrd_oem_full_service_requested, \
                                        E_PARAM_ENUM, sizeof(jrd_oem_full_service_requested));

  jrd_db_get_value("network", "signal_level_support", &p_jrd_nw_local_cache->Support_sig_level, E_PARAM_BOOL, sizeof(boolean));
  jrd_db_get_value("network", "lte_signal_level", &p_jrd_nw_local_cache->jrd_oem_lte_signal_level, E_PARAM_STR, sizeof(p_jrd_nw_local_cache->jrd_oem_lte_signal_level));
  jrd_db_get_value("network", "umts_signal_level", &p_jrd_nw_local_cache->jrd_oem_umts_signal_level, E_PARAM_STR, sizeof(p_jrd_nw_local_cache->jrd_oem_umts_signal_level));
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_oem_umts_signal_level = %s\n", p_jrd_nw_local_cache->jrd_oem_lte_signal_level);  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_oem_lte_signal_level = %s\n", p_jrd_nw_local_cache->jrd_oem_umts_signal_level);  

  jrd_db_get_value("network", "registrestore", \
                                    &p_jrd_nw_local_cache->regist_failed_to_restore, \
                                    E_PARAM_U8, sizeof(uint8));
  
 JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Regist failed, set auto %d\n",
	                p_jrd_nw_local_cache->regist_failed_to_restore);
	
  jrd_db_get_value("network", "selection_change_mode", \
                                      &p_jrd_nw_local_cache->selection_change_mode, \
                                      E_PARAM_ENUM, sizeof(uint8));
    
  jrd_db_get_value("network", "known_plmn_rule", \
                                    &p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_rule, \
                                    E_PARAM_ENUM, sizeof(e_known_plmn_show_rule_t));

  jrd_db_get_value("network", "unknown_plmn_rule", \
                                    &p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.unknown_plmn_rule, \
                                    E_PARAM_ENUM, sizeof(e_unknown_plmn_show_rule_t));

  jrd_db_get_value("network", "show_spn_while_roam", \
                                    &p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_roaming_spn, \
                                    E_PARAM_BOOL, sizeof(boolean));

  jrd_db_get_value("network", "spec_plmn_spec_name", \
                                    &p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_spec_name, \
                                    E_PARAM_BOOL, sizeof(boolean));

  if(p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_spec_name)
  {
    jrd_db_get_value("network", "plmn", \
                                      &p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_plmn_name.plmn, \
                                      E_PARAM_U32, sizeof(uint32));
    jrd_db_get_value("network", "nw_name", \
                                      &p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_plmn_name.nw_name, \
                                      E_PARAM_STR, MAX_NW_NAME_LEN);
  }
  jrd_db_get_value("network", "show_roaming", \
                                    &p_jrd_nw_local_cache->jrd_network_db_config.nw_roam_rule, \
                                    E_PARAM_ENUM, sizeof(e_nw_roam_show_rule_t));

  jrd_db_get_value("network", "force_to_show_spn_name", \
                                    &p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.force_to_show_spn_name, \
                                    E_PARAM_BOOL, sizeof(boolean));
  jrd_db_get_value("network", "show_pnn_name_switch", \
	                                &p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_show_pnn_name, \
	                                E_PARAM_BOOL, sizeof(boolean));                                  

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Get nw name rule:%d,unknown_plmn_rule:%d, show roaming:%d nw_show_pnn_name:%d\n", \
                                     p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.show_rule, \
                                     p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_name_rule.unknown_plmn_rule, \
                                     p_jrd_nw_local_cache->jrd_network_db_config.nw_roam_rule,
                                     p_jrd_nw_local_cache->jrd_network_db_config.nw_name_show_config.nw_show_pnn_name);
  
  jrd_oem_nw_get_all_params();
  return 0;
}

