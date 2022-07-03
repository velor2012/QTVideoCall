#include "CamMicLoader.h"


CamMicLoader::CamMicLoader(int out_width, int out_height):
    out_width(out_width),
    out_height(out_height),
    pFrameYUV(NULL),
    img_convert_ctx(NULL),
    audio_convert_ctx(NULL),
    out_buff(NULL),
    m_pVidFmtCtx(NULL),
    m_pAudFmtCtx(NULL),
    dec_pkt(NULL),
    m_pInputFormat(NULL),
    pVideoCodecCtx(NULL),
    pAudioCodecCtx(NULL),
    m_pAudioCBFunc(NULL),
    m_pVideoCBFunc(NULL),
    m_hCapVideoThread(NULL),
    m_hCapAudioThread(NULL),
    m_audio_out_buffer(nullptr)
{

    //Initialize the buffer to store YUV frames.
    pFrameYUV = av_frame_alloc();
    pFrameYUV->width = out_width;
    pFrameYUV->height = out_height;
    pFrameYUV->format = AV_PIX_FMT_YUV420P;
    int frame_buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pFrameYUV->width, pFrameYUV->height, 1);
    m_out_buffer = (uint8_t*)av_malloc(frame_buf_size);
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, m_out_buffer, AV_PIX_FMT_YUV420P, pFrameYUV->width, pFrameYUV->height, 1);

    out_buff = (uint8_t*)malloc(frame_buf_size);

}

CamMicLoader::~CamMicLoader()
{
    //if (out_buff != NULL) {
    //	free(out_buff);
    //	out_buff = NULL;
    //}
}

void  CamMicLoader::SetVideoCaptureCB(VideoCaptureCB pFuncCB)
{
    m_pVideoCBFunc = pFuncCB;
}

void  CamMicLoader::SetAudioCaptureCB(AudioCaptureCB pFuncCB)
{
    m_pAudioCBFunc = pFuncCB;
}
// 用于将中文字符串，转化为UTF8
static char* dup_wchar_to_utf8(const wchar_t* w)
{
    char* s = NULL;
    int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
    s = (char*)av_malloc(l);
    if (s)
        WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, 0, 0);
    return s;
}

static std::string WideCharToUTF8(const wchar_t* pUnicode)
{
    std::string str_utf8("");
    BYTE* pUtfData = NULL;
    do
    {
        int utfNeed = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)pUnicode, -1, (char*)pUtfData, 0, NULL, NULL);
        pUtfData = new BYTE[utfNeed + 1];
        memset(pUtfData, 0, utfNeed + 1);
        int utfDone = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)pUnicode, -1, (char*)pUtfData, utfNeed, NULL, NULL);

        if (utfNeed != utfDone)
        {
            break;
        }
        str_utf8.assign((char*)pUtfData);
    } while (false);

    if (pUtfData)
    {
        delete[] pUtfData;
    }

    return str_utf8;
}


static std::string AnsiToUTF8(const char* _ansi, int _ansi_len)
{
    std::string str_utf8("");
    wchar_t* pUnicode = NULL;
    BYTE* pUtfData = NULL;
    do
    {
        int unicodeNeed = MultiByteToWideChar(CP_ACP, 0, _ansi, _ansi_len, NULL, 0);
        pUnicode = new wchar_t[unicodeNeed + 1];
        memset(pUnicode, 0, (unicodeNeed + 1) * sizeof(wchar_t));
        int unicodeDone = MultiByteToWideChar(CP_ACP, 0, _ansi, _ansi_len, (LPWSTR)pUnicode, unicodeNeed);

        if (unicodeDone != unicodeNeed)
        {
            break;
        }

        int utfNeed = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)pUnicode, unicodeDone, (char*)pUtfData, 0, NULL, NULL);
        pUtfData = new BYTE[utfNeed + 1];
        memset(pUtfData, 0, utfNeed + 1);
        int utfDone = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)pUnicode, unicodeDone, (char*)pUtfData, utfNeed, NULL, NULL);

        if (utfNeed != utfDone)
        {
            break;
        }
        str_utf8.assign((char*)pUtfData);
    } while (false);

    if (pUnicode)
    {
        delete[] pUnicode;
    }
    if (pUtfData)
    {
        delete[] pUtfData;
    }

    return str_utf8;
}

