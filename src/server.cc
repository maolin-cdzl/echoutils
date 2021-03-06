#include <stddef.h>
#include <stdint.h>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ev.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "proto.h"


struct Context {
	int			sock;
	char		buffer[1024];

	Context() : sock(-1) { }
};


static int init_context(Context* ctx);
static void on_signal_cb(struct ev_loop* loop,ev_signal* watcher,int e);
static void on_sock_readable(struct ev_loop *loop,ev_io* watcher,int e);
static void on_timer_cb(struct ev_loop* loop,ev_timer* watcher,int e);

DEFINE_bool(daemon,false,"start as daemon");
DEFINE_string(address,"0.0.0.0","listen address");
DEFINE_int32(port,8972,"listen port");


int main(int argc,char* argv[]) {
	google::ParseCommandLineFlags(&argc,&argv,false);

	if( FLAGS_daemon ) {
		daemon(1,0);
	}
	
	FLAGS_max_log_size=64;
	google::InitGoogleLogging(argv[0]);

	struct Context ctx;
	if( -1 == init_context(&ctx) ) {
		return -1;
	}

	struct ev_loop* loop = ev_default_loop(0);
	struct ev_io w_sock;
	struct ev_timer w_timer;
	struct ev_signal w_signal;

	ev_signal_init(&w_signal,on_signal_cb,SIGINT);
	ev_signal_start(loop,&w_signal);

	ev_io_init(&w_sock,on_sock_readable,ctx.sock,EV_READ);
	w_sock.data = &ctx;
	ev_io_start(loop,&w_sock);

	ev_timer_init(&w_timer,on_timer_cb,0.02,0.02);
	w_timer.data = &ctx;
	ev_timer_start(loop,&w_timer);

	LOG(INFO) << argv[0] << " start listen on " << FLAGS_address << ":" << FLAGS_port;
	ev_run(loop,0);

	ev_loop_destroy(loop);
	close(ctx.sock);

	google::ShutdownGoogleLogging();
	return 0;
}


static int init_context(Context* ctx) {
	int err = 0;
	struct sockaddr_in addr;

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	if( -1 == inet_aton(FLAGS_address.c_str(),&addr.sin_addr) ) {
		LOG(FATAL) << "Bad listen ip: " << FLAGS_address;
		return -1;
	}
	if( FLAGS_port <= 0 || FLAGS_port >= UINT16_MAX ) {
		LOG(FATAL) << "Bad listen port: " << FLAGS_port;
		return -1;
	}
	addr.sin_port = htons((uint16_t)( ((uint32_t)FLAGS_port) & 0x0000FFFF ));

	ctx->sock = socket(AF_INET,SOCK_DGRAM,0);
	if( -1 == ctx->sock ) {
		err = errno;
		LOG(FATAL) << "Can not create socket : errno =" << err;
		return -1;
	}

	if( 0 != bind(ctx->sock,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))) {
		err = errno;
		close(ctx->sock);
		ctx->sock = -1;
		LOG(FATAL) << "Can not bind socket: " << err;
		return -1;
	}

	return 0;
}

static void on_signal_cb(struct ev_loop* loop,ev_signal* watcher,int e) {
	ev_break(loop,EVBREAK_ALL);
}

static void on_sock_readable(struct ev_loop *loop,ev_io* watcher,int e) {
	if( ! (e & EV_READ)  ) {
		return;
	}

	Context* ctx = (Context*) watcher->data;
	ssize_t readbytes = 0;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	readbytes = recvfrom(watcher->fd,ctx->buffer,sizeof(ctx->buffer),0,(struct sockaddr*)&addr,&addrlen);
	if( readbytes <= 0 )
		return;

	google::protobuf::Message* msg = unpack_message(ctx->buffer,readbytes);
	if( ! msg ) {
		LOG(WARNING) << "Recv unknown package " << readbytes << " bytes from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port);
		return;
	}

	LOG(INFO) << "Recv message " << msg->GetTypeName() << " from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port);

	if( msg->GetDescriptor() == echo::Ping::descriptor() ) {
		send_pong(ctx->sock,dynamic_cast<echo::Ping*>(msg),&addr);
	}

	delete msg;
}

static void on_timer_cb(struct ev_loop* loop,ev_timer* watcher,int e) {
}




