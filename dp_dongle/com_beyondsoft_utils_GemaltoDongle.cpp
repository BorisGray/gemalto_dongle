#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include "hasp_api.h"
#include "com_beyondsoft_utils_GemaltoDongle.h"
#include "hasp_vcode.h"  /* contains HASP DEMOMA vendor code */

#define FIXED_MEMBUFFER_SECTION_SIZE   20
#define DUMP_COLS_PER_ROW     16
#define DUMP_SPACE_COLS        8

int find_pos_in_string(const std::string full_str, const std::string sub_str)
{
  int len = full_str.length();
  int i = full_str.find(sub_str);
  if (i >0 && i < len)
    {
      return i;
    }
  else {
      return -1;
    }
}

jstring char_to_jstring(JNIEnv* env, const char* pat) {
  jclass strClass = (env)->FindClass("Ljava/lang/String;");
  jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
  jbyteArray bytes = (env)->NewByteArray(strlen(pat));
  (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*) pat);
  jstring encoding = (env)->NewStringUTF("utf-8");
  return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}

void dump_hex(unsigned char *data, unsigned int datalen, char *margin)
{
  unsigned int i;

  for (i = 0; i < datalen; i++)
    {
      if (((i % DUMP_SPACE_COLS) == 0) && (i != 0))
        printf(" ");

      if ((i % DUMP_COLS_PER_ROW) == 0)
        {
          if (i != 0)
            printf("\n");

          if (margin != NULL)
            printf("%s", margin);
        }

      /* dump character in hex */
      printf("%02X ", data[i]);
    }
} /* dump_hex */

/****************************************************************************
 * helper function: dumps a given block of data, in ascii
 */

void dump_ascii(unsigned char *data, unsigned int datalen, char *margin)
{
  unsigned int i;

  for (i = 0; i < datalen; i++)
    {
      if (((i % DUMP_SPACE_COLS) == 0) && (i != 0))
        printf(" ");

      if ((i % DUMP_COLS_PER_ROW) == 0)
        {
          if (i != 0)
            printf("\n");

          if (margin != NULL)
            printf("%s", margin);
        }

      /* dump printable character in ascii */
      printf("%c", ((data[i] > 31) && (data[i] < 128)) ? data[i] : '.');
    }
} /* dump_ascii */

/****************************************************************************
 * helper function: dumps a given block of data, in hex and ascii
 */

void dump(unsigned char *data, unsigned int datalen, char *margin)
{
  unsigned int i, icols;

  for (i = 0; i < datalen; )
    {
      icols = datalen - i;

      if (icols > DUMP_COLS_PER_ROW)
        icols = DUMP_COLS_PER_ROW;

      dump_hex(&data[i], icols, margin);
      dump_ascii(&data[i], icols, "  ");
      printf("\n");

      i += icols;
    }
} /* dump */


#define ERR_CODE_LOGIN            0xFFFFFFFFFFFFFFFF
#define ERR_CODE_GET_SESSION_INFO 0xFFFFFFFFFFFFFFFE
#define ERR_CODE_XML_PARSE        0xFFFFFFFFFFFFFFFD
#define ERR_CODE_LOGOUT           0xFFFFFFFFFFFFFFFC

