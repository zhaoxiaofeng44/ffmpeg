#include "CCMovieNode.h"
#include "CCDirector.h"
#include "CCTexture2D.h"
#include "structtypes.h"

extern "C" {

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/mem.h"
#include "libavutil/fifo.h"
#include "libswscale/swscale.h"

};

#undef   PixelFormat

#ifdef WIN32
#define av_sprintf sprintf_s
#else
#define av_sprintf snprintf
#endif // WIN32

typedef struct ffmpeg_stream_context
{
	int stream_index;
	AVCodecContext *stream_context;

}ffmpeg_stream_context;

static void func_ffmpeg_alloc_stream(ffmpeg_stream_context ** stream)
{
	(*stream) = new ffmpeg_stream_context();
	(*stream)->stream_context = NULL;
}

static void func_ffmpeg_free_stream(ffmpeg_stream_context ** stream)
{
	if(*stream)
	{
		avcodec_close((*stream)->stream_context);
		delete *stream; *stream = NULL;
	}
}

typedef struct ffmpeg_video_context
{
    AVFrame *frame;

	int64_t last_pts;
	int64_t revise_time;
	
	int64_t draw_pts;

	int width;
	int height;

	uint8_t *data;
	int data_size;
	SwsContext *_sws_context;
	
	uint8_t *_video_list[3];
	int _video_list_stride[3];

}ffmpeg_video_context;

