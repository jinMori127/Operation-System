#include <linux/ctype.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include "encdec.h"

#define MODULE_NAME "encdec"
//last
MODULE_LICENSE("GPL");
MODULE_AUTHOR("YOUR NAME");

int 	encdec_open(struct inode *inode, struct file *filp);
int 	encdec_release(struct inode *inode, struct file *filp);
int 	encdec_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

ssize_t encdec_read_caesar( struct file *filp, char *buf, size_t count, loff_t *f_pos );
ssize_t encdec_write_caesar(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

ssize_t encdec_read_xor( struct file *filp, char *buf, size_t count, loff_t *f_pos );
ssize_t encdec_write_xor(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

int memory_size = 0;

MODULE_PARM(memory_size, "i");

int major = 0;
char* Caesae_Cipher_Buffer=NULL ;
char* XOR_Buffer=NULL;

struct file_operations fops_caesar = {
	.open 	 =	encdec_open,
	.release =	encdec_release,
	.read 	 =	encdec_read_caesar,
	.write 	 =	encdec_write_caesar,
	.llseek  =	NULL,
	.ioctl 	 =	encdec_ioctl,
	.owner 	 =	THIS_MODULE
};

struct file_operations fops_xor = {
	.open 	 =	encdec_open,
	.release =	encdec_release,
	.read 	 =	encdec_read_xor,
	.write 	 =	encdec_write_xor,
	.llseek  =	NULL,
	.ioctl 	 =	encdec_ioctl,
	.owner 	 =	THIS_MODULE
};

// Implemetation suggestion:
// -------------------------
// Use this structure as your file-object's private data structure
typedef struct {
	unsigned char key;
	int read_state;
} encdec_private_date;

int init_module(void)
{
	major = register_chrdev(major, MODULE_NAME, &fops_caesar);
	if(major < 0)
	{
		return major;
	}

	Caesae_Cipher_Buffer=kmalloc(memory_size,GFP_KERNEL);
	if(Caesae_Cipher_Buffer==NULL)
	{
		printk("can't allocate memory \n");
    	unregister_chrdev(major, MODULE_NAME);
		return -1 ;
	}

	XOR_Buffer=kmalloc(memory_size,GFP_KERNEL);

	if(XOR_Buffer==NULL)
	{
		printk("can't allocate memory \n");
		kfree(Caesae_Cipher_Buffer);
    	unregister_chrdev(major, MODULE_NAME);
		return -1 ;
	}

	return 0;
}

void cleanup_module(void)
{
	unregister_chrdev(major, MODULE_NAME);
	kfree(Caesae_Cipher_Buffer);
	kfree(XOR_Buffer);

}

int encdec_open(struct inode *inode, struct file *filp)
{
	int minor = MINOR(inode->i_rdev);

	if(minor==0)
	{
		filp->f_op= &fops_caesar;
	}
	else if(minor==1)
	{
		filp->f_op= &fops_xor;
	}
	else      return -1;

if ((filp->private_data = (encdec_private_date*)kmalloc(sizeof(encdec_private_date), GFP_KERNEL)) == NULL)	{
		printk("can't allocate memory \n");
		cleanup_module();
		return -1 ;
	}
    ((encdec_private_date*)filp->private_data)->read_state=0;
	((encdec_private_date*)filp->private_data)->key=0;
	// Implemetation suggestion:
	// -------------------------
	// 1. Set 'filp->f_op' to the correct file-operations structure (use the minor value to determine which)
	// 2. Allocate memory for 'filp->private_data' as needed (using kmalloc)

	return 0;
}

int encdec_release(struct inode *inode, struct file *filp)
{

	if(filp->private_data!=NULL)
	{
		kfree(filp->private_data);
	}
	// Implemetation suggestion:
	// -------------------------
	// 1. Free the allocated memory for 'filp->private_data' (using kfree)

	return 0;
}


int encdec_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int minor;
	switch (cmd)
	{
	case ENCDEC_CMD_CHANGE_KEY:
        ((encdec_private_date*)filp->private_data)->key=arg;

		break;

	case ENCDEC_CMD_SET_READ_STATE:
		((encdec_private_date*)filp->private_data)->read_state=arg;
		break;


	case ENCDEC_CMD_ZERO:
         minor = MINOR(inode->i_rdev);
        if(minor==0){
            memset(Caesae_Cipher_Buffer,0,memory_size);
        }
        else if(minor==1){
            memset(XOR_Buffer,0,memory_size);
        }


		break;
	default:
		return -EINVAL ;
		break;
	}



	return 0;
}

ssize_t encdec_read_caesar( struct file *filp, char *buf, size_t count, loff_t *f_pos )
{

	if(*f_pos>=memory_size)
	{
		return -EINVAL ;
	}

    int key= ((encdec_private_date*)filp->private_data)->key;

	size_t bytes_to_copy = min(count, memory_size - (size_t)*f_pos);
	size_t not_copied=copy_to_user(buf,Caesae_Cipher_Buffer+(*f_pos),bytes_to_copy);

	*f_pos = min((loff_t)memory_size, *f_pos + (loff_t)(bytes_to_copy - not_copied));

if(((encdec_private_date*)filp->private_data)->read_state==ENCDEC_READ_STATE_DECRYPT){
        int i=0;
    for(i=0 ;i<(bytes_to_copy - not_copied) ;i++)
	              buf[i] =(((buf[i]-key )+128) % 128) ;
}

	return bytes_to_copy - not_copied;

}


ssize_t encdec_write_caesar(struct file *filp, const char *buf, size_t count, loff_t *f_pos){
int i;
if(*f_pos==memory_size) return -ENOSPC;

    int key= ((encdec_private_date*)filp->private_data)->key;

	size_t bytes_to_copy = min(count, memory_size - (size_t)*f_pos);




   int num_can_copy=(bytes_to_copy-copy_from_user(Caesae_Cipher_Buffer+(*f_pos),buf,bytes_to_copy));
for ( i=0 ; i<bytes_to_copy ; i++){
        Caesae_Cipher_Buffer[i]=(Caesae_Cipher_Buffer[i]+key) %128;
    }
    *f_pos += num_can_copy;

    return num_can_copy;
}



ssize_t encdec_read_xor( struct file *filp, char *buf, size_t count, loff_t *f_pos )
{
	int i=0 ;

	if(*f_pos>=memory_size)
	{
		return -EINVAL ;
	}

	size_t bytes_to_copy = min(count, memory_size - (size_t)*f_pos);
	size_t not_copied=copy_to_user(buf,XOR_Buffer+(*f_pos),bytes_to_copy);

	*f_pos = min((loff_t)memory_size, *f_pos + (loff_t)(bytes_to_copy - not_copied));

 if(((encdec_private_date*)filp->private_data)->read_state==ENCDEC_READ_STATE_DECRYPT){
      for(i=0 ;i<(bytes_to_copy - not_copied) ;i++)
                 buf[i]=buf[i] ^ ((encdec_private_date*)filp->private_data)->key;
                 }


	return bytes_to_copy - not_copied;



}


ssize_t encdec_write_xor(struct file *filp, const char *buf, size_t count, loff_t *f_pos){

int i;
if(*f_pos==memory_size) return -ENOSPC;

    int key= ((encdec_private_date*)filp->private_data)->key;
    size_t bytes_to_copy = min(count, (size_t)(memory_size - (size_t)*f_pos));

   int num_can_copy=(bytes_to_copy-copy_from_user(XOR_Buffer+(*f_pos),buf,bytes_to_copy));

   for( i=0 ; i<count ; i++){
        XOR_Buffer[i]=XOR_Buffer[i]^key;
    }
    *f_pos += num_can_copy;

    return num_can_copy;
}