bool CamMicLoader::initLoader(std::wstring video_device_name, std::wstring audio_device_name) {

    avdevice_register_all();
    int i;
    m_audio_device = audio_device_name;
    m_video_device = video_device_name;
    if (m_video_device.empty() && m_audio_device.empty())
    {
        ATLTRACE("you have not set any capture device \n");
        return false;
    }

    //打开Directshow设备前需要调用FFmpeg的avdevice_register_all函数，否则下面返回失败;
    m_pInputFormat = av_find_input_format("dshow");
    if (m_pInputFormat == NULL) {
        ATLTRACE("error: can't use dshow!!");
        return false;
    }

    // Set device params
    AVDictionary* device_param = 0;
    //if not setting rtbufsize, error messages will be shown in cmd, but you can still watch or record the stream correctly in most time
    //setting rtbufsize will erase those error messages, however, larger rtbufsize will bring latency
    //av_dict_set(&device_param, "rtbufsize", "10M", 0);

    if (!m_video_device.empty())
    {
        int res = 0;

        std::wstring device_name = ::SysAllocString(L"video=");
        device_name = device_name.append(m_video_device);

        //string device_name_utf8 = AnsiToUTF8(device_name.c_str(), device_name.length());  //转成UTF-8，解决设备名称包含中文字符出现乱码的问题;
        std::string device_name_utf8 = WideCharToUTF8(device_name.c_str());
            //Set own video device's name
        if ((res = avformat_open_input(&m_pVidFmtCtx, device_name_utf8.c_str(), m_pInputFormat, &device_param)) != 0)
        {
            ATLTRACE("Couldn't open input video stream.（无法打开输入流）\n");
            return false;
        }
        //input video initialize
        if (avformat_find_stream_info(m_pVidFmtCtx, NULL) < 0)
        {
            ATLTRACE("Couldn't find video stream information.（无法获取流信息）\n");
            return false;
        }


        m_videoindex = -1;
        for (i = 0; i < m_pVidFmtCtx->nb_streams; i++)
        {
            if (m_pVidFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                m_videoindex = i;
                break;
            }
        }

        if (m_videoindex == -1)
        {
            ATLTRACE("Couldn't find a video stream.（没有找到视频流）\n");
            return false;
        }

        pVideoCodecCtx = avcodec_alloc_context3(NULL);
        if (pVideoCodecCtx == NULL)
        {
            printf("Could not allocate AVCodecContext\n");
            return -1;
        }
        avcodec_parameters_to_context(pVideoCodecCtx, m_pVidFmtCtx->streams[m_videoindex]->codecpar);

        if (avcodec_open2(pVideoCodecCtx, avcodec_find_decoder(m_pVidFmtCtx->streams[m_videoindex]->codecpar->codec_id), NULL) < 0)
        {
            ATLTRACE("Could not open video codec.（无法打开解码器）\n");
            return false;
        }
    }
    //////////////////////////////////////////////////////////

    if (!m_audio_device.empty())
    {
        std::wstring device_name = ::SysAllocString(L"audio=");
        device_name = device_name.append(m_audio_device);

        //string device_name_utf8 = AnsiToUTF8(device_name.c_str(), device_name.length());  //转成UTF-8，解决设备名称包含中文字符出现乱码的问题;
        char* device_name_utf8 = dup_wchar_to_utf8(device_name.c_str());

        //string device_name = "audio=" + m_audio_device;

        //string device_name_utf8 = AnsiToUTF8(device_name.c_str(), device_name.length());  //转成UTF-8，解决设备名称包含中文字符出现乱码的问题;
        av_dict_set(&device_param, "audio_buffer_size", "20", 0);
        //Set own audio device's name;
        if (avformat_open_input(&m_pAudFmtCtx, device_name_utf8, m_pInputFormat, &device_param) != 0) {

            ATLTRACE("Couldn't open input audio stream.（无法打开输入流）\n");
            return false;
        }

        //input audio initialize;
        if (avformat_find_stream_info(m_pAudFmtCtx, NULL) < 0)
        {
            ATLTRACE("Couldn't find audio stream information.（无法获取流信息）\n");
            return false;
        }
        m_audioindex = -1;
        for (i = 0; i < m_pAudFmtCtx->nb_streams; i++)
        {
            if (m_pAudFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                m_audioindex = i;
                break;
            }
        }
        if (m_audioindex == -1)
        {
            ATLTRACE("Couldn't find a audio stream.（没有找到音频流）\n");
            return false;
        }

        pAudioCodecCtx = avcodec_alloc_context3(NULL);
        if (pAudioCodecCtx == NULL)
        {
            printf("Could not allocate AVCodecContext\n");
            return false;
        }
        avcodec_parameters_to_context(pAudioCodecCtx, m_pAudFmtCtx->streams[m_audioindex]->codecpar);


        if (avcodec_open2(pAudioCodecCtx, avcodec_find_decoder(m_pAudFmtCtx->streams[m_audioindex]->codecpar->codec_id), NULL) < 0)
        {
            ATLTRACE("Could not open audio codec.（无法打开解码器）\n");
            return false;
        }
    }

    return true;
}