JNIEXPORT jlong JNICALL FUNCNAME(getDongleId)(JNIEnv *env, jobject obj)
{
  (void)env;
  (void)obj;
  hasp_status_t   status;
  hasp_handle_t   handle;
  char           *info;

  status = hasp_login(HASP_DEFAULT_FID,
                      (hasp_vendor_code_t)vendor_code,
                      &handle);

  switch (status)
    {
    case HASP_STATUS_OK:
      printf("OK\n");
      break;

    case HASP_FEATURE_NOT_FOUND:
      printf("login to default feature failed\n");
      break;

    case HASP_CONTAINER_NOT_FOUND:
      printf("no sentinel key with vendor code DEMOMA found\n");
      break;

    case HASP_OLD_DRIVER:
      printf("outdated driver version installed\n");
      break;

    case HASP_NO_DRIVER:
      printf("sentinel driver not installed\n");
      break;

    case HASP_INV_VCODE:
      printf("invalid vendor code\n");
      break;

    default:
      printf("login to default feature failed with status %d\n", status);
    }
  if (status) {
      return ERR_CODE_LOGIN;
    }


  /************************************************************************
         * hasp_get_sessioninfo
         *   retrieve Sentinel key attributes
         *
         * Please note: In case of performing an activation we recommend to use
         *              hasp_get_info() instead of hasp_get_sessioninfo(), as
         *              demonstrated in the activation sample. hasp_get_info()
         *              can be called without performing a login.
         */

  printf("\nget session info                 : ");

  status = hasp_get_sessioninfo(handle,
                                HASP_KEYINFO,
                                &info);

  switch (status)
    {
    case HASP_STATUS_OK:
      printf("OK, Sentinel key attributes retrieved\n\n"
             "Key info:\n===============\n%s===============\n", info);
      break;

    case HASP_INV_HND:
      printf("handle not active\n");
      break;

    case HASP_INV_FORMAT:
      printf("unrecognized format\n");
      break;

    case HASP_CONTAINER_NOT_FOUND:
      printf("key/license container not available\n");
      break;

    default:
      printf("hasp_get_sessioninfo failed\n");
    }
  if (status) {
      return ERR_CODE_GET_SESSION_INFO;
    }

  std::string full_xml_str(info);
  hasp_free(info);
  std::string hasp_id_node_str1("<haspid>");
  std::string hasp_id_node_str2("</haspid>");

  int pos1 =-1;
  pos1 = find_pos_in_string(full_xml_str, hasp_id_node_str1);
  if (pos1 < 0)
    {
      return ERR_CODE_XML_PARSE;
    }

  int pos2 = -1;
  pos2 = find_pos_in_string(full_xml_str, hasp_id_node_str2);
  if (pos2 < 0)
    {
      return ERR_CODE_XML_PARSE;
    }

  std::string sub_str(full_xml_str.substr(pos1, pos2-pos1));
  std::string hasp_id_str = sub_str.substr(hasp_id_node_str1.length(), sub_str.length());
  std::cout << "hasp_id:  " << hasp_id_str << std::endl;

  /************************************************************************
       * hasp_logout
       *   closes established session and releases allocated memory
       */

  printf("\nlogout from default feature      : ");

  status = hasp_logout(handle);

  switch (status)
    {
    case HASP_STATUS_OK:
      printf("OK\n");
      break;

    case HASP_INV_HND:
      printf("failed: handle not active\n");
      break;

    default:
      printf("failed\n");
    }
  if (status) {
      return ERR_CODE_LOGOUT;
    }

  return atol(hasp_id_str.c_str());
}

