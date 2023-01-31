#include "libavutil/avutil.h"
#include "libavformat/avformat.h"



int main(int argc, char ** argv)
{
	av_log_set_level(AV_LOG_INFO);
	if (argc<2)
	{
		av_log(NULL, AV_LOG_ERROR, "Usage: %s <inputfileName>\n", argv[0]);
		return -1;
	}

	const char * inFileName = argv[1];

	AVFormatContext *inFmtCtx = NULL;

	avformat_open_input(&inFmtCtx, inFileName, NULL, NULL);

	avformat_find_stream_info(inFmtCtx, NULL);

	av_dump_format(inFmtCtx, 0, inFileName, 0);

	//duration 字段的时间基是 AV_time_base_q

	av_log(NULL, AV_LOG_INFO, "input file duration:%ld ms,duration time %lf s.\n",inFmtCtx->duration, inFmtCtx->duration * av_q2d(AV_TIME_BASE_Q));


	AVRational videoTimeBase;
	AVRational audioTimeBase;

	for(int i=0; i< inFmtCtx->nb_streams; i++){
		AVStream *inStream = inFmtCtx->streams[i];
		if(inStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoTimeBase = inStream->time_base;
			av_log(NULL,AV_LOG_INFO,"video time base: num = %d, den = %d\n",videoTimeBase.num,videoTimeBase.den);
		}
		if(inStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioTimeBase = inStream->time_base;
			av_log(NULL,AV_LOG_INFO,"audio time base: num = %d, den = %d\n",audioTimeBase.num,audioTimeBase.den);
		}
	}
	
	return 0;
}
