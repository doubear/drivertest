#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/sched.h>
#include<linux/moduleparam.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/of.h>
#include<linux/timer.h>
#include<linux/jiffies.h>
#include<linux/time.h>
#include<linux/slab.h>

#define TIMER_CNT    1
#define TIMER_NAME  "timer"

MODULE_LICENSE("GPL");


struct timer_dev
{
	dev_t devid;
	int major;
	int minor;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	struct device_node *nd;
	struct timer_list timer;
};

struct timer_dev timerdev;
static int count = 0;
//struct timeval tv_begin,tv_end;

static int timer_open(struct inode *inode,struct file *filp)
{
	filp->private_data = &timerdev;
	return 0;
}

static int timer_release(struct inode *inode,struct file *filp)
{
	return 0;
}

static ssize_t timer_write(struct file *filp,const char __user *buf,size_t count,loff_t *ppos)
{
	int ret = 0;
	return ret;
}

static const struct file_operations timerdev_fops = 
{
	.owner   = THIS_MODULE,
	.write   = timer_write,
	.open    = timer_open,
	.release = timer_release,
};

static void timer_func(struct timer_list *t)
{	
	count ++;
	printk(KERN_ALERT "timer_test ok,count:%d\n",count);
	mod_timer(&timerdev.timer,jiffies + 6*HZ);
//	gettimeofday(&tv_end,NULL);
//	printk(KERN_ALERT "timer ok tv_end.sec:%d tv_end,usec:%d \n",tv_end.tv_sec,tv_end.tv_usec);
}


static int __init timer_init(void)
{
	int ret = 1;
	timerdev.major = 0;
	if(timerdev.major)
	{
		timerdev.devid =MKDEV(timerdev.major,0);
		ret = register_chrdev_region(timerdev.devid,TIMER_CNT,TIMER_NAME);
	}else{
		ret = alloc_chrdev_region(&timerdev.devid,0,TIMER_CNT,TIMER_NAME);
		timerdev.major = MAJOR(timerdev.devid);
		timerdev.minor = MINOR(timerdev.devid);
	}
	if(ret<0)
		goto fail_devid;

	timerdev.cdev.owner = THIS_MODULE;
	cdev_init(&timerdev.cdev,&timerdev_fops);
	
	ret = cdev_add(&timerdev.cdev,timerdev.devid,TIMER_CNT);
	if(ret<0)
		goto fail_cdevadd;
		
	timerdev.class = class_create(THIS_MODULE,TIMER_NAME);
	if(IS_ERR(timerdev.class))
	{
		ret = PTR_ERR(timerdev.class);
		goto fail_class;
	}
	
	timerdev.device = device_create(timerdev.class,NULL,timerdev.devid,NULL,TIMER_NAME);
	if(IS_ERR(timerdev.device))
	{
		ret = PTR_ERR(timerdev.device);
		goto fail_device;
	}

//	gettimeofday(&tv_begin,NULL);
//	printk(KERN_ALERT "timer ok tv_begin.sec:%d tv_begin,usec:%d \n",tv_begin.tv_sec,tv_begin.tv_usec);
	timer_setup(&timerdev.timer,timer_func,0);
//	timerdev.timer.expires = jiffies +msecs_to_jiffies(2000);
	add_timer(&timerdev.timer);

	printk(KERN_ALERT "test timer module ok\n");
	printk(KERN_ALERT "the process is %s pid %i\n",current->comm,current->pid);
	return 0;
fail_device:	
	class_destroy(timerdev.class);
fail_class:
	cdev_del(&timerdev.cdev);
fail_cdevadd:
	unregister_chrdev_region(timerdev.devid,TIMER_CNT);
fail_devid:
	return ret;

}
static void __exit timer_exit(void)
{
	del_timer(&timerdev.timer);
	cdev_del(&timerdev.cdev);
	unregister_chrdev_region(timerdev.devid,TIMER_CNT);
	device_destroy(timerdev.class,timerdev.devid);
	class_destroy(timerdev.class);
	printk(KERN_ALERT "exit timer ok\n");	
}


module_init(timer_init);
module_exit(timer_exit);