bool  CamMicLoader::StartCapture()
{

    if (m_videoindex == -1 && m_audioindex == -1)
    {
        ATLTRACE("错误：你没有打开设备 \n");
        return false;
    }

    m_start_time = av_gettime();

    m_exit_thread = false;

    if (!m_video_device.empty())
    {
        if (!GetVideoInputInfo(src_width, src_height, fps, pixel_fmt)) {
            ATLTRACE("错误：无法获取摄像头数据 \n");
            return false;
        }

        if (img_convert_ctx == NULL)
        {
            //camera data may has a pix fmt of RGB or sth else,convert it to YUV420
            img_convert_ctx = sws_getContext(src_width, src_height,
                pixel_fmt, out_width, out_height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
        }

        m_hCapVideoThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size
            CaptureVideoThreadFunc,       // thread function name
            this,          // argument to thread function
            0,                      // use default creation flags
            NULL);   // returns the thread identifier
    }

    if (!m_audio_device.empty())
    {
        if (!GetAudioInputInfo(sample_fmt, sample_rate, channel)) {
            ATLTRACE("错误：无法获取麦克风数据 \n");
            return false;
        }

        if (audio_convert_ctx == NULL)
        {
            int out_channel_layout = av_get_default_channel_layout(2); //codec2需要1通道的输入，因此重采样为1通道;
            int in_channel_layout = av_get_default_channel_layout(channel);
            int out_sample_rate = 44100; //codec2需要8000hz采样率的输入;
            //camera data may has a pix fmt of RGB or sth else,convert it to YUV420;
            audio_convert_ctx = swr_alloc_set_opts(audio_convert_ctx, out_channel_layout, AV_SAMPLE_FMT_S16, out_sample_rate,
                in_channel_layout, sample_fmt, sample_rate, 0, NULL); //配置重采样率;
            swr_init(audio_convert_ctx); // 初始化重采样率;

            audio_nsamples = 882;//20ms一帧，一秒50帧，每秒数据量除以50得到每帧大小;
            audio_out_buffer_size = audio_nsamples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * 2;
            m_audio_out_buffer = (uint8_t*)av_malloc(audio_out_buffer_size);

            count = 0;
        }

        m_hCapAudioThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size
            CaptureAudioThreadFunc,       // thread function name
            this,          // argument to thread function
            0,                      // use default creation flags
            NULL);   // returns the thread identifier
    }

    return true;
}

