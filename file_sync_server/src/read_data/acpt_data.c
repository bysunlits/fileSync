//
// Created by sunlit on 2026/9/29.
//

enum INFOMESSAGE
{
	START_MSG, IN_PROGRESS_MSG, END_MSG
};
typedef enum
{
	ACPT_SUCCESS,NET_ERR,DISK_ERR,UNKNOWN_ERR
}ACCEPT_DATA_RESULT;

ACCEPT_DATA_RESULT accept_data();

typedef enum
{
	HLD_SUCCESS
}HLD_HEAD_RET;

HLD_HEAD_RET hdl_head();

MerkleTree metainfo_deal();

void recv_chunk(MerkleTree hash_tree);

typedef enum
{
	FAIL_RECV,ERR_PKG
}RECV_RESULT;

RECV_RESULT rd_to_buffer();




ACCEPT_DATA_RESULT acpt_data()
{
	MerkleTree hash_tree=metainfo_deal();
	recv_chunk(hash_tree);
}

void recv_chunk(MerkleTree hash_tree)
{
	HLD_HEAD_RET ret=hdl_head();
	if(ret==HLD_SUCCESS)
	{
		RECV_RESULT result=rd_to_buffer();
		if (result==FAIL_RECV)
		{
			try2reconnect();
		}
		else if (result=ERR_PKG)
		{
			resend();
		}
	}
	else
	{
		goto cleanup;
	}
cleanup:
}


RECV_RESULT rd_to_buffer()
{
	;
	;
	;
	//The specific implementation of recv data from .sock here can be left blank for now.
	check_hash();

}



MerkleTree metainfo_deal()
{
	deal_notify();
	build_recv_info();
}


