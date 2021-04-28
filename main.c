
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <zbar.h>

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/
#define CAPTURE_IMAGE_WIDTH		640
#define CAPTURE_IMAGE_HEIGHT	480
/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/
unsigned char *outputBuf;

/*----------------------------------------------------------------------------
 *        Functions
 *----------------------------------------------------------------------------*/
int Zbar_Test(void* raw, int width, int height);

static int xioctl(int fd, int request, void *arg)
{
        int r;
 
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
 
        return r;
}

int init_caps(int fd)
{
	struct v4l2_format fmt = {0};
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = CAPTURE_IMAGE_WIDTH;
	fmt.fmt.pix.height = CAPTURE_IMAGE_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
	{
		printf("Setting Pixel Format\r\n");
		return 1;
	}

	return 0;
}

int init_mmap(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
 
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        printf("Requesting Buffer\r\n");
        return 1;
    }
 
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
    {
        printf("Querying Buffer\r\n");
        return 1;
    }
 
	outputBuf = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

    return 0;
}

int capture_image(int fd)
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

	if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    {
        printf("Query Buffer\n");
        return 1;
    }
 
   if(-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type))
    {
        printf("Start Capture\n");
        return 1;
    }
 
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 2;
    int r = select(fd+1, &fds, NULL, NULL, &tv);
    if(-1 == r)
    {
        printf("Waiting for Frame\n");
        return 1;
    }
 
    if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
    {
        printf("Retrieving Frame\n");
        return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/


int camProc(char *camDevice)
{
	int fd;

	fd = open(camDevice, O_RDWR);
    if (fd == -1)
    {
            printf("Opening video device\r\n");
            return 1;
    }

	if( init_caps(fd) )
        return 1;
    
    if(init_mmap(fd))
        return 1;

    capture_image(fd);

    close(fd);
}


// return value
// > 0 if found data
int zbarProc(void* raw, int width, int height)
{
	zbar_image_scanner_t *scanner = NULL;
	
    /* create a reader */
    scanner = zbar_image_scanner_create();
    /* configure the reader */
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);
    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);
    /* scan the image for barcodes */
 	int n = zbar_scan_image(scanner, image);
    printf("n = %d\r\n", n);
	
    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        const char *data = zbar_symbol_get_data(symbol);
        printf("%s\n", data);
    }

    /* clean up */
    zbar_image_scanner_destroy(scanner);

    return n;
}      

extern int main(void)
{
	printf("\r\nZBAR Example....Start\r\n");
	printf("----------------------------------------\r\n");

	camProc("/dev/video0");	
	zbarProc((void* )outputBuf, CAPTURE_IMAGE_WIDTH, CAPTURE_IMAGE_HEIGHT);
	
	printf("----------------------------------------\r\n");
	printf("\r\nZBAR Example....Finished\r\n");	
}
