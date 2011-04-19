/**
 * loader ...
 */

#include "nxc_vm.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #define CRTDBG_MAP_ALLOC
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#else
    #define _CrtDumpMemoryLeaks()
#endif






/**===========================================================================*/
void *xmalloc(void*ctx,int size)
{
    void *ret = malloc(size);
    (void)ctx;
    return ret;
}

void xfree(void*ptr,int size)
{
    ///memset(ptr,0,size);
    free(ptr);
}


#define __arg(idx)   (ctx->esp[(idx)])
#define __parg(idx)  (&ctx->esp[(idx)])

/**
 * format : int printf(char *fmt,...);
 */
void do_printf(nxc_vm_ctx_t *ctx)
{
    ctx->eax = (long)vprintf((char *)__arg(0),(char *)__parg(1));
}

/**
 * format : int printf(char *fmt,...);
 */
void do_print_float(nxc_vm_ctx_t *ctx)
{
    printf("%f",*(float *)__parg(0));
}

/**
 * init or get global memory pool ...
 */
nxc_mempool_t *get_global_mempool()
{
    static nxc_mempool_t __global_mem_pool={0};
    
    if(!__global_mem_pool.do_malloc)
    {
        nxc_init_mpool(&__global_mem_pool,xmalloc,xfree,0);
    }
    return &__global_mem_pool;
}

/**
 * load file into mem with given limit size ...
 * @return memory allocated that hold the data ...
 */
char *nxc_load_file(char *path,int max_sz,int *bytes_read)
{
    FILE *fp;
    char *ptr;
    int ret;
    ptr = (char *)nxc_pmalloc(get_global_mempool(),max_sz+1);
    if(!ptr)
    {
        return 0;
    }
    ptr[max_sz] = 0;
    
    fp = fopen(path,"rb");
    if(fp)
    {
        ret = fread(ptr,1,max_sz,fp);
        
        fclose(fp);
        
        if(ret<=0)
        {
            nxc_pfree(ptr);
            return 0;
        }
        if(bytes_read)
        {
            *bytes_read = ret;
        }
        ptr[ret] = 0;
        return ptr;
    }
    else
    {
        nxc_pfree(ptr);
    }
    return 0;
}

/**
 * vm's output callback ...
 */
int output_gw(nxc_compiler_t *compiler,const char *fmt,va_list args)
{
    return vprintf(fmt,args);
}

nxc_script_image_t *dup_image(nxc_script_image_t *image)
{
    char *ptr;
    char* another;

    another = malloc(sizeof(*image) + image->data_seg_size + image->text_seg_size);
    ptr = (char*)another;
    nxc_memcpy(ptr,image,sizeof(*image));

    ptr += sizeof(*image);
    //copy data segment ...
    nxc_memcpy(ptr,image->data_base,image->data_seg_size);

    ptr += image->data_seg_size;
    //copy text segment ...
    nxc_memcpy(ptr ,image->text_base,image->text_seg_size);

    return (nxc_script_image_t*)another;
}

/**
 * just demostrate how relocating (leak)...
 */
int main_compile_and_reloc(int argc,char *argv[])
{
    int ret;
    char *str1;
    nxc_script_image_t *image;
    nxc_script_image_t *another;
    void *addr;

    nxc_api_table_t api_table[1];
    
    ret = nxc_init_api_table(api_table,xmalloc,xfree,0);
    if (nxc_register_api(api_table,"printf",do_printf))
    if (nxc_register_api(api_table,"print_float",do_print_float))
    ///load script max 8KB ...
    str1 = nxc_load_file("./test.c",8*1024,0);

    ///compile script ...
    //image = nxc_do_compile(compiler,"test.c",str1);
    image = nxc_compile_ex(str1,"test.c",xmalloc,xfree,0,(nxc_vprintf_t)output_gw);

    ///release script memory ...
    nxc_pfree(str1);

    ///show error if necessary
    if(!image)
    {
        printf("============compile error===========\n");
        ///leak here , see below 'main' to fix ...
        return 0;
    }
    else
    {
        printf("===========compiler okay=========\n");
    }

    ///dup image ...
    another = dup_image(image);

    nxc_script_image_do_destroy(image,xfree);

    ///do relocation ...
    nxc_script_image_do_reloc(another,(long)another + sizeof(*another),(long)another + sizeof(*another) + another->data_seg_size);

    nxc_script_image_do_fix_api(another/*image*/,api_table);

    addr = nxc_script_image_do_find_sym_addr(another,"main");
    if (addr)
    {
        printf("find -------------main!!!\n");
    }

    nxc_call((nxc_instr_t *)addr,2,1,"aaa");    

    nxc_script_image_do_destroy(another,xfree);

    nxc_fini_api_table(api_table);

//  return 0;

    
    _CrtDumpMemoryLeaks();
    return 0;
}


int main(int argc,char *argv[])
{
    int ret;
    char *str1;
    nxc_script_image_t *image;
    void *addr;
    
    nxc_api_table_t api_table[1];
    
    ret = nxc_init_api_table(api_table,xmalloc,xfree,0);
    if (nxc_register_api(api_table,"printf",do_printf))
    {
        printf("failed register api!!!!!!!!!!!!!!!!!!\n");
    }
    if (nxc_register_api(api_table,"print_float",do_print_float))
    {
        printf("failed register api!!!!!!!!!!!!!!!!!!\n");
    }
    ///load script max 8KB ...
    str1 = nxc_load_file("./test.c",8*1024,0);
    
    ///compile script ...
    //image = nxc_do_compile(compiler,"test.c",str1);
    image = nxc_compile_ex(str1,"test.c",xmalloc,xfree,0,(nxc_vprintf_t)output_gw);
    
    ///release script memory ...
    nxc_pfree(str1);
    
    ///show error if necessary
    if(!image)
    {
        printf("============compile error===========\n");
        
        ///destroy api-table ...
        nxc_fini_api_table(api_table);

        ///check memory leak ..
        _CrtDumpMemoryLeaks();
        return 0;
    }
    else
    {
        printf("===========compiler okay=========\n");
    }
    
    ///fix Import-table ...
    nxc_script_image_do_fix_api(image,api_table);
    
    ///find symbol by name ...
    addr = nxc_script_image_do_find_sym_addr(image,"main");
    if (addr)
    {
        printf("find -------------main!!!\n");
    }
    
    ///call target function-symbol ...
    nxc_call((nxc_instr_t *)addr,2,1,"aaa");    
    
    ///release image memory ...
    nxc_script_image_do_destroy(image,xfree);
    
    ///destroy api-table ...
    nxc_fini_api_table(api_table);
    
    ///check memory leak ...
    _CrtDumpMemoryLeaks();
    return 0;
}
