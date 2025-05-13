/**
 * @file
 *
 * @author      Alexander Epstine
 * @mail        a@epstine.com
 * @brief
 *
 **************************************************************************************
 * Copyright (c) 2025, Alexander Epstine (a@epstine.com)
 **************************************************************************************
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


#include "RTSPVideoStreamer.h"
#include <thread>
#include <iostream>
#include "BaseQueue.h"
#include "std_utils.h"

#include <UsageEnvironment.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>

#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/types.hpp>

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
}

using namespace cs;
using namespace std;
using namespace cv;

class FFmpegH264Source : public FramedSource {
public:
	static FFmpegH264Source* createNew(UsageEnvironment& env)
	{
		return new FFmpegH264Source(env);
	}

	FFmpegH264Source(UsageEnvironment& env) : FramedSource(env)
	{
		//create_encoder();
	}

	~FFmpegH264Source() {}

	bool create_encoder(int width = 1280, int height = 720, int fps = 30)
	{
		codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		if (!codec) {
			std::cerr << "Codec with specified id not found" << std::endl;
			return false;
		}

		context = avcodec_alloc_context3(codec);
		if (!context) {
			std::cerr << "Can't allocate video codec context" << std::endl;
			return false;
		}

		/// Resolution must be a multiple of two
		context->height = height;
		context->width = width;

		/// Frames per second
		context->time_base.num = 1;
		context->time_base.den = fps;
		context->framerate.num = fps;
		context->framerate.den = 1;

		context->pix_fmt = AV_PIX_FMT_YUV420P;
		context->gop_size = fps * 2;
		context->max_b_frames = 3;
		context->refs = 3;
		context->bit_rate = 400000;

		av_opt_set(context->priv_data, "preset", "ultrafast", 0); //slow
		av_opt_set(context->priv_data, "crf", "35", 0);
		av_opt_set(context->priv_data, "tune", "zerolatency", 0);

		const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(AV_PIX_FMT_RGB24);
		if (!desc) {
			std::cerr << "Can't get descriptor for pixel format" << std::endl;
			return false;
		}

		bytesPerPixel = av_get_bits_per_pixel(desc) / 8;
		if (!(bytesPerPixel == 3 && !(av_get_bits_per_pixel(desc) % 8))) {
			std::cerr << "Unhandled bits per pixel, bad in pix fmt" << std::endl;
			return false;
		}

		if (avcodec_open2(context, codec, nullptr) < 0) {
			std::cerr << "Could not open codec" << std::endl;
			return false;
		}

		std::cout << "Codec opened!!!" << std::endl;

		swsContext = sws_getContext(
			width, height,
			AV_PIX_FMT_BGR24,
			width, height,
			AV_PIX_FMT_YUV420P,
			0, nullptr, nullptr, nullptr);

		if (!swsContext) {
			std::cerr << "Could not allocate sws context" << std::endl;
			return false;
		}

		av_init_packet(&packet);

		frame = av_frame_alloc();
		if (frame == nullptr) {
			std::cerr << "Could not allocate video frame" << std::endl;
			return false;
		}

		frame->format = context->pix_fmt;
		frame->height = context->height;
		frame->width = context->width;

		if (av_frame_get_buffer(frame, 0) < 0) {
			std::cerr << "Can't allocate the video frame data" << std::endl;
			return false;
		}

		return true;
	}
private:
	static void deliverFrameStub(void* clientData) { ((FFmpegH264Source*)clientData)->deliverFrame(); };
	
	virtual void doGetNextFrame()
	{
		deliverFrame();
	}

	AVFrame* frame = nullptr;
	AVCodecContext* context = nullptr;
	AVCodec* codec = nullptr;
	SwsContext* swsContext = nullptr;
	AVPacket packet;

	int bytesPerPixel = 0;
	int64_t frameIdx = 1;
	bool isKeyFrame = false;


	void encode()
	{
		if (avcodec_send_frame(context, frame) == 0) {
			avcodec_receive_packet(context, &packet);
		}
		else
			fprintf(stderr, "Error sending a frame for encoding\n");
	}

	void deliverFrame()
	{
		if (!isCurrentlyAwaitingData()) return;

		fflush(stdout);

		if (av_frame_make_writable(frame) < 0) {
			std::cerr << "Frame data not writable" << std::endl;
			return;
		}
		
		int in_line_size = bytesPerPixel * context->width;
		sws_scale(swsContext, &fr.data, &in_line_size, 0, context->height, frame->data, frame->linesize);

		/// Set frame index in range: [1, fps]
		frame->pts = frameIdx;
		frameIdx++;
		//if (frameIdx > context->framerate.num)
		//	frameIdx = 1;

		/// Set frame type
		if (isKeyFrame) {
			frame->key_frame = 1;
			frame->pict_type = AVPictureType::AV_PICTURE_TYPE_I;
		}

		encode();

		unsigned newFrameSize = packet.size;
		//cout << "New frame size: " << newFrameSize << endl;
		if (newFrameSize > fMaxSize) 
			fFrameSize = fMaxSize;
		else 
			fFrameSize = newFrameSize;

		gettimeofday(&fPresentationTime, NULL);
		memcpy(fTo, packet.data, fFrameSize);
		fNumTruncatedBytes = newFrameSize - fFrameSize;

		av_packet_unref(&packet);

		FramedSource::afterGetting(this);
	}

	virtual void doStopGettingFrames()
	{
		FramedSource::doStopGettingFrames();
	}

	void onFrame()
	{
		envir().taskScheduler().triggerEvent(m_eventTriggerId, this);
	}
private:
	EventTriggerId m_eventTriggerId;
public:
	BaseQueue<cv::Mat> frames;
	cv::Mat fr;
};

class LiveServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
	static LiveServerMediaSubsession* createNew(UsageEnvironment& env, StreamReplicator* replicator)
	{
		return new LiveServerMediaSubsession(env, replicator);
	}

protected:
	LiveServerMediaSubsession(UsageEnvironment& env, StreamReplicator* replicator)
		: OnDemandServerMediaSubsession(env, False), m_replicator(replicator) {
	};

	virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
	{
		FramedSource* source = m_replicator->createStreamReplica();
		return H264VideoStreamDiscreteFramer::createNew(envir(), source);
	}

	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
	{
		return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	}

	StreamReplicator* m_replicator;
};

void rtsp_loop_thread_func(UsageEnvironment* env, EventLoopWatchVariable* quit)
{
	env->taskScheduler().doEventLoop();
}

RTSPVideoStreamer::RTSPVideoStreamer()
{
}

RTSPVideoStreamer::~RTSPVideoStreamer()
{
	//Medium::close(rtspServer);
	//Medium::close(inputDevice);
}

void RTSPVideoStreamer::add_user_credentials(const char* user, const char* password)
{
	if (authDB == nullptr) {
		authDB = new UserAuthenticationDatabase();
	}
	cout << "Adding user: " << user << " with password: " << password << endl;
	authDB->addUserRecord(user, password);
}

void RTSPVideoStreamer::init(int port, const char* channel_name, int width, int height, int fps)
{
	scheduler = BasicTaskScheduler::createNew();
	if (scheduler == nullptr) {
		throw std::runtime_error("BasicTaskScheduler::createNew() failed");
	}

	env = BasicUsageEnvironment::createNew(*scheduler);

	OutPacketBuffer::maxSize = 10000000;
	rtspServer = RTSPServer::createNew(*env, port, authDB);

	source = FFmpegH264Source::createNew(*env);
	source->create_encoder(width, height, fps);
	StreamReplicator* inputDevice = StreamReplicator::createNew(*env, source, false);

	std::string channel = channel_name;
	channel = trim(channel);
	if (channel.at(0) == '/') {
		channel = channel.substr(1);
	}
	ServerMediaSession* sms = ServerMediaSession::createNew(*env, channel.c_str());
	sms->addSubsession(LiveServerMediaSubsession::createNew(*env, inputDevice));
	rtspServer->addServerMediaSession(sms);

	char* url = rtspServer->rtspURL(sms);
	*env << "Play this stream using the URL \"" << url << "\"\n";
	delete url;
}

int RTSPVideoStreamer::open(int port, int tunneling_port)
{
	EventLoopWatchVariable quit;

	std::thread rtsp_loop_thread(rtsp_loop_thread_func, env, &quit);
	rtsp_loop_thread.detach();

	return 1;
}

void RTSPVideoStreamer::show_frame(cv::Mat& frame, const char* channel)
{
	if (source == nullptr) 
		return;

	frame.copyTo(source->fr);
	//cv::Mat* new_frame = new cv::Mat(frame);
	//if (new_frame != nullptr) {
	//	source->frames.push(new_frame);
	//}
}