static void func_ffmpeg_alloc_video(ffmpeg_video_context ** video,ffmpeg_stream_context *stream)
{
	(* video) = new ffmpeg_video_context();
	ffmpeg_video_context *context = (* video);

	context->frame = av_frame_alloc();

	context->last_pts = 0;
	context->revise_time = 0;

	context->draw_pts = 0;

	context->width = stream->stream_context->width;
	context->height = stream->stream_context->height;
	
	context->data_size = avpicture_get_size(PIX_FMT_RGBA,context->width,context->height);
	context->data = (uint8_t *) av_malloc(context->data_size);
	memset(context->data, 0, context->data_size);
	
	context->_sws_context = sws_getContext(context->width,context->height, stream->stream_context->pix_fmt, context->width,context->height, PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
	
	context->_video_list[0] = context->data;
	context->_video_list[1] = NULL;
	context->_video_list[2] = NULL;
	context->_video_list_stride[0] = 4 * context->width;
	context->_video_list_stride[1] = 0;
	context->_video_list_stride[2] = 0;
}

static void func_ffmpeg_free_video(ffmpeg_video_context ** video)
{
	if(*video)
	{
		av_free((*video)->data);
		av_frame_free(&(*video)->frame);
		sws_freeContext((*video)->_sws_context);

		delete *video; *video = NULL;
	}
}

static void func_ffmpeg_draw_frame(ffmpeg_video_context * context)
{
	if(context->draw_pts != context->frame->pts)
	{
		context->draw_pts = context->frame->pts;
		sws_scale(context->_sws_context, context->frame->data, context->frame->linesize, 0, context->height, context->_video_list, context->_video_list_stride);
	}
}

typedef struct ffmpeg_movie_context
{
	AVFormatContext *format_context;

	ffmpeg_stream_context *audio_stream;
	ffmpeg_stream_context *video_stream;

}ffmpeg_movie_context;

static void func_ffmpeg_alloc_movie(ffmpeg_movie_context ** movie)
{
	(*movie) = new ffmpeg_movie_context();
	
	(*movie)->format_context = NULL;

	(*movie)->audio_stream = new ffmpeg_stream_context();
	func_ffmpeg_alloc_stream(&(*movie)->audio_stream);

	(*movie)->video_stream = new ffmpeg_stream_context();
	func_ffmpeg_alloc_stream(&(*movie)->video_stream);
}

static void func_ffmpeg_free_movie(ffmpeg_movie_context ** movie)
{
	if(*movie)
	{
		func_ffmpeg_free_stream(&(*movie)->audio_stream);
		func_ffmpeg_free_stream(&(*movie)->video_stream);

		avformat_close_input(&(*movie)->format_context);

		delete *movie; *movie = NULL;
	}
}

static int func_ffmpeg_open_movie(ffmpeg_movie_context *movie,uint8_t *data, size_t size)
{
	int ret = -1;
	
	if (!(movie->format_context = avformat_alloc_context())) 
	{
        return  ret = AVERROR(ENOMEM);
    }

	uint8_t *avio_ctx_buffer = NULL;
    if (!(avio_ctx_buffer = (uint8_t *)av_malloc(size))) 
	{
        return  ret = AVERROR(ENOMEM);
    }
	memcpy(avio_ctx_buffer, data, size);

	AVIOContext *avio_ctx = NULL;
    avio_ctx = avio_alloc_context(avio_ctx_buffer, size,
                                  0, NULL, NULL, NULL, NULL);
    if (!avio_ctx) 
	{
       return  ret = AVERROR(ENOMEM);
    }

    movie->format_context->pb = avio_ctx;

	if ((ret = avformat_open_input(&movie->format_context,NULL, NULL, NULL)) != 0) 
	{
		char buf[1024];
		av_strerror(ret, buf, 1024);
		cocos2d::log("Couldn't open file : %d(%s)",  ret, buf);
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

	if ((ret = avformat_find_stream_info(movie->format_context, NULL)) < 0) 
	{
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

	AVCodec *dec;
	ffmpeg_stream_context *audioContext = movie->audio_stream;
	ffmpeg_stream_context *videoContext = movie->video_stream;

	/* select the audio stream */
	ret = av_find_best_stream(movie->format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (true || ret < 0) 
	{
        av_log(NULL, AV_LOG_ERROR, "Cannot find a audio stream in the input file\n");
        audioContext->stream_index = -1;
		audioContext->stream_context = NULL;
    }
	else
	{
		audioContext->stream_index = ret;
		audioContext->stream_context = movie->format_context->streams[audioContext->stream_index]->codec;

		/* init the audio decoder */
		if ((ret = avcodec_open2(audioContext->stream_context, dec, NULL)) < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Cannot open audio decoder\n");
			return ret;
		}
	}

	 /* select the video stream */
	ret = av_find_best_stream(movie->format_context , AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0)
	{
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        return ret;
    }

	videoContext->stream_index = ret;
	videoContext->stream_context = movie->format_context->streams[videoContext->stream_index]->codec;

    /* init the video decoder */
    if ((ret = avcodec_open2(videoContext->stream_context, dec, NULL)) < 0)
	{
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }

	return 0;
}

static int func_ffmpeg_decode_frame(ffmpeg_movie_context *movie,ffmpeg_video_context *video,float delay)
{
	int ret = -1;

	AVRational &rate = movie->video_stream->stream_context->time_base;
	
	int64_t delayPts  = int64_t(delay * rate.den * 1000);

	delayPts = delayPts/rate.num;
	delayPts = video->revise_time + delayPts;
	
	int64_t nFramePts = int64_t(delayPts / 1000) * 1000;
	video->revise_time = delayPts - nFramePts;

	AVPacket packet;
	int got_frame = 0;

	while(nFramePts > 0)
	{
		if(nFramePts > 4000)
		{
			video->last_pts +=  nFramePts - 4000;
			av_seek_frame(movie->format_context,movie->video_stream->stream_index,video->last_pts,AVSEEK_FLAG_ANY);
		}

		if ((ret = av_read_frame(movie->format_context, &packet)) < 0)
		{
			return ret;
		}
	
		if (packet.stream_index == movie->video_stream->stream_index)
		{
			got_frame = 0;
			if ((ret = avcodec_decode_video2(movie->video_stream->stream_context, video->frame, &got_frame, &packet)) < 0) 
			{
				av_free_packet(&packet);
				av_log(NULL, AV_LOG_ERROR, "Error decoding video\n");
				return ret;
			}

			nFramePts -= 1000;
			video->last_pts += 1000; 
			
			if (got_frame && 0 <= nFramePts) 
			{
				video->frame->pts = av_frame_get_best_effort_timestamp(video->frame);
				video->last_pts = video->frame->pts;

				if(video->frame->pts != AV_NOPTS_VALUE)
				{
					func_ffmpeg_draw_frame(video);
				}
			}

			av_frame_unref(video->frame);
		}

		av_free_packet(&packet);
	}

	return 0;
}

static void func_ffmpeg_register_all()
{
	av_register_all();
	avcodec_register_all();
}

NS_CC_BEGIN

MovieNode::MovieNode()
{
	ffmpegMovieCtx = NULL;
	ffmpegVideoCtx = NULL;
}

MovieNode::~MovieNode()
{
	func_ffmpeg_free_video(&ffmpegVideoCtx);
	func_ffmpeg_free_movie(&ffmpegMovieCtx);
}

bool MovieNode::init(const std::string &pszFileName)
{
	do
	{
		mMovieState = MovieState::MOVIE_PAUSE;

		func_ffmpeg_register_all();

		func_ffmpeg_alloc_movie(&ffmpegMovieCtx);

		Data ffmpegData = FileUtils::getInstance()->getDataFromFile(pszFileName);

		CC_BREAK_IF(0 != func_ffmpeg_open_movie(ffmpegMovieCtx,ffmpegData.getBytes(),ffmpegData.getSize()));
	
		func_ffmpeg_alloc_video(&ffmpegVideoCtx,ffmpegMovieCtx->video_stream);
	
		Size ffmpegSize = Size(ffmpegVideoCtx->width, ffmpegVideoCtx->height);

		Texture2D *texture = new Texture2D();
		texture->initWithData(ffmpegVideoCtx->data,ffmpegVideoCtx->data_size,cocos2d::Texture2D::PixelFormat::RGBA8888,ffmpegVideoCtx->width, ffmpegVideoCtx->height,ffmpegSize);
		CC_BREAK_IF( !initWithTexture(texture));
		
		scheduleUpdate();

		return true;
	}while(0);

	return false;
}

MovieNode *MovieNode::create(const std::string &pszFileName)
{
	MovieNode *node = new MovieNode();
	if (node && node->init(pszFileName)) 
	{
		node->autorelease();
		return node;
	}
	CC_SAFE_DELETE(node);
	return NULL;
}

void MovieNode::update(float dt)
{
	if(MovieState::MOVIE_PLAYING == mMovieState)
	{
		if(0 != func_ffmpeg_decode_frame(ffmpegMovieCtx,ffmpegVideoCtx,dt))
		{
			mMovieState = MovieState::MOVIE_PAUSE;
		
			dispatherEvent(EventValue(MovieEvent::PLAYING_FINISH,this));
		}
	}
}

void MovieNode::play(void)
{
	mMovieState = MovieState::MOVIE_PLAYING;
}

void MovieNode::pause(void)
{
	mMovieState = MovieState::MOVIE_PAUSE;
}

void MovieNode::drawSprite(Renderer *renderer, const kmMat4& transform)
{
	GL::blendFunc(getBlendFunc().src, getBlendFunc().dst );

	auto tempTexture = getTexture();
	if (tempTexture != NULL)
	{
		GL::bindTexture2D( tempTexture->getName() );
		
		if(MovieState::MOVIE_PLAYING == mMovieState)
		{
			glTexSubImage2D(GL_TEXTURE_2D, 0,0,0,ffmpegVideoCtx->width, ffmpegVideoCtx->height,GL_RGBA, GL_UNSIGNED_BYTE,ffmpegVideoCtx->data);
		}
	}
	else
	{
		GL::bindTexture2D(0);
	}

	//
	// Attributes
	//
	GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

	#define kQuadSize sizeof(_movieCustomQuad.bl)
		
	long offset = (long)&_movieCustomQuad;

	// vertex
	int diff = offsetof( V3F_C4B_T2F, vertices);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (void*) (offset + diff));

	// texCoods
	diff = offsetof( V3F_C4B_T2F, texCoords);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

	// color
	diff = offsetof( V3F_C4B_T2F, colors);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (void*)(offset + diff));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	CHECK_GL_ERROR_DEBUG();

	CC_INCREMENT_GL_DRAWS(1);
}

void MovieNode::draw(Renderer *renderer, const kmMat4& transform, bool transformUpdated)
{
	 // Don't do calculate the culling if the transform was not updated
    _insideBounds = transformUpdated ? isInsideBounds() : _insideBounds;
	if(true || _insideBounds)
	{
		if(transformUpdated)
		{
			_movieCustomQuad = getQuad();
			convertToWorldCoordinates(&_movieCustomQuad, 1, transform);
		}

		_movieCustomCommand.init(_globalZOrder);
		_movieCustomCommand.func = 	CC_CALLBACK_0(MovieNode::drawSprite,this,renderer,transform);

		renderer->addCommand(&_movieCustomCommand);
#if CC_SPRITE_DEBUG_DRAW
        _customDebugDrawCommand.init(_globalZOrder);
        _customDebugDrawCommand.func = CC_CALLBACK_0(Sprite::drawDebugData, this);
        renderer->addCommand(&_customDebugDrawCommand);
#endif //CC_SPRITE_DEBUG_DRAW
	}
}

void MovieNode::convertToWorldCoordinates(V3F_C4B_T2F_Quad* quads, ssize_t quantity, const kmMat4& modelView)
{
//    kmMat4 matrixP, mvp;
//    kmGLGetMatrix(KM_GL_PROJECTION, &matrixP);
//    kmMat4Multiply(&mvp, &matrixP, &modelView);

    for(ssize_t i=0; i<quantity; ++i) {
        V3F_C4B_T2F_Quad *q = &quads[i];

        kmVec3 *vec1 = (kmVec3*)&q->bl.vertices;
        kmVec3Transform(vec1, vec1, &modelView);

        kmVec3 *vec2 = (kmVec3*)&q->br.vertices;
        kmVec3Transform(vec2, vec2, &modelView);

        kmVec3 *vec3 = (kmVec3*)&q->tr.vertices;
        kmVec3Transform(vec3, vec3, &modelView);

        kmVec3 *vec4 = (kmVec3*)&q->tl.vertices;
        kmVec3Transform(vec4, vec4, &modelView);
    }
}

NS_CC_END
