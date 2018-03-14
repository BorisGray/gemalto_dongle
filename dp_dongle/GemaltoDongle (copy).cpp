#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
//#include<libxml/parser.h>
//#include<libxml/tree.h>
#include "hasp_api.h"
#include "GemaltoDongle.h"
#include "hasp_vcode.h"  /* contains HASP DEMOMA vendor code */

/*
jint Hasp_Login(JNIEnv *env, jclass obj, jlong feature_id,
                jstring vendor_code, jintArray handle)
{
    const char *ptrByte_vendor_code =
            (*env)->GetStringUTFChars(env, vendor_code, 0);
    jint *ptr32bit_handle = (*env)->GetIntArrayElements(env, handle, 0);
    jint result = 0;

    (void)obj;

    result = (jint)hasp_login((hasp_feature_t)feature_id,
                    (hasp_vendor_code_t)ptrByte_vendor_code,
                    (hasp_handle_t*)ptr32bit_handle);

    (*env)->ReleaseStringUTFChars(env, vendor_code, ptrByte_vendor_code);
    (*env)->ReleaseIntArrayElements(env, handle, ptr32bit_handle, 0);

    return result;
}
*/

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


std::string int_to_str_len2(int i)
{
  std::stringstream ss;
  std::string s;
  char t[256];
  sprintf(t, "%02d", i);
  s = t;
  //    std::cout << "i = " << i <<","
  //              << "s = " << s << std::endl;
  return s;
}

//size_t get_executable_path( char* processdir,char* processname, size_t len)
//{
//  char* path_end;
//  if(readlink("/proc/self/exe", processdir,len) <=0)
//    return -1;
//  path_end = strrchr(processdir,  '/');
//  if(path_end == NULL)
//    return -1;
//  ++path_end;
//  strcpy(processname, path_end);
//  *path_end = '\0';
//  return (size_t)(path_end - processdir);
//}
#define FIXED_MEMBUFFER_SECTION_SIZE   20
#define DUMP_COLS_PER_ROW     16
#define DUMP_SPACE_COLS        8
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

