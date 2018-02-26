#include "jrd_oem.h"
#include "jrd_oem_common.h"
#include "jrd_oem_indication.h"
#include "sock_srv.h"
#include "jrd_oem_soft_dog.h"
#include <sys/syscall.h> 

//#define jrd_cmn_mutex_type pthread_mutex_t
typedef pthread_mutex_t jrd_cmn_mutex_type;
#define jrd_cmn_lock( ptr )         pthread_mutex_lock((ptr))
#define jrd_cmn_unlock( ptr )       pthread_mutex_unlock((ptr))
#define jrd_cmn_lock_init( ptr )    pthread_mutex_init((ptr), NULL)

static jrd_cmn_mutex_type        cmn_mutex;
static jrd_thread_info_type* jrd_thread_info_tbl[MODULE_MAX] = {NULL};

static jrd_ind_list_type* jrd_sock_ind_list[E_MSG_IND_TYPE_MAX] = {NULL,};
static jrd_ind_list_info_type jrd_sock_ind_info = 
{
  jrd_sock_ind_list,
  MODULE_SOCK,
  E_MSG_IND_TYPE_MAX,
};

/*Begin weiqiang.lei add thread start flag from file defined 2014-10-14*/
#define JRD_THREAD_BASE_SIZE(obj)    (sizeof(obj) / sizeof(obj[0]))

/*0 is default the thread all enable*/
static jrd_thread_base_type jrd_thread_base_info[] = 
{
    {MODULE_MAIN,           "thread_main",              0,   JRD_KICKDOG_CYCLES_DEFAULT, {0}, },
    {MODULE_SOCK,          "thread_sock",               0,   JRD_KICKDOG_CYCLES_DEFAULT, {0}, },
    {MODULE_SMS,           "thread_sms",                0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_USIM,           "thread_usim",              0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_CONNECTION, "thread_connection",     0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_PROFILE,       "thread_profile",            0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_NETWORK,     "thread_network",         0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_WIFI,            "thread_wifi",               0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_ROUTER,                "thread_router",            0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_SYS,           "thread_sys",              0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_MISC,           "thread_misc",              0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_SDSHARE,      "thread_sdshare",         0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_DEBUG,         "thread_debug",            0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_CHG,            "thread_charger",          0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_UI,               "thread_ui",                 0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_KEY,             "thread_key",              0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_TIME,            "thread_time",             0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_INPUT_LISTEN, "thread_input_listen", 0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_LED,                "thread_led",            0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_FOTA,                "thread_fota",            0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_USAGE,                "thread_usage",            0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_SOFT_DOG,     "thread_softdog",       0,   JRD_KICKDOG_CYCLES_DEFAULT, {0},},
	{MODULE_LCD, "thread_lcd", 0, JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    {MODULE_PBM , "thread_pbm", 0, JRD_KICKDOG_CYCLES_DEFAULT, {0},},
    //Added by Tian Yiqing, Add cwmp module, Start.
#ifdef JRD_FEATURE_CWMP
    {MODULE_CWMP, "thread_cwmp", 0, JRD_KICKDOG_CYCLES_DEFAULT, {0},},
#endif
    //Added by Tian Yiqing, Add cwmp module, End.
};

/*0 is anble the thread, 1 is disable the thread*/
jrd_thread_base_type *jrd_thread_base_module_get(module_enum_type thread_id)
{
    int i = 0;
    
    for(i = 0;  i < JRD_THREAD_BASE_SIZE(jrd_thread_base_info); i++)
    {
        if(thread_id == jrd_thread_base_info[i].id)
        {
            return &jrd_thread_base_info[i];
        }
    }

    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter thread_id = %d\n", __func__, thread_id);
    return NULL;
}

jrd_thread_base_type *jrd_thread_base_name_get(const char *thread_name)
{
    int i = 0;
    
    for(i = 0;  i < JRD_THREAD_BASE_SIZE(jrd_thread_base_info); i++)
    {
        if(!JRD_STRNCMP(thread_name, jrd_thread_base_info[i].name, JRD_STRLEN(jrd_thread_base_info[i].name)))
        {
            return &jrd_thread_base_info[i];
        }
    }

    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter thread_name = %s\n", __func__, thread_name);
    return NULL;
}

boolean jrd_thread_judge_disable(module_enum_type thread_id)
{
    jrd_thread_base_type *thread_info = NULL;
    boolean jrd_res = FALSE;

    thread_info = jrd_thread_base_module_get(thread_id);
    if(NULL != thread_info)
    {
        /*Thread is disable, do not need to start*/
        if(TRUE == thread_info->disable_flag)
        {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "%s Thread is disable, do not need to start\n", thread_info->name);
            jrd_res = TRUE;
        }
        else
        {
            jrd_res = FALSE;
        }
    }
    else
    {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter thread_id = %d\n", __func__, thread_id);
        jrd_res = FALSE;
    }

    return jrd_res;
}
/*End weiqiang.lei add thread start flag from file defined 2014-10-14*/

/*Begin weiqiang.lei add autosleep feature 2015-01-31*/
static boolean jrd_autosleep_flag = 0;
boolean jrd_get_autosleep_flag(void)
{
    return jrd_autosleep_flag;
}

void jrd_set_autosleep_flag(boolean autosleep_flag)
{
    jrd_autosleep_flag = autosleep_flag;
}
/*End weiqiang.lei add autosleep feature 2015-01-31*/



int jrd_oem_register_thread_info(jrd_thread_info_type* p_thread_info)
{
  jrd_cmn_lock(&cmn_mutex);
  if(p_thread_info == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid thread info pointer\n");
    jrd_cmn_unlock(&cmn_mutex);
    return -1;
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_oem_register_thread_info module: %d, info: 0x%08X\n", p_thread_info->module_id, p_thread_info);
  if(p_thread_info->module_id >= MODULE_MAX || p_thread_info->module_id < MODULE_MIN)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid module ID: %d\n", p_thread_info->module_id);
    jrd_cmn_unlock(&cmn_mutex);
    return -1;
  }
  if(p_thread_info->p_module_q == NULL || p_thread_info->p_os_sig == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid queue pointer or signal pointer, module ID: %d\n", p_thread_info->module_id);
    jrd_cmn_unlock(&cmn_mutex);
    return -1;
  }

  if(jrd_thread_info_tbl[p_thread_info->module_id])
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Module has been register, module ID: %d\n", p_thread_info->module_id);
    jrd_cmn_unlock(&cmn_mutex);
    return -1;
  }

  jrd_thread_info_tbl[p_thread_info->module_id] = p_thread_info;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_oem_register_thread_info module rehistered: %d, info: 0x%08X\n", p_thread_info->module_id, jrd_thread_info_tbl[p_thread_info->module_id]);
  jrd_cmn_unlock(&cmn_mutex);
  
  return 0;
}



jrd_thread_info_type* jrd_oem_get_thread_info(module_enum_type module_id)
{
  //JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_oem_get_thread_info thread_info: 0x%08X, module ID: %d\n", jrd_thread_info_tbl[module_id], module_id);

  if(module_id >= MODULE_MAX || module_id < MODULE_MIN)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid module ID: %d\n", module_id);
    return NULL;
  }
  if(!jrd_thread_info_tbl[module_id])
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_get_thread_info thread_info: 0x%08X, module ID: %d\n", jrd_thread_info_tbl[module_id], module_id);
  }
  return jrd_thread_info_tbl[module_id];
}


 /*jrd_oem_send_cmd(cmd_q_data);*/
