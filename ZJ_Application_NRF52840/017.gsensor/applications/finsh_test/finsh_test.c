#include <finsh.h>

#ifdef FINSH_USING_MSH
#include "msh.h"
#endif
/*cstyle*/
int zj_t1(void)
{
#ifdef FINSH_USING_MSH
   if(msh_is_used())
    {
       rt_kprintf("Welcome to use ZJ-SDK! this is finsh msh test\n");
    }
    else
#endif
    {
      rt_kprintf("Welcome to use ZJ-SDK! this is finsh c-tyle test\n");
    }
    return 0;
}

FINSH_FUNCTION_EXPORT(zj_t1, this is finsh c-tyle test(zj_t1() will work))
FINSH_FUNCTION_EXPORT_ALIAS(zj_t1, zj_ft1, this is finsh c-tyle test(new name zj_ft1,zj_ft1() will work))
FINSH_FUNCTION_EXPORT_ALIAS(zj_t1, __cmd_zj_t1, this is finsh msh test)


int var;
int zj_t2(int a)
{
#ifdef FINSH_USING_MSH    
    if(msh_is_used())
     {
        rt_kprintf("Welcome to use ZJ-SDK! this num is %d(%d), this is finsh msh test with args\n", a++,var);
     }
     else
#endif
     {
        rt_kprintf("Welcome to use ZJ-SDK! this num is %d(%d), this is finsh c-tyle test with args\n", a++,var);
     }

    var = a;
    return a;
}
FINSH_FUNCTION_EXPORT(zj_t2, this is finsh c-tyle test with args(zj_t2(number) will work))
FINSH_VAR_EXPORT(var, finsh_type_int, just a var for zj_t2)
FINSH_FUNCTION_EXPORT_ALIAS(zj_t2, zj_ft2, this is finsh c-tyle test with args(new name zj_t2,zj_ft2(number) will work))

FINSH_FUNCTION_EXPORT_ALIAS(zj_t2, __cmd_zj_t2, this is finsh msh test with args(zj_t2 number will work))

/*msh*/
int zj_t3(int argc, char** argv)
{
    rt_kprintf("Welcome to use ZJ-SDK! this is finsh msh test with args\n");
    for(rt_uint8_t i=0;i<argc;i++)
    {
       rt_kprintf("argv[%d]: %s\n",i, argv[i]);
    }
    return 0;
}
MSH_CMD_EXPORT(zj_t3, this is finsh msh test with args(zj_t3 argv1 argv2...));
