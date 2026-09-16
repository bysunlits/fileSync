//
// Created by bysunlits on 2026/9/6.
//
#include <stdio.h>

enum INFOMESSAGE
{
	START_MSG, IN_PROGRESS_MSG, END_MSG
};

int main()
{
	load_config();
	read_data();
	save_data_to_disk();
	check_data();
	read_meta_data_from_file();
	notify_target_client();


	{
		size_t buffer[256*1024];
		if (fread(buffer,1,sizeof(buffer),stdin)) {

			//Firstly,read the bytes to buffer
			//1.1,recognise which client sent it,every client should register itself by the api /regiser,then allocate a uuid to the client
			//1.2 GET the file by the api /push_file
			//1.3 get the config file set by user
			//1.4 According to the meta info of the file,set the file to disk
			//secondly,check the hash to verify the integrity of the snippet
			//2.1,if the file(snippet check should be done while TCP data flow) has broken,inform client to resend the file
			//2.2 if that snippet is integral,save the meta data of the file
			//Thirdly,
			//1.1inform other that they should pull the lastest version file
		}
	}

}
	//For clients,the time should be updated every 5 min via NTP
	//Mtime should be set as NTP-based time instead of local machine time.
	//File updating bases on mtime
	//client will pull data from server when: 1.periodic polling every 5 min 2.received server's notify
	//Client must receive a message from server to regard the file sync as success.