jrd_cmd_q_type* jrd_oem_send_cmd(jrd_cmd_q_type *cmd_ptr)
{
  jrd_cmd_hdr_type*  cmd_hdr = NULL;
  q_type*                  target_cmd_q = NULL;
  jrd_thread_info_type*  target_thread_info = NULL;
  os_signal_type*           os_sig_info = NULL;
  jrd_cmd_q_type*         cmd_q_head = NULL;
  
  if(!cmd_ptr)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid CMD queue data\n");
    return NULL;
  }
  cmd_hdr = &(cmd_ptr->cmd_hdr);
  if(cmd_hdr->type <= CMD_INVALID ||cmd_hdr->type >= CMD_MAX)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid CMD type: %d\n", cmd_hdr->type);
    return NULL;
  }
  if(cmd_hdr->dest_m_id <= MODULE_INVALID ||cmd_hdr->dest_m_id >= MODULE_MAX)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid CMD dest_m_id: %d\n", cmd_hdr->dest_m_id);
    return NULL;
  }
  target_thread_info = jrd_thread_info_tbl[cmd_hdr->dest_m_id];
  
  if(!target_thread_info)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "***************module %d not start !!!!!!!!!!!!!!!\n", cmd_hdr->dest_m_id);
    if(CMD_IND != cmd_ptr->cmd_hdr.type)
    {
      jrd_cmd_data_type *cmd_data = NULL;
      cmd_data = (jrd_cmd_data_type*)cmd_ptr->cmd_data;
      cmd_data->error_code = 111111111;
      if(cmd_data->jrd_cmd_cb)
        cmd_data->jrd_cmd_cb(cmd_ptr, cmd_data->user_data);
      else
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_cmd_cb is NULL!!!\n");
      }
    }
    else
    {
      jrd_free_cmd_q_data(cmd_ptr);
    }
    return NULL;
  }
  target_cmd_q = target_thread_info->p_module_q;

 /* 初始化队列链表结点 cmd_ptr->link   */
  q_link(cmd_ptr, &cmd_ptr->link);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_oem_send_cmd src: %d, dest: %d, type: %d, cmd_q 0x%08X, q_data: 0x%08X\n", 
                                    cmd_hdr->source_m_id, cmd_hdr->dest_m_id, cmd_hdr->type, target_cmd_q, cmd_ptr);
  if(cmd_hdr->type == CMD_IND)
  {
    #if 0
    cmd_q_head = (jrd_cmd_q_type*)q_check(target_cmd_q);
    if(cmd_q_head)
    {
      q_insert(target_cmd_q, &cmd_ptr->link, &cmd_q_head->link);
    }
    else
    {
      q_put(target_cmd_q, &cmd_ptr->link);
    }
    #else
    q_put(target_cmd_q, &cmd_ptr->link);
  #endif
  }
  else
  {
   /* 把 队列链表结点 cmd_ptr->link  插入到队列target_cmd_q 中 */
    q_put(target_cmd_q, &cmd_ptr->link);
  }
  JRD_OS_SIGNAL_SET(target_thread_info->p_os_sig);
  
  return NULL;
}

jrd_cmd_q_type* jrd_malloc_cmd_q_data(cmd_enum_type cmd_type)
{
  void* cmd_data = NULL;
  jrd_cmd_q_type* cmd_data_q = NULL;
  jrd_cmd_hdr_type*  cmd_hdr = NULL;

  switch (cmd_type)
  {
    case CMD_WEB_REQ:
    case CMD_RSP_WEB:
    case CMD_UI_REQ:
    case CMD_RSP_UI:
    case CMD_NORMAL_REQ:
    case CMD_RSP_NORMAL:
      JRD_MALLOC(sizeof(jrd_cmd_data_type), cmd_data);
    break;
    case CMD_IND:
      JRD_MALLOC(sizeof(jrd_ind_data_type), cmd_data);
    break;
    default:
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid CMD type: %d\n", cmd_type);
    break;
  }
  
  if(cmd_data == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Malloc CMD data failed\n");
    return NULL;
  }
  
  JRD_MALLOC(sizeof(jrd_cmd_q_type), cmd_data_q);
  if(cmd_data_q == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Malloc CMD data queue failed\n");
    JRD_FREE(cmd_data);
    return NULL;
  }
  cmd_hdr = &(cmd_data_q->cmd_hdr);
  cmd_hdr->type = cmd_type;
  cmd_hdr->act_id = JRD_INVALID_ACT_ID;
  cmd_data_q->cmd_data = cmd_data;

  return cmd_data_q;
}

void jrd_free_cmd_q_data(jrd_cmd_q_type* cmd_data_q)
{
  jrd_cmd_data_type* cmd_data = NULL;
  jrd_ind_data_type*   ind_data = NULL;
  jrd_cmd_hdr_type*  cmd_hdr = NULL;

  if(cmd_data_q == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_free_cmd_q_data invalid queue data\n");
    return;
  }
  cmd_hdr = &(cmd_data_q->cmd_hdr);
  switch (cmd_hdr->type)
  {
    case CMD_WEB_REQ:
    case CMD_RSP_WEB:
    case CMD_UI_REQ:
    case CMD_RSP_UI:
    case CMD_NORMAL_REQ:
    case CMD_RSP_NORMAL:
      cmd_data = (jrd_cmd_data_type*)cmd_data_q->cmd_data;
      if(cmd_data != NULL)
      {
        if(cmd_data->req_buf)
        {
          JRD_FREE(cmd_data->req_buf);
        }
        if(cmd_data->rsp_buf)
        {
          JRD_FREE(cmd_data->rsp_buf);
        }
        JRD_FREE(cmd_data);
      }
    break;
    case CMD_IND:
      ind_data = (jrd_ind_data_type*)cmd_data_q->cmd_data;
      if(ind_data != NULL)
      {
        if(ind_data->ind_data)
        {
          JRD_FREE(ind_data->ind_data);
        }
        JRD_FREE(ind_data);
      }
    break;
    default:
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid CMD type: %d\n", cmd_hdr->type);
    break;
  }
  
  JRD_FREE(cmd_data_q);
  
  return;
}

void jrd_common_init(void)
{
  jrd_cmn_lock_init(&cmn_mutex);
}

// Connie add start for json parser, 2014/8/12
#define JRD_OEM_MAX_PARAM_NAME_LENGTH 32
static boolean jrd_oem_is_valid_object_type(json_object * object_param, e_jrd_paramtype_type  param_type)
{
  json_type object_type =  json_object_get_type(object_param);
  switch (param_type)
  {
    case E_PARAM_U8:
    case E_PARAM_U16:
    case E_PARAM_INT:
    case E_PARAM_U32:
    case E_PARAM_ENUM:
    {
      if (object_type == json_type_int)
        return TRUE;
      else
        return FALSE;
    }
    case E_PARAM_U64:
    case E_PARAM_I64:
    {
      if (object_type == json_type_int64)
        return TRUE;
      else
        return FALSE;
    }
    case E_PARAM_NULL:
    {
      if (object_type == json_type_null)
        return TRUE;
      else
        return FALSE;
    }
    case E_PARAM_STR:
    {
      if (object_type == json_type_string)
        return TRUE;
      else
        return FALSE;
    }
    default:
      return FALSE;
  }
}

