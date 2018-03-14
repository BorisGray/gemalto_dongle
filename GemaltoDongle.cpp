#include <stdio.h>
#include "GemaltoDongle.h"
#include "hasp_vcode.h"  /* contains HASP DEMOMA vendor code */


JNIEXPORT void JNICALL Java_GemaltoDongle_getDongleInfo(JNIEnv *, jobject)
{
    printf("Java_GemaltoDongle_getDongleInfo!\n");

    /************************************************************************
     * hasp_login
     *   establishes a context for HASP services
     */

    printf("login to default feature         : ");

    /* login to default feature (0)                 */
    /* this default feature is available on any key */
    /* search for local and remote HASP key         */

    status = hasp_login(HASP_DEFAULT_FID,
                    (hasp_vendor_code_t)vendor_code,
                    &handle);


}
