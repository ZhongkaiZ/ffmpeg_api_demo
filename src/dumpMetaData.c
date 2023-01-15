#include "libavutil/log.h"
#include "libavformat/avformat.h"

int main(int argc, char**argv)
{
    //gcc dumpMetaData.c -I../include/ -L../lib -lavutil -lavformat
	av_log_set_level(AV_LOG_DEBUG);
	if(argc< 2){
		av_log(NULL,AV_LOG_ERROR, "Usage: %s inputfileName.", argv[0]);
		return -1;
	}
	const char *inputfileName = argv[1];
	
	AVFormatContext *pFormatCtx = NULL;
	int ret = avformat_open_input(&pFormatCtx, inputfileName, NULL, NULL);
	if(ret != 0){
		av_log(NULL,AV_LOG_ERROR,"open input file:%s failed %s\n",inputfileName, av_err2str(ret));
	}

	av_dump_format(pFormatCtx, 0, inputfileName, 0); // input 0 ,output 1

	avformat_close_input(&pFormatCtx);
	return 0;
}