// Connie add start, 2014/12/27
static int jrd_oem_add_object
(
  json_object*         target_object,
  uint16               module_id,
  uint16               param_id,
  e_jrd_sock_msg_type  msg_type, 
  void                *param_value
)
{
  json_object*  result_object = NULL;
  char          param_name[JRD_OEM_MAX_PARAM_NAME_LENGTH];
  uint16        param_type;
  int           rc = -1;

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "msg_type: %d, module_id: %d, param_id: %d\n", msg_type, module_id, param_id);
  if (msg_type == E_JSON_MSG_REQ)
    rc = jrd_oem_get_json_param_info_from_id(module_id, param_id, &param_type, param_name, JRD_OEM_MAX_PARAM_NAME_LENGTH);
  else if (msg_type == E_TN_MSG_REQ)
    rc = jrd_oem_tn_get_param_info_from_id(module_id, param_id, &param_type, param_name, JRD_OEM_MAX_PARAM_NAME_LENGTH);
  else
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid msg_type %d\n",msg_type);
  
  if (rc != 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get param info, module_id: %d, param_id: %d\n", module_id, param_id);
    return -1;
  }

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "param_name %s, param_type %d\n",param_name, param_type);  
  switch (param_type)
  {
    case E_PARAM_U8:
      json_object_object_add(target_object, param_name, json_object_new_int(*(uint8 *)param_value));
      break;
    case E_PARAM_INT:
      json_object_object_add(target_object, param_name, json_object_new_int(*(int *)param_value));
      break;
    case E_PARAM_U16:
      json_object_object_add(target_object, param_name, json_object_new_int(*(uint16 *)param_value));
      break;
    case E_PARAM_U32:
      json_object_object_add(target_object, param_name, json_object_new_int(*(uint32 *)param_value));
      break;
    case E_PARAM_I64:
      json_object_object_add(target_object, param_name, json_object_new_int64(*(int64*)param_value));
      break;
    case E_PARAM_U64:
      json_object_object_add(target_object, param_name, json_object_new_int64(*(uint64*)param_value));
      break;
    case E_PARAM_STR:
      if (param_value)
        json_object_object_add(target_object, param_name, json_object_new_string((char *)param_value));
      else
        json_object_object_add(target_object, param_name, json_object_new_string(""));
      break;
    case E_PARAM_F32:
      json_object_object_add(target_object, param_name, json_object_new_double(*(float*)param_value));
      break;
    case E_PARAM_ENUM:
    {
      uint8 type;
      int   web_data;
      if (msg_type == E_TN_MSG_REQ)
        rc = jrd_oem_tn_get_web_data_from_enum(module_id, param_id,(*(int *)param_value), &type, &web_data);
      else
        rc = jrd_oem_json_get_web_data_from_enum(module_id, param_id,(*(int *)param_value), &type, &web_data);
      if (rc == 0)
      {
        if (type == PARAM_INT)
          json_object_object_add(target_object, param_name, json_object_new_int(web_data));
        else if (type == PARAM_STR)
        {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "web_data %s\n",(char *)web_data);
          json_object_object_add(target_object, param_name, json_object_new_string((char *)web_data));
        }
      }
      else
      {
        json_object_object_add(target_object, param_name, json_object_new_int(*(int *)param_value));
      }
      break;
    }
    case E_PARAM_OBJECT:
      json_object_object_add(target_object, param_name, (json_object *)param_value);
      break;
    case E_PARAM_OBJECT_DATA:
    {
      int                    i;
      json_object_info_type *object_info = (json_object_info_type *)param_value;
      json_object           *value_object = json_object_new_object();
      
      if (value_object == NULL)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Create value_object fail!\n");
        return -1;
      }
      for (i=0; i<object_info->NameValuePairs_count; i++)
      {
        jrd_oem_add_object( value_object, 
                            module_id, 
                            object_info->NameValuePairs_data[i].param_id, 
                            msg_type, 
                            object_info->NameValuePairs_data[i].param_data);
      }
      json_object_object_add(target_object, param_name, value_object);
      break;
    }
    case E_PARAM_ARRAY_DATA:
    {
      int                    i;
      json_array_info_type  *array_info = (json_array_info_type *)param_value;
      json_object           *value_array = json_object_new_array();
      json_object           *array_member;
      
      if (value_array == NULL)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Create value_array fail!\n");
        return -1;
      }
      for (i=0; i<array_info->member_count; i++)
      {
        switch (array_info->member_type)
        {
          case E_PARAM_STR:
          {
            array_member = json_object_new_string(((char**)array_info->member_data)[i]);
            break;
          }
          case E_PARAM_INT:
          {
            array_member = json_object_new_int(((int*)array_info->member_data)[i]);
            break;
          }
          case E_PARAM_OBJECT_DATA:
          {
            int                    j;
            json_object_info_type object_info = ((json_object_info_type *)array_info->member_data)[i];
            array_member = json_object_new_object();
            if (array_member == NULL)
            {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Create array_member fail!\n");
              return -1;
            }
            
            for (j=0; j<object_info.NameValuePairs_count; j++)
            {
              jrd_oem_add_object( array_member, 
                                  module_id, 
                                  object_info.NameValuePairs_data[j].param_id, 
                                  msg_type, 
                                  object_info.NameValuePairs_data[j].param_data);
            }
            break;
          }
          default:
          {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error! not support this array member type now. member_type %d\n", array_info->member_type);
            json_object_put(value_array);
            return -1;
          }
        }
        json_object_array_add(value_array, array_member);
      }
      json_object_object_add(target_object, param_name, value_array);
      break;
    }
    default:
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "param type error!!! param_type: %d\n", param_type);
      return -1;
  }
  return 0;
}
// Connie add end, 2014/12/27
// Connie add start, 2015/1/4
e_jrd_sock_msg_type jrd_oem_get_msg_type(jrd_cmd_data_type *cmd_data)
{
  e_jrd_sock_msg_type msg_type = E_JSON_MSG_REQ;
  if (cmd_data != NULL)
  {
    msg_type = ((web_data_q_type*)(cmd_data->user_data))->oem_web_data->jrd_sock_hdr.msg_type;
  }
  return msg_type;
}
// Connie add end, 2015/1/4

int jrd_oem_add_param_to_resp_object
(
  jrd_cmd_data_type *cmd_data,
  uint16       module_id,
  uint16       param_id, 
  void        *param_value
)
{
  e_jrd_sock_msg_type msg_type = jrd_oem_get_msg_type(cmd_data);

  if (cmd_data->object_rsp == NULL)
    cmd_data->object_rsp = json_object_new_object();
  
  if (cmd_data->object_rsp == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Create resp_object fail!\n");
    return -1;
  }

  jrd_oem_add_object(cmd_data->object_rsp, module_id, param_id, msg_type, param_value);
  return 0;
}

