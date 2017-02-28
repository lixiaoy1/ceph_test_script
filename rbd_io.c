#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rados/librados.h>
#include <rbd/librbd.h>

const int TEST_IO_SIZE = 512;

void test_aio_write(rbd_image_t image, const char *test_data, uint64_t off, size_t len)
{
        rbd_completion_t comp;
        int err = rbd_aio_create_completion(NULL, NULL, &comp);
        if(err < 0)
        {
            fprintf(stderr, "Cannot create comp: %s\n", strerror(-err));
            return;
        }
        rbd_aio_write(image, off, len, test_data, comp);
        printf("Started write\n");
        rbd_aio_wait_for_complete(comp);
        int r = rbd_aio_get_return_value(comp);
        printf("return value is: %d\n", r);
        printf("finished write\n");
        rbd_aio_release(comp);
}

void test_aio_read(rbd_image_t image, char *read_buf, uint64_t off, size_t len)
{
        rbd_completion_t comp;
        int err = rbd_aio_create_completion(NULL, NULL, &comp);
        if(err < 0)
        {
            fprintf(stderr, "Cannot create comp: %s\n", strerror(-err));
            return;
        }
        rbd_aio_read(image, off, len, read_buf, comp);
        printf("Started read\n");
        rbd_aio_wait_for_complete(comp);
        int r = rbd_aio_get_return_value(comp);
        printf("return value is: %d\n", r);
        printf("\nThe data is:\n");
        int i = 0;
        for(; i<r; i++)
             printf("%c", read_buf[i]);
        printf("\n");

        rbd_aio_release(comp);
}

int main (int argc, const char* argv[])
{

        /* Declare the cluster handle and required arguments. */
        rados_t cluster;
        //char cluster_name[] = "ceph";
        char cluster_name[] = "6ff29c93-4568-40a4-a8dc-e2749f4ae22c";
        char user_name[] = "client.admin";
        uint64_t flags;

        /* Initialize the cluster handle with the "ceph" cluster name and the "client.admin" user */
        int err;
        err = rados_create2(&cluster, cluster_name, user_name, flags);

        if (err < 0) {
                fprintf(stderr, "%s: Couldn't create the cluster handle! %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated a cluster handle.\n");
        }

        /* Read a Ceph configuration file to configure the cluster handle. */
        //err = rados_conf_read_file(cluster, "/etc/ceph/ceph.conf");
        err = rados_conf_read_file(cluster, "/opt/ceph_test_script/ceph.conf");
        if (err < 0) {
                fprintf(stderr, "%s: cannot read config file: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the config file.\n");
        }

        /* Read command line arguments */
        err = rados_conf_parse_argv(cluster, argc, argv);
        if (err < 0) {
                fprintf(stderr, "%s: cannot parse command line arguments: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the command line arguments.\n");
        }

        /* Connect to the cluster */
        err = rados_connect(cluster);
        if (err < 0) {
                fprintf(stderr, "%s: cannot connect to cluster: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nConnected to the cluster.\n");
        }

	/*
         * Continued from previous C example, where cluster handle and
         * connection are established. First declare an I/O Context.
         */

        rados_ioctx_t io;
        //char *poolname = "data";
        char *poolname = "rbd";

        err = rados_ioctx_create(cluster, poolname, &io);
        if (err < 0) {
                fprintf(stderr, "%s: cannot open rados pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_shutdown(cluster);
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated I/O context.\n");
        }

        rbd_image_t image;
        err = rbd_open(io, "rbd0", &image, NULL);
        if (err < 0) {
		fprintf(stderr, "%s: cannot open image rbd0: %s\n", argv[0], strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(EXIT_FAILURE);
        } else {
                printf("\nOpen image.\n");
        }

        char write_buf[TEST_IO_SIZE+1];
        int i;
        for (i = 0; i < TEST_IO_SIZE; ++i)
            write_buf[i] = (char) (rand() % (126 - 33) + 33);
        write_buf[TEST_IO_SIZE] = '\0';
       
        test_aio_write(image, write_buf, 0, TEST_IO_SIZE);

        char read_buf[TEST_IO_SIZE+1];
        test_aio_read(image, read_buf, 0, TEST_IO_SIZE);

        /* Release the asynchronous I/O complete handle to avoid memory leaks. */
        rbd_close(image);
        rados_ioctx_destroy(io);
        rados_shutdown(cluster);
}
