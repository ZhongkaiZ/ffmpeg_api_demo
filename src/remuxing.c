#include <libavutil/avutil.h>
#include <libavformat/avformat.h>


int main(int argc, char **argv)
{
	av_log_set_level(AV_LOG_DEBUG);
	if(argc < 3)
	{
		av_log(NULL, AV_LOG_ERROR, "Usage: %s <inFileName> <outFileName>.\n", argv[0]);
		return -1;
	}
	const char* inFileName = argv[1];
	const char* outFileName = argv[2];

	AVFormatContext * inFmtCtx = NULL;

	int ret = 0;
	//打开输入上下文
	ret = avformat_open_input(&inFmtCtx, inFileName, NULL, NULL);
	if(ret != 0)
	{
		av_log(NULL, AV_LOG_ERROR, "open input format failed:%s\n", av_err2str(ret));
		return -1;
	}

	ret = avformat_find_stream_info(inFmtCtx, NULL);
	
	if(ret != 0)
	{
		av_log(NULL, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
		ret = -1;
		goto fail;
	}
	//打印metaData
	av_dump_format(inFmtCtx, 0 ,inFileName, 0);

	AVFormatContext *outFmtCtx = NULL;
	//创建输出文件的format上下文
	ret = avformat_alloc_output_context2(&outFmtCtx, NULL, NULL, outFileName);
	if(ret != 0)
	{
		av_log(NULL, AV_LOG_ERROR, "alloc out format failed:%s\n", av_err2str(ret));
		ret = -1;
		goto fail;
	}

	//流总数
	int streamCount = inFmtCtx->nb_streams;

	int* handleStreamArray = av_mallocz_array(streamCount, sizeof(int));
	if(handleStreamArray == NULL)
	{
		av_log(NULL, AV_LOG_ERROR, "malloc stream array failed.\n");
		ret = -1;
		goto fail;
	}

	int streamIndex = 0;
	//创建输出流并拷贝编码器参数
	for(int i =0; i< inFmtCtx->nb_streams; i++){
		AVStream *inStream = inFmtCtx->streams[i];
		//仅处理视频音频字幕
		if(	inStream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
			inStream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
			inStream->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE)
		{
			handleStreamArray[i] = -1;
			continue;
		}
		// 输出文件中流重新计数
		handleStreamArray[i] = streamIndex++;
		
		//此任务仅转封装，所以不需要指定codec, 指定了上下文。
		AVStream * outStream = avformat_new_stream(outFmtCtx, NULL);
		if(outStream == NULL)
		{
			av_log(NULL, AV_LOG_ERROR, "new out stream failed\n");
			ret = -1;
			goto fail;
		}

		ret = avcodec_parameters_copy(outStream->codecpar, inFmtCtx->streams[i]->codecpar);
		if(ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "copy in stream codecpar failed: %s\n", av_err2str(ret));
			goto fail;
		}

		//不知道为啥
		outStream->codecpar->codec_tag = 0;
	}

	//如果这个标志位被制成了AVFMT_NOFILE,则不应该去设成write
	if(!(outFmtCtx->oformat->flags & AVFMT_NOFILE))
	{
		//打开输出的文件io。 不同于以前的fifo文件写入
		ret = avio_open(&outFmtCtx->pb, outFileName, AVIO_FLAG_WRITE);
		if(ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "open %s file failed.\n", outFileName);
			goto fail;
		}
	}
	//三部曲， writer_header, interleaved_write_frame,      write_trailer
	ret = avformat_write_header(outFmtCtx, NULL);
	if(ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "writer header failed: %s\n", av_err2str(ret));
		goto fail;
	}

	//循环将输入文件的包写入输出文件
	AVPacket packet; 
	av_init_packet(&packet);

	while(1)
	{
		
		ret = av_read_frame(inFmtCtx, &packet);
		if(ret < 0)
		{
			break;
		}
		//跳过不需要的包
		if(packet.stream_index >= streamCount ||
			handleStreamArray[packet.stream_index] < 0)
		{
			av_packet_unref(&packet);
			continue;
		}
		AVStream *inStream = inFmtCtx->streams[packet.stream_index];
		packet.stream_index = handleStreamArray[packet.stream_index];
		AVStream *outStream = outFmtCtx->streams[packet.stream_index];

		//转time_base, 不同封装格式的流有不同的time_base.
		packet.pts = av_rescale_q(packet.pts, inStream->time_base, outStream->time_base);
		packet.dts = av_rescale_q(packet.dts, inStream->time_base, outStream->time_base);
		packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
		packet.pos = -1;

		//写入输入文件
		ret = av_interleaved_write_frame(outFmtCtx, &packet);
		if(ret != 0)
		{
			av_log(NULL, AV_LOG_ERROR, "write frame failed: %s\n", av_err2str(ret));
			break;
		}

		av_packet_unref(&packet);
	}
	ret = av_write_trailer(outFmtCtx);
	if(ret != 0)
	{
		av_log(NULL, AV_LOG_ERROR, "write trailer failed: %s\n", av_err2str(ret));
	}
	
fail:
	if(inFmtCtx)
	{
		avformat_close_input(&inFmtCtx);
		inFmtCtx = NULL;
	}
	
	if(handleStreamArray)
	{
		av_freep(&handleStreamArray);
		handleStreamArray = NULL;
	}

	if(outFmtCtx && !(outFmtCtx->oformat->flags & AVFMT_NOFILE))
	{
		avio_closep(&outFmtCtx->pb);
	}
	
	if(outFmtCtx)
	{
		avformat_free_context(outFmtCtx);
		outFmtCtx = NULL;
	}	
	return ret;	
}

