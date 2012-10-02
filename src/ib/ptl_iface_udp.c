/**
 * @file ptl_iface_udp.c
 *
 * @brief Interface support for UDP transport.
 */
#include "ptl_loc.h"

/**
 * Accept an incoming connection request.
 *
 * @param[in] ni
 * @param[in] conn
 * @param[in] msg
 * @param[in] from_addr
 * @param[in] from_addr_len
 *
 * @return status
 *
 * conn is locked
 */
static void accept_udp_connection_request(ni_t *ni, conn_t *conn,
										  struct udp_conn_msg *msg,
										  struct sockaddr_in *from_addr,
										  socklen_t from_addr_len)
{
	struct udp_conn_msg rep;
	int ret;

	conn->state = CONN_STATE_CONNECTING;

	rep.msg_type = UDP_CONN_MSG_REP;
	rep.port = ni->udp.src_port;
	rep.req_cookie = msg->req_cookie;
	rep.rep_cookie = (uintptr_t)conn;

	ret = sendto(ni->iface->udp.connect_s, &rep, sizeof(rep), 0,
				 from_addr, from_addr_len);

	if (ret != sizeof(rep)) {
		WARN();
		conn->state = CONN_STATE_DISCONNECTED;
	}
}

/**
 * Process an incoming connection request event.
 *
 * @param[in] iface
 * @param[in] event
 * @param[in] msg
 * @param[in] from_addr
 * @param[in] from_addr_len
 *
 * @return status
 */
static void process_udp_connect_request(struct iface *iface,
										struct udp_conn_msg *msg,
										struct sockaddr_in *from_addr,
										socklen_t from_addr_len)
{
	conn_t *conn;
	int c;
	ni_t *ni;
	struct udp_conn_msg rep;

	ni = iface->ni[ni_options_to_type(msg->req.options)];

	if (!ni) {
		WARN();
		rep.rej.reason = REJECT_REASON_NO_NI;
		goto reject;
	}

	conn = get_conn(ni, msg->req.src_id);
	if (!conn) {
		WARN();
		rep.rej.reason = REJECT_REASON_ERROR;
		goto reject;
	}

	pthread_mutex_lock(&conn->mutex);

	switch (conn->state) {
	case CONN_STATE_CONNECTED:
		/* We received a connection request but we are already
		 * connected. Ignore. */
		conn_put(conn);
		break;

	case CONN_STATE_DISCONNECTED:
		/* We received a connection request and we are disconnected.
		 * Accept it */
		accept_udp_connection_request(ni, conn, msg, from_addr, from_addr_len);
		break;

	case CONN_STATE_DISCONNECTING:
		/* Not sure how to handle that case. Ignore and disconnect
		 * anyway? */
		abort();
		break;

	case CONN_STATE_CONNECTING:
		/* we received a connection request but we are already connecting
		 * - accept connection from higher id or self
		 * - ignore from lower id
		 */
		c = compare_id(&msg->req.src_id, &ni->id);
		if (c < 0) {
			conn_put(conn);
		}
		else {
			accept_udp_connection_request(ni, conn, msg, from_addr, from_addr_len);
		}
		break;

	case CONN_STATE_RESOLVING_ADDR:
	case CONN_STATE_RESOLVING_ROUTE:
		/* Never for UDP. */
		abort();
	}

	pthread_mutex_unlock(&conn->mutex);

	return;

 reject:
	WARN();
	rep.msg_type = UDP_CONN_MSG_REJ;
	sendto(ni->iface->udp.connect_s, &msg, sizeof(msg), 0,
		   from_addr, from_addr_len);
	
	return;
}

/**
 * Process UDP connection established.
 *
 * @param[in] iface
 * @param[in] event
 *
 * @return status
 */
static void process_udp_connect_established(struct iface *iface,
											struct udp_conn_msg *msg,
											conn_t *conn)
{
	pthread_mutex_lock(&conn->mutex);

	//		atomic_inc(&ni->rdma.num_conn);

	if (conn->state != CONN_STATE_CONNECTING) {
		/* UDP loopback goes here for instance. */
		pthread_mutex_unlock(&conn->mutex);
		return;
	}

	conn->state = CONN_STATE_CONNECTED;

	/* Update the destination address and port. */
	conn->udp.dest_addr = conn->sin;
	conn->udp.dest_addr.sin_port = msg->port;

	flush_pending_xi_xt(conn);

	pthread_mutex_unlock(&conn->mutex);
}

/**
 * Process a UDP connection event.
 *
 * there is a listening socket per iface
 * this is called as a handler from libev
 *
 * @param[in] w
 * @param[in] revents
 */