void  CamMicLoader::CloseInputStream()
{
    m_exit_thread = true;
    if (m_hCapVideoThread)
    {
        if (WAIT_TIMEOUT == WaitForSingleObject(m_hCapVideoThread, 1000))
        {
            printf("WaitForSingleObject timeout.\n");
            ::TerminateThread(m_hCapVideoThread, 0);
        }
        CloseHandle(m_hCapVideoThread);
        m_hCapVideoThread = NULL;
        m_pVideoCBFunc = NULL;
    }

    if (m_hCapAudioThread)
    {
        if (WAIT_TIMEOUT == WaitForSingleObject(m_hCapAudioThread, 1000))
        {
            printf("WaitForSingleObject timeout.\n");
            ::TerminateThread(m_hCapAudioThread, 0);
        }
        CloseHandle(m_hCapAudioThread);
        m_hCapAudioThread = NULL;
        m_pAudioCBFunc = NULL;
    }

    //关闭输入流;
    if (m_pVidFmtCtx != NULL)
    {
        avformat_close_input(&m_pVidFmtCtx);
        //m_pVidFmtCtx = NULL;
    }
    if (m_pAudFmtCtx != NULL)
    {
        avformat_close_input(&m_pAudFmtCtx);
        //m_pAudFmtCtx = NULL;
    }

    if (m_pVidFmtCtx)
        avformat_free_context(m_pVidFmtCtx);
    if (m_pAudFmtCtx)
        avformat_free_context(m_pAudFmtCtx);

    m_pVidFmtCtx = NULL;
    m_pAudFmtCtx = NULL;
    m_pInputFormat = NULL;

    m_videoindex = -1;
    m_audioindex = -1;
}

DWORD WINAPI CamMicLoader::CaptureVideoThreadFunc(LPVOID lParam)
{
    CamMicLoader* pThis = (CamMicLoader*)lParam;

    pThis->ReadVideoPackets();

    return 0;
}

//把AVFrame中的数据以yuv的形式读取出来;
static int copyFrame(AVFrame* frame, unsigned char* output_buff) {
    int luma_size = frame->width * frame->height;
    int chroma_size = luma_size / 4;
    for (int i = 0; i < frame->height; i++) {
        memcpy(output_buff + i * frame->width, frame->data[0] + i * frame->linesize[0], frame->width);
    }

    int loop = frame->height / 2;
    int len_uv = frame->width / 2;

    for (int i = 0; i < loop; i++) {
        memcpy(output_buff + luma_size + i * len_uv, frame->data[1] + i * frame->linesize[1], len_uv);

    }

    for (int i = 0; i < loop; i++) {
        memcpy(output_buff + luma_size + chroma_size + i * len_uv, frame->data[2] + i * frame->linesize[2], len_uv);

    }
    return 1;

}

int CamMicLoader::decodeVideo(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt)
{
    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return -1;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }
        if (m_pVideoCBFunc)
        {
            lock.lock();
            //重采样;
            sws_scale(img_convert_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, src_height, pFrameYUV->data, pFrameYUV->linesize);
            copyFrame(pFrameYUV, out_buff);

            int64_t lTimeStamp = av_gettime() - m_start_time;

            m_pVideoCBFunc(out_buff, lTimeStamp);
            lock.unlock();
        }
//        printf("saving frame %3d\n", dec_ctx->frame_number);
        fflush(stdout);
        return 1;
    }
}


int  CamMicLoader::ReadVideoPackets()
{


    int encode_video = 1;
    int ret;

    //start decode and encode

    while (encode_video)
    {
        if (m_exit_thread)
            break;
        if (dec_pkt == NULL)
        {
            ////prepare before decode and encode
            dec_pkt = (AVPacket*)av_malloc(sizeof(AVPacket));
        }
        AVFrame* pframe = NULL;
        if ((ret = av_read_frame(m_pVidFmtCtx, dec_pkt)) >= 0)
        {
            pframe = av_frame_alloc();
            if (!pframe)
            {
                ret = AVERROR(ENOMEM);
                return ret;
            }
            int dec_got_frame = 0;
            ret = decodeVideo(pVideoCodecCtx, pframe, dec_pkt);
            av_frame_free(&pframe);
        }
        else
        {
            if (ret == AVERROR_EOF)
                encode_video = 0;
            else
            {
                ATLTRACE("Could not read video frame\n");
                break;
            }
        }
        av_packet_free(&dec_pkt);
    }

    return 0;
}

