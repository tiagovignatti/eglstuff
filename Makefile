eglkms: 
	clang -g3 -O0 `pkg-config --libs --cflags gl egl gbm libdrm` eglkms.c

eglcontext:
	clang `pkg-config --libs --cflags gl egl gbm libdrm` eglcontext.c