JNIEXPORT jstring JNICALL FUNCNAME(getDongleInfo)(JNIEnv *env, jobject obj, jlong dongle_id)
{
  printf("Java_GemaltoDongle_getDongleInfo!\n");
  printf("login to feature XXX        : ");

  (void)obj;
  hasp_status_t   status;
  hasp_handle_t   handle;
  char           *info;
  hasp_time_t     current_time;
  unsigned int    current_day, current_month, current_year, current_hour, current_minute, current_second;
  unsigned int    mem_day, mem_month, mem_year;
  unsigned char fixed_membuffer[FIXED_MEMBUFFER_SECTION_SIZE];
  hasp_size_t     fsize;

  jstring js_err_str = NULL;
  /************************************************************************
     * hasp_login
     *   establishes a context for HASP services
     */

  //    printf("pid=%d\n", getpid());

  //    char path[PATH_MAX];
  //           char processname[1024];
  //           get_executable_path(path, processname, sizeof(path));
  //           printf("directory:%s\nprocessname:%s\n",path,processname);

  //    sleep(60);

  /* login to default feature (0)                 */
  /* this default feature is available on any key */
  /* search for local and remote HASP key         */

  status = hasp_login(HASP_DEFAULT_FID,
                      (hasp_vendor_code_t)vendor_code,
                      &handle);

  switch (status)
    {
    case HASP_STATUS_OK:
      printf("OK\n");
      break;

    case HASP_FEATURE_NOT_FOUND:
      printf("login to default feature failed\n");
      break;

    case HASP_CONTAINER_NOT_FOUND:
      printf("no sentinel key with vendor code DEMOMA found\n");
      break;

    case HASP_OLD_DRIVER:
      printf("outdated driver version installed\n");
      break;

    case HASP_NO_DRIVER:
      printf("sentinel driver not installed\n");
      break;

    case HASP_INV_VCODE:
      printf("invalid vendor code\n");
      break;

    default:
      printf("login to default feature failed with status %d\n", status);
    }
  if (status) {
      std::string err_str = "hasp_login ERROR !!!";
      char* chardata = const_cast<char*>(err_str.c_str());
      js_err_str = char_to_jstring(env, chardata);
      return js_err_str;
    }


  /************************************************************************
       * hasp_get_sessioninfo
       *   retrieve Sentinel key attributes
       *
       * Please note: In case of performing an activation we recommend to use
       *              hasp_get_info() instead of hasp_get_sessioninfo(), as
       *              demonstrated in the activation sample. hasp_get_info()
       *              can be called without performing a login.
       */

  printf("\nget session info                 : ");

  status = hasp_get_sessioninfo(handle,
                                HASP_KEYINFO,
                                &info);

  switch (status)
    {
    case HASP_STATUS_OK:
      printf("OK, Sentinel key attributes retrieved\n\n"
             "Key info:\n===============\n%s===============\n", info);
      break;

    case HASP_INV_HND:
      printf("handle not active\n");
      break;

    case HASP_INV_FORMAT:
      printf("unrecognized format\n");
      break;

    case HASP_CONTAINER_NOT_FOUND:
      printf("key/license container not available\n");
      break;

    default:
      printf("hasp_get_sessioninfo failed\n");
    }
  if (status) {
      hasp_logout(handle);
      std::string err_str = "hasp_get_sessioninfo ERROR !!!";
      char* chardata = const_cast<char*>(err_str.c_str());
      js_err_str = char_to_jstring(env, chardata);
      return js_err_str;
    }

  std::string full_xml_str(info);
  hasp_free(info);
  std::string hasp_id_node_str1("<haspid>");
  std::string hasp_id_node_str2("</haspid>");

  int pos1 =-1;
  pos1 = find_pos_in_string(full_xml_str, hasp_id_node_str1);
  if (pos1 < 0)
    {
      std::string err_str = "XMP data parse ERROR !!!";
      char* chardata = const_cast<char*>(err_str.c_str());
      js_err_str = char_to_jstring(env, chardata);
      return js_err_str;
    }

  int pos2 = -1;
  pos2 = find_pos_in_string(full_xml_str, hasp_id_node_str2);
  if (pos2 < 0)
    {

      std::string err_str = "XMP data parse ERROR !!!";
      char* chardata = const_cast<char*>(err_str.c_str());
      js_err_str = char_to_jstring(env, chardata);
      return js_err_str;
    }
  //  std::cout <<pos1 <<", " << pos2 << std::endl;

  std::string sub_str(full_xml_str.substr(pos1, pos2-pos1));
  //  std::cout << "sub_str:  " << sub_str << std::endl;

  std::string hasp_id_str = sub_str.substr(hasp_id_node_str1.length(), sub_str.length());
  std::cout << "hasp_id:  " << hasp_id_str << std::endl;
  std::cout << "dongle id:  " << dongle_id << std::endl;

  if (atol(hasp_id_str.c_str()) != dongle_id)
    {
      std::string err_str = "ID NOT match!!!";
      char* chardata = const_cast<char*>(err_str.c_str());

      js_err_str = char_to_jstring(env, chardata);
      return js_err_str;
    }


  /************************************************************************
       * hasp_get_size
       *   retrieve the memory size of the HASP key
           *   you can also retrieve dynamic memory file size,
           *   only need to pass the dynamic memory file id which
           *	 created in token
       */

  printf("\nretrieving the key's memory size : ");

  status = hasp_get_size(handle,
                         HASP_FILEID_RW,
                         &fsize);

  switch (status)
    {
    case HASP_STATUS_OK:
      printf("sentinel memory size is %d bytes\n", fsize);
      break;

    case HASP_INV_HND:
      printf("handle not active\n");
      break;

    case HASP_INV_FILEID:
      printf("invalid file id\n");
      break;

    case HASP_CONTAINER_NOT_FOUND:
      printf("key/license container not available\n");
      break;

    default:
      printf("could not retrieve memory size\n");
    }
  if (status) {
      hasp_logout(handle);
      std::string err_str = "hasp_get_size ERROR !!!";
      char* chardata = const_cast<char*>(err_str.c_str());
      js_err_str = char_to_jstring(env, chardata);
      return js_err_str;
    }

  if (fsize != 0)       /* skip memory access if no memory available */
    {

      /********************************************************************
           * hasp_read
           *   read from HASP memory
                   *   you can also read dynamic memory file,
                   *   only need to pass the dynamic memory file id which
                   *	 created in token
           */

      /* limit memory size to be used in this demo program */

      if (fsize > FIXED_MEMBUFFER_SECTION_SIZE)
        fsize = FIXED_MEMBUFFER_SECTION_SIZE;

      printf("\nreading %4d bytes from memory   : ", fsize);

      status = hasp_read(handle,
                         HASP_FILEID_RW,    /* read/write file ID */
                         0,                 /* offset */
                         fsize,             /* length */
                         &fixed_membuffer[0]);    /* file data */

      switch (status)
        {
        case HASP_STATUS_OK:
          printf("OK\n");
          //          dump(fixed_membuffer, fsize, "    ");
          break;

        case HASP_INV_HND:
          printf("handle not active\n");
          break;

        case HASP_INV_FILEID:
          printf("invalid file id\n");
          break;

        case HASP_MEM_RANGE:
          printf("beyond memory range of attached sentinel key\n");
          break;

        case HASP_CONTAINER_NOT_FOUND:
          printf("key/license container not available\n");
          break;

        default:
          printf("read memory failed\n");
        }
      if (status) {
          hasp_logout(handle);
          std::string err_str = "hasp_read ERROR !!!";
          char* chardata = const_cast<char*>(err_str.c_str());
          js_err_str = char_to_jstring(env, chardata);
          return js_err_str;
        }

      /* Parse result string */
      std::cout << "fixed memory section buffer string:  " << fixed_membuffer << std::endl;

      std::string str_fixed_membuf = (char*)fixed_membuffer;
      mem_year = atoi(str_fixed_membuf.substr(6, 4).c_str());
      mem_month = atoi(str_fixed_membuf.substr(10, 2).c_str());
      mem_day = atoi(str_fixed_membuf.substr(12, 2).c_str());
      std::cout << "memory date string:  " << mem_year << " " << mem_month << " " << mem_day << std::endl;

      /************************************************************************
           * hasp_get_rtc
           *   read current time from HASP Time key
           */

      printf("\nreading current time and date    : ");

      status = hasp_get_rtc(handle, &current_time);

      switch (status)
        {
        case HASP_STATUS_OK:
          /* convert HASP time value to time and date */
          status = hasp_hasptime_to_datetime(current_time, &current_day, &current_month, &current_year,
                                             &current_hour, &current_minute, &current_second);

          switch (status)
            {
            case HASP_STATUS_OK:
              printf("time: %d:%d:%d H/M/S\n                  "
                     "                 date: %d/%d/%d D/M/Y\n",
                     current_hour, current_minute, current_second, current_day, current_month, current_year);
              break;

            case HASP_INV_TIME:
              printf("time value outside supported range\n");
              break;

            default:
              printf("time conversion failed\n");
            }
          break;

        case HASP_INV_HND:
          printf("handle not active\n");
          break;

        case HASP_NO_TIME:
          printf("no sentinel Time connected\n");
          break;

        case HASP_CONTAINER_NOT_FOUND:
          printf("key/license container not available\n");
          break;

        default:
          printf("could not read time from sentinel key, status %d\n", status);
        }
      if ( (status) && (status != HASP_NO_TIME) ) {
          hasp_logout(handle);
          std::string err_str = "hasp_get_rtc ERROR !!!";
          char* chardata = const_cast<char*>(err_str.c_str());
          js_err_str = char_to_jstring(env, chardata);
          return js_err_str;
        }

      if (current_year > mem_year)
        {
          std::cout << "current_year:  " << current_year << " > mem_year: " << mem_year << std::endl;

          std::string err_str = "Dongle out of year!!!";
          char* chardata = const_cast<char*>(err_str.c_str());

          js_err_str = char_to_jstring(env, chardata);
          return js_err_str;
        }
      else {
          if (current_year == mem_year)
            {
              if (current_month >mem_month)
                {
                  std::cout << "current_month:  " << current_month << " > mem_month:  " << mem_month << std::endl;

                  std::string err_str = "Dongle out of month!!!";
                  char* chardata = const_cast<char*>(err_str.c_str());

                  js_err_str = char_to_jstring(env, chardata);
                  return js_err_str;
                }
              else {
                  if (current_month == mem_month)
                    {
                      if (current_day > mem_day)
                        {
                          std::cout << "current_day:  " << current_day << " > mem_day:  " << mem_day << std::endl;
                          std::string err_str = "Dongle out of day!!!";
                          char* chardata = const_cast<char*>(err_str.c_str());

                          js_err_str = char_to_jstring(env, chardata);
                          return js_err_str;
                        }

                    }
                }
            }
        }

      std::string str_all_membuffer_len = str_fixed_membuf.substr(0, 6);
      std::cout << "all memory buffer length in hex: " << str_all_membuffer_len << std::endl;
      char *str = 0;
      int all_membuffer_lenth = (int)strtol(str_all_membuffer_len.c_str(), &str, 16);
      printf("all memory buffer length in decimal:  %d\n",all_membuffer_lenth);

      printf("\nreading %4d bytes from memory   : ", all_membuffer_lenth);

      unsigned char all_mem_data[all_membuffer_lenth];

      status = hasp_read(handle,
                         HASP_FILEID_RW,    /* read/write file ID */
                         6,                 /* offset */
                         all_membuffer_lenth,             /* length */
                         &all_mem_data[0]);    /* file data */

      switch (status)
        {
        case HASP_STATUS_OK:
          printf("OK\n");
          //          dump(all_mem_data, fsize, "    ");
          break;

        case HASP_INV_HND:
          printf("handle not active\n");
          break;

        case HASP_INV_FILEID:
          printf("invalid file id\n");
          break;

        case HASP_MEM_RANGE:
          printf("beyond memory range of attached sentinel key\n");
          break;

        case HASP_CONTAINER_NOT_FOUND:
          printf("key/license container not available\n");
          break;

        default:
          printf("read memory failed\n");
        }
      if (status) {
          hasp_logout(handle);
          std::string err_str = "hasp_read ERROR !!!";
          char* chardata = const_cast<char*>(err_str.c_str());
          js_err_str = char_to_jstring(env, chardata);
          return js_err_str;
        }

      std::cout << "data memory section buffer string:  " << all_mem_data << std::endl;
      js_err_str = char_to_jstring(env, (char*)(all_mem_data));
      return js_err_str;
    } /* end of memory demo */


  /************************************************************************
     * hasp_logout
     *   closes established session and releases allocated memory
     */

  printf("\nlogout from default feature      : ");

  status = hasp_logout(handle);

  switch (status)
    {
    case HASP_STATUS_OK:
      printf("OK\n");
      break;

    case HASP_INV_HND:
      printf("failed: handle not active\n");
      break;

    default:
      printf("failed\n");
    }
  if (status) {
      std::string err_str = "hasp_logout ERROR !!!";
      char* chardata = const_cast<char*>(err_str.c_str());
      js_err_str = char_to_jstring(env, chardata);
      return js_err_str;
    }
}