static void process_udp_connect(EV_P_ ev_io *w, int revents)
{
	struct iface *iface = w->data;
	struct udp_conn_msg msg;
	ssize_t ret;
	struct sockaddr_in from_addr;
	socklen_t from_addr_len;
	conn_t *conn;

	ret = recvfrom(iface->udp.connect_s, &msg, sizeof(msg), MSG_DONTWAIT,
				   (struct sockaddr *)&from_addr, &from_addr_len);

	if (ret == -1)
		return;

	if (ret != sizeof(msg)) {
		WARN();
		return;
	}

	assert(from_addr_len == sizeof(struct sockaddr_in));

	switch(le16_to_cpu(msg.msg_type)) {
	case UDP_CONN_MSG_REQ:
		process_udp_connect_request(iface, &msg, &from_addr, from_addr_len);
		break;
		
	case UDP_CONN_MSG_REP:
		conn = (void *)(uintptr_t)msg.req_cookie;

		/* Send the RTU . */
		struct udp_conn_msg rtu;
		int ret;

		rtu.msg_type = UDP_CONN_MSG_RTU;
		rtu.rep_cookie = msg.rep_cookie;

		ret = sendto(obj_to_ni(conn)->iface->udp.connect_s, &rtu, sizeof(rtu), 0,
					 &from_addr, from_addr_len);

		if (ret != sizeof(rtu)) {
			WARN();
			conn->state = CONN_STATE_DISCONNECTED;
		} else {
			process_udp_connect_established(iface, &msg, conn);
		}

		break;

	case UDP_CONN_MSG_RTU:
		conn = (void *)(uintptr_t)msg.rep_cookie;
		process_udp_connect_established(iface, &msg, conn);
		break;
	}
		   
	return;	
}

/**
 * @brief Get an IPv4 address from network device name (e.g. ib0).
 *
 * Returns INADDR_ANY on error or if address is not assigned.
 *
 * @param[in] ifname The network interface name to use
 *
 * @return IPV4 address as an in_addr_t in network byte order
 */
static in_addr_t get_ip_address(const char *ifname)
{
	int fd;
	struct ifreq devinfo;
	struct sockaddr_in *sin = (struct sockaddr_in*)&devinfo.ifr_addr;
	in_addr_t addr;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0)
		return INADDR_ANY;

	strncpy(devinfo.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(fd, SIOCGIFADDR, &devinfo) == 0 &&
	    sin->sin_family == AF_INET)
		addr = sin->sin_addr.s_addr;
	else
		addr = INADDR_ANY;

	close(fd);

	return addr;
}

/**
 * @brief Initialize interface.
 *
 * @param[in] iface The iface to init
 *
 * @return status
 */
int init_iface_udp(iface_t *iface)
{
	int err;
	in_addr_t addr;

	/* check to see if interface is already initialized. */
	if (iface->udp.connect_s != -1)
		return PTL_OK;

	/* check to see if interface device is present */
	if (if_nametoindex(iface->ifname) == 0) {
		ptl_warn("interface %d doesn't exist\n",
			 iface->iface_id);
		err = PTL_FAIL;
		goto err1;
	}

	/* check to see if interface has a valid IPV4 address */
	addr = get_ip_address(iface->ifname);
	if (addr == INADDR_ANY) {
		ptl_warn("interface %d doesn't have an IPv4 address\n",
			 iface->iface_id);
		err = PTL_FAIL;
		goto err1;
	}

	iface->udp.sin.sin_family = AF_INET;
	iface->udp.sin.sin_addr.s_addr = addr;

	return PTL_OK;

 err1:
	cleanup_iface(iface);
	return err;
}

/**
 * @brief Prepare interface for accepting connections.
 *
 * This routine creates CM ID and binds it to the local IPV4 address
 * and port number. rdma_cm assigns the rdma device and allocates
 * a protection domain.
 *
 * The rdma device has a file descriptor for returning asynchronous
 * events and this is set to non blocking so libev can poll for
 * events.
 *
 * @pre caller should hold gbl mutex.
 *
 * @param[in] iface interface to prepare
 * @param[in] port TCP/RDMA_CM port number in network byte order
 *
 * @return status
 */