int jrd_oem_get_param_from_req_object
(
    jrd_cmd_data_type *cmd_data,      // in
    uint16             module_id,     // in
    uint16             param_id,      // in
    char             **str_param_value,// string out
    void              *num_param_value,// number out
    uint8              param_index    // int, first param index is 0
)
{
    char          param_name[JRD_OEM_MAX_PARAM_NAME_LENGTH];
    uint16        param_type;
    json_object * object_this_param = NULL;
    boolean       is_array = FALSE;
    int           rc = -1;
    e_jrd_sock_msg_type msg_type;

    if (!cmd_data || (!num_param_value&&!str_param_value))
      return -1;
    
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "req_object: %s\n", json_object_get_string(cmd_data->object_req));
    // Get method
    msg_type = jrd_oem_get_msg_type(cmd_data);

    if (msg_type == E_JSON_MSG_REQ)
      rc = jrd_oem_get_json_param_info_from_id(module_id, param_id, &param_type, param_name, JRD_OEM_MAX_PARAM_NAME_LENGTH);
    else if (msg_type == E_TN_MSG_REQ)
      rc = jrd_oem_tn_get_param_info_from_id(module_id, param_id, &param_type, param_name, JRD_OEM_MAX_PARAM_NAME_LENGTH);
    else
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid msg_type %d\n",msg_type);
    
    if (rc != 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get param info, msg_type: %d, module_id: %d, param_id: %d\n", msg_type,module_id, param_id);
      return -1;
    }
    
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "param_name: %s, param_type: %d\n", param_name, param_type);
    object_this_param = json_object_object_get(cmd_data->object_req, param_name);
    if (object_this_param == NULL)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get object for param %s\n", param_name);
      return -1;
    }
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "object_this_param type: %d\n", json_object_get_type(object_this_param));

    if (json_object_is_type(object_this_param, json_type_array))
    {
      is_array = TRUE;
      if (param_index >= json_object_array_length(object_this_param))
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "param_index is out of range, param_index: %d, array_length: %d\n", param_index, json_object_array_length(object_this_param));
        return -1;
      }
      object_this_param = json_object_array_get_idx(object_this_param, param_index);
      if (object_this_param == NULL)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get object for param %s\n", param_name);
        return -1;
      }
    }

    if (param_index > 0 && is_array == FALSE)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "This param is not an array %s\n", param_name);
      return -1;
    }
    //if(jrd_oem_is_valid_object_type(object_this_param, param_type)== FALSE) // json_type_array will return -1
    //{
    //  return -1;
    //}

    if (msg_type == E_TN_MSG_REQ)
    {
      if(E_PARAM_STR == param_type)
      {
        if(str_param_value == NULL)
        {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "str_param_value pointer is NULL!\n");
          return -1;
        }
      }
      else if(E_PARAM_ENUM != param_type)
      {
        if(num_param_value == NULL)
        {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "num_param_value pointer is NULL!\n");
          return -1;
        }
      }
      if (json_object_get_type(object_this_param) != json_type_string)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "param must be string for tn %s\n", param_name);
        return -1;
      }
      else
      {
        char *value = (char*)json_object_get_string(object_this_param);
        int    int_value = 0;
        int64  i64_value = 0;
        uint64 u64_value = 0;
        float  f32_value = 0;
        switch(param_type)
        {
          case E_PARAM_U8:
          case E_PARAM_U16:
          case E_PARAM_U32:
          case E_PARAM_INT:
            int_value = (int)JRD_ATOI(value);  
            *(int*)num_param_value = int_value;
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "E_PARAM_U16 |E_PARAM_INT value: %d\n", int_value);
          break;
          case E_PARAM_I64:
          {
            JRD_SSCANF(value, "%lld", &i64_value);
            *(int64*)num_param_value = i64_value;
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "E_PARAM_I64 |E_PARAM_INT value: %lld\n", i64_value);
            break;
          }
          case E_PARAM_U64:
          {
            JRD_SSCANF(value, "%llu", &u64_value);
            *(uint64*)num_param_value = u64_value;
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "E_PARAM_U64 |E_PARAM_INT value: %llu\n", u64_value);
            break;
          }
          case E_PARAM_STR: 
            *str_param_value = value;
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "E_PARAM_STR value: %s\n", value);
          break;
          case E_PARAM_F32:
            JRD_SSCANF(value, "%f", &f32_value);
            *(float*)num_param_value = f32_value;
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "E_PARAM_F32 value: %f\n", f32_value);
          break;
          // Connie add start, 2014/12/26
          case E_PARAM_ENUM:
          {
            int rc;
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "E_PARAM_ENUM value: %s\n", value);
            rc = jrd_oem_tn_get_enum_from_web_data(module_id, param_id, value, &int_value);
            if (rc != 0)
            {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error: not found in enum table, param_name:%s, value:%s\n", param_name, value);
              if(str_param_value == NULL)
              {
                JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "str_param_value pointer is NULL!\n");
                return -1;
              }
              *str_param_value = value;
            }
            else
            {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "local_enum_value: %d\n", int_value);
              if(num_param_value == NULL)
              {
                JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "num_param_value pointer is NULL!\n");
                return -1;
              }
              *(int*)num_param_value = int_value;
            }
            break;
          }
          // Connie add end, 2014/12/26
          default:
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "invalid param_type %d\n", param_type);
            return -1;
          break;
        }
      }
    }
    else
    {
      json_type json_pram_type = json_object_get_type(object_this_param);
      if(json_type_string != json_pram_type)
      {
        if(num_param_value == NULL)
        {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "num_param_value pointer is NULL!\n");
          return -1;
        }
      }
      else if(E_PARAM_ENUM != param_type)
      {
        if(str_param_value == NULL)
        {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "str_param_value pointer is NULL!\n");
          return -1;
        }
      }
      switch (json_pram_type)
      {
        case json_type_int:
        {
          if (param_type == E_PARAM_ENUM)
          {
            int local_enum;
            int web_value = json_object_get_int(object_this_param);
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "web_value: %d\n", web_value);
            rc = jrd_oem_json_get_enum_from_web_data(module_id, param_id, &web_value, &local_enum);
            if (rc != 0)
            {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error: not found in enum table, param_name:%s, web_value:%d\n", param_name, web_value);
              *(int*)num_param_value = web_value;
            }
            else
            {
              *(int*)num_param_value = local_enum;
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "local_enum_value: %d\n", local_enum);
            }
          }
          else
            *(int*)num_param_value = json_object_get_int(object_this_param);
          break;
        }      
        case json_type_int64:
        {
          *(int64*)num_param_value = json_object_get_int64(object_this_param);
          break;
        }      
        case json_type_string:
        {
          if (param_type == E_PARAM_ENUM)
          {
            int local_enum;
            char *web_value = (char *)json_object_get_string(object_this_param);
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "web_value: %s\n", web_value);
            rc = jrd_oem_json_get_enum_from_web_data(module_id, param_id, &web_value, &local_enum);
            if (rc != 0)
            {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error: not found in enum table, param_name:%s, web_value:%s\n", param_name, web_value);
              if(str_param_value == NULL)
              {
                JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "str_param_value pointer is NULL!\n");
                return -1;
              }
              *str_param_value = web_value;
            }
            else
            {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "local_enum_value: %d\n", local_enum);
              if(num_param_value == NULL)
              {
                JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "num_param_value pointer is NULL!\n");
                return -1;
              }
              *(int*)num_param_value = local_enum;
            }
          }
          else
            *str_param_value = (char *)json_object_get_string(object_this_param);
          break;
        }
        case json_type_object:
        case json_type_array:
	    {
            *(json_object**)num_param_value = object_this_param;
        }
	  break;	

        default:
        {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "invalid param_type: %d\n", json_pram_type);
          return -1;
        }
      }
    }
    return 0;
}
// Connie add end for json parser, 2014/8/12