JNIEXPORT jstring JNICALL FUNCNAME(getDongleInfo)(JNIEnv *env, jobject obj, jlong dongle_id)
{
  printf("Java_GemaltoDongle_getDongleInfo!\n");
  printf("login to feature XXX        : ");

  hasp_status_t   status;
  hasp_handle_t   handle;
  char           *info;
  hasp_time_t     time, *hasp_mem_date_time = NULL;
  unsigned int    day, month, year, hour, minute, second;
  unsigned char membuffer[FIXED_MEMBUFFER_SECTION_SIZE];
  hasp_size_t     fsize;

  std::string err_str;
  const std::string mem_str ="FFFF20181231120102FFFF933304507";

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
      //      exit(-1);
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
      //      exit(-1);
    }

  std::string full_xml_str(info);
  hasp_free(info);
  std::string hasp_id_node_str1("<haspid>");
  std::string hasp_id_node_str2("</haspid>");

  int pos1 =-1;
  pos1 = find_pos_in_string(full_xml_str, hasp_id_node_str1);
  if (pos1 < 0)
    {
      std::cout <<"pos1 ERROR! " << std::endl;
      jstring js_err_str = NULL;
      return js_err_str;
    }

  int pos2 = -1;
  pos2 = find_pos_in_string(full_xml_str, hasp_id_node_str2);
  if (pos2 < 0)
    {
      std::cout <<"pos2 ERROR! " << std::endl;
      jstring js_err_str = NULL;
      return js_err_str;
    }
  //  std::cout <<pos1 <<", " << pos2 << std::endl;

  std::string sub_str(full_xml_str.substr(pos1, pos2-pos1));
  //  std::cout << "sub_str:  " << sub_str << std::endl;

  std::string hasp_id_str = sub_str.substr(hasp_id_node_str1.length(), sub_str.length());
  std::cout << "hasp_id:  " << hasp_id_str << std::endl;

  // !!!!!!!!!!!!!!!!TYPE_CAST!!!!!!!!!!!!!!!!!!!!!!!!!
  std::cout << "dongle id:  " << dongle_id << std::endl;
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
      //      exit(-1);
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
                         &membuffer[0]);    /* file data */

      switch (status)
        {
        case HASP_STATUS_OK:
          printf("OK\n");
          dump(membuffer, fsize, "    ");
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
          //          exit(-1);
        }

      /* Parse result string */
      std::cout << "fixed memory section buffer string:  " << membuffer << std::endl;


      //-------------------------------------------------------------------
      printf("\nreading %4d bytes from memory   : ", 128);

      status = hasp_read(handle,
                         HASP_FILEID_RW,    /* read/write file ID */
                         20,                 /* offset */
                         128,             /* length */
                         &membuffer[0]);    /* file data */

      switch (status)
        {
        case HASP_STATUS_OK:
          printf("OK\n");
          dump(membuffer, fsize, "    ");
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
          //          exit(-1);
        }

      /* Parse result string */
      std::cout << "data memory section buffer string:  " << membuffer << std::endl;


      //      /********************************************************************
      //         * hasp_write
      //         *   write to HASP memory
      //                 *   you can also write dynamic memory file,
      //                 *   only need to pass the dynamic memory file id which
      //                 *	 created in token
      //         */

      //      printf("\nincrementing every byte in memory buffer\n");
      //      for (size_t i = 0; i < fsize; i++)
      //        membuffer[i]++;

      //      printf("\nwriting %4d bytes to memory     : ", fsize);

      //      status = hasp_write(handle,
      //                          HASP_FILEID_RW,
      //                          0,                /* offset */
      //                          fsize,            /* length */
      //                          &membuffer);

      //      switch (status)
      //        {
      //        case HASP_STATUS_OK:
      //          printf("OK\n");
      //          break;

      //        case HASP_INV_HND:
      //          printf("handle not active\n");
      //          break;

      //        case HASP_INV_FILEID:
      //          printf("invalid file id\n");
      //          break;

      //        case HASP_MEM_RANGE:
      //          printf("beyond memory range of attached sentinel key\n");
      //          break;

      //        case HASP_CONTAINER_NOT_FOUND:
      //          printf("key/license container not available\n");
      //          break;

      //        default:
      //          printf("write memory failed\n");
      //        }
      //      if (status) {
      //          hasp_logout(handle);
      //          exit(-1);
      //        }

      //      /********************************************************************
      //         * hasp_read
      //         *   read from HASP memory
      //         */

      //      printf("\nreading %4d bytes from memory   : ", fsize);

      //      status = hasp_read(handle,
      //                         HASP_FILEID_RW,
      //                         0,                 /* offset */
      //                         fsize,             /* length */
      //                         &membuffer);

      //      switch (status)
      //        {
      //        case HASP_STATUS_OK:
      //          printf("OK\n");
      //          dump(membuffer, fsize, "    ");
      //          break;

      //        case HASP_INV_HND:
      //          printf("handle not active\n");
      //          break;

      //        case HASP_INV_FILEID:
      //          printf("invalid file id\n");
      //          break;

      //        case HASP_MEM_RANGE:
      //          printf("beyond memory range of attached sentinel key\n");
      //          break;

      //        case HASP_CONTAINER_NOT_FOUND:
      //          printf("key/license container not available\n");
      //          break;

      //        default:
      //          printf("read memory failed\n");
      //        }
      //      if (status) {
      //          hasp_logout(handle);
      //          //            exit(-1);
      //        }

    } /* end of memory demo */



//  /************************************************************************
//     * hasp_get_rtc
//     *   read current time from HASP Time key
//     */

//  printf("\nreading current time and date    : ");

//  status = hasp_get_rtc(handle, &time);

//  switch (status)
//    {
//    case HASP_STATUS_OK:
//      /* convert HASP time value to time and date */
//      status = hasp_hasptime_to_datetime(time, &day, &month, &year,
//                                         &hour, &minute, &second);

//      switch (status)
//        {
//        case HASP_STATUS_OK:
//          printf("time: %d:%d:%d H/M/S\n                  "
//                 "                 date: %d/%d/%d D/M/Y\n",
//                 hour, minute, second, day, month, year);
//          break;

