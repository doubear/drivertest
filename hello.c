#include<linux/init.h>
#include<linux/module.h>
#include<linux/of.h>
#include<linux/sched.h>
#include<linux/moduleparam.h>
#include<linux/slab.h>
#include<linux/vmalloc.h>
#include<linux/mm.h>
#include<linux/dma-mapping.h>
#include<linux/percpu.h>


static char *whom = "world";
static int howmany = 1;
DEFINE_PER_CPU(int,testcpu);

module_param(howmany,int,S_IRUGO);
module_param(whom,charp,S_IRUGO);

MODULE_LICENSE("GPL");

static int n = 0;
struct slab_test
{
	int val;
};
struct kmem_cache *test_cachep = NULL;
struct slab_test *object1,*object2;
void slab_ctor(void *cachep)
{
//	if (n == 255||n == 256)
//		printk(KERN_ALERT "slab_ctor is called object:%d\n",n);
	n++;
}

static int __init hello_init(void)
{
	static int i = 0;	
	unsigned char * normal_virt,*vmalloc_virt;
	phys_addr_t phys;
	unsigned long pfn;
//	struct device_node *node;
	
	printk(KERN_ALERT "test hello module ok\n");
	printk(KERN_ALERT "the process is %s pid %i\n",current->comm,current->pid);
	for(i = 0;i<howmany;i++){
		printk(KERN_ALERT "%d hello %s\n",i,whom);
	}
	/*test kmalloc*/
	normal_virt = kmalloc(64,GFP_KERNEL);
	phys = virt_to_phys(normal_virt);
	printk(KERN_ALERT "kmalloc virt: %p <==>phys:%16.16llx\n",normal_virt,phys);
	kfree(normal_virt);

	/*test kmem_cache*/
	test_cachep = kmem_cache_create("test_cachep",sizeof(struct slab_test),0,SLAB_HWCACHE_ALIGN,slab_ctor);
	printk(KERN_ALERT "slab_ctor is called object:%d\n",n);
	
	object1 = kmem_cache_alloc(test_cachep,GFP_KERNEL);
	printk(KERN_ALERT "slab_ctor is called object:%d\n",n);
	object2 = kmem_cache_alloc(test_cachep,GFP_KERNEL);
	kmem_cache_free(test_cachep,object1);
	kmem_cache_free(test_cachep,object2);	
	kmem_cache_destroy(test_cachep);
	
	/* test vmalloc*/
	vmalloc_virt = vmalloc(128);
	pfn = vmalloc_to_pfn(vmalloc_virt);
	phys = (pfn<<12)|((unsigned long)vmalloc_virt & 0xffffffff);
	normal_virt = phys_to_virt(phys);
	printk(KERN_ALERT "vmalloc:%p,phys:%16.16llx,normal:%p\n",vmalloc_virt,phys,normal_virt);
	
	/*dma-vmalloc*/
	/*
	normal_virt = dma_alloc_coherent(NULL,1024,&phys,GFP_KERNEL);
	printk(KERN_ALERT "dma:vmalloc:%p,normal_virt = %p\n",vmalloc_virt,phys);
	dma_free_coherent(NULL,1024,normal_virt,phys);
	*/
	vfree(vmalloc_virt);
	
	/*test percpu*/
//	get_cpu_var(testcpu) = 10086 ;
//	put_cpu_var(testcpu);
	per_cpu(testcpu,1) = 10086;
	per_cpu(testcpu,0) = 10010;
	printk(KERN_ALERT "CPU0:%d  CPU1:%d\n",per_cpu(testcpu,1),per_cpu(testcpu,0));
/* useless*/
/*	node = of_find_all_nodes(NULL);
	while(node)
	{
		node = of_find_all_nodes(node);
		printk(KERN_ALERT "%s\n",node->name);
	}
*/
	return 0;
}
static void __exit hello_exit(void)
{
	printk(KERN_ALERT "exit hello ok\n");
	
}

module_init(hello_init);
module_exit(hello_exit);
