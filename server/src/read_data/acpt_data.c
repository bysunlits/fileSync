//
// Created by sunlit on 2026/9/29.
//

typedef struct MerkleTree
{
	void (*left_node)(struct MerkleTree *self);
	void (*right_node)(struct MerkleTree *self);
}MerkleTree;


MerkleTree metainfo_deal()
{
	deal_notify();
	build_recv_info();
}


ACCEPT_DATA_RESULT accept_data()
{
	MerkleTree hash_tree=metainfo_deal();
	recv_chunk(hash_tree);
}

typedef enum
{
	FAIL_RECV,ERR_PKG
}RECV_RESULT;

RECV_RESULT rd_to_buffer()
{
	;
	;
	;
	//The specific implementation of recv data from .sock here can be left blank for now.
	check_hash();

}


void recv_chunk(MerkleTree hash_tree)
{
	HLD_HEAD_RET ret=hdl_head();
	if(ret==HLD_SUCCESS)
	{
		RECV_RESULT result=rd_to_buffer();
		if (result==FAIL_RECV)
		{
			resend();
		}
		else if (result=ERR_PKG)
			set_buffer_to_tmpfile();

	}
	else
	{
		proc_hld_err(ret);
	}
}