int jrd_oem_tn_web_response_cmd_send
(
 int fd,
 msg_head_t msg_head,
 jrd_sock_hdr_t* jrd_sock_hdr_ptr, 
 uint8* format_data, 
 uint32 format_data_len
)
{
  uint32 max_data_len = JRD_SOCK_CORE_APP_TX_BUF_SIZE - sizeof(jrd_sock_hdr_t) - 2*sizeof(int);
  uint8  temp_buf[max_data_len];
  uint8 *pdata = format_data;
  uint32 copy_len;
  uint32 remain_len = format_data_len;
  pack_head_t pack_head = {0};
  pack_head.pack_head = PACK_HEAD;

  if(fd == JRD_SOCK_INVALID_FD)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid sock fd: %d, should hang up the program\n", fd);
    //Error handler here, should hang up here
    return -1;
  }
  if(!jrd_sock_hdr_ptr)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid jrd_sock_hdr_ptr: 0x%08X, should hang up the program\n", 
                                      jrd_sock_hdr_ptr);
    //Error handler here, should hang up here
    return -1;
  }
  jrd_sock_hdr_ptr->content_len = format_data_len; // Connie add, 2014/9/8
  jrd_sock_hdr_ptr->data_start = 1;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "send multy times start\n");
  while(remain_len)
  {
    if (remain_len > max_data_len -1)
      copy_len = max_data_len-1;
    else
      copy_len = remain_len;
    JRD_MEMSET(temp_buf, 0, max_data_len);
    JRD_MEMCPY(temp_buf, pdata, copy_len);
    pdata += copy_len;
    remain_len -= copy_len;
    
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "copy_len = %d\n", copy_len);
    if (remain_len)
    {
      jrd_sock_hdr_ptr->data_complete = 0;
      msg_head.msg_not_finish = TRUE;
    }
    else
    {
      jrd_sock_hdr_ptr->data_complete = 1;
      msg_head.msg_not_finish = FALSE;
    }

    {
      jrd_thread_info_type* jrd_sock_tx_thread_info = NULL;
      jrd_sock_cmd_q_type* sock_cmd_q = NULL;
      uint8* data_ptr = NULL;
      
      JRD_MALLOC(sizeof(jrd_sock_cmd_q_type), sock_cmd_q);
      if(!sock_cmd_q)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "JRD_MALLOC sock_cmd_q failed, should hang up the program\n");
        //Error handler here, should hang up here
        return -1;
      }
      sock_cmd_q->fd = fd;
      
      sock_cmd_q->data_len = copy_len+1 + sizeof(jrd_sock_hdr_t) + sizeof(pack_head) + sizeof(msg_head);
      JRD_MALLOC(sock_cmd_q->data_len, sock_cmd_q->data_buf);
      if(!sock_cmd_q->data_buf)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "JRD_MALLOC sock_cmd_q->data_buf failed, should hang up the program\n");
        //Error handler here, should hang up here
        JRD_FREE(sock_cmd_q);
        return -1;
      }
      data_ptr = sock_cmd_q->data_buf;
      //JRD_MEMCPY(data_ptr, &pack_head, sizeof(int));//add pack head
      //data_ptr += sizeof(int);
      data_ptr += sizeof(pack_head);//reserv for pack head
      JRD_MEMCPY(data_ptr, &msg_head, sizeof(msg_head_t));
      data_ptr += sizeof(msg_head_t);//add msg info
      JRD_MEMCPY(data_ptr, jrd_sock_hdr_ptr, sizeof(jrd_sock_hdr_t));
      data_ptr += sizeof(jrd_sock_hdr_t);
      
      if(temp_buf)
      {
        gsprintf(data_ptr, "%s", temp_buf);
      }
      pack_head.pack_len = sock_cmd_q->data_len = sizeof(jrd_sock_hdr_t) + gstrlen(data_ptr) + 1 + sizeof(pack_head) + sizeof(msg_head);
      JRD_MEMCPY(sock_cmd_q->data_buf, &pack_head, sizeof(pack_head));//add pack head to buff
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "reponse len: %d, response:\n%s\n", sock_cmd_q->data_len, data_ptr);
      jrd_sock_tx_thread_info = jrd_oem_get_thread_info(MODULE_SOCK);
      if(!jrd_sock_tx_thread_info)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid jrd_sock_tx_thread_info pointer, should hang up the program\n");
        JRD_FREE(sock_cmd_q->data_buf);
        JRD_FREE(sock_cmd_q);
        return -1;
      }

      q_link(sock_cmd_q, &sock_cmd_q->link);
      q_put(jrd_sock_tx_thread_info->p_module_q, &sock_cmd_q->link);
      JRD_OS_SIGNAL_SET(jrd_sock_tx_thread_info->p_os_sig);
    }
    jrd_sock_hdr_ptr->data_start = 0;
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "send multy times end\n");
  
  return 0;
}
void jrd_oem_keyword_decode(char_t *decoded, char_t *token, int len)
{
	char_t	*ip,  *op;
	int		num, i, c;

	a_assert(decoded);
	a_assert(token);

	op = decoded;
	for (ip = token; *ip && len > 0; ip++, op++) {
		if (*ip == '+') {
			*op = ' ';
		} else if (*ip == '%' && gisxdigit(ip[1]) && gisxdigit(ip[2])) {

/*
 *			Convert %nn to a single character
 */
			ip++;
			for (i = 0, num = 0; i < 2; i++, ip++) {
				c = tolower(*ip);
				if (c >= 'a' && c <= 'f') {
					num = (num * 16) + 10 + c - 'a';
				} else {
					num = (num * 16) + c - '0';
				}
			}
			*op = (char_t) num;
			ip--;

		} else {
			*op = *ip;
		}
		len--;
	}
	*op = '\0';
}

char jrd_itoa(uint8 digit)
{  
  if (digit>15)
    return ' ';

  if (digit<=9)
    return ('0'+digit);
  if (digit >9 && digit <= 15)
    return ('a'+digit-10);
}
void jrd_oem_imsi_decode(uint8 *imsi_buf, char *imsi_str, int imsi_str_len)
{
  int imsi_len;
  char *data_ptr;
  int i;
  
  if (imsi_buf == NULL || imsi_str == NULL || imsi_str_len == 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid CMD type: %d\n");
    return;
  }

  imsi_len = imsi_buf[0];
  
  if ( imsi_len == 0 )
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "imsi_str_len is 0,NULL\n");
    return;
  }

  if (imsi_str_len <= imsi_len*2 -1)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "imsi_str_len too short\n");
    return;
  }

  JRD_MEMSET(imsi_str, 0, imsi_str_len);
  data_ptr = imsi_str;
  *data_ptr++ = jrd_itoa(imsi_buf[1]>>4 & 0xF);
  
  
  for(i = 2; i<=imsi_len; i++)
  {
    *data_ptr = jrd_itoa(imsi_buf[i] & 0xF);
    if (*data_ptr != 'f')
      data_ptr++;
    *data_ptr = jrd_itoa(imsi_buf[i]>>4 & 0xF);
    if (*data_ptr != 'f')
      data_ptr++;
  }
}
void jrd_oem_iccid_decode(uint8 *iccid_buf, char *iccid_str, int iccid_str_len)
{
  int iccid_len = 10;
  char *data_ptr;
  int i;
  
  if (iccid_buf == NULL || iccid_str == NULL || iccid_str_len == 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid CMD type: %d\n");
    return;
  }

  if (iccid_str_len <= iccid_len*2)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "iccid_str_len too short\n");
    return;
  }

  JRD_MEMSET(iccid_str, 0, iccid_str_len);
  data_ptr = iccid_str;
  
  
  for(i = 0; i<iccid_len; i++)
  {
    *data_ptr = jrd_itoa(iccid_buf[i] & 0xF);
    if (*data_ptr != 'f')
      data_ptr++;
    *data_ptr = jrd_itoa(iccid_buf[i]>>4 & 0xF);
    if (*data_ptr != 'f')
      data_ptr++;
  }
}
int jrd_printf(char* sprint_buf, int size, char *fmt, ...)
{
  va_list args;
  int n;
  
  va_start(args, fmt);
  n = vsnprintf(sprint_buf, size, fmt, args);
  va_end(args);
  
  return n;
}

int jrd_prinf_mem_info(void)
{
  long page_size;
  long free_pages;
  long long free_mem;
  
  page_size = sysconf (_SC_PAGESIZE);
  //printf ("page size: %ld K\n", page_size / 1024 );
  free_pages = sysconf (_SC_AVPHYS_PAGES);
  //printf ("free pages: %ld \n", free_pages);
  free_mem = (long long)free_pages * (long long)page_size;
  printf("*****************************************************************************************************************free: %lld bytes\n", free_mem);
}

void print_module_id(void)
{
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_MAIN          = %d\n",   MODULE_MAIN);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_SOCK          = %d\n",   MODULE_SOCK);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_SMS           = %d\n",   MODULE_SMS);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_USIM          = %d\n",   MODULE_USIM);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_CONNECTION    = %d\n",   MODULE_CONNECTION);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_PROFILE       = %d\n",   MODULE_PROFILE);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_NETWORK       = %d\n",   MODULE_NETWORK);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_WIFI          = %d\n",   MODULE_WIFI);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_ROUTER        = %d\n",   MODULE_ROUTER);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_SYS           = %d\n",   MODULE_SYS);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_MISC          = %d\n",   MODULE_MISC);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_SDSHARE       = %d\n",   MODULE_SDSHARE);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_LCD           = %d\n",   MODULE_LCD); 
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_DEBUG         = %d\n",   MODULE_DEBUG);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_PBM           = %d\n",   MODULE_PBM);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_CHG           = %d\n",   MODULE_CHG);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_UI            = %d\n",   MODULE_UI);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_KEY           = %d\n",   MODULE_KEY);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_TIME          = %d\n",   MODULE_TIME);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_INPUT_LISTEN  = %d\n",   MODULE_INPUT_LISTEN);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_LED           = %d\n",   MODULE_LED);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_FOTA          = %d\n",   MODULE_FOTA);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_USAGE         = %d\n",   MODULE_USAGE);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_SOFT_DOG      = %d\n",   MODULE_SOFT_DOG);
     //Added by Tian Yiqing, Add cwmp module, Start.
