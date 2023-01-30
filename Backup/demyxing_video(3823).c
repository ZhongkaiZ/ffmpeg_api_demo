#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavutil/log.h"



int main(int argc,char ** argv){

	av_log_set_level(AV_LOG_DEBUG);
	if (argc<3)
	{
		av_log(NULL, AV_LOG_ERROR,  "Usage: %s <infileName> <outfileName>\n", argv[0]);
		return -1;
	}
	
	const char *inputName = argv[1];
	const char *outputName = argv[2];

	AVFormatContext inputForamtctx= NULL;
	ret = avformat_open_input(&inputForamtctx, inputName, NULL, NULL);
	if (ret!= 0){
		av_log(NULL, AV_LOG_ERROR, "open input format failed: %s\n", av_err2str(ret));
		avformat_close_input(&inputFoarmatCtx);
		return -1;
	}

	ret = avformat_find_stream_info(inputForamtctx, NULL);
	if(ret != 0)
	{
		av_log(NULL, AV_LOG_ERROR, "find input stream info failed: %s\n", av_err2str(ret));
		ret = -1;
		goto fail;
	}

	ret = av_find_best_stream(inputForamtctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if(ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "find best stream index failed.\n");
		ret = -1;
		goto fail;
	}
	int video_idx = ret;

	FILE * dest_fp= fopen(outputName,"wb+");
	if (dest_fp){
		av_log(NULL, AV_LOG_ERROR, "open %s file failed.\n", outFileName);
		ret = -1;
		goto fail;
	}

	AVPacket packet;
	av_init_packet(&packet);
	const AVBitStreamFilter *bsf = av_bsf_get_by_name("h264_mp4toannexb");
	if(bsf == NULL)
	{
		av_log(NULL, AV_LOG_ERROR, "get h264_mp4toannexb bsf failed\n");
		ret = -1;
		goto fail;
	}

	AVBSFContext * bsfCtx =NULL;
	av_bsf_alloc(bsf,&bsfCtx);
	avcodec_parameters_copy(bsfCtx->par_in, inFmtCtx->streams[videoIndex]->codecpar);
	av_bsf_init(bsfCtx);

	while(av_read_frame(inputForamtctx, &packet)==0){
		if(packet.stream_index==video_idx){
			if(av_bsf_send_packet(bsfCtx, &packet)==0){
				while(av_bsf_receive_packet(bsfCtx, &packet)==0){
					int wirte_size = fwrite(packet.data, 1, packet.size, dest_fp);
					if(writeSize != packet.size)
					{
						av_packet_unref(&packet);
						ret = -1;
						break;
					}
				}
			}
		}
		av_packet_unref(&packet);
	}
fail:
	if(inFmtCtx)
	{
		avformat_close_input(&inFmtCtx);
	}
	if(bsfCtx)
	{
		av_bsf_free(&bsfCtx);
	}
	if(dest_fp)
	{
		fclose(dest_fp);
	}

	
	return 0;
}
