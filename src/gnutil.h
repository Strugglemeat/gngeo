/*
 * gnutil.h
 *
 */

#ifndef GNUTIL_H_
#define GNUTIL_H_

char *get_gngeo_dir(void);
void chomp(char *str);
char *my_fgets(char *s, int size, FILE *stream);
char *file_basename(char *filename);
int check_dir(char *dir_name);
void gn_set_error_msg(char *fmt, ...);
void gn_strncat_dir(char *basedir, char *dir, size_t n);

#define GN_TRUE 1
#define GN_FALSE 0

#define CHECK_ALLOC(a) {if (!a) {printf("Out of Memory\n");exit(1);}}

#define GNERROR_SIZE 1024
extern char gnerror[GNERROR_SIZE];

#endif /* GNUTIL_H_ */