#ifdef JRD_FEATURE_CWMP
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_CWMP          = %d\n",   MODULE_CWMP);
#endif
   //Added by Tian Yiqing, Add cwmp module, End.
}
// Connie add start, 2015/1/8
void jrd_oem_add_pointer(pointer_list_type **pointer_list_head_addr, void *new_pointer)
{
  pointer_list_type *new_pointer_node;
  
  if (pointer_list_head_addr == NULL || new_pointer == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Null pointer\n");
    return;
  }

  JRD_MALLOC(sizeof(pointer_list_type),new_pointer_node);
  if (new_pointer_node == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "JRD_MALLOC pointer_node fail\n");
    return;
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "JRD_MALLOC %p\n", new_pointer_node);    
  new_pointer_node->pointer = new_pointer;
  new_pointer_node->next = NULL;
  
  if (*pointer_list_head_addr == NULL)
  {
    *pointer_list_head_addr = new_pointer_node;
  }
  else
  {
    pointer_list_type *curr_pointer_node = *pointer_list_head_addr;
    while(curr_pointer_node->next)
    {
      curr_pointer_node = curr_pointer_node->next;
    }
    curr_pointer_node->next = new_pointer_node;
  }
  return;
}

void jrd_oem_free_pointer_list(pointer_list_type **pointer_list_head_addr)
{
  pointer_list_type *curr_pointer_node;
  pointer_list_type *next_pointer_node;
  if (pointer_list_head_addr == NULL || *pointer_list_head_addr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Null pointer\n");
    return;
  }
  curr_pointer_node = *pointer_list_head_addr;
  while(curr_pointer_node)
  {
    next_pointer_node = curr_pointer_node->next;
    if(curr_pointer_node->pointer)
    {    
        JRD_FREE(curr_pointer_node->pointer);
        curr_pointer_node->pointer=NULL;
    }
    if(curr_pointer_node)
    {
        JRD_FREE(curr_pointer_node);
    }
    curr_pointer_node = next_pointer_node;
  }
  *pointer_list_head_addr = NULL;
  return;
}
// Connie add end, 2015/1/8

#define NANO_SEC 1000000000
int
jrd_wait_for_sig_with_timeout
(
  os_signal_type  *signal_ptr,
  uint32                         timeout_milli_secs
)
{
  int rc = 0;
  struct timeval curr_time;
  struct timespec wait_till_time;
  JRD_LOCK (signal_ptr);
  (signal_ptr)->sig_set == 0;

  /* Get current time of day */
  gettimeofday (&curr_time,NULL);

  /* Set wait time seconds to current + the number of seconds needed for timeout */
  wait_till_time.tv_sec =  curr_time.tv_sec + (timeout_milli_secs/1000);
  wait_till_time.tv_nsec = (curr_time.tv_usec * 1000) +  ((timeout_milli_secs % 1000) * 1000 * 1000);

  /* Check the nano sec overflow */
  if (wait_till_time.tv_nsec >= NANO_SEC ) {

      wait_till_time.tv_sec +=  wait_till_time.tv_nsec/NANO_SEC;
      wait_till_time.tv_nsec %= NANO_SEC;
  }

  while ((signal_ptr)->sig_set == 0)
  {
    if (pthread_cond_timedwait (&(signal_ptr)->cond,
                                &(signal_ptr)->mutex,
                                &wait_till_time) == ETIMEDOUT)
    {
      rc = -3;
      break;
    }
  }
  JRD_UNLOCK_EX (&(signal_ptr)->mutex);

  return rc;

}
void jrd_oem_sock_ind_early_init(void)
{
  if(TRUE == jrd_thread_judge_disable(MODULE_SOCK))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_SOCK Thread isnot init!!!\n");
    return;
  }

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "start %d!\n", jrd_sock_ind_info.module_id);
  jrd_oem_register_ind(&jrd_sock_ind_info);
  return;
}

int jrd_oem_handle_web_ind(e_jrd_sock_msg_ind_type e_msg_ind)
{
  jrd_ind_data_type *p_ind_data = NULL;
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "Connie jrd_oem_handle_web_ind\n");
  JRD_MALLOC(sizeof(jrd_ind_data_type),p_ind_data);
  if (p_ind_data)
  {
    p_ind_data->module_id = MODULE_SOCK;
    p_ind_data->ind_id = e_msg_ind;
    p_ind_data->ind_data = NULL;
    p_ind_data->data_size = 0;
    jrd_oem_ind(p_ind_data);
    JRD_FREE(p_ind_data);
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Malloc p_ind_data fail!\n");
    return -1;
  }
  return 0;
}

void jrd_oem_set_value(void *param, void *value, int size, pthread_mutex_t *mutex)
{
  JRD_LOCK_EX(mutex);
  JRD_MEMCPY(param, value, size);
  JRD_UNLOCK_EX(mutex);
}

int jrd_oem_system_call(char *cmd)
{
  int status;
  jrd_system_call_error_code_t *error_code;
  if(!cmd)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "cmd is NULL!\n");
    return -1;
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "cmd:%s\n",cmd);
  
  status = system(cmd);
  if(!status)
  {
    return 0;
  }
  else if(status == -1)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "system call failed!\n");
    return -1;
  }
  else if(error_code)
  {
    if(WIFEXITED(status))
    {
      error_code->if_shell_finished = WIFEXITED(status);
      error_code->exit_code = WEXITSTATUS(status);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "exit code is %d\n", error_code->exit_code);
      if(error_code->exit_code != 126 && error_code->exit_code != 127)
      {
        return 0;
      }
    }else if(WIFSIGNALED(status))
    {
      error_code->if_finished_by_signal = WIFSIGNALED(status);
      error_code->signal_code = WTERMSIG(status);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "signal_code is %d\n", error_code->signal_code);
    }else if(WIFSTOPPED(status))
    {
      error_code->if_stoped = WIFSTOPPED(status);
      error_code->stop_signal_code = WSTOPSIG(status);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "stop_signal_code is %d\n", error_code->stop_signal_code);
    }
  }
  return -1;
}

pid_t gettid(void)
{
     return syscall(SYS_gettid);
}

int jrd_oem_create_thread(jrd_oem_thread_data_t *thrd_data)
{
     pthread_attr_t attr;
    int rc;

    if(!thrd_data) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " Input parameters are invalid!\n");
        return -1;
    }

    rc = pthread_attr_init(&attr);    
    if (rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, " Failed to init thread attribute, rc=%d\n", rc);
        return rc;
    }

    rc = pthread_attr_setstacksize (&attr, thrd_data->thread_stacksize);    
    if (rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Failed to set stack size, rc=%d\n", rc);
        return rc;
    }

    rc = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);    
    if (rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Failed to set setdetachstate, rc=%d\n", rc);
    }

    rc = pthread_create(&(thrd_data->thread_handle), &attr, thrd_data->thread_func,  ((void*)(thrd_data)));    
    if (rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Failed to create thread, rc=%d\n", rc);
        return rc;
    }

    return 0;
}

int jrd_oem_ip_string_to_integer(char *ip_addr_str,uint32 *ip_addr)
{
    uint32 addr = 0;
    int n, i;

    for (i = 0; i < 4; i++)
    {
        n = strtol(ip_addr_str, &ip_addr_str, 10);

        if (n < 0 || n > 255)
            return -1;

        addr = addr << 8 | n;

        if (*ip_addr_str == '\0')
            break;

        if (*ip_addr_str != '.')
            return -1;

        ip_addr_str++;
    }

    if (i != 3)
        return -1;

    *ip_addr = addr;

    return 0;
}