static int iface_bind(iface_t *iface, unsigned int port)
{
	int ret;
	int flags;
	struct sockaddr_in addr;
	socklen_t addrlen;

	/* Check whther that interface is already configured */
	if (iface->udp.connect_s != -1) {
		/* It does. If we want to bind to the same port, or a random
		 * port then it's ok. */
		if (port == 0 || port == iface->udp.sin.sin_port)
			return PTL_OK;

		ptl_warn("interface already exists\n");
		return PTL_FAIL;
	}

	iface->udp.sin.sin_port = port;

	/* Create the UDP listen socket. */
	iface->udp.connect_s = socket(AF_INET, SOCK_DGRAM, 0);
	if (iface->udp.connect_s == -1) {
		ptl_warn("unable to create UDP socket\n");
		goto err1;
	}

	/* Bind it to the selected port. */
	ret = bind(iface->udp.connect_s, (struct sockaddr *)&iface->udp.sin, sizeof(iface->udp.sin));
	if (ret == -1) {
		ptl_warn("unable to bind to local address %x\n",
				 iface->udp.sin.sin_addr.s_addr);
		goto err1;
	}

	/* In case we asked for any port get the actual source port */
	ret = getsockname(iface->udp.connect_s, (struct sockaddr *)&addr, &addrlen);
	if (ret == -1) {
		ptl_warn("unable to retrieve local port\n");
		goto err1;
	}

	/* remember the physical pid. */
	iface->id.phys.pid = port_to_pid(addr.sin_port);

	/* Set the socket in non blocking mode. */
	flags = fcntl(iface->udp.connect_s, F_GETFL);
	ret = fcntl(iface->udp.connect_s, F_SETFL, flags | O_NONBLOCK);
	if (ret == -1) {
		ptl_warn("cannot set asynchronous fd to non blocking\n");
		goto err1;
	}

	/* add a watcher for CM events */
	iface->udp.watcher.data = iface;
	ev_io_init(&iface->udp.watcher, process_udp_connect,
			   iface->udp.connect_s, EV_READ);

	EVL_WATCH(ev_io_start(evl.loop, &iface->udp.watcher));

	return PTL_OK;

 err1:
	if (iface->udp.connect_s != -1) {
		close(iface->udp.connect_s);
		iface->udp.connect_s = -1;
	}

	return PTL_FAIL;
}

int PtlNIInit_UDP(gbl_t *gbl, ni_t *ni)
{
	int err;
	int ret;
	int flags;
	struct sockaddr_in addr;
	uint16_t port;
	iface_t *iface = ni->iface;

	ni->udp.s = -1;
	ni->id.phys.nid = addr_to_nid(&iface->udp.sin);

	if (iface->id.phys.nid == PTL_NID_ANY) {
		iface->id.phys.nid = ni->id.phys.nid;
	} else if (iface->id.phys.nid != ni->id.phys.nid) {
		WARN();
		err = PTL_FAIL;
		goto error;
	}

	ptl_info("setting ni->id.phys.nid = %x\n", ni->id.phys.nid);

	err = iface_bind(iface, pid_to_port(ni->id.phys.pid));
	if (err) {
		ptl_warn("Binding failed\n");
		WARN();
		goto error;
	}

	if ((ni->options & PTL_NI_PHYSICAL) &&
		(ni->id.phys.pid == PTL_PID_ANY)) {
		/* No well known PID was given. Retrieve the pid given by
		 * bind. */
		ni->id.phys.pid = iface->id.phys.pid;

		ptl_info("set iface pid(1) = %x\n", iface->id.phys.pid);
	}

	/* Create a socket to be used for the transport. All connections
	 * will use it. */
	ni->udp.s = socket(AF_INET, SOCK_DGRAM, 0);
	if (ni->udp.s == -1) {
		ptl_warn("Failed to create socket\n");
		err = PTL_FAIL;
		goto error;
	}

	/* Set the socket in non blocking mode. */
	flags = fcntl(ni->udp.s, F_GETFL);
	ret = fcntl(ni->udp.s, F_SETFL, flags | O_NONBLOCK);
	if (ret == -1) {
		ptl_warn("cannot set asynchronous fd to non blocking\n");
		err = PTL_FAIL;
		goto error;
	}

	/* Bind it and retrieve the port assigned. */
	addr = iface->udp.sin;

	for(port = 49152; port <= 65535; port ++) {
		addr.sin_port = cpu_to_be16(port);
		ret = bind(ni->udp.s, (struct sockaddr *)&addr, sizeof(addr));
		if (ret == -1) {
			if (errno == EADDRINUSE)
				continue;

			ptl_warn("unable to bind to local address:port %x:%d (errno=%d)\n",
					 addr.sin_addr.s_addr, port, errno);
			break;
		}
		break;
	}

	if (ret == -1) {
		/* Bind failed or no port available. */
		err = PTL_FAIL;
		goto error;
	}

	ni->udp.src_port = port;

	// UO & REB: setup ni's udp?
	ni->udp.dest_addr = &iface->udp.sin;

	// TODO: Does this belong here or even in UDP at all?
	off_t bounce_buf_offset;
	off_t bounce_head_offset;

	bounce_head_offset = ni->udp.comm_pad_size;
	ni->udp.comm_pad_size += ROUND_UP(sizeof(struct udp_bounce_head), pagesize);

	ni->udp.udp_buf.buf_size = get_param(PTL_BOUNCE_BUF_SIZE);
	ni->udp.udp_buf.num_bufs = get_param(PTL_BOUNCE_NUM_BUFS);

	bounce_buf_offset = ni->udp.comm_pad_size;
	ni->udp.comm_pad_size += ni->udp.udp_buf.buf_size * ni->udp.udp_buf.num_bufs;

	return PTL_OK;

 error:
	if (ni->udp.s != -1) {
		close(ni->udp.s);
		ni->udp.s = -1;
	}
	return err;
}