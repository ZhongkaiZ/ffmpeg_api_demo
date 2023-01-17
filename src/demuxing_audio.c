#include "libavutil/log.h"
#include "libavformat/avformat.h"

int main(int argc, char**argv)
{
    //gcc demuxing_audio.c -I../include/ -L../lib -lavutil -lavformat -lavcodec
	av_log_set_level(AV_LOG_DEBUG);
	if(argc< 3){
		av_log(NULL,AV_LOG_ERROR, "Usage: %s infile outfile\n.", argv[0]);
		return -1;
	}
	const char * infileName = argv[1];
	const char * outfileName = argv[2];

	AVFormatContext * inputFoarmatCtx = NULL;
	int ret = avformat_open_input(&inputFoarmatCtx, infileName, NULL,NULL);
	if (ret!=0){
		av_log(NULL,AV_LOG_ERROR,"open input file format failed:%s \n",av_err2str(ret));
		return -1;
	}
	ret = avformat_find_stream_info(inputFoarmatCtx, NULL);
	if(ret<0){
		av_log(NULL,AV_LOG_ERROR,"find stream info failed: %s\n", av_err2str(ret));
		avformat_close_input(&inputFoarmatCtx);
		return -1;
	}
	
	//get best audio stream index
	int audioIndex = av_find_best_stream(inputFoarmatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (audioIndex<0){
		av_log(NULL,AV_LOG_ERROR,"find best stream failed, index is %d\n",audioIndex);
		avformat_close_input(&inputFoarmatCtx);
		return -1;
	}

	av_log(NULL,AV_LOG_INFO,"find best stream index is %d\n",audioIndex);

	// not ponit index
	AVPacket packet;
	av_init_packet(&packet);

	// out file index
	FILE *dest_fp = fopen(outfileName, "wb");
	
	if (dest_fp == NULL){
		av_log(NULL,AV_LOG_ERROR,"open %s file failed\n", outfileName);
		avformat_close_input(&inputFoarmatCtx);
		return -1;
	}
	while(av_read_frame(inputFoarmatCtx, &packet)==0){
		if(packet.stream_index == audioIndex){
			ret = fwrite(packet.data, 1, packet.size, dest_fp);
			if (ret != packet.size){
				av_log(NULL,AV_LOG_ERROR,"write file failed!\n");
				fclose(dest_fp);
				avformat_close_input(&inputFoarmatCtx);
				return -1;
			}
		}
		// free packet
		av_packet_unref(&packet);
	}

	if (inputFoarmatCtx){
		avformat_close_input(&inputFoarmatCtx);
	}

	if(dest_fp){
		fclose(dest_fp);
	}

	

	
	return 0;
}