void jrd_oem_ip_integer_to_string(uint32 sdata,char* ddata)
{
   uint8 *ip_field = (uint8*)&sdata;

    uint8 ip_1_field = ip_field[3];
    uint8 ip_2_field = ip_field[2];
    uint8 ip_3_field = ip_field[1];
    uint8 ip_4_field = ip_field[0];

    JRD_SPRINTF(ddata, "%d.%d.%d.%d", ip_1_field,ip_2_field,ip_3_field,ip_4_field);
}


void jrd_oem_mac_uint64_to_asciix(uint64 mac_add,char* buffer)
{
    uint8 * ac_mac = (uint8*)&mac_add;
    uint8 mac_1 = ac_mac[5];
    uint8 mac_2 = ac_mac[4];
    uint8 mac_3 = ac_mac[3];
    uint8 mac_4 = ac_mac[2];
    uint8 mac_5 = ac_mac[1];
    uint8 mac_6 = ac_mac[0];

    JRD_SPRINTF(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", mac_1,mac_2,mac_3,mac_4,mac_5,mac_6);
}

#define JRD_ADD_PRAM_ID_TO_MASK(val,id) (val[id>>5] |= (1<<(id&31)))
static int jrd_file_get_one_json(FILE *fp, char *buffer, int max_len)
{
  int rtnval,rtnval_pre = 0;
  int string_len = 0;
  boolean is_one_json_finish = FALSE;
  boolean is_comment_line = FALSE;
  while(!feof(fp))
  { 
    rtnval = fgetc(fp);

    if(rtnval == '/')
    {
      if(rtnval_pre == '/')
      {
        is_comment_line = TRUE;
      }
      else
      {
        rtnval_pre = '/';
      }
      continue;
    }
    else
    {
      rtnval_pre = rtnval;
    }
    
    if(rtnval == EOF)
    {
      if(!string_len)
      {
        buffer[0] = '\0';
        return -1;
      }
    }
    else if(rtnval == '\n')
    {
      if(is_one_json_finish)
      {
        break;
      }
      else if(is_comment_line)
      {
        is_comment_line = FALSE;
      }
    }
    else if(rtnval == ' '|| rtnval == '\r')
    {
      continue;
    }
    else if(!is_comment_line)
    { 
      if(';' == rtnval)
      {
        is_one_json_finish = TRUE;//maybe have conmment so continue read
      }
      if(is_one_json_finish)
        continue;
      buffer[string_len++] = rtnval;
      if(string_len + 1 == max_len)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Json is too big, max len:%d\n", max_len);
        break;
      }
    }
  }
  
  buffer[string_len] = '\0';
  return string_len;
}

int jrd_oem_parser_req_conf_file
(
  char * file_name,
  sym_fd_t	symtab
)
{
  FILE *fp; 
  char json_data[768] = {0};
  int data_len = 0;
  json_object * object_data = NULL;
  json_object * info_list_obj = NULL;
  json_object * act_info_obj = NULL;
  json_object * param_list_obj = NULL;
  json_object * param_id_obj = NULL;
  int act_index = 0,param_id_index = 0;
  char *req_str,*module_str,*act_str,*param_str;
  int module_id,act_id,param_id;
  web_act_info_type **act_info = NULL;
  web_act_info_type *one_act_info = NULL;
  if((fp = JRD_FOPEN(file_name, "r")) == NULL) 
  { 
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"have no such file %s !\n",file_name);
      return -1; 
  }
  while(!feof(fp)) 
  { 
    data_len = jrd_file_get_one_json(fp, json_data, sizeof(json_data));
    if(data_len > 0)
    {
      object_data = json_tokener_parse(json_data);
      if(!object_data)
      {
        //parser error
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "parser json error str:%s!!!\n", json_data);
        continue;
      }
      req_str = (char *)json_object_get_string(json_object_object_get(object_data,"req"));
      if(!req_str)
      {
        continue;
      }
      info_list_obj = json_object_object_get(object_data,"info");
      
      act_index = 0;
      do{
        act_info_obj = json_object_array_get_idx(info_list_obj, act_index);//json_object_get_object(object_data);
        act_index ++;
      }while(act_info_obj);
      act_index --;
      
      if(0 == act_index)
      {
        return;
      }
      
      JRD_MALLOC((act_index+1) * sizeof(*act_info), act_info);
      symEnter(symtab, req_str, valueInteger((long)act_info), (int) NULL);
      act_index = 0;
      do{
        act_info_obj = json_object_array_get_idx(info_list_obj, act_index);//json_object_get_object(object_data);
        act_index ++;
        if(act_info_obj)
        {
          module_str = (char *)json_object_get_string(json_object_object_get(act_info_obj,"module"));
          act_str = (char *)json_object_get_string(json_object_object_get(act_info_obj,"act"));
          if(!module_str||!act_str)
          {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "module_str:%s, act_str:%s!!!\n", module_str, act_str);
            break;
          }
          module_id = JRD_ATOI(module_str);
          act_id = JRD_ATOI(act_str);
          JRD_MALLOC(sizeof(*one_act_info), one_act_info);
          *act_info++ = one_act_info;
          one_act_info->module_id = module_id;
          one_act_info->act_id = act_id;
          param_list_obj = json_object_object_get(act_info_obj,"id");
          param_id_index = 0;
          do{
            param_id_obj = json_object_array_get_idx(param_list_obj, param_id_index);
            param_str = (char *)json_object_get_string(param_id_obj);
            if(param_str)
            {
              param_id = JRD_ATOI(param_str);
            }
            else
            {
              continue;
            }
            if(param_id > 128)
            {
              JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "invalid param_id:%d!!!\n", param_id);
              continue;
            }
            
            //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "param_str:%s,param_id:%d\n", param_str,param_id);
            JRD_ADD_PRAM_ID_TO_MASK(one_act_info->sub_mask, param_id);
            param_id_index ++;
          }while(param_id_obj);
          
          //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "module:%d,act:%d,mask:0x%04x\n", one_act_info->module_id,one_act_info->act_id,one_act_info->sub_mask[0]);
        }
      }while(act_info_obj);
      json_object_put(object_data);
      JRD_MALLOC(sizeof(*one_act_info), one_act_info);
      one_act_info->module_id = MODULE_INVALID;
      *act_info = one_act_info;
    }
    else
    {
      
    }
  }
  JRD_FCLOSE(fp); 
  return 0; 
}

void jrd_creat_json_and_write_to_file(FILE *fp,web_data_info_type req_info)
{
  json_object * info_object = NULL;
  json_object * info_list_object = NULL;
  json_object * one_act_info_object = NULL;
  json_object * id_list_object = NULL;
  web_act_info_type* module_act_info; 
  int mask_index = 0; 
  int offset = 0;
  int total_offset = 0;
  /*symEnter(tn_json_symtab, 
                 tn_json_sms_info[i].tn_web_req_name, 
                 valueInteger((long)tn_json_sms_info[i].module_act_info), (int) NULL);*/
  info_object = json_object_new_object();
  json_object_object_add(info_object, "req", json_object_new_string((char *)req_info.tn_web_req_name));
  module_act_info = req_info.module_act_info;
  info_list_object = json_object_new_array();
  while(module_act_info&&(module_act_info->module_id!=MODULE_INVALID))
  {
    one_act_info_object = json_object_new_object();
    //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"module_act_info->module_id:%d\n",module_act_info->module_id);
    json_object_object_add(one_act_info_object,"module",json_object_new_int(module_act_info->module_id));
    json_object_object_add(one_act_info_object,"act",json_object_new_int(module_act_info->act_id));
    //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"one act string:%s\n",json_object_to_json_string(one_act_info_object));
    id_list_object = json_object_new_array();
    for(mask_index = 0;mask_index <4;mask_index++)
    {
      for(offset = 0;module_act_info->sub_mask[mask_index]>>offset,offset < 32;offset++)
      {
        if((module_act_info->sub_mask[mask_index]>>offset)&1)
          json_object_array_add(id_list_object, json_object_new_int(mask_index*32+offset));
      }
    }
    //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"id_list_object string:%s\n",json_object_to_json_string(id_list_object));
    //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"one act string:%s\n",json_object_to_json_string(one_act_info_object));
    json_object_object_add(one_act_info_object,"id",id_list_object);
    //JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"one act string:%s\n",json_object_to_json_string(one_act_info_object));
    json_object_array_add(info_list_object,one_act_info_object);
    module_act_info ++;
  }
  json_object_object_add(info_object, "info",info_list_object);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"json string:%s\n",json_object_to_json_string(info_object));
  fputs(json_object_to_json_string(info_object),fp);
  fputs("\n;\n",fp);
  
}

