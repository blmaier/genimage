#define _GNU_SOURCE
#include <confuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "genimage.h"

void image_error(struct image *image, const char *fmt, ...)
{
	va_list args;
	char *buf;

	va_start (args, fmt);

	vasprintf(&buf, fmt, args);

	va_end (args);

	fprintf(stderr, "%s(%s): %s", image->handler->type, image->file, buf);

	free(buf);
}

void image_log(struct image *image, int level,  const char *fmt, ...)
{
	va_list args;
	char *buf;

	va_start (args, fmt);

	vasprintf(&buf, fmt, args);

	va_end (args);

	fprintf(stderr, "%s(%s): %s", image->handler->type, image->file, buf);

	free(buf);
}

void error(const char *fmt, ...)
{
	va_list args;

	va_start (args, fmt);

	vfprintf(stderr, fmt, args);

	va_end (args);
}

/*
 * printf wrapper around 'system'
 */
int systemp(struct image *image, const char *fmt, ...)
{
	va_list args;
	char *buf;
	int ret;

	va_start (args, fmt);

	vasprintf(&buf, fmt, args);

	va_end (args);

	if (!buf)
		return -ENOMEM;

	if (image)
		image_log(image, 1, "cmd: %s\n", buf);
	else
		fprintf(stderr, "cmd: %s\n", buf);

	ret = system(buf);

	free(buf);

	return ret;
}

/*
 * xzalloc - safely allocate zeroed memory
 */
void *xzalloc(size_t n)
{
	void *m = malloc(n);

	if (!m) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	memset(m, 0, n);

	return m;
}

/*
 * Like simple_strtoul() but handles an optional G, M, K or k
 * suffix for Gigabyte, Megabyte or Kilobyte
 */
unsigned long long strtoul_suffix(const char *str, char **endp, int base)
{
	unsigned long long val;
	char *end;

	val = strtoull(str, &end, base);

	switch (*end) {
	case 'G':
		val *= 1024;
	case 'M':
		val *= 1024;
	case 'k':
	case 'K':
		val *= 1024;
		end++;
	default:
		break;
	}

	if (endp)
		*endp = (char *)end;

	return val;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

int pad_file(const char *infile, const char *outfile, size_t size,
		unsigned char fillpattern, enum pad_mode mode)
{
	FILE *f = NULL, *outf = NULL;
	void *buf;
	int now, r;
	int ret = 0;

	if (infile) {
		f = fopen(infile, "r");
		if (!f) {
			error("open %s: %s\n", infile, strerror(errno));
			ret = -errno;
			goto err_out;
		}
	}

	outf = fopen(outfile, mode == MODE_OVERWRITE ? "w" : "a");
	if (!outf) {
		error("open %s: %s\n", outfile, strerror(errno));
		ret = -errno;
		goto err_out;
	}

	buf = xzalloc(4096);

	if (!infile) {
		struct stat s;
		ret = stat(outfile, &s);
		if (ret)
			goto err_out;
		if (s.st_size > size) {
			ret = -EINVAL;
			goto err_out;
		}
		size = size - s.st_size;
		goto fill;
	}

	while (size) {
		now = min(size, 4096);

		r = fread(buf, 1, now, f);
		if (r < now)
			goto fill;

		r = fwrite(buf, 1, now, outf);
		if (r < now) {
			ret = -errno;
			goto err_out;
		}

		size -= now;
	}

	now = fread(buf, 1, 1, f);
	if (now == 1) {
		fprintf(stderr, "pad size smaller than input size\n");
		ret = -EINVAL;
		goto err_out;
	}

fill:
	memset(buf, fillpattern, 4096);

	while (size) {
		now = min(size, 4096);

		r = fwrite(buf, 1, now, outf);
		if (r < now) {
			ret = -errno;
			goto err_out;
		}
		size -= now;
	}
err_out:
	if (f)
		fclose(f);
	if (outf)
		fclose(outf);

	return ret;
}