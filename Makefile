all:
	arm-linux-gcc src/*.c -o main -L ./lib/jpeg_lib/ -ljpeg -L ./lib/camera_lib/ -lapi_v4l2_arm1 -lpthread
	arm-linux-gcc ubuntu_src/udp_ubuntu.c -o backend -lpthread
	# gcc ubuntu_src/udp_ubuntu_recv.c -o backend_recv -lpthread

clean:
	rm main udp_ubuntu