int jrd_oem_popen_fopen_fgets(char* cmd_str,char*middle_file_name_and_dir,char*tmp_buf)
{
      //FILE *fd = NULL;
      char jrd_cmd_str[320] = {0};
      FILE *f_fd = NULL;
  
      JRD_SNPRINTF(jrd_cmd_str,sizeof(jrd_cmd_str),"%s  > %s",cmd_str,middle_file_name_and_dir);
      // fd = JRD_POPEN(cmd_str, "r");
      jrd_oem_system_call(jrd_cmd_str);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_popen_fopen_fgets\n");
      f_fd = JRD_FOPEN(middle_file_name_and_dir,"r");
      if(0 != JRD_FGETS(tmp_buf, sizeof(jrd_cmd_str), f_fd))
      {
           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_oem_popen_fopen_fgets tmp_buf = %s\n",tmp_buf);
      }
      else
      {
			return -1;
      }
      JRD_FCLOSE(f_fd);
      JRD_MEMSET(jrd_cmd_str,0,sizeof(jrd_cmd_str));
      JRD_SNPRINTF(jrd_cmd_str,sizeof(jrd_cmd_str),"rm -rf %s",middle_file_name_and_dir);
      jrd_oem_system_call(jrd_cmd_str);
	return 0;
}

int jrd_writer_file_conte(char* p_path_and_name, char *p_cont)
{
	FILE *fp = NULL;

    if((NULL==p_path_and_name)||(NULL==p_cont))
    {
   	 	JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "wds_write_file_cont: null pointer\n");
   	 	return -1;
  	}

  	fp = fopen(p_path_and_name, "wb");
 	/* File not found at the given path */
 	if (NULL == fp)
 	{
    	JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "wds_write_file_cont: unable to open file\n");
    	return -1;
  	} 
 	fprintf(fp, "%s",p_cont);
  	fclose(fp);
  	return 0;
}

int wds_read_file_cont(char* p_path_and_name, char *p_cont, int *p_len)
{
  FILE *fp = NULL;
  unsigned int cur_pos;
  size_t read_size;
		 
  if((NULL ==p_path_and_name) ||(NULL ==p_cont) ||(NULL==p_len))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "wds_read_file_cont: null pointer\n");
    return -1;
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "wds_read_file_cont:open file %s\n",p_path_and_name);
  fp = fopen(p_path_and_name, "rb");

  /* File not found at the given path */
  if (NULL == fp)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "wds_read_file_cont: unable to open file %s\n",p_path_and_name);
    return -1;
  }
  /* If seek to the end failed or file size is greater than what we support */
  else if (fseek(fp, 0, SEEK_END) ||
          ((cur_pos = ftell(fp)) < 0 || cur_pos > 512) )
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_MEDIAM, "wds_read_file_cont:out of buffer range\n");
    fclose(fp);
    return -1;
  }
  else
  {
    /* Reset to the beginning of the file */
    if (!fseek(fp, 0, SEEK_SET))
    {
      /* Read the data from file to buffer */
      memset(p_cont,0,cur_pos);
      read_size = fread(p_cont, 1, cur_pos, fp);

      if (!ferror(fp) && (read_size == cur_pos))
      {
        *p_len=read_size;
        fclose(fp);
      }
      else
      {
        fclose(fp);
        return -1;
      }
    }
    else
    {
      fclose(fp);
      return -1;	  
    }
  }
  return 0;
}
static int jrd_oem_ind_cmd_send
(
 int fd,
 e_ind_to_clt_id_type ind_id,
 void* format_data, 
 uint32 format_data_len
)
{
  uint32 max_data_len = JRD_SOCK_CORE_APP_TX_BUF_SIZE  - sizeof(msg_head_t);
  msg_head_t msg_head = {0};
  pack_head_t pack_head = {0};
  pack_head.pack_head = PACK_HEAD;
  msg_head.msg_type = MSG_IND;

  if(fd == JRD_SOCK_INVALID_FD)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid sock fd: %d, should hang up the program\n", fd);
    //Error handler here, should hang up here
    return -1;
  }
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "send ind start\n");
  {
    jrd_thread_info_type* jrd_sock_tx_thread_info = NULL;
    jrd_sock_cmd_q_type* sock_cmd_q = NULL;
    uint8* data_ptr = NULL;
    
    JRD_MALLOC(sizeof(jrd_sock_cmd_q_type), sock_cmd_q);
    if(!sock_cmd_q)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "JRD_MALLOC sock_cmd_q failed, should hang up the program\n");
      //Error handler here, should hang up here
      return -1;
    }
    sock_cmd_q->fd = fd;
    
    pack_head.pack_len = sock_cmd_q->data_len = format_data_len + sizeof(int) + sizeof(msg_head) + sizeof(pack_head);
    JRD_MALLOC(sock_cmd_q->data_len, sock_cmd_q->data_buf);
    if(!sock_cmd_q->data_buf)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "JRD_MALLOC sock_cmd_q->data_buf failed, should hang up the program\n");
      //Error handler here, should hang up here
      JRD_FREE(sock_cmd_q);
      return -1;
    }
    data_ptr = sock_cmd_q->data_buf;
    JRD_MEMCPY(sock_cmd_q->data_buf, &pack_head, sizeof(pack_head));//add pack head to buff
    data_ptr += sizeof(pack_head);
    JRD_MEMCPY(data_ptr, &msg_head, sizeof(msg_head_t));
    data_ptr += sizeof(msg_head_t);//add msg info
    JRD_MEMCPY(data_ptr, &ind_id, sizeof(int));
    data_ptr += sizeof(int);
    memcpy(data_ptr,format_data,format_data_len);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "ind len: %d, ind str:\n%s\n", sock_cmd_q->data_len, data_ptr);
    jrd_sock_tx_thread_info = jrd_oem_get_thread_info(MODULE_SOCK);
    if(!jrd_sock_tx_thread_info)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid jrd_sock_tx_thread_info pointer, should hang up the program\n");
      JRD_FREE(sock_cmd_q->data_buf);
      JRD_FREE(sock_cmd_q);
      return -1;
    }
    q_link(sock_cmd_q, &sock_cmd_q->link);
    q_put(jrd_sock_tx_thread_info->p_module_q, &sock_cmd_q->link);
    JRD_OS_SIGNAL_SET(jrd_sock_tx_thread_info->p_os_sig);
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "send ind end\n");
  
  return 0;
}

int ind_msg_to_clients(e_ind_to_clt_id_type ind_id,void *msg, uint32 msg_len)
{
  int fds[32]={0};
  int i = 0;
  if(get_client_fd_list(fds))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "get fd list fail!\n");
    return -1;
  }
  while(fds[i])
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "fd:%d\n",fds[i]);
    jrd_oem_ind_cmd_send(fds[i],ind_id,msg, msg_len);
    i++;
  }
}

