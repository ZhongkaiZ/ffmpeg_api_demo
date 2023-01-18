#include "libavutil/log.h"
#include "libavformat/avformat.h"

const int sampleFrequencyTable[] = {
	96000,
	88200,
	64000,
	48000,
	44100,
	32000,
	24000,
	22050,
	16000,
	12000,
	11025,
	8000,
	7350
};

int getADTSHeader(char *adtsHeader, int packetSize, int profile, int sampleRate, int channels)
{
	int sampleFrequenceIndex = 3;	
	int adtsLength = packetSize + 7;
	
	for(int i = 0; i < sizeof(sampleFrequencyTable) / sizeof(sampleFrequencyTable[0]); i++)
	{
		if(sampleRate == sampleFrequencyTable[i])
		{
			sampleFrequenceIndex = i;
			break;
		}
	}
	adtsHeader[0] = 0xff;		//syncword:0xfff                          8bits

	adtsHeader[1] = 0xf0;		//syncword:0xfff                          4bits
	adtsHeader[1] |= (0 << 3);	//MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
	adtsHeader[1] |= (0 << 1);	//Layer:0                                 2bits
	adtsHeader[1] |= 1;			//protection absent:1                     1bit

	adtsHeader[2] = (profile << 6);							//profile:profile               2bits
	adtsHeader[2] |= (sampleFrequenceIndex & 0x0f) << 2;	//sampling frequency index:sampling_frequency_index  4bits
	adtsHeader[2] |= (0 << 1);								//private bit: 0                   1bit
	adtsHeader[2] |= (channels & 0x04) >> 2;				//channel configuration:channels  1bit

	adtsHeader[3] = (channels & 0x03) << 6;					//channel configuration:channels 2bits
	adtsHeader[3] |= (0 << 5);								//original:0				1bit
	adtsHeader[3] |= (0 << 4);								//home:0                	1bit
	adtsHeader[3] |= (0 << 3);								//copyright id bit:0       	1bit
	adtsHeader[3] |= (0 << 2);								//copyright id start:0		1bit
	adtsHeader[3] |= ((adtsLength & 0x1800) >> 11);			//frame length:value   		2bits

	adtsHeader[4] = (uint8_t)((adtsLength & 0x7f8) >> 3);	//frame length:value    8bits
	adtsHeader[5] = (uint8_t)((adtsLength & 0x7) << 5);		//frame length:value    3bits
	adtsHeader[5] |= 0x1f;									//buffer fullness:0x7ff 5bits
	adtsHeader[6] = 0xfc;									//buffer fullness:0x7ff 6bits
	return 0;
}


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
			char adtsHeader[7] = {0};
			getADTSHeader(adtsHeader, packet.size, inFormatCtx->streams[audioIndex]->codecpar->profile,   //写入adts头
				inFormatCtx->streams[audioIndex]->codecpar->sample_rate, 
				inFormatCtx->streams[audioIndex]->codecpar->channels);
			ret = fwrite(adtsHeader, 1, sizeof(adtsHeader), dest_fp);
			if(ret != sizeof(adtsHeader))
			{
				//
			}
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