DWORD WINAPI CamMicLoader::CaptureAudioThreadFunc(LPVOID lParam)
{
    CamMicLoader* pThis = (CamMicLoader*)lParam;

    pThis->ReadAudioPackets();

    return 0;
}


int CamMicLoader::decodeAudio(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt)
{
    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return -1;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        //copyFrame(pFrameYUV, out_buff);
        if (m_pAudioCBFunc)
        {
            lock.lock();

            int64_t lTimeStamp = av_gettime() - m_start_time;

            //重采样;
            swr_convert(audio_convert_ctx, &m_audio_out_buffer, audio_nsamples, // 对音频的采样率进行转换;
                (const uint8_t**)frame->extended_data, frame->nb_samples);
            //m_pAudioCBFunc(*frame->extended_data, frame_size, lTimeStamp);
            m_pAudioCBFunc(m_audio_out_buffer, audio_out_buffer_size, lTimeStamp);
            count++;
            lock.unlock();
        }
        return 1;
    }
}

int CamMicLoader::ReadAudioPackets()
{
    //audio trancoding here
    int ret;

    int encode_audio = 1;
    int dec_got_frame_a = 0;

    //start decode and encode
    while (encode_audio)
    {
        if (m_exit_thread)
            break;

        /**
        * Decode one frame worth of audio samples, convert it to the
        * output sample format and put it into the FIFO buffer.
        */
        AVFrame* input_frame = av_frame_alloc();
        if (!input_frame)
        {
            ret = AVERROR(ENOMEM);
            return ret;
        }

        /** Decode one frame worth of audio samples. */
        /** Packet used for temporary storage. */
        AVPacket input_packet;
        av_init_packet(&input_packet);
        input_packet.data = NULL;
        input_packet.size = 0;

        /** Read one audio frame from the input file into a temporary packet. */
        if ((ret = av_read_frame(m_pAudFmtCtx, &input_packet)) < 0)
        {
            /** If we are at the end of the file, flush the decoder below. */
            if (ret == AVERROR_EOF)
            {
                encode_audio = 0;
            }
            else
            {
                ATLTRACE("Could not read audio frame\n");
                return ret;
            }
        }

        /**
        * Decode the audio frame stored in the temporary packet.
        * The input audio stream decoder is used to do this.
        * If we are at the end of the file, pass an empty packet to the decoder
        * to flush it.
        */
        ret = decodeAudio(pAudioCodecCtx, input_frame, &input_packet);
        //if ((ret = avcodec_decode_audio4(pAudioCodecCtx, input_frame, &dec_got_frame_a, &input_packet)) < 0)
        if(ret < 0)
        {
            ATLTRACE("Could not decode audio frame\n");
            return ret;
        }
        av_packet_unref(&input_packet);

        av_frame_free(&input_frame);


    }//while

    return 0;
}


bool CamMicLoader::GetVideoInputInfo(int& width, int& height, int& frame_rate, AVPixelFormat& pixFmt)
{
    if (m_videoindex != -1)
    {
        width = pVideoCodecCtx->width;
        height = pVideoCodecCtx->height;

        AVStream* stream = m_pVidFmtCtx->streams[m_videoindex];

        pixFmt = pVideoCodecCtx->pix_fmt;

        //frame_rate = stream->avg_frame_rate.num/stream->avg_frame_rate.den;//每秒多少帧;

        if (stream->r_frame_rate.den > 0)
        {
            frame_rate = stream->r_frame_rate.num / stream->r_frame_rate.den;
        }
        else if (pVideoCodecCtx->framerate.den > 0)
        {
            frame_rate = pVideoCodecCtx->framerate.num / pVideoCodecCtx->framerate.den;
        }

        return true;
    }
    return false;
}

bool  CamMicLoader::GetAudioInputInfo(AVSampleFormat& sample_fmt, int& sample_rate, int& channels)
{
    if (m_audioindex != -1)
    {
        sample_fmt = pAudioCodecCtx->sample_fmt;
        sample_rate = pAudioCodecCtx->sample_rate;
        channels = pAudioCodecCtx->channels;
        return true;
    }
    return false;
}
