#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/proc_fs.h> 
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Display the current time & the amount of time that's passed since the last call on each process read.");

#define MODULE_NAME "my_timer"
#define MODULE_PERMISSION 0644
#define MODULE_PARENT NULL
#define STRING_LENGTH 256

static struct file_operations FO;
static char *msg1;
static char *msg2;
static int read_Proc;
static int proc_Read_Number = 0;
struct timespec current_Time;
long elapsed_Seconds;
long elapsed_NanoSeconds;

static struct Time {
    long sec;
    long nano_sec;
} time;

int Open_Proc(struct inode *proc_inode, struct file *proc_file) {
    printk(KERN_INFO "Open_Proc() is called.\n");
    read_Proc = 1;

    proc_Read_Number++;
    
    msg1 = kmalloc(sizeof(char) * STRING_LENGTH, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    msg2 = kmalloc(sizeof(char) * STRING_LENGTH, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    if (msg1 || msg2 == NULL) {
        printk(KERN_WARNING "Error -- could not allocate memory. ");

        return -ENOMEM;
    }
    if (current_time != 0)
    {
    current_Time = current_kernel_time();
    sprintf(msg1, "Current time : %ld.%ld\n", current_Time.tv_sec, current_Time.tv_nsec);

    if (current_Time.tv_nsec - time.nano_sec < 0) {
        elapsed_Seconds = current_Time.tv_sec - time.sec - 1;
        elapsed_NanoSeconds = current_Time.tv_nsec - time.nano_sec + 1000000000;
    } else {
        elapsed_Seconds = current_Time.tv_sec - time.sec;
        elapsed_NanoSeconds = current_Time.tv_nsec - time.nano_sec;
    }

    if (proc_Read_Number > 1) {
        sprintf(msg2, "Elapsed time : %ld.%ld\n", elapsed_Seconds, elapsed_NanoSeconds);
        strcat(msg1, msg2);
    }

    time.sec = current_Time.tv_sec;
    time.nano_sec = current_Time.tv_nsec;
    
    return 0;
}

ssize_t Read_Proc(struct file *proc_file, char __user *buffer, size_t size, loff_t *offset) {
    int length = strlen(msg1);

    read_Proc = !read_Proc;
    if (read_Proc) {
        return 0;
    }

    printk(KERN_INFO "Read_Proc() is called.\n");
    copy_to_user(buffer, msg1, length);

    return length;
}

int Release_Proc(struct inode *proc_inode, struct file *proc_file) {
    printk(KERN_NOTICE "Release_Proc() is called.\n");
    kfree(msg1);
    kfree(msg2);

    return 0;
}

static int InitializeModule(void) {
    printk(KERN_NOTICE "Creating /proc/timer entry...\n");

    FO.open = Open_Proc;
    FO.read = Read_Proc;
    FO.release = Release_Proc;

    if (!proc_create(MODULE_NAME, MODULE_PERMISSIONS, MODULE_PARENT, &FO)) {
        printk(KERN_WARNING "Error -- could not create the proc entry.\n");
        remove_proc_entry(MODULE_NAME, MODULE_PARENT);

        return -ENOMEM;
    }

    return 0;
}

static void ExitModule(void) {
    printk(KERN_NOTICE "Removing /proc/timer entry...\n");
    remove_proc_entry(MODULE_NAME, MODULE_PARENT);
}

module_init(InitializeModule);
module_exit(ExitModule);
