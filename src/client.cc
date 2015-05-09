#include <stddef.h>
#include <stdint.h>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <list>
#include <ev.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "proto.h"

struct Context {
	int					sock;
	size_t				ping_count;
	size_t				pong_count;
	double				last_ping;
	struct sockaddr_in	remote;


	char				buffer[1024];

	Context() : sock(-1),ping_count(0),pong_count(0),last_ping(0.0f) { }
};


static int init_context(Context* ctx);
static void on_signal_cb(struct ev_loop* loop,ev_signal* watcher,int e);
static void on_sock_readable(struct ev_loop *loop,ev_io* watcher,int e);
static void on_timer_cb(struct ev_loop* loop,ev_timer* watcher,int e);
static uint32_t get_ms_diff(struct ev_loop* loop,double last);
static void send_ping(struct ev_loop* loop,Context* ctx);


DEFINE_bool(daemon,false,"start as daemon");
DEFINE_string(address,"0.0.0.0","listen address");
DEFINE_int32(port,8972,"listen port");
DEFINE_int32(count,4,"ping times");
DEFINE_double(period,1.0,"ping period");
DEFINE_double(timeout,5.0,"timeout time");

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

	ev_timer_init(&w_timer,on_timer_cb,0.02,FLAGS_period);
	w_timer.data = &ctx;
	ev_timer_start(loop,&w_timer);

	std::cout << argv[0] << " start with remote " << FLAGS_address << ":" << FLAGS_port << std::endl;
	LOG(INFO) << argv[0] << " start with remote " << FLAGS_address << ":" << FLAGS_port;

	send_ping(loop,&ctx);
	ev_run(loop,0);

	ev_loop_destroy(loop);
	close(ctx.sock);

	std::cout << argv[0] << " exit" << std::endl;
	LOG(INFO) << argv[0] << " exit";
	google::ShutdownGoogleLogging();
	return 0;
}


static int init_context(Context* ctx) {
	int err = 0;

	memset(&ctx->remote,0,sizeof(ctx->remote));
	ctx->remote.sin_family = AF_INET;
	if( -1 == inet_aton(FLAGS_address.c_str(),&ctx->remote.sin_addr) ) {
		std::cerr << "bad remote ip: " << FLAGS_address << std::endl;
		LOG(FATAL) << "bad remote ip: " << FLAGS_address;
		return -1;
	}
	if( FLAGS_port <= 0 || FLAGS_port >= UINT16_MAX ) {
		std::cerr << "bad remote port: " << FLAGS_port << std::endl;;
		LOG(FATAL) << "bad remote port: " << FLAGS_port;
		return -1;
	}
	ctx->remote.sin_port = htons((uint16_t)( ((uint32_t)FLAGS_port) & 0x0000FFFF ));
	ctx->sock = socket(AF_INET,SOCK_DGRAM,0);
	if( -1 == ctx->sock ) {
		err = errno;
		std::cerr << "Can not create socket : errno =" << err << std::endl;
		LOG(FATAL) << "Can not create socket : errno =" << err;
		return -1;
	}

	return 0;
}

static void on_signal_cb(struct ev_loop* loop,ev_signal* watcher,int e) {
	(void)&watcher;
	(void)&e;
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
		std::cerr << "Recv unknown package " << readbytes << " bytes from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port) << std::endl;
		LOG(WARNING) << "Recv unknown package " << readbytes << " bytes from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port);
		return;
	}


	if( msg->GetDescriptor() == echo::Ping::descriptor() ) {
		std::cout << "recv Ping from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port) << std::endl;
		LOG(INFO) << "recv Ping from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port);
		send_pong(ctx->sock,dynamic_cast<echo::Ping*>(msg),&addr);
	} else if( msg->GetDescriptor() == echo::Pong::descriptor() ) {
		echo::Pong* pong = dynamic_cast<echo::Pong*>(msg);
		ctx->pong_count++;
		std::cout << ctx->pong_count << "\trecv Pong from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port) << " in " << get_ms_diff(loop,pong->timestamp()) << " ms" << std::endl;
		LOG(INFO) << ctx->pong_count << "\trecv Pong from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port) << " in " << get_ms_diff(loop,pong->timestamp()) << " ms";

		if( ctx->pong_count >= (size_t) FLAGS_count ) {
			ev_break(loop,EVBREAK_ALL);
		}
	} else {
		std::cout << "recv message " << msg->GetTypeName() << " from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port) << std::endl;
		LOG(INFO) << "recv message " << msg->GetTypeName() << " from " << inet_ntoa(addr.sin_addr) << ":" << htons(addr.sin_port);
	}

	delete msg;
}

static void on_timer_cb(struct ev_loop* loop,ev_timer* watcher,int e) {
	const double now = ev_now(loop);
	Context* ctx = (Context*) watcher->data;

	if( ctx->ping_count < (size_t)FLAGS_count ) {
		send_ping(loop,ctx);
	} else {
		if( now - ctx->last_ping >= FLAGS_timeout ) {
			ev_break(loop,EVBREAK_ALL);
		}
	}
}


static uint32_t get_ms_diff(struct ev_loop* loop,double last) {
	return (uint32_t) ((ev_now(loop) - last) * 1000);
}


static void send_ping(struct ev_loop* loop,Context* ctx) {
	echo::Ping ping;
	const double now = ev_now(loop);

	ctx->last_ping = now;
	ctx->ping_count++;
	ping.set_timestamp(now);
	std::string pkg = pack_message(&ping);
	sendto(ctx->sock,pkg.c_str(),pkg.size(),0,(const struct sockaddr*)&ctx->remote,sizeof(struct sockaddr_in));
}

