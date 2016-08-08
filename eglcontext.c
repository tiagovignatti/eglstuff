/*
 * Copyright © 2016 Tiago Vignatti
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <gbm.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	EGLDisplay dpy;
	EGLContext ctx;
	EGLConfig config;
	EGLint major, minor, n;
	int ret, fd;
	struct gbm_device *gbm;
	char *device_name;
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_DONT_CARE,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 0,
		EGL_DEPTH_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};

	for (int i = 128; i < (128 + 16); i++) {
		asprintf(&device_name, "/dev/dri/renderD%u", i);
		fd = open(device_name, O_RDWR);
		if (fd > 0)
			break;
	}
	if (fd < 0) {
		fprintf(stderr,"Couldn't open drm device.");
		return -1;
	}

	fd = open(device_name, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "couldn't open %s, skipping\n", device_name);
		return -1;
	}
	gbm = gbm_create_device(fd);
	if (gbm == NULL) {
		fprintf(stderr, "couldn't create gbm device\n");
		ret = -1;
		goto close_fd;
	}
	dpy = eglGetDisplay(gbm);
	if (dpy == EGL_NO_DISPLAY) {
		fprintf(stderr, "eglGetDisplay() failed\n");
		ret = -1;
		goto destroy_gbm_device;
	}
	if (!eglInitialize(dpy, &major, &minor)) {
		printf("eglInitialize() failed\n");
		ret = -1;
		goto egl_terminate;
	}
	eglBindAPI(EGL_OPENGL_API);
	if (!eglChooseConfig(dpy, attribs, &config, 1, &n) || n != 1) {
		fprintf(stderr, "failed to choose argb config\n");
		goto egl_terminate;
	}
	ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, NULL);
	if (ctx == NULL) {
		fprintf(stderr, "failed to create context\n");
		ret = -1;
		goto egl_terminate;
	}
	if (!eglMakeCurrent(dpy, NULL, NULL, ctx)) {
		fprintf(stderr, "failed to make context current\n");
		ret = -1;
		goto destroy_gbm_device;
	}

	fprintf(stderr, "GL_RENDERER = %s\n", (const char *) glGetString(GL_RENDERER));
	fprintf(stderr, "GL_VERSION = %s\n", (const char *) glGetString(GL_VERSION));
	fprintf(stderr, "GL_VENDOR = %s\n", (const char *) glGetString(GL_VENDOR));

out:
	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
destroy_context:
	eglDestroyContext(dpy, ctx);
egl_terminate:
	eglTerminate(dpy);
destroy_gbm_device:
	gbm_device_destroy(gbm);
close_fd:
	close(fd);
	return ret;
}
