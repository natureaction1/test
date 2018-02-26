
#define JRD_LOG_MODULE_MASK (1 << 0) /*MODULE_MAIN*/

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include "stdio.h"
#include "jrd_oem_indication.h"


jrd_ind_list_info_type* jrd_ind_info_tbl[MODULE_MAX] = {NULL};

int jrd_oem_ind_notify(jrd_ind_data_type *ind, module_enum_type target_module)
{
  jrd_cmd_q_type* cmd_data_q = NULL;
  void *ind_data = NULL;
  jrd_ind_data_type *ind_tmp;
  
  if(!ind)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "ind=%d\n", ind);
    return -1;
  }
  if(ind->module_id <= MODULE_INVALID ||ind->module_id >= MODULE_MAX)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid indication module id: %d\n", ind->module_id);
    return -1;
  }

  if(ind->data_size && ind->ind_data)
  {
      JRD_MALLOC(ind->data_size, ind_data);
      if(ind_data == NULL)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Molloc ind_data failed src module: %d, , target module: %d, ind: %d\n", 
                                        ind->module_id, target_module, ind->ind_id);
        return -1;
        
      }
  }
  
  cmd_data_q = jrd_malloc_cmd_q_data(CMD_IND);
  if(!cmd_data_q)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, 
                                      "Invalid malloc_cmd_q_data failed, module ID: %d, ind ID: %d\n", 
                                      ind->module_id, ind->ind_id);
    if(ind_data)
  	{
  	  JRD_FREE(ind_data);
  	}
    return -1;
  }
  cmd_data_q->cmd_hdr.source_m_id = ind->module_id;
  cmd_data_q->cmd_hdr.dest_m_id = target_module;
  ind_tmp = (jrd_ind_data_type*)cmd_data_q->cmd_data;
  JRD_MEMCPY(ind_tmp, ind, sizeof(jrd_ind_data_type));
  if(ind->ind_data)
  {
    JRD_MEMCPY(ind_data, ind->ind_data, ind->data_size);
    ind_tmp->ind_data = ind_data;
  } 
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Ind notify src: %d, dest: %d, ind: %d\n", 
                                    cmd_data_q->cmd_hdr.source_m_id, cmd_data_q->cmd_hdr.dest_m_id, ind->ind_id);
  jrd_oem_send_cmd(cmd_data_q);

  return 0;
}

int jrd_oem_register_ind_cb
(
  jrd_ind_proc_info_type* p_proc_info, module_enum_type reg_module
 )
{
  jrd_ind_list_info_type * p_ind_list_info = NULL;
  jrd_ind_list_type*         ind_list_node = NULL;
  jrd_ind_list_type*         ind_list_node_tmp = NULL;

  if(p_proc_info->module_id >= MODULE_MAX || p_proc_info->module_id < MODULE_MIN)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid module ID: %d\n", p_proc_info->module_id);
    return JRD_FAIL;
  }
  if(reg_module >= MODULE_MAX || reg_module < MODULE_MIN)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid register module ID: %d\n", p_proc_info->module_id);
    return JRD_FAIL;
  }
  p_ind_list_info = jrd_ind_info_tbl[p_proc_info->module_id];
  if(!p_ind_list_info)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "No indication info for this module ID: %d\n", p_proc_info->module_id);
    return JRD_FAIL;
  }

  if(p_proc_info->ind_id >= p_ind_list_info->ind_count)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "No this indication (%d) exist  in this module (%d)\n", p_proc_info->ind_id, p_proc_info->module_id);
    return JRD_FAIL;
  }

  JRD_MALLOC(sizeof(jrd_ind_list_type), ind_list_node);
  if(!ind_list_node)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Malloc ind list memory failed, indication (%d) module (%d)\n", p_proc_info->ind_id, p_proc_info->module_id);
    return JRD_FAIL;
  }
  ind_list_node->next = NULL;
  ind_list_node->reg_module = reg_module;
  ind_list_node_tmp = p_ind_list_info->list[p_proc_info->ind_id];
  while(ind_list_node_tmp)
  {
/*这个while循环就是保证每次ind_list_node_tmp指向最后一个链表结点，
而最后一个链表
结点的ind_list_node_tmp->next必定是ind_list_node_tmp->next=NULL，break退出*/
    if(!ind_list_node_tmp->next)
    {
      break;
    }
    ind_list_node_tmp = ind_list_node_tmp->next;
  }
  if(!ind_list_node_tmp)
  {
    p_ind_list_info->list[ p_proc_info->ind_id] = ind_list_node;
  }
  else
  {   /*当一个module的某个indication需要发给多个module时
就形成了这个indication的一个module链表*/
    ind_list_node_tmp->next = ind_list_node;
  }
  
  return 0;
}

int jrd_oem_register_ind
(
jrd_ind_list_info_type * p_ind_list_info
)
{
  if(p_ind_list_info == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind_list_info pointer\n");
  }
  if(p_ind_list_info->module_id >= MODULE_MAX || p_ind_list_info->module_id < MODULE_MIN)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid module ID: %d\n", p_ind_list_info->module_id);
    return -1;
  }
  if(jrd_ind_info_tbl[p_ind_list_info->module_id])
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Module has been register, module ID: %d\n", p_ind_list_info->module_id);
    return -1;
  }
  /*这个就是注册indication，就这一条命令完成注册；*/
  jrd_ind_info_tbl[p_ind_list_info->module_id] = p_ind_list_info;
  
  return 0;
}

int jrd_oem_ind
(
jrd_ind_data_type*        ind_data
)
{
  jrd_ind_list_type *         p_ind_list = NULL;

  if(ind_data == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid ind_data pointer\n");
    return -1;
  }

  p_ind_list = jrd_ind_info_tbl[ind_data->module_id]->list[ind_data->ind_id];
  while(p_ind_list)
  {
    jrd_oem_ind_notify(ind_data, p_ind_list->reg_module);
    p_ind_list = p_ind_list->next;
  }

  return 0;
}

