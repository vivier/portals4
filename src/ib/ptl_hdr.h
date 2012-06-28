/**
 * @file ptl_hdr.h
 *
 * @brief This file contains the interface for message headers.
 */

#ifndef PTL_HDR_H
#define PTL_HDR_H

/*
 * PTL_XPORT_HDR
 *	mainly carries information needed to route
 *	packet to remote site and validate it
 *	packed some unrelated info into the first word
 */

#define PTL_HDR_VER_1		(1)

/* note: please keep all init->target before all tgt->init */
enum hdr_op {
	/* from init to target */
	OP_PUT=1,
	OP_GET,
	OP_ATOMIC,
	OP_FETCH,
	OP_SWAP,					/* must be last of requests */

	/* Either way. */
	OP_RDMA_DISC,

	/* from target to init. Do not change the order. */
	OP_REPLY,
	OP_ACK,
	OP_CT_ACK,
	OP_OC_ACK,
	OP_NO_ACK,	 /* when remote ME has ACK_DISABLE */

	OP_LAST,
};

enum hdr_fmt {
	PKT_FMT_REQ,
	PKT_FMT_REPLY,
	PKT_FMT_ACK,
	PKT_FMT_LAST,
};

/**
 * @brief Common header for portals request/response messages.
 */
struct hdr_common1 {
	unsigned int	version:4;
	unsigned int	operation:4;
	unsigned int	ni_fail:4;	/* response only */
	unsigned int	data_in:1;
	unsigned int	data_out:1;
	unsigned int    matching_list:2; 	/* response only */	
	unsigned int    pad:8;			
	unsigned int	ni_type:4;	/* request only */	
	unsigned int	pkt_fmt:4;	/* request only */	
	__le32			handle; 
#ifdef IS_PPE
	/* Padded to 64 bits. Might not be necessary. todo: try removing. */
	__le32 hash; __le32 pad_unused;
#endif
};

struct hdr_common2 {
	unsigned int	ack_req:4;		
	unsigned int	atom_type:4;	
	unsigned int	atom_op:5;		
	unsigned int	reserved_19:19;	
	union {							
	__le32			dst_nid;		
	__le32			dst_rank;		
	};								
	__le32			dst_pid;		
	union {							
	__le32			src_nid;		
	__le32			src_rank;		
	};								
	__le32			src_pid;		
};

struct hdr_region {
	__le64			length;			
	__le64			offset;			
};

/**
 * @brief Header for Portals request messages.
 */
typedef struct ptl_hdr {
	struct hdr_common1 h1;
	struct hdr_common2 h2;
	struct hdr_region h3;
} hdr_t;

typedef struct req_hdr {
	struct hdr_common1 h1;
	struct hdr_common2 h2;
	__le64			match_bits;		
	__le64			hdr_data;		
	__le64			operand;		
	__le32			pt_index;		
	__le32			uid;
	struct hdr_region h3;
} req_hdr_t;

/* Header for an ack or a reply. */
typedef struct ack_hdr {
	struct hdr_common1 h1;
	struct hdr_region h3;
} ack_hdr_t;

#endif /* PTL_HDR_H */