//        case HASP_INV_TIME:
//          printf("time value outside supported range\n");
//          break;

//        default:
//          printf("time conversion failed\n");
//        }
//      break;

//    case HASP_INV_HND:
//      printf("handle not active\n");
//      break;

//    case HASP_NO_TIME:
//      printf("no sentinel Time connected\n");
//      break;

//    case HASP_CONTAINER_NOT_FOUND:
//      printf("key/license container not available\n");
//      break;

//    default:
//      printf("could not read time from sentinel key, status %d\n", status);
//    }
//  if ( (status) && (status != HASP_NO_TIME) ) {
//      hasp_logout(handle);
//      //      exit(-1);
//    }

//  int token_pos1 = find_pos_in_string(mem_str, "FFFF");
//  std::string mem_sub_str =mem_str.substr(4, mem_str.length());
//  int token_pos2 = find_pos_in_string(mem_sub_str, "FFFF");

//  std::string mem_date_time = mem_sub_str.substr(0, token_pos2);
//  std::string mem_dongleid = mem_sub_str.substr(token_pos2+4, mem_sub_str.length());
//  std::cout << "mem_date_time:  " << mem_date_time << ", "
//                                                      "mem_dongleid: " << mem_dongleid <<std::endl;

//  if (hasp_id_str.compare(mem_dongleid) != 0)
//    {
//      err_str = "ID NOT match!!!";
//      std::cout << err_str << std::endl;
//    }
//  else {

//      // current date time ion dongle
//      std::string real_date_time = std::to_string(year);
//      real_date_time += int_to_str_len2(month);
//      real_date_time += int_to_str_len2(day);
//      real_date_time += int_to_str_len2(hour);
//      real_date_time += int_to_str_len2(minute);
//      real_date_time += int_to_str_len2(second);
//      std::cout << "real date time: " << real_date_time << std::endl;


//      std::cout << atoi(mem_date_time.substr(6, 2).c_str()) << std::endl
//                << atoi(mem_date_time.substr(4, 2).c_str()) << std::endl
//                << atoi(mem_date_time.substr(0, 4).c_str()) << std::endl
//                << atoi(mem_date_time.substr(8, 2).c_str()) << std::endl
//                << atoi(mem_date_time.substr(10, 2).c_str()) << std::endl
//                << atoi(mem_date_time.substr(12, 2).c_str()) << std::endl;

//      hasp_mem_date_time = new hasp_time_t;
//      if (NULL == hasp_mem_date_time)
//        {
//          std::cout <<"hasp_mem_date_time allocate memory FAILURE!!!" << std::endl;
//        }
//      else {

//          // !!!!!!!!!!!!!!!!!!!TIME CONVERSION FAILURE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
//          status = hasp_datetime_to_hasptime ((unsigned int)atoi(mem_date_time.substr(6, 2).c_str()),
//                                              (unsigned int)atoi(mem_date_time.substr(4, 2).c_str()),
//                                              (unsigned int)atoi(mem_date_time.substr(0, 2).c_str()),
//                                              (unsigned int)atoi(mem_date_time.substr(8, 2).c_str()),
//                                              (unsigned int)atoi(mem_date_time.substr(10, 2).c_str()),
//                                              (unsigned int)atoi(mem_date_time.substr(12, 2).c_str()),
//                                              (hasp_time_t*)hasp_mem_date_time);
//          switch (status)
//            {
//            case HASP_STATUS_OK:
//              break;

//            case HASP_INV_TIME:
//              printf("time value outside supported range\n");
//              break;

//            default:
//              printf("time conversion failed\n");
//            }

//          std::cout << "current time=" << time  << std::endl;
//          std::cout << "memory time=" << hasp_mem_date_time << ", " << (*hasp_mem_date_time)  << std::endl;

//          if (time > (*hasp_mem_date_time)) {
//              err_str = "Dongle out of date!!!";
//              std::cout << err_str  << std::endl;
//            }

//          delete hasp_mem_date_time;
//        }


//    }


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
      //        exit(-1);
    }
